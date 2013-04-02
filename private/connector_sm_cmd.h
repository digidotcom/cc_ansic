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
static connector_status_t sm_copy_user_request(connector_sm_data_t * const sm_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_abort;
    connector_bool_t response_needed;

    ASSERT_GOTO(sm_ptr->pending.data != NULL, error);
    session->bytes_processed = 0;

    switch (sm_ptr->pending.request)
    {
        case connector_initiate_ping_request:
        {
            connector_sm_ping_request_t const * const request = sm_ptr->pending.data;

            session->user.context = request->user_context;
            session->user.header = NULL;
            session->command = connector_sm_cmd_ping;
            response_needed = request->response_required;
            session->sm_state = connector_sm_state_prepare_segment;
            break;
        }

        case connector_initiate_send_data:
        {
            connector_request_data_service_send_t const * const request = sm_ptr->pending.data;

            session->user.context = request->user_context;
            session->user.header = request->path;
            session->command = (request->path != NULL) ? connector_sm_cmd_data : connector_sm_cmd_no_path_data;
            response_needed = request->response_required;
            session->sm_state = connector_sm_state_get_total_length;
            #if (defined CONNECTOR_DATA_POINTS)
            {
                char const dp_prefix[] = "DataPoint/";
                size_t const dp_prefix_bytes = sizeof dp_prefix - 1;

                if (!strncmp(request->path, dp_prefix, dp_prefix_bytes))
                    SmSetDatapoint(session->flags);
            }
            #endif
            break;
        }

        #if (defined CONNECTOR_TRANSPORT_SMS)
        case connector_initiate_config_message:
        {
            connector_device_to_server_config_t const * const request = sm_ptr->pending.data;

            ASSERT_GOTO(request->transport == connector_transport_sms, error);
            session->user.context = request->user_context;
            session->user.header = request;
            session->command = connector_sm_cmd_config;
            response_needed = request->response_required;
            session->sm_state = connector_sm_state_prepare_payload;
            break;
        }
        #endif

        default:
            ASSERT_GOTO(connector_false, error);
            break;
    }

    if (response_needed) SmSetResponseNeeded(session->flags);

    result = connector_working;

error:
    return result;
}

static void sm_verify_result(connector_sm_data_t * const sm_ptr, connector_status_t * const result)
{
    switch (*result)
    {
        case connector_pending:
        case connector_working:
        case connector_idle:
            goto done;

        case connector_abort:
        case connector_invalid_response:
            sm_ptr->close.status = connector_close_status_abort;
            break;

        default:
            sm_ptr->close.status = connector_close_status_device_error;
            break;
    }

    *result = connector_working;
    switch(sm_ptr->transport.state)
    {
        case connector_transport_idle:
        case connector_transport_close:
        case connector_transport_terminate:
            break;

        default:
            sm_ptr->transport.state = connector_transport_close;
            break;
    }

done:
    return;
}

#if (defined CONNECTOR_COMPRESSION) || (defined CONNECTOR_SM_MULTIPART)
static size_t sm_get_max_payload_bytes(connector_sm_data_t * const sm_ptr)
{
    size_t const sm_header_size = 5;
    size_t const max_payload_bytes = sm_ptr->transport.ms_mtu - sm_header_size;

    return max_payload_bytes;
}
#endif

static connector_status_t sm_allocate_user_buffer(connector_data_t * const connector_ptr, sm_data_block_t * const dblock)
{
    void * ptr = NULL;
    connector_status_t result = connector_working;

    if (dblock->bytes > 0)
    {
        ASSERT(dblock->data == NULL);
        result = malloc_data_buffer(connector_ptr, dblock->bytes, named_buffer_id(sm_data_block), &ptr);
    }

    dblock->data = ptr;

    return result;
}

static connector_status_t sm_map_callback_status_to_connector_status(connector_callback_status_t const callback_status)
{
    connector_status_t result;

    switch (callback_status)
    {
        case connector_callback_continue:
            result = connector_working;
            break;

        case connector_callback_busy:
            result = connector_pending;
            break;

        default:
            result = connector_abort;
            break;
    }

    return result;
}

