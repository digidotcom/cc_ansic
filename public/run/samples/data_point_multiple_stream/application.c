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
extern void app_update_stream(void * const buffer, size_t const stream_index);
extern void app_update_point(void * const buffer, size_t const stream_index, size_t const point_index);
extern connector_status_t app_send_data(connector_handle_t handle, void * const buffer);
extern void * app_allocate_data(size_t const stream_count, size_t const point_count);
extern void app_free_data(void * buffer, size_t const stream_count);

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
    void * const buffer = app_allocate_data(APP_NUM_STREAMS, APP_NUM_POINTS_PER_STREAM);
    size_t point_index = 0;
    size_t busy_count = 0;
    size_t stream_idx = 0;

    if (buffer == NULL)  goto error;

    for (stream_idx = 0; stream_idx < APP_NUM_STREAMS; stream_idx++)
        app_update_stream(buffer, stream_idx);

    for(;;)
    {
        int const sample_interval_in_seconds = 2;

        if (status == connector_success)
        {
            size_t stream_index;

            /* Collect a sample for each stream */
            for(stream_index = 0; stream_index < APP_NUM_STREAMS; stream_index++)
                app_update_point(buffer, stream_index, point_index);
            point_index++;
        }

        if (point_index == APP_NUM_POINTS_PER_STREAM)
        {
            /* Now is time to send all collected samples to Device Cloud */
            status = app_send_data(handle, buffer);

            switch (status)
            {
                case connector_init_error:
                case connector_service_busy:
                case connector_unavailable:
                {
                    int const point_delay_in_seconds = 4;

                    if (++busy_count > APP_NUM_POINTS_PER_STREAM) goto done;
                    APP_DEBUG(".");
                    sleep(point_delay_in_seconds);
                    break;
                }

                case connector_success:
                    point_index = 0;
                    busy_count = 0;
                    break;

                default:
                    APP_DEBUG("Failed to send data point multiple. status:%d\n", status);
                    goto done;
            }
        }
        
        sleep(sample_interval_in_seconds);
    }

done:
    app_free_data(buffer, APP_NUM_STREAMS);

error:
    APP_DEBUG("Data point multiple sample is exited!\n");
    return 1;
}
