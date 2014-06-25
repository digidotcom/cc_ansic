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
    #define DP_FILE_PATH_SIZE   64
    #else
    #define DP_FILE_PATH_SIZE   32
    #endif
    connector_request_data_service_send_t header;
    char file_path[DP_FILE_PATH_SIZE];

    enum
    {
        dp_content_type_binary,
        dp_content_type_csv_single,
#if (CONNECTOR_VERSION >= 0x02020000)
        dp_content_type_csv_multiple
#endif
    } type;

    union
    {
        struct
        {
            connector_request_data_point_single_t const * dp_request_single;
#if (CONNECTOR_VERSION >= 0x02020000)
            connector_request_data_point_multiple_t const * dp_request_multiple;
            connector_data_stream_t const * current_ds;
#endif
            connector_data_point_t const * current_dp;
            size_t bytes_sent;
            size_t bytes_to_send;

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
                dp_state_forward_to,
#if (CONNECTOR_VERSION >= 0x02020000)
                dp_state_stream_id
#endif
            } state;

            connector_bool_t first_point;
        } csv;

        struct
        {
            connector_request_data_point_binary_t const * bp_request;
            uint8_t * current_bp;
            size_t bytes_to_send;
        } binary;

    } data;

} data_point_info_t;

static connector_request_data_point_single_t const * data_point_single_pending = NULL;
#if (CONNECTOR_VERSION >= 0x02020000)
static connector_request_data_point_multiple_t const * data_point_multiple_pending = NULL;
#endif
static connector_request_data_point_binary_t const * data_point_binary_pending = NULL;

STATIC connector_status_t dp_initiate_data_point_single(connector_request_data_point_single_t const * const dp_ptr)
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
        connector_debug_line("dp_initiate_data_point_single: NULL data point path");
        goto error;
    }

    if ((dp_ptr->point == NULL))
    {
        connector_debug_line("dp_initiate_data_point_single: NULL data point");
        goto error;
    }

    data_point_single_pending = dp_ptr;
    result = connector_success;

error:
    return result;
}

#if (CONNECTOR_VERSION >= 0x02020000)
STATIC connector_status_t dp_initiate_data_point_multiple(connector_request_data_point_multiple_t const * const dp_ptr)
{
    connector_status_t result = connector_invalid_data;

    ASSERT_GOTO(dp_ptr != NULL, error);

    if (data_point_multiple_pending != NULL)
    {
        result = connector_service_busy;
        goto error;
    }

    if ((dp_ptr->stream == NULL))
    {
        connector_debug_line("dp_initiate_data_point_multiple: NULL data stream");
        goto error;
    }
    else if ((dp_ptr->stream->point == NULL))
    {
        connector_debug_line("dp_initiate_data_point_multiple: NULL data point");
        goto error;
    }

    data_point_multiple_pending = dp_ptr;
    result = connector_success;

error:
    return result;
}
#endif

STATIC connector_status_t dp_initiate_data_point_binary(connector_request_data_point_binary_t const * const bp_ptr)
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
        connector_debug_line("dp_initiate_data_point_binary: NULL data point path");
        goto error;
    }

    if ((bp_ptr->point == NULL))
    {
        connector_debug_line("dp_initiate_data_point_binary: NULL data point");
        goto error;
    }

    data_point_binary_pending = bp_ptr;
    result = connector_success;

error:
    return result;
}

STATIC connector_status_t dp_callback_status_to_status(connector_callback_status_t const callback_status)
{
    connector_status_t status;

    switch (callback_status)
    {
        case connector_callback_continue:
            status = connector_working;
            break;

        case connector_callback_busy:
            status = connector_pending;
            break;

        default:
            status = connector_abort;
            break;
    }

    return status;
}

STATIC connector_status_t dp_inform_status(connector_data_t * const connector_ptr, connector_request_id_data_point_t request,
                                           connector_transport_t const transport, void * context, connector_session_error_t const error)
{
    connector_status_t result;
    connector_data_point_status_t dp_status;

    dp_status.transport = transport;
    dp_status.user_context = context;
    dp_status.session_error = connector_session_error_none;

    switch (error)
    {
        case connector_session_error_none:
            dp_status.status = connector_data_point_status_complete;
            break;

        case connector_session_error_cancel:
            dp_status.status = connector_data_point_status_cancel;
            break;

        case connector_session_error_timeout:
            dp_status.status = connector_data_point_status_timeout;
            break;

        case connector_session_error_format:
            dp_status.status = connector_data_point_status_invalid_data;
            break;

        default:
            dp_status.status = connector_data_point_status_session_error;
            dp_status.session_error = error;
            break;
    }

    {
        connector_callback_status_t callback_status;
        connector_request_id_t request_id;

        request_id.data_point_request = request;
        callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_point, request_id, &dp_status, connector_ptr->context);
        result = dp_callback_status_to_status(callback_status);
    }

    return result;
}

