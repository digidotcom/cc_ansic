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

typedef struct dev_health_metrics dev_health_metrics_t;

STATIC connector_status_t dev_health_send_metrics(connector_data_t * const connector_ptr)
{
    connector_status_t status;
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    dev_health_data_push_t * dev_health_data_push = NULL;
    connector_request_data_service_send_t * send_request = NULL;
    unsigned int const csv_len = dev_health_info->csv.total_size - dev_health_info->csv.free_bytes - 1; /* Skip the last '\n' */

    status = malloc_data(connector_ptr, sizeof *dev_health_data_push, (void * *)&dev_health_data_push);
    ASSERT_GOTO(status == connector_working, done);

    dev_health_data_push->p_csv = dev_health_info->csv.data;
    dev_health_data_push->bytes_available = csv_len;
    send_request = &dev_health_data_push->send_request;

    send_request->user_context = dev_health_data_push;
    send_request->content_type = "";
    send_request->option = connector_data_service_send_option_overwrite;
    send_request->path = dev_health_path;
    send_request->request_id = NULL;
    send_request->timeout_in_seconds = 0;
    send_request->transport = connector_transport_tcp;

    status = connector_initiate_action(connector_ptr, connector_initiate_send_data, send_request);

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

    if (connector_ptr->dev_health.info.csv.data == NULL)
    {
        dev_health_setup_csv_data(connector_ptr);
    }

    status = get_system_time(connector_ptr, &now);
    if (status != connector_working)
    {
        connector_debug_line("Error while getting system uptime");
        goto done;
    }

    if (now == connector_ptr->dev_health.last_check)
    {
        goto done;
    }
    connector_ptr->dev_health.last_check = now;

    switch (dev_health_info->csv.status)
    {
        case DEV_HEALTH_CSV_STATUS_PROCESSING:
        {
            size_t i;

            for (i = 0; i < asizeof(connector_ptr->dev_health.metrics); i++)
            {
                dev_health_metrics_t * const item = &connector_ptr->dev_health.metrics[i];
                unsigned int const seconds_in_a_minute = 60;
                unsigned int const sampling_interval = item->sampling_interval;
                unsigned int const reporting_interval = item->reporting_interval * seconds_in_a_minute;

                if (item->path[0] == '\0')
                {
                    continue;
                }

                if (item->sample_at == 0)
                {
                    item->sample_at = now + sampling_interval;
                }

                if (item->report_at == 0)
                {
                    item->report_at = now + reporting_interval;
                }

                if (now >= item->sample_at)
                {
                    dev_health_process_path(connector_ptr, item->path);
                    item->sample_at = now + sampling_interval;
                }

                if (now >= item->report_at)
                {
                    dev_health_info->csv.status = DEV_HEALTH_CSV_STATUS_READY_TO_SEND;
                    item->report_at = now + reporting_interval;
                }
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
                connector_debug_line("Enhanced Services: couldn't send metrics, reasong %d, retrying", status);
            }
            break;
        }
        case DEV_HEALTH_CSV_STATUS_SENDING:
            break;
        case DEV_HEALTH_CSV_STATUS_SENT:
            dev_health_teardown_csv_data(connector_ptr);
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

STATIC connector_callback_status_t dev_health_handle_response_callback(connector_data_t * const connector_ptr, connector_data_service_send_response_t * const data_ptr)
{
    connector_callback_status_t const status = connector_callback_continue;

    UNUSED_PARAMETER(connector_ptr);
    connector_debug_line("dev_health_handle_response_callback, response %d", data_ptr->response);
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
STATIC connector_callback_status_t dev_health_handle_length_callback(connector_data_t * const connector_ptr, connector_data_service_length_t * const data_ptr)
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
            status = dev_health_handle_response_callback(connector_ptr, data);
            break;

        case connector_request_id_data_service_send_status:
            status = dev_health_handle_status_callback(connector_ptr, data);
            break;

        #if (defined CONNECTOR_SHORT_MESSAGE)
        case connector_request_id_data_service_send_length:
            status = dev_health_handle_length_callback(connector_ptr, data);
            break;
        #endif

        default:
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
