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
#include <stdlib.h>
#include "connector_api.h"
#include "platform.h"

int main (void)
{
    connector_handle_t connector_handle;

    APP_DEBUG("main: Starting iDigi\n");
    /* TODO: Initialized iik by calling connector_init
     * with a callback. Replace connector_callback with
     * your callback function or add connector_callback.
     *
     */
    connector_handle = connector_init(app_connector_callback);
    if (connector_handle == NULL)
    {
        APP_DEBUG("main: connector_init() failed\n");
        goto done;
    }

    /* Sample program control loop */
    for(;;)
    {
        connector_status_t status;
        /*
         * Example of using connector_step(), we run a portion of the iDigi Connector then run
         * a portion of the users application.
         */
        status = connector_step(connector_handle);
        if (status != connector_idle && status != connector_working && status != connector_pending &&
            status != connector_active && status != connector_open_error)
        {
            APP_DEBUG("main: connector_step() failed\n");
            break;
        }

        /* TODO: execute other processes
         *
         */
    }

done:
    return 0;
}
