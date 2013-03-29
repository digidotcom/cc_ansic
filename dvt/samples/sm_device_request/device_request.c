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
#include "connector_api.h"
#include "platform.h"
#include "application.h"

/* supported target (path) names */
static char const device_request_target[] = "myTarget";
static char const device_response_data[]  = "My device response data";
static char const null_target_response[]  = "Empty target in SM device request";
static char const unexpected_target_response[] = "Unexpected target in SM device request";

typedef struct device_request_handle {
    void * device_handle;
    char const * response_data;
    char const * target;
    size_t length_in_bytes;
} device_request_handle_t;

static unsigned int device_request_active_count;

#ifdef CONNECTOR_TRANSPORT_UDP
app_bool_t app_ping_pending = app_false;

connector_status_t app_send_ping(connector_handle_t handle)
{
    connector_status_t status;
    static connector_message_status_request_t request; /* idigi connector will hold this until reply received or send completes */

    if (app_ping_pending)
    {
       static connector_message_status_request_t request;

       request.transport = connector_transport_udp;
       request.flags = 0;
       request.user_context = &app_ping_pending;

       APP_DEBUG("Previous ping pending, cancel it\n");
       status = connector_initiate_action(handle, connector_initiate_session_cancel, &request);
       if (status == connector_success)
           status = connector_service_busy;
       else
           APP_DEBUG("connector_initiate_session_cancel returned %d\n", status);
       goto done;
    }
    app_ping_pending = app_true;

    request.transport = connector_transport_udp;
    request.user_context = &app_ping_pending;
    request.flags = 0;
    status = connector_initiate_action(handle, connector_initiate_status_message, &request);
    if (status != connector_success)
        app_ping_pending = app_false;

    APP_DEBUG("Sent ping [%d].\n", status);

done:
    return status;
}
#endif

static connector_callback_status_t app_process_device_request(connector_data_service_msg_request_t const * const request_data,
                                                      connector_data_service_msg_response_t * const response_data)
{
    connector_data_service_device_request_t * const server_device_request = request_data->service_context;
    connector_data_service_block_t * const server_data = request_data->server_data;

    device_request_handle_t * client_device_request = response_data->user_context;

    if (server_data == NULL)
    {
        APP_DEBUG("app_process_device_request: server_data is NULL\n");
        goto done;
    }

    if ((server_data->flags & CONNECTOR_MSG_FIRST_DATA) == CONNECTOR_MSG_FIRST_DATA)
    {
        /* 1st chunk of device request so let's allocate memory for it
         * and setup user_context for the client_device_request.
         */
        {
            void * ptr;

            ptr = malloc(sizeof *client_device_request);
            if (ptr == NULL)
            {
                /* no memeory so cancel this request */
                APP_DEBUG("app_process_device_request: malloc fails for device request on session %p\n", server_device_request->device_handle);
                response_data->message_status = connector_msg_error_memory;
                goto done;
            }

            client_device_request = ptr;
        }
        client_device_request->length_in_bytes = 0;
        client_device_request->response_data = NULL;
        client_device_request->device_handle = server_device_request->device_handle;
        client_device_request->target = server_device_request->target;

         /* setup the user_context for our device request data */
         response_data->user_context = client_device_request;
         device_request_active_count++;
    }
    else
    {
        /* device request should be our user_context */
        if (client_device_request == NULL)
        {
            APP_DEBUG("app_process_device_request: NULL client_device_request\n");
            goto done;
        }
    }

    {
        /* prints device request data */
        char * device_request_data = server_data->data;
        if (client_device_request->target != NULL)
        {
            /* target is only available on 1st chunk of data */
            APP_DEBUG("Device request data: received data = \"%.*s\" for target = \"%s\"\n", (int)server_data->length_in_bytes,
                    device_request_data, client_device_request->target);
        }
        else
        {
            APP_DEBUG("Device request data: received data = \"%.*s\" for unknown target\n", (int)server_data->length_in_bytes,
                    device_request_data);

        }
    }

    if ((server_data->flags & CONNECTOR_MSG_LAST_DATA) == CONNECTOR_MSG_LAST_DATA)
    {   /* No more chunk */
        /* setup response data for this target */
        if ((server_data->flags & CONNECTOR_MSG_RESPONSE_NOT_NEEDED) == CONNECTOR_MSG_RESPONSE_NOT_NEEDED)
        {
            device_request_active_count--;
            free(client_device_request);
        }
        else
        {
            /* target should not be null on 1st chunk of data */
            if (client_device_request->target == NULL)
            {
                APP_DEBUG("app_process_device_request: NULL target\n");
                client_device_request->response_data = null_target_response;
            }
            else
            if (strcmp(client_device_request->target, device_request_target) != 0)
            {
                /* unsupported target so let's cancel it */
                APP_DEBUG("app_process_device_request: unsupported target %s\n", client_device_request->target);
                client_device_request->response_data = unexpected_target_response;
            }
            else
            {
                client_device_request->response_data = device_response_data;
            }
            client_device_request->length_in_bytes = strlen(client_device_request->response_data);
        }
    }

done:
    return connector_callback_continue;
}

