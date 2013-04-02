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

#define PUT_REQUEST_TEST_ERROR

#define INITIAL_WAIT_COUNT      4

static int gWait = 0;

extern connector_callback_status_t app_os_malloc(size_t const size, void ** ptr);
extern void app_os_free(void * const ptr);
extern int firmware_download_started;

#define DS_MAX_USER   20
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
        if (is_ok != connector_callback_continue || ptr == NULL)
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

    status = connector_initiate_action(handle, connector_initiate_send_data, &user->header);
    if (status == connector_success)
    {
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

    if (put_file_active_count >= DS_MAX_USER)
    {
        status = connector_invalid_data_range;
        goto done;
    }

    if (firmware_download_started)
    {
        status = connector_service_busy;
        goto done;
    }

    if (first_time)
    {
        int i;

        for (i = 0; i < DS_DATA_SIZE; i++)
            ds_buffer[i] = 0x41 + (rand() % 0x3B);
        first_time = false;
    }

    sprintf(filename, "test/dvt%d.txt", index);
    status = send_file(handle, index, filename, ds_buffer, (rand() % (DS_DATA_SIZE +1)));

done:
    return status;
}

connector_callback_status_t app_put_request_handler(connector_request_id_data_service_t const request_id, void * data)
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

            if (gWait < INITIAL_WAIT_COUNT)
            {
                APP_DEBUG("busy\n");
                status = connector_callback_busy;
                gWait++;
                goto done;
            }
            if (user->bytes_sent > 0 && gWait < (INITIAL_WAIT_COUNT + 2))
            {
                APP_DEBUG("busy\n");
                status = connector_callback_busy;
                gWait++;
                goto done;
            }
             {
                connector_data_service_block_t * message = put_response->client_data;
                char * dptr = message->data;
                size_t const bytes_available = message->length_in_bytes;
                size_t const bytes_to_send = user->file_length_in_bytes - user->bytes_sent;
                size_t bytes_copy = (bytes_to_send > bytes_available) ? bytes_available : bytes_to_send;

#if defined(PUT_REQUEST_TEST_ERROR)
                switch(user->index)
                {
                case 0:
                    goto error;
                case 1:
                    if (message->length_in_bytes > bytes_copy)
                    {
                        goto error;
                    }
                    break;
                default:
                    break;
                }
#endif

                memcpy(dptr, &ds_buffer[user->bytes_sent], bytes_copy);
                message->length_in_bytes = bytes_copy;
                message->flags = 0;
                if (user->bytes_sent == 0)
                {
                    message->flags |= CONNECTOR_MSG_FIRST_DATA;
                    APP_DEBUG("app_put_request_handler: (need data) %s %p\n", user->file_path, (void *)user);
                }

                user->bytes_sent += bytes_copy;
                if (user->bytes_sent == user->file_length_in_bytes)
                    message->flags |= CONNECTOR_MSG_LAST_DATA;

            }
            break;

        case connector_data_service_type_have_data:
            {
                connector_data_service_block_t * message = put_request->server_data;
                char * data = message->data;

                if (message->length_in_bytes > 0)
                {
                	data[message->length_in_bytes] = '\0';
                    APP_DEBUG("app_put_request_handler: server response (%zu) %s\n", message->length_in_bytes, data);
                }

                APP_DEBUG("app_put_request_handler (have_data): status = 0x%x %s done this session %p\n",
                		   message->flags, user->file_path, (void *)user);
              /* should be done now */
                ASSERT(user != NULL);
                app_os_free(user);
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
        goto done;

error:
        APP_DEBUG("app_put_request_handler (need_data): %s cancel this session %p\n", user->file_path, (void *)user);
        put_response->message_status = connector_msg_error_cancel;
        app_os_free(user);
        put_file_active_count--;

    }

done:
    return status;
}

/* supported targets */
static char const device_request_target[] = "iik_target";
static char const device_request_not_handle_target[] = "iik_not_handle_target";
static char const device_request_cancel_target[] = "iik_cancel_target";
static char const device_request_invalid_response_target[] = "iik_cancel_response_target";
static char const device_request_invalid_response_target1[] = "iik_cancel_response_target1";

#define DEVICE_REPONSE_COUNT    2

typedef struct device_request_handle {
    void * session;
    char * response_data;
    size_t length_in_bytes;
    char const * target;
    unsigned int count;
} device_request_handle_t;

static unsigned int device_request_active_count = 0;

