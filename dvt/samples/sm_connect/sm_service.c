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
    status = connector_initiate_action(handle, connector_initiate_status_message, &request, NULL);
    if (status != connector_success)
        app_ping_pending = app_false;

    APP_DEBUG("Sent ping [%d].\n", status);

done:
    return status;
}
#endif

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
            status = connector_callback_unrecognized;
            APP_DEBUG("app_sm_handler: Request not supported in this sample: %d\n", request);
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
            app_ping_pending = app_false;

            APP_DEBUG("Received ping response [%d].\n", status_response->status);
            break;
        }

        case connector_status_ping_request:
        {
            connector_session_status_t * const status_request = response_data;

            APP_DEBUG("Received ping request.\n");
            *status_request = connector_session_status_success;
            break;
        }

        case connector_status_tcp:
            status = app_tcp_status(request_data);
            break;

        case connector_status_stop_completed:
            APP_DEBUG("connector_restore_keepalive\n");
            break;

        default:
            status = connector_callback_unrecognized;
            APP_DEBUG("app_status_handler: Request not supported in this sample: %d\n", request);
            break;
    }

    return status;
}
