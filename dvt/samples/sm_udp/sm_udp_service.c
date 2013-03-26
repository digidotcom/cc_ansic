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

static int app_waiting_for_ping_complete = 0;
static int app_waiting_for_ping_response = 0;
static int app_waiting_for_data_complete = 0;
static int app_waiting_for_data_response = 0;

#define APP_MAX_TIMEOUT   60
static int app_ping_time_remaining = APP_MAX_TIMEOUT;
static int app_data_time_remaining = APP_MAX_TIMEOUT;

connector_status_t app_send_ping(connector_handle_t handle)
{
    connector_status_t status;
    static connector_message_status_request_t request; /* idigi connector will hold this until send completes */
    static unsigned long flag = 0;

    request.transport = connector_transport_udp;
    if ((flag & CONNECTOR_DATA_RESPONSE_NOT_NEEDED) == CONNECTOR_DATA_RESPONSE_NOT_NEEDED)
    {
        if (app_waiting_for_ping_complete)
        {
            APP_DEBUG("ERROR: No complete callback for ping.\n");
            status = connector_exceed_timeout;
            goto error;
        }
        app_waiting_for_ping_complete = 1;
        request.user_context = &app_waiting_for_ping_complete;
    }
    else
    {
        if (app_waiting_for_ping_response)
        {
            app_ping_time_remaining--;

            if (app_ping_time_remaining == 0)
            {
                APP_DEBUG("ERROR: No response callback for ping.\n");
                status = connector_exceed_timeout;
            }
            else
                status = connector_service_busy;

            goto error;
        }

        app_ping_time_remaining = APP_MAX_TIMEOUT;
        app_waiting_for_ping_response = 1;
        request.user_context = &app_waiting_for_ping_response;
    }

    request.flags = flag;
    status = connector_initiate_action(handle, connector_initiate_status_message, &request);
    if (status != connector_success) /* don't wait, need to set this before calling initiate action to avoid any possible race condition */
    {
        if ((request.flags & CONNECTOR_DATA_RESPONSE_NOT_NEEDED) == CONNECTOR_DATA_RESPONSE_NOT_NEEDED)
            app_waiting_for_ping_complete = 0;
        else
            app_waiting_for_ping_response = 0;
    }
    else
        flag ^= CONNECTOR_DATA_RESPONSE_NOT_NEEDED;

    APP_DEBUG("Sent ping [%d].\n", status);

error:
    return status;
}

typedef struct
{
    char const * data_ptr;
    size_t bytes;
} client_data_t;

