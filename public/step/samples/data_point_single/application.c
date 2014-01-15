/*
 * Copyright (c) 2013 Digi International Inc.,
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

extern connector_callback_status_t app_data_point_handler(connector_request_id_data_point_t const request, void  * const data);
extern connector_status_t app_send_data_point(connector_handle_t handle, void * const points, size_t const index);
extern void * app_allocate_data_points(size_t const points_count);
extern void app_free_data_points(void * points);

connector_bool_t app_connector_reconnect(connector_class_id_t const class_id, connector_close_status_t const status)
{
    connector_bool_t type;

    UNUSED_ARGUMENT(class_id);

    switch (status)
    {
           /* if either Device Cloud or our application cuts the connection, don't reconnect */
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

connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
                                                   connector_request_id_t const request_id,
                                                   void * const data)
{
    connector_callback_status_t   status = connector_callback_unrecognized;

    switch (class_id)
    {
    case connector_class_id_config:
        status = app_config_handler(request_id.config_request, data);
        break;

    case connector_class_id_operating_system:
        status = app_os_handler(request_id.os_request, data);
        break;

#if (defined CONNECTOR_TRANSPORT_TCP)
    case connector_class_id_network_tcp:
        status = app_network_tcp_handler(request_id.network_request, data);
        break;
#endif

    case connector_class_id_data_point:
        status = app_data_point_handler(request_id.data_point_request, data);
        break;

    case connector_class_id_status:
        status = app_status_handler(request_id.status_request, data);
        break;

    default:
        status = connector_callback_unrecognized;
        /* not supported */
        break;
    }

    return status;
}

int application_step(connector_handle_t handle)
{
    int result = 0;
    size_t const points_per_message = 5;
    static void * points = NULL;

    if (points == NULL)
    {
        points = app_allocate_data_points(points_per_message);
        if (points == NULL) goto error;
    }

    {
        time_t const point_interval_in_seconds = 2;
        static time_t last_time = 0;
        time_t current_time = time(NULL);

        if ((current_time - last_time) < point_interval_in_seconds)
            goto done;

        last_time = current_time;
    }

    {
        static size_t points_sent = 0;
        static size_t busy_count = 0;
        size_t const current_index = points_sent % points_per_message;
        connector_status_t const status = app_send_data_point(handle, points, current_index);

        switch (status)
        {
            case connector_init_error:
            case connector_service_busy:
            case connector_unavailable:
                if (++busy_count < points_per_message) goto done;
                break;

            case connector_success:
                points_sent++;
                busy_count = 0;
                goto done;

            default:
                APP_DEBUG("Failed to send data point:%" PRIsize ", status:%d\n", points_sent, status);
                break;
        }
    }

error:
    APP_DEBUG("Data point sample error\n");

done:
    if ((result != 0) && (points != NULL))
    {
        app_free_data_points(points);
        points = NULL;
    }
    return result;
}
