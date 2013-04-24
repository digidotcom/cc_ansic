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

#include "connector_api.h"
#include "platform.h"

int connector_snprintf(char * const str, size_t const size, char const * const format, ...)
{
    int result = 0;

    UNUSED_ARGUMENT(str);
    UNUSED_ARGUMENT(size);
    UNUSED_ARGUMENT(format);
    return result;
}

connector_callback_status_t app_os_malloc(size_t const size, void ** ptr)
{
    UNUSED_ARGUMENT(size);
    UNUSED_ARGUMENT(ptr);
    return connector_callback_continue;
}

connector_callback_status_t app_os_free(void const * const ptr)
{
    UNUSED_ARGUMENT(ptr);
    return connector_callback_continue;
}

connector_callback_status_t app_os_get_system_time(unsigned long * const uptime)
{
    UNUSED_ARGUMENT(uptime);
    return connector_callback_continue;
}

connector_callback_status_t app_os_yield(connector_status_t const * const status)
{
    UNUSED_ARGUMENT(status);
    return connector_callback_continue;
}

static connector_callback_status_t app_os_reboot(void)
{

    APP_DEBUG("Reboot\n");
    /* should not return from rebooting the system */
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

            status    = app_os_malloc(*bytes, response_data);
        }
        break;

    case connector_os_free:
        status = app_os_free(request_data);
        break;

    case connector_os_system_up_time:
        status    = app_os_get_system_time(response_data);
        break;

    case connector_os_yield:
        status = app_os_yield(request_data);
        break;

    case connector_os_reboot:
        status = app_os_reboot();
        break;

    default:
        status = connector_callback_unrecognized;
        break;
    }

    return status;
}
/**
 * @endcond
 */