STATIC connector_status_t dp_cancel_session(connector_data_t * const connector_ptr, void const * const session, uint32_t const * const request_id)
{
    connector_status_t status = connector_working;
    connector_bool_t cancel_all = connector_bool(request_id == NULL);

    if (data_point_binary_pending != NULL)
    {
        connector_bool_t const has_request_id = connector_bool(data_point_binary_pending->request_id != NULL);
        connector_bool_t const matching_request = connector_bool(has_request_id && *data_point_binary_pending->request_id == *request_id);

        if (cancel_all || matching_request)
        {
            if (session == NULL)
            {
                status = dp_inform_status(connector_ptr, connector_request_id_data_point_binary_status, data_point_binary_pending->transport, data_point_binary_pending->user_context, connector_session_error_cancel);
                if (status != connector_working)
                  goto done;
            }
            data_point_binary_pending = NULL;
        }
    }

    if (data_point_single_pending != NULL)
    {
        connector_bool_t const pending_dp_has_request_id = connector_bool(data_point_single_pending->request_id != NULL);
        connector_bool_t const matching_request = connector_bool(pending_dp_has_request_id && *data_point_single_pending->request_id == *request_id);

        if (cancel_all || matching_request)
        {
            if (session == NULL)
            {
                status = dp_inform_status(connector_ptr, connector_request_id_data_point_single_status, data_point_single_pending->transport, data_point_single_pending->user_context, connector_session_error_cancel);
                if (status != connector_working)
                  goto done;
            }
            data_point_single_pending = NULL;
        }
    }
done:
    return status;
}

STATIC connector_status_t dp_fill_file_path(data_point_info_t * const dp_info, char const * const path, char const * const extension)
{
    connector_status_t result;
    size_t const available_path_bytes = sizeof dp_info->file_path - 1;
    char const path_prefix[] = "DataPoint/";
    size_t const path_prefix_bytes = sizeof path_prefix - 1;
    size_t const path_bytes = (path==NULL)?0:strlen(path);  /* Allow NULL path: User responsible of filling each point stream_id */
    size_t const extension_bytes = strlen(extension);
    size_t const full_path_bytes = path_prefix_bytes + path_bytes + extension_bytes;

    if (full_path_bytes < available_path_bytes)
    {
        strncpy(dp_info->file_path, path_prefix, path_prefix_bytes);
        if (path_bytes)
            strncpy(&dp_info->file_path[path_prefix_bytes], path, path_bytes);
        strncpy(&dp_info->file_path[path_prefix_bytes + path_bytes], extension, extension_bytes);
        dp_info->file_path[full_path_bytes] = '\0';
        result = connector_working;
    }
    else
    {
        connector_debug_line("dp_fill_file_path [DataPoint/%s.%s]: file path bytes [%" PRIsize "] exceeds the limit [%" PRIsize "]", path, extension, full_path_bytes, available_path_bytes);
        result = connector_invalid_data;
    }

    return result;
}

STATIC connector_status_t dp_send_message(connector_data_t * const connector_ptr, data_point_info_t * const dp_info,
                                          connector_transport_t const transport, connector_bool_t const response_needed, 
                                          uint32_t * request_id, unsigned long timeout_in_seconds)
{
    connector_status_t result;

    dp_info->header.transport = transport;
    dp_info->header.user_context = dp_info;
    dp_info->header.path = dp_info->file_path;
    dp_info->header.response_required = response_needed;
    dp_info->header.timeout_in_seconds = timeout_in_seconds;
    dp_info->header.content_type = NULL;
    dp_info->header.option = connector_data_service_send_option_overwrite;
    dp_info->header.request_id = request_id;

    result = connector_initiate_action(connector_ptr, connector_initiate_send_data, &dp_info->header);
    switch (result)
    {
        case connector_init_error:
        case connector_unavailable:
        case connector_service_busy:
            result = connector_pending;
            break;

        case connector_success:
            result = connector_working;
            goto done;

        default:
            connector_debug_line("dp_send_message: connector_initiate_action failed [%d]", result);
            break;
    }

done:
    return result;
}

STATIC void * dp_create_dp_info(connector_data_t * const connector_ptr, connector_status_t * result)
{
    void * ptr;

    *result = malloc_data_buffer(connector_ptr, sizeof(data_point_info_t), named_buffer_id(data_point_block), &ptr);
    if (*result != connector_working)
    {
        connector_debug_line("dp_create_dp_info: failed to malloc [%d]", *result);
        ptr = NULL;
    }

    return ptr;
}

