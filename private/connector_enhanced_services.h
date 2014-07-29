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

#include "connector_enhanced_services_structs.h"
#include "connector_enhanced_services_process.h"

static char const enhs_path[] = "DataPoint/metrics.csv";
static unsigned int const enhs_path_strlen = sizeof enhs_path - 1;

typedef struct {
    char const * p_csv;
    unsigned int bytes_available;
    connector_request_data_service_send_t const * send_request;
} enhs_data_push_t;

typedef struct enhs_metrics enhs_metrics_t;

STATIC connector_status_t enhs_send_metrics(connector_data_t * const connector_ptr)
{
    connector_status_t status;
    enhs_info_t * const enhs_info = &connector_ptr->enhs.info;
    connector_request_data_service_send_t * send_request = NULL;
    enhs_data_push_t * enhs_data_push = NULL;
    unsigned int const csv_len = enhs_info->csv.total_size - enhs_info->csv.free_bytes - 1; /* Skip the last '\n' */

    status = malloc_data(connector_ptr, sizeof *send_request, (void * *)&send_request);
    ASSERT_GOTO(status == connector_working, done);
    status = malloc_data(connector_ptr, sizeof *enhs_data_push, (void * *)&enhs_data_push);
    ASSERT_GOTO(status == connector_working, done);

    enhs_data_push->p_csv = enhs_info->csv.data;
    enhs_data_push->bytes_available = csv_len;
    enhs_data_push->send_request = send_request;

    send_request->user_context = enhs_data_push;
    send_request->content_type = "";
    send_request->option = connector_data_service_send_option_overwrite;
    send_request->path = enhs_path;
    send_request->request_id = NULL;
    send_request->timeout_in_seconds = 0;
    send_request->transport = connector_transport_tcp;

    status = connector_initiate_action(connector_ptr, connector_initiate_send_data, send_request);

done:
    if (status != connector_success)
    {
        if (send_request != NULL)
        {
            free_data(connector_ptr, send_request);
        }
        if (enhs_data_push != NULL)
        {
            free_data(connector_ptr, send_request);
        }
    }
    return status;
}

STATIC connector_status_t connector_enhs_step(connector_data_t * const connector_ptr)
{
    unsigned long now;
    connector_status_t status;
    enhs_info_t * const enhs_info = &connector_ptr->enhs.info;

    if (connector_ptr->enhs.info.csv.data == NULL)
    {
        enhs_setup_csv_data(connector_ptr);
    }

    status = get_system_time(connector_ptr, &now);
    if (status != connector_working)
    {
        connector_debug_line("Error while getting system uptime");
        goto error;
    }

    switch (enhs_info->csv.status)
    {
        case ENHS_CSV_STATUS_PROCESSING:
        {
            int i;

            for (i = 0; i < asizeof(connector_ptr->enhs.metrics); i++)
            {
                enhs_metrics_t * const item = &connector_ptr->enhs.metrics[i];

                if (item->path[0] == '\0')
                {
                    continue;
                }

                if (item->sample_at == 0)
                {
                    item->sample_at = now + item->sampling_interval;
                }

                if (item->report_at == 0)
                {
                    item->report_at = now + item->reporting_interval * 60;
                }

                if (now >= item->sample_at)
                {
                    enhs_process_path(connector_ptr, item->path);
                    item->sample_at = now + item->sampling_interval;
                }

                if (now >= item->report_at)
                {
                    enhs_info->csv.status = ENHS_CSV_STATUS_READY_TO_SEND;
                    item->report_at = now + item->reporting_interval * 60;
                }
            }
            break;
        }
        case ENHS_CSV_STATUS_READY_TO_SEND:
        {
            connector_status_t const status =  enhs_send_metrics(connector_ptr);

            if (status == connector_success)
            {
                enhs_info->csv.status = ENHS_CSV_STATUS_SENDING;
                connector_debug_line("Enhanced services, pushing CSV:*%s*\n", enhs_info->csv.data); /* TODO: remove this line */
            }
            else
            {
                connector_debug_line("Enhanced Services: couldn't send metrics, reasong %d, retrying", status);
            }
            break;
        }
        case ENHS_CSV_STATUS_SENDING:
            break;
        case ENHS_CSV_STATUS_SENT:
            enhs_teardown_csv_data(connector_ptr);
            break;
    }

error:
    return status;
}

STATIC connector_callback_status_t enhs_handle_data_callback(connector_data_service_send_data_t * const data_ptr)
{
    connector_callback_status_t status = connector_callback_error;
    enhs_data_push_t * const enhs_data = data_ptr->user_context;
    unsigned int bytes_used;

    ASSERT_GOTO(enhs_data != NULL, error);

    bytes_used = MIN_VALUE(data_ptr->bytes_available, enhs_data->bytes_available);

    memcpy(data_ptr->buffer, enhs_data->p_csv, bytes_used);
    data_ptr->bytes_used = bytes_used;
    data_ptr->more_data = connector_bool(enhs_data->bytes_available);

    enhs_data->p_csv += bytes_used;
    enhs_data->bytes_available -= bytes_used;

    status = connector_callback_continue;

error:
    return status;
}

STATIC connector_callback_status_t enhs_handle_response_callback(connector_data_t * const connector_ptr, connector_data_service_send_response_t * const data_ptr)
{
    connector_callback_status_t const status = connector_callback_continue;

    connector_debug_line("enhs_handle_response_callback, response %d", data_ptr->response);
    return status;
}

STATIC connector_callback_status_t enhs_handle_status_callback(connector_data_t * const connector_ptr, connector_data_service_status_t * const data_ptr)
{
    connector_callback_status_t const status = connector_callback_continue;
    enhs_data_push_t * const enhs_data_push_info = data_ptr->user_context;
    connector_request_data_service_send_t const * const send_request = enhs_data_push_info->send_request;

    connector_debug_line("enhs_handle_status_callback, status %d", data_ptr->status);
    free(enhs_data_push_info);
    free((void *)send_request);
    connector_ptr->enhs.info.csv.status = ENHS_CSV_STATUS_SENT;

    return status;
}


#if (defined CONNECTOR_SHORT_MESSAGE)
STATIC connector_callback_status_t enhs_handle_length_callback(connector_data_t * const connector_ptr, connector_data_service_length_t * const data_ptr)
{
    connector_callback_status_t status = connector_callback_abort;
    enhs_data_push_t * const enhs_data = data_ptr->user_context;
    ASSERT_GOTO(enhs_data != NULL, error);

    data_ptr->total_bytes = enhs_data->bytes_available;
    status = connector_callback_continue;
error:
    return status;
}
#endif

STATIC connector_callback_status_t enhs_handle_callback(connector_data_t * const connector_ptr, connector_request_id_data_service_t const ds_request_id, void * const data)
{
    connector_callback_status_t status;

    switch (ds_request_id)
    {
        case connector_request_id_data_service_send_data:
            status = enhs_handle_data_callback(data);
            break;

        case connector_request_id_data_service_send_response:
            status = enhs_handle_response_callback(connector_ptr, data);
            break;

        case connector_request_id_data_service_send_status:
            status = enhs_handle_status_callback(connector_ptr, data);
            break;

        #if (defined CONNECTOR_SHORT_MESSAGE)
        case connector_request_id_data_service_send_length:
            status = enhs_handle_length_callback(connector_ptr, data);
            break;
        #endif

        default:
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