static connector_status_t sm_inform_session_complete(connector_data_t * const connector_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_working;
    connector_request_id_t request_id;
    connector_callback_status_t callback_status = connector_callback_continue;

    if (session->sm_state == connector_sm_state_complete) goto done;

    switch (session->command)
    {
        case connector_sm_cmd_data:
        case connector_sm_cmd_no_path_data:
        {
            connector_data_service_send_status_t status_info;

            status_info.transport = session->transport;
            status_info.user_context = session->user.context;
            status_info.session_error = connector_session_error_none;
            switch (session->error)
            {
                case connector_session_error_cancel:
                    status_info.status = connector_data_service_send_status_cancel;
                    break;

                case connector_session_error_timeout:
                    status_info.status = connector_data_service_send_status_timeout;
                    break;

                case connector_session_error_none:
                    status_info.status = connector_data_service_send_status_complete;
                    break;

                default:
                    status_info.status = connector_data_service_send_status_session_error;
                    status_info.session_error = session->error;
                    break;
            }

            #if (defined CONNECTOR_DATA_POINTS)
            if (SmIsDatapoint(session->flags))
            {
                callback_status = dp_handle_callback(connector_ptr, connector_request_id_data_service_send_status, &status_info);
            }
            else
            #endif
            {
                size_t response_bytes = sizeof session->error;

                request_id.data_service_request = SmIsClientOwned(session->flags) ? connector_request_id_data_service_send_status : connector_request_id_data_service_receive_status;
                callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_service, request_id, &status_info);
            }
            break;
        }

        #if (defined CONNECTOR_SM_CLI)
        case connector_sm_cmd_cli:
        {
            connector_sm_cli_request_t cli_request;
            size_t request_size = sizeof cli_request;

            cli_request.user_context = session->user.context;
            cli_request.type = connector_data_service_type_session_status;
            cli_request.content.status = session->error;
            request_id.sm_request = connector_sm_cli_request;
            callback_status = connector_callback_no_request_data(connector_ptr->callback, connector_class_id_short_message, request_id, &cli_request, &request_size);

            break;
        }
        #endif

        default:
        {
            connector_message_status_response_t response_data;
            connector_class_id_t class_id;

            response_data.user_context = session->user.context;
            response_data.status = session->error;
            response_data.error_text = NULL;

            switch (session->command)
            {
                case connector_sm_cmd_config:
                    class_id = connector_class_id_short_message;
                    request_id.sm_request = SmIsClientOwned(session->flags) ? connector_sm_device_to_server_config : connector_sm_server_to_device_config;
                    break;

                case connector_sm_cmd_ping:
                    class_id = connector_class_id_status;
                    request_id.status_request = SmIsClientOwned(session->flags) ? connector_status_ping_response : connector_status_ping_request;
                    break;

                default:
                    goto done;
            }

            callback_status = connector_callback_no_response(connector_ptr->callback, class_id, request_id, &response_data, sizeof response_data);
            break;
        }
    }

    if (callback_status == connector_callback_abort)
    {
        result = connector_abort;
        goto error;
    }

done:
    if (session->in.data != NULL)
    {
        result = free_data_buffer(connector_ptr, named_buffer_id(sm_data_block), session->in.data);
        session->in.bytes = 0;
        session->in.data = NULL;
    }

error:
    return result;
}

static connector_status_t sm_switch_path(connector_data_t * const connector_ptr, connector_sm_session_t * const session, connector_sm_state_t const next_state)
{
    connector_status_t result = connector_working;

    if (session->in.data != NULL)
    {
        result = free_data_buffer(connector_ptr, named_buffer_id(sm_data_block), session->in.data);
        session->in.bytes = 0;
        session->in.data = NULL;
        if (result != connector_working) goto error;
    }

    if (SmIsResponseNeeded(session->flags))
    {
        session->sm_state = next_state;
        SmClearCompressed(session->flags);
        SmClearMultiPart(session->flags);
        SmSetResponse(session->flags);
        session->segments.processed = 0;
    }
    else
    {
        if (SmIsClientOwned(session->flags))
        {
            session->error = connector_session_status_complete;
            result = sm_inform_session_complete(connector_ptr, session);
        }

        session->sm_state = connector_sm_state_complete;
    }

error:
    return result;
}

