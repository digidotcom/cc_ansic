/*
 * Copyright (c) 2013 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

typedef struct
{
    #if (defined CONNECTOR_TRANSPORT_TCP)
    #define DP_FILE_PATH_SIZE   256
    #else
    #define DP_FILE_PATH_SIZE   32
    #endif
    connector_request_data_service_send_t header;
    char file_path[DP_FILE_PATH_SIZE];

    enum
    {
        dp_content_type_binary,
        dp_content_type_csv
    } type;

    union
    {
        struct
        {
            connector_request_data_point_single_t const * dp_request;
            connector_data_point_t  const * current_dp;
            size_t bytes_to_send;
            char * last_entry_ptr;

            /*************************************************************************
            ** WARNING: Please don't change the order of the state unless default  **
            **          CSV format described in the Cloud documentation changes.   **
            *************************************************************************/
            enum
            {
                dp_state_data,
                dp_state_time,
                dp_state_quality,
                dp_state_description,
                dp_state_location,
                dp_state_type,
                dp_state_unit,
                dp_state_forward_to
            } state;

            connector_bool_t first_point;
        } csv;

        struct
        {
            connector_request_data_point_binary_t const * bp_request;
            uint8_t const * current_bp;
            size_t bytes_to_send;
        } binary;

    } data;

} data_point_info_t;

static connector_request_data_point_single_t const * data_point_single_pending = NULL;
static connector_request_data_point_binary_t const * data_point_binary_pending = NULL;

static connector_status_t dp_initiate_data_point_single(connector_request_data_point_single_t const * const dp_ptr)
{
    connector_status_t result = connector_invalid_data;

    ASSERT_GOTO(dp_ptr != NULL, error);

    if (data_point_single_pending != NULL)
    {
        result = connector_service_busy;
        goto error;
    }

    if (dp_ptr->path == NULL)
    {
        connector_debug_printf("dp_initiate_data_point_single: NULL data point path\n");
        goto error;
    }

    if ((dp_ptr->point == NULL))
    {
        connector_debug_printf("dp_initiate_data_point_single: NULL data point\n");
        goto error;
    }

    data_point_single_pending = dp_ptr;
    result = connector_success;

error:
    return result;
}

static connector_status_t dp_initiate_data_point_binary(connector_binary_point_request_t const * const bp_ptr)
{
    connector_status_t result = connector_invalid_data;

    ASSERT_GOTO(bp_ptr != NULL, error);

    if (data_point_binary_pending != NULL)
    {
        result = connector_service_busy;
        goto error;
    }

    if (bp_ptr->path == NULL)
    {
        connector_debug_printf("dp_initiate_data_point_binary: NULL data point path\n");
        goto error;
    }

    if ((bp_ptr->point == NULL))
    {
        connector_debug_printf("dp_initiate_data_point_binary: NULL data point\n");
        goto error;
    }

    data_point_binary_pending = bp_ptr;
    result = connector_success;

error:
    return result;
}

static connector_callback_status_t dp_return_response(connector_data_t * const connector_ptr, connector_request_id_data_point_t const request, void * const cb_data)
{
    connector_callback_status_t callback_status;
    connector_request_id_t request_id;

    request_id.data_service_request = connector_data_service_dp_response;
    callback_status = connector_callback_no_response(connector_ptr->callback, connector_class_id_data_service, request_id, &response_data, sizeof response_data);

    if (dp_info != NULL)
    {
        if (free_data_buffer(connector_ptr, named_buffer_id(data_point_block), dp_info) != connector_working)
        {
            connector_debug_printf("dp_return_response: Failed to free!\n");
            callback_status = connector_callback_abort;
        }
    }

    return callback_status;
}

