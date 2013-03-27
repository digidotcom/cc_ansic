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

#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include "connector_api.h"
#include "platform.h"
#include <linux/reboot.h>
#include <sys/reboot.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int connector_snprintf(char * const str, size_t const size, char const * const format, ...)
{
    va_list args;
    int result;

    va_start(args, format);

    #if __STDC_VERSION__ >= 199901L
    result = vsnprintf(str, size, format, args);
    #else
    {
        #define SAFE_BUFFER_BYTES 64;
        size_t const max_format_bytes = 12;

        CONFIRM(strlen(format) < max_format_bytes);
        if (size >= SAFE_BUFFER_BYTES)
        {
            result = vsprintf(str, format, args);
        }
        else
        {
            char local_buffer[SAFE_BUFFER_BYTES];
            size_t const bytes_needed = vsprintf(local_buffer, format, args);

            result = (bytes_needed < size) ? bytes_needed : size - 1;
            memcopy(str, local_buffer, result);
            str[result] = '\0';
            result = bytes_needed;
        }
        #undef SAFE_BUFFER_BYTES
    }
    #endif
    va_end(args);

    return result;
}

connector_callback_status_t app_os_malloc(size_t const size, void ** ptr)
{
    connector_callback_status_t status = connector_callback_abort;

    *ptr = malloc(size);
    if (*ptr != NULL)
    {
        status = connector_callback_continue;
    }
    else
    {
        APP_DEBUG("app_os_malloc: Failed to malloc\n");
    }

    return status;
}

connector_callback_status_t app_os_free(void const * const ptr)
{
    void * const free_ptr = (void *)ptr;

    if (free_ptr != NULL)
    {
        free(free_ptr);
    }
    else
    {
        APP_DEBUG("app_os_free: called with NULL\n");
    }

    return connector_callback_continue;
}

time_t start_system_up_time;

connector_callback_status_t app_os_get_system_time(unsigned long * const uptime)
{
	time_t present_time;

    time(&present_time);

    if (start_system_up_time == 0)
       start_system_up_time = present_time;

    present_time -= start_system_up_time;
	*uptime = (unsigned long) present_time;

    return connector_callback_continue;
}

connector_callback_status_t app_os_yield(connector_status_t const * const status)
{
    if (*status == connector_idle)
    {
        unsigned int const timeout_in_microseconds =  1000000;
        usleep(timeout_in_microseconds);
    }

    return connector_callback_continue;
}

static connector_callback_status_t app_os_reboot(void)
{

    /* Note: we must be running as the superuser to reboot the system */
    sync();
    reboot(LINUX_REBOOT_CMD_RESTART);
    return connector_callback_continue;
}

connector_callback_status_t app_os_handler(connector_os_request_t const request,
                                        void const * const request_data, size_t const request_length,
                                        void * response_data, size_t * const response_length)
{
    connector_callback_status_t status;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {
    case connector_os_malloc:
        {
            size_t const * const bytes = request_data;

            status = app_os_malloc(*bytes, response_data);
        }
        break;

    case connector_os_free:
        status = app_os_free(request_data);
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


