/*
*********************************************************************************************************
*                                     MICIRUM BOARD SUPPORT PACKAGE
*
*                             (c) Copyright 2010; Micrium, Inc.; Weston, FL
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
*                                        BOARD SUPPORT PACKAGE
*                                     ANALOG TO DIGITAL CONVERTER
*
*                                          Freescale Kinetis
*                                           featured on the
*                                        Freescale TWR-K53N512
*                                          Evaluation Board
*
* Filename      : bsp_adc.h
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                 MODULE
*
* Note(s) : (1) This header file is protected from multiple pre-processor inclusion through use of the
*               BSP present pre-processor macro definition.
*********************************************************************************************************
*/

#ifndef  BSP_ADC_PRESENT
#define  BSP_ADC_PRESENT


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef   BSP_ADC_MODULE
#define  BSP_ADC_EXT
#else
#define  BSP_ADC_EXT  extern
#endif


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               DEFINITIONS
*********************************************************************************************************
*/

#define  BSP_ADC_CFG_MODE_POLL                   1
#define  BSP_ADC_CFG_MODE_INT                    2

#define  BSP_ADC_SAMPLING_RATE_50_HZ             3
#define  BSP_ADC_SAMPLING_RATE_100_HZ            2
#define  BSP_ADC_SAMPLING_RATE_200_HZ            1
#define  BSP_ADC_SAMPLING_RATE_400_HZ            0


/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                               DATA TYPES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                                 MACRO'S
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void         BSP_ADC_Init             (CPU_INT08U, CPU_INT08U sampling_rate);
void         BSP_ADC_PDB_Init         (CPU_INT08U);
void         BSP_ADC_Calibrate        (void);
void         BSP_ADC_SimDAQ_Init      (void);
void         BSP_ADC_SimDAQ_En        (void);
void         BSP_ADC_SimDAQ_Dis       (void);
void         BSP_ADC_BioDAQ_Init      (CPU_INT08U);
void         BSP_ADC_BioDAQ_En        (void);
void         BSP_ADC_BioDAQ_Dis       (void);
void         BSP_ADC_ADC0_ISR         (void);
void         BSP_ADC_ADC1_ISR         (void);


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/


#endif                                                          /* End of module include.                               */
