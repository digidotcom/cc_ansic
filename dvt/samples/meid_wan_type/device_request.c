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

extern connector_callback_status_t app_os_malloc(size_t const size, void ** ptr);
extern void app_os_free(void * const ptr);

#define DS_MAX_USER   5
#define DS_FILE_NAME_LEN  20
#define DS_DATA_SIZE  (1024 * 16)

static char ds_buffer[DS_DATA_SIZE];

typedef struct
{
    connector_data_service_put_request_t header;
    char file_path[DS_FILE_NAME_LEN];
    size_t bytes_sent;
    size_t file_length_in_bytes;
    char * file_data;
    int index;
    unsigned long initiate_time;
    unsigned long first_data_time;
    unsigned long last_data_time;
} ds_record_t;

unsigned int put_file_active_count = 0;
static bool first_time = true;
size_t put_request_size = 0;

connector_status_t send_file(connector_handle_t handle, int index, char * const filename, char * const content, size_t content_length)
{

    connector_status_t status = connector_success;
    static char file_type[] = "text/plain";
    ds_record_t * user;

    {
        void * ptr;

        connector_callback_status_t const is_ok = app_os_malloc(sizeof *user, &ptr);
        if (is_ok != connector_callback_continue|| ptr == NULL)
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
    user->file_length_in_bytes = content_length;
    user->index = index;
    app_os_get_system_time(&user->initiate_time);

    status = connector_initiate_action(handle, connector_initiate_send_data, &user->header);
    if (status == connector_success)
    {
        APP_DEBUG("send_file: %p %s length %zu\n", (void *)user, user->file_path, user->file_length_in_bytes);
        put_file_active_count++;
    }
    else
    {
        app_os_free(user);
    }

done:
    return status;
}

connector_status_t send_put_request(connector_handle_t handle, int index)
{

    connector_status_t status = connector_success;
    char filename[DS_FILE_NAME_LEN];
    static int fileindex = 0;

    if (put_file_active_count >= DS_MAX_USER)
    {
        status = connector_invalid_data_range;
        goto done;
    }


    if (first_time)
    {
        int i;

        for (i = 0; i < DS_DATA_SIZE; i++)
            ds_buffer[i] = 0x41 + (rand() % 0x3B);
        first_time = false;
    }
    fileindex = (fileindex > 9) ? 0 : fileindex +1;
    sprintf(filename, "test/dvt%d.txt", fileindex);
    status = send_file(handle, index, filename, ds_buffer, (rand() % (DS_DATA_SIZE +1)));

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
                {
                    app_os_get_system_time(&user->first_data_time);
                    message->flags |= CONNECTOR_MSG_FIRST_DATA;
                    APP_DEBUG("app_put_request_handler: (need data) %s %p total length %zu\n", user->file_path, (void *)user, user->file_length_in_bytes);
                }

                user->bytes_sent += bytes_copy;
                if (user->bytes_sent == user->file_length_in_bytes)
                {
                    app_os_get_system_time(&user->last_data_time);
                    message->flags |= CONNECTOR_MSG_LAST_DATA;
                    APP_DEBUG("app_put_request_handler: (need data) %s %p done\n", user->file_path, (void *)user);
                }

            }
            break;

        case connector_data_service_type_have_data:
            {
                connector_data_service_block_t * message = put_request->server_data;
                char * data = message->data;
                unsigned long current_time;

                app_os_get_system_time(&current_time);
                APP_DEBUG("app_put_request_handler: (have data) %s %p\n", user->file_path, (void *)user);
                APP_DEBUG("app_put_request_handler: (have data) time initiate = %lu\n", user->initiate_time);
                APP_DEBUG("app_put_request_handler: (have data) time between initiate and 1st data  = %lu\n", (user->first_data_time - user->initiate_time));
                APP_DEBUG("app_put_request_handler: (have data) time between 1st and last data = %lu\n", (user->last_data_time - user->first_data_time));
                APP_DEBUG("app_put_request_handler: (have data) time between last data and response = %lu\n", (current_time - user->last_data_time));

                if (message->length_in_bytes > 0)
                {
                    data[message->length_in_bytes] = '\0';
                    APP_DEBUG("app_put_request_handler: server response (%zu) %s\n", message->length_in_bytes, data);
                }

                /* should be done now */
                app_os_free(user);
                APP_DEBUG("app_put_request_handler (have_data): status = 0x%x %s done this session %p\n",
                        message->flags, user->file_path, (void *)user);
                put_file_active_count--;

            }
            break;

        case connector_data_service_type_error:
            {

                APP_DEBUG("app_put_request_handler (type_error): %s cancel this session %p\n", user->file_path, (void *)user);
                ASSERT(user != NULL);
                app_os_free(user);
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

/* supported target name */
static char const wan_type_target[] = "wan_type";

device_request_target_t device_request_target = device_request_not_support;

typedef struct device_request_handle {
    void * device_handle;
    char const * response_data;
    device_request_target_t target;
    size_t length_in_bytes;
} device_request_handle_t;

static unsigned int device_request_active_count = 0;

static connector_callback_status_t app_process_device_request(connector_data_service_msg_request_t const * const request_data,
                                                      connector_data_service_msg_response_t * const response_data)
{
    connector_callback_status_t result = connector_callback_continue;
    connector_data_service_device_request_t * const server_device_request = request_data->service_context;
    connector_data_service_block_t * const server_data = request_data->server_data;
    device_request_target_t target_type = device_request_not_support;


    device_request_handle_t * client_device_request = response_data->user_context;

    if (server_data == NULL)
    {
        APP_DEBUG("app_process_device_request: server_data is NULL\n");
        goto done;
    }

    if ((server_data->flags & CONNECTOR_MSG_FIRST_DATA) == CONNECTOR_MSG_FIRST_DATA)
    {
        /* target should not be null on 1st chunk of data */
        if (server_device_request->target == NULL)
        {
            APP_DEBUG("app_process_device_request: NULL target\n");
            response_data->message_status = connector_msg_error_cancel;
            goto done;
        }

        if (strcmp(server_device_request->target, wan_type_target) == 0)
        {
            target_type = device_request_wan_type_target;
        }

        /* 1st chunk of device request so let's allocate memory for it
         * and setup user_context for the client_device_request.
         */
        {
            void * ptr;

            ptr = malloc(sizeof *client_device_request);
            if (ptr == NULL)
            {
                /* no memeory so cancel this request */
                APP_DEBUG("app_process_device_request: malloc fails for device request on session %p\n", server_device_request->device_handle);
                response_data->message_status = connector_msg_error_memory;
                goto done;
            }

            client_device_request = ptr;
        }
        client_device_request->length_in_bytes = 0;
        client_device_request->response_data = NULL;
        client_device_request->device_handle = server_device_request->device_handle;
        client_device_request->target = target_type;

         /* setup the user_context for our device request data */
         response_data->user_context = client_device_request;
         device_request_active_count++;
    }
    else
    {
        /* device request should be our user_context */
        if (client_device_request == NULL)
        {
            APP_DEBUG("app_process_device_request: NULL client_device_request\n");
            goto done;
        }
    }

done:
    return result;
}

static connector_callback_status_t app_process_device_response(connector_data_service_msg_request_t const * const request_data,
                                                       connector_data_service_msg_response_t * const response_data)
{
    device_request_handle_t * const client_device_request = response_data->user_context;

    UNUSED_ARGUMENT(request_data);

    if ((response_data->client_data == NULL) || (client_device_request == NULL))
    {
        APP_DEBUG("app_process_device_response: invalid input\n");
        goto error;
    }

    if (client_device_request->length_in_bytes == 0)
    {
        static char const device_request_done[] = "Device request success";
        static char const device_request_unable_to_processed[] = "Device request unsupported";

        connector_data_service_block_t * const client_data = response_data->client_data;
        char * response_message_data = (char *)device_request_done;

        switch (client_device_request->target)
        {
        case device_request_not_support:
            client_data->flags = CONNECTOR_MSG_DATA_NOT_PROCESSED;
            response_message_data = (char *)device_request_unable_to_processed;
            break;

        case device_request_wan_type_target:
        {
            #define WAN_ID_LENGTH            18
            #define VALUE_TO_STRING(value)   # value
            #define MACRO_TO_STRING(macro)   VALUE_TO_STRING(macro)

            #define WAN_TYPE_FORMAT "<type>%d</type><id>%s</id>"

            static char wan_type_response[sizeof WAN_TYPE_FORMAT + sizeof(INT_MAX) + WAN_ID_LENGTH];

            ASSERT(strlen(wan_type_response) <= WAN_ID_LENGTH);
            sprintf(wan_type_response, WAN_TYPE_FORMAT, wan_type, wan_id_string);
            APP_DEBUG("%s\n", wan_type_response);
            response_message_data = (char *)wan_type_response;
            break;
        }
        }

        device_request_target = client_device_request->target;
        client_device_request->response_data = response_message_data;
        client_device_request->length_in_bytes = strlen(response_message_data);


        {
            /* get number of bytes written to the client data buffer */
            size_t const bytes = (client_device_request->length_in_bytes < client_data->length_in_bytes) ? client_device_request->length_in_bytes : client_data->length_in_bytes;

            /* let's copy the response data to service_response buffer */
            memcpy(client_data->data, client_device_request->response_data, bytes);

            client_device_request->response_data += bytes;
            client_device_request->length_in_bytes -= bytes;

            client_data->flags |= (client_device_request->length_in_bytes == 0) ? CONNECTOR_MSG_LAST_DATA : 0;
            client_data->length_in_bytes = bytes;

        }
     }

    response_data->message_status = connector_msg_error_none;

    if (client_device_request->length_in_bytes == 0)
    {   /* done */
        device_request_active_count--;
        free(client_device_request);
        response_data->user_context = NULL;

    }

error:
    return connector_callback_continue;
}

static connector_callback_status_t app_process_device_error(connector_data_service_msg_request_t const * const request_data,
                                                    connector_data_service_msg_response_t * const response_data)
{
    device_request_handle_t * const client_device_request = response_data->user_context;
    connector_data_service_block_t * error_data = request_data->server_data;
    connector_msg_error_t const error_code = *((connector_msg_error_t *)error_data->data);


    if (client_device_request != NULL)
    {
        APP_DEBUG("app_process_device_error: handle %p error %d from server\n",
                    client_device_request->device_handle, error_code);

        device_request_active_count--;
        free(client_device_request);
    }
    else
    {
        APP_DEBUG("app_process_device_error: No handle error %d from server\n", error_code);
    }

    return connector_callback_continue;
}


connector_callback_status_t app_device_request_handler(void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_data_service_msg_request_t const * const service_device_request = request_data;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (service_device_request->message_type)
    {
    case connector_data_service_type_have_data:
        status = app_process_device_request(request_data, response_data);
        break;
    case connector_data_service_type_need_data:
        status = app_process_device_response(request_data, response_data);
        break;
    case connector_data_service_type_error:
        status = app_process_device_error(request_data, response_data);
        break;
    default:
        APP_DEBUG("app_device_request_handler: unknown message type %d for connector_data_service_device_request\n", service_device_request->message_type);
        break;
    }

    return status;
}

connector_callback_status_t app_data_service_handler(connector_data_service_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request)
    {
    case connector_data_service_put_request:
        status = app_put_request_handler(request_data, request_length, response_data, response_length);
        break;
    case connector_data_service_device_request:
        status = app_device_request_handler(request_data, request_length, response_data, response_length);
        break;
    default:
        APP_DEBUG("app_data_service_handler: Request not supported: %d\n", request);
        break;
    }
    return status;
}