STATIC connector_status_t dp_process_csv_single(connector_data_t * const connector_ptr, connector_request_data_point_single_t const * const dp_ptr)
{
    connector_status_t result = connector_idle;
    data_point_info_t * const dp_info = dp_create_dp_info(connector_ptr, &result);

    if (dp_info == NULL) goto done;

    dp_info->type = dp_content_type_csv_single;
    dp_info->data.csv.dp_request_single = dp_ptr;
    dp_info->data.csv.current_dp = dp_ptr->point;
    dp_info->data.csv.bytes_sent = 0;
    dp_info->data.csv.bytes_to_send = 0;
    dp_info->data.csv.state = dp_state_data;
    dp_info->data.csv.first_point = connector_true;

    result = dp_fill_file_path(dp_info, dp_ptr->path, ".csv");
    if (result != connector_working) goto error;
    result = dp_send_message(connector_ptr, dp_info, dp_ptr->transport, dp_ptr->response_required, dp_ptr->request_id, dp_ptr->timeout_in_seconds);
    if (result == connector_working) goto done;

error:
    if (result != connector_pending)
        result = dp_inform_status(connector_ptr, connector_request_id_data_point_single_status, dp_ptr->transport,
                                  dp_ptr->user_context, connector_session_error_format);

    if (free_data_buffer(connector_ptr, named_buffer_id(data_point_block), dp_info) != connector_working)
        result = connector_abort;

done:
    return result;
}

#if (CONNECTOR_VERSION >= 0x02020000)
STATIC connector_status_t dp_process_csv_multiple(connector_data_t * const connector_ptr, connector_request_data_point_multiple_t const * const dp_ptr)
{
    connector_status_t result = connector_idle;
    data_point_info_t * const dp_info = dp_create_dp_info(connector_ptr, &result);

    if (dp_info == NULL) goto done;

    dp_info->type = dp_content_type_csv_multiple;
    dp_info->data.csv.dp_request_multiple = dp_ptr;
    dp_info->data.csv.current_ds = dp_ptr->stream;
    dp_info->data.csv.current_dp = dp_ptr->stream->point;
    dp_info->data.csv.bytes_sent = 0;
    dp_info->data.csv.bytes_to_send = 0;
    dp_info->data.csv.state = dp_state_data;
    dp_info->data.csv.first_point = connector_true;

    result = dp_fill_file_path(dp_info, NULL, ".csv");
    if (result != connector_working) goto error;
    result = dp_send_message(connector_ptr, dp_info, dp_ptr->transport, dp_ptr->response_required, dp_ptr->request_id, dp_ptr->timeout_in_seconds);
    if (result == connector_working) goto done;

error:
    if (result != connector_pending)
        result = dp_inform_status(connector_ptr, connector_request_id_data_point_multiple_status, dp_ptr->transport,
                                  dp_ptr->user_context, connector_session_error_format);

    if (free_data_buffer(connector_ptr, named_buffer_id(data_point_block), dp_info) != connector_working)
        result = connector_abort;

done:
    return result;
}
#endif

STATIC connector_status_t dp_process_binary(connector_data_t * const connector_ptr, connector_request_data_point_binary_t const * const bp_ptr)
{
    connector_status_t result = connector_idle;
    data_point_info_t * const dp_info = dp_create_dp_info(connector_ptr, &result);

    if (dp_info == NULL) goto done;

    dp_info->type = dp_content_type_binary;
    dp_info->data.binary.bp_request = bp_ptr;
    dp_info->data.binary.current_bp = bp_ptr->point;
    dp_info->data.binary.bytes_to_send = bp_ptr->bytes_used;

    result = dp_fill_file_path(dp_info, bp_ptr->path, ".bin");
    if (result != connector_working) goto error;
    result = dp_send_message(connector_ptr, dp_info, bp_ptr->transport, bp_ptr->response_required, bp_ptr->request_id, bp_ptr->timeout_in_seconds);
    if (result == connector_working) goto done;

error:
    if (result != connector_pending)
        result = dp_inform_status(connector_ptr, connector_request_id_data_point_binary_status, bp_ptr->transport,
                                  bp_ptr->user_context, connector_session_error_format);

    if (free_data_buffer(connector_ptr, named_buffer_id(data_point_block), dp_info) != connector_working)
        result = connector_abort;

done:
    return result;
}

