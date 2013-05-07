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
*                                     ANALOG TO DIGITAL CONVERTER
*                                        BOARD SUPPORT PACKAGE
*
*                                          Freescale Kinetis
*                                           featured on the
*                                        Freescale TWR-K53N512
*                                          Evaluation Board
*
* Filename      : bsp_adc.c
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_ADC_MODULE
#include <includes.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
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


/*
*********************************************************************************************************
*                                             REGISTERS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            REGISTER BITS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                          BSP_ADC_ADC0_ISR()
*
* Description : Handle the ADC # 0 interrupt.
* Argument(s) : none.
* Return(s)   : none.
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_ADC_ADC0_ISR (void)
{
    OS_ERR      err;
    CPU_INT16U  tmp;


    if (DEF_BIT_IS_SET(ADC0_SC1A, ADC_SC1_COCO_MASK)) {
        AppTaskDAQ_Msg1.Type = APP_MSG_TYPE_ADC_CH1;
        AppTaskDAQ_Msg1.Data = ADC0_RA;                         /* The interrupt flag will clear after reading.         */
                                                                /* Post to the DAQ Task with the ADC data.              */
        OSTaskQPost((OS_TCB    *)&AppTaskDAQ_TCB,
                    (void      *)&AppTaskDAQ_Msg1,
                    (OS_MSG_SIZE)sizeof(AppTaskDAQ_Msg1),
                    (OS_OPT     )OS_OPT_POST_FIFO,
                    (OS_ERR    *)&err);
        
    } else if (DEF_BIT_IS_SET(ADC0_SC1B, ADC_SC1_COCO_MASK)) {
        tmp = ADC0_RB;                                          /* The interrupt flag will clear after reading.         */
        (void)&tmp;                                             /* Avoid 'variable set but never used' compiler warning.*/
    }
}


/*
*********************************************************************************************************
*                                             BSP_ADC_ADC1_ISR()
*
* Description : Handle the ADC # 1 interrupt.
* Argument(s) : none.
* Return(s)   : none.
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_ADC_ADC1_ISR (void)
{
    OS_ERR      err;


    if (DEF_BIT_IS_SET(ADC1_SC1A, ADC_SC1_COCO_MASK)) {

        AppTaskDAQ_Msg2.Type = APP_MSG_TYPE_ADC_CH2;
        AppTaskDAQ_Msg2.Data = ADC1_RA;                         /* The interrupt flag will clear after reading.         */
                                                                /* Post to the DAQ Task with the ADC data.              */

        OSTaskQPost((OS_TCB    *)&AppTaskDAQ_TCB,
                    (void      *)&AppTaskDAQ_Msg2,
                    (OS_MSG_SIZE)sizeof(AppTaskDAQ_Msg2),
                    (OS_OPT     )OS_OPT_POST_FIFO,
                    (OS_ERR    *)&err);

    } else if (DEF_BIT_IS_SET(ADC1_SC1B, ADC_SC1_COCO_MASK)) {

        AppTaskSimMsg.Type = APP_MSG_TYPE_ADC_CH1;
        AppTaskSimMsg.Data = ADC1_RB;                           /* The interrupt flag will clear after reading.         */
                                                                /* Post to the Sim Task with the ADC data.              */
        OSTaskQPost((OS_TCB    *)&AppTaskSimTCB,
                    (void      *)&AppTaskSimMsg,
                    (OS_MSG_SIZE)sizeof(AppTaskSimMsg),
                    (OS_OPT     )OS_OPT_POST_FIFO,
                    (OS_ERR    *)&err);
    }
}


/*
*********************************************************************************************************
*                                           BSP_ADC_Init()
*
* Description : Initialize the Analog to Digital Converters.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP_ADC function is called.
*********************************************************************************************************
*/

void  BSP_ADC_Init (CPU_INT08U cfg_mode, CPU_INT08U sampling_rate)
{    
    BSP_ADC_PDB_Init(sampling_rate);
    BSP_ADC_SimDAQ_Init();                                      /* Initialize the ADC that converts the Potentiometer.  */
    BSP_ADC_BioDAQ_Init(cfg_mode);                              /* Initialize the ADC that converts the Biomedical sig. */
}


