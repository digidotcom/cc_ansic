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

#include "connector_api.h"
#include "platform.h"
#include "os_support.h"
#include "data_service.h"
#include "connector_debug.h"

typedef struct
{
    char * target;
    void * context;
} connector__request_t;

#if 0
static void free_connector_request(connector__request_t * request)
{
    if (request != NULL)
    {
        if (request->target != NULL)
            ic_free(request->target);
        ic_free(request);
    }
}

static connector__error_t map_msg_error_to_user_error(connector_msg_error_t const msg_error)
{
    connector__error_t user_error;

    APP_DEBUG("Received connector msg error: %d\n", msg_error);
    switch (msg_error)
    {
        case connector_msg_error_none:
            user_error = connector__success;
            break;

        case connector_msg_error_fatal:
        case connector_msg_error_memory:
        case connector_msg_error_no_service:
            user_error = connector__resource_error;
            break;

        case connector_msg_error_invalid_opcode:
        case connector_msg_error_format:
            user_error = connector__invalid_parameter;
            break;

        case connector_msg_error_session_in_use:
        case connector_msg_error_unknown_session:
        case connector_msg_error_cancel:
            user_error = connector__session_error;
            break;

        case connector_msg_error_compression_failure:
        case connector_msg_error_decompression_failure:
            user_error = connector__compression_error;
            break;

        case connector_msg_error_send:
        case connector_msg_error_ack:
            user_error = connector__network_error;
            break;

        case connector_msg_error_busy:
        case connector_msg_error_timeout:
            user_error = connector__timeout;
            break;

        default:
            user_error = connector__init_error;
            break;
    }

    return user_error;
}

static connector_callback_status_t process_device_request(connector_data_service_msg_request_t const * const request_data,
                                                      connector_data_service_msg_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_abort;
    connector_data_service_device_request_t * const server_device_request = request_data->service_context;
    connector_data_service_block_t * const server_data = request_data->server_data;
    connector__callbacks_t * const app_callbacks = connector_get_app_callbacks();

    if (app_callbacks->device_request != NULL)
    {
        connector_app_error_t result;
        connector__data_t app_data;
        connector__request_t * connector_request;

        if ((server_data->flags & connector_MSG_FIRST_DATA) == connector_MSG_FIRST_DATA)
        {
            connector_request = ic_malloc(sizeof(connector__request_t));
            ASSERT_GOTO(connector_request != NULL, error);

            connector_request->target = ic_malloc(strlen(server_device_request->target) + 1);
            ASSERT_GOTO(connector_request->target != NULL, error);

            strcpy(connector_request->target, server_device_request->target);
            connector_request->context = NULL;
            response_data->user_context = connector_request;
        }
        else
            connector_request = response_data->user_context;

        app_data.error = connector__success;
        app_data.data_ptr = server_data->data;
        app_data.length_in_bytes = server_data->length_in_bytes;
        app_data.flags = ((server_data->flags & connector_MSG_LAST_DATA) == connector_MSG_LAST_DATA) ? connector_FLAG_LAST_DATA : 0;
        app_data.app_context = connector_request->context;

        result = app_callbacks->device_request(connector_request->target, &app_data);
        switch (result)
        {
            case connector_app_success:
                connector_request->context = app_data.app_context;
                status = connector_callback_continue;
                break;

            case connector_app_busy:
                status = connector_callback_busy;
                break;

            case connector_app_unknown_target:
                response_data->message_status = connector_msg_error_cancel;
                status = connector_callback_continue;
                break;

            default:
                APP_DEBUG("process_device_request: application error [%d]\n", result);
                break;
        }
    }
    else
    {
        APP_DEBUG("process_device_request: callback is not registered\n");
        response_data->message_status = connector_msg_error_cancel;
        status = connector_callback_continue;
    }

error:
    return status;
}

static connector_callback_status_t process_device_response(connector_data_service_msg_request_t const * const request_data,
                                                       connector_data_service_msg_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_data_service_block_t * const client_data = response_data->client_data;
    connector__callbacks_t * const app_callbacks = connector_get_app_callbacks();

    UNUSED_ARGUMENT(request_data);
    if (app_callbacks->device_response != NULL)
    {
        size_t response_bytes;
        connector__data_t app_data;
        connector__request_t * const connector_request = response_data->user_context;

        app_data.error = connector__success;
        app_data.data_ptr = client_data->data;
        app_data.length_in_bytes = client_data->length_in_bytes;
        app_data.flags = ((client_data->flags & connector_MSG_LAST_DATA) == connector_MSG_LAST_DATA) ? connector_FLAG_LAST_DATA : 0;
        app_data.app_context = connector_request->context;

        response_bytes = app_callbacks->device_response(connector_request->target, &app_data);
        if (app_data.error == connector__success)
        {
            client_data->length_in_bytes = response_bytes;
            if ((app_data.flags & connector_FLAG_LAST_DATA) == connector_FLAG_LAST_DATA)
            {
                client_data->flags = connector_MSG_LAST_DATA;
                free_connector_request(connector_request);
            }
            else
                status = (response_bytes == 0) ? connector_callback_busy : connector_callback_continue;
        }
        else
        {
            APP_DEBUG("process_device_response: app returned error [%d]\n", app_data.error);
            client_data->flags = connector_MSG_DATA_NOT_PROCESSED;
            free_connector_request(connector_request);
        }
    }
    else
    {
        APP_DEBUG("process_device_response: callback is not registered\n");
        client_data->flags = connector_MSG_DATA_NOT_PROCESSED;
        status = connector_callback_continue;
    }

    return status;
}

