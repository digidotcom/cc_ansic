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
#include <stdlib.h>
#include <main.h>
#include "os_support.h"
#include "connector_config.h"
#include "platform.h"

static char const * event_list[] =
{
    "idigi_send"
};

static size_t const event_list_size = asizeof(event_list);

connector_error_t ecc_create_event(int const event)
{
    connector_error_t status = connector_error_event_error;

    ASSERT_GOTO(event < event_list_size, error);
    if (_event_create(event_list[event]) != MQX_OK)
    {
        APP_DEBUG("ecc_create_event: failed on %s event\n", event_list[event]);
        goto error;
    }

    status = connector_error_success;

error:
    return status;
}

connector_error_t ecc_clear_event(int const event, unsigned long const event_bit)
{
    pointer event_ptr;
    connector_error_t status = connector_error_event_error;

    ASSERT_GOTO(event < event_list_size, error);
    if (_event_open(event_list[event], &event_ptr) != MQX_OK)
    {
        APP_DEBUG("ecc_clear_event: _event_open failed on event %s\n", event_list[event]);
        goto error;
    }

    if (_event_clear(event_ptr, event_bit) != MQX_OK)
    {
        APP_DEBUG("ecc_clear_event: _event_clear failed on event %s\n", event_list[event]);
        goto error;
    }

    if (_event_close(event_ptr) != MQX_OK)
    {
        APP_DEBUG("ecc_clear_event: _event_close failed on event %s\n", event_list[event]);
        goto error;
    }

    status = connector_error_success;

error:
    return status;
}

connector_error_t ecc_get_event(int const event, unsigned long const event_bit, unsigned long timeout_ms)
{
    pointer event_ptr;
    connector_error_t status = connector_error_event_error;

    ASSERT_GOTO(event < event_list_size, error);
    if (_event_open(event_list[event], &event_ptr) != MQX_OK)
    {
        APP_DEBUG("ecc_get_event: _event_open failed on event %s\n", event_list[event]);
        goto error;
    }

    if (_event_wait_all(event_ptr, event_bit, timeout_ms) != MQX_OK)
    {
        APP_DEBUG("ecc_get_event: _event_wait_all failed on event %s\n", event_list[event]);
        goto error;
    }

    if (_event_clear(event_ptr, event_bit) != MQX_OK)
    {
        APP_DEBUG("ecc_get_event: _event_clear failed to clear event %s\n", event_list[event]);
        goto error;
    }

    if (_event_close(event_ptr) != MQX_OK)
    {
        APP_DEBUG("ecc_get_event: _event_close failed to close event %s\n", event_list[event]);
        goto error;
    }

    status = connector_error_success;

error:
    return status;
}

connector_error_t ecc_set_event(int const event, unsigned long const event_bit)
{
    pointer event_ptr;
    connector_error_t status = connector_error_event_error;

    ASSERT_GOTO(event < event_list_size, error);
    if (_event_open(event_list[event], &event_ptr) != MQX_OK)
    {
        APP_DEBUG("ecc_set_event: _event_open failed on event(get) %s\n", event_list[event]);
        goto error;
    }

    if (_event_set(event_ptr, event_bit) != MQX_OK)
    {
        APP_DEBUG("ecc_set_event: _event_set failed on event %s\n", event_list[event]);
        goto error;
    }

    if (_event_close(event_ptr) != MQX_OK)
    {
        APP_DEBUG("ecc_set_event: _event_close failed on event %s\n", event_list[event]);
        goto error;
    }

    status = connector_error_success;

error:
    return status;
}

void * ecc_malloc(size_t size)
{
    return _mem_alloc(size);
}

void ecc_free(void * ptr)
{
    if (ptr != NULL)
    {
        _mem_free(ptr);
    }
    else
    {
        APP_DEBUG("Free called with NULL pointer\n");
    }
}

unsigned long ecc_create_task(unsigned long const index_number, unsigned long const parameter)
{
    uint32_t task_id;

    task_id = _task_create(0, index_number, parameter);
 
    return task_id;
}

unsigned long ecc_destroy_task(unsigned long const task_id)
{
    unsigned long status;

    status = _task_destroy(task_id);

    return status;
}

void ecc_software_reset(void)
{
    #define VECTKEY  0x05FA0000

    // Issue a System Reset Request
    SCB_AIRCR = VECTKEY | SCB_AIRCR_SYSRESETREQ_MASK;

    while (1) {}
}

void ecc_watchdog_reset(void)
{
    /* Issue a watchdog */

    /* disable all interrupts */
    asm(" CPSID i");

    /* Write 0xC520 to the unlock register */ WDOG_UNLOCK = 0xC520;
 
    /* Followed by 0xD928 to complete the unlock */ WDOG_UNLOCK = 0xD928;
 
    /* enable all interrupts */
    asm(" CPSIE i");
 
    /* Clear the WDOGEN bit to disable the watchdog */ //WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;

    WDOG_STCTRLH != WDOG_STCTRLH_WDOGEN_MASK;
}

connector_error_t ecc_create_thread(void)
{
    connector_error_t status = connector_error_success;

    if (ecc_create_task(CONNECTOR_TASK, 0) == MQX_NULL_TASK_ID)
    {
        APP_DEBUG("Failed to create CONNECTOR_CONNECTOR_TASK\n");
        status = connector_error_failed_to_create_thread;
    }

    return status;
}

