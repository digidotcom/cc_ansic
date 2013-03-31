/*
 *  Copyright (c) 1996-2011 Digi International Inc., All Rights Reserved
 *
 *  This software contains proprietary and confidential information of Digi
 *  International Inc.  By accepting transfer of this copy, Recipient agrees
 *  to retain this software in confidence, to prevent disclosure to others,
 *  and to make no use of this software other than that for which it was
 *  delivered.  This is an unpublished copyrighted work of Digi International
 *  Inc.  Except as permitted by federal law, 17 USC 117, copying is strictly
 *  prohibited.
 *
 *  Restricted Rights Legend
 *
 *  Use, duplication, or disclosure by the Government is subject to
 *  restrictions set forth in sub-paragraph (c)(1)(ii) of The Rights in
 *  Technical Data and Computer Software clause at DFARS 252.227-7031 or
 *  subparagraphs (c)(1) and (2) of the Commercial Computer Software -
 *  Restricted Rights at 48 CFR 52.227-19, as applicable.
 *
 *  Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 *
 * =======================================================================
 *
 */
#include <stdlib.h>
#include <stdbool.h>
#include "connector_api.h"
#include "platform.h"
#include "application.h"

#define DS_FILE_NAME_LEN  20
#define DS_DATA_SIZE  (1024 * 8)

#define PUT_FILE_MAX    1

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


static unsigned int put_file_active_count = 0;
static unsigned int put_file_count = 0;
static bool first_time = true;
bool start_put_file = true;

static connector_status_t send_file(connector_handle_t handle, int index, char * const filename, char * const content, size_t content_length)
{

    connector_status_t status = connector_success;
    static char file_type[] = "text/plain";

    ds_record_t * user = malloc(sizeof *user);

    if (user == NULL)
    {
        /* no memeory stop IIK */
        APP_DEBUG("send_put_request: malloc fails\n");
        status = connector_no_resource;
        goto done;
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
    if (status != connector_success)
    {
        if (status != connector_init_error)
            APP_DEBUG("send_file: connector_initiate_action failed %d\n", status);

        free(user);
    }
    else
    {
        if (put_file_count == 0)
        {
            startWritingCount = 0;

        }

        {
            #define SIZE_TEXT "Size = "

            static char ds_complete_text[sizeof user->file_path + sizeof (SIZE_TEXT MACRO_TO_STRING(SIZE_MAX)) + 1];

            sprintf(ds_complete_text, "%s %s%zu", user->file_path, SIZE_TEXT, user->file_length_in_bytes);

            open_timing_table("Initiate put file: ", ds_complete_text);
        }
    }

done:
    return status;
}


void send_put_request(connector_handle_t handle)
{


    if (start_put_file && put_file_active_count < PUT_FILE_MAX)
    {

        if (first_time)
        {
            int i;

            for (i = 0; i < DS_DATA_SIZE; i++)
                ds_buffer[i] = 0x41 + (rand() % 0x3B);
            first_time = false;
        }

        {
            char filename[DS_FILE_NAME_LEN];

            sprintf(filename, "test/dvt%d.txt", put_file_active_count);
            if (send_file(handle, put_file_active_count, filename, ds_buffer, DS_DATA_SIZE) == connector_success)
            {
                put_file_active_count++;
                put_file_count++;
                if (put_file_active_count == PUT_FILE_MAX) start_put_file = false;
            }
        }
    }

    return;
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

                memcpy(dptr, &ds_buffer[user->bytes_sent], bytes_copy);
                message->length_in_bytes = bytes_copy;
                message->flags = 0;
                if (user->bytes_sent == 0)
                {
                    message->flags |= CONNECTOR_MSG_FIRST_DATA;
                    APP_DEBUG("app_put_request_handler: (need data) %s %p\n", user->file_path, (void *)user);
                    writing_timing_description("start put data: ", user->file_path);
                }

                user->bytes_sent += bytes_copy;
                if (user->bytes_sent == user->file_length_in_bytes)
                {
                    message->flags |= CONNECTOR_MSG_LAST_DATA;
                    writing_timing_description("End put data: ", user->file_path);
                }
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

                close_timing_table("Got response for put data: ", user->file_path);

                APP_DEBUG("app_put_request_handler (have_data): status = 0x%x %s done this session %p\n",
                   message->flags, user->file_path, (void *)user);
              /* should be done now */
                ASSERT(user != NULL);
                free(user);
                put_file_active_count--;
            }
            break;

        case connector_data_service_type_error:
            {

                APP_DEBUG("app_put_request_handler (type_error): %s cancel this session %p\n", user->file_path, (void *)user);
                ASSERT(user != NULL);
                free(user);
                put_file_active_count--;
            }
            break;

        default:
            APP_DEBUG("app_put_request_handler: Unexpected message type: %d\n", put_request->message_type);
            break;
        }
        goto done;
    }

done:
    return status;
}

/* supported targets */
static char const device_request_target[] = "ds_timing_test";
static char const put_device_request_target[] = "put_ds_timing_test";
static char const request_terminate_target[] = "request_terminate";