static connector_status_t dp_send_message(connector_data_t * const connector_ptr, data_point_info_t * const dp_info, connector_data_point_header_t const * const dp_header, char * const extension)
{
    connector_status_t result;

    {
        size_t const available_path_bytes = sizeof dp_info->file_path;
        size_t const path_bytes = connector_snprintf(dp_info->file_path, available_path_bytes, "DataPoint/%s.%s", dp_header->path, extension);

        if (path_bytes >= available_path_bytes)
        {
            connector_debug_printf("dp_send_message [%s]: file path bytes [%zu] exceeds the limit [%zu]\n", dp_info->file_path, path_bytes, available_path_bytes);
            result = connector_invalid_data;
            goto error;
        }
    }

    dp_info->header.path = dp_info->file_path;
    dp_info->header.flags = dp_header->flags | CONNECTOR_DATA_POINT_REQUEST;
    dp_info->header.transport = dp_header->transport;
    dp_info->header.content_type = NULL;
    dp_info->header.context = dp_info;

    result = connector_initiate_action(connector_ptr, connector_initiate_send_data, &dp_info->header);
    switch (result)
    {
        case connector_init_error:
        case connector_unavailable:
            result = connector_service_busy;
            /* no break */
        case connector_service_busy:
            break;

        case connector_success:
            goto done;

        default:
            connector_debug_printf("dp_send_message: connector_initiate_action failed [%d]!\n", result);
            break;
    }

error:
    if (result != connector_service_busy)
    {
        connector_callback_status_t const status = dp_return_response(connector_ptr, dp_info, connector_session_status_internal_error, NULL);

        if (status == connector_callback_abort)
            result = connector_abort;
    }
    else
    {
        /* retry later */
        if (free_data_buffer(connector_ptr, named_buffer_id(data_point_block), dp_info) != connector_working)
            result = connector_abort;
    }

done:
    return result;
}

static void * dp_create_dp_info(connector_data_t * const connector_ptr, connector_status_t * result)
{
    void * ptr;

    *result = malloc_data_buffer(connector_ptr, sizeof(data_point_info_t), named_buffer_id(data_point_block), &ptr);
    if (*result != connector_working)
    {
        connector_debug_printf("dp_create_dp_info: failed to malloc [%d]!\n", *result);
        ptr = NULL;
    }

    return ptr;
}

static connector_status_t dp_process_csv(connector_data_t * const connector_ptr, connector_data_point_request_t const * const dp_ptr)
{
    connector_status_t result = connector_idle;
    data_point_info_t * const dp_info = dp_create_dp_info(connector_ptr, &result);

    if (dp_info == NULL) goto done;

    dp_info->type = dp_content_type_csv;
    dp_info->data.csv.dp_request = dp_ptr;
    dp_info->data.csv.current_dp = dp_ptr->point;
    dp_info->data.csv.bytes_to_send = 0;
    dp_info->data.csv.state = dp_state_data;
    dp_info->data.csv.first_point = connector_true;
    dp_info->data.csv.last_entry_ptr = NULL;

    result = dp_send_message(connector_ptr, dp_info, &dp_ptr->header, "csv");

done:
    return result;
}

static connector_status_t dp_process_binary(connector_data_t * const connector_ptr, connector_binary_point_request_t const * const bp_ptr)
{
    connector_status_t result = connector_idle;
    data_point_info_t * const dp_info = dp_create_dp_info(connector_ptr, &result);

    if (dp_info == NULL) goto done;

    dp_info->type = dp_content_type_binary;
    dp_info->data.binary.bp_request = bp_ptr;
    dp_info->data.binary.current_bp = bp_ptr->point;
    dp_info->data.binary.bytes_to_send = 0;

    result = dp_send_message(connector_ptr, dp_info, &bp_ptr->header, "bin");

done:
    return result;
}

static connector_status_t dp_process_request(connector_data_t * const connector_ptr, connector_transport_t const transport)
{
    connector_status_t result = connector_idle;
    static connector_bool_t process_csv = connector_true;

    if (process_csv)
    {
        if ((data_point_pending != NULL) && (data_point_pending->header.transport == transport))
        {
            result = dp_process_csv(connector_ptr, data_point_pending);
            if (result != connector_service_busy)
            {
                process_csv = connector_false;
                data_point_pending = NULL;
                goto done;
            }
        }
    }
    else
        process_csv = connector_true;

    if ((binary_point_pending != NULL) && (binary_point_pending->header.transport == transport))
    {
        result = dp_process_binary(connector_ptr, binary_point_pending);
        if (result != connector_service_busy)
            binary_point_pending = NULL;
    }

done:
    return result;
}

