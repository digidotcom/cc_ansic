/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                             (c) Copyright 2011; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           MASTER INCLUDES
*
*                                          Freescale Kinetis
*                                           featured on the
*                                        Freescale TWR-K53N512
*                                          Evaluation Board
*
* Filename      : includes.h
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

#ifndef  INCLUDES_PRESENT
#define  INCLUDES_PRESENT


/*
*********************************************************************************************************
*                                         STANDARD LIBRARIES
*********************************************************************************************************
*/

#include  <stdarg.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <math.h>

/*
*********************************************************************************************************
*                                              LIBRARIES
*********************************************************************************************************
*/

#include  <cpu.h>
#include  <lib_def.h>
#include  <lib_ascii.h>
#include  <lib_math.h>
#include  <lib_mem.h>
#include  <lib_str.h>


/*
*********************************************************************************************************
*                                        Freescale Header File
*********************************************************************************************************
*/

#include  <MK53N512CMD100.h>


/*
*********************************************************************************************************
*                                              APP / BSP
*********************************************************************************************************
*/

#include  <app_cfg.h>
#include  <bsp.h>
#include  <bsp_os.h>
#include  <bsp_ser.h>


/*
*********************************************************************************************************
*                                                 OS
*********************************************************************************************************
*/

#include  <os.h>


/*
*********************************************************************************************************
*                                          TCP-IP, DHCP, DNS
*********************************************************************************************************
*/

#include    <net.h>
#include    <net_bsp.h>
#include    <net_dev_cfg.h>
#include    <net_dev_macnet.h>
#include    <net_phy.h>

#include    <dhcp-c.h>
#include    <dns-c.h>

/*
*********************************************************************************************************
*                                            INCLUDES END
*********************************************************************************************************
*/

#endif

