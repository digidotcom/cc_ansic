/*
 * Copyright (c) 2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
#if (defined CONNECTOR_TRANSPORT_SMS)
static connector_status_t sm_decode_segment(connector_data_t * const connector_ptr, connector_sm_packet_t * const recv_ptr)
{
    size_t const data_size = 1 + (recv_ptr->total_bytes * 4)/5;
    void * data_ptr = NULL;
    connector_status_t result = malloc_data_buffer(connector_ptr, data_size, named_buffer_id(sm_data_block), &data_ptr);

    if (result == connector_working)
    {
        recv_ptr->total_bytes = sm_decode85(data_ptr, data_size, recv_ptr->data, recv_ptr->total_bytes);
        memcpy(recv_ptr->data, data_ptr, recv_ptr->total_bytes);
        result = free_data_buffer(connector_ptr, named_buffer_id(sm_data_block), data_ptr);
    }

    return result;
}

static connector_status_t sm_verify_sms_preamble(connector_sm_data_t * const sm_ptr)
{
    connector_status_t result = connector_working;

    if (sm_ptr->transport.id_length > 0)
    {
        uint8_t * const data_ptr = sm_ptr->network.recv_packet.data;
        char const prefix = '(';
        char const suffix[] = "):";
        size_t const suffix_bytes = sizeof suffix - 1;
        size_t const prefix_bytes = sizeof prefix;
        size_t const suffix_position = prefix_bytes + sm_ptr->transport.id_length;
        connector_bool_t const valid_prefix = connector_bool(*data_ptr == prefix);
        connector_bool_t const valid_shared_key = connector_bool(memcmp(data_ptr + prefix_bytes, sm_ptr->transport.id, sm_ptr->transport.id_length) == 0);
        connector_bool_t const valid_suffix = connector_bool(memcmp(data_ptr + suffix_position, suffix, suffix_bytes) == 0);

        if (valid_prefix && valid_shared_key && valid_suffix)
            sm_ptr->network.recv_packet.processed_bytes = suffix_position + suffix_bytes;
        else
            result = connector_invalid_response;
    }

    return result;
}
#endif

#if (defined CONNECTOR_TRANSPORT_UDP)
static connector_status_t sm_verify_udp_header(connector_sm_data_t * const sm_ptr)
{
    connector_status_t result = connector_invalid_response;
    connector_sm_packet_t * const recv_ptr = &sm_ptr->network.recv_packet;
    uint8_t * data_ptr = recv_ptr->data;
    uint8_t const version_and_type = *data_ptr++;
    uint8_t const version = version_and_type >> 4;
    connector_sm_id_type_t const type = (connector_sm_id_type_t)(version_and_type & 0x0F);

    if (version != SM_UDP_VERSION)
    {
        connector_debug_printf("sm_verify_udp_header: invalid SM UDP version [%d]\n", version);
        result = connector_abort;
        goto error;
    }

    if (type != sm_ptr->transport.id_type)
        goto done;

    if (memcmp(data_ptr, sm_ptr->transport.id, sm_ptr->transport.id_length))
        goto done; /* not for us */

    recv_ptr->processed_bytes = sm_ptr->transport.id_length + 1;
    result = connector_working;

done:
error:
    return result;
}
#endif

#if (defined CONNECTOR_SM_MULTIPART)
static connector_status_t sm_multipart_allocate(connector_data_t * const connector_ptr, connector_sm_data_t * const sm_ptr, connector_sm_session_t * const session, size_t const segments)
{
    connector_status_t status = connector_working;
    size_t const max_payload_bytes = sm_get_max_payload_bytes(sm_ptr);
    size_t const max_session_bytes = (((max_payload_bytes * segments) + 3)/4) * 4; /* make sure it is word aligned */
    size_t const size_array_bytes = sizeof(*session->segments.size_array) * segments;

    ASSERT_GOTO(session->in.data == NULL, error);
    session->in.bytes = max_session_bytes + size_array_bytes;
    status = sm_allocate_user_buffer(connector_ptr, &session->in);
    ASSERT_GOTO(status == connector_working, error);
    session->segments.size_array = (void *)(session->in.data + max_session_bytes); /* alignment issue is taken care where max_session_bytes is defined */
    memset(session->segments.size_array, 0, size_array_bytes);

error:
    return status;
}
#endif

