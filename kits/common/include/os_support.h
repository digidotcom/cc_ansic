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

#ifndef _OS_SUPPORT_H
#define _OS_SUPPORT_H

#include <stdlib.h>
#include "connector.h"

typedef struct
{
    connector_status_callback_t            status_callback;
    connector_device_request_callback_t    device_request;
    connector_device_response_callback_t   device_response;
    connector_firmware_download_callback_t device_download;
    connector_reset_callback_t             device_reset;
} connector_callbacks_t;

#define ECC_SEND_DATA_EVENT 	0
#define ECC_DATA_POINTS_EVENT 	1
#define ECC_MAX_NUM_EVENTS  	1

connector_error_t ecc_create_event(int const event);
connector_error_t ecc_set_event(int const event, unsigned long const event_bit);
connector_error_t ecc_get_event(int const event, unsigned long const event_bit, unsigned long timeout);
connector_error_t ecc_clear_event(int const event, unsigned long const event_bit);
connector_error_t ecc_create_thread(void);
void ecc_free(void *ptr);
void *ecc_malloc(size_t size);
void ecc_software_reset(void);
void ecc_watchdog_reset(void);

void connector_thread(unsigned long initial_data);
extern connector_callbacks_t * connector_get_app_callbacks(void);
extern unsigned long start_system_up_time;

#define UNUSED_PARAMETER(x)     ((void)x)

#define ON_FALSE_DO_(cond, code)        do { if (!(cond)) {code;} } while (0)

#if defined(CONNECTOR_DEBUG)
#define ON_ASSERT_DO_(cond, code, output)   ON_FALSE_DO_((cond), {ASSERT(cond); code;})
#else
#define ON_ASSERT_DO_(cond, code, output)   ON_FALSE_DO_((cond), {code})
#endif

#define ASSERT_GOTO(cond, label)    ON_ASSERT_DO_((cond), {goto label;}, {})
#define CONFIRM(cond)               do { switch(0) {case 0: case (cond):;} } while (0)

#endif



