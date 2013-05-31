/*
*********************************************************************************************************
*                                               uC/DNSc
*                                     Domain Name Server (client)
*
*                          (c) Copyright 2005-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/DNSc is provided in source form to registered licensees ONLY.  It is
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
*                                    DNS CLIENT CONFIGURATION FILE
*
*                                              TEMPLATE
*
* Filename      : dns-c_cfg.h
* Version       : V1.93.00
* Programmer(s) : JDH
*                 SR
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 DNSc
*********************************************************************************************************
*/

#define  DNSc_CFG_IPPORT                                  53    /* Configure client IP port (default is 53).            */

                                                                /* Configure maximum inactivity time for receive  ...   */
#define  DNSc_CFG_MAX_RX_TIMEOUT_MS                     5000    /* ... in integer milliseconds.                         */

                                                                /* Configure maximum inactivity time for transmit ...   */
#define  DNSc_CFG_MAX_TX_TIMEOUT_MS                     5000    /* ... in integer milliseconds.                         */


/*
*********************************************************************************************************
*                                                TRACING
*********************************************************************************************************
*/

#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                                   0
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO                                  1
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                                   2
#endif

#define  DNSc_TRACE_LEVEL                   TRACE_LEVEL_INFO
#define  DNSc_TRACE                                   printf

