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
#include <stdlib.h>
#include <stdbool.h>
#include "connector_api.h"
#include "platform.h"
#include "application.h"

#define PUT_REQUEST_TEST_ERROR

#define INITIAL_WAIT_COUNT      4

#define DS_FILE_NAME_LEN  20

typedef struct
{
    connector_data_service_put_request_t header;
    char file_path[DS_FILE_NAME_LEN];
    size_t bytes_sent;
    size_t file_length_in_bytes;
    char * file_data;
    int index;
} ds_record_t;

unsigned int put_file_active_count = 0;

connector_status_t send_put_request(connector_handle_t handle, char * const filename, char * const content)
{

    connector_status_t status = connector_success;
    static char file_type[] = "text/plain";
    ds_record_t * user;

    if (put_file_active_count != 0)
    {
        status = connector_service_busy;
        goto done;
    }
    {
        void * ptr;

        ptr = malloc(sizeof *user);
        if (ptr == NULL)
        {
            /* no memeory stop iDigi Connector */
            APP_DEBUG("send_put_request: malloc fails\n");
            status = connector_no_resource;
            goto done;
        }
        user = ptr;
    }

    sprintf(user->file_path, "%s", filename);
    user->header.flags = 0;
    user->header.path  = user->file_path;
    user->header.content_type = file_type;
    user->header.context = user;
    user->header.transport = connector_transport_tcp;
    user->bytes_sent = 0;
    user->file_data = content;
    user->file_length_in_bytes = strlen(content);

    status = connector_initiate_action(handle, connector_initiate_send_data, &user->header);
    if (status == connector_success)
    {
        put_file_active_count++;
    }
    else
    {
        free(user);
    }

done:
    return status;
}

connector_callback_status_t app_put_request_handler(void const * request_data, size_t const request_length,
                                                   void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    {
        connector_data_service_msg_request_t const * const put_request = request_data;
        connector_data_service_msg_response_t * const put_response = response_data;

        connector_data_service_put_request_t const * const header = put_request->service_context;
        ds_record_t * const user = (ds_record_t * const)header->context;

        if ((put_request == NULL) || (put_response == NULL))
        {
             APP_DEBUG("app_put_request_handler: Invalid request_data [%p] or response_data[%p]\n", request_data, response_data);
             goto done;
        }

        switch (put_request->message_type)
        {
        case connector_data_service_type_need_data:
             {

                connector_data_service_block_t * message = put_response->client_data;
                char * dptr = message->data;
                size_t const bytes_available = message->length_in_bytes;
                size_t const bytes_to_send = user->file_length_in_bytes - user->bytes_sent;
                size_t bytes_copy = (bytes_to_send > bytes_available) ? bytes_available : bytes_to_send;

                memcpy(dptr, &user->file_data[user->bytes_sent], bytes_copy);
                message->length_in_bytes = bytes_copy;
                message->flags = 0;
                if (user->bytes_sent == 0)
                    message->flags |= CONNECTOR_MSG_FIRST_DATA;

                user->bytes_sent += bytes_copy;
                if (user->bytes_sent == user->file_length_in_bytes)
                {
                    message->flags |= CONNECTOR_MSG_LAST_DATA;
                    APP_DEBUG("app_put_request_handle: (need_data) done sending %s file\n", user->file_path);
                }

            }
            break;

        case connector_data_service_type_have_data:
            {
                connector_data_service_block_t * message = put_request->server_data;
                uint8_t const * data = message->data;

                ASSERT(user != NULL);

                if (message->length_in_bytes > 0)
                {
                    APP_DEBUG("app_put_request_handler: (have_data) server response for %s file: %s\n",
                            user->file_path, (char *)data);
                }
                if ((message->flags & CONNECTOR_MSG_RESP_SUCCESS) != CONNECTOR_MSG_RESP_SUCCESS)
                {
                    APP_DEBUG("app_put_request_handler: (have data) server response for %s file with an error 0x%X\n",
                                user->file_path, message->flags);
                }

                /* should be done now */
                free(user);
                put_file_active_count--;
                reboot_state = no_reboot_received;
                delay_receive_state = no_delay_receive;
            }
            break;

        case connector_data_service_type_error:
            {

                ASSERT(user != NULL);
                APP_DEBUG("app_put_request_handler: type_error for putting %s file\n", user->file_path);
                free(user);
                put_file_active_count--;
            }
            break;

        default:
            APP_DEBUG("app_put_request_handler: Unexpected message type: %d\n", put_request->message_type);
            break;
        }
    }
done:
    return status;
}

