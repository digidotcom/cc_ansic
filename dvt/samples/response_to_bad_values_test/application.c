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


#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "connector_api.h"
#include "platform.h"
#include "application.h"


extern connector_callback_status_t app_data_service_handler(connector_data_service_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length);
extern connector_callback_status_t app_firmware_handler(connector_firmware_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length);

extern connector_status_t send_put_request(connector_handle_t handle, char * const filename, char * const content);

/*
 * This table lists all the errors we wish to cause in response to the callbacks.
 */

config_test_t config_test[connector_config_password +1] = {config_test_invalid_data};

connector_auto_connect_type_t app_connector_reconnect(connector_class_t const class_id, connector_close_status_t const status)
{
    UNUSED_ARGUMENT(class_id);

    connector_auto_connect_type_t type;

    switch (status)
    {
           /* if either the server or our application cuts the connection, don't reconnect */
        case connector_close_status_device_terminated:
        case connector_close_status_device_stopped:
        case connector_close_status_abort:
             type = connector_manual_connect;
             break;

       /* otherwise it's an error and we want to retry */
       default:
             type = connector_auto_connect;
             break;
    }

    return type;
}

connector_callback_status_t app_connector_callback(connector_class_t const class_id, connector_request_t const request_id,
                                    void const * const request_data, size_t const request_length,
                                    void * response_data, size_t * const response_length)
{
    connector_callback_status_t   status = config_test_invalid_data;

    switch (class_id)
    {
    case connector_class_config:
    {
        unsigned int const i = request_id.config_request;

        ASSERT(request_id.config_request < sizeof config_test);

        switch (config_test[i])
        {
        case config_test_invalid_data:
            status = app_invalid_data_config_handler(request_id.config_request, request_data, request_length, response_data, response_length);
            config_test[i]++;
            break;
        case config_test_invalid_size:
            status = app_invalid_size_config_handler(request_id.config_request, request_data, request_length, response_data, response_length);
            config_test[i]++;
            break;
        case config_test_none:
            if (request_id.config_request == connector_config_meid || request_id.config_request == connector_config_esn || request_id.config_request == connector_config_imei_number)
            {
                status = app_invalid_size_config_handler(request_id.config_request, request_data, request_length, response_data, response_length);
            }
            else
            {
                status = app_config_handler(request_id.config_request, request_data, request_length, response_data, response_length);
            }
            break;
        }

        break;
    }
    case connector_class_operating_system:
        status = app_os_handler(request_id.os_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_network_tcp:
        status = app_network_tcp_handler(request_id.network_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_data_service:
        status = app_data_service_handler(request_id.data_service_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_firmware:
        status = app_firmware_handler(request_id.firmware_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_status:
        status = app_status_handler(request_id.status_request, request_data, request_length, response_data, response_length);
        break;

    default:
        /* not supported */
        status = connector_callback_unrecognized;
        break;
    }
    return status;
}

int application_run(connector_handle_t handle)
{
    UNUSED_ARGUMENT(handle);

    return 0;
}