static size_t dp_process_data(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    connector_data_point_request_t const * const dp_request = dp_info->data.csv.dp_request;
    size_t bytes_processed = 0;

    if (dp_ptr->data.type == connector_data_type_text)
    {
        bytes_processed = connector_snprintf(buffer, bytes_available, "%s", dp_ptr->data.element.text);
        goto done;
    }

    switch (dp_request->type)
    {
        case connector_data_point_type_integer:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%d", dp_ptr->data.element.native.int_value);
            break;

        case connector_data_point_type_long:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%ld", dp_ptr->data.element.native.long_value);
            break;

        case connector_data_point_type_string:
        {
            size_t const bytes_sent = strlen(dp_ptr->data.element.native.string_value) - dp_info->data.csv.bytes_to_send;

            bytes_processed = connector_snprintf(buffer, bytes_available, "%s", &dp_ptr->data.element.native.string_value[bytes_sent]);
            if (bytes_processed >= bytes_available)
                bytes_processed = bytes_available - 1; /* exclude null-terminate, allowing partial data transfer only in this case */

            dp_info->data.csv.bytes_to_send -= bytes_processed;
            break;
        }

#if (defined FLOATING_POINT_SUPPORT)
        case connector_data_point_type_float:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%f", dp_ptr->data.element.native.float_value);
            break;

        case connector_data_point_type_double:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%lf", dp_ptr->data.element.native.double_value);
            break;
#endif

        default:
            ASSERT_GOTO(connector_false, done);
            break;
    }

done:
    return bytes_processed;
}

static size_t dp_process_time(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;

    switch (dp_ptr->time.source)
    {
        case connector_time_server:
            break;

        case connector_time_local_epoch:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%ld%03u",
                                             dp_ptr->time.value.msec_since_epoch.seconds,
                                             dp_ptr->time.value.msec_since_epoch.milliseconds);
            break;

        case connector_time_local_iso8601:
            bytes_processed = connector_snprintf(buffer, bytes_available, "\"%s\"", dp_ptr->time.value.iso8601_string);
            break;
    }

    return bytes_processed;
}

static size_t dp_process_quality(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;

    if (dp_ptr->quality.type != connector_quality_type_ignore)
        bytes_processed = connector_snprintf(buffer, bytes_available, "%d", dp_ptr->quality.value);

    return bytes_processed;
}

static size_t dp_process_description(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;

    if (dp_ptr->description != 0)
        bytes_processed = connector_snprintf(buffer, bytes_available, "\"%s\"", dp_ptr->description);

    return bytes_processed;
}

static size_t dp_process_location(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;

    switch (dp_ptr->location.type)
    {
        case connector_location_type_ignore:
            break;

        case connector_location_type_text:
            bytes_processed = connector_snprintf(buffer, bytes_available, "\"%s, %s, %s\"",
                                                dp_ptr->location.value.text.latitude,
                                                dp_ptr->location.value.text.longitude,
                                                dp_ptr->location.value.text.elevation);
            break;

        #if (defined FLOATING_POINT_SUPPORTED)
        case connector_location_type_native:
            bytes_processed = connector_snprintf(buffer, bytes_available, "\"%f, %f, %f\"",
                                                dp_ptr->location.value.native.latitude,
                                                dp_ptr->location.value.native.longitude,
                                                dp_ptr->location.value.native.elevation);
            break;
        #endif
    }

    return bytes_processed;
}

static size_t dp_process_type(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    char const * const type_list[] = {"Integer", "Long", "Float", "Double", "String", "Binary"};
    connector_data_point_request_t const * request = dp_info->data.csv.dp_request;
    size_t bytes_processed = 0;

    ASSERT_GOTO(asizeof(type_list) > request->type, error);
    bytes_processed = connector_snprintf(buffer, bytes_available, "%s", type_list[request->type]);

error:
    return bytes_processed;
}

static size_t dp_process_unit(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_request_t const * request = dp_info->data.csv.dp_request;
    size_t bytes_processed = 0;

    if (request->unit != NULL)
        bytes_processed = connector_snprintf(buffer, bytes_available, "%s", request->unit);

    return bytes_processed;
}

