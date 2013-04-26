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
#include "connector_config.h"

#if (defined CONNECTOR_DEBUG)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
/* Would be safer to inlcude just app_cfg.h; but then we miss the TRACE function 
 * custom declaration.
 */
#include <includes.h>

void connector_debug_printf(char const * const format, ...)
{
#if defined(APP_TRACE)		// uCOS-III TRACE macro
    APP_TRACE((char *)format);
#elif defined(APP_CFG_TRACE)	// uCOS-II TRACE macro
    APP_CFG_TRACE((char *)format);
#else
    (void) format;
#endif
}
#else
 /* to avoid ISO C forbids an empty translation unit compiler error */
typedef int dummy;

#endif