static connector_callback_status_t app_process_device_request_target(connector_data_service_receive_target_t * const target_data)
{
    connector_callback_status_t status = connector_callback_continue;
    device_request_handle_t * device_request = target_data->user_context;

    APP_DEBUG("Device request data: target = \"%s\"\n", target_data->target);

    if (target_data->user_context == NULL)
    {

        /* 1st chunk of device request so let's allocate memory for it
         * and setup user_context for the client_device_request.
         */
       void * ptr;

        connector_callback_status_t const ccode = app_os_malloc(sizeof *device_request, &ptr);
        if (ccode != connector_callback_continue || ptr == NULL)
        {
            /* no memeory stop iDigi Connector */
            APP_DEBUG("process_device_request: malloc fails for device request on session %p\n", server_device_request->device_handle);
            status = connector_callback_error;
            goto done;
        }

        device_request = ptr;
        target_data->user_context = ptr;

    }

    if (gWait < INITIAL_WAIT_COUNT + 4)
    {
        APP_DEBUG("busy\n");
        status = connector_callback_busy;
        gWait++;
        goto done;
    }

    put_request_size += sizeof *device_request;

    device_request->length_in_bytes = 0;
    device_request->response_data = NULL;
    device_request->count = 0;
    device_request->target = NULL;

    device_request_active_count++;

    ASSERT(target_data->target != NULL);
    if (strcmp(target_data->target, device_request_target) == 0)
    {
        /* testing regular process target */
        device_request->target = device_request_target;
    }
    else if (strcmp(target_data->target, device_request_not_handle_target) == 0)
    {
        /* testing to return not processed message */
        device_request->target = device_request_not_handle_target;
        status = connector_callback_error;
    }
    else if (strcmp(target_data->target, device_request_invalid_response_target) == 0)
    {
        /* testing to return error in response callback */
        device_request->target = device_request_invalid_response_target;
    }
    else if (strcmp(target_data->target, device_request_invalid_response_target1) == 0)
    {
        /* testing to return error in response callback */
        device_request->target = device_request_invalid_response_target1;
    }
    else
    {
        /* testing to return unrecognized status */
        APP_DEBUG("process_device_request: unrecognized target = %s\n", server_device_request->target);
        status = connector_callback_error;
    }


done:
    return status;
}

static connector_callback_status_t app_process_device_request_data(connector_data_service_receive_data_t * const receive_data)
{
    connector_callback_status_t status = connector_callback_continue;
    device_request_handle_t * device_request = receive_data->user_context;

    ASSERT(device_request != NULL);

    if (gWait < (INITIAL_WAIT_COUNT+6))
    {
        printf("busy\n");
        status = connector_callback_busy;
        gWait++;
        goto done;
    }

    device_request->length_in_bytes += receive_data->bytes_used;
    APP_DEBUG("app_process_device_request_data: handle %p target = \"%s\" data length = %zu total length = %zu\n",
                                 (void *)device_request,
                                 device_request->target,
                                 receive_data->bytes_used,
                                 device_request->length_in_bytes);

    if (!receive_data->more_data)
    {   /* No more chunk. let's setup response data */
        /* don't care about what target in here */
        if (first_time)
        {
            int i;

            for (i = 0; i < DS_DATA_SIZE; i++)
                ds_buffer[i] = 0x41 + (rand() % 0x3B);
            first_time = false;
        }
        device_request->response_data = ds_buffer;
        device_request->length_in_bytes = (rand() % (DS_DATA_SIZE +1));
        device_request->count = DEVICE_REPONSE_COUNT;
    }

done:
    return status;
}

static connector_callback_status_t app_process_device_request_response(connector_data_service_receive_reply_data_t * const reply_data)
{
    connector_callback_status_t status = connector_callback_continue;
    device_request_handle_t * const device_request = reply_data->user_context;

    ASSERT(device_request != NULL);

#if VID  /* we cannot cancel the request anymore */

    if (device_request->target == device_request_invalid_response_target)
    {
        /* cancel before sending any response data */
        APP_DEBUG("process_device_response: handle %p cancel\n", (void *)device_request);
        response_data->message_status = connector_msg_error_cancel;
        goto error;
    }
#endif

    if (gWait < (INITIAL_WAIT_COUNT + 8))
    {
        printf("busy\n");
        status = connector_callback_busy;
        gWait++;
        goto done;
    }

    {
        connector_data_service_block_t * const client_data = response_data->client_data;
        size_t const bytes = (device_request->length_in_bytes < reply_data->bytes_available) ? device_request->length_in_bytes : reply_data->bytes_available;


#if VID  /* we cannot cancel the request anymore */
        if ((device_request->target == device_request_invalid_response_target1) &&
            (device_request->count == 1))
        {
            /* cancel client response after sending some response data */
            APP_DEBUG("process_device_response: handle %p cancel\n", (void *)device_request);
            response_data->message_status = connector_msg_error_cancel;
            goto error;
        }
#endif

        APP_DEBUG("process_device_response: handle %p total length = %zu send_byte %zu\n",
                                    (void *)device_request,
                                    device_request->length_in_bytes,
                                    bytes);

        /* let's copy the response data to service_response buffer */
        memcpy(reply_data->buffer, device_request->response_data, bytes);
        device_request->response_data += bytes;
        device_request->length_in_bytes -= bytes;

        reply_data->bytes_used = bytes;
        reply_data->more_data = (device_request->length_in_bytes == 0 && device_request->count == 1) ? connector_false : connector_true;
    }

    if (device_request->length_in_bytes == 0)
    {
        device_request->count--;
        if (device_request->count > 0)
        {
            /* setup more data to be sent */
            device_request->response_data = ds_buffer;
            device_request->length_in_bytes = (rand() % (DS_DATA_SIZE +1));
        }
        else
        {
            /* done. let free memory */
            goto error;
        }
    }
    goto done;

error:
    /* done */
    device_request_active_count--;
    app_os_free(device_request);

done:
    return status;
}

static connector_callback_status_t app_process_device_request_status(connector_data_service_status_t const * const status_data)
{
    connector_callback_status_t status = connector_callback_continue;

    device_request_handle_t * const device_request = status_data->user_context;

    ASSERT(device_request != NULL);

    switch (status_data->status)
    {
    case connector_data_service_status_session_error:
        APP_DEBUG("app_process_device_request_error: handle %p session error %d\n",
                   (void *) device_request, status_data->session_error);
        break;
    default:
        APP_DEBUG("app_process_device_request_error: handle %p session error %d\n",
                    (void *)device_request, status_data->status);
        break;
    }

    device_request_active_count--;
    app_os_free(device_request);

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