STATIC connector_status_t dp_process_request(connector_data_t * const connector_ptr, connector_transport_t const transport)
{
    connector_status_t result = connector_idle;
    static connector_bool_t process_csv = connector_true;

    if (process_csv)
    {
        if ((data_point_single_pending != NULL) && (data_point_single_pending->transport == transport))
        {
            result = dp_process_csv_single(connector_ptr, data_point_single_pending);
            if (result != connector_pending)
            {
                process_csv = connector_false;
                data_point_single_pending = NULL;
                goto done;
            }
        }
#if (CONNECTOR_VERSION >= 0x02020000)
        if ((data_point_multiple_pending != NULL) && (data_point_multiple_pending->transport == transport))
        {
            result = dp_process_csv_multiple(connector_ptr, data_point_multiple_pending);
            if (result != connector_pending)
            {
                process_csv = connector_false;
                data_point_multiple_pending = NULL;
                goto done;
            }
        }
#endif
    }
    else
        process_csv = connector_true;

    if ((data_point_binary_pending != NULL) && (data_point_binary_pending->transport == transport))
    {
        result = dp_process_binary(connector_ptr, data_point_binary_pending);
        if (result != connector_pending)
            data_point_binary_pending = NULL;
    }

done:
    return result;
}

STATIC connector_bool_t string_needs_quotes(char const * const string)
{
    connector_bool_t need_quotes = connector_false;
    size_t index;

    for (index = 0; !need_quotes && string[index] != '\0'; index++)
    {
        switch(string[index])
        {
            case ' ':
            case ',':
            case '\"':
            case '\t':
            case '\n':
            case '\r':
                need_quotes = connector_true;
                break;
            default:
                break;
        }
    }

    return need_quotes;
}

STATIC size_t dp_process_string(char * const string, char * const buffer, size_t const bytes_available, size_t * bytes_used_ptr, connector_bool_t need_quotes, connector_bool_t first_chunk)
{
    size_t bytes_processed = 0;
    size_t i;
    size_t extra_quotes = 0;
    size_t const max_strlen = bytes_available - 1;

    ASSERT(string != NULL);

    if (need_quotes && first_chunk && bytes_processed < max_strlen)
    {
        buffer[bytes_processed++] = '\"';
        extra_quotes++;
    }

    for (i = 0; string[i] != '\0' && bytes_processed < max_strlen; i++)
    {
        if (string[i] == '\"')
        {
            buffer[bytes_processed++] = '\"';
            extra_quotes++;
        }
        buffer[bytes_processed++] = string[i];
    }

    if (need_quotes && bytes_processed < max_strlen)
    {
        buffer[bytes_processed++] = '\"';
        extra_quotes++;
    }

    buffer[bytes_processed] = '\0';

    if (bytes_used_ptr != NULL)
    {
        *bytes_used_ptr = bytes_processed - extra_quotes;
    }

    return bytes_processed;
}

