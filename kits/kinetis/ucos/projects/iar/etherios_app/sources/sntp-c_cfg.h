/*
*********************************************************************************************************
*                                              uC/SNTPc
*                                Simple Network Time Protocol (client)
*
*                          (c) Copyright 2005-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/SNTPc is provided in source form to registered licensees ONLY.  It is
*               illegal to distribute this source code to any third party unless you receive
*               written permission by an authorized Micrium representative.  Knowledge of
*               the source code may NOT be used to develop a similar product.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                   SNTP CLIENT CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename     : sntp-c_cfg.h
* Version      : V1.89
* Programmer(s): JDH
*                SR
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 SNTPc
*********************************************************************************************************
*/

#define  SNTPc_CFG_IPPORT                                123    /* SNTP client port. Default is 123.                    */

#define  SNTPc_CFG_MAX_RX_TIMEOUT_MS                    5000    /* Maximum inactivity time (ms) on RX.                  */
#define  SNTPc_CFG_MAX_TX_TIMEOUT_MS                    5000    /* Maximum inactivity time (ms) on TX.                  */


/*
*********************************************************************************************************
*                                                TRACING
*********************************************************************************************************
*/
#ifndef TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                                   0
#endif
#ifndef TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO                                  1
#endif
#ifndef TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                                   2
#endif

#define  SNTPc_TRACE_LEVEL                      TRACE_LEVEL_OFF
#define  SNTPc_TRACE                            printf

