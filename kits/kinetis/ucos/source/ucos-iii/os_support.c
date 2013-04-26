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
#include <os_cfg_app.h>
#include <os.h>   
#include <connector_debug.h>
#include "os_support.h"
#include "platform.h"

#include <bsp.h>


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

#if ( !defined LIB_MEM_CFG_ALLOC_EN ) 
#warning "LIB_MEM_CFG_ALLOC_EN not enabled in app_cfg.h !!!!!"
#endif


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
static  OS_TCB   ECC_Run_TaskTCB;
static  CPU_STK  ECC_Run_TaskStk[CONNECTOR_RUN_CFG_TASK_STK_SIZE];  /* Stack for iDigiConnector Run task. */

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
    "idigi_send",
};

OS_FLAG_GRP EventFlagGrpList[ECC_MAX_NUM_EVENTS];

connector_error_t ecc_create_event(int const event)
{
    connector_error_t status = connector_error_event_error;
    OS_ERR err;

    ASSERT_GOTO(event < ECC_MAX_NUM_EVENTS, error);
    
    ASSERT(ECC_MAX_NUM_EVENTS == asizeof(event_list)); //TODO

    OSFlagCreate(&EventFlagGrpList[event],
                 (char*)event_list[event],
                 (OS_FLAGS)0,
                 &err);
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
    OS_ERR err;
   
    OSFlagPost(&EventFlagGrpList[event],
               event_bit,
               (OS_OPT)OS_OPT_POST_FLAG_CLR,
               &err);
    if (err == OS_ERR_NONE)
        status = connector_error_success;

    return status;
}

connector_error_t ecc_get_event(int const event, unsigned long const event_bit, unsigned long timeout_ms)
{
    connector_error_t status = connector_error_event_error;
    OS_TICK timeout_in_ticks;
    OS_ERR err;
    CPU_TS ts;
    
    timeout_in_ticks = timeout_ms * 1000 / OS_CFG_TICK_RATE_HZ;
    
    OSFlagPend(&EventFlagGrpList[event],
               event_bit,
               (OS_TICK )timeout_in_ticks,
               (OS_OPT)OS_OPT_PEND_FLAG_SET_ANY,
               &ts,
               &err);
    if (err == OS_ERR_NONE)
        status = connector_error_success;

    return status;
}

connector_error_t ecc_set_event(int const event, unsigned long const event_bit)
{
    connector_error_t status = connector_error_event_error;
    OS_ERR err;
    
    OSFlagPost(&EventFlagGrpList[event],
               event_bit,
               (OS_OPT)OS_OPT_POST_FLAG_SET,
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
    OS_ERR os_err;
      
    OSTaskCreate((OS_TCB     *)&ECC_Run_TaskTCB,
                 (CPU_CHAR   *)"ECC Run",
                 (OS_TASK_PTR ) ECC_Run_Task,
                 (void       *) 0,
                 (OS_PRIO     ) CONNECTOR_RUN_CFG_TASK_PRIO,
                 (CPU_STK    *)&ECC_Run_TaskStk[0],
                 (CPU_STK_SIZE)(CONNECTOR_RUN_CFG_TASK_STK_SIZE / 10u),
                 (CPU_STK_SIZE) CONNECTOR_RUN_CFG_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) CONNECTOR_RUN_CFG_QUANTA,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&os_err);

      
    if (os_err !=  OS_ERR_NONE)
    {
        APP_DEBUG("Failed to create IDIGI_CONNECTOR_TASK\n");
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