STATIC size_t dp_process_data(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;
    connector_data_point_type_t type;

    if (dp_ptr->data.type == connector_data_type_text)
    {
        bytes_processed = connector_snprintf(buffer, bytes_available, "%s", dp_ptr->data.element.text);
        goto done;
    }

    if (dp_info->type == dp_content_type_csv_single)
    {
        connector_request_data_point_single_t const * const dp_request_single = dp_info->data.csv.dp_request_single;

        ASSERT_GOTO(connector_data_point_type_binary >= dp_request_single->type, done);
        type = dp_request_single->type;
    }
#if (CONNECTOR_VERSION >= 0x02020000)
    else if (dp_info->type == dp_content_type_csv_multiple)
    {
        connector_data_stream_t const * ds_ptr = dp_info->data.csv.current_ds;

        ASSERT_GOTO(connector_data_point_type_binary >= ds_ptr->type, done);
        type = ds_ptr->type;
    }
#endif
    else ASSERT_GOTO(connector_false, done);

    switch (type)
    {
        case connector_data_point_type_integer:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%" PRId32, dp_ptr->data.element.native.int_value);
            break;

#if (defined CONNECTOR_HAS_64_BIT_INTEGERS)
        case connector_data_point_type_long:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%" PRId64, dp_ptr->data.element.native.long_value);
            break;
#endif


        case connector_data_point_type_string:
        {
            size_t bytes_copied = 0;
            connector_bool_t const need_quotes = string_needs_quotes(dp_ptr->data.element.native.string_value);
            connector_bool_t const first_chunk = connector_bool(dp_info->data.csv.bytes_sent == 0);
            char * const start_of_string = dp_ptr->data.element.native.string_value;
            unsigned int const bytes_sent = dp_info->data.csv.bytes_sent;
            char * const string_to_send = start_of_string + bytes_sent;

            if (first_chunk)
            {
                dp_info->data.csv.bytes_to_send = strlen(start_of_string);
            }

            bytes_processed = dp_process_string(string_to_send, buffer, bytes_available, &bytes_copied, need_quotes, first_chunk);

            dp_info->data.csv.bytes_to_send -= bytes_copied;
            dp_info->data.csv.bytes_sent = dp_info->data.csv.bytes_to_send > 0 ? dp_info->data.csv.bytes_sent + bytes_copied : 0;
            break;
        }

#if (defined FLOATING_POINT_SUPPORTED)
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

STATIC size_t dp_process_time(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;

    switch (dp_ptr->time.source)
    {
        case connector_time_cloud:
            break;

        case connector_time_local_epoch_fractional:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%u%03u",
                                                 dp_ptr->time.value.since_epoch_fractional.seconds,
                                                 dp_ptr->time.value.since_epoch_fractional.milliseconds);
            break;

        #if (defined CONNECTOR_HAS_64_BIT_INTEGERS)
        case connector_time_local_epoch_whole:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%lld", dp_ptr->time.value.since_epoch_whole.milliseconds);
            break;
        #endif

        case connector_time_local_iso8601:
            bytes_processed = connector_snprintf(buffer, bytes_available, "%s", dp_ptr->time.value.iso8601_string);
            break;
    }

    return bytes_processed;
}

STATIC size_t dp_process_quality(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;

    if (dp_ptr->quality.type != connector_quality_type_ignore)
        bytes_processed = connector_snprintf(buffer, bytes_available, "%d", dp_ptr->quality.value);

    return bytes_processed;
}

STATIC size_t dp_process_description(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;

    if (dp_ptr->description != 0)
    {
        connector_bool_t const needs_quotes = string_needs_quotes(dp_ptr->description);
        connector_bool_t const first_chunk = connector_true;

        bytes_processed = dp_process_string(dp_ptr->description, buffer, bytes_available, NULL, needs_quotes, first_chunk);
    }

    return bytes_processed;
}

STATIC size_t dp_process_location(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    connector_data_point_t const * dp_ptr = dp_info->data.csv.current_dp;
    size_t bytes_processed = 0;

    switch (dp_ptr->location.type)
    {
        case connector_location_type_ignore:
            break;

        case connector_location_type_text:
            bytes_processed = connector_snprintf(buffer, bytes_available, "\"%s,%s,%s\"",
                                                dp_ptr->location.value.text.latitude,
                                                dp_ptr->location.value.text.longitude,
                                                dp_ptr->location.value.text.elevation);
            break;

        #if (defined FLOATING_POINT_SUPPORTED)
        case connector_location_type_native:
            bytes_processed = connector_snprintf(buffer, bytes_available, "\"%f,%f,%f\"",
                                                dp_ptr->location.value.native.latitude,
                                                dp_ptr->location.value.native.longitude,
                                                dp_ptr->location.value.native.elevation);
            break;
        #endif
    }

    return bytes_processed;
}

STATIC size_t dp_process_type(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    char const * const type_list[] = {"INTEGER", "LONG", "FLOAT", "DOUBLE", "STRING", "BINARY"};
    size_t bytes_processed = 0;

    if (dp_info->data.csv.first_point == connector_false)
        return 0;

    if (dp_info->type == dp_content_type_csv_single)
    {
        connector_request_data_point_single_t const * request = dp_info->data.csv.dp_request_single;

        ASSERT_GOTO(connector_data_point_type_binary >= request->type, error);
        bytes_processed = connector_snprintf(buffer, bytes_available, "%s", type_list[request->type]);
    }
#if (CONNECTOR_VERSION >= 0x02020000)
    else if (dp_info->type == dp_content_type_csv_multiple)
    {
        connector_data_stream_t const * ds_ptr = dp_info->data.csv.current_ds;

        ASSERT_GOTO(connector_data_point_type_binary >= ds_ptr->type, error);
        bytes_processed = connector_snprintf(buffer, bytes_available, "%s", type_list[ds_ptr->type]);
    }
#endif
    else
    {
        ASSERT(connector_false);
    }

error:
    return bytes_processed;
}

STATIC size_t dp_process_unit(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    size_t bytes_processed = 0;

    if (dp_info->data.csv.first_point == connector_false)
        return 0;

    if (dp_info->type == dp_content_type_csv_single)
    {
        connector_request_data_point_single_t const * request = dp_info->data.csv.dp_request_single;
        if (request->unit != NULL)
        {
            connector_bool_t const needs_quotes = connector_false;
            connector_bool_t const first_chunk = connector_true;

            bytes_processed = dp_process_string(request->unit, buffer, bytes_available, NULL, needs_quotes, first_chunk);
        }
    }
#if (CONNECTOR_VERSION >= 0x02020000)
    else if (dp_info->type == dp_content_type_csv_multiple)
    {
        connector_data_stream_t const * ds_ptr = dp_info->data.csv.current_ds;
        if (ds_ptr->unit != NULL)
        {
            connector_bool_t const needs_quotes = connector_false;
            connector_bool_t const first_chunk = connector_true;

            bytes_processed = dp_process_string(ds_ptr->unit, buffer, bytes_available, NULL, needs_quotes, first_chunk);
        }
    }
#endif
    else
    {
        ASSERT(connector_false);
    }

    return bytes_processed;
}

STATIC size_t dp_process_forward_to(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    size_t bytes_processed = 0;

    if (dp_info->data.csv.first_point == connector_false)
        return 0;

    if (dp_info->type == dp_content_type_csv_single)
    {
        connector_request_data_point_single_t const * request = dp_info->data.csv.dp_request_single;
        if (request->forward_to != NULL)
        {
            connector_bool_t const needs_quotes = connector_false;
            connector_bool_t const first_chunk = connector_true;

            bytes_processed = dp_process_string(request->forward_to, buffer, bytes_available, NULL, needs_quotes, first_chunk);
        }
    }
#if (CONNECTOR_VERSION >= 0x02020000)
    else if (dp_info->type == dp_content_type_csv_multiple)
    {
        connector_data_stream_t const * ds_ptr = dp_info->data.csv.current_ds;
        if (ds_ptr->forward_to != NULL)
        {
            connector_bool_t const needs_quotes = connector_false;
            connector_bool_t const first_chunk = connector_true;

            bytes_processed = dp_process_string(ds_ptr->forward_to, buffer, bytes_available, NULL, needs_quotes, first_chunk);
        }
    }
#endif

    return bytes_processed;
}

#if (CONNECTOR_VERSION >= 0x02020000)
STATIC size_t dp_process_stream_id(data_point_info_t * const dp_info, char * const buffer, size_t const bytes_available)
{
    size_t bytes_processed = 0;
    connector_data_stream_t const * ds_ptr = dp_info->data.csv.current_ds;

    ASSERT(dp_info->type == dp_content_type_csv_multiple);

    if (ds_ptr->stream_id != NULL)
    {
        connector_bool_t const needs_quotes = connector_false;
        connector_bool_t const first_chunk = connector_true;

        bytes_processed = dp_process_string(ds_ptr->stream_id, buffer, bytes_available, NULL, needs_quotes, first_chunk);
    }

    return bytes_processed;
}
#endif

STATIC size_t dp_update_state(data_point_info_t * const dp_info, char * const buffer)
{
    if (((dp_info->type == dp_content_type_csv_single) && (dp_info->data.csv.state == dp_state_forward_to))
#if (CONNECTOR_VERSION >= 0x02020000)
        || ((dp_info->type == dp_content_type_csv_multiple) && (dp_info->data.csv.state == dp_state_stream_id))
#endif
        )
    {
        *buffer = '\n';
        dp_info->data.csv.current_dp = dp_info->data.csv.current_dp->next;
        dp_info->data.csv.first_point = connector_false;
#if (CONNECTOR_VERSION >= 0x02020000)
        if ((dp_info->type == dp_content_type_csv_multiple) && (dp_info->data.csv.current_dp == NULL))
        {
            dp_info->data.csv.current_ds = dp_info->data.csv.current_ds->next;
            if (dp_info->data.csv.current_ds != NULL)
            {
                dp_info->data.csv.current_dp = dp_info->data.csv.current_ds->point;
                dp_info->data.csv.first_point = connector_true;
            }
        }
#endif
        dp_info->data.csv.state = dp_state_data;
    }
    else
    {
        *buffer = ',';
        dp_info->data.csv.state++;
    }

    return 1;
}


STATIC size_t dp_fill_csv_payload(data_point_info_t * const dp_info, void * const payload, size_t const total_bytes, connector_transport_t transport)
{
    size_t bytes_copied = 0;
    char * data_ptr = payload;
    size_t bytes_remaining = total_bytes;
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

#if (CONNECTOR_VERSION >= 0x02020000)
            case dp_state_stream_id:
                process_fn = dp_process_stream_id;
                break;
#endif
        }

        bytes_copied = process_fn(dp_info, data_ptr, bytes_remaining);
        if (bytes_copied > 0)
        {
            if (bytes_copied >= bytes_remaining)
            {
                #if (defined CONNECTOR_SHORT_MESSAGE)
                /* For SM transports this is a problem because the buffer where the CSV is
                 * written is preallocated. If the CSV grows to fill the buffer, then it
                 * is too late.
                 */
                switch (transport)
                {
                    #if (defined CONNECTOR_TRANSPORT_UDP)
                    case connector_transport_udp:
                    #endif
                    #if (defined CONNECTOR_TRANSPORT_SMS)
                    case connector_transport_sms:
                    #endif
                        connector_debug_line("WARNING: Not enough space for processing the CSV DataPoint, increase the value of CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS");
                        ASSERT(connector_false);
                        break;
                    #if (defined CONNECTOR_TRANSPORT_TCP)
                    case connector_transport_tcp:
                    #endif
                    case connector_transport_all:
                        /* For connector_transport_tcp this is not a problem: once the packet is sent,
                         * this function will be called again to finish the CSV. */
                        break;
                }
                #else
                UNUSED_PARAMETER(transport);
                #endif
                break;
            }

            data_ptr += bytes_copied;
            bytes_remaining -= bytes_copied;
        }

        if (dp_info->data.csv.bytes_to_send == 0)
        {
            size_t const bytes_offset = dp_update_state(dp_info, data_ptr);

            bytes_remaining -= bytes_offset;
            data_ptr += bytes_offset;

            if (dp_info->data.csv.current_dp == NULL)
            {
                break;
            }
        }
        else
        {
            break;
        }

    } while (bytes_remaining > 0);

    bytes_copied = total_bytes - bytes_remaining;

    return bytes_copied;
}

