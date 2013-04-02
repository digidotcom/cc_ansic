/*
 * Copyright (c) 2011, 2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
#include <time.h>
#include "connector_api.h"
#include "platform.h"

typedef struct
{
    char const * data_ptr;
    size_t total_bytes;
    size_t bytes_sent;
} client_data_t;

extern char * sm_dvt_random_buffer;
extern size_t sm_dvt_buffer_size;

connector_status_t app_send_ping(connector_handle_t handle)
{
    connector_status_t status;
    static connector_message_status_request_t request; /* idigi connector will hold this until send completes */

    request.transport = connector_transport_udp;
    request.user_context = NULL;
    request.flags = (request.flags & CONNECTOR_DATA_RESPONSE_NOT_NEEDED) ? 0 : CONNECTOR_DATA_RESPONSE_NOT_NEEDED;
    status = connector_initiate_action(handle, connector_initiate_ping_request, &request);
    APP_DEBUG("Sent ping [%d].\n", status);

    return status;
}

connector_status_t app_send_data(connector_handle_t handle)
{
    static size_t test_cases = 0;
    connector_status_t status = connector_no_resource;
    static connector_data_service_put_request_t header[CONNECTOR_SM_MAX_SESSIONS]; /* idigi connector will hold this until we get a response/error callback */
    static char const file_path[] = "test/sm_udp_mp.txt";
    static client_data_t app_data;
    connector_data_service_put_request_t * const header_ptr = &header[test_cases % CONNECTOR_SM_MAX_SESSIONS];

    ASSERT(sm_dvt_random_buffer != NULL);

    app_data.data_ptr = sm_dvt_random_buffer;
    app_data.total_bytes = sm_dvt_buffer_size;
    app_data.bytes_sent = 0;
    header_ptr->transport = connector_transport_udp;
    header_ptr->flags = (header_ptr->flags & CONNECTOR_DATA_RESPONSE_NOT_NEEDED) ? 0 : CONNECTOR_DATA_RESPONSE_NOT_NEEDED;
    header_ptr->path  = file_path;
    header_ptr->context = &app_data; /* will be returned in all subsequent callbacks */

    status = connector_initiate_action(handle, connector_initiate_send_data, header_ptr);
    APP_DEBUG("Status: %d, file: %s\n", status, file_path);

    if (status == connector_success)
        test_cases++;

    return status;
}

static connector_callback_status_t app_process_cli(connector_sm_cli_request_t * const cli_request)
{
    connector_callback_status_t status = connector_callback_continue;

    ASSERT(cli_request != NULL);

    switch (cli_request->type)
    {
        case connector_data_service_type_have_data:
        {
            static int send_busy = 1;
            static client_data_t app_data;

            if (send_busy)
            {
                APP_DEBUG("CLI request [%s] returning busy...\n", cli_request->content.request.buffer);
                status = connector_callback_busy;
                send_busy = 0;
                break;
            }

            APP_DEBUG("Executing %s.\n", cli_request->content.request.buffer);

            app_data.data_ptr = sm_dvt_random_buffer;
            app_data.total_bytes = sm_dvt_buffer_size;
            cli_request->user_context = &app_data;
            break;
        }

        case connector_data_service_type_need_data:
        {
            static int send_busy = 1;
            client_data_t const * const app_ptr = cli_request->user_context;

            if (send_busy)
            {
                APP_DEBUG("CLI response returning busy...\n");
                status = connector_callback_busy;
                send_busy = 0;
                break;
            }

            if (cli_request->content.response.bytes > app_ptr->total_bytes)
                cli_request->content.response.bytes = app_ptr->total_bytes;
            memcpy(cli_request->content.response.buffer, app_ptr->data_ptr, cli_request->content.response.bytes);
            APP_DEBUG("Sending CLI response. [%s]\n", cli_request->content.response.buffer);
            break;
        }

        case connector_data_service_type_total_length:
        {
            static int send_busy = 1;
            client_data_t const * const app_ptr = cli_request->user_context;
            size_t * const length = cli_request->content.total_bytes_ptr;

            if (send_busy)
            {
                APP_DEBUG("CLI get length returning busy...\n");
                status = connector_callback_busy;
                send_busy = 0;
                break;
            }

            *length = app_ptr->total_bytes;
            break;
        }

        case connector_data_service_type_session_status:
        {
            APP_DEBUG("CLI session is closed, status[%d]\n", cli_request->content.status);
            break;
        }

        default:
            APP_DEBUG("Unexpected CLI request type: %d\n", cli_request->type);
            status = connector_callback_abort;
            break;
    }

    return status;
}

