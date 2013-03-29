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
#include "application.h"


#if defined APP_USE_SM_UDP
const connector_transport_t app_transport = connector_transport_udp;

#elif defined APP_USE_SM_SMS
const connector_transport_t app_transport = connector_transport_sms;
#endif

typedef struct
{
    char const * data_ptr;
    size_t bytes;
} client_data_t;

static client_data_t * app_send_data = NULL;

#ifdef APP_USE_SM_UDP
connector_status_t app_send_ping(connector_handle_t handle)
{
    static connector_message_status_request_t request; /* idigi connector will hold this until reply received or send completes */
    connector_status_t status;

    request.transport = connector_transport_udp;
    request.user_context = &request;
    request.flags = 0;
    status = connector_initiate_action(handle, connector_initiate_status_message, &request);
    /* APP_DEBUG("Sent ping [%d].\n", status); */

    return status;
}
#endif

app_bool_t app_put_request_complete(void)
{
    return app_send_data == NULL;
}

connector_status_t app_cancel_put_request(connector_handle_t handle)
{
    connector_status_t status = connector_success;

    if (app_send_data != NULL)
    {
       static connector_message_status_request_t request;

       request.transport = app_transport;
       request.flags = 0;
       request.user_context = app_send_data;

       APP_DEBUG("Previous data send pending, cancel it\n");
       status = connector_initiate_action(handle, connector_initiate_session_cancel, &request);
       if (status == connector_success)
           status = connector_service_busy;
       else
           APP_DEBUG("connector_initiate_session_cancel returned %d\n", status);
    }
    return status;
}

connector_status_t app_send_put_request(connector_handle_t handle, app_bool_t response_needed)
{
    connector_status_t status;
    static connector_data_service_put_request_t header; /* idigi connector will hold this until we get a response/error callback */
    static char const file_type[] = "text/plain";
    static char const buffer[] = "iDigi sm_send_data sample data\n";


    if (app_send_data != NULL)
    {
       status = connector_service_busy;
       goto done;
    }

    app_send_data = malloc(sizeof *app_send_data);

    if (app_send_data == NULL)
    {
        status = connector_no_resource;
        goto done;
    }

    app_send_data->data_ptr = buffer;
    app_send_data->bytes = strlen(buffer);

    if (response_needed == app_false)
    {
        header.flags = CONNECTOR_DATA_RESPONSE_NOT_NEEDED;
        header.path = "test/sm_data_noresp.txt";
    }
    else
    {
        header.flags = 0;
        header.path = "test/sm_data.txt";
    }


    header.transport = app_transport;
    header.content_type = file_type;
    header.context = app_send_data; /* will be returned in all subsequent callbacks */

    status = connector_initiate_action(handle, connector_initiate_send_data, &header);
    APP_DEBUG("Status: %d, file: %s\n", status, header.path);

    if (status != connector_success)
    {
        free(app_send_data);
        app_send_data = NULL;
    }

done:
    return status;
}

connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_data_service_msg_request_t const * const put_request = request_data;
    connector_data_service_msg_response_t * const put_response = response_data;
    connector_data_service_put_request_t * const header = put_request->service_context;
    client_data_t * const app_data_ptr = (client_data_t *)header->context;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    if ((put_request == NULL) || (put_response == NULL) || (header == NULL) || (app_data_ptr == NULL))
    {
         APP_DEBUG("Invalid request_data [%p] or response_data[%p]\n", request_data, response_data);
         goto done;
    }

    if (request == connector_data_service_put_request)
    {
        switch (put_request->message_type)
        {
        case connector_data_service_type_need_data:
            {
                connector_data_service_block_t * const message = put_response->client_data;

                if (message->length_in_bytes > app_data_ptr->bytes)
                    message->length_in_bytes = app_data_ptr->bytes;

                memcpy(message->data, app_data_ptr->data_ptr, message->length_in_bytes);
                app_data_ptr->data_ptr += message->length_in_bytes;
                app_data_ptr->bytes -= message->length_in_bytes;
                if (app_data_ptr->bytes == 0)
                    message->flags = CONNECTOR_MSG_LAST_DATA;
                put_response->message_status = connector_msg_error_none;
            }
            break;

        case connector_data_service_type_have_data:
            {
                connector_data_service_block_t * const message = put_request->server_data;
                ASSERT(app_data_ptr == app_send_data);
                free(app_send_data);
                app_send_data = NULL;

                APP_DEBUG("Received %s response from server\n", ((message->flags & CONNECTOR_MSG_RESP_SUCCESS) != 0) ? "success" : "error");
                if (message->length_in_bytes > 0)
                {
                    char * const data = message->data;

                    data[message->length_in_bytes] = '\0';
                    APP_DEBUG("Server response %s\n", data);
                }
            }
            break;

        case connector_data_service_type_error:
            {
                connector_data_service_block_t * const message = put_request->server_data;
                connector_msg_error_t const * const error_value = message->data;
                APP_DEBUG("Data service error: %d\n", *error_value);

                ASSERT(app_data_ptr == app_send_data);
                free(app_send_data);
                app_send_data = NULL;
            }
            break;

        case connector_data_service_type_total_length:
            {
                size_t * const length = response_data;

                *length = app_data_ptr->bytes;
            }
            break;

        case connector_data_service_type_session_status:
            {
                connector_session_status_t * const session_status = response_data;

                ASSERT(app_data_ptr == app_send_data);
                free(app_send_data);
                app_send_data = NULL;
                APP_DEBUG("Send data complete, status[%d]\n", *session_status);
            }
            break;

        default:
            APP_DEBUG("Unexpected msg type in put request: %d\n", put_request->message_type);
            status = connector_callback_unrecognized;
            break;
        }
    }
    else
    {
        APP_DEBUG("Request not supported in this sample: %d\n", request);
        status = connector_callback_unrecognized;
    }

done:
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
        case connector_sm_opaque_response:
        {
            connector_sm_opaque_response_t * const response = response_data;

            APP_DEBUG("Received %zu opaque bytes on id %d\n", response->bytes, response->id);
            break;
        }

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}

connector_callback_status_t app_status_handler(connector_request_id_status_t const request, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;


    switch (request)
    {
        case connector_status_ping_response:
        {
            connector_message_status_response_t const * const status_response = data;

            APP_DEBUG("Received ping response [%d].\n", status_response->status);
            break;
        }

        case connector_status_ping_request:
        {
            connector_status_t * const status_request = data;

            APP_DEBUG("Received ping request.\n");
            *status_request = connector_success;
            break;
        }

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
