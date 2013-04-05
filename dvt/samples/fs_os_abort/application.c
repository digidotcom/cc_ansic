/*
 * Copyright (c) 2012, 2013 Digi International Inc.,
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
#include "connector_api.h"
#include "platform.h"


connector_bool_t app_connector_reconnect(connector_class_id_t const class_id, connector_close_status_t const status)
{
    UNUSED_ARGUMENT(class_id);

    connector_bool_t type;

    switch (status)
    {
       /* if either the server or our application cuts the connection, don't reconnect */
       case connector_close_status_device_terminated:
       case connector_close_status_device_stopped:
             type = connector_false;
             break;

       /* otherwise it's an error and we want to retry */
        default:
             type = connector_true;
             break;
    }

    return type;
}

typedef enum
{
    action_none,
    return_abort,
    return_busy,
    return_error,
    return_unrecognized,
    return_invalid_code,
    set_bad_resp_length,
    set_null_data,
    set_zero_datalength,
    set_invalid_datalength,
    set_invalid_data,
    action_end_test

} test_action_t;

typedef enum
{
    test_type_none,
    test_type_os,
    test_type_config_notify

} dvt_test_t;

typedef struct
{
    char name[256];
    int  callback_cnt;
    dvt_test_t test_type;

} dvt_data_t;

static dvt_data_t dvt_data = {"", 0, test_type_none};

typedef struct
{

    char * name;
    int    callback_cnt;
    connector_os_request_t callback_id;
    test_action_t  action;

} dvt_test_data_t;


static dvt_test_data_t test_table[] = {

    {"dvt_os_malloc_abort1",       1, connector_os_malloc, return_abort},
    {"dvt_os_malloc_abort2",       2, connector_os_malloc, return_abort},
    {"dvt_os_malloc_null2",        2, connector_os_malloc, set_null_data},
    {"dvt_os_free_abort1",         1, connector_os_free, return_abort},
    {"dvt_os_free_abort2",         2, connector_os_free, return_abort},
    {"dvt_os_sysuptime_abort",      1, connector_os_system_up_time, return_abort},
    {"dvt_os_sysuptime_invalid",    1, connector_os_system_up_time, return_invalid_code},
    {"dvt_os_yeld_abort",           1, connector_os_yield, return_abort},
    {"dvt_os_reboot_abort",         1, connector_os_reboot, return_abort},
    {"dvt_os_notify_abort",         2, connector_os_malloc, set_null_data},
    {"dvt_config_notify_abort",     1, connector_config_error_status, return_abort},

    {"dvt_os_malloc_busy2",        2, connector_os_malloc, return_busy},
    {"dvt_os_reboot_busy",         1, connector_os_reboot, return_busy},
};

static void dvt_init_test(char const * name)
{


    dvt_data.callback_cnt = 0;
    if (strstr(name, "dvt_os_"))
    {
        strcpy(dvt_data.name, name);
        dvt_data.test_type = test_type_os;
    }
    else
    if (strstr(name, "dvt_config_notify_"))
    {
        strcpy(dvt_data.name, name);
        dvt_data.test_type = test_type_config_notify;
    }

    APP_DEBUG("dvt_init_test: %s\n", name);
}

static void dvt_cleanup_test(void)
{
    dvt_data.test_type = test_type_none;
    APP_DEBUG("dvt_cleanup_test\n");
}


static void dvt_new_test(connector_file_system_request_t const request_id,
                         void const * const request_data)
{

    if (dvt_data.test_type == test_type_none && request_id == connector_file_system_open)
    {
        connector_file_path_request_t const * file_request = (connector_file_path_request_t  *)request_data;
        char const * ptr;

        ptr = strstr(file_request->path, "dvt_os_");
        if (ptr == NULL)
            ptr = strstr(file_request->path, "dvt_config_");
        if (ptr != NULL)
            dvt_init_test(ptr);
    }
}


static test_action_t dvt_find_test(connector_os_request_t const request_id, void const * const request_data)
{

    test_action_t action = action_none;
    unsigned int i;

    UNUSED_ARGUMENT(request_data);

    if (dvt_data.test_type == test_type_none)
        goto done;

    for (i = 0; i < sizeof test_table / sizeof test_table[0]; i++)
    {
        if ((test_table[i].callback_id == request_id) &&
            strcmp(dvt_data.name, test_table[i].name) == 0)
        {
            dvt_data.callback_cnt++;
            if (dvt_data.callback_cnt == test_table[i].callback_cnt)
            {
                action = test_table[i].action;
            }
            else
            if (dvt_data.callback_cnt > test_table[i].callback_cnt)
            {
                action = action_end_test;
            }
            APP_DEBUG("dvt_find_test,  request_id %d, action %d, callback_cnt %d\n", request_id, action, dvt_data.callback_cnt);
            break;
        }
    }

done:
    return action;
}


static int dvt_os_pre_test(connector_os_request_t const request_id,
                        void const * const request_data,
                        void * const response_data,
                        size_t * const response_length,
                        connector_callback_status_t * status)
{
    int result = -1;

    test_action_t action = dvt_find_test(request_id, request_data);

    UNUSED_ARGUMENT(request_data);
    UNUSED_ARGUMENT(response_data);

    switch (action)
    {
        case return_abort:
            *status = connector_callback_abort;
            break;

        case return_busy:
            *status = connector_callback_busy;
            break;

        case return_unrecognized:
            *status = connector_callback_unrecognized;
            break;

        case return_error:
            *status = connector_callback_error;
            break;

        case return_invalid_code:
            *status = (connector_callback_status_t) -1;
            break;

        case set_bad_resp_length:
            (*response_length)--;
            break;

        case set_null_data:
            if (request_id == connector_os_malloc)
            {
                void ** ptr = (void **)response_data;
                *ptr = NULL;

                if (strcmp(dvt_data.name, "dvt_os_notify_abort") == 0)
                {
                    strcpy(dvt_data.name, "dvt_config_notify_abort");
                    dvt_data.test_type = test_type_config_notify;
                    dvt_data.callback_cnt = 0;
                    goto done;
                }
            }
            break;

        case action_none:
        case action_end_test:
        default:
            result = 0;
    }

    if (dvt_data.test_type != test_type_none &&
        action !=  action_none && action !=  return_busy)
    {
        dvt_cleanup_test();
    }

done:
    return result;
}

connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
                                                   connector_request_id_t const request_id,
                                                   void * const data)
{
    connector_callback_status_t   status = connector_callback_continue;

    switch (class_id)
    {
    case connector_class_id_config:
        if (dvt_data.test_type == test_type_config_notify &&
            dvt_os_pre_test(request_id.config_request, data &status) != 0)
        {
            break;
        }
        status = app_config_handler(request_id.config_request, data);
        break;

    case connector_class_id_operating_system:
        if (dvt_data.test_type == test_type_os &&
            dvt_os_pre_test(request_id.os_request, data, &status) != 0)
        {
            break;
        }
        status = app_os_handler(request_id.os_request, data);
        break;

    case connector_class_id_network_tcp:
        status = app_network_tcp_handler(request_id.network_request, data);
        break;

    case connector_class_id_file_system:
        dvt_new_test(request_id.file_system_request, request_data);
        status = app_file_system_handler(request_id.file_system_request, data);
         break;

    case connector_class_id_status:
        status = app_status_handler(request_id.status_request, data);
        break;

    default:
        /* not supported */
        break;
    }
    return status;
}

int application_run(connector_handle_t handle)
{
    UNUSED_ARGUMENT(handle);

    return 0;
}