static connector_callback_status_t app_handle_put_request(connector_data_service_msg_request_t const * const put_request, connector_data_service_msg_response_t * const put_response)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_data_service_put_request_t * const header = put_request->service_context;
    client_data_t * const app_ptr = (client_data_t *)header->context;

    if ((put_request == NULL) || (header == NULL) || (app_ptr == NULL))
    {
        APP_DEBUG("app_handle_put_request: Invalid input\n");
        status = connector_callback_abort;
        goto error;
    }

    switch (put_request->message_type)
    {
        case connector_data_service_type_need_data:
        {
            connector_data_service_block_t * const message = put_response->client_data;
            size_t const bytes_remaining = app_ptr->total_bytes - app_ptr->bytes_sent;

            if (message->length_in_bytes > bytes_remaining)
                message->length_in_bytes = bytes_remaining;

            memcpy(message->data, &app_ptr->data_ptr[app_ptr->bytes_sent], message->length_in_bytes);
            app_ptr->bytes_sent += message->length_in_bytes;
            if (app_ptr->bytes_sent >= app_ptr->total_bytes)
                message->flags = CONNECTOR_MSG_LAST_DATA;
            put_response->message_status = connector_msg_error_none;

            break;
        }

        case connector_data_service_type_have_data:
        {
            connector_data_service_block_t * const message = put_request->server_data;

            APP_DEBUG("Received %s response from server\n", ((message->flags & CONNECTOR_MSG_RESP_SUCCESS) != 0) ? "success" : "error");
            if (message->length_in_bytes > 0)
            {
                char * const data = message->data;

                data[message->length_in_bytes] = '\0';
                APP_DEBUG("Server response %s\n", data);
            }

            break;
        }

        case connector_data_service_type_error:
        {
            connector_data_service_block_t * const message = put_request->server_data;
            connector_msg_error_t const * const error_value = message->data;

            APP_DEBUG("Put request error: %d\n", *error_value);

            break;
        }

        case connector_data_service_type_total_length:
        {
            size_t * const length = (size_t *)put_response;

            *length = app_ptr->total_bytes;
            break;
        }

        case connector_data_service_type_session_status:
        {
            static size_t timeout_count = 0;
            connector_session_status_t * const status_ptr = (connector_session_status_t *)put_response;

            APP_DEBUG("SM Put request session is completed, status[%d]\n", *status_ptr);
            switch (*status_ptr)
            {
                case connector_session_status_success:
                case connector_session_status_complete:
                case connector_session_status_cancel:
                    break;

                case connector_session_status_timeout:
                    if (timeout_count++ > CONNECTOR_SM_MAX_SESSIONS)
                    {
                        APP_DEBUG("SM Put request timeout error. No response from the server!");
                        status = connector_callback_abort;
                    }
                    break;

                default:
                    status = connector_callback_abort;
                    break;
            }
            break;
        }

        default:
            APP_DEBUG("Unexpected type in put request: %d\n", put_request->message_type);
            status = connector_callback_abort;
            break;
    }

error:
    return status;
}

