/*
 * Copyright (c) 2011 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

typedef enum
{
    data_service_opcode_put_request,
    data_service_opcode_put_response,
    data_service_opcode_device_request,
    data_service_opcode_device_response
} data_service_opcode_t;

typedef struct
{
    void * callback_context;
    connector_bool_t dp_request;

    enum
    {
        data_service_put_request,
        data_service_device_request
    } request_type;

} data_service_context_t;

static void set_data_service_error(msg_service_request_t * const service_request, connector_session_error_t const error_code)
{
    service_request->error_value = error_code;
    service_request->service_type = msg_service_type_error;
}

static connector_status_t process_device_request(connector_data_t * const connector_ptr,
                                                      msg_service_request_t * const service_request)
{
    connector_status_t status = connector_working;
    msg_session_t * const session = service_request->session;
    data_service_context_t * device_request_service = session->service_context;
    msg_service_data_t * const service_data = service_request->have_data;
    uint8_t const * ds_device_request = service_data->data_ptr;
    connector_session_error_t error_status = connector_session_error_cancel;
    connector_bool_t const isFirstRequest = connector_bool(MsgIsStart(service_data->flags));
    char * target_string = NULL;
    connector_callback_status_t callback_status;

    if (isFirstRequest)
    {
        /* 1st message so let's parse message-start packet:
         *
         * Data Service Device request format:
         *  -------------------------------------------------------------------------------------------------------
         * |   0    |   1    |  2+N   |    +1     |     +1      |    +1       |    +M       |  ...       | +P      |
         *  -------------------------------------------------------------------------------------------------------
         * | Opcode | Target | Target | Parameter | Parameter 1 | Parameter 1 | Parameter 1 | Additioanl | Payload |
         * |        | length | string |   count   |     ID      | data length |    data     | parameters |         |
         *  -------------------------------------------------------------------------------------------------------
         *
         */
        enum {
            field_define(ds_device_request, opcode, uint8_t),
            field_define(ds_device_request, target_length, uint8_t),
            record_end(ds_device_request_header)
        };

        enum {
            field_define(ds_device_request, parameter_count, uint8_t)
        };

        enum {
            field_define(ds_device_request, parameter_id, uint8_t),
            field_define(ds_device_request, parameter_length, uint8_t),
            record_end(ds_device_request_parameter)
        };

        uint8_t const target_length =  message_load_u8(ds_device_request, target_length);

        size_t const min_data_length = (size_t)(target_length +
                          record_bytes(ds_device_request_header) +
                          field_named_data(ds_device_request, parameter_count, size));

        ASSERT_GOTO((message_load_u8(ds_device_request, opcode) == data_service_opcode_device_request), done);
        ASSERT_GOTO(service_data->length_in_bytes >= min_data_length, done);

        ds_device_request += record_bytes(ds_device_request_header);

        if (device_request_service == NULL)
        {
            /* 1st time here so let's allocate service context memory for device request service */
            void * ptr;

            status = malloc_data_buffer(connector_ptr, sizeof *device_request_service, named_buffer_id(msg_service), &ptr);
            if (status != connector_working)
            {
                if (status == connector_pending)
                {
                    error_status = connector_session_error_none;
                }
                goto done;
            }

            device_request_service = ptr;
            session->service_context = device_request_service;
            device_request_service->callback_context = NULL;
        }

        target_string = (char *)ds_device_request;

        ds_device_request += target_length;

        {
            /* TODO: Parse and process each parameter in the future.
             *      Ignore all parameters now.
             */

            uint8_t const parameter_count = message_load_u8(ds_device_request, parameter_count);
            uint8_t i;

            ds_device_request += field_named_data(ds_device_request, parameter_count, size);

            for (i=0; i < parameter_count; i++)
            {
                 unsigned int const parameter_length = message_load_u8(ds_device_request, parameter_length);
                 size_t const data_length = min_data_length + record_bytes(ds_device_request_parameter) + parameter_length;
                 ASSERT_GOTO(service_data->length_in_bytes >= data_length, done);

                 ds_device_request += record_bytes(ds_device_request_parameter); /* skip id and length */
                 ds_device_request += parameter_length;
            }
        }
        /* Add NUL to the target string. Must NUL-terminate it after parsing all parameters.
         * The NUL char is on parameter_count field in the request.
         */
        target_string[target_length] = '\0';
    }

    {
        /* pass data to the callback */
        connector_request_id_t request_id;
        connector_data_service_msg_request_t request_data;
        connector_data_service_msg_response_t response_data;
        connector_data_service_block_t server_data;
        connector_data_service_device_request_t device_request;

        size_t response_data_length = sizeof response_data;

        ASSERT(device_request_service != NULL);
        /* setup structure to the callback */
        request_data.message_type = connector_data_service_type_have_data;
        request_data.service_context = &device_request;
        request_data.server_data = &server_data;
        /* setup device request data */
        server_data.data = (void *)ds_device_request;

        {
            size_t const data_length = (size_t)(ds_device_request - (uint8_t *)service_data->data_ptr);
            server_data.length_in_bytes = service_data->length_in_bytes - data_length;
        }

        server_data.flags = ((isFirstRequest ? CONNECTOR_MSG_FIRST_DATA : 0U) |
                            (MsgIsLastData(service_data->flags) ? CONNECTOR_MSG_LAST_DATA: 0U));

        /* setup request target */
        device_request.target = target_string;
        device_request.device_handle = session;


        response_data.user_context = device_request_service->callback_context;
        response_data.message_status = connector_session_error_none;
        response_data.client_data = NULL;

        request_id.data_service_request = connector_data_service_device_request;
        callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_service, request_id,
                                &request_data, sizeof request_data, &response_data, &response_data_length);

        switch (callback_status)
        {
        case connector_callback_unrecognized:
            /* let message continue to process error status */
            status = connector_working;
            goto done;

        case connector_callback_continue:
            if (response_data_length != sizeof response_data)
            {
                /* wrong size returned and let's cancel the request */
                if (notify_error_status(connector_ptr->callback, connector_class_id_data_service, request_id, connector_invalid_data_size) != connector_working)
                    status = connector_abort;
                goto done;
            }

            if (response_data.message_status != connector_session_error_none)
            {
                /* error returned so cancel this message */
                error_status = response_data.message_status;
                goto done;
            }
            error_status = connector_session_error_none;
            status = connector_working;
            break;

        case connector_callback_busy:
            error_status = connector_session_error_none;
            status = connector_pending;
            break;

        case connector_callback_abort:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_callback_error:
#endif
            /* cancel the message */
            status = connector_abort;
            goto done;
        }
        device_request_service->callback_context = response_data.user_context;
    }

done:
    if (error_status != connector_session_error_none)
    {
        set_data_service_error(service_request, error_status);
    }
    return status;
}

static connector_status_t process_device_response(connector_data_t * const connector_ptr,
                                                       msg_service_request_t * const service_request)
{
    /* Data Service Device response format:
     *  ---------------------------------
     * |   0    |   1    |     2...      |
     *  ---------------------------------
     * | Opcode | status | Response Data |
     *  ---------------------------------
     */
    enum {
        field_define(ds_device_response, opcode, uint8_t),
        field_define(ds_device_response, status, uint8_t),
        record_end(ds_device_response_header)
    };

    connector_status_t status = connector_working;
    connector_callback_status_t callback_status;
    msg_service_data_t * const service_data = service_request->need_data;
    msg_session_t * const session = service_request->session;
    data_service_context_t * const device_request_service = session->service_context;
    connector_bool_t const isFirstResponse = connector_bool(MsgIsStart(service_data->flags));

    /* save some room for response header on 1st response data */
    size_t const header_length = isFirstResponse ? record_bytes(ds_device_response_header) : 0;
    uint8_t * const data_ptr = service_data->data_ptr;

    connector_request_id_t request_id;
    connector_data_service_msg_request_t request_data;
    connector_data_service_msg_response_t response_data;
    connector_data_service_device_request_t device_request;
    connector_data_service_block_t client_data;

    size_t response_data_length = sizeof response_data;

    /* setup request data passed to callback */
    request_data.service_context = &device_request;
    request_data.message_type = connector_data_service_type_need_data;
    request_data.server_data = NULL;

    device_request.target = NULL;
    device_request.device_handle = service_request->session;

    /* setup response data so that callback updates it */
    response_data.client_data = &client_data;
    response_data.user_context = device_request_service->callback_context;
    response_data.message_status = connector_session_error_none;

    client_data.data = data_ptr + header_length;
    client_data.length_in_bytes = service_data->length_in_bytes - header_length;
    client_data.flags = 0;

    request_id.data_service_request = connector_data_service_device_request;
    callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_service, request_id,
                            &request_data, sizeof request_data, &response_data, &response_data_length);

    switch (callback_status)
    {
    case connector_callback_continue:
        status = connector_working;
        if (response_data_length != sizeof response_data)
        {
            /* wrong size returned and let's cancel the request */
            if (notify_error_status(connector_ptr->callback, connector_class_id_data_service, request_id, connector_invalid_data_size) != connector_working)
                status = connector_abort;
            response_data.message_status = connector_session_error_cancel;
        }
        if (response_data.message_status != connector_session_error_none)
        {
            /* cancel this message */
            set_data_service_error(service_request, response_data.message_status);
            goto done;
        }

        if (isFirstResponse)
        {
            enum {
                connector_data_service_device_success,
                connector_data_service_device_not_handled
            };

            /* Add header for 1st response message */
            uint8_t * const ds_device_response = service_data->data_ptr;
            uint8_t const status = ((client_data.flags & CONNECTOR_MSG_DATA_NOT_PROCESSED) == CONNECTOR_MSG_DATA_NOT_PROCESSED) ?
                                    connector_data_service_device_not_handled : connector_data_service_device_success;

            message_store_u8(ds_device_response, opcode, data_service_opcode_device_response);
            message_store_u8(ds_device_response, status, status);
        }

        if ((client_data.flags & CONNECTOR_MSG_LAST_DATA) == CONNECTOR_MSG_LAST_DATA)
        {
            MsgSetLastData(service_data->flags);
        }
        service_data->length_in_bytes = client_data.length_in_bytes + header_length;
        device_request_service->callback_context = response_data.user_context;
        break;

    case connector_callback_busy:
        status = connector_pending;
        break;

    case connector_callback_abort:
    case connector_callback_unrecognized:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
    case connector_callback_error:
#endif

        status = connector_abort;
        break;
    }


done:
    return status;
}

static connector_status_t process_device_error(connector_data_t * const connector_ptr,
                                                       msg_service_request_t * const service_request)
{
    connector_status_t status = connector_working;

    msg_session_t * const session = service_request->session;
    data_service_context_t * const device_request_service = session->service_context;
    connector_request_id_t request_id;

    connector_data_service_msg_request_t request_data;
    connector_data_service_msg_response_t response_data;
    connector_data_service_device_request_t device_request;
    connector_data_service_block_t server_data;

    size_t response_data_length = sizeof response_data;

    device_request.target = NULL;
    device_request.device_handle = session;

    request_data.service_context = &device_request;
    request_data.message_type = connector_data_service_type_error;
    request_data.server_data = &server_data;

    /* get error code from the data pointer */
    server_data.data = &service_request->error_value;
    server_data.length_in_bytes = sizeof service_request->error_value;
    server_data.flags = CONNECTOR_MSG_FIRST_DATA | CONNECTOR_MSG_LAST_DATA;

    response_data.user_context = device_request_service->callback_context;
    response_data.message_status = connector_session_error_none;
    response_data.client_data = NULL;

    request_id.data_service_request = connector_data_service_device_request;

    {
        connector_callback_status_t callback_status;

        callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_service, request_id,
                                        &request_data, sizeof request_data,
                                        &response_data, &response_data_length);

        switch (callback_status)
        {
        case connector_callback_continue:
            status = connector_working;
            break;
        case connector_callback_busy:
            status = connector_pending;
            break;
        case connector_callback_abort:
        case connector_callback_unrecognized:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_callback_error:
#endif
            status = connector_abort;
            break;
        }
    }

    return status;
}

static connector_status_t data_service_device_request_callback(connector_data_t * const connector_ptr, msg_service_request_t * const service_request)
{
    connector_status_t status = connector_working;

    switch (service_request->service_type)
    {
    case msg_service_type_need_data:
        status = process_device_response(connector_ptr, service_request);
        break;

    case msg_service_type_have_data:
        status = process_device_request(connector_ptr, service_request);
        break;

    case msg_service_type_error:
        status = process_device_error(connector_ptr, service_request);
        break;

    case msg_service_type_free:
        {
            msg_session_t * const session = service_request->session;

            status = free_data_buffer(connector_ptr, named_buffer_id(msg_service), session->service_context);
        }
        break;

    default:
        ASSERT(connector_false);
    }

    return status;
}

static size_t fill_put_request_header(connector_request_data_service_send_t const * const request, uint8_t * const data)
{
    uint8_t * ptr = data;

    *ptr++ = data_service_opcode_put_request;

    /* fill path */
    if (request->path != NULL)
    {
        uint8_t const bytes = (uint8_t) strlen(request->path);

        ASSERT(strlen(request->path) <= UCHAR_MAX);
        *ptr++ = bytes;
        memcpy(ptr, request->path, bytes);
        ptr += bytes;
    }

    /* fill parameters */
    {
        connector_bool_t const have_type = connector_bool(request->content_type != NULL);
        uint8_t const parameter_requested = 1;
        uint8_t params = have_type ? 1 : 0;

        enum
        {
            parameter_id_content_type,
            parameter_id_archive,
            parameter_id_append,
            parameter_id_transient
        };

        if (request->option != connector_data_service_send_option_overwrite) params++;

        *ptr++ = params;

        if (have_type)
        {
            uint8_t const bytes = (uint8_t) strlen(request->content_type);

            ASSERT(strlen(request->content_type) <= UCHAR_MAX);
            *ptr++ = parameter_id_content_type;
            *ptr++ = bytes;
            memcpy(ptr, request->content_type, bytes);
            ptr += bytes;
        }

        switch(request->option)
        {
            case connector_data_service_send_option_archive:
                *ptr++ = parameter_id_archive;
                *ptr++ = parameter_requested;
                break;

            case connector_data_service_send_option_append:
                *ptr++ = parameter_id_append;
                *ptr++ = parameter_requested;
                break;

            case connector_data_service_send_option_transient:
                *ptr++ = parameter_id_transient;
                *ptr++ = parameter_requested;
                break;

            default:
                break;
        }
    }

    return (size_t)(ptr - data);
}

static connector_status_t call_put_request_user(connector_data_t * const connector_ptr, msg_service_request_t * const service_request, connector_request_id_data_service_t const request_id, void * const cb_data)
{
    connector_status_t status = connector_working;
    msg_session_t * const session = service_request->session;
    data_service_context_t * const context = (session != NULL) ? session->service_context : NULL;
    connector_callback_status_t callback_status;

    if ((context == NULL) || (context->dp_request == connector_false))
    {
        connector_request_id_t request;

        request.data_service_request = request_id;
        callback_status = connector_callback(connector_ptr->callback, connector_class_id_data_service, request, cb_data);
    }
    #if (defined CONNECTOR_DATA_POINTS)
    else
    {
        callback_status = dp_handle_callback(connector_ptr, cb_data);
    }
    #endif

    switch (callback_status)
    {
        case connector_callback_continue:
            status = connector_working;
            break;

        case connector_callback_error:
            set_data_service_error(service_request, connector_session_error_cancel);
            status = connector_device_error;
            break;

        case connector_callback_busy:
            status = connector_pending;
            break;

        default:
            status = connector_abort;
            break;
    }

if (service_request->service_type == msg_service_type_need_data)
        {
            msg_service_data_t * const service_data = service_request->need_data;
            connector_data_service_block_t * const user_data = response->client_data;


        }
        break;

    return status;
}

static connector_status_t process_send_request(connector_data_t * const connector_ptr, msg_service_request_t * const service_request, void * cb_context)
{
    connector_status_t status = connector_working;
    msg_service_data_t * const service_data = service_request->need_data;
    connector_data_service_send_data_t user_data;

    user_data.transport = connector_transport_tcp;
    user_data.user_context = cb_context;
    user_data.bytes_used = 0;
    user_data.more_data = connector_false;

    if (MsgIsStart(service_data->flags))
    {
        uint8_t * dptr = service_data->data_ptr;
        size_t const bytes = fill_put_request_header(request->service_context, dptr);

        ASSERT_GOTO(bytes < service_data->length_in_bytes, error);
        user_data.buffer = dptr + bytes;
        user_data.bytes_available = service_data->length_in_bytes - bytes;
        service_data->length_in_bytes = bytes;
    }
    else
    {
        user_data.buffer = service_data->data_ptr;
        user_data.bytes_available = service_data->length_in_bytes;
        service_data->length_in_bytes = 0;
    }

    status = call_put_request_user(idigi_ptr, service_request, connector_request_id_data_service_send_data, &user_data);
    if (status == idigi_working)
    {
        service_data->flags = 0;
        service_data->length_in_bytes += user_data.bytes_used;
        if (user_data.more_data == connector_false)
            MsgSetLastData(service_data->flags);
    }

    goto done;

error:
    set_data_service_error(service_request, connector_session_error_format);

done:
    return status;
}

static connector_status_t process_send_response(connector_data_t * const connector_ptr, msg_service_request_t * const service_request, void * cb_context)
{
    connector_status_t status = connector_working;
    connector_data_service_send_response_t user_data;

    /* Data Service put response format:
     *  ---------------------------------
     * |   0    |   1    |     2...      |
     *  ---------------------------------
     * | Opcode | status | Response Data |
     *  ---------------------------------
     */
    enum
    {
        field_define(put_response, opcode, uint8_t),
        field_define(put_response, status, uint8_t),
        record_end(put_response)
    };

    enum
    {
        ds_data_success,
        ds_data_bad_request,
        ds_data_service_unavailable,
        ds_data_server_error
    };

    msg_service_data_t * const service_data = service_request->have_data;
    uint8_t * const put_response = service_data->data_ptr;
    uint8_t const opcode = message_load_u8(put_response, opcode);
    uint8_t const result = message_load_u8(put_response, status);

    ASSERT_GOTO(MsgIsStart(service_data->flags), error);
    ASSERT_GOTO(opcode == data_service_opcode_put_response, error);

    user_data.transport = connector_transport_tcp;
    user_data.user_context = cb_context;
    user_data.hint = (char *)(service_data->length_in_bytes > record_end(put_response)) ? put_response + record_end(put_response) : NULL;

    switch (result)
    {
    case ds_data_success:
        user_data.response = connector_data_service_send_response_success;
        break;

    case ds_data_bad_request:
        user_data.response = connector_data_service_send_response_bad_request;
        break;

    case ds_data_service_unavailable:
        user_data.response = connector_data_service_send_response_unavailable;
        break;

    case ds_data_server_error:
        user_data.response = connector_data_service_send_response_cloud_error;
        break;

    default:
        ASSERT(connector_false);
        break;
    }

    status = call_put_request_user(connector_ptr, service_request, connector_request_id_data_service_send_response, &user_data);
    goto done;

error:
    set_data_service_error(service_request, connector_session_error_format);

done:
    return status;
}

static connector_status_t process_send_error(connector_data_t * const connector_ptr, msg_service_request_t * const service_request, void * cb_context)
{
    connector_status_t status = connector_working;
    msg_service_data_t * const service_data = service_request->need_data;
    connector_data_service_send_status_t user_data;

    user_data.transport = connector_transport_tcp;
    user_data.user_context = cb_context;
    user_data.session_error = connector_session_error_none;

    switch (service_request->error_value)
    {
        case connector_session_error_cancel:
            user_data.status = connector_data_service_send_status_cancel;
            break;

        case connector_session_error_timeout:
            user_data.status = connector_data_service_send_status_timeout;
            break;

        default:
            user_data.status = connector_data_service_send_status_session_error;
            user_data.session_error = service_request->error_value;
            break;
    }

    status = call_put_request_user(idigi_ptr, service_request, connector_request_id_data_service_send_status, &user_data);

    return status;
}

static connector_status_t data_service_put_request_callback(connector_data_t * const connector_ptr, msg_service_request_t * const service_request)
{
    connector_status_t status;
    msg_session_t * const session = service_request->session;
    data_service_context_t * const context = (session != NULL) ? session->service_context : NULL;
    void * cb_context = (context != NULL) ? context->callback_context : NULL;

    switch (service_request->service_type)
    {
        case msg_service_type_need_data:
            status = process_send_request(connector_ptr, service_request, cb_context);
            break;

        case msg_service_type_have_data:
            status = process_send_response(connector_ptr, service_request, cb_context);
            break;

        case msg_service_type_error:
            status = process_send_error(connector_ptr, service_request, cb_context);
            break;

        case msg_service_type_free:
            if (context != NULL)
                status = free_data_buffer(connector_ptr, named_buffer_id(put_request), context);
            else
                status = connector_working;
            goto done;

        default:
            status = connector_idle;
            ASSERT_GOTO(connector_false, error);
            break;
    }

error:
done:
    return status;
}

static connector_status_t data_service_put_request_init(connector_data_t * const connector_ptr, msg_service_request_t * const service_request)
{
    connector_session_error_t result = service_request->error_value;

    if (result != connector_session_error_none)
        goto error;

    {
        msg_session_t * const session = service_request->session;
        data_service_context_t * context = NULL;
        void * data_ptr;
        connector_status_t const ret = malloc_data_buffer(connector_ptr, sizeof *context, named_buffer_id(put_request), &data_ptr);

        if (ret != connector_working)
        {
            result = connector_session_error_memory;
            goto error;
        }

        context = data_ptr;
        context->request_type = connector_data_service_put_request;
        session->service_context = context;

        {
            char const data_point_prefix[] = "DataPoint/";
            connector_request_data_service_send_t * const request = service_request->have_data;

            context->callback_context = request->user_context;
            context->dp_request = connector_bool(strncmp(request->path,data_point_prefix, strlen(data_point_prefix)) == 0);
        }

        goto done;
    }

error:
    set_data_service_error(service_request, result);
    data_service_put_request_callback(connector_ptr, service_request);

done:
    return connector_working;
}

static connector_status_t data_service_callback(connector_data_t * const connector_ptr, msg_service_request_t * const service_request)
{
    connector_status_t status = connector_idle;
    msg_session_t * const session = service_request->session;
    data_service_context_t const * const context = (session != NULL) ? session->service_context : NULL;

    ASSERT_GOTO(connector_ptr != NULL, done);
    ASSERT_GOTO(service_request != NULL, done);

    if (context == NULL)
    {
        connector_msg_callback_t * const init_fn = (service_request->service_type == msg_service_type_pending_request) ? data_service_put_request_init : data_service_device_request_callback;

        status = init_fn(connector_ptr, service_request);
        goto done;
    }

    switch (context->request_type)
    {
        case connector_data_service_put_request:
            status = data_service_put_request_callback(connector_ptr, service_request);
            break;

        case connector_data_service_device_request:
            status = data_service_device_request_callback(connector_ptr, service_request);
            break;

        default:
            ASSERT(connector_false);
            break;
    }

done:
    return status;
}

static connector_status_t connector_facility_data_service_cleanup(connector_data_t * const connector_ptr)
{
    return msg_cleanup_all_sessions(connector_ptr,  msg_service_id_data);
}

static connector_status_t connector_facility_data_service_delete(connector_data_t * const data_ptr)
{
    return msg_delete_facility(data_ptr, msg_service_id_data);
}

static connector_status_t connector_facility_data_service_init(connector_data_t * const data_ptr, unsigned int const facility_index)
{
    return msg_init_facility(data_ptr, facility_index, msg_service_id_data, data_service_callback);
}

static connector_status_t data_service_initiate(connector_data_t * const connector_ptr,  void const * request)
{
    connector_status_t status = connector_invalid_data;

    ASSERT_GOTO(request != NULL, error);

    status = msg_initiate_request(connector_ptr, request) ? connector_success : connector_service_busy;

error:
    return status;
}