static connector_callback_status_t process_device_error(connector_data_service_msg_request_t const * const request_data,
                                                    connector_data_service_msg_response_t * const response_data)
{
    connector_data_service_block_t * error_data = request_data->server_data;
    connector_msg_error_t const error_code = *((connector_msg_error_t *)error_data->data);
    connector__request_t * const connector_request = response_data->user_context;
    connector__callbacks_t * const app_callbacks = connector_get_app_callbacks();

    /*APP_DEBUG("process_device_error: target %s error %d from server\n", request_data->server_data, error_code);*/
    APP_DEBUG("process_device_error\n");

    if (app_callbacks->device_response != NULL)
    {
        size_t response_bytes;
        connector__data_t app_data;

        app_data.error = map_msg_error_to_user_error(error_code);
        app_data.data_ptr = NULL;
        response_bytes = app_callbacks->device_response(connector_request->target, &app_data);
        ASSERT(response_bytes == 0);
    }

    free_connector_request(connector_request);
    return connector_callback_continue;
}
#endif

static void send_data_completed(connector_app_send_data_t * const app_dptr, connector_error_t const error_code)
{
    app_dptr->error = error_code;
    ecc_set_event(ECC_SEND_DATA_EVENT, app_dptr->event_bit);
}

static connector_callback_status_t process_send_data_request(connector_data_service_send_data_t *send_ptr)
{
	connector_callback_status_t status = connector_callback_continue;
	
    if (send_ptr != NULL)
    {
        connector_app_send_data_t * const app_data = (connector_app_send_data_t * const)send_ptr->user_context;

        ASSERT(app_data != NULL);
        send_ptr->bytes_used = (send_ptr->bytes_available > app_data->bytes_remaining) ? app_data->bytes_remaining : send_ptr->bytes_available;

        memcpy(send_ptr->buffer, app_data->next_data, send_ptr->bytes_used);
        app_data->next_data = ((char *)app_data->next_data) + send_ptr->bytes_used;
        app_data->bytes_remaining -= send_ptr->bytes_used;
        send_ptr->more_data = (app_data->bytes_remaining > 0) ? connector_true : connector_false;
    }
    else
    {
        APP_DEBUG("process_send_data_request: no app data set to send\n");
        status = connector_callback_abort;
    }

    return status;
}

static connector_callback_status_t process_send_data_response(connector_data_service_send_response_t const * const resp_ptr)
{

    connector_app_send_data_t * const app_dptr = (connector_app_send_data_t * const)resp_ptr->user_context;
    connector_error_t error_code;

    error_code = (resp_ptr->response == connector_data_service_send_response_success) ? connector_success : connector_unavailable;
    APP_DEBUG("Received %s response from server\n", (resp_ptr->response == connector_data_service_send_response_success) ? "success" : "error");

    send_data_completed(app_dptr, error_code);
    return connector_callback_continue;
}

static connector_callback_status_t process_send_data_error(connector_data_service_status_t const * const error_ptr)
{
    connector_app_send_data_t * const app_dptr = (connector_app_send_data_t * const)error_ptr->user_context;

    APP_DEBUG("Data service error: %d\n", error_ptr->status);
    send_data_completed(app_dptr, error_ptr->status);

    return connector_callback_continue;
}

connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request_id, void * const cb_data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request_id)
    {
        case connector_request_id_data_service_send_data:
        {
            connector_data_service_send_data_t * send_ptr = cb_data;

            status = process_send_data_request(send_ptr);
            break;
        }
        case connector_request_id_data_service_send_response:
        {
            connector_data_service_send_response_t * const resp_ptr = cb_data;

            status = process_send_data_response(resp_ptr);
            break;
        }
        case connector_request_id_data_service_send_status:
        {
            connector_data_service_status_t * const error_ptr = cb_data;
            
            status = process_send_data_error(error_ptr);
            break;
        }
#if 0
        case connector_request_id_data_service_receive_target:
            status = device_request_process_target(data);
            break;
        case connector_request_id_data_service_receive_data:
            status = device_request_process_data(data);
            break;
        case connector_request_id_data_service_receive_status:
            status = device_request_process_status(data);
            break;
        case connector_request_id_data_service_receive_reply_data:
            status = device_request_process_response(data);
            break;
        case connector_request_id_data_service_receive_reply_length:
#endif
        default:
            APP_DEBUG("app_data_service_handler: unknown request id type %d for connector_request_id_data_service\n", request_id);
            break;
    }

    return status;
}
