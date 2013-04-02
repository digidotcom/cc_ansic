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

#define INITIAL_WAIT_COUNT      4

terminate_flag_t terminate_flag = device_request_idle;

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

    if (firmware_download_started != 0)
    {
        status = connector_service_busy;
        goto done;
    }

    if (terminate_flag == device_request_terminate_in_application_start &&
        put_file_active_count > 0)
    {
        connector_status_t ccode;

        APP_DEBUG("send_put_request: terminate iik (active session = %d)\n", put_file_active_count);
        ccode = app_terminate_tcp_transport(handle);
        if (ccode != connector_success)
        {
            APP_DEBUG("process_device_request: app_terminate_tcp_transport error %d\n", ccode);
        }
        else
        {
            terminate_flag = device_request_terminate_done;
            status = connector_callback_busy;
            goto done;
        }
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

            if (terminate_flag == device_request_terminate_start &&
                put_file_active_count > 0)
            {
                connector_status_t ccode;

                APP_DEBUG("app_put_request_handler: terminate iik (active session = %d)\n", put_file_active_count);
                ccode = app_terminate_tcp_transport(connector_handle);
                if (ccode != connector_success)
                {
                    APP_DEBUG("process_device_request: app_terminate_tcp_transport error %d\n", ccode);
                }
                else
                {
                    terminate_flag = device_request_terminate_done;
                    status = connector_callback_busy;
                    goto done;
                }
            }

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
                    APP_DEBUG("app_put_request_handler: (need data) %s %p\n", user->file_path, (void *)user);
                }

                user->bytes_sent += bytes_copy;
                if (user->bytes_sent == user->file_length_in_bytes)
                {
                    app_os_get_system_time(&user->last_data_time);
                    message->flags |= CONNECTOR_MSG_LAST_DATA;
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

                if (strcmp(user->file_path, TERMINATE_TEST_FILE) == 0)
                {
                    if (terminate_flag != device_request_terminate_done)
                    {
                        APP_DEBUG("app_put_request_handle: (have data) unexpected terminate_flag = 0x%x\n", (unsigned)terminate_flag);
                    }
                    terminate_flag = device_request_idle;
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

/* we only supported 1 target */
static char const request_terminate_target[] = "request_terminate_in_callback";
static char const request_terminate_application_target[] = "request_terminate_in_application";

static connector_callback_status_t app_process_device_request_target(connector_data_service_receive_target_t * const target_data)
{
    connector_callback_status_t status = connector_callback_continue;
    ASSERT(server_data != NULL);
    ASSERT(server_device_request != NULL);


    {
        unsigned long current_time;
        terminate_flag_t request_flag = device_request_idle;

        if (strcmp(target_info->target, request_terminate_target) == 0)
        {
            request_flag = device_request_terminate;
        }
        else if (strcmp(target_info->target, request_terminate_application_target) == 0)
        {
            request_flag = device_request_terminate_in_application;
        }
        else
        {
            /* testing to return unrecognized status */
            APP_DEBUG("app_process_device_request_target: unrecognized target = \"%s\"\n", target_data->target);
            status = connector_callback_error;
        }

        if (request_flag != device_request_idle)
        {
            /* are we still processing previous in device_request */
            if (terminate_flag != device_request_idle)
            {
                /* not done processing previous device request */
                status = connector_callback_busy;
            }
            else
            {
                app_os_get_system_time(&current_time);
                APP_DEBUG("process_device_request: request = %d time stamp = %lu (active session = %d)\n", request_flag, current_time, put_file_active_count);
                target_data->user_context = &terminate_flag;
                terminate_flag = request_flag;
            }
        }

    }
    /* don't care any data in the request */
    return status;

}
static connector_callback_status_t app_process_device_request_data(connector_data_service_receive_data_t * const receive_data)
{
    connector_callback_status_t status = connector_callback_continue;

    ASSERT(receive_data->user_context != NULL);

    /* don't care any data in the request */
    return status;
}

static connector_callback_status_t app_process_device_request_response(connector_data_service_receive_reply_data_t * const reply_data)
{
    connector_callback_status_t status = connector_callback_continue;

    if (reply_data->user_context == &terminate_flag)
    {
        unsigned long current_time;

        app_os_get_system_time(&current_time);
        APP_DEBUG("app_process_device_request_response: time stamp = %lu (active session = %d)\n", current_time, put_file_active_count);

        switch (terminate_flag)
        {
        case device_request_terminate_in_application:
            terminate_flag = device_request_terminate_in_application_start;
            APP_DEBUG("app_process_device_request_response: device_request_terminate_in_application_start\n");
            break;
        case device_request_terminate:
            APP_DEBUG("app_process_device_request_response: device_request_terminate\n");
            terminate_flag = device_request_terminate_start;
            break;
        default:
            APP_DEBUG("app_process_device_request_response: unknown terminate_flag = %d\n", terminate_flag);
        }
    }
    reply_data->length_in_bytes = 0;

    return status;
}

static connector_callback_status_t app_process_device_request_status(connector_data_service_status_t const * const status_data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (status_data->status)
    {
    case connector_data_service_status_session_error:
        APP_DEBUG("app_process_device_request_status: session error %d\n",
                   status_data->session_error);
        break;
    default:
        APP_DEBUG("app_process_device_request_status: error %d\n",
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

