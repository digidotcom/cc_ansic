/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                              (c) Copyright 2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      APPLICATION CONFIGURATION
*
*                                        Freescale Kinetis Kxx
*                                               on the
*
*                                       Freescale TWR-KxxN512
*                                          Evaluation Board
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

/*
*********************************************************************************************************
*                                       MODULE ENABLE / DISABLE
*********************************************************************************************************
*/

// Application Filesystem will only be available if CONNECTOR_FILE_SYSTEM is defined in connector_config.h
#ifdef CONNECTOR_FILE_SYSTEM
#define  APP_CFG_FS_EN                         DEF_ENABLED
#endif
#define  APP_CFG_DNS                           DEF_ENABLED


/*
*********************************************************************************************************
*                              STATIC IP CONFIGURATION AND SERIAL BAUDRATE
*********************************************************************************************************
*/

#define  APP_CFG_IP_MODE_DHCP                  DEF_ENABLED

#if (APP_CFG_IP_MODE_DHCP == DEF_DISABLED)
#define  APP_CFG_IP_ADDRESS_STR                "10.101.1.129"
#define  APP_CFG_IP_MASK_STR                   "255.255.255.0"
#define  APP_CFG_IP_GATEWAY_STR                "10.101.1.1"
#define  APP_CFG_IP_DNS_SRVR_STR               "10.49.8.62"
#endif

#define  APP_CFG_SERIAL_BAUDRATE               115200


/*
*********************************************************************************************************
*                                            BSP CONFIGURATION
*********************************************************************************************************
*/

#define  BSP_CFG_LED_SPI2_EN                    DEF_ENABLED     /* Enable/disable LEDs on SPI port.                     */
#define  BSP_CFG_LED_PIOC_EN                    DEF_ENABLED     /* Enable/disable PIOC LEDs.                            */


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/

#define  APP_TASK_START_PRIO                              2u

#define  NET_OS_CFG_TMR_TASK_PRIO                         8u
#define  NET_OS_CFG_IF_TX_DEALLOC_TASK_PRIO               9u
#define  NET_OS_CFG_IF_RX_TASK_PRIO                      10u
#define  NET_OS_CFG_IF_LOOPBACK_TASK_PRIO                11u
#define  CLK_OS_CFG_TASK_PRIO                             6u

#define  CONNECTOR_RUN_CFG_TASK_PRIO                     15u

/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*                             Size of the task stacks (# of OS_STK entries)
*********************************************************************************************************
*/

#define  APP_TASK_START_STK_SIZE                        (1u * 512u)

#define  NET_OS_CFG_TMR_TASK_STK_SIZE                   512u
#define  NET_OS_CFG_IF_TX_DEALLOC_TASK_STK_SIZE         128u
#define  NET_OS_CFG_IF_RX_TASK_STK_SIZE                 512u

#define  CLK_OS_CFG_TASK_STK_SIZE                       512

#define  CONNECTOR_RUN_CFG_TASK_STK_SIZE                (2u* 512u)

/*
*********************************************************************************************************
*                                          uC/Probe CONFIGURATION
*********************************************************************************************************
*/

#define  APP_CFG_PROBE_OS_PLUGIN_EN            DEF_DISABLED
#define  APP_CFG_PROBE_COM_EN                  DEF_DISABLED


/*
*********************************************************************************************************
*                                            uC/TCP-IP v2.0
*********************************************************************************************************
*/

#define  NET_OS_CFG_IF_LOOPBACK_Q_SIZE                    5u
#define  NET_OS_CFG_IF_RX_Q_SIZE                         20u
#define  NET_OS_CFG_IF_TX_DEALLOC_Q_SIZE                 20u

/*
*********************************************************************************************************
*                                       uC/FS: DRIVER CONFIGURATION
*********************************************************************************************************
*/

//TODO: use fs_app_cfg.h

#define  APP_CFG_FS_DEV_CNT                               1
#define  APP_CFG_FS_VOL_CNT                               1
#define  APP_CFG_FS_FILE_CNT                              1
#define  APP_CFG_FS_DIR_CNT                               1
#define  APP_CFG_FS_BUF_CNT                       (2 * APP_CFG_FS_VOL_CNT)
#define  APP_CFG_FS_DEV_DRV_CNT                           1
#define  APP_CFG_FS_WORKING_DIR_CNT                       0
#define  APP_CFG_FS_MAX_SEC_SIZE                        512




#define  APP_CFG_FS_IDE_EN                         DEF_DISABLED
#define  APP_CFG_FS_MSC_EN                         DEF_DISABLED
#define  APP_CFG_FS_NAND_EN                        DEF_DISABLED
#define  APP_CFG_FS_NOR_EN                         DEF_DISABLED
#define  APP_CFG_FS_RAM_EN                         DEF_ENABLED
#define  APP_CFG_FS_SD_EN                          DEF_DISABLED
#define  APP_CFG_FS_SD_CARD_EN                     DEF_DISABLED

#define  APP_CFG_FS_RAM_NBR_SECS                    55  //org: (1 * 1024)
#define  APP_CFG_FS_RAM_SEC_SIZE                          512


/*
*********************************************************************************************************
*                                    BSP CONFIGURATION: RS-232
*********************************************************************************************************
*/

#define  BSP_SER_COMM_EN                        DEF_ENABLED
#define  BSP_CFG_SER_COMM_SEL          BSP_SER_COMM_UART_03
#define  BSP_CFG_TS_TMR_SEL                               2


/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#ifndef  TRACE_LEVEL_OFF
#define  TRACE_LEVEL_OFF                                   0u
#endif

#ifndef  TRACE_LEVEL_INFO
#define  TRACE_LEVEL_INFO                                  1u
#endif

#ifndef  TRACE_LEVEL_DBG
#define  TRACE_LEVEL_DBG                                   2u
#endif

#ifndef  TRACE_LEVEL_LOG
#define  TRACE_LEVEL_LOG                                   3u
#endif

#define  APP_TRACE_LEVEL                   TRACE_LEVEL_DBG
#define  APP_TRACE                           BSP_Ser_Printf

#define  APP_TRACE_INFO(x)           ((APP_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_TRACE x) : (void)0)
#define  APP_TRACE_DEBUG(x)          ((APP_TRACE_LEVEL >= TRACE_LEVEL_DBG) ? (void)(APP_TRACE x) : (void)0)


#endif