static size_t dp_process_forward_to(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_request_t const * request = dp_info->data.csv.dp_request;
    size_t bytes_processed = 0;

    if (request->forward_to != NULL)
        bytes_processed = connector_snprintf(buffer, bytes_available, "\"%s\"", request->forward_to);

    return bytes_processed;
}

static int dp_update_state(data_point_info_t * const dp_info, char * const buffer)
{
    char next_char = ',';
    int bytes_offset = sizeof next_char;

    if (dp_info->data.csv.state < dp_state_location) goto next_state;

    if (dp_info->data.csv.first_point)
    {
        if (dp_info->data.csv.state < dp_state_forward_to) goto next_state;

        dp_info->data.csv.first_point = connector_false;
    }

    dp_info->data.csv.current_dp = dp_info->data.csv.current_dp->next;

    next_char = (dp_info->data.csv.current_dp != NULL) ? '\n' : '\0';
    *dp_info->data.csv.last_entry_ptr = next_char;
    bytes_offset -= (buffer - dp_info->data.csv.last_entry_ptr); /* cleanup empty data */
    dp_info->data.csv.state = dp_state_data;
    goto done;

next_state:
    dp_info->data.csv.state++;
    *buffer = next_char;

done:
    return bytes_offset;
}

static size_t dp_fill_csv_payload(data_point_info_t * const dp_info, void * const payload, size_t const total_bytes)
{
    size_t bytes_copied = 0;
    char * data_ptr = payload;
    size_t bytes_remaining = total_bytes - 1; /* for null-termination byte */
    size_t (* process_fn) (data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available) = NULL;

    do
    {
        switch (dp_info->data.csv.state)
        {
            case dp_state_data:
                process_fn = dp_process_data;
                break;

            case dp_state_time:
                process_fn = dp_process_time;
                break;

            case dp_state_quality:
                process_fn = dp_process_quality;
                break;

            case dp_state_description:
                process_fn = dp_process_description;
                break;

            case dp_state_location:
                process_fn = dp_process_location;
                break;

            case dp_state_type:
                process_fn = dp_process_type;
                break;

            case dp_state_unit:
                process_fn = dp_process_unit;
                break;

            case dp_state_forward_to:
                process_fn = dp_process_forward_to;
                break;
        }

        bytes_copied = process_fn(dp_info, data_ptr, bytes_remaining);
        if (bytes_copied > 0)
        {
            /* not enough space to send current entry, use the next packet to send the remaining data */
            if (bytes_copied >= bytes_remaining)
            {
                bytes_remaining = 0;
                break;
            }

            data_ptr += bytes_copied;
            bytes_remaining -= bytes_copied;
            dp_info->data.csv.last_entry_ptr = data_ptr;
        }

        if (dp_info->data.csv.bytes_to_send == 0)
        {
            int const bytes_offset = dp_update_state(dp_info, data_ptr);

            bytes_remaining -= bytes_offset;
            data_ptr += bytes_offset;

            if (dp_info->data.csv.current_dp == NULL) break;
        }
        else
            break;

    } while (bytes_remaining > 0);

    bytes_copied = total_bytes - bytes_remaining;

    return bytes_copied;
}

static connector_session_status_t dp_map_msg_error_to_status(connector_msg_error_t const msg_error)
{
    connector_session_status_t status;

    switch (msg_error)
    {
        case connector_msg_error_cancel:
            status = connector_session_status_cancel;
            break;

        case connector_msg_error_decompression_failure:
        case connector_msg_error_ack:
        case connector_msg_error_invalid_opcode:
        case connector_msg_error_busy:
            status = connector_session_status_cloud_error;
            break;

        case connector_msg_error_memory:
            status = connector_session_status_no_resource;
            break;

        case connector_msg_error_timeout:
            status = connector_session_status_timeout;
            break;

        case connector_msg_error_send:
            status = connector_session_status_device_error;
            break;

        default:
            status = connector_session_status_internal_error;
            break;
    }

    return status;
}

