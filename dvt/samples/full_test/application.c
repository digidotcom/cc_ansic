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
#include "stdio.h"
#include "stdlib.h"
#include "connector_api.h"
#include "platform.h"
#include "connector_dvt.h"

extern void check_stack_size(void);
extern void clear_stack_size(void);

dvt_ds_t  data_service_info;

extern connector_callback_status_t app_data_service_handler(connector_data_service_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length);
extern connector_callback_status_t app_firmware_handler(connector_firmware_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length);

extern connector_status_t send_put_request(connector_handle_t handle, dvt_ds_t * const ds_info);

connector_auto_connect_type_t app_connector_reconnect(connector_class_id_t const class_id, connector_close_status_t const status)
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

connector_callback_status_t app_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id,
                                    void const * const request_data, size_t const request_length,
                                    void * response_data, size_t * const response_length)
{
    connector_callback_status_t   status = connector_callback_continue;

    check_stack_size();

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

    case connector_class_id_status:
        status = app_status_handler(request_id.status_request, request_data, request_length, response_data, response_length);
        break;

    default:
        /* not supported */
        break;
    }

    clear_stack_size();
    return status;
}

#define PATTERN_FILE_MAX_SIZE   (32 * 1024)
//#define SOURCE_PATH  "../../cases/test_files/pattern.txt"
#define SOURCE_PATH  "dvt/cases/test_files/pattern.txt"

int application_run(connector_handle_t handle)
{
    #define SLEEP_IN_SECONDS  1
    int stop_calling = 0;
    dvt_ds_t * const ds_info = &data_service_info;
    FILE * fp = fopen(SOURCE_PATH, "r");

    if (fp == NULL)
    {
        APP_DEBUG("Failed to open %s\n", SOURCE_PATH);
        goto done;
    }

    ds_info->state = dvt_state_init;
    ds_info->file_size = 0;
    ds_info->file_buf = malloc(PATTERN_FILE_MAX_SIZE);
    if (ds_info->file_buf == NULL)
    {
        APP_DEBUG("Failed to malloc in full test application.c\n");
        goto done;
    }

    /* hope it will read everything in one shot, if not modify accordingly */
    ds_info->file_size = fread(ds_info->file_buf, 1, PATTERN_FILE_MAX_SIZE, fp);
    if ((feof(fp) == 0) || (ferror(fp) != 0))
    {
        APP_DEBUG("Failed to read pattern.txt\n");
    }
    else
    {
        APP_DEBUG("Read %zu bytes\n", ds_info->file_size);
    }
    fclose(fp);
    fp = NULL;

    while (!stop_calling)
    {
        switch (ds_info->state)
        {
            case dvt_state_request_start:
            {
                connector_status_t const status = send_put_request(handle, ds_info);

                 switch (status)
                 {
                    case connector_success:
                    case connector_init_error:
                        break;

                    default:
                        APP_DEBUG("Exiting main loop because of error [%d]\n", status);
                        stop_calling = 1;
                        break;
                }
                break;
            }

            default:
                break;
        }

        usleep(SLEEP_IN_SECONDS * 1000000);
    }

done:
    if (ds_info->file_buf != NULL)
        free(ds_info->file_buf);

    if (fp != NULL)
        fclose(fp);

    return 0;
}

