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

#include "connector_config.h"
#include "platform.h"
#include "os_support.h"
#include "data_service.h"
#include "connector_debug.h"

#define SEND_DATA_SIZE  64
typedef struct
{
    char const * data_ptr;
    size_t bytes;
} client_data_t;

#if 0
static connector_callback_status_t device_request_process_target(connector_data_service_receive_target_t * const target_data)
{
    connector_callback_status_t status = connector_callback_abort;
	connector_callbacks_t * const app_callbacks = connector_get_app_callbacks();
	
	if (app_callbacks->device_request != NULL)
	{
		connector_app_error_t result;
		connector_dataservice_data_t app_data;
		
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
		{
			connector_request = response_data->user_context;
		}
		
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
#endif

connector_callback_status_t app_data_service_handler(connector_request_id_data_service_t const request_id, void * const cb_data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request_id)
    {
        case connector_request_id_data_service_send_data:
        {
            connector_data_service_send_data_t * send_ptr = cb_data;
            client_data_t * const app_data = send_ptr->user_context;

            ASSERT(app_data != NULL);
            send_ptr->bytes_used = (send_ptr->bytes_available > app_data->bytes) ? app_data->bytes : send_ptr->bytes_available;

            memcpy(send_ptr->buffer, app_data->data_ptr, send_ptr->bytes_used);
            app_data->data_ptr += send_ptr->bytes_used;
            app_data->bytes -= send_ptr->bytes_used;
            send_ptr->more_data = (app_data->bytes > 0) ? connector_true : connector_false;

            break;
        }

        case connector_request_id_data_service_send_response:
        {
            connector_data_service_send_response_t * const resp_ptr = cb_data;
            client_data_t * const app_data = resp_ptr->user_context;

            APP_DEBUG("Received %s response from server\n", (resp_ptr->response == connector_data_service_send_response_success) ? "success" : "error");
            if (resp_ptr->hint != NULL)
            {
                APP_DEBUG("Server response %s\n", resp_ptr->hint);
            }

            free(app_data); /* SP: TODO */
            break;
        }

        case connector_request_id_data_service_send_status:
        {
            connector_data_service_status_t * const error_ptr = cb_data;
            client_data_t * const app_data = error_ptr->user_context;

            APP_DEBUG("Data service error: %d\n", error_ptr->status);

            free(app_data); /* SP: TODO */
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
