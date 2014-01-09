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
#include <time.h>
#include <unistd.h>
#include "connector_api.h"
#include "platform.h"
#include "application.h"

static connector_bool_t data_points_valid = connector_false;

void * app_allocate_data_points(size_t const points_count)
{
    connector_request_data_point_multiple_t * dp_ptr = malloc(sizeof *dp_ptr);

    if (dp_ptr != NULL)
    {
        size_t index;
        dp_ptr->point = malloc(points_count * sizeof *dp_ptr->point);

        if (dp_ptr->point != NULL)
        {
            connector_data_point_t * point = dp_ptr->point;

            for (index = 1; index < points_count; index++)
            {
                point->next = point + 1;
                point++;
            }

            point->next = NULL;
 
            dp_ptr->user_context = dp_ptr;
            dp_ptr->transport = connector_transport_tcp;

            data_points_valid = connector_true;
        }
        else
        {
            free(dp_ptr);
            dp_ptr = NULL;
        }
    }

    return dp_ptr;
}

void app_free_data_points(connector_request_data_point_multiple_t * dp_ptr)
{
    if (dp_ptr != NULL)
    {
        if (dp_ptr->point != NULL)
            free(dp_ptr->point);

        free(dp_ptr);
    }

    data_points_valid = connector_false;
}

typedef struct
{
    unsigned long user;
    unsigned long nice;
    unsigned long system;
    unsigned long idle;
    unsigned long iowait;
} stat_cpu_t;

static connector_bool_t get_cpu_stat(stat_cpu_t * const stat)
{
    connector_bool_t success = connector_false;
    char stat_path[] = "/proc/stat";
    FILE * file_ptr = NULL;

    file_ptr = fopen(stat_path, "r");
    if (file_ptr != NULL)
    {
        if (fscanf(file_ptr, "cpu %lu %lu %lu %lu %lu", &stat->user, &stat->nice, &stat->system, &stat->idle, &stat->iowait) == 5)
            success = connector_true;
    }

    if (file_ptr != NULL)
        fclose(file_ptr);

    return success;
}

static int app_get_cpu_usage(void)
{
    int cpu_usage;
    static stat_cpu_t last_stat;
    stat_cpu_t current_stat;

    if (get_cpu_stat(&current_stat))
    {
        static connector_bool_t first_time = connector_true;

        if (first_time)
        {
            first_time = connector_false;
            cpu_usage = 0;
        }
        else
        {
            int const user_time = (int)(current_stat.user - last_stat.user);
            int const nice_time = (int)(current_stat.nice - last_stat.nice);
            int const system_time = (int)(current_stat.system - last_stat.system);
            int const iowait_time = (int)(current_stat.iowait - last_stat.iowait);
            int const idle_time = (int)(current_stat.idle - last_stat.idle);
            int const active_time = user_time + nice_time + system_time + iowait_time;
            int const total_time = active_time + idle_time;

            cpu_usage = (total_time > 0) ? (active_time * 100)/total_time : 0;
        }

        last_stat = current_stat; /* use memcpy on the platform where this doesn't work */
    }
    else
    {
        static connector_bool_t first_time = connector_true;

        if (first_time)
        {
            APP_DEBUG("Failed to get cpu usage, using random value...\n");
            first_time = connector_false;
        }

        cpu_usage = rand() % 100;
    }

    return cpu_usage;
}

static float app_get_cpu_temperature(void)
{
    float cpu_temperature;

    /* Use a random float value in 0.0 to 99.99 range */
    cpu_temperature = (float)drand48();

    cpu_temperature *= 100;

    return cpu_temperature;
}


static float app_get_incremental(void)
{
    static int incremental = 0;

    return incremental++;
}