/* we only supported 1 target */
static char const request_reboot_ready[] = "request_reboot_ready";

static connector_callback_status_t app_process_device_request_target(connector_data_service_receive_target_t * const target_data)
{
    connector_callback_status_t status = connector_callback_continue;

    if (strcmp(target_data->target, request_reboot_ready) == 0)
    {
        /* cause to delay calling receive */
        if (delay_receive_state == no_delay_receive)
            delay_receive_state = start_delay_receive;
        else
            APP_DEBUG("app_process_device_request_target: %s already started\n", request_reboot_ready);
    }
    else
    {
        /* testing to return unrecognized status */
        APP_DEBUG("process_device_request: unrecognized target = \"%s\"\n", target_info->target);
        status = connector_callback_error;
    }
    /* don't care any data in the request */
    return status;
}

static connector_callback_status_t app_process_device_request_data(connector_data_service_receive_data_t * const receive_data)
{
    connector_callback_status_t status = connector_callback_continue;

    /* don't care any data in the request */
    return status;
}

static connector_callback_status_t app_process_device_request_response(connector_data_service_receive_reply_data_t * const reply_data)
{
    connector_callback_status_t status = connector_callback_continue;
    /* user just lets us know that reboot request is about to start.
     * just respond so he knows we are connected and ready.
     */
    reply_data->bytes_used = 0; /* no data */

    return status;
}

static connector_callback_status_t app_process_device_request_status(connector_data_service_status_t const * const status_data)
{
    connector_callback_status_t status = connector_callback_continue;


    switch (status_data->status)
    {
    case connector_data_service_status_session_error:
        APP_DEBUG("app_process_device_request_error: session error %d\n",
                   status_data->session_error);
        break;
    default:
        APP_DEBUG("app_process_device_request_error: error %d\n",
                   status_data->status);
        break;
    }

    return status;
}

connector_callback_status_t app_device_request_handler(connector_request_id_data_service_t const request_id, void * data)
{
    connector_callback_status_t status = connector_callback_unrecognized;

    switch (request_id)
    {
        case connector_request_id_data_service_receive_target:
            status = app_process_device_request_target(data);
            break;
        case connector_request_id_data_service_receive_data:
            status = app_process_device_request_data(data);
            break;
        case connector_request_id_data_service_receive_status:
            status = app_process_device_request_status(data);
            break;
        case connector_request_id_data_service_receive_reply_data:
            status = app_process_device_request_response(data);
            break;
        case connector_request_id_data_service_receive_reply_length:
        default:
            APP_DEBUG("app_device_request_handler: unknown request id type %d for connector_request_id_data_service\n", request_id);
            break;
    }

    return status;
}

connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request_id,
                                                  void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request_id)
    {
    case connector_request_id_data_service_send_length:
    case connector_request_id_data_service_send_data:
    case connector_request_id_data_service_send_status:
    case connector_request_id_data_service_send_response:
        status = app_put_request_handler(request_id, data);
        break;
    case connector_request_id_data_service_receive_target:
    case connector_request_id_data_service_receive_data:
    case connector_request_id_data_service_receive_status:
    case connector_request_id_data_service_receive_reply_data:
    case connector_request_id_data_service_receive_reply_length:
        status = app_device_request_handler(request_id, data);
        break;
    default:
        APP_DEBUG("app_data_service_handler: Request not supported: %d\n", request_id);
        break;
    }
    return status;
}