typedef enum {
    device_request_timing,
    device_request_put_timing,
    device_request_terminate,
    device_request_unsupported
} devcie_request_target_t;

#define DEVICE_REPONSE_COUNT    1

typedef struct device_request_handle {
    void * session;
    char * response_data;
    size_t length_in_bytes;
    devcie_request_target_t target;
    unsigned int count;
} device_request_handle_t;

static unsigned int device_request_active_count = 0;

static connector_callback_status_t app_process_device_request_target(connector_data_service_receive_target_t * const target_data)
{
    connector_callback_status_t status = connector_callback_continue;
    device_request_handle_t * device_request = target_data->user_context;

    ASSERT(target_data->target != NULL);

    open_timing_table("Start device request", target_data->target);

    APP_DEBUG("Device request data: target = \"%s\"\n", target_data->target);

    if (target_data->user_context == NULL)
    {
        device_request = malloc(sizeof *device_request);
        if (device_request == NULL)
        {
            /* no memeory stop IIK */
            APP_DEBUG("app_process_device_request_target: malloc fails for device request target %s\n", target_data->target);
            status = connector_callback_error;
            goto done;
        }
        target_data->user_context = device_request;
        device_request->target = NULL;
    }

    device_request->length_in_bytes = 0;
    device_request->response_data = NULL;
    device_request->count = 0;
    device_request_active_count++;


    if (strcmp(target_data->target, request_terminate_target) == 0)
    {
        connector_status_t action_status;

        device_request->target = (char *)device_request_terminate;

        APP_DEBUG("process_device_request: terminate IC (active session = %d)\n", put_file_active_count);
        action_status = connector_initiate_action(connector_handle, connector_initiate_terminate, NULL);
        if (action_status != connector_success)
        {
            APP_DEBUG("process_device_request: connector_initiate_terminate error %d\n", action_status);
        }
        status = connector_callback_error;
    }
    else if (strcmp(target_data->target, device_request_target) == 0)
    {
        device_request->target = device_request_timing;
    }
    else if (strcmp(target_data->target, put_device_request_target) == 0)
    {
        device_request->target = device_request_put_timing;
    }
    else
    {
        device_request->target = device_request_supported;
        status = connector_callback_error;
    }

done:
    return status;
}

static connector_callback_status_t app_process_device_request_data(connector_data_service_receive_data_t * const receive_data)
{
    connector_callback_status_t status = connector_callback_continue;

    device_request_handle_t * device_request = receive_data->user_context;

    switch (device_request->target)
    {
        case device_request_timing:
            break;
        case device_request_put_timing:
#if 0
            /* trigger put file */
            start_put_file = true;
            break;
#endif
        case device_request_terminate:
        case device_request_unsupported:
            /* should not be here */
            ASSERT(connector_false);
            goto done;
    }

    device_request->length_in_bytes += receive_data->bytes_used;
    APP_DEBUG("process_device_request: handle %p target = \"%d\" data length = %lu total length = %lu\n",
                                 (void *)device_request,
                                 device_request->target,
                                 (unsigned long int)receive_data->bytes_used,
                                 (unsigned long int)device_request->length_in_bytes);

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
        device_request->length_in_bytes = DS_DATA_SIZE;
        device_request->count = DEVICE_REPONSE_COUNT;
    }

done:
    return status;
}

static connector_callback_status_t app_process_device_request_response(connector_data_service_receive_reply_data_t * const reply_data)
{
    connector_callback_status_t status = connector_callback_continue;

    device_request_handle_t * const device_request = reply_data->user_context;

   {
        size_t const bytes = (device_request->length_in_bytes < reply_data->bytes_available) ? device_request->length_in_bytes : reply_data->bytes_available;

        APP_DEBUG("process_device_response: handle %p total length = %lu send_byte %lu\n",
                                    (void *)device_request,
                                    (unsigned long int)device_request->length_in_bytes,
                                    (unsigned long int)bytes);

        /* let's copy the response data to service_response buffer */
        memcpy(reply_data->buffer, device_request->response_data, bytes);
        device_request->response_data += bytes;
        device_request->length_in_bytes -= bytes;

        reply_data->bytes_used = bytes;
        reply_data->more_data = (device_request->length_in_bytes == 0 && device_request->count == 1) ? connector_false : connector_true;
   }

    if (!reply_data->more_data)
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
            close_timing_table("End device request ", device_request->target);
            APP_DEBUG("process_device_response: End device request\n");

            request_active_count--;
            free(device_request);
        }
    }

    return status;
}

static connector_callback_status_t app_process_device_request_status(connector_data_service_receive_status_t const * const status_data)
{
    connector_callback_status_t status = connector_callback_continue;

    device_request_handle_t * const device_request = status_data->user_context;

    ASSERT(device_request != NULL);

    switch (status_data->status)
    {
    case connector_data_service_receive_status_session_error:
        APP_DEBUG("app_process_device_request_error: handle %p session error %d\n",
                   (void *) device_request, status_data->session_error);
        break;
    default:
        APP_DEBUG("app_process_device_request_error: handle %p session error %d\n",
                    (void *)device_request, status_data->status);
        break;
    }

    device_request_active_count--;
    free(device_request);

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