static connector_callback_status_t dp_handle_callback(connector_data_t * const connector_ptr, void const * const request_data, void * response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_data_service_msg_request_t const * const put_request = request_data;
    connector_data_service_msg_response_t * const put_response = response_data;

    ASSERT_GOTO(put_request != NULL, error);
    ASSERT_GOTO(put_response != NULL, error);
    ASSERT_GOTO(put_request->service_context != NULL, error);

    {
        connector_data_service_put_request_t const * const header = put_request->service_context;
        data_point_info_t * const dp_info = (data_point_info_t *)header->context;

        ASSERT(dp_info != NULL);
        switch (put_request->message_type)
        {
            case connector_data_service_type_need_data:
            {
                connector_data_service_block_t * const message = put_response->client_data;

                switch (dp_info->type)
                {
                    case dp_content_type_binary:
                        if (dp_info->data.binary.bytes_to_send < message->length_in_bytes)
                        {
                            message->length_in_bytes = dp_info->data.binary.bytes_to_send;
                            message->flags = CONNECTOR_MSG_LAST_DATA;
                        }

                        memcpy(message->data, dp_info->data.binary.current_bp, message->length_in_bytes);
                        dp_info->data.binary.current_bp += message->length_in_bytes;
                        dp_info->data.binary.bytes_to_send -= message->length_in_bytes;
                        break;

                    case dp_content_type_csv:
                        message->length_in_bytes = dp_fill_csv_payload(dp_info, message->data, message->length_in_bytes);
                        if (dp_info->data.csv.current_dp == NULL)
                            message->flags = CONNECTOR_MSG_LAST_DATA;
                        break;
                }

                put_response->message_status = connector_msg_error_none;
                break;
            }

            case connector_data_service_type_have_data:
            {
                connector_data_service_block_t * const message = put_request->server_data;
                connector_session_status_t const resp_status = ((message->flags & CONNECTOR_MSG_RESP_SUCCESS) == CONNECTOR_MSG_RESP_SUCCESS) ? connector_session_status_success : connector_session_status_cloud_error;
                char * const data = (message->length_in_bytes > 0) ? message->data : NULL;

                if (data != NULL)
                    data[message->length_in_bytes] = '\0';

                status = dp_return_response(connector_ptr, dp_info, resp_status, data);
                break;
            }

            case connector_data_service_type_error:
            {
                connector_data_service_block_t * const message = put_request->server_data;
                connector_msg_error_t const * const error_value = message->data;
                connector_session_status_t const resp_status = dp_map_msg_error_to_status(*error_value);

                status = dp_return_response(connector_ptr, dp_info, resp_status, NULL);
                break;
            }

            case connector_data_service_type_session_status:
            {
                connector_session_status_t * const status_ptr = response_data;

                status = dp_return_response(connector_ptr, dp_info, *status_ptr, NULL);
                break;
            }

            #if (defined CONNECTOR_TRANSPORT_UDP) || (defined CONNECTOR_TRANSPORT_SMS)
            case connector_data_service_type_total_length:
            {
                size_t * const length = response_data;

                if (dp_info->type == dp_content_type_binary)
                {
                    *length =  dp_info->data.binary.bytes_to_send;
                }
                else
                {
                    size_t const sm_header_bytes = 6;
                    size_t const transport_layer_bytes = (dp_info->header.transport == connector_transport_udp) ? 18 : 10;
                    size_t const max_packet_size = (dp_info->header.transport == connector_transport_udp) ? SM_PACKET_SIZE_UDP : SM_PACKET_SIZE_SMS;
                    size_t const max_payload_bytes = max_packet_size - (sm_header_bytes + transport_layer_bytes);
                    size_t max_segments = 1;

                    #if (defined CONNECTOR_SM_MAX_SEGMENTS)
                    max_segments = CONNECTOR_SM_MAX_SEGMENTS;
                    #endif

                    *length = max_payload_bytes * max_segments;
                }

                break;
            }
            #endif

            default:
                status = connector_callback_unrecognized;
                ASSERT_GOTO(connector_false, error);
                break;
        }
    }

error:
    return status;
}
