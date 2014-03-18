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
 /**
  * @file
  *  @brief Debug routine for the Cloud Connector.
  *
  */

#include "connector_config.h"

#if (defined CONNECTOR_DEBUG)
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "connector_debug.h"

/**
 * @brief Cloud Connector debug
 *
 * Debug output from Cloud Connector, Writes a formatted string to stdout, expanding the format
 * tags with the value of the argument list arg.
 * Cloud Connector uses this routine to display debug information when @ref CONNECTOR_DEBUG is defined.
 *
 * @param [in] format Tells how to format the various arguments
 * @param [out] ...   A variable argument list of expressions whose values should be printed according
 *                    to the placeholders in the "format" string. If there are more placeholders than
 *                    supplied arguments, the result is undefined. If there are more arguments than
 *                    placeholders, the excess arguments are simply ignored.
 *
 * @see @ref CONNECTOR_DEBUG
 */
void connector_debug_vprintf(debug_t const debug, char const * const format, va_list args)
{
    if ((debug == debug_all) || (debug == debug_beg))
    {
        /* lock mutex here. */
        printf("CC: ");
    }

    vprintf(format, args);

    if ((debug == debug_all) || (debug == debug_end))
    {
        /* unlock mutex here */
        printf("\n");
        fflush(stdout);
    }
}

#else
/* to avoid ISO C forbids an empty translation unit compiler error */
typedef int dummy;

#endif

