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
#include "connector.h"
#include "os_support.h"
#ifdef CONNECTOR_DATA_SERVICE
#include "data_service.h"
#endif
#ifdef CONNECTOR_DATA_POINTS
#include "data_point.h"
#endif
#include "connector_config.h"
#include "connector_debug.h"

static connector_callbacks_t connector_callback_list =
{
    NULL, /* status callback */
    NULL, /* device request */
    NULL, /* device response */
    NULL, /* device firmware download */
    NULL /* device reset */
};

#ifdef CONNECTOR_DATA_SERVICE
typedef struct
{
	connector_request_data_service_send_t header;
    connector_app_send_data_t data_ptr;
} connector_send_t;
#endif

static connector_handle_t connector_handle = NULL;
static connector_bool_t connection_ready = connector_false;
void *get_connector_handle(void)
{
	return connector_handle;
}

connector_bool_t get_connection_status(void)
{
	return connection_ready;
}
static connector_callback_status_t app_tcp_status(connector_tcp_status_t const * const status)
{

    switch (*status)
    {
    case connector_tcp_communication_started:
        APP_DEBUG("connector_tcp_communication_started\n");
        connection_ready = connector_true;
        break;
    case connector_tcp_keepalive_missed:
        APP_DEBUG("connector_tcp_keepalive_missed\n");
        break;
    case connector_tcp_keepalive_restored:
        APP_DEBUG("connector_tcp_keepalive_restored\n");
        connection_ready = connector_true;
        break;
    }

    return connector_callback_continue;
}

static connector_callback_status_t app_status_handler(connector_request_id_status_t const request, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request)
    {
        case connector_request_id_status_tcp:
            status = app_tcp_status(data);
            break;

        case connector_request_id_status_stop_completed:
            APP_DEBUG("connector_restore_keepalive\n");
            break;

        default:
            APP_DEBUG("Status request not supported in sm_udp: %d\n", request);
            break;
    }

    return status;
}

connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
                                                   connector_request_id_t const request_id,
                                                   void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (class_id)
    {
    case connector_class_id_config:
        status = app_config_handler(request_id.config_request, data);
        break;

    case connector_class_id_operating_system:
        status = app_os_handler(request_id.os_request, data);
        break;

    case connector_class_id_network_tcp:
        status = app_network_tcp_handler(request_id.network_request, data);
        break;

#if (defined CONNECTOR_DATA_SERVICE)
    case connector_class_id_data_service:
    	status = app_data_service_handler(request_id.data_service_request, data);
        break;
#endif

#if (defined CONNECTOR_DATA_POINTS)
    case connector_class_id_data_point:
    	status = app_data_point_handler(request_id.data_point_request, data);
        break;
#endif
        
#if (defined CONNECTOR_FIRMWARE_SERVICE)
    case connector_class_id_firmware:
        status = app_firmware_handler(request_id.firmware_request, data);
        break;
#endif

    case connector_class_id_status:
        status = app_status_handler(request_id.status_request, data);
        break;
#if (defined CONNECTOR_FILE_SYSTEM)
    case connector_class_id_file_system:
        status = app_file_system_handler(request_id.file_system_request, data);
        break;
#endif

#if (defined CONNECTOR_RCI_SERVICE)
    case connector_class_id_remote_config:
        status = app_remote_config_handler(request_id.remote_config_request, data);
        break;
#endif

    default:
        /* not supported */
        break;
    }

    return status;
}

void connector_thread(unsigned long initial_data)
{
    connector_status_t status = connector_success;

    UNUSED_PARAMETER(initial_data);

    APP_DEBUG("connector_thread start\n");

    do
    {
        status = connector_run(connector_handle);

        if (status == connector_keepalive_error)
        {
            APP_DEBUG("connector_run returned %d..restarting\n", status);
            status = connector_success; /* Network error: restart and reconnect to Device Cloud. */
            
            /* Update Keepalive time */
            start_system_up_time = 0;
        }
    } while (status == connector_success);

    APP_DEBUG("connector_thread exit %d\n", status);
}

connector_error_t connector_start(connector_status_callback_t status_callback)
{
    connector_error_t status = connector_error_init_error;

    UNUSED_PARAMETER(status_callback);

    connector_handle = connector_init((connector_callback_t)app_connector_callback);
    ASSERT_GOTO(connector_handle != NULL, error);

    status = ecc_create_thread();
    ASSERT_GOTO(status == connector_error_success, error);

    status = ecc_create_event(ECC_SEND_DATA_EVENT);
    ASSERT_GOTO(status == connector_error_success, error);

error:
    return status;
}