/*
*********************************************************************************************************
*                                      BSP_ADC_PDB_Init()
*
* Description : Initialize the PDB to trigger the ADC conversions.
*
* Argument(s) : sampling_rate.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_ADC_PDB_Init (CPU_INT08U sampling_rate)
{
    SIM_SCGC6 |= SIM_SCGC6_PDB_MASK;                            /* Clock Gate Control: Enable the PDB  Clock.           */

                                                                /* Configure the Peripheral Delay Block (PDB):          */
       
    PDB0_SC = PDB_SC_CONT_MASK   |                              /* Contintuous mode enabled.                            */
              PDB_SC_PDBEN_MASK  |                              /* PDB enabled.                                         */
              PDB_SC_PDBIE_MASK  |                              /* PDB interrupt enabled.                               */
              PDB_SC_PRESCALER(sampling_rate) |                 /* Sampling rate.                                       */
              PDB_SC_TRGSEL(0xf) |                              /* Software trigger selected.                           */
              PDB_SC_MULT(0);                                   /* Sampling rate.                                       */

    PDB0_IDLY = 0x0000;                                         /* Need to trigger interrupt every counter reset which  */
                                                                /* happens when modulus reached.                        */

    PDB0_MOD = 0xCD66;                                          /* Modulus.                                             */    

                                                                /* channel 0 pretrigger 0 and 1 delayed.                */
    PDB0_CH0C1 = PDB_C1_TOS(0x01);

    PDB0_CH0DLY0 = 0x3FFF;
    PDB0_CH0DLY1 = 0xBFFD;
    
                                                                /* channel 1 pretrigger 0 and 1 delayed.                */
    PDB0_CH1C1 = PDB_C1_TOS(0x01);

    PDB0_CH1DLY0 = 0x3FFF;
    PDB0_CH1DLY1 = 0xBFFD;

    PDB0_SC |= PDB_SC_LDOK_MASK;
}


/*
*********************************************************************************************************
*                                      BSP_ADC_SimDAQ_Init()
*
* Description : Initialize the Analog to Digital Converter that converts the signal coming from
*               the potentiometer.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_ADC_SimDAQ_Init (void)
{
    SIM_SCGC6 |= SIM_SCGC6_PDB_MASK;                            /* Clock Gate Control: Enable the PDB  Clock.           */
  
    SIM_SCGC3 |= SIM_SCGC3_ADC1_MASK;                           /* Clock Gate Control: Enable the ADC1 Clock.           */
    
    SIM_SOPT7 &= ~(SIM_SOPT7_ADC1ALTTRGEN_MASK  |               /* Select PDB trigger for ADC1.                         */
                   SIM_SOPT7_ADC1PRETRGSEL_MASK);               /* Select Pre-trigger A for ADC1.                       */

    ADC1_CFG1 = ADC_CFG1_ADIV(0x02)   |
                ADC_CFG1_ADLSMP_MASK  |
                ADC_CFG1_MODE(0x03)   |
                ADC_CFG1_ADICLK(0x00);

    ADC1_CFG2 = ADC_CFG2_ADHSC_MASK  |
                ADC_CFG2_ADLSTS(0x0);

    ADC1_SC2  = ADC_SC2_ADTRG_MASK  |
                ADC_SC2_ACFGT_MASK  |
                ADC_SC2_ACREN_MASK  |
                ADC_SC2_REFSEL(0);

    ADC1_SC3  = ADC_SC3_AVGE_MASK  |
                ADC_SC3_AVGS(0x03);

    ADC1_PGA  = ADC_PGA_PGAG(0x06);
 
    ADC1_SC1A = ADC_SC1_AIEN_MASK | 
                ADC_SC1_ADCH(0);                                /* ADC1 Ch. 00 (ADC1_DP0).                              */

    ADC1_SC1B = ADC_SC1_AIEN_MASK | 
                ADC_SC1_ADCH(20);                               /* ADC1 Ch. 20 (ADC1_DM1).                              */
     
    BSP_IntEn(BSP_INT_ID_ADC1);                                 /* Enable Interrupts on ADC1.                           */
    BSP_IntEn(BSP_INT_ID_PDB0);                                 /* Enable Interrupts on PDB0.                           */
        
    PDB0_SC |= PDB_SC_SWTRIG_MASK ;                             /* Start the PDB.                                       */    
}


