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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "connector_api.h"
#include "platform.h"

static unsigned int sample_number;

connector_status_t app_send_put_request(connector_handle_t handle)
{
    connector_status_t status = connector_success;
    static connector_data_service_put_request_t header;
    static char file_path[] = "keepalive.txt";
    static char file_type[] = "text/plain";

    header.flags = 0;
    header.path  = file_path;
    header.content_type = file_type;
    header.transport = connector_transport_tcp;


    status = connector_initiate_action(handle, connector_initiate_send_data, &header);
    APP_DEBUG("Status: %d, file: %s\n", status, file_path);

    return status;
}
#define BUFFER_SIZE 64

connector_callback_status_t app_data_service_handler(connector_data_service_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_data_service_msg_request_t const * const put_request = request_data;
    connector_data_service_msg_response_t * const put_response = response_data;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    if ((put_request == NULL) || (put_response == NULL))
    {
         APP_DEBUG("Invalid request_data [%p] or response_data[%p]\n", request_data, response_data);
         goto done;
    }

    if (request == connector_data_service_put_request)
    {
        switch (put_request->message_type)
        {
        case connector_data_service_type_need_data:
            {
                connector_data_service_block_t * message = put_response->client_data;
                char * dptr = message->data;
                char buffer[BUFFER_SIZE];

                snprintf(buffer, BUFFER_SIZE, "keepalive DVT success\n");
                size_t const bytes = strlen(buffer);

                memcpy(dptr, buffer, bytes);
                message->length_in_bytes = bytes;
                message->flags = CONNECTOR_MSG_LAST_DATA | CONNECTOR_MSG_FIRST_DATA;
                put_response->message_status = connector_msg_error_none;
                sample_number++;
            }
            break;

        case connector_data_service_type_have_data:
            {
                connector_data_service_block_t * message = put_request->server_data;
                uint8_t const * data = message->data;

                APP_DEBUG("Received %s response from server\n", ((message->flags & CONNECTOR_MSG_RESP_SUCCESS) != 0) ? "success" : "error");
                if (message->length_in_bytes > 0) 
                {
                    APP_DEBUG("Server response %s\n", (char *)data);
                }
            }
            break;

        case connector_data_service_type_error:
            {
                connector_data_service_block_t * message = put_request->server_data;
                connector_msg_error_t const * const error_value = message->data;

                APP_DEBUG("Data service error: %d\n", *error_value);
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