#if (defined CONNECTOR_SM_BATTERY)
static connector_status_t sm_more_data_callback(connector_data_t * const connector_ptr)
{
    connector_status_t result;
    connector_request_t request_id;
    connector_callback_status_t callback_status;

    request_id.sm_request = connector_sm_more_data;
    callback_status = connector_callback_no_response(connector_ptr->callback, connector_class_sm, request_id, NULL, 0);
    result = sm_map_callback_status_to_connector_status(callback_status);

    return result;
}
#endif

static connector_status_t sm_process_header(connector_data_t * const connector_ptr, connector_sm_data_t * const sm_ptr)
{
    connector_sm_session_t * session = NULL;
    connector_status_t result = connector_invalid_response;
    connector_sm_packet_t * const recv_ptr = &sm_ptr->network.recv_packet;
    size_t sm_header_size = 5;
    #if (defined CONNECTOR_SM_MULTIPART)
    size_t const max_payload_bytes = sm_get_max_payload_bytes(sm_ptr);
    #endif
    size_t sm_bytes = recv_ptr->total_bytes - recv_ptr->processed_bytes;
    #if (defined CONNECTOR_SM_BATTERY)
    connector_bool_t isPackCmd = connector_false;
    #endif

    do
    {
        uint8_t * data_ptr = &recv_ptr->data[recv_ptr->processed_bytes];
        uint8_t * const segment = data_ptr;
        uint8_t info_field = message_load_u8(segment, info);
        uint8_t const request_id_high_bits_mask = 0x03;
        connector_bool_t const client_originated = SmIsRequest(info_field) ? connector_false : connector_true;
        uint32_t const request_id = ((info_field & request_id_high_bits_mask) << 8) + message_load_u8(segment, request);
        uint8_t segment_number = 0;
        uint8_t segment_count = 1;
        uint8_t cs_byte = 0;
        uint16_t crc = 0;

        info_field &= ~request_id_high_bits_mask;
        ASSERT_GOTO((sm_bytes >= sm_header_size) && (sm_bytes <= sm_ptr->transport.ms_mtu), error);
        #if (defined CONNECTOR_SM_MULTIPART)
        if (SmIsMultiPart(info_field))
        {
            uint8_t * const segment0 = data_ptr;

            segment_number = message_load_u8(segment0, segment);
            if (segment_number > 0)
            {
                uint8_t * const segmentn = data_ptr;

                #if (defined CONNECTOR_SM_BATTERY)
                if (isPackCmd)
                {
                    sm_header_size = (record_end(segmentn) - sizeof crc);
                }
                else
                #endif
                {
                    crc = message_load_be16(segmentn, crc);
                    message_store_be16(segmentn, crc, 0);
                    sm_header_size = record_end(segmentn);
                }
            }
            else
            {
                segment_count = message_load_u8(segment0, count);
                cs_byte = message_load_u8(segment0, cmd_status);
                #if (defined CONNECTOR_SM_BATTERY)
                if (isPackCmd)
                {
                    sm_header_size = (record_end(segment0) - sizeof crc);
                }
                else
                #endif
                {
                    crc = message_load_be16(segment0, crc);
                    message_store_be16(segment0, crc, 0);
                    sm_header_size = record_end(segment0);
                }
            }
        }
        else
        #endif
        {
            if (SmIsMultiPart(info_field))
            {
                connector_debug_printf("sm_process_header : Multi-part is disabled, discarding SM packet...\n");
                goto done;
            }

            cs_byte = message_load_u8(segment, cmd_status);
            #if (defined CONNECTOR_SM_BATTERY)
            if (isPackCmd)
            {
                sm_header_size = (record_end(segment) - sizeof crc);
            }
            else
            #endif
            {
                crc = message_load_be16(segment, crc);
                message_store_be16(segment, crc, 0);
                sm_header_size = record_end(segment);
            }
        }

        #if (!defined CONNECTOR_COMPRESSION)
        if (SmIsCompressed(cs_byte))
        {
            connector_debug_printf("sm_process_header: Received compressed packet, but compression is disabled!\n");
            goto done; /* silently ignore the packet */
        }
        #endif

        recv_ptr->processed_bytes += sm_header_size;
        #if (defined CONNECTOR_SM_BATTERY)
        if (!isPackCmd)
        #endif
        {
            uint16_t calculated_crc = 0;

            calculated_crc = sm_calculate_crc16(calculated_crc, data_ptr, sm_bytes);
            if(calculated_crc != crc)
            {
                connector_debug_printf("sm_process_header: crc error!\n");
                goto done; /* silently ignore the packet */
            }

            #if (defined CONNECTOR_SM_BATTERY)
            if ((cs_byte & 0x7F) == connector_sm_cmd_pack)
            {
                #define SM_MESSAGE_PENDING 0x01
                enum sm_pack_t
                {
                    field_define(pack_header, flag, uint8_t),
                    field_define(pack_header, length, uint16_t),
                    record_end(pack_header)
                };
                uint8_t * const pack_header = &recv_ptr->data[recv_ptr->processed_bytes];
                uint8_t const flag = message_load_u8(pack_header, flag);

                if ((flag & SM_MESSAGE_PENDING) == SM_MESSAGE_PENDING)
                {
                    result = sm_more_data_callback(connector_ptr);
                    if (result != connector_working) goto done;
                }

                isPackCmd = connector_true;
                sm_bytes = message_load_be16(pack_header, length);
                sm_header_size = record_end(pack_header);
                recv_ptr->processed_bytes += sm_header_size;
                #undef SM_MESSAGE_PENDING
                continue;
            }
            #endif
        }

        sm_bytes -= sm_header_size;
        session = get_sm_session(sm_ptr, request_id, client_originated);
        if (session == NULL)
        {
            if ((segment_count > sm_ptr->session.max_segments) || (segment_number >= sm_ptr->session.max_segments))
            {
                connector_debug_printf("sm_process_header: Exceeded maximum segments [%zu/%zu]!\n", segment_count, sm_ptr->session.max_segments);
                goto error;
            }

            session = sm_create_session(connector_ptr, sm_ptr, connector_false);
            if (session == NULL)
            {
                result = connector_pending;
                goto error;
            }

            session->request_id = request_id;
            if (SmIsResponse(info_field)) /* Is session cancelled? */
                session->command = connector_sm_cmd_opaque_response;
        }
        else
        {
            if (segment_number >= sm_ptr->session.max_segments)
            {
                session->sm_state = connector_sm_state_error;
                session->error = connector_session_status_cloud_error;
                SmSetError(session->flags);
            }
        }

        #if (defined CONNECTOR_SM_MULTIPART)
        if (SmIsMultiPart(info_field))
        {
            SmSetMultiPart(session->flags);
        }

        if (segment_number == 0)
        #endif
        {
            uint8_t const cmd_status = cs_byte & 0x7F;

            if (SmIsCompressed(cs_byte))
                SmSetCompressed(session->flags);

            if (client_originated == connector_false)
            {
                if (SmIsResponseNeeded(info_field))
                    SmSetResponseNeeded(session->flags);
                session->command = (connector_sm_cmd_t)cmd_status;
            }
            else
            {
                if (SmIsError(cmd_status))
                    SmSetError(session->flags);
            }

            result = connector_pending;
            #if (defined CONNECTOR_SM_MULTIPART)
            if (SmIsMultiPart(info_field) && (segment_count > 1))
            {
                if (session->in.data == NULL)
                {
                    result = sm_multipart_allocate(connector_ptr, sm_ptr, session, segment_count);
                    ASSERT_GOTO(result == connector_working, error);
                    ASSERT_GOTO(session->in.data != NULL, error);
                }
                session->segments.size_array[segment_number] = sm_bytes;
                memcpy(session->in.data, &recv_ptr->data[recv_ptr->processed_bytes], sm_bytes);
            }
            else
            #endif
            {
                session->in.bytes = sm_bytes;
                if (sm_bytes > 0)
                {
                    result = sm_allocate_user_buffer(connector_ptr, &session->in);
                    ASSERT_GOTO(result == connector_working, error);
                    memcpy(session->in.data, &recv_ptr->data[recv_ptr->processed_bytes], sm_bytes);
                }
                else
                    session->in.data = NULL;
            }

            session->segments.count = segment_count;
            session->segments.processed = 1;
        }
        #if (defined CONNECTOR_SM_MULTIPART)
        else
        {

            if (session->in.data == NULL)
            {
                result = sm_multipart_allocate(connector_ptr, sm_ptr, session, sm_ptr->session.max_segments);
                ASSERT_GOTO(result == connector_working, error);
                ASSERT_GOTO(session->in.data != NULL, error);
            }

            if (session->segments.size_array[segment_number] == 0)
            {
                uint8_t * copy_to = session->in.data + (segment_number * max_payload_bytes);

                session->segments.size_array[segment_number] = sm_bytes;
                session->segments.processed++;
                memcpy(copy_to, &recv_ptr->data[recv_ptr->processed_bytes], sm_bytes);
            }
            else
            {
                connector_debug_printf("sm_process_header: duplicate segment %d, in id %d\n", segment_number, session->request_id);
            }
        }
        #endif

        if (session->segments.processed == session->segments.count)
        {
            session->bytes_processed = 0;
            session->segments.processed = 0;

            #if (defined CONNECTOR_COMPRESSION)
            if (SmIsCompressed(session->flags))
            {
                session->compress.out.data = NULL;
                session->sm_state = connector_sm_state_decompress;
            }
            else
            #endif
            {
                ASSERT(SmNotCompressed(session->flags));
                session->sm_state = connector_sm_state_process_payload;
            }
        }

        recv_ptr->processed_bytes += sm_bytes;
        #if (defined CONNECTOR_SM_BATTERY)
        if ((isPackCmd) && (recv_ptr->processed_bytes < recv_ptr->total_bytes))
        {
            data_ptr = &recv_ptr->data[recv_ptr->processed_bytes];
            sm_bytes = LoadBE16(data_ptr);
            recv_ptr->processed_bytes += sizeof(uint16_t);
        }
        #else
        break;
        #endif
    } while (recv_ptr->processed_bytes < recv_ptr->total_bytes);

done:
    result = connector_working;

error:
    recv_ptr->total_bytes = 0;
    return result;
}

