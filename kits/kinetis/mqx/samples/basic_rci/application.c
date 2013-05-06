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

#include "platform.h"
#include "connector.h"

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_DEBUG("idigi_status: status update %d [%s]\n", status, status_message);
}

int application_start(void)
{
    connector_error_t ret;
    int status=-1;

    APP_DEBUG("application_start: calling idigi_connector_start\n");
    ret = connector_start(connector_status);
    if (ret != connector_success)
    {
        APP_DEBUG("idigi_connector_start failed [%d]\n", ret);
        goto error;
    }

    status = 0;

error:
    return status;
}

