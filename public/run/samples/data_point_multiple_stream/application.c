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
#include "application.h"

extern connector_callback_status_t app_data_point_handler(connector_request_id_data_point_t const request, void  * const data);
extern void app_update_point(size_t stream_index, void * const points, size_t const index);
extern connector_status_t app_send_data_point(connector_handle_t handle, void * const points);
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

int application_run(connector_handle_t handle)
{
    connector_status_t status = connector_success;
    size_t const points_per_message = APP_NUM_STREAMS * APP_NUM_POINTS_PER_STREAM;
    void * const points = app_allocate_data_points(points_per_message);
    size_t current_point_index = 0;
    size_t busy_count = 0;

    if (points == NULL)  goto error;

    for(;;)
    {
        int const sample_interval_in_seconds = 2;

        if (status == connector_success)
        {
            /* Collect a sample for each stream */
            for(size_t stream_index = 0; stream_index < APP_NUM_STREAMS; stream_index++)
                app_update_point(stream_index, points, current_point_index++);
        }

        if (current_point_index == points_per_message)
        {
            /* Now is time to send all collected samples to Device Cloud */
            status = app_send_data_point(handle, points);

            switch (status)
            {
                case connector_init_error:
                case connector_service_busy:
                case connector_unavailable:
                {
                    int const point_delay_in_seconds = 4;

                    if (++busy_count > points_per_message) goto done;
                    APP_DEBUG(".");
                    sleep(point_delay_in_seconds);
                    break;
                }

                case connector_success:
                    current_point_index = 0;
                    busy_count = 0;
                    break;

                default:
                    APP_DEBUG("Failed to send data point. status:%d\n", status);
                    goto done;
            }

            sleep(sample_interval_in_seconds);
        }
    }

done:
    app_free_data_points(points);

error:
    APP_DEBUG("Data point sample is exited!\n");
    return 1;
}