static void sm_set_payload_process(connector_sm_session_t * const session)
{
    size_t const zlib_header_bytes = 2;

    session->in.bytes = session->bytes_processed;
    session->sm_state = connector_sm_state_prepare_segment;
    session->bytes_processed = SmIsCompressed(session->flags) ? zlib_header_bytes : 0;
}

static void sm_set_payload_complete(connector_sm_session_t * const session)
{
    ASSERT(session->bytes_processed <= session->in.bytes);

    #if (defined CONNECTOR_COMPRESSION)
    session->sm_state = connector_sm_state_compress;
    #else
    sm_set_payload_process(session);
    #endif
}

static void sm_set_header_complete(connector_sm_session_t * const session)
{
    if (session->bytes_processed < session->in.bytes)
        session->sm_state = connector_sm_state_more_data;
    else
        sm_set_payload_complete(session);
}

static connector_status_t sm_prepare_data_request(connector_data_t * const connector_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_abort;
    size_t const path_length_field_bytes = 1;
    char const * path = NULL;
    size_t path_len = 0;

    if (session->command == connector_sm_cmd_data)
    {
        connector_data_service_put_request_t const * request = session->user.header;
        size_t const five_bits_max_len = (1 << 5);

        path = request->path;
        ASSERT_GOTO(path != NULL, error);
        path_len = strlen(path);
        ASSERT_GOTO(path_len < five_bits_max_len, error);
        session->in.bytes += (path_length_field_bytes + path_len);
    }

    session->bytes_processed = 0;
    result = sm_allocate_user_buffer(connector_ptr, &session->in);
    if (result != connector_working)
    {
        session->error = connector_session_status_no_resource;
        goto error;
    }

    if (path != NULL)
    {
        uint8_t * header = session->in.data;

        *header++ = path_len;
        memcpy(header, path, path_len);
        session->bytes_processed = path_length_field_bytes + path_len;
    }

    sm_set_header_complete(session);

error:
    return result;
}

static connector_status_t sm_prepare_data_response(connector_data_t * const connector_ptr, connector_sm_session_t * const session)
{
    connector_status_t const result = sm_allocate_user_buffer(connector_ptr, &session->in);

    if (result == connector_working)
    {
        session->bytes_processed = 0;
        sm_set_header_complete(session);
    }
    else
        session->error = connector_session_status_no_resource;

    return result;
}

#if (defined CONNECTOR_SM_CLI)
static connector_status_t sm_process_cli_request(connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * const payload, size_t const bytes)
{
    connector_status_t result = connector_abort;
    char * const cli_command = payload;
    size_t const cli_bytes = strlen(cli_command) + 1; /* +1 for nul-terminate */

    if (bytes > cli_bytes)
    {
        size_t const max_response_bytes = LoadBE16(cli_command + cli_bytes);

        session->user.header = (void *)max_response_bytes;
    }
    else
    {
        session->user.header = NULL;
    }

    {
        connector_request_id_t request_id;
        connector_sm_cli_request_t cli_request;
        size_t request_size = sizeof cli_request;
        connector_callback_status_t callback_status;

        cli_request.user_context = NULL;
        cli_request.type = connector_data_service_type_have_data;
        cli_request.content.request.buffer = cli_command;
        cli_request.content.request.bytes = cli_bytes;
        cli_request.content.request.flags = (SmIsResponseNeeded(session->flags)) ? 0 : CONNECTOR_MSG_RESPONSE_NOT_NEEDED;

        request_id.sm_request = connector_sm_cli_request;
        callback_status = connector_callback_no_request_data(connector_ptr->callback, connector_class_id_short_message, request_id, &cli_request, &request_size);
        result = sm_map_callback_status_to_connector_status(callback_status);
        if (callback_status == connector_callback_continue)
            session->user.context = cli_request.user_context;
    }

    return result;
}