static connector_callback_status_t app_handle_device_request(connector_data_service_msg_request_t const * const device_request, connector_data_service_msg_response_t * const device_response)
{
    connector_callback_status_t status = connector_callback_continue;

    if ((device_request == NULL) || (device_response == NULL))
    {
        APP_DEBUG("app_handle_device_request: Invalid input\n");
        status = connector_callback_abort;
        goto error;
    }

    switch (device_request->message_type)
    {
        case connector_data_service_type_have_data:
        {
            static client_data_t app_data;
            connector_data_service_device_request_t const * const request_ptr = device_request->service_context;
            connector_data_service_block_t const * const device_data = device_request->server_data;

            APP_DEBUG("Received device request for target %s, size: %zu bytes\n", request_ptr->target, device_data->length_in_bytes);

            if ((device_data->flags & CONNECTOR_MSG_FIRST_DATA) == CONNECTOR_MSG_FIRST_DATA)
            {
                app_data.data_ptr = sm_dvt_random_buffer;
                app_data.total_bytes = sm_dvt_buffer_size;
                app_data.bytes_sent = 0;
                device_response->user_context = &app_data;
            }

            if (memcmp(device_data->data, &app_data.data_ptr[app_data.bytes_sent], device_data->length_in_bytes))
            {
                APP_DEBUG("ERROR: Mismatch in expected and received data in sm multipart device request\n");
                status = connector_callback_abort;
            }

            if ((device_data->flags & CONNECTOR_MSG_LAST_DATA) != CONNECTOR_MSG_LAST_DATA)
                app_data.bytes_sent += device_data->length_in_bytes;
            else
                app_data.bytes_sent = 0;
            break;
        }

        case connector_data_service_type_need_data:
        {
            client_data_t * const app_ptr = device_response->user_context;
            connector_data_service_block_t * const client_data = device_response->client_data;
            size_t const bytes_remaining = app_ptr->total_bytes - app_ptr->bytes_sent;

            if (client_data->length_in_bytes > bytes_remaining)
                client_data->length_in_bytes = bytes_remaining;

            memcpy(client_data->data, &app_ptr->data_ptr[app_ptr->bytes_sent], client_data->length_in_bytes);
            app_ptr->bytes_sent += client_data->length_in_bytes;
            if (app_ptr->bytes_sent >= app_ptr->total_bytes)
                client_data->flags = CONNECTOR_MSG_LAST_DATA;
            break;
        }

        case connector_data_service_type_error:
        {
            connector_data_service_block_t * const message = device_request->server_data;
            connector_msg_error_t const * const error_value = message->data;

            APP_DEBUG("Device request error: %d\n", *error_value);
            break;
        }

        case connector_data_service_type_total_length:
        {
            client_data_t const * const app_ptr = device_request->service_context;
            size_t * const length = (size_t *)device_response;

            *length = app_ptr->total_bytes;
            break;
        }

        case connector_data_service_type_session_status:
        {
            connector_session_status_t * const status_ptr = (connector_session_status_t *)device_response;

            APP_DEBUG("SM device request session is closed, status[%d]\n", *status_ptr);
            break;
        }

        default:
            APP_DEBUG("Unexpected type in device request: %d\n", device_request->message_type);
            status = connector_callback_abort;
            break;
    }

error:
    return status;
}

connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {
        case connector_data_service_put_request:
            status = app_handle_put_request(request_data, response_data);
            break;

        case connector_data_service_device_request:
            status = app_handle_device_request(request_data, response_data);
            break;

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            break;
    }

    return status;
}

connector_callback_status_t app_sm_handler(connector_sm_request_t const request,
                                       void const * const request_data, size_t const request_length,
                                       void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_data);
    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {
        case connector_sm_cli_request:
            status = app_process_cli(response_data);
            break;

        case connector_sm_opaque_response:
        {
            connector_sm_opaque_response_t * const response = response_data;
            static size_t test_case = 0;

            APP_DEBUG("Received %zu opaque bytes on id %d\n", response->bytes, response->id);
            switch (test_case)
            {
                case 0:
                    status = connector_callback_busy;
                    break;

                case 1:
                    break;

                case 2:
                    response->status = connector_invalid_data;
                    break;

                default:
                    break;
            }
            test_case++;
            break;
        }

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            break;
    }

    return status;
}

static connector_callback_status_t app_tcp_status(connector_tcp_status_t const * const status)
{

    switch (*status)
    {
    case connector_tcp_communication_started:
        APP_DEBUG("connector_tcp_communication_started\n");
        break;
    case connector_tcp_keepalive_missed:
        APP_DEBUG("connector_tcp_keepalive_missed\n");
        break;
    case connector_tcp_keepalive_restored:
        APP_DEBUG("connector_tcp_keepalive_restored\n");
        break;
    }

    return connector_callback_continue;
}

connector_callback_status_t app_status_handler(connector_request_id_status_t const request, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request)
    {
        case connector_status_ping_response:
        {
            static size_t timeout_count = 0;
            connector_message_status_response_t const * const status_response = data;

            APP_DEBUG("Received ping response [%d].\n", status_response->status);
            switch (status_response->status)
            {
                case connector_session_status_success:
                case connector_session_status_complete:
                case connector_session_status_cancel:
                    break;

                case connector_session_status_timeout:
                    if (timeout_count++ > CONNECTOR_SM_MAX_SESSIONS)
                    {
                        APP_DEBUG("Ping request timeout error. No response from the server!");
                        status = connector_callback_abort;
                    }
                    break;

                default:
                    status = connector_callback_abort;
                    break;
            }
            break;
        }

        case connector_status_ping_request:
        {
            connector_status_t * const status = data;

            APP_DEBUG("Received ping request.\n");
            *status = connector_success;
            break;
        }

        case connector_request_id_status_tcp:
            status = app_tcp_status(data);
            break;

        case connector_request_id_status_stop_completed:
            APP_DEBUG("connector_restore_keepalive\n");
            break;

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            break;
    }

    return status;
}
