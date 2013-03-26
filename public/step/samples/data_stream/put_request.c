/*
 * Copyright (c) 2011, 2012 Digi International Inc.,
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
#include <malloc.h>
#include "connector_api.h"
#include "platform.h"

#define DIA_SAMPLE_DATA_SIZE  256
typedef struct
{
    char buffer[DIA_SAMPLE_DATA_SIZE];
    size_t bytes_left;
    size_t bytes_sent;
} dia_info_t;

static dia_info_t * get_dia_info(void)
{
    int result;
    dia_info_t * info = calloc(1, sizeof(dia_info_t));

    if (info == NULL)
    {
        APP_DEBUG("Failed to malloc dia info record\n");
        goto done;
    }

    {
        char timestamp[sizeof "yyyy-mm-dd hh:mm:ss"];
        time_t uptime = time(NULL);
        struct tm *tmval = gmtime(&uptime);

        sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d", tmval->tm_year+1900, tmval->tm_mon+1, tmval->tm_mday,
                                                            tmval->tm_hour, tmval->tm_min, tmval->tm_sec);
        APP_DEBUG("Time[UTC]: %s\n", timestamp);
        result = sprintf(info->buffer, "<connector_data> <sample>\n"
                                       "    <name>sensor0.noise_level</name>\n"
                                       "    <value>%d</value>\n"
                                       "    <unit>dB</unit>\n"
                                       "    <timestamp>%s</timestamp>\n"
                                       "</sample> </connector_data>\n", 10 + rand()%75, timestamp);
        if (result > 0)
        {
            info->bytes_left = result;
            goto done;
        }

        APP_DEBUG("Failed to assign dia data\n");
    }

    free(info);
    info = NULL;

done:
    return info;
}

connector_status_t app_send_put_request(connector_handle_t handle)
{
    connector_status_t status = connector_no_resource;
    static connector_data_service_put_request_t header; /* idigi connector will hold this until we get a response/error callback */
    static char const file_type[] = "text/xml";
    static char const file_path[] = "dia_file.xml";
    dia_info_t * const dia_info = get_dia_info();

    if (dia_info == NULL)
        goto error;

    header.flags = 0;
    header.path  = file_path;
    header.content_type = file_type;
    header.context = dia_info; /* will be returned in all subsequent callbacks */

    status = connector_initiate_action(handle, connector_initiate_send_data, &header);
    APP_DEBUG("Status: %d, file: %s\n", status, header.path);
    if (status != connector_success)
        free(dia_info);

error:
    return status;
}

connector_callback_status_t app_data_service_handler(connector_data_service_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_data_service_msg_request_t const * const put_request = request_data;
    connector_data_service_msg_response_t * const put_response = response_data;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    if ((put_request == NULL) || (put_response == NULL) || (put_request->service_context == NULL))
    {
         APP_DEBUG("Invalid request_data [%p] or response_data[%p]\n", request_data, response_data);
         goto done;
    }

    if (request == connector_data_service_put_request)
    {
        connector_data_service_put_request_t const * const header = put_request->service_context;
        dia_info_t * const dia_info = (dia_info_t *)header->context;

        ASSERT(dia_info != NULL);
        switch (put_request->message_type)
        {
        case connector_data_service_type_need_data:
            {
                connector_data_service_block_t * const message = put_response->client_data;

                if (message->length_in_bytes > dia_info->bytes_left)
                    message->length_in_bytes = dia_info->bytes_left;

                memcpy(message->data, &dia_info->buffer[dia_info->bytes_sent], message->length_in_bytes);
                dia_info->bytes_sent += message->length_in_bytes;
                dia_info->bytes_left -= message->length_in_bytes;
                if (dia_info->bytes_left == 0)
                    message->flags = CONNECTOR_MSG_LAST_DATA;
                put_response->message_status = connector_msg_error_none;
            }
            break;

        case connector_data_service_type_have_data:
            {
                connector_data_service_block_t * const message = put_request->server_data;

                APP_DEBUG("Received %s response from server\n", ((message->flags & CONNECTOR_MSG_RESP_SUCCESS) != 0) ? "success" : "error");
                if (message->length_in_bytes > 0)
                {
                    char * const data = message->data;

                    data[message->length_in_bytes] = '\0';
                    APP_DEBUG("Server response %s\n", data);
                }

                free(dia_info);
            }
            break;

        case connector_data_service_type_error:
            {
                connector_data_service_block_t * const message = put_request->server_data;
                connector_msg_error_t const * const error_value = message->data;

                APP_DEBUG("Data service error: %d\n", *error_value);
                free(dia_info);
            }
            break;

        default:
            APP_DEBUG("Unexpected command: %d\n", request);
            break;
        }
    }
    else
    {
        APP_DEBUG("Request not supported in this sample: %d\n", request);
    }

done:
    return status;
}