static connector_status_t sm_prepare_cli_response(connector_data_t * const connector_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_abort;
    connector_callback_status_t status;
    connector_request_id_t request_id;
    connector_sm_cli_request_t cli_response;
    size_t response_bytes = sizeof cli_response;
    size_t const allowed_bytes = (session->user.header != NULL) ? (size_t)session->user.header : SIZE_MAX;

    if (session->in.bytes > allowed_bytes)
        session->in.bytes = allowed_bytes;

    if (session->in.data == NULL)
    {
        result = sm_allocate_user_buffer(connector_ptr, &session->in);
        if (result != connector_working)
            goto error;
    }

    cli_response.user_context = session->user.context;
    cli_response.type = connector_data_service_type_need_data;
    cli_response.content.response.status = connector_session_status_success;
    cli_response.content.response.bytes = session->in.bytes;
    cli_response.content.response.buffer = (char *)session->in.data;
    request_id.sm_request = connector_sm_cli_request;

    status = connector_callback_no_request_data(connector_ptr->callback, connector_class_id_short_message, request_id, &cli_response, &response_bytes);
    result = sm_map_callback_status_to_connector_status(status);
    if (status == connector_callback_continue)
    {
        ASSERT_GOTO(response_bytes == sizeof cli_response, error);
        ASSERT_GOTO(cli_response.content.response.bytes <= allowed_bytes, error);

        {
            connector_bool_t const empty_response = connector_bool((cli_response.content.response.bytes == 0) && (cli_response.content.response.buffer == NULL));
            connector_bool_t const non_empty_response = connector_bool((cli_response.content.response.bytes > 0) && (cli_response.content.response.buffer != NULL));

            ASSERT_GOTO(empty_response || non_empty_response, error);
        }

        session->bytes_processed = cli_response.content.response.bytes;
        session->error = cli_response.content.response.status;
        sm_set_payload_complete(session);
    }

error:
    return result;
}
#endif

#if (defined CONNECTOR_TRANSPORT_SMS)
static connector_status_t sm_prepare_config_request(connector_data_t * const connector_ptr, connector_sm_session_t * const session)
{
    connector_status_t result;
    uint8_t const device_id_prefix[] = {'0', 'x'};
    size_t const device_id_prefix_size = sizeof device_id_prefix;
    connector_device_to_server_config_t const * const request = session->user.header;
    size_t const sim_slot_number_bytes = 1;
    size_t const id_bytes = strlen(request->identifier) + 1; /* +1 for nul-termination */

    session->in.bytes = device_id_prefix_size + DEVICE_ID_LENGTH;
    session->in.bytes += sim_slot_number_bytes + id_bytes;
    result = sm_allocate_user_buffer(connector_ptr, &session->in);
    if (result != connector_working)
    {
        session->error = connector_session_status_no_resource;
        goto error;
    }

    {
        uint8_t * data_ptr = session->in.data;

        memcpy(data_ptr, device_id_prefix, device_id_prefix_size);
        data_ptr += device_id_prefix_size;
        ASSERT(connector_ptr->device_id != NULL);
        memcpy(data_ptr, connector_ptr->device_id, DEVICE_ID_LENGTH);
        data_ptr += DEVICE_ID_LENGTH;
        ASSERT(request->sim_slot <= UCHAR_MAX);
        *data_ptr++ = (uint8_t)request->sim_slot;
        memcpy(data_ptr, request->identifier, id_bytes);
        data_ptr += id_bytes;
        session->bytes_processed = data_ptr - session->in.data;
    }

    sm_set_payload_complete(session);

error:
    return result;
}

static connector_status_t sm_process_config_request(connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * const payload, size_t const bytes)
{
    connector_status_t result = connector_success;
    connector_server_to_device_config_t config_request;

    ASSERT(SmIsLastData(session->flags));
    config_request.phone_number = payload;

    {
        size_t const phone_bytes = strlen(config_request.phone_number) + 1;

        config_request.service_id = (phone_bytes < bytes) ? config_request.phone_number + phone_bytes : NULL;
    }

    {
        connector_request_id_t request_id;
        size_t response_data_length = sizeof session->error;
        connector_callback_status_t callback_status;

        request_id.sm_request = connector_sm_server_to_device_config;
        callback_status = connector_callback(connector_ptr->callback, connector_class_id_short_message, request_id, &config_request,
                                         sizeof config_request, &session->error, &response_data_length);
        result = sm_map_callback_status_to_connector_status(callback_status);
    }

    return result;
}
#endif

