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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "connector_api.h"
#include "platform.h"
#include "application.h"

connector_bool_t app_connector_reconnect(connector_class_id_t const class_id, connector_close_status_t const status)
{
    UNUSED_ARGUMENT(class_id);

    connector_bool_t type;

    switch (status)
    {
           /* if either the server or our application cuts the connection, don't reconnect */
        case connector_close_status_device_terminated:
        case connector_close_status_device_stopped:
        case connector_close_status_abort:
             type = connector_false;
             break;

       /* otherwise it's an error and we want to retry */
       default:
             type = connector_true;
             break;
    }

    return type;
}


connector_callback_status_t app_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id,
                                    void const * const request_data, size_t const request_length,
                                    void * response_data, size_t * const response_length)
{
    connector_callback_status_t   status = connector_callback_continue;

    switch (class_id)
    {
    case connector_class_config:
        status = app_config_handler(request_id.config_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_operating_system:
        status = app_os_handler(request_id.os_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_network_udp:
        status = app_network_udp_handler(request_id.network_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_data_service:
        status = app_data_service_handler(request_id.data_service_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_short_message:
        status = app_sm_handler(request_id.sm_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_status:
        status = app_status_handler(request_id.status_request, request_data, request_length, response_data, response_length);
        break;

    default:
        status = connector_callback_unrecognized;
        /* not supported */
        break;
    }

    return status;
}

int application_run(connector_handle_t handle)
{
    int return_status = 0;

#ifdef CONNECTOR_TRANSPORT_UDP

    unsigned int const ping_interval_in_seconds = 10;

    while (return_status == 0)
    {
        connector_status_t const status = app_send_ping(handle);

        switch (status)
        {
            case connector_init_error:
            case connector_service_busy:
            case connector_unavailable:
                sleep(2);
                break;

        case connector_success:
                sleep(2);
                if (!app_ping_pending)
                     sleep(ping_interval_in_seconds - 2);
                break;

            default:
                return_status = 1;
                break;
        }
    }
#else
    UNUSED_ARGUMENT(handle);
#endif
    return return_status;
}

