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
#include <time.h>
#include <errno.h>
#include "connector_api.h"
#include "platform.h"
#include "application.h"
#include "connector_config.h"

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

connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
                                                   connector_request_id_t const request_id,
                                                   void * const data)
{
    connector_callback_status_t   status = connector_callback_continue;

    struct timespec start_time;
    struct timespec end_time;

    get_time(&start_time);
    callback_count++;

//    APP_DEBUG("app_connector_callback: class %d id %d\n", class_id, request_id.config_request);

    switch (class_id)
    {
    case connector_class_id_config:
        status = app_config_handler(request_id.config_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_operating_system:
        status = app_os_handler(request_id.os_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_network_tcp:
        status = app_network_tcp_handler(request_id.network_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_data_service:
        status = app_data_service_handler(request_id.data_service_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_firmware:
        status = app_firmware_handler(request_id.firmware_request, request_data, request_length, response_data, response_length);
        break;

    case connector_class_id_file_system:
        status = app_file_system_handler(request_id.file_system_request, data);
        break;

    case connector_class_id_remote_config:
        status = app_remote_config_handler(request_id.remote_config_request, data);
        break;

    case connector_class_id_status:
        status = app_status_handler(request_id.status_request, data);
        break;

    default:
        status = connector_callback_unrecognized;
        /* not supported */
        break;
    }

    get_time(&end_time);

    total_elapsed_time(&callback_time, start_time, end_time);

    return status;
}

int application_step(connector_handle_t handle)
{
#if defined CONNECTOR_DATA_SERVICE
    send_put_request(handle);
#else
    UNUSED_ARGUMENT(handle);
#endif
    return 0;
}