static connector_status_t sm_receive_data(connector_data_t * const connector_ptr, connector_sm_data_t * const sm_ptr)
{
    connector_status_t result = connector_pending;
    connector_sm_packet_t * const recv_ptr = &sm_ptr->network.recv_packet;

    if (recv_ptr->total_bytes > 0)
        goto done;

    {
        connector_request_t request_id;
        connector_read_request_t read_data;
        connector_callback_status_t status;
        size_t bytes_read = 0;
        size_t resp_size = sizeof bytes_read;

        read_data.timeout = 0;
        read_data.network_handle = sm_ptr->network.handle;
        read_data.buffer = recv_ptr->data;
        read_data.length = sm_ptr->transport.mtu;

        request_id.network_request = connector_request_id_network_receive;
        status = connector_callback(connector_ptr->callback, sm_ptr->network.class_id, request_id, &read_data, sizeof read_data, &bytes_read, &resp_size);
        switch (status)
        {
           case connector_callback_busy:
                result = connector_idle;
                goto done;

            case connector_callback_continue:
                recv_ptr->total_bytes = bytes_read;
                recv_ptr->processed_bytes = 0;

                switch (sm_ptr->network.class_id)
                {
                    #if (defined CONNECTOR_TRANSPORT_SMS)
                    case connector_class_network_sms:
                        result = sm_decode_segment(connector_ptr, recv_ptr);
                        if(result != connector_working) goto done; /* not iDigi packet? */
                        result = sm_verify_sms_preamble(sm_ptr);
                        break;
                    #endif

                    #if (defined CONNECTOR_TRANSPORT_UDP)
                    case connector_class_network_udp:
                        result = sm_verify_udp_header(sm_ptr);
                        break;
                    #endif

                    default:
                        ASSERT_GOTO(connector_false, error);
                        break;
                }

                if(result != connector_working) goto done; /* not iDigi packet? */
                result = sm_process_header(connector_ptr, sm_ptr);
                break;

            case connector_callback_abort:
                result = connector_abort;
                break;

            default:
                connector_debug_printf("tcp_receive_buffer: callback returned error [%d]\n", result);
                result = connector_no_resource;
                break;
        }
    }

    sm_verify_result(sm_ptr, &result);

error:
done:
    return result;
}

