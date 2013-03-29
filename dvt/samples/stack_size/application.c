/*
 * Copyright (c) 2011, 2012 Digi International Inc.,
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
#include "application.h"
#include "connector_config.h"
#include "connector_api.h"
#include "platform.h"
#include "remote_config.h"

connector_bool_t app_connector_reconnect(connector_class_id_t const class_id, connector_close_status_t const status)
{
    UNUSED_ARGUMENT(class_id);

    connector_bool_t type;

    switch (status)
    {
           /* if either the server or our application cuts the connection, don't reconnect */
        case connector_close_status_device_terminated:
        case connector_close_status_device_stopped:
        case connector_close_status_abort:
             type = connector_false;
             break;

       /* otherwise it's an error and we want to retry */
       default:
             type = connector_true;
             break;
    }

    return type;
}

connector_callback_status_t app_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id,
                                    void const * const request_data, size_t const request_length,
                                    void * response_data, size_t * const response_length)
{
    connector_callback_status_t   status;

    switch (class_id)
    {
    case connector_class_config:
        status = app_config_handler(request_id.config_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_operating_system:
        status = app_os_handler(request_id.os_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_network_tcp:
        status = app_network_tcp_handler(request_id.network_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_data_service:
        status = app_data_service_handler(request_id.data_service_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_firmware:
        status = app_firmware_handler(request_id.firmware_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_file_system:
        status = app_file_system_handler(request_id.file_system_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_remote_config_service:
        status = app_remote_config_handler(request_id.remote_config_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_status:
        status = app_status_handler(request_id.status_request, request_data, request_length, response_data, response_length);
        break;

    default:
        status = connector_callback_unrecognized;
        /* not supported */
        break;
    }

    clear_stack_size();

    return status;
}

int application_run(connector_handle_t handle)
{
    connector_status_t status;

#if 0
    size_t file_length = file_length = strlen(terminate_file_content);

     usleep(5 * 1000000);

     while (terminate_flag == device_request_terminate_done  && file_length > 0)
     {

         status = send_file(handle, 255, (char * const)TERMINATE_TEST_FILE, (char * const)terminate_file_content, file_length);
         if (status != connector_success)
         {
             APP_DEBUG("application_run: unable to send terminate_test.txt %d\n", status);
             usleep(2 * 1000000);
         }
         else
         {
             /* assume it's done sending */
             APP_DEBUG("application_run: sent terminate_test.txt\n");
             break;
         }
     }
#endif

    for (;connector_run_thread_status != connector_device_terminated;)
    {
        status = send_put_request(handle);

        switch (status)
        {
        case connector_init_error:
        case connector_service_busy:
           #define SLEEP_ONE_SECOND  (1 * 1000000)
            usleep(SLEEP_ONE_SECOND);
            break;

        case connector_success:
            break;

        case connector_invalid_data_range:
             #define SLEEP_BETWEEN_TESTS   (1 * 1000000)
            usleep(SLEEP_BETWEEN_TESTS);
            break;
        case connector_device_terminated:
            goto done;
        default:
            break;
        }
    }
    APP_DEBUG("application_run: done!\n");
done:
    return 0;
}


