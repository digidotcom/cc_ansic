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

#define DEVICE_REQUEST_TARGET_MAX_LEN	50

typedef struct
{
    char * target;
    connector_bool_t response_required;
    void * context;
} connector_request_t;

static void free_connector_request(connector_request_t * request)
{
    if (request != NULL)
    {
        if (request->target != NULL)
            ecc_free(request->target);
        ecc_free(request);
    }
}

connector_callbacks_t * connector_get_app_callbacks(void);


static connector_callback_status_t app_process_device_request_target(connector_data_service_receive_target_t * const target_data)
{
    connector_callback_status_t status = connector_callback_continue;

    APP_DEBUG("Device request data: target = \"%s\"\n", target_data->target);

    if (target_data->user_context == NULL)
    {
    	connector_request_t * device_request;
		int target_str_size;

		device_request = ecc_malloc(sizeof *device_request);
		if (device_request == NULL)
		{
			/* no memeory so cancel this request */
			APP_DEBUG("app_process_device_request: malloc fails for device request \"%s\" target\n", target_data->target);
			status = connector_callback_error;
			goto done;
		}

		target_data->user_context = device_request;
		target_str_size = strlen(target_data->target) + 1;
		device_request->target = ecc_malloc(target_str_size);
		if (device_request->target == NULL)
		{
			/* no memeory so cancel this request */
			APP_DEBUG("app_process_device_request: malloc fails for device_request->target \"%s\" target\n", target_data->target);
			status = connector_callback_error;
			goto done;
		}
		memcpy(device_request->target, target_data->target, target_str_size);
    }

done:
    return status;
}

static connector_callback_status_t app_process_device_request_data(connector_data_service_receive_data_t * const receive_data)
{
    connector_callback_status_t status = connector_callback_abort;
    connector_callbacks_t * const app_callbacks = connector_get_app_callbacks();

    if (app_callbacks->device_request != NULL)
    {
        connector_app_error_t result;
        connector_dataservice_data_t app_data;
        connector_request_t * connector_request = receive_data->user_context;

        app_data.error = connector_success;
        app_data.data_ptr = (uint8_t *)receive_data->buffer;
        app_data.length_in_bytes = receive_data->bytes_used;
        app_data.app_context = connector_request->context;
        app_data.more_data = receive_data->more_data;
        
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
                APP_DEBUG("process_device_request: Unkown target %s!\n", connector_request->target);
                /* TODO, should we free this string? */
                free_connector_request(connector_request);
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
        status = connector_callback_continue;
    }

error:
    return status;
}

static connector_callback_status_t app_process_device_request_response(connector_data_service_receive_reply_data_t * const reply_data)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_callbacks_t * const app_callbacks = connector_get_app_callbacks();

    if (app_callbacks->device_response != NULL)
    {
        size_t response_bytes;
        connector_dataservice_data_t app_data;
        connector_request_t * const connector_request = reply_data->user_context;

        app_data.error = connector_success;
        app_data.data_ptr = reply_data->buffer;
        app_data.length_in_bytes = reply_data->bytes_available;
        app_data.app_context = connector_request->context;

        response_bytes = app_callbacks->device_response(connector_request->target, &app_data);
        reply_data->more_data = app_data.more_data;

        if (app_data.error == connector_success)
        {
        	reply_data->bytes_used = response_bytes;
            if (reply_data->more_data == connector_false)
            {
                free_connector_request(connector_request);
            }
            else
            {
                status = (response_bytes == 0) ? connector_callback_busy : connector_callback_continue;
            }
        }
        else
        {
            APP_DEBUG("process_device_response: app returned error [%d]\n", app_data.error);
            free_connector_request(connector_request);
        }
    }
    else
    {
        APP_DEBUG("process_device_response: callback is not registered\n");
    }

    return status;
}

static connector_callback_status_t app_process_device_request_status(connector_data_service_status_t const * const status_data)
{
    connector_request_t * const connector_request = status_data->user_context;
    connector_callbacks_t * const app_callbacks = connector_get_app_callbacks();

    APP_DEBUG("app_process_device_request_status: target %s error %d from server\n", connector_request->target, status_data->session_error);

    if (app_callbacks->device_response != NULL)
    {
        size_t response_bytes;
        connector_dataservice_data_t app_data;

        app_data.data_ptr = NULL;
        response_bytes = app_callbacks->device_response(connector_request->target, &app_data);
        ASSERT(response_bytes == 0);
    }

    free_connector_request(connector_request);
    return connector_callback_continue;
}

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
    /*APP_DEBUG("Received %s response from server\n", (resp_ptr->response == connector_data_service_send_response_success) ? "success" : "error");*/

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

connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request_id, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request_id)
    {
        case connector_request_id_data_service_send_data:
        {
            connector_data_service_send_data_t * send_ptr = data;

            status = process_send_data_request(send_ptr);
            break;
        }
        case connector_request_id_data_service_send_response:
        {
            connector_data_service_send_response_t * const resp_ptr = data;

            status = process_send_data_response(resp_ptr);
            break;
        }
        case connector_request_id_data_service_send_status:
        {
            connector_data_service_status_t * const error_ptr = data;
            
            status = process_send_data_error(error_ptr);
            break;
        }

        case connector_request_id_data_service_receive_target:
        {
        	connector_data_service_receive_target_t * const target_data = data;
        	status = app_process_device_request_target(target_data);
			break;
        }
            
        case connector_request_id_data_service_receive_data:
        {
        	connector_data_service_receive_data_t * const receive_data = data;
        	
            status = app_process_device_request_data(receive_data);
            break;
        }
        case connector_request_id_data_service_receive_status:
        {
        	connector_data_service_status_t const * const status_data = data;
            status = app_process_device_request_status(status_data);
            break;
        }
        case connector_request_id_data_service_receive_reply_data:
        {
        	connector_data_service_receive_reply_data_t * const reply_data = data;
            status = app_process_device_request_response(data);
            break;
        }

        case connector_request_id_data_service_receive_reply_length:
        default:
            APP_DEBUG("app_data_service_handler: unknown request id type %d for connector_request_id_data_service\n", request_id);
            break;
    }

    return status;
}