connector_status_t app_send_data(connector_handle_t handle)
{
    connector_status_t status = connector_no_resource;
    static connector_data_service_put_request_t header; /* idigi connector will hold this until we get a response/error callback */
    static char const file_path[] = "test/sm_udp.txt";
    static char const buffer[] = "iDigi sm udp dvt for device data\n";
    static client_data_t app_data;
    static unsigned long flag = 0;

    app_data.data_ptr = buffer;
    app_data.bytes = strlen(buffer);
    header.transport = connector_transport_udp;
    if ((flag & CONNECTOR_DATA_RESPONSE_NOT_NEEDED) == 0)
    {
        if (app_waiting_for_data_complete)
        {
            APP_DEBUG("ERROR: No complete callback for data.\n");
            status = connector_exceed_timeout;
            goto error;
        }

        app_waiting_for_data_complete = 1;
    }
    else
    {
        if (app_waiting_for_data_response)
        {
            app_data_time_remaining--;

            if (app_data_time_remaining == 0)
            {
                APP_DEBUG("ERROR: No response callback for data.\n");
                status = connector_exceed_timeout;
            }
            else
                status = connector_service_busy;

            goto error;
        }

        app_data_time_remaining = APP_MAX_TIMEOUT;
        app_waiting_for_data_response = 1;
    }

    header.context = &app_data; /* will be returned in all subsequent callbacks */
    header.path  = file_path;
    header.flags = flag;

    status = connector_initiate_action(handle, connector_initiate_send_data, &header);
    if (status != connector_success) /* don't wait, need to set this before calling initiate action to avoid any possible race condition */
    {
        if ((header.flags & CONNECTOR_DATA_RESPONSE_NOT_NEEDED) == CONNECTOR_DATA_RESPONSE_NOT_NEEDED)
            app_waiting_for_data_complete = 0;
        else
            app_waiting_for_data_response = 0;
    }
    else
        flag ^= CONNECTOR_DATA_RESPONSE_NOT_NEEDED;

    APP_DEBUG("Status: %d, file: %s\n", status, file_path);

error:
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
            static char response_string[] = "Time: Day Mon DD HH:MM:SS YYYY ";
            static client_data_t app_data;
            time_t const cur_time = time(NULL);

            APP_DEBUG("Executing %s.\n", cli_request->content.request.buffer);

            app_data.bytes = snprintf(response_string, sizeof response_string, "Time: %s", ctime(&cur_time));
            app_data.data_ptr = response_string;
            cli_request->user_context = &app_data;
            break;
        }

        case connector_data_service_type_need_data:
        {
            client_data_t const * const app_ptr = cli_request->user_context;

            if (cli_request->content.response.bytes > app_ptr->bytes)
                cli_request->content.response.bytes = app_ptr->bytes;
            memcpy(cli_request->content.response.buffer, app_ptr->data_ptr, cli_request->content.response.bytes);
            APP_DEBUG("Sending CLI response, %s\n", cli_request->content.response.buffer);
            break;
        }

        case connector_data_service_type_total_length:
        {
            client_data_t const * const app_ptr = cli_request->user_context;
            size_t * const length = cli_request->content.total_bytes_ptr;

            *length = app_ptr->bytes;
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

            if (message->length_in_bytes > app_ptr->bytes)
                message->length_in_bytes = app_ptr->bytes;

            memcpy(message->data, app_ptr->data_ptr, message->length_in_bytes);
            app_ptr->data_ptr += message->length_in_bytes;
            app_ptr->bytes -= message->length_in_bytes;
            if (app_ptr->bytes == 0)
                message->flags = CONNECTOR_MSG_LAST_DATA;
            put_response->message_status = connector_msg_error_none;

            break;
        }

        case connector_data_service_type_have_data:
        {
            connector_data_service_block_t * const message = put_request->server_data;

            app_waiting_for_data_response = 0;
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

            *length = app_ptr->bytes;
            break;
        }

        case connector_data_service_type_session_status:
        {
            connector_session_status_t * const status_ptr = (connector_session_status_t *)put_response;

            APP_DEBUG("SM Put request session is completed, status[%d]\n", *status_ptr);
            app_waiting_for_data_complete = 0;
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
            static char const device_response_data[] = "My response data for device request data\n";
            connector_data_service_device_request_t const * const request_ptr = device_request->service_context;
            connector_data_service_block_t const * const device_data = device_request->server_data;
            char const * const data = device_data->data;

            APP_DEBUG("Received device request for target %s.\n", request_ptr->target);
            APP_DEBUG("Data: \"%.*s\".\n", (int)device_data->length_in_bytes, data);

            app_data.data_ptr = device_response_data;
            app_data.bytes = strlen(device_response_data);
            device_response->user_context = &app_data;
            break;
        }

        case connector_data_service_type_need_data:
        {
            client_data_t const * const app_ptr = device_response->user_context;
            connector_data_service_block_t * const client_data = device_response->client_data;

            if (client_data->length_in_bytes > app_ptr->bytes)
                client_data->length_in_bytes = app_ptr->bytes;

            memcpy(client_data->data, app_ptr->data_ptr, client_data->length_in_bytes);
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

            *length = app_ptr->bytes;
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

connector_callback_status_t app_data_service_handler(connector_data_service_request_t const request,
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

            APP_DEBUG("Received %zu opaque bytes on id %d\n", response->bytes, response->id);
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

connector_callback_status_t app_status_handler(connector_status_request_t const request,
                                           void const * const request_data, size_t const request_length,
                                           void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {
        case connector_status_ping_response:
        {
            connector_message_status_response_t const * const status_response = request_data;

            if (status_response->status == connector_session_status_success)
                app_waiting_for_ping_response = 0;
            else
                app_waiting_for_ping_complete = 0;
            APP_DEBUG("Received ping response [%d].\n", status_response->status);
            break;
        }

        case connector_status_ping_request:
        {
            connector_session_status_t * const status = response_data;

            APP_DEBUG("Received ping request.\n");
            *status = connector_session_status_success;
            break;
        }

        case connector_status_tcp:
            status = app_tcp_status(request_data);
            break;

        case connector_status_stop_completed:
            APP_DEBUG("connector_restore_keepalive\n");
            break;

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            break;
    }

    return status;
}
