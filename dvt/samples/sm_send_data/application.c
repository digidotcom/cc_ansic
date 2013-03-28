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

connector_auto_connect_type_t app_connector_reconnect(connector_class_id_t const class_id, connector_close_status_t const status)
{
    UNUSED_ARGUMENT(class_id);

    connector_auto_connect_type_t type;

    switch (status)
    {
           /* if either the server or our application cuts the connection, don't reconnect */
        case connector_close_status_device_terminated:
        case connector_close_status_device_stopped:
        case connector_close_status_abort:
             type = connector_manual_connect;
             break;

       /* otherwise it's an error and we want to retry */
       default:
             type = connector_auto_connect;
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
    const unsigned int send_cnt_max = 2;
    const unsigned int sleep_sec_busy = 2;
    const unsigned int app_response_timeout = 5;

    app_bool_t response_needed = app_true;
    unsigned int send_cnt = 0;

    for (send_cnt = 0; send_cnt < send_cnt_max; ) 
    {
        connector_status_t status = app_send_put_request(handle, response_needed);
        switch(status) 
        {
            case connector_success:
            {
                unsigned int i;
                for (i = 0; i < app_response_timeout; i++)
                {
                    sleep(1);
                    if (app_put_request_complete())
                        break;
                }
                if (app_put_request_complete())
                {
                    response_needed = !response_needed;
                    send_cnt++;
                }
                else
                {
                    app_cancel_put_request(handle);
                    sleep(sleep_sec_busy);
                }
                break;
             }
    
            case connector_init_error:
            case connector_service_busy:
            case connector_unavailable:
                sleep(sleep_sec_busy);
                break;
    
            default:
                APP_DEBUG("Send data failed [%d]\n", status);
                goto done;
        }
    }

#ifdef APP_USE_SM_UDP 
    for (;;) 
    {
        const unsigned int sleep_sec_ping = 10;
        connector_status_t status = app_send_ping(handle);
        switch (status) 
        {
            case connector_service_busy:
                sleep(sleep_sec_busy);
                break;

            case connector_success:
                sleep(sleep_sec_ping);
                break;
       
            default:
                APP_DEBUG("Send data failed [%d]\n", status);
                goto done;
        }
    }
#endif

done:
    return 1;
}