static connector_status_t sm_process_data_command(connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * const payload, size_t const bytes)
{
    #define MAX_PATH_LEN  32
    connector_status_t status = connector_abort;
    connector_data_service_msg_request_t request_data;
    connector_data_service_msg_response_t response_data;
    connector_data_service_block_t server_data;
    connector_data_service_device_request_t device_request;
    uint8_t * data_ptr = payload;
    char path[MAX_PATH_LEN];
    size_t path_length = 0;

    request_data.message_type = connector_data_service_type_have_data;
    request_data.service_context = SmIsRequest(session->flags) ? &device_request : (void *)session->user.context;
    request_data.server_data = &server_data;
    server_data.flags = SmIsLastData(session->flags) ? CONNECTOR_MSG_LAST_DATA : 0;
    if (SmIsNoResponseNeeded(session->flags))
        server_data.flags |= CONNECTOR_MSG_RESPONSE_NOT_NEEDED;
    device_request.target = NULL;

    /* first packet? */
    if (session->bytes_processed == 0)
    {
        server_data.flags |= CONNECTOR_MSG_FIRST_DATA;
        if (SmIsRequest(session->flags))
            session->user.context = NULL;

        if ((session->command == connector_sm_cmd_data) && SmIsRequest(session->flags))
        {
            path_length = (*data_ptr++) & 0x1F;
            memcpy(path, data_ptr, path_length);
            path[path_length] = '\0';
            device_request.target = path;
            data_ptr += path_length;
            path_length++; /* increment for length byte */
        }
    }

    server_data.flags |= SmIsError(session->flags) ? CONNECTOR_MSG_BAD_REQUEST : CONNECTOR_MSG_RESP_SUCCESS;
    device_request.device_handle = session;
    server_data.data = data_ptr;
    server_data.length_in_bytes = bytes - path_length;
    response_data.user_context = (void *)session->user.context; /* TODO: fix unconst */
    response_data.message_status = connector_msg_error_none;
    response_data.client_data = NULL;

    {
        connector_request_id_t request_id;
        size_t response_data_length = sizeof response_data;
        connector_callback_status_t callback_status;

#if (defined CONNECTOR_DATA_POINTS)
        if (SmIsDatapoint(session->flags))
        {
            callback_status = dp_handle_callback(connector_ptr, &request_data, &response_data);
        }
        else
#endif
        {
            request_id.data_service_request = SmIsRequest(session->flags) ? connector_data_service_device_request : connector_data_service_put_request;
            callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_service, request_id,
                                    &request_data, sizeof request_data, &response_data, &response_data_length);
        }

        status = sm_map_callback_status_to_connector_status(callback_status);
        if (callback_status == connector_callback_continue)
        {
            if (response_data.message_status != connector_msg_error_none)
                session->error = connector_session_status_device_error;
            else
                session->user.context = response_data.user_context;
        }
    }

    return status;
}

static connector_status_t sm_process_reboot(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_abort;
    connector_request_id_t request_id;
    connector_callback_status_t callback_status;

    request_id.os_request = connector_os_reboot;
    callback_status = connector_callback_no_response(connector_ptr->callback, connector_class_id_operating_system, request_id, NULL, 0);
    result = sm_map_callback_status_to_connector_status(callback_status);

    return result;
}