static connector_callback_status_t app_process_device_response(connector_data_service_msg_request_t const * const request_data,
                                                       connector_data_service_msg_response_t * const response_data)
{
    device_request_handle_t * const client_device_request = response_data->user_context;

    UNUSED_ARGUMENT(request_data);

    if ((response_data->client_data == NULL) || (client_device_request == NULL))
    {
        APP_DEBUG("app_process_device_response: invalid input\n");
        goto error;
    }

    {
        connector_data_service_block_t * const client_data = response_data->client_data;
        /* get number of bytes written to the client data buffer */
        size_t const bytes = (client_device_request->length_in_bytes < client_data->length_in_bytes) ? client_device_request->length_in_bytes : client_data->length_in_bytes;

        APP_DEBUG("Device response data: send response data = %.*s\n", (int)bytes, client_device_request->response_data);

        /* let's copy the response data to service_response buffer */
        memcpy(client_data->data, client_device_request->response_data, bytes);
        client_device_request->response_data += bytes;
        client_device_request->length_in_bytes -= bytes;

        client_data->length_in_bytes = bytes;
        client_data->flags = (client_device_request->length_in_bytes == 0) ? CONNECTOR_MSG_LAST_DATA : 0;
   }

    response_data->message_status = connector_msg_error_none;

    if (client_device_request->length_in_bytes == 0)
    {   /* done */
        device_request_active_count--;
        free(client_device_request);
    }

error:
    return connector_callback_continue;
}

static connector_callback_status_t app_process_device_error(connector_data_service_msg_request_t const * const request_data,
                                                    connector_data_service_msg_response_t * const response_data)
{
    device_request_handle_t * const client_device_request = response_data->user_context;
    connector_data_service_block_t * error_data = request_data->server_data;
    connector_msg_error_t const error_code = *((connector_msg_error_t *)error_data->data);


    APP_DEBUG("app_process_device_error: handle %p error %d from server\n",
                client_device_request->device_handle, error_code);

    device_request_active_count--;
    free(client_device_request);

    return connector_callback_continue;
}

static connector_callback_status_t app_process_total_length(connector_data_service_msg_request_t const * const request_data, size_t * const total_length)
{
    device_request_handle_t * const client_device_request = request_data->service_context;

    *total_length = strlen(client_device_request->response_data);

    return connector_callback_continue;
}

static connector_callback_status_t app_process_session_status(connector_data_service_msg_request_t const * const request_data, connector_session_status_t * const status)
{
    ASSERT(request_data->service_context == &app_ping_pending);
    if (request_data->service_context == &app_ping_pending)
    {
        app_ping_pending = app_false;
        APP_DEBUG("connector_sm_send_complete status [%d]\n", *status);
    }

    return connector_callback_continue;
}

connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request,
                                                      void const * const request_data, size_t const request_length,
                                                      void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_data_service_msg_request_t const * const service_device_request = request_data;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    if (request == connector_data_service_device_request)
    {
        switch (service_device_request->message_type)
        {
        case connector_data_service_type_have_data:
            status = app_process_device_request(request_data, response_data);
            break;

        case connector_data_service_type_need_data:
            status = app_process_device_response(request_data, response_data);
            break;

        case connector_data_service_type_error:
            status = app_process_device_error(request_data, response_data);
            break;

        case connector_data_service_type_total_length:
            status = app_process_total_length(request_data, response_data);
            break;

        case connector_data_service_type_session_status:
            status = app_process_session_status(request_data, response_data);
            break;

        default:
            APP_DEBUG("app_data_service_handler: unknown message type %d for connector_data_service_device_request\n", service_device_request->message_type);
            status = connector_callback_unrecognized;
            break;
        }
    }
    else
    {
        APP_DEBUG("Unsupported %d  (Only support connector_data_service_device_request)\n", request);
        status = connector_callback_unrecognized;
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
        case connector_sm_opaque_response:
        {
            connector_sm_opaque_response_t * const response = response_data;

            APP_DEBUG("Received %zu opaque bytes on id %d\n", response->bytes, response->id);
            break;
        }

        default:
            APP_DEBUG("app_sm_handler: Request not supported in this sample: %d\n", request);
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
#ifdef CONNECTOR_TRANSPORT_UDP
        case connector_status_ping_response:
        {
            connector_message_status_response_t const * const status_response = data;
            app_ping_pending = app_false;

            APP_DEBUG("Received ping response [%d].\n", status_response->status);
            break;
        }
#endif

        case connector_status_ping_request:
        {
            connector_status_t * const status_response = data;

            APP_DEBUG("Received ping request.\n");
            *status_response = connector_success;
            break;
        }

        default:
            APP_DEBUG("app_status_handler: Request not supported in this sample: %d\n", request);
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