STATIC connector_callback_status_t dp_handle_data_callback(connector_data_service_send_data_t * const data_ptr)
{
    connector_callback_status_t status = connector_callback_abort;
    data_point_info_t * const dp_info = data_ptr->user_context;

    ASSERT_GOTO(dp_info != NULL, error);
    switch (dp_info->type)
    {
        case dp_content_type_binary:
            if (dp_info->data.binary.bytes_to_send > data_ptr->bytes_available)
            {
                data_ptr->bytes_used = data_ptr->bytes_available;
                data_ptr->more_data = connector_true;
            }
            else
            {
                data_ptr->bytes_used = dp_info->data.binary.bytes_to_send;
                data_ptr->more_data = connector_false;
            }

            memcpy(data_ptr->buffer, dp_info->data.binary.current_bp, data_ptr->bytes_used);
            dp_info->data.binary.current_bp += data_ptr->bytes_used;
            dp_info->data.binary.bytes_to_send -= data_ptr->bytes_used;
            break;

        case dp_content_type_csv_single:
#if (CONNECTOR_VERSION >= 0x02020000)
        case dp_content_type_csv_multiple:
#endif
            data_ptr->bytes_used = dp_fill_csv_payload(dp_info, data_ptr->buffer, data_ptr->bytes_available, data_ptr->transport);
            data_ptr->more_data = (dp_info->data.csv.current_dp == NULL) ? connector_false : connector_true;
            break;
    }

    status = connector_callback_continue;

error:
    return status;
}