#if (defined CONNECTOR_COMPRESSION)
static connector_status_t sm_decompress_data(connector_data_t * const connector_ptr, connector_sm_data_t * const sm_ptr, connector_sm_session_t * const session)
{
    connector_status_t status = connector_working;
    size_t const max_payload_bytes = sm_get_max_payload_bytes(sm_ptr);
    uint8_t zlib_header[] = {0x58, 0xC3};
    z_streamp const zlib_ptr = &session->compress.zlib;
    int zret;

    if (session->compress.out.data == NULL)
    {
        session->compress.out.bytes = max_payload_bytes;
        status = sm_allocate_user_buffer(connector_ptr, &session->compress.out);
        ASSERT_GOTO(status == connector_working, done);

        memset(zlib_ptr, 0, sizeof *zlib_ptr);
        zret = inflateInit(zlib_ptr);
        ASSERT_GOTO(zret == Z_OK, error);
        zlib_ptr->next_out = session->compress.out.data;
        zlib_ptr->avail_out = session->compress.out.bytes;
        zlib_ptr->next_in = zlib_header;
        zlib_ptr->avail_in = sizeof zlib_header;
    }

    while (zlib_ptr->avail_out > 0)
    {
        if (zlib_ptr->avail_in == 0)
        {
            if (session->segments.processed == session->segments.count)
            {
                SmSetLastData(session->flags);
                break;
            }
            else
            {
                size_t const data_index = session->segments.processed * max_payload_bytes;

                zlib_ptr->next_in = &session->in.data[data_index];
                zlib_ptr->avail_in = SmIsMultiPart(session->flags) ? session->segments.size_array[session->segments.processed] : session->in.bytes;
                session->segments.processed++;
            }
        }

        zret = inflate(zlib_ptr, Z_NO_FLUSH);
        switch(zret)
        {
            case Z_STREAM_END:
            case Z_BUF_ERROR:
            case Z_OK:
                break;

            default:
                status = connector_abort;
                connector_debug_printf("ZLIB Return value [%d]\n", zret);
                ASSERT_GOTO(connector_false, error);
                break;
        }
    }

    {
        size_t const payload_bytes = session->compress.out.bytes - zlib_ptr->avail_out;

        status = sm_pass_user_data(connector_ptr, session, session->compress.out.data, payload_bytes);
        switch (status)
        {
            case connector_pending:
                goto done;

            case connector_working:
                if (SmIsNotLastData(session->flags))
                {
                    zlib_ptr->next_out = session->compress.out.data;
                    zlib_ptr->avail_out = session->compress.out.bytes;
                    goto done;
                }
                break;

            default:
                break;
        }
    }

error:
    zret = inflateEnd(zlib_ptr);
    ASSERT(zret == Z_OK);

    if (status != connector_abort)
    {
        status = free_data_buffer(connector_ptr, named_buffer_id(sm_data_block), session->compress.out.data);
        session->compress.out.data = NULL;
    }

done:
    return status;
}
#endif

