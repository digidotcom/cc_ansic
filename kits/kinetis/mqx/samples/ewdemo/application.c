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
#include <bsp.h>
#include <lwevent.h>
#include <io_gpio.h>
#include "platform.h"
#include "connector.h"

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_DEBUG("connector_status: status update %d [%s]\n", status, status_message);
}

int application_start(void)
{
    connector_dataservice_data_t ecc_data;
    connector_error_t ret;
    int status=-1;

    APP_DEBUG("application_start: calling connector_start\n");
    ret = connector_start(connector_status);
    if (ret != connector_error_success)
    {
    	APP_DEBUG("connector_start failed [%d]\n", ret);
        goto error;
    }
	
    initialize_k60_tower_demo();
 
    status = 0;

error:
    return status;
    
}