static connector_status_t sm_process_empty_response(connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * const payload, size_t const bytes, connector_class_id_t const class_id, connector_request_id_t const request_id)
{
    connector_status_t result = connector_success;
    connector_message_status_response_t response_data;
    connector_callback_status_t callback_status;

    response_data.user_context = session->user.context;
    if (SmIsError(session->flags))
    {
        uint8_t * const response_payload = payload;
        enum
        {
            field_define(response_payload, error_id, uint16_t),
            record_end(response_payload)
        };
        uint16_t const error_id = message_load_be16(response_payload, error_id);
        connector_sm_error_id const error_code = (connector_sm_error_id)error_id;

        if (error_code != connector_sm_error_none)
            response_data.status = connector_session_status_cloud_error;

        {
            char const * error_string = payload;

            response_data.error_text = (bytes > record_end(response_payload)) ? error_string + record_end(response_payload) : NULL;
        }

        connector_debug_printf("Received SM error response from cloud class[%d], request[%d], error[%d], text[%s]\n", class_id, request_id, error_code, response_data.error_text);
    }
    else
    {
        response_data.status = connector_session_status_success;
        response_data.error_text = NULL;
    }

    callback_status = connector_callback_no_response(connector_ptr->callback, class_id, request_id, &response_data, sizeof response_data);
    result = sm_map_callback_status_to_connector_status(callback_status);

    return result;
}

static connector_status_t sm_process_status_response(connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * payload, size_t const bytes)
{
    connector_request_id_t request_id;

    request_id.status_request = connector_status_ping_response;
    return sm_process_empty_response(connector_ptr, session, payload, bytes, connector_class_id_status, request_id);
}

static connector_status_t sm_process_status_reqest(connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * payload, size_t const bytes)
{
    connector_status_t result;
    connector_request_id_t request_id;
    connector_callback_status_t callback_status;
    size_t response_length = sizeof session->error;

    UNUSED_PARAMETER(payload);
    UNUSED_PARAMETER(bytes);
    request_id.status_request = connector_status_ping_request;
    callback_status = connector_callback_no_request_data(connector_ptr->callback, connector_class_id_status, request_id, &session->error, &response_length);
    result = sm_map_callback_status_to_connector_status(callback_status);

    return result;
}

#if (defined CONNECTOR_TRANSPORT_SMS)
static connector_status_t sm_process_config_response(connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * payload, size_t const bytes)
{
    connector_request_id_t request_id;

    request_id.sm_request = connector_sm_device_to_server_config;
    return sm_process_empty_response(connector_ptr, session, payload, bytes, connector_class_id_short_message, request_id);
}
#endif

static connector_status_t sm_process_opaque_response(connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * payload, size_t const bytes)
{
    connector_status_t result = connector_success;
    connector_request_id_t request_id;
    connector_callback_status_t callback_status;
    connector_sm_opaque_response_t opaque_response;
    size_t response_length = sizeof opaque_response;

    opaque_response.id = session->request_id;
    opaque_response.status = connector_session_status_success;
    opaque_response.data = payload;
    opaque_response.bytes = bytes;
    opaque_response.flags = SmIsLastData(session->flags) ? CONNECTOR_MSG_LAST_DATA : 0;

    request_id.sm_request = connector_sm_opaque_response;
    callback_status = connector_callback_no_request_data(connector_ptr->callback, connector_class_id_short_message, request_id, &opaque_response, &response_length);
    result = sm_map_callback_status_to_connector_status(callback_status);
    if (callback_status == connector_callback_continue)
        session->error = opaque_response.status;

    return result;
}

static connector_status_t sm_prepare_payload(connector_data_t * const connector_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_abort;
    connector_status_t (* prepare_fn) (connector_data_t * const connector_ptr, connector_sm_session_t * const session) = NULL;

    ASSERT_GOTO(session != NULL, error);
    switch (session->command)
    {
        case connector_sm_cmd_data:
        case connector_sm_cmd_no_path_data:
            prepare_fn = SmIsClientOwned(session->flags) ? sm_prepare_data_request : sm_prepare_data_response;
            break;

        #if (defined CONNECTOR_SM_CLI)
        case connector_sm_cmd_cli:
            ASSERT_GOTO(SmIsCloudOwned(session->flags), error);
            prepare_fn = sm_prepare_cli_response;
            break;
        #endif

        #if (defined CONNECTOR_TRANSPORT_SMS)
        case connector_sm_cmd_config:
            ASSERT_GOTO(SmIsClientOwned(session->flags), error);
            prepare_fn = sm_prepare_config_request;
            break;
        #endif

        default:
            result = connector_unavailable;
            goto unexpected;
    }

    result = prepare_fn(connector_ptr, session);

unexpected:
    if ((result == connector_working) && (session->error != connector_session_status_success))
    {
        session->sm_state = connector_sm_state_error;
        SmSetError(session->flags);
    }

error:
    return result;
}

