/*
 * Copyright (c) 2013 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
#include "platform.h"
#include "connector_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define DEBUG_MAX_LINE_LEN 160

static char buf[DEBUG_MAX_LINE_LEN + 1];
static unsigned long seconds_since_reset;

#ifdef VERBOSE
static unsigned long days = 0, hours = 0, minutes = 0, seconds = 0;
#endif

#if defined(CONNECTOR_DEBUG)
void connector_debug_printf(char const * const format, ...)
{
    va_list args;

    va_start(args, format);
    vsnprintf(buf, DEBUG_MAX_LINE_LEN, format, args);
    app_os_get_system_time(&seconds_since_reset);

    days    = seconds_since_reset / (60 * 60 * 24);
    hours   = (seconds_since_reset / (60 * 60)) % 24;
    minutes = (seconds_since_reset / 60) % 60;
    seconds = seconds_since_reset % 60;

#ifdef VERBOSE
    _io_printf("days[%d] hr[%d] min[%d] sec[%d] ", days, hours, minutes, seconds);
#else
    _io_printf("[%d] ", seconds_since_reset);        
#endif
    _io_printf(buf);
    va_end(args);
}

#endif

void log_printf(const char * const msg, ...)
{	
    if (msg)
    {        
        va_list args;
        
        va_start(args, msg);
        vsnprintf(buf, DEBUG_MAX_LINE_LEN, msg, args);
        
        app_os_get_system_time(&seconds_since_reset);

#ifdef VERBOSE
        days    = seconds_since_reset / (60 * 60 * 24);
        hours   = (seconds_since_reset / (60 * 60)) % 24;
        minutes = (seconds_since_reset / 60) % 60;
        seconds = seconds_since_reset % 60;

        printf("days[%d] hr[%d] min[%d] sec[%d] ", days, hours, minutes, seconds);
#else
        printf("[%d] ", seconds_since_reset);        
#endif
        printf(buf);
        va_end(args);
    }
}
