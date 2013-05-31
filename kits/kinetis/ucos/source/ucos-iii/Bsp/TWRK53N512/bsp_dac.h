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
*                                     DIGITAL TO ANALOG CONVERTER
*
*                                          Freescale Kinetis
*                                           featured on the
*                                        Freescale TWR-K53N512
*                                          Evaluation Board
*
* Filename      : bsp_dac.h
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

#ifndef  BSP_DAC_PRESENT
#define  BSP_DAC_PRESENT


/*
*********************************************************************************************************
*                                                 EXTERNS
*********************************************************************************************************
*/

#ifdef   BSP_DAC_MODULE
#define  BSP_DAC_EXT
#else
#define  BSP_DAC_EXT  extern
#endif


/*
*********************************************************************************************************
*                                              INCLUDE FILES
*********************************************************************************************************
*/


#define   BSP_DAC_DAC0                   0
#define   BSP_DAC_DAC1                   1


/*
*********************************************************************************************************
*                                             CFG DEFINITIONS
*********************************************************************************************************
*/

#define  BSP_DAC_CFG_MODE_BUF            1
#define  BSP_DAC_CFG_MODE_SINGLE         2
#define  BSP_DAC_CFG_MODE_VREF           3

#define  BSP_DAC_AMPLITUDE_300           1
#define  BSP_DAC_AMPLITUDE_3000          2


/*
*********************************************************************************************************
*                                        REGISTER BIT DEFINITIONS
*********************************************************************************************************
*/

#define   BSP_DAC_SR_RESET               0x02u                  /* DAC's Status  Register   reset value.                */
#define   BSP_DAC_C0_RESET               0x00u                  /* DAC's Control Register   reset value.                */
#define   BSP_DAC_C1_RESET               0x00u                  /* DAC's Control Register 1 reset value.                */
#define   BSP_DAC_C2_RESET               0x0Fu                  /* DAC's Control Register 2 reset value.                */

                                                                /* DAC's Control Register bit definitions.              */

#define   BSP_DAC_DISABLE                0x00u                  /* DAC's Control Register enable bit.                   */
#define   BSP_DAC_ENABLE                 DAC_C0_DACEN_MASK

#define   BSP_DAC_SEL_VREFO              0x00u                  /* DAC's Reference voltage Vref0.                       */
#define   BSP_DAC_SEL_VDDA               DAC_C0_DACRFS_MASK     /* DAC's Reference voltage Vdd.                         */

#define   BSP_DAC_SEL_PDB_HW_TRIG        0x00u                  /* DAC Triggered by the PDB module.                     */
#define   BSP_DAC_SEL_SW_TRIG            DAC_C0_DACTRGSEL_MASK  /* DAC Triggered by software.                           */

#define   BSP_DAC_SW_TRIG_STOP           0x00u
#define   BSP_DAC_SW_TRIG_NEXT           DAC_C0_DACSWTRG_MASK 

#define   BSP_DAC_HI_PWR_MODE            0x00u
#define   BSP_DAC_LO_PWR_MODE            DAC_C0_LPEN_MASK 

#define   BSP_DAC_BUF_WTR_MARK_INT_DIS   0x00u
#define   BSP_DAC_BUF_WTR_MARK_INT_EN    DAC_C0_DACBWIEN_MASK

#define   BSP_DAC_BUF_TOP_PTR_INT_DIS    0x00u
#define   BSP_DAC_BUF_TOP_PTR_INT_EN     DAC_C0_DACBTIEN_MASK

#define   BSP_DAC_BUF_BOT_PTR_INT_DIS    0x00u
#define   BSP_DAC_BUF_BOT_PTR_INT_EN     DAC_C0_DACBBIEN_MASK

                                                                /* DAC's Control Register 1 bit definitions.            */
#define   BSP_DAC_BUF_NORMAL_MODE        DAC_C1_DACBFMD(0)
#define   BSP_DAC_BUF_SWING_MODE         DAC_C1_DACBFMD(1) 
#define   BSP_DAC_BUF_ONE_TIME_MODE      DAC_C1_DACBFMD(2)

#define   BSP_DAC_BUF_DISABLE            0x00u
#define   BSP_DAC_BUF_ENABLE             DAC_C1_DACBFEN_MASK 

                                                                /* DAC's Control Register 2 bit definitions.            */
#define   BSP_DAC_SET_PTR_AT_BUF(x)      DAC_C2_DACBFRP(x)
#define   BSP_DAC_SET_PTR_UP_LIM(x)      DAC_C2_DACBFUP(x)

#define   BSP_DAC_VREF_SC_INT_MODE       0x81


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

void  BSP_DAC_DAC0_ISR (void);
void  BSP_DAC_DAC1_ISR (void);

void  BSP_DAC_Init (CPU_INT08U dac_nbr, CPU_INT08U cfg_mode);

void  BSP_DAC_BufModeInit (DAC_MemMapPtr dacx_base_ptr, CPU_INT08U wtr_mark_mode,
                           CPU_INT08U    buf_mode,      CPU_INT08U ref_volt,
                           CPU_INT08U    trig_mode,     CPU_INT08U buf_init_pos,
                           CPU_INT08U    buf_up_lim);

void  BSP_DAC_TrigModeInit (DAC_MemMapPtr dacx_base_ptr, CPU_INT08U buf_mode, 
                            CPU_INT08U    ref_volt,      CPU_INT08U trig_mode, 
                            CPU_INT08U    buf_init_pos,  CPU_INT08U buf_up_lim);

void  BSP_DAC_BufInit (DAC_MemMapPtr dacx_base_ptr);

void  BSP_DAC_PDB_TrigInit (void);

void  BSP_DAC_UpdateInterval (CPU_INT08U dac_nbr, CPU_INT16U interval);
void  BSP_DAC_SwitchVoltage  (CPU_INT08U dac_nbr, CPU_INT08U amplitude);
void  BSP_DAC_SetOutputVoltage (CPU_INT08U dac_nbr, CPU_INT16U amplitude_cnts);

void  BSP_DAC_En  (CPU_INT08U dac_nbr);
void  BSP_DAC_Dis (CPU_INT08U dac_nbr);


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/


#endif                                                          /* End of module include.                               */