/*
 * Copyright (c) 2012, 2013 Digi International Inc.,
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


connector_bool_t app_ping_pending = connector_false;

connector_status_t app_send_ping(connector_handle_t handle)
{
    connector_status_t status;
    static connector_sm_ping_request_t request; /* idigi connector will hold this until reply received or send completes */

    if (app_ping_pending)
    {
        static connector_sm_cancel_request_t cancel_request;

        cancel_request.transport = connector_transport_udp;
        cancel_request.user_context = &app_ping_pending;

        APP_DEBUG("Previous ping pending, cancel it\n");
        status = connector_initiate_action(handle, connector_initiate_session_cancel, &cancel_request);
        if (status == connector_success)
            status = connector_service_busy;
        else
            APP_DEBUG("connector_initiate_session_cancel returned %d\n", status);
        goto done;
    }
    app_ping_pending = connector_true;

    request.transport = connector_transport_udp;
    request.user_context = &app_ping_pending;
    request.response_required = connector_true;
    status = connector_initiate_action(handle, connector_initiate_ping_request, &request);
    if (status != connector_success)
        app_ping_pending = connector_false;

    APP_DEBUG("Sent ping [%d].\n", status);

done:
    return status;
}

connector_callback_status_t app_sm_handler(connector_request_id_sm_t const request, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request)
    {
        case connector_request_id_sm_ping_request:
        {
            connector_sm_ping_request_t * const ping_request = data;

            APP_DEBUG("Received ping request. response %s needed\n", ping_request->response_required ? "is" : "is not");
            break;
        }

        case connector_request_id_sm_ping_response:
        {
            connector_sm_ping_response_t * const ping_resp = data;

            if (ping_resp->status == connector_sm_ping_status_success)
                app_ping_pending = connector_false;

            APP_DEBUG("Received ping response [%d].\n", ping_resp->status);
            break;
        }

        case connector_request_id_sm_opaque_response:
        {
            connector_sm_opaque_response_t * const response = data;

            APP_DEBUG("Received %zu opaque bytes on id %d\n", response->bytes_used, response->id);
            break;
        }

        case connector_request_id_sm_more_data:
        {
            connector_sm_more_data_t * const more_data = data;

            APP_DEBUG("More SM data is waiting on %s in Etherios Device Cloud\n", (more_data->transport == connector_transport_udp) ? "UDP" : "SMS");
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
        case connector_request_id_status_tcp:
            status = app_tcp_status(data);
            break;

        case connector_request_id_status_stop_completed:
            APP_DEBUG("connector_restore_keepalive\n");
            break;

        default:
            APP_DEBUG("Status request not supported: %d\n", request);
            break;
    }

    return status;
}