static connector_status_t sm_pass_user_data(connector_data_t * const connector_ptr, connector_sm_session_t * const session, uint8_t * payload, size_t const bytes)
{
    connector_status_t result = connector_abort;
    connector_status_t (* process_fn) (connector_data_t * const connector_ptr, connector_sm_session_t * const session, void * const payload, size_t const bytes) = NULL;
    connector_sm_state_t new_state = connector_sm_state_send_data;

    switch (session->command)
    {
        case connector_sm_cmd_data:
        case connector_sm_cmd_no_path_data:
            process_fn = sm_process_data_command;
            new_state = SmIsCloudOwned(session->flags) ? connector_sm_state_get_total_length : connector_sm_state_complete;
            break;

        #if (defined CONNECTOR_SM_CLI)
        case connector_sm_cmd_cli:
            ASSERT_GOTO(SmIsCloudOwned(session->flags), error);
            process_fn = sm_process_cli_request;
            new_state = connector_sm_state_get_total_length;
            break;
        #endif

        #if (defined CONNECTOR_TRANSPORT_SMS)
        case connector_sm_cmd_config:
            process_fn = SmIsRequest(session->flags) ? sm_process_config_request : sm_process_config_response;
            break;
        #endif

        case connector_sm_cmd_connect:
            #if (defined CONNECTOR_TRANSPORT_TCP)
            if (edp_get_active_state(connector_ptr) == connector_transport_idle)
                edp_set_active_state(connector_ptr, connector_transport_open);
            result = connector_working;
            #endif
            goto done;

        case connector_sm_cmd_ping:
            process_fn = SmIsCloudOwned(session->flags) ? sm_process_status_reqest : sm_process_status_response;
            if (SmIsClientOwned(session->flags))
                new_state = connector_sm_state_complete;
            break;

        case connector_sm_cmd_reboot:
            SmSetReboot(session->flags);
            result = connector_working;
            goto done;

        case connector_sm_cmd_opaque_response:
            process_fn = sm_process_opaque_response;
            break;

        default:
            result = connector_unavailable;
            goto error;
    }

    result = process_fn(connector_ptr, session, payload, bytes);

error:
done:
    if (result == connector_working)
    {
        session->bytes_processed += bytes;
        if (session->error != connector_session_status_success)
        {
            session->sm_state = connector_sm_state_error;
            SmSetError(session->flags);
        }
        else if (SmIsLastData(session->flags))
        {
            sm_switch_path(connector_ptr, session, new_state);
        }
    }

    return result;
}

static connector_status_t sm_process_payload(connector_data_t * const connector_ptr, connector_sm_data_t * const sm_ptr, connector_sm_session_t * const session)
{
    connector_status_t result = connector_abort;
    uint8_t * data_ptr;
    size_t bytes;

#if (defined CONNECTOR_SM_MULTIPART)
    if (SmIsMultiPart(session->flags))
    {
        size_t const max_payload_bytes = sm_get_max_payload_bytes(sm_ptr);
        size_t const data_index = session->segments.processed * max_payload_bytes;

        ASSERT_GOTO(session->in.data != NULL, error);
        ASSERT_GOTO(session->segments.processed < session->segments.count, error);
        if (session->segments.processed == (session->segments.count - 1))
            SmSetLastData(session->flags);

        data_ptr = &session->in.data[data_index];
        bytes = session->segments.size_array[session->segments.processed];
    }
    else
#endif
    {
        UNUSED_PARAMETER(sm_ptr);
        ASSERT_GOTO(SmIsNotMultiPart(session->flags), error);
        SmSetLastData(session->flags);
        data_ptr = session->in.data;
        bytes = session->in.bytes;
    }

    result = sm_pass_user_data(connector_ptr, session, data_ptr, bytes);
    if ((result == connector_working) && (SmIsNotLastData(session->flags)))
        session->segments.processed++;

error:
    return result;
}
