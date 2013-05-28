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
#include <app_cfg.h>
#include <ucos_ii.h> 
#include <connector_debug.h>
#include "os_support.h"
#include "platform.h"
#include <connector_bsp.h>

/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/* TODO Supervise: connector documentation claims:
 * Program Stack Usage: 
 * The following program stack usage was observed using gcc 4.2.0 compiled for a 32-bit ARM9 TDMI architecture
 * IDIGI_DEBUG disabled: 1723
 * IDIGI_DEBUG disabled: 2091
 */

/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/
static  OS_STK  ECC_Run_TaskStk[CONNECTOR_RUN_CFG_TASK_STK_SIZE];  /* Stack for iDigiConnector Run task. */

int idigi_malloc_failures = 0;

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  ECC_Run_Task  (void *p_arg);                        /* iDigiConnector Run task. */


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

static char const * event_list[] =
{
    "connector_send",
    "connector_data_point"
};

OS_FLAG_GRP EventFlagGrpList[ECC_MAX_NUM_EVENTS];

connector_error_t ecc_create_event(int const event)
{
    connector_error_t status = connector_error_event_error;
    INT8U err;

    ASSERT_GOTO(event < ECC_MAX_NUM_EVENTS, error);
    
    ASSERT(ECC_MAX_NUM_EVENTS == asizeof(event_list)); //TODO

    EventFlagGrpList[event] = *OSFlagCreate(0, &err);
    if (err != OS_ERR_NONE)
    {
        APP_DEBUG("Failed to create %s event\n", event_list[event]);
        goto error;
    }

    status = connector_error_success;

error:
    return status;
}

connector_error_t ecc_clear_event(int const event, unsigned long const event_bit)
{
    connector_error_t status = connector_error_event_error;
    INT8U err;
   
    OSFlagPost(&EventFlagGrpList[event],
               event_bit,
               OS_FLAG_CLR,
               &err);
    if (err == OS_ERR_NONE)
        status = connector_error_success;

    return status;
}

connector_error_t ecc_get_event(int const event, unsigned long const event_bit, unsigned long timeout_ms)
{
    connector_error_t status = connector_error_event_error;
#if (OS_VERSION >= 287u)
    INT32U timeout_in_ticks;
#else
    INT16U timeout_in_ticks;
#endif
    INT8U err;
    
    timeout_in_ticks = timeout_ms * 1000 / OS_TICKS_PER_SEC;
    
    OSFlagPend(&EventFlagGrpList[event],
               event_bit,
               timeout_in_ticks,
               OS_FLAG_WAIT_SET_ANY,
               &err);
    if (err == OS_ERR_NONE)
        status = connector_error_success;

    return status;
}

connector_error_t ecc_set_event(int const event, unsigned long const event_bit)
{
    connector_error_t status = connector_error_event_error;
    INT8U err;
    
    OSFlagPost(&EventFlagGrpList[event],
               event_bit,
               OS_FLAG_SET,
               &err);
    if (err == OS_ERR_NONE)
        status = connector_error_success;

    return status;
}

/* Even if IDIGI_NO_MALLOC is defined in connector_config.h
 * some platform files will use this function to get 
 * memory
 */
void * ecc_malloc(size_t size)
{
    void * ptr = NULL;
    
    ptr = (void *)malloc(size);
    
    if (ptr == NULL)
    {
        APP_DEBUG ("ecc_malloc: failed\n");
        idigi_malloc_failures ++;
    }
    
    return ptr;
}

void ecc_free(void * ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
    }
    else
    {
        APP_DEBUG("Free called with NULL pointer\n");
    }
}

void ecc_software_reset(void)
{
    // uCOS doen't have any Reboot command. Tell BSP to do that.
    Connector_BSP_software_reset();

    while (1) {}
}

void ecc_watchdog_reset(void)
{
    // uCOS doen't have any Reboot command. Tell BSP to do that.
    Connector_BSP_watchdog_reset();
}

connector_error_t ecc_create_thread(void)
{
    connector_error_t status = connector_error_success;
    INT8U os_err;

#if OS_TASK_CREATE_EXT_EN > 0u
    #if OS_STK_GROWTH == 1u
    os_err=OSTaskCreateExt(ECC_Run_Task,
                          (void *)0,                                       /* No arguments passed to OSTmrTask()      */
                          &ECC_Run_TaskStk[CONNECTOR_RUN_CFG_TASK_STK_SIZE - 1u],  /* Set Top-Of-Stack                        */
                          CONNECTOR_RUN_CFG_TASK_PRIO,
                          CONNECTOR_RUN_CFG_TASK_PRIO,
                          &ECC_Run_TaskStk[0],                               /* Set Bottom-Of-Stack                     */
                          CONNECTOR_RUN_CFG_TASK_STK_SIZE,
                          (void *)0,                                       /* No TCB extension                        */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);      /* Enable stack checking + clear stack     */
    #else
    os_err=OSTaskCreateExt(ECC_Run_Task,
                          (void *)0,                                       /* No arguments passed to OSTmrTask()      */
                          &ECC_Run_TaskStk[0],                              /* Set Top-Of-Stack                        */
                          CONNECTOR_RUN_CFG_TASK_PRIO,
                          CONNECTOR_RUN_CFG_TASK_PRIO,
                          &ECC_Run_TaskStk[CONNECTOR_RUN_CFG_TASK_STK_SIZE - 1u],  /* Set Bottom-Of-Stack                     */
                          CONNECTOR_RUN_CFG_TASK_STK_SIZE,
                          (void *)0,                                       /* No TCB extension                        */
                          OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);      /* Enable stack checking + clear stack     */
    #endif
#else
    #if OS_STK_GROWTH == 1u
    os_err=OSTaskCreate(ECC_Run_Task,
                       (void *)0,
                       &ECC_Run_TaskStk[CONNECTOR_RUN_CFG_TASK_STK_SIZE - 1u],
                       CONNECTOR_RUN_CFG_TASK_PRIO);
    #else
    os_err=OSTaskCreate(ECC_Run_Task,
                       (void *)0,
                       &ECC_Run_TaskStk[0],
                       CONNECTOR_RUN_CFG_TASK_PRIO);
    #endif
#endif
        
    if (os_err !=  OS_ERR_NONE)
    {
        APP_DEBUG("Failed to create CONNECTOR_TASK\n");
        status = connector_error_failed_to_create_thread;
    }
  
    return status;
}

/*
*********************************************************************************************************
*                                        ECC_Run_Task()
*
* Description : Receives and transmits packets.
*
* Argument(s) : p_arg       Argument passed to ECC_Run_Task() by 'OSTaskCreate()'.
*
* Return(s)   : none.
*
* Caller(s)   : This is a task.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static  void  ECC_Run_Task (void *p_arg)
{    
    connector_thread((unsigned long)p_arg);
}