STATIC connector_callback_status_t dp_handle_response_callback(connector_data_t * const connector_ptr, connector_data_service_send_response_t * const data_ptr)
{
    connector_callback_status_t callback_status = connector_callback_abort;
    data_point_info_t * const dp_info = data_ptr->user_context;
    connector_request_id_t request_id;
    connector_data_point_response_t user_data;

    ASSERT_GOTO(dp_info != NULL, error);
    switch (dp_info->type)
    {
        case dp_content_type_binary:
            user_data.user_context = dp_info->data.binary.bp_request->user_context;
            request_id.data_point_request = connector_request_id_data_point_binary_response;
            break;

        case dp_content_type_csv_single:
            user_data.user_context = dp_info->data.csv.dp_request_single->user_context;
            request_id.data_point_request = connector_request_id_data_point_single_response;
            break;

#if (CONNECTOR_VERSION >= 0x02020000)
        case dp_content_type_csv_multiple:
            user_data.user_context = dp_info->data.csv.dp_request_multiple->user_context;
            request_id.data_point_request = connector_request_id_data_point_multiple_response;
            break;
#endif
    }

    user_data.transport = data_ptr->transport;
    user_data.hint = data_ptr->hint;
    switch (data_ptr->response)
    {
        case connector_data_service_send_response_success:
            user_data.response = connector_data_point_response_success;
            break;

        case connector_data_service_send_response_bad_request:
            user_data.response = connector_data_point_response_bad_request;
            break;

        case connector_data_service_send_response_unavailable:
            user_data.response = connector_data_point_response_unavailable;
            break;

        case connector_data_service_send_response_cloud_error:
            user_data.response = connector_data_point_response_cloud_error;
            break;
        default:
            ASSERT(connector_false);
    }

    callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_point, request_id, &user_data, connector_ptr->context);
    if (callback_status == connector_callback_busy) goto error;

error:
    return callback_status;
}