/*
*********************************************************************************************************
*                                       BSP_ADC_BioDAQ_Init()
*
* Description : Initialize the Analog to Digital Converter for the biomedical signal.
*               The sampling rate for the ECG signal is 200 Hz.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_ADC_BioDAQ_Init (CPU_INT08U cfg_mode)
{  
                                                                /* Biomedical signal at ADC0_DM0                        */

    SIM_SCGC6 |= SIM_SCGC6_ADC0_MASK;                           /* Clock Gate Control: Enable the ADC0 Clock.           */

    if (cfg_mode == BSP_ADC_CFG_MODE_INT) {
        SIM_SOPT7 &= ~(SIM_SOPT7_ADC0ALTTRGEN_MASK  |           /* Select PDB trigger for ADC0.                         */
                       SIM_SOPT7_ADC0PRETRGSEL_MASK);           /* Select Pre-trigger A for ADC0.                       */
    }

    ADC0_CFG1 = ADC_CFG1_ADIV(0x02)   |
                ADC_CFG1_ADLSMP_MASK  |
                ADC_CFG1_MODE(0x03)   |
                ADC_CFG1_ADICLK(0x00);

    ADC0_CFG2 = ADC_CFG2_ADHSC_MASK  |
                ADC_CFG2_ADLSTS(0x0);

    ADC0_SC2  = ADC_SC2_ADTRG_MASK  |
                ADC_SC2_ACFGT_MASK  |
                ADC_SC2_ACREN_MASK  |
                ADC_SC2_REFSEL(0);

    ADC0_SC3  = ADC_SC3_AVGE_MASK  |
                ADC_SC3_AVGS(0x03);

    ADC0_PGA  = ADC_PGA_PGAG(0x06);
 
    ADC0_SC1A = ADC_SC1_AIEN_MASK | 
                ADC_SC1_ADCH(0);                                /* ADC0 Ch. 00 (ADC0_DP0).                              */

    ADC0_SC1B = ADC_SC1_AIEN_MASK | 
                ADC_SC1_ADCH(0);                                /* ADC0 Ch. 00 (ADC0_DP0).                              */

    if (cfg_mode == BSP_ADC_CFG_MODE_INT) {
        BSP_IntEn(BSP_INT_ID_ADC0);                             /* Enable Interrupts on ADC0.                           */
        BSP_IntEn(BSP_INT_ID_PDB0);                             /* Enable Interrupts on PDB0.                           */

        PDB0_SC |= PDB_SC_SWTRIG_MASK ;                         /* Start the PDB.                                       */
    }
}


/*
*********************************************************************************************************
*                                       BSP_ADC_BioDAQ_En()
*
* Description : Enable the Analog to Digital Converter for the biomedical signal
*               by enabling the triggers from the PDB (Programmable Delay Block).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_ADC_BioDAQ_En (void)
{
    PDB0_CH1C1 |= PDB_C1_EN_MASK;                               /* Enable PDB trigger for ADC1.                         */
}


/*
*********************************************************************************************************
*                                         BSP_ADC_BioDAQ_Dis()
*
* Description : Disable the Analog to Digital Converter for the biomedical signal
*               by disabling the triggers from the PDB (Programmable Delay Block).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_ADC_BioDAQ_Dis (void)
{
    PDB0_CH1C1 &= ~PDB_C1_EN_MASK;                              /* Disable PDB trigger for ADC1.                        */
}


/*
*********************************************************************************************************
*                                        BSP_ADC_SimDAQ_En()
*
* Description : Enable the Analog to Digital Converter for the potentiometer signal
*               by enabling the triggers from the PDB (Programmable Delay Block).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_ADC_SimDAQ_En (void)
{
    PDB0_CH0C1 |= PDB_C1_EN_MASK;                               /* Enable PDB trigger for ADC0.                         */
}


/*
*********************************************************************************************************
*                                         BSP_ADC_SimDAQ_Dis()
*
* Description : Disable the Analog to Digital Converter for the potentiometer signal
*               by disabling the triggers from the PDB (Programmable Delay Block).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void BSP_ADC_SimDAQ_Dis (void)
{
    PDB0_CH0C1 &= ~PDB_C1_EN_MASK;                              /* Disable PDB trigger for ADC0.                        */
}