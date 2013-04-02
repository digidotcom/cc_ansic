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

#include <stdlib.h>
#include <stdbool.h>
#include "connector_api.h"
#include "platform.h"
#include "application.h"

typedef struct device_request_handle {
    connector_request_id_data_service_t request;
    char const * response_data;
    device_request_target_t target;
    size_t length_in_bytes;
    connector_transport_t transport;
    connector_bool_t response_required;
} device_request_handle_t;

static unsigned int device_request_active_count = 0;

#define enum_to_case(name)  case name:  result = #name;             break

static char const * device_request_to_string(device_request_target_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(device_request_not_support);
        enum_to_case(device_request_invalid_data);
        enum_to_case(device_request_stop_idigi);
        enum_to_case(device_request_start_idigi);
        enum_to_case(device_request_stop_terminate_idigi);
        enum_to_case(device_request_app_stop_idigi);
        enum_to_case(device_request_app_start_idigi);
        enum_to_case(device_request_stop_all_transports);
        enum_to_case(device_request_abort_device_request);
        enum_to_case(device_request_abort_stop_callback);
        enum_to_case(device_request_terminate_idigi);
    }
    return result;
}

char const * transport_to_string(connector_transport_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_transport_tcp);
        enum_to_case(connector_transport_udp);
        enum_to_case(connector_transport_sms);
        enum_to_case(connector_transport_all);
    }
    return result;
}

static connector_status_t device_request_action(device_request_handle_t * const client_device_request)
{
    connector_status_t status = connector_success;

    switch (client_device_request->target)
    {
        case device_request_not_support:
            break;

        case device_request_stop_idigi:
        APP_DEBUG("device_request_action: stop %s iDigi (connector_wait_sessions_complete)\n", transport_to_string(client_device_request->transport));
        connector_close_status = connector_close_status_device_stopped;
        {
            connector_initiate_stop_request_t const request_data = {client_device_request->transport, connector_wait_sessions_complete, NULL};

            stop_transport_count[request_data.transport]++;  /* counter which is used to ensure the status_complete callback is called */
            status = connector_initiate_action(connector_handle, connector_initiate_transport_stop, &request_data);
            if (status != connector_success)
            {
                APP_DEBUG("device_request_action: connector_initiate_action for connector_initiate_transport_stop returns error %d\n", status);
                goto done;
            }

            break;
        }
        case device_request_start_idigi:
        APP_DEBUG("device_request_action: start %s iDigi\n", transport_to_string(client_device_request->transport));
        {
            status = connector_initiate_action(connector_handle, connector_initiate_transport_start, &client_device_request->transport);
            if (status != connector_success)
            {
                APP_DEBUG("device_request_action: connector_initiate_action for connector_initiate_transport_start returns error %d\n", status);
                goto done;
            }

            break;
        }
        case device_request_stop_terminate_idigi:
        APP_DEBUG("device_request_action: stop %s iDigi immediately and terminate\n", transport_to_string(client_device_request->transport));
        connector_close_status = connector_close_status_device_stopped;
        {
            connector_initiate_stop_request_t const request_data = {client_device_request->transport, connector_stop_immediately, NULL};

            stop_transport_count[request_data.transport]++;  /* counter which is used to ensure the status_complete callback is called */
            status = connector_initiate_action(connector_handle, connector_initiate_transport_stop, &request_data);
            if (status != connector_success)
            {
                APP_DEBUG("device_request_action: connector_initiate_action for connector_initiate_transport_stop returns error %d\n", status);
                goto done;
            }
            break;
        }
        case device_request_app_stop_idigi:
            APP_DEBUG("device_request_action: application stop %s idigi\n", transport_to_string(client_device_request->transport));
            break;

        case device_request_app_start_idigi:
            APP_DEBUG("device_request_action: application start %s idigi\n", transport_to_string(client_device_request->transport));
            break;

        case device_request_stop_all_transports:
            APP_DEBUG("device_request_action: stop All iDigi transport\n");
            connector_close_status = connector_close_status_device_stopped;
            {
                connector_initiate_stop_request_t const request_data = {connector_transport_all, connector_wait_sessions_complete, NULL};

                stop_transport_count[request_data.transport]++;  /* counter which is used to ensure the status_complete callback is called */
                status = connector_initiate_action(connector_handle, connector_initiate_transport_stop, &request_data);
                if (status != connector_success)
                {
                    APP_DEBUG("device_request_action: connector_initiate_action for connector_initiate_transport_stop returns error %d\n", status);
                    goto done;
                }
                break;
            }
        case device_request_abort_stop_callback:
            APP_DEBUG("device_request_action: abort at %s stop callback\n", transport_to_string(client_device_request->transport));
            connector_close_status = connector_close_status_device_stopped;
            stop_callback_status = connector_callback_abort;
            {
                /* call initiate action to stop and let the close callback to return abort */
                connector_initiate_stop_request_t const request_data = {client_device_request->transport, connector_wait_sessions_complete, NULL};

                stop_transport_count[request_data.transport]++;  /* counter which is used to ensure the status_complete callback is called */
                status = connector_initiate_action(connector_handle, connector_initiate_transport_stop, &request_data);
                if (status != connector_success)
                {
                    APP_DEBUG("device_request_action: connector_initiate_action for connector_initiate_transport_stop returns error %d\n", status);
                    goto done;
               }
                break;
            }

        case device_request_abort_device_request:
            break;

        case device_request_terminate_idigi:
        APP_DEBUG("device_request_action: stop iDigi immediately and terminate\n");
        connector_close_status = connector_close_status_device_terminated;
        {
            status = connector_initiate_action(connector_handle, connector_initiate_terminate, NULL);
            if (status != connector_success)
            {
                APP_DEBUG("device_request_action: connector_initiate_action for connector_initiate_terminate returns error %d\n", status);
                goto done;
            }
            break;
        }
    }

    initiate_action.target = client_device_request->target;
    initiate_action.transport = client_device_request->transport;
    APP_DEBUG("device_request_action: initiate transport %d\n", initiate_action.transport);


done:
    return status;
}

static char const device_request_done[] = "Initiate action done";
static char const device_request_unable_to_processed[] = "Initiate action error";
static char const device_request_not_supported_target = "Target not supported";
static char const device_request_not_supported_data = "Invalid data";
static char const device_device_no_memory[] = "No memory";

static size_t const device_request_max_response_length = (sizeof device_request_done > sizeof device_request_unable_to_processed) ? sizeof device_request_done : sizeof device_request_unable_to_processed;

static connector_callback_status_t app_process_device_request_target(connector_data_service_receive_target_t * const target_data)
{
    /* supported target name */
    static char const stop_target[] = "stop_idigi";
    static char const start_target[] = "start_idigi";
    static char const stop_terminate_target[] = "stop_terminate_idigi";
    static char const app_stop_target[] = "application_stop_idigi";
    static char const app_start_target[] = "application_start_idigi";
    static char const stop_all_transports_target[] = "stop_all_transports";
    static char const abort_device_request[] = "abort_device_request";
    static char const abort_stop_callback[] = "abort_stop_callback";
    static char const terminate_idigi[] = "terminate_idigi";

    connector_callback_status_t result = connector_callback_continue;
    device_request_target_t target_type = device_request_not_support;

    device_request_handle_t * device_request = target_info->user_context;

    ASSERT(target_info->target != NULL);

    if (device_request == NULL)
    {

        if (strcmp(target_info->target, stop_target) == 0)
        {
            target_type = device_request_stop_idigi;
        }
        else if (strcmp(target_info->target, start_target) == 0)
        {
            target_type = device_request_start_idigi;
        }
        else if (strcmp(target_info->target, stop_terminate_target) == 0)
        {
            target_type = device_request_stop_terminate_idigi;
        }
        else if (strcmp(target_info->target, app_stop_target) == 0)
        {
            target_type = device_request_app_stop_idigi;
        }
        else if (strcmp(target_info->target, app_start_target) == 0)
        {
            target_type = device_request_app_start_idigi;
        }
        else if (strcmp(target_info->target, stop_all_transports_target) == 0)
        {
            target_type = device_request_stop_all_transports;
        }
        else if (strcmp(target_info->target, abort_device_request) == 0)
        {
            connector_close_status = connector_close_status_abort;
            result = connector_callback_abort;
#if 0
            {
                /* call initiate action to stop and let the close callback to return abort */
                connector_initiate_stop_request_t const request_data = {transport_type, connector_wait_sessions_complete, NULL};

                stop_transport_count[request_data.transport]++;  /* counter which is used to ensure the status_complete callback is called */
                connector_status_t const status = connector_initiate_action(connector_handle, connector_initiate_transport_stop, &request_data);
                if (status != connector_success)
                {
                    APP_DEBUG("app_process_device_request: connector_initiate_action for connector_initiate_transport_stop returns error %d\n", status);
                }
            }
#endif
            goto done;
        }
        else if (strcmp(target_info->target, abort_stop_callback) == 0)
        {
            target_type = device_request_abort_stop_callback;
        }
        else if (strcmp(target_info->target, terminate_idigi) == 0)
        {
            target_type = device_request_terminate_idigi;
        }

        /* 1st chunk of device request so let's allocate memory for it
         * and setup user_context for the client_device_request.
         */
        {
            void * ptr;

            connector_callback_status_t const is_ok = app_os_malloc(sizeof * client_device_request, &ptr);
            if (is_ok != connector_callback_continue|| ptr == NULL)
            {
                /* no memeory so cancel this request */
                APP_DEBUG("app_process_device_request_target: malloc fails for device request target = %s\n", target_info->target);
                result = connector_callback_error;
                goto done;
            }

            evice_request = ptr;
        }
        device_request->length_in_bytes = 0;
        device_request->response_data = NULL;
        device_request->target = target_type;
        device_request->transport = connector_transport_all;
        device_request->response_required = target_info->response_required;

        APP_DEBUG("app_process_device_request_target: %p request %s on %s previous %s\n", (void *) client_device_request, device_request_to_string(target_type), transport_to_string(transport_type), device_request_to_string(initiate_action.target));

         /* setup the user_context for our device request data */
         target_info->user_context = device_request;
         device_request_active_count++;
    }

done:
    return result;
}

static connector_callback_status_t app_process_device_request_data(connector_data_service_receive_data_t * const receive_data)
{
    static char const udp_transport[] = "UDP";
    static char const sms_transport[] = "SMS";
    static char const tcp_transport[] = "TCP";

    connector_callback_status_t result = connector_callback_continue;
    device_request_handle_t * device_request = receie_data->user_context;

    connector_transport_t transport_type = connector_transport_all;

    {
        char * transport = receive_data->buffer;

        if (memcmp(transport, udp_transport, receive_data->bytes_used) == 0)
        {
#if (defined CONNECTOR_TRANSPORT_UDP)
            transport_type = connector_transport_udp;
#endif
        }
        else if (memcmp(transport, sms_transport,receive_data->bytes_used) == 0)
        {
#if (defined CONNECTOR_TRANSPORT_SMS)
            transport_type = connector_transport_sms;
#endif
        }
        else if (memcmp(transport, tcp_transport, receive_data->bytes_used) == 0)
        {
#if (defined CONNECTOR_TRANSPORT_TCP)
            transport_type = connector_transport_tcp;
#endif
        }
        else
        {
            APP_DEBUG("app_process_device_request_data: invalid data  %zu \"%.*s\"\n", receive_data->bytes_used, (int)receive_data->bytes_used, transport);
            device_request->target = device_request_invalid_data;
            result = connector_callback_error;
        }
    }

    if (!target_info->response_required)
    {
        device_request->transport = transport_type;

        if ((result == connecotr_callback_continue) && (device_request_action(device_request) == connector_service_busy))
        {
            result = connector_callback_busy;
            goto done;
        }

        app_os_free(device_request);
        device_request_active_count--;
    }

done:
    return result;
}

static connector_callback_status_t app_process_device_request_reply_length(connector_data_service_receive_reply_length_t const * const reply_data)
{
    connector_callback_status_t result = connector_callback_continue;

    device_request_handle_t * const device_request = status_data->user_context;

    if (device_request == NULL)
    {
        reply_data->total_bytes = sizeof device_device_no_memory -1;
    }
    else
    {
        switch (device_request->target)
        {
            case device_request_not_support:
                reply_data->total_bytes = sizeof device_request_not_supported_target -1;
                break;

            case device_request_invalid_data:
                reply_data->total_bytes = sizeof device_request_not_supported_data -1;
                break;
            default:
                reply_data->total_bytes = device_request_max_response_length;
                break;
        }
    }

    return result;
}

static connector_callback_status_t app_process_device_request_response(connector_data_service_receive_reply_data_t * const reply_data)
{
    connector_callback_status_t result = connector_callback_continue;
    device_request_handle_t * const device_request = reply_data->user_context;


    UNUSED_ARGUMENT(request_data);

    if (device_request == NULL)
    {
        size_t const device_device_no_memory_lenght = sizeof device_device_no_memory -1;

        ASSERT(reply_data->bytes_available >= device_device_no_memory_lenght);

        memcpy(reply_data->buffer, device_device_no_memory, device_device_no_memory_lenght);
        reply_data->bytes_used = device_device_no_memory_lenght;
        goto done;
    }


    if (device_request->length_in_bytes == 0)
    {
        char * response_message_data = (char *)device_request_done;

        if (initiate_action.target != device_request_not_support)
        {
            /* busy doing previous initiate_action */
            result = connector_callback_busy;
            goto done;
        }

        switch (device_request->target)
        {
            case device_request_not_support:
                response_message_data = device_request_not_supported_target;
                break;

            case device_request_invalid_data:
                response_message_data = device_request_not_supported_data;
                break;

            case device_request_abort_device_request:
                /* should not come here. Abort in the request */
                ASSERT(0);
                break;

            default:
            {
                connector_status_t const status = device_request_action(client_device_request);
                if (status == connector_service_busy)
                {
                    result = connector_callback_busy;
                    goto done;
                }
                if (status != connector_success)
                {
                    response_message_data = (char *)device_request_unable_to_processed;
                }
            }

        }

        APP_DEBUG("app_process_device_request_response: device_request_target = %s\n", device_request_to_string(initiate_action.target));
        device_request->response_data = response_message_data;
        device_request->length_in_bytes = strlen(response_message_data);
    }

    {
        /* get number of bytes written to the client data buffer */
        size_t const bytes = (device_request->length_in_bytes < reply_data->bytes_available) ? device_request->length_in_bytes : reply_data->bytes_available;

        /* let's copy the response data to service_response buffer */
        memcpy(reply_data->buffer, device_request->response_data, bytes);

        device_request->response_data += bytes;
        client_device_request->length_in_bytes -= bytes;

        reply_data->more_data = (device_request->length_in_bytes == 0) ? connector_false : connector_true;
        reply_data->bytes_used = bytes;

    }

    if (!reply_data->more_data)
    {   /* done */
        APP_DEBUG("app_process_device_request_response: done %p\n", (void *)device_request);
        device_request_active_count--;
        app_os_free(device_request);
    }

done:
    return result;
}

static connector_callback_status_t app_process_device_request_status(connector_data_service_status_t const * const status_data)
{
    connector_callback_status_t status = connector_callback_continue;

    device_request_handle_t * const device_request = status_data->user_context;

    ASSERT(device_request != NULL);

    switch (status_data->status)
    {
    case connector_data_service_status_session_error:
        APP_DEBUG("app_process_device_request_status: handle %p session error %d\n",
                   (void *) device_request, status_data->session_error);
        break;
    default:
        APP_DEBUG("app_process_device_request_status: handle %p error %d\n",
                    (void *)device_request, status_data->status);
        break;
    }

    device_request_active_count--;
    put_file_active_count[client_device_request->transport]--;
    app_os_free(device_request);

    return status;
}

connector_callback_status_t app_device_request_handler(connector_request_id_data_service_t const request_id, void * data)
{
    connector_callback_status_t status = connector_callback_unrecognized;

    switch (request_id)
    {
        case connector_request_id_data_service_receive_target:
            status = app_process_device_request_target(data);
            break;
        case connector_request_id_data_service_receive_data:
            status = app_process_device_request_data(data);
            break;
        case connector_request_id_data_service_receive_status:
            status = app_process_device_request_status(data);
            break;
        case connector_request_id_data_service_receive_reply_data:
            status = app_process_device_request_response(data);
            break;
        case connector_request_id_data_service_receive_reply_length:
            app_process_device_request_reply_length(data);
            break;
        default:
            APP_DEBUG("app_device_request_handler: unknown request id type %d for connector_request_id_data_service\n", request_id);
            break;
    }

    return status;
}


static connector_callback_status_t put_request_need_data_handle(connector_data_service_msg_request_t const * const put_request, connector_data_service_msg_response_t * const put_response)
{
    connector_data_service_put_request_t const * const header = put_request->service_context;
    ds_record_t * const user = (ds_record_t * const)header->context;

    connector_data_service_block_t * message = put_response->client_data;

    char * dptr = message->data;
    size_t const bytes_available = message->length_in_bytes;
    size_t const bytes_to_send = user->file_length_in_bytes - user->bytes_sent;
    size_t bytes_copy = (bytes_to_send > bytes_available) ? bytes_available : bytes_to_send;

    memcpy(dptr, &user->file_data[user->bytes_sent], bytes_copy);
    message->length_in_bytes = bytes_copy;
    message->flags = 0;
    if (user->bytes_sent == 0)
    {
        app_os_get_system_time(&user->first_data_time);
        message->flags |= CONNECTOR_MSG_FIRST_DATA;
        APP_DEBUG("put_request_need_data_handle: %s %p total length %zu\n", user->file_path, (void *)user, user->file_length_in_bytes);
    }

    user->bytes_sent += bytes_copy;
    if (user->bytes_sent == user->file_length_in_bytes)
    {
        app_os_get_system_time(&user->last_data_time);
        message->flags |= CONNECTOR_MSG_LAST_DATA;
        APP_DEBUG("put_request_need_data_handle: %s %p done\n", user->file_path, (void *)user);
    }

    return connector_callback_continue;
}

static connector_callback_status_t put_request_have_data_handle(connector_data_service_msg_request_t const * const put_request)
{
    connector_data_service_put_request_t const * const header = put_request->service_context;
    ds_record_t * const user = (ds_record_t * const)header->context;

    connector_data_service_block_t * message = put_request->server_data;
    char * data = message->data;
    unsigned long current_time;

    app_os_get_system_time(&current_time);
    APP_DEBUG("put_request_have_data_handle: %s %p\n", user->file_path, (void *)user);
    APP_DEBUG("put_request_have_data_handle: time initiate = %lu\n", user->initiate_time);
    APP_DEBUG("put_request_have_data_handle: time between initiate and 1st data  = %lu\n", (user->first_data_time - user->initiate_time));
    APP_DEBUG("put_request_have_data_handle: time between 1st and last data = %lu\n", (user->last_data_time - user->first_data_time));
    APP_DEBUG("put_request_have_data_handle: time between last data and response = %lu\n", (current_time - user->last_data_time));

    if (message->length_in_bytes > 0)
    {
        data[message->length_in_bytes] = '\0';
        APP_DEBUG("put_request_have_data_handle: server response (%zu) %s\n", message->length_in_bytes, data);
    }

    /* should be done now */
    put_file_active_count[user->header.transport]--;
    app_os_free(user);
    APP_DEBUG("put_request_have_data_handle: status = 0x%x %s done this session %p\n",
            message->flags, user->file_path, (void *)user);

    return connector_callback_continue;
}

static connector_callback_status_t put_request_error_handle(connector_data_service_msg_request_t const * const put_request)
{
    connector_data_service_put_request_t const * const header = put_request->service_context;
    ds_record_t * const user = (ds_record_t * const)header->context;

    APP_DEBUG("put_request_error_handle: %s cancel this session %p\n", user->file_path, (void *)user);
    ASSERT(user != NULL);
    put_file_active_count[user->header.transport]--;
    app_os_free(user);

    return connector_callback_continue;
}

static connector_callback_status_t put_request_total_length_handle(connector_data_service_msg_request_t const * const put_request, size_t * const length)
{
//    connector_data_service_put_request_t const * const header = put_request->service_context;
    //ds_record_t * const user = (ds_record_t * const)header->context;
    ds_record_t * const user = (ds_record_t * const)put_request->service_context;

    *length = user->file_length_in_bytes;

    return connector_callback_continue;
}

static connector_callback_status_t put_request_status_handle(connector_data_service_msg_request_t const * const put_request, connector_session_status_t * const response_status)
{
//    connector_data_service_put_request_t const * const header = put_request->service_context;
//    ds_record_t * const user = (ds_record_t * const)header->context;

    ds_record_t * const user = (ds_record_t * const)put_request->service_context;

    APP_DEBUG("put_request_status_handle: status[%d]\n", *response_status);

    put_file_active_count[user->header.transport]--;
    app_os_free((void *)user);

    return connector_callback_continue;
}

idigi_callback_status_t app_put_request_handler(void const * request_data, size_t const request_length,
                                                   void * response_data, size_t * const response_length)
{
    idigi_callback_status_t status = idigi_callback_unrecognized;

    idigi_data_service_msg_request_t const * const put_request = request_data;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (put_request->message_type)
    {
    case idigi_data_service_type_need_data:
        status = put_request_need_data_handle(request_data, response_data);
        break;

    case idigi_data_service_type_have_data:
        status = put_request_have_data_handle(request_data);
        break;

    case idigi_data_service_type_error:
        status = put_request_error_handle(request_data);
        break;

    case idigi_data_service_type_total_length:
        status = put_request_total_length_handle(request_data, response_data);
        break;

    case idigi_data_service_type_session_status:
        status = put_request_status_handle(request_data, response_data);
        break;
    default:
        APP_DEBUG("app_put_request_handler: Unexpected message type: %d\n", put_request->message_type);
        break;
    }

    return status;
}

connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request_id,
                                                  void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request_id)
    {
    case connector_request_id_data_service_send_length:
    case connector_request_id_data_service_send_data:
    case connector_request_id_data_service_send_status:
    case connector_request_id_data_service_send_response:
        status = app_put_request_handler(request_id, data);
        break;
    case connector_request_id_data_service_receive_target:
    case connector_request_id_data_service_receive_data:
    case connector_request_id_data_service_receive_status:
    case connector_request_id_data_service_receive_reply_data:
    case connector_request_id_data_service_receive_reply_length:
        status = app_device_request_handler(request_id, data);
        break;
    default:
        APP_DEBUG("app_data_service_handler: Request not supported: %d\n", request_id);
        break;
    }
    return status;
}