static connector_status_t sm_handle_error(connector_data_t * const connector_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_working;
    connector_sm_state_t next_state = connector_sm_state_complete;

    if (SmIsCloudOwned(session->flags) && SmIsRequest(session->flags))
        next_state = connector_sm_state_send_data;

    SmSetError(session->flags);
    if (session->user.context != NULL) /* let the user know */
    {
        result = sm_inform_session_complete(connector_ptr, session);
        if (result != connector_working) goto error;
    }

    result = sm_switch_path(connector_ptr, session, next_state);

error:
    return result;
}

static connector_status_t sm_handle_complete(connector_data_t * const connector_ptr, connector_sm_data_t * const sm_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_working;

    if (session->in.data != NULL)
    {
        result = free_data_buffer(connector_ptr, named_buffer_id(sm_data_block), session->in.data);
        if (result != connector_working) goto error;
    }

    if (SmIsReboot(session->flags))
        result = sm_process_reboot(connector_ptr);

    if (result != connector_pending)
        result = sm_delete_session(connector_ptr, sm_ptr, session);

error:
    return result;
}

static connector_status_t sm_process_recv_path(connector_data_t * const connector_ptr, connector_sm_data_t * const sm_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_abort;

    ASSERT_GOTO(session != NULL, error);
    switch (session->sm_state)
    {
        case connector_sm_state_receive_data:
            if (sm_ptr->timeout_in_seconds != SM_WAIT_FOREVER)
            {
                unsigned long current_time = 0;

                result = get_system_time(connector_ptr, &current_time);
                ASSERT_GOTO(result == connector_working, error);
                if (current_time > (session->start_time + sm_ptr->timeout_in_seconds))
                {
                    session->sm_state = connector_sm_state_error;
                    session->error = connector_session_status_timeout;
                    connector_debug_printf("Sm session [%u] timeout... start time:%u, current time:%u\n", session->request_id, session->start_time, current_time);
                }
            }

            result = connector_idle; /* still receiving data, handled in sm_receive_data() */
            break;

        #if (defined CONNECTOR_COMPRESSION)
        case connector_sm_state_decompress:
            result = sm_decompress_data(connector_ptr, sm_ptr, session);
            break;
        #endif

        case connector_sm_state_process_payload:
            result = sm_process_payload(connector_ptr, sm_ptr, session);
            break;

        case connector_sm_state_complete:
            result = sm_handle_complete(connector_ptr, sm_ptr, session);
            break;

        case connector_sm_state_error:
            result = sm_handle_error(connector_ptr, session);
            break;

        default:
            ASSERT(connector_false);
            break;
    }

    sm_verify_result(sm_ptr, &result);

error:
    return result;
}