void app_update_point(size_t stream_index, connector_request_data_point_multiple_t * const dp_ptr, size_t const index)
{
    connector_data_point_t * const point = dp_ptr->point + index;

    {
        time_t const current_time = time(NULL);

        point->time.source = connector_time_local_epoch_fractional;
        point->time.value.since_epoch_fractional.seconds = current_time;
        point->time.value.since_epoch_fractional.milliseconds = 0;
    }

    {
        static char latitude_str[] = "42.32";
        static char longitude_str[] = "71.23";
        static char elevation_str[] = "63.95";

        point->location.type = connector_location_type_text;
        point->location.value.text.latitude = latitude_str;
        point->location.value.text.longitude = longitude_str;
        point->location.value.text.elevation = elevation_str;
    }

    point->quality.type = connector_quality_type_ignore;

    {

        static char * DataStreams[APP_NUM_STREAMS] = {"cpu_usage", "cpu_temp", "incremental"};
        static char * DataStreams_description[APP_NUM_STREAMS] = {"CPU usage", "CPU temperature (random)", "Just an incremental count"};
		static char * DataStreams_units[APP_NUM_STREAMS] = {"%", "Celsius degrees", "Counts"};
	    //static char * DataStreams_forward[APP_NUM_STREAMS] = {"steam1_fw_a,steam1_fw_b", "steam2_fw", "steam3_fw"};
		static connector_data_point_type_t DataStreams_type[APP_NUM_STREAMS] = { connector_data_point_type_integer, connector_data_point_type_float, connector_data_point_type_integer };

        ASSERT(stream_index < APP_NUM_STREAMS);

        point->stream_id = DataStreams[stream_index];
        point->description = DataStreams_description[stream_index];
        point->unit = DataStreams_units[stream_index];
		//point->forward_to = DataStreams_forward[stream_index];
        point->type = DataStreams_type[stream_index];

        point->data.type = connector_data_type_native;

        switch (stream_index)
        {
            case 0:
                point->data.element.native.int_value = app_get_cpu_usage();
                break;

            case 1:
                point->data.element.native.float_value = app_get_cpu_temperature();
                break;

            case 2:
                point->data.element.native.int_value = app_get_incremental();
                break;
            
			default:
                ASSERT(0);
                break;
        }
    }
}

static connector_bool_t app_dp_waiting_for_response = connector_false;

connector_status_t app_send_data_point(connector_handle_t const handle, connector_request_data_point_multiple_t * const dp_ptr)
{
    connector_status_t status = connector_success;

    if (app_dp_waiting_for_response)
    {
        APP_DEBUG("Data point app: waiting for a response...\n");
        status = connector_service_busy;
        goto done;
    }

    app_dp_waiting_for_response = connector_true;
    status = connector_initiate_action(handle, connector_initiate_data_point_multiple, dp_ptr);
    APP_DEBUG("Data point message sent, status[%d]\n", status);
    if (status != connector_success)
        app_dp_waiting_for_response = connector_false;

done:
    return status;
}

connector_callback_status_t app_data_point_handler(connector_request_id_data_point_t const request_id, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    if (!data_points_valid)
    {
        APP_DEBUG("Error: Response received [%d], but application timed out\n", request_id);
        goto error;
    }

    switch (request_id)
    {
        case connector_request_id_data_point_multiple_response:
        {
            connector_data_point_response_t * const resp_ptr = data;
            connector_request_data_point_multiple_t * const dp_ptr = resp_ptr->user_context;

            if (dp_ptr == NULL)
            {
                APP_DEBUG("Error: Received null context in data point response\n");
                status = connector_callback_error;
                goto error;
            }

            app_dp_waiting_for_response = connector_false;
            APP_DEBUG("Received data point response [%d]\n", resp_ptr->response);
            if (resp_ptr->hint != NULL)
            {
                APP_DEBUG("Hint: %s\n", resp_ptr->hint);
            }
            break;
        }

        case connector_request_id_data_point_multiple_status:
        {
            connector_data_point_status_t * const status_ptr = data;
            connector_request_data_point_multiple_t * const dp_ptr = status_ptr->user_context;

            if (dp_ptr == NULL)
            {
                APP_DEBUG("Error: Received null context in data point status\n");
                status = connector_callback_error;
                goto error;
            }

            app_dp_waiting_for_response = connector_false;
            APP_DEBUG("Received data point status [%d]\n", status_ptr->status);
            break;
        }

        default:
            APP_DEBUG("Data point callback: Request not supported: %d\n", request_id);
            status = connector_callback_unrecognized;
            break;
    }

error:
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
            APP_DEBUG("Status request not supported in sm_udp: %d\n", request);
            break;
    }

    return status;
}