connector_error_t connector_register_device_request_callbacks(connector_device_request_callback_t    request_callback, 
                                                                connector_device_response_callback_t   response_callback,
                                                                connector_firmware_download_callback_t download_callback,
                                                                connector_reset_callback_t             reset_callback)
{
    connector_error_t status = connector_error_success;

    if ((request_callback == NULL) || (response_callback == NULL))
    {
        APP_DEBUG("connector_register_device_request_callbacks: NULL parameter\n");
        status = connector_error_invalid_parameter;
        goto done;
    }

    if (connector_callback_list.device_request != NULL)
    {
        APP_DEBUG("connector_register_device_request_callbacks: already registered once\n");
        status = connector_error_already_registered;
        goto done;
    }

    connector_callback_list.device_request  = request_callback;
    connector_callback_list.device_response = response_callback;
    connector_callback_list.device_download = download_callback;
    connector_callback_list.device_reset    = reset_callback;

done:
    return status;
}

#ifdef CONNECTOR_DATA_SERVICE
connector_error_t connector_send_data(char const * const path, connector_dataservice_data_t * const device_data, char const * const content_type)
{
    static unsigned long send_event_block = 0;
    connector_error_t result = connector_error_network_error;
    connector_send_t * const send_info = ecc_malloc(sizeof(connector_send_t));

    if ((path == NULL) || (device_data == NULL))
    {
        APP_DEBUG("connector_send_data: invalid parameter\n");
        result = connector_error_invalid_parameter;
        goto error;
    }

    if (send_info == NULL)
    {
        APP_DEBUG("connector_send_data: malloc failed\n");
        result = connector_error_resource_error;
        goto error;
    }

    {
        unsigned long available_bit = 0x80000000;

        while (available_bit != 0)
        {
            if (!(send_event_block & available_bit))
                break;
            available_bit >>= 1;
        }

        send_event_block |= available_bit;
        send_info->data_ptr.event_bit = available_bit;
        if (available_bit == 0)
        {
            APP_DEBUG("connector_send_data: Exceeded maximum of 32 sessions active at a time\n");
            result = connector_error_resource_error;
            goto error;
        }

        /* make sure none of the stale event is pending */
        ecc_clear_event(ECC_SEND_DATA_EVENT, available_bit);
    }

    /* we are storing some stack variables here, need to block until we get a response */
    send_info->data_ptr.error = connector_success;
    send_info->data_ptr.next_data = device_data->data_ptr;
    send_info->data_ptr.bytes_remaining = device_data->length_in_bytes;
    send_info->header.path = path;
    send_info->header.content_type = content_type;
    send_info->header.user_context = &send_info->data_ptr;
    send_info->header.transport = device_data->transport;
    send_info->header.response_required = connector_false;

    if ((device_data->flags & CONNECTOR_FLAG_APPEND_DATA) == CONNECTOR_FLAG_APPEND_DATA)
    {
    	send_info->header.option = connector_data_service_send_option_append;
    }
    else if ((device_data->flags & CONNECTOR_FLAG_ARCHIVE_DATA) == CONNECTOR_FLAG_ARCHIVE_DATA)
    {
    	send_info->header.option = connector_data_service_send_option_archive;
    } else {
    	send_info->header.option = connector_data_service_send_option_overwrite;
    }

    {
        connector_status_t const status = connector_initiate_action(connector_handle, connector_initiate_send_data, &send_info->header);

        if (status == connector_success)
        {
            #define ECC_SEND_TIMEOUT_IN_MSEC 90000
            result = ecc_get_event(ECC_SEND_DATA_EVENT, send_info->data_ptr.event_bit, ECC_SEND_TIMEOUT_IN_MSEC);
        	send_info->data_ptr.error = connector_error_success;
            //TODO: Timeout is not evaluated

            result = send_info->data_ptr.error;
        }
        else
        {
            result = (status == connector_init_error) ? connector_init_error : connector_no_resource;
        }
    }

error:
    if (send_info != NULL)
    {
        send_event_block &= ~send_info->data_ptr.event_bit;
        ecc_free(send_info);
    }

    return result;
}
#endif

connector_callbacks_t * connector_get_app_callbacks(void)
{
    return &connector_callback_list;
}

