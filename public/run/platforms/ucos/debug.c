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
#include "connector_config.h"

#if (defined CONNECTOR_DEBUG)
#include <connector.h>
#include <connector_bsp.h>

void connector_debug_printf(char const * const format, ...)
{
    connector_BSP_debug_printf(format);
}
#else
 /* to avoid ISO C forbids an empty translation unit compiler error */
typedef int dummy;

#endif