STATIC connector_callback_status_t dp_handle_status_callback(connector_data_t * const connector_ptr, connector_data_service_status_t * const data_ptr)
{
    connector_callback_status_t callback_status = connector_callback_abort;
    data_point_info_t * const dp_info = data_ptr->user_context;
    connector_request_id_t request_id;
    connector_data_point_status_t user_data;

    ASSERT_GOTO(dp_info != NULL, error);
    switch (dp_info->type)
    {
        case dp_content_type_binary:
            user_data.user_context = dp_info->data.binary.bp_request->user_context;
            request_id.data_point_request = connector_request_id_data_point_binary_status;
            break;

        case dp_content_type_csv_single:
            user_data.user_context = dp_info->data.csv.dp_request_single->user_context;
            request_id.data_point_request = connector_request_id_data_point_single_status;
            break;

#if (CONNECTOR_VERSION >= 0x02020000)
        case dp_content_type_csv_multiple:
            user_data.user_context = dp_info->data.csv.dp_request_multiple->user_context;
            request_id.data_point_request = connector_request_id_data_point_multiple_status;
            break;
#endif
    }

    user_data.transport = data_ptr->transport;
    user_data.session_error = data_ptr->session_error;
    switch (data_ptr->status)
    {
        case connector_data_service_status_complete:
            user_data.status = connector_data_point_status_complete;
            break;

        case connector_data_service_status_cancel:
            user_data.status = connector_data_point_status_cancel;
            break;

        case connector_data_service_status_timeout:
            user_data.status = connector_data_point_status_timeout;
            break;

        case connector_data_service_status_session_error:
            user_data.status = connector_data_point_status_session_error;
            break;

        default:
            user_data.status = connector_data_point_status_session_error;
            break;
    }

    callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_point, request_id, &user_data, connector_ptr->context);
    if (callback_status == connector_callback_busy) goto error;

    if (free_data_buffer(connector_ptr, named_buffer_id(data_point_block), dp_info) != connector_working)
        callback_status = connector_callback_abort;

error:
    return callback_status;
}

#if (defined CONNECTOR_SHORT_MESSAGE)
STATIC connector_callback_status_t dp_handle_length_callback(connector_data_t * const connector_ptr, connector_data_service_length_t * const data_ptr)
{
    connector_callback_status_t status = connector_callback_abort;
    data_point_info_t * const dp_info = data_ptr->user_context;

    ASSERT_GOTO(dp_info != NULL, error);
    switch (dp_info->type)
    {
        case dp_content_type_binary:
            data_ptr->total_bytes = dp_info->data.binary.bytes_to_send;
            break;

        case dp_content_type_csv_single:
#if (CONNECTOR_VERSION >= 0x02020000)
        case dp_content_type_csv_multiple:
#endif
        {
            connector_sm_data_t * sm_ptr = NULL;
            size_t max_payload_bytes;

            switch (data_ptr->transport)
            {
                #if (defined CONNECTOR_TRANSPORT_UDP)
                case connector_transport_udp:
                    sm_ptr = &connector_ptr->sm_udp;
                    break;
                #endif

                #if (defined CONNECTOR_TRANSPORT_SMS)
                case connector_transport_sms:
                    sm_ptr = &connector_ptr->sm_sms;
                    break;
                #endif

                default:
                    ASSERT(connector_false);
                    break;
            }

            max_payload_bytes = sm_ptr->transport.sm_mtu_tx - record_end(segment);

            #if !(defined CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS)
            #define CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS   1
            #endif

            if (CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS == 1)
            {
                data_ptr->total_bytes = max_payload_bytes;
            }
            else
            {
                size_t const segment0_overhead_bytes = record_end(segment0) - record_end(segmentn);
                data_ptr->total_bytes = CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS * max_payload_bytes - segment0_overhead_bytes;
            }

            break;
        }
    }

    status = connector_callback_continue;

error:
    return status;
}
#endif

STATIC connector_callback_status_t dp_handle_callback(connector_data_t * const connector_ptr, connector_request_id_data_service_t const ds_request_id, void * const data)
{
    connector_callback_status_t status;

    switch (ds_request_id)
    {
        case connector_request_id_data_service_send_data:
            status = dp_handle_data_callback(data);
            break;

        case connector_request_id_data_service_send_response:
            status = dp_handle_response_callback(connector_ptr, data);
            break;

        case connector_request_id_data_service_send_status:
            status = dp_handle_status_callback(connector_ptr, data);
            break;

        #if (defined CONNECTOR_SHORT_MESSAGE)
        case connector_request_id_data_service_send_length:
            status = dp_handle_length_callback(connector_ptr, data);
            break;
        #endif

        default:
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
