/*
 * Copyright (c) 2014 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

#include "connector_dev_health_structs.h"
#include "connector_dev_health_process.h"

static char const dev_health_path[] = "DataPoint/metrics.csv";
static size_t const dev_health_path_strlen = sizeof dev_health_path - 1;

typedef struct {
    char const * p_csv;
    unsigned int bytes_available;
    connector_request_data_service_send_t send_request;
} dev_health_data_push_t;

STATIC connector_status_t dev_health_send_metrics(connector_data_t * const connector_ptr)
{
    connector_status_t status;
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    dev_health_data_push_t * dev_health_data_push = NULL;
    connector_request_data_service_send_t * send_request = NULL;
    char const * const next_header = strstr(dev_health_info->csv.next_header + sizeof csv_header - 1, csv_header);

    status = malloc_data(connector_ptr, sizeof *dev_health_data_push, (void * *)&dev_health_data_push);
    ASSERT_GOTO(status == connector_working, done);

    dev_health_data_push->p_csv = dev_health_info->csv.next_header;
    dev_health_data_push->bytes_available = next_header != NULL ? next_header - dev_health_info->csv.next_header : (unsigned int)strlen(dev_health_data_push->p_csv);
    send_request = &dev_health_data_push->send_request;

    send_request->user_context = dev_health_data_push;
    send_request->content_type = "";
    send_request->option = connector_data_service_send_option_overwrite;
    send_request->path = dev_health_path;
    send_request->request_id = NULL;
    send_request->timeout_in_seconds = 0;
    send_request->transport = connector_transport_tcp;

    status = connector_initiate_action(connector_ptr, connector_initiate_send_data, send_request);
    dev_health_info->csv.next_header = next_header;
done:
    if (status != connector_success)
    {
        if (dev_health_data_push != NULL)
        {
            free_data(connector_ptr, dev_health_data_push);
        }
    }
    return status;
}

STATIC connector_status_t connector_dev_health_step(connector_data_t * const connector_ptr)
{
    unsigned long now;
    connector_status_t status;
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;

    status = get_system_time(connector_ptr, &now);
    if (status != connector_working)
    {
        connector_debug_line("Error while getting system uptime");
        goto done;
    }

    switch (dev_health_info->csv.status)
    {
        case DEV_HEALTH_CSV_STATUS_PROCESSING:
        {
#if !(defined DEVICE_HEALTH_FIRST_REPORT_AT)
#define DEVICE_HEALTH_FIRST_REPORT_AT    0
#endif
            dev_health_root_t root_group;

            if (now >= connector_ptr->dev_health.last_check)
            {
                goto done;
            }
            connector_ptr->dev_health.last_check = now;

            if (connector_ptr->dev_health.info.csv.data == NULL)
            {
                dev_health_allocate_csv_data(connector_ptr);
            }

            for (root_group = 0; root_group < dev_health_root_COUNT; root_group++)
            {
                static const char * paths[] = {"eth", "mobile", "sys"};
                dev_health_simple_metric_t * item = NULL;
                unsigned long const seconds_in_a_minute = 60;
                unsigned long const reporting_interval = connector_ptr->dev_health.simple_metrics.config.report_rate * seconds_in_a_minute;
                unsigned long * const sample_at = &connector_ptr->dev_health.simple_metrics.sample_at[root_group];
                unsigned long * const report_at = &connector_ptr->dev_health.simple_metrics.report_at;
                unsigned long sampling_interval;

                if (reporting_interval == 0)
                {
                    break;
                }

                switch (root_group)
                {
                    case dev_health_root_eth:
                        item = &connector_ptr->dev_health.simple_metrics.config.eth;
                        break;
                    case dev_health_root_mobile:
                        item = &connector_ptr->dev_health.simple_metrics.config.mobile;
                        break;
                    case dev_health_root_sys:
                        item = &connector_ptr->dev_health.simple_metrics.config.sys;
                        break;
                    case dev_health_root_COUNT:
                        ASSERT_GOTO(root_group < dev_health_root_COUNT, done);
                        break;
                }

                sampling_interval = item->sample_rate;

                if (item->metrics == connector_false || item->sample_rate == 0)
                {
                    continue;
                }

                if (*sample_at == 0)
                {
               		*sample_at = DEVICE_HEALTH_FIRST_REPORT_AT;
                }

                if (*report_at == 0)
                {
                	*report_at = DEVICE_HEALTH_FIRST_REPORT_AT;
                }

                if (now >= *sample_at)
                {
                    dev_health_process_path(connector_ptr, paths[root_group]);
                    *sample_at = now + sampling_interval;
                }

                if (now >= *report_at)
                {
                    dev_health_info->csv.status = DEV_HEALTH_CSV_STATUS_READY_TO_SEND;
                    *report_at = now + reporting_interval;
                }
            }

            if (dev_health_info->csv.status == DEV_HEALTH_CSV_STATUS_READY_TO_SEND)
            {
                dev_health_info->csv.next_header = dev_health_info->csv.data;
            }
            break;
        }
        case DEV_HEALTH_CSV_STATUS_READY_TO_SEND:
        {
            connector_status_t const status =  dev_health_send_metrics(connector_ptr);

            if (status == connector_success)
            {
                dev_health_info->csv.status = DEV_HEALTH_CSV_STATUS_SENDING;
            }
            else
            {
                connector_debug_line("Enhanced Services: couldn't send metrics, reason %d, retrying", status);
            }
            break;
        }
        case DEV_HEALTH_CSV_STATUS_SENDING:
            break;
        case DEV_HEALTH_CSV_STATUS_SENT:
            if (dev_health_info->csv.next_header != NULL)
            {
                dev_health_info->csv.status = DEV_HEALTH_CSV_STATUS_READY_TO_SEND;
            }
            else
            {
                dev_health_reset_csv_data(&connector_ptr->dev_health.info);
                connector_ptr->dev_health.info.csv.status = DEV_HEALTH_CSV_STATUS_PROCESSING;
            }
            break;
    }

done:
    return status;
}

STATIC connector_callback_status_t dev_health_handle_data_callback(connector_data_service_send_data_t * const data_ptr)
{
    connector_callback_status_t status = connector_callback_error;
    dev_health_data_push_t * const dev_health_data_push = data_ptr->user_context;
    unsigned int bytes_used;

    ASSERT_GOTO(dev_health_data_push != NULL, error);

    bytes_used = MIN_VALUE(data_ptr->bytes_available, dev_health_data_push->bytes_available);

    memcpy(data_ptr->buffer, dev_health_data_push->p_csv, bytes_used);
    data_ptr->bytes_used = bytes_used;
    data_ptr->more_data = connector_bool(dev_health_data_push->bytes_available);

    dev_health_data_push->p_csv += bytes_used;
    dev_health_data_push->bytes_available -= bytes_used;

    status = connector_callback_continue;

error:
    return status;
}

STATIC connector_callback_status_t dev_health_handle_response_callback(connector_data_service_send_response_t * const data_ptr)
{
    connector_callback_status_t const status = connector_callback_continue;

    connector_debug_line("dev_health_handle_response_callback, response %d '%s'", data_ptr->response, data_ptr->hint);
    return status;
}

STATIC connector_callback_status_t dev_health_handle_status_callback(connector_data_t * const connector_ptr, connector_data_service_status_t * const data_ptr)
{
    connector_callback_status_t const status = connector_callback_continue;
    dev_health_data_push_t * const dev_health_data_push = data_ptr->user_context;

    connector_debug_line("dev_health_handle_status_callback, status %d", data_ptr->status);
    free_data(connector_ptr, dev_health_data_push);
    connector_ptr->dev_health.info.csv.status = DEV_HEALTH_CSV_STATUS_SENT;

    return status;
}


#if (defined CONNECTOR_SHORT_MESSAGE)
STATIC connector_callback_status_t dev_health_handle_length_callback(connector_data_service_length_t * const data_ptr)
{
    connector_callback_status_t status = connector_callback_abort;
    dev_health_data_push_t * const dev_health = data_ptr->user_context;
    ASSERT_GOTO(dev_health != NULL, error);

    data_ptr->total_bytes = dev_health->bytes_available;
    status = connector_callback_continue;
error:
    return status;
}
#endif

STATIC connector_callback_status_t dev_health_handle_callback(connector_data_t * const connector_ptr, connector_request_id_data_service_t const ds_request_id, void * const data)
{
    connector_callback_status_t status;

    switch (ds_request_id)
    {
        case connector_request_id_data_service_send_data:
            status = dev_health_handle_data_callback(data);
            break;

        case connector_request_id_data_service_send_response:
            status = dev_health_handle_response_callback(data);
            break;

        case connector_request_id_data_service_send_status:
            status = dev_health_handle_status_callback(connector_ptr, data);
            break;

        #if (defined CONNECTOR_SHORT_MESSAGE)
        case connector_request_id_data_service_send_length:
            status = dev_health_handle_length_callback(data);
            break;
        #endif

        default:
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
