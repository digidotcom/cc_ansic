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

#include <mqx.h>
#include "connector_api.h"
#include "platform.h"
#include "connector_debug.h"

int connector_malloc_failures = 0;
unsigned long start_system_up_time = 0;

connector_callback_status_t app_os_malloc(size_t const size, void ** ptr)
{
    connector_callback_status_t status = connector_callback_abort;

    *ptr = _mem_alloc(size);
    if (*ptr != NULL)
    {
        status = connector_callback_continue;
    }
    else
    {
        APP_DEBUG ("os_malloc: failed\n");
        connector_malloc_failures ++;
    }

    return status;
}

connector_callback_status_t app_os_free(void * const ptr)
{
    ASSERT(ptr != NULL);
    if (ptr != NULL)
    {
        unsigned int const result = _mem_free(ptr);

        if (result)
        {
            APP_DEBUG("os_free: _mem_free failed [%d]\n");
        }
    }

    return connector_callback_continue;
}

connector_callback_status_t app_os_get_system_time(unsigned long * const uptime)
{
    TIME_STRUCT curtime;

    _time_get(&curtime);
    if (start_system_up_time == 0)
        start_system_up_time = curtime.SECONDS;
 
    /* Up time in seconds */
    *uptime = curtime.SECONDS - start_system_up_time;

    return connector_callback_continue;
}

connector_callback_status_t app_os_yield(connector_status_t const * const status)
{
    if (*status == connector_idle)
    {
        unsigned int const timeout_in_microseconds =  1000000;
        usleep(timeout_in_microseconds);
    }
    gStatus = *status;

    return connector_callback_continue;
}

static connector_callback_status_t app_os_reboot(void)
{

    APP_DEBUG("Reboot from server\n");
    /* should not return from rebooting the system */
    return connector_callback_continue;
}


connector_callback_status_t app_os_handler(connector_os_request_t const request,
                                        void const * const request_data, size_t const request_length,
                                        void * const response_data, size_t * const response_length)
{
    connector_callback_status_t status;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {
    case connector_os_malloc:
        status = app_os_malloc(*((size_t *)request_data), response_data);
        break;

    case connector_os_free:
        app_os_free((void * const)request_data);
        status = connector_callback_continue;
        break;

    case connector_os_system_up_time:
        status = app_os_get_system_time(response_data);
        break;

    case connector_os_yield:
        status = app_os_yield(request_data);
        break;

    case connector_os_reboot:
        status = app_os_reboot();
        break;

    default:
        APP_DEBUG("app_os_handler: unrecognized request [%d]\n", request);
        status = connector_callback_unrecognized;
        break;
    }

    return status;
}




