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
*                                     DIGITAL TO ANALOG CONVERTER
*                                        BOARD SUPPORT PACKAGE
*
*                                          Freescale Kinetis
*                                           featured on the
*                                        Freescale TWR-K53N512
*                                          Evaluation Board
*
* Filename      : bsp_dac.c
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_DAC_MODULE
#include <includes.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define   BSP_DAC_MASK_CLR_INT        0x05


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

CPU_INT08U  BSP_DAC_DAC0_BufIx;                                 /* Index to the data buffer's current position.         */
CPU_INT08U  BSP_DAC_DAC1_BufIx;                                 /* Index to the data buffer's current position.         */


/*
*********************************************************************************************************
*                                             REGISTERS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                         BSP_DAC_DAC0_ISR()
*
* Description : Handle the DAC 0 interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR called from app_vect.c.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_DAC_DAC0_ISR (void)
{
#ifdef BSP_DAC_CFG_BPM_EN
    CPU_INT08U  i;
#endif

    if (DEF_BIT_IS_SET(DAC_SR_REG(DAC0_BASE_PTR),               /* Buffer read pointer is at the top limit.             */
                       DAC_SR_DACBFRPTF_MASK)) {
#ifdef BSP_DAC_CFG_BPM_EN
        BSP_DAC_DAC0_BufIx++;                                   /* Update the index.                                    */

        if (BSP_DAC_DAC0_BufIx == BSP_DAC_BUF_MAX_ROWS) {
            BSP_DAC_DAC0_BufIx = 0;                             /* Go back to the start position.                       */
        }

        for (i = 0; i < BSP_DAC_BUF_MAX_COLS; i++) {            /* Update the DAC buffer.                               */
            DAC_DATL_REG(DAC0_BASE_PTR, i) = App_DAC_DAC0_BufLo[BSP_DAC_DAC0_BufIx][i];
            DAC_DATH_REG(DAC0_BASE_PTR, i) = App_DAC_DAC0_BufHi[BSP_DAC_DAC0_BufIx][i];
        }
#endif
        DAC_SR_REG(DAC0_BASE_PTR) = BSP_DAC_MASK_CLR_INT;       /* Clear the interrupt flag.                            */
    }
}


/*
*********************************************************************************************************
*                                         BSP_DAC_DAC1_ISR()
*
* Description : Handle the DAC 1 interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR called from app_vect.c.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_DAC_DAC1_ISR (void)
{
    CPU_INT08U  i;


    if (DEF_BIT_IS_SET(DAC_SR_REG(DAC1_BASE_PTR),               /* Buffer read pointer is at the top limit.             */
                       DAC_SR_DACBFRPTF_MASK)) {

        BSP_DAC_DAC1_BufIx++;                                   /* Update the index.                                    */

        if (BSP_DAC_DAC1_BufIx == BSP_DAC_BUF_MAX_ROWS) {
            BSP_DAC_DAC1_BufIx = 0;                             /* Go back to the start position.                       */
        }

        for (i = 0; i < BSP_DAC_BUF_MAX_COLS; i++) {            /* Update the DAC buffer.                               */
            DAC_DATL_REG(DAC1_BASE_PTR, i) = App_DAC_DAC1_BufLo[BSP_DAC_DAC1_BufIx][i];
            DAC_DATH_REG(DAC1_BASE_PTR, i) = App_DAC_DAC1_BufHi[BSP_DAC_DAC1_BufIx][i];
        }

        DAC_SR_REG(DAC1_BASE_PTR) = BSP_DAC_MASK_CLR_INT;       /* Clear the interrupt flag.                            */
    }
}


/*
*********************************************************************************************************
*                                          BSP_DAC_En()
*
* Description : Enables the DAC-based Simulator.
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

void  BSP_DAC_En (CPU_INT08U dac_nbr)
{
    switch (dac_nbr) {
        case BSP_DAC_DAC0:
             DAC0_C0 |= DAC_C0_DACEN_MASK;                      /* Set the DAC system Enable/Disable bit.               */
             break;

        case BSP_DAC_DAC1:
             DAC1_C0 |= DAC_C0_DACEN_MASK;                      /* Set the DAC system Enable/Disable bit.               */
             break;
             
        default:
             break;
    }
    FTM1_MODE |= FTM_MODE_FTMEN_MASK;                           /* FTM 1 Enabled.                                       */
}


/*
*********************************************************************************************************
*                                          BSP_DAC_Dis()
*
* Description : Disables the DAC-based Simulator.
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

void  BSP_DAC_Dis (CPU_INT08U dac_nbr)
{
    switch (dac_nbr) {
        case BSP_DAC_DAC0:
             DAC0_C0 &= ~DAC_C0_DACEN_MASK;                     /* Clear the DAC system Enable/Disable bit.             */
             break;

        case BSP_DAC_DAC1:
             DAC1_C0 &= ~DAC_C0_DACEN_MASK;                     /* Clear the DAC system Enable/Disable bit.             */
             break;

        default:
             break;
    }
    FTM1_MODE &= ~FTM_MODE_FTMEN_MASK;                          /* FTM 1 Enabled.                                       */
}


/*
*********************************************************************************************************
*                                             BSP_DAC_Init()
*
* Description : Initialize the Digital to Analog Converter.
*
* Argument(s) : dac_nbr      [0,1] DAC module to intialize.
*               min_spl_dly  Delay between samples.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP_DAC function is called.
*********************************************************************************************************
*/

void  BSP_DAC_Init (CPU_INT08U dac_nbr, CPU_INT08U cfg_mode)
{
    DAC_MemMapPtr  dacx_base_ptr;


    switch (dac_nbr) {
        case BSP_DAC_DAC0:
             dacx_base_ptr = DAC0_BASE_PTR;                     /* Set the base address pointer for DAC0.               */
             BSP_DAC_DAC0_BufIx = 0;                            /* Initialize the Buffer Index to the first position.   */
             SIM_SCGC2 |= SIM_SCGC2_DAC0_MASK;                  /* Allow clock to enable DAC0.                          */
             break;

        case BSP_DAC_DAC1:
             dacx_base_ptr = DAC1_BASE_PTR;                     /* Set the base address pointer for DAC1.               */
             BSP_DAC_DAC1_BufIx = 0;                            /* Initialize the Buffer Index to the first position.   */
             SIM_SCGC2 |= SIM_SCGC2_DAC1_MASK;                  /* Allow clock to enable DAC1.                          */
             break;

        default:
             break;
    }

    DAC_SR_REG(dacx_base_ptr) = BSP_DAC_SR_RESET;               /* DAC Status  Register   reset.                        */
    DAC_C0_REG(dacx_base_ptr) = BSP_DAC_C0_RESET;               /* DAC Control Register   reset.                        */
    DAC_C1_REG(dacx_base_ptr) = BSP_DAC_C1_RESET;               /* DAC Control Register 1 reset.                        */
    DAC_C2_REG(dacx_base_ptr) = BSP_DAC_C2_RESET;               /* DAC Control Register 2 reset.                        */
    
    switch (cfg_mode) {
        case BSP_DAC_CFG_MODE_BUF:                              /* Configure DAC0 in software trigger mode operation.   */
             BSP_DAC_TrigModeInit(dacx_base_ptr,             BSP_DAC_BUF_NORMAL_MODE,
                                  BSP_DAC_SEL_VREFO,         BSP_DAC_SEL_SW_TRIG,
                                  BSP_DAC_SET_PTR_AT_BUF(0), BSP_DAC_SET_PTR_UP_LIM(15));
             
             BSP_FTM1_Init();
             break;
             
        case BSP_DAC_CFG_MODE_SINGLE:
             DAC_C0_REG(dacx_base_ptr) = BSP_DAC_SEL_VDDA;      /* DAC Control Register Voltage Reference select.       */
             break;

        case BSP_DAC_CFG_MODE_VREF:
            DAC_C1_REG(dacx_base_ptr) &= ~DAC_C1_DACBFEN_MASK;
            DAC_DATH_REG(dacx_base_ptr, 0) = 0x07;
            DAC_DATL_REG(dacx_base_ptr, 0) = 0xC1;
            DAC_C0_REG(dacx_base_ptr) |= DAC_C0_DACRFS_MASK;
            break;
             
        default:
            break;
    }
}


/*
*********************************************************************************************************
*                                   BSP_DAC_HwTrigModeInit()
*
* Description : Configures the DAC to Hardware triggered mode operation.
*
* Argument(s) : dacx_base_ptr    DAC Module's Base Address.
*               buf_mode         En/Dis Flag [0:Dis, 1:En].
*               ref_volt         DAC reference voltage [0:Int, 1:Ext].
*               trig_mode        DAC trigger select [0:Hw, 1:Sw].
*               buf_init_pos     DAC buffer's read pointer value.
*               buf_up_limit     DAC buffer's upper limit.
*               
* Return(s)   : none.
*
* Caller(s)   : bsp_dac.c.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_DAC_TrigModeInit (DAC_MemMapPtr dacx_base_ptr, CPU_INT08U buf_mode, 
                            CPU_INT08U    ref_volt,      CPU_INT08U trig_mode, 
                            CPU_INT08U    buf_init_pos,  CPU_INT08U buf_up_lim)
{
    BSP_DAC_BufModeInit(dacx_base_ptr, 0, buf_mode, ref_volt, trig_mode, buf_init_pos, buf_up_lim);

    BSP_DAC_BufInit(dacx_base_ptr);

    DAC_C0_REG(dacx_base_ptr) |= (BSP_DAC_BUF_BOT_PTR_INT_DIS | /* DACBIE = 0: buffer read ptr bottom flag int disabled */        
                                  BSP_DAC_BUF_TOP_PTR_INT_EN  | /* DACTIE = 0: buffer read ptr top    flag int enabled  */          
                                  BSP_DAC_BUF_WTR_MARK_INT_DIS); /* DACWIE = 0: buffer water mark interrupt disabled.   */                                

    if (dacx_base_ptr == DAC0_BASE_PTR) {
        BSP_IntEn(BSP_INT_ID_DAC0);                             /* Enable Interrupts on DAC 0.                          */
    } else if (dacx_base_ptr == DAC1_BASE_PTR) {
        BSP_IntEn(BSP_INT_ID_DAC1);                             /* Enable Interrupts on DAC 1.                          */
    }

    if (trig_mode == BSP_DAC_SEL_PDB_HW_TRIG) {
        BSP_DAC_PDB_TrigInit();                                 /* Initialize PDB for DAC hardware triggering.          */
    } else if (trig_mode == BSP_DAC_SEL_SW_TRIG) {
        DAC_C0_REG(dacx_base_ptr) |= DAC_C0_DACTRGSEL_MASK;     /* Software trigger.                                    */
    }    
}


/*
*********************************************************************************************************
*                                          BSP_DAC_BufModeInit()
*
* Description : Configures the DAC to Buffered Mode Operation, where the DAC converts the data
*               in the data buffer to analog output voltage.
*
* Argument(s) : dacx_base_ptr    DAC Module's Base Address.
*               wtr_mark_mode    En/Dis Flag [0:Dis, 1:En].
*               buf_mode         En/Dis Flag [0:Dis, 1:En].
*               ref_volt         DAC reference voltage [0:Int, 1:Ext].
*               trig_mode        DAC trigger select [0:Hw, 1:Sw].
*               buf_init_pos     DAC buffer's read pointer value.
*               buf_up_limit     DAC buffer's upper limit.
*               
* Return(s)   : none.
*
* Caller(s)   : bsp_dac.c.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_DAC_BufModeInit (DAC_MemMapPtr dacx_base_ptr, CPU_INT08U wtr_mark_mode,
                           CPU_INT08U    buf_mode,      CPU_INT08U ref_volt,
                           CPU_INT08U    trig_mode,     CPU_INT08U buf_init_pos,
                           CPU_INT08U    buf_up_lim)
{

                                                                /* DAC Control Register.                                */
    DAC_C0_REG(dacx_base_ptr) = (BSP_DAC_DISABLE              | /* DAC Disabled.                                        */
                                 ref_volt                     | /* DAC Reference Voltage Select.                        */
                                 trig_mode                    | /* DAC Trigger Select.                                  */
                                 BSP_DAC_SW_TRIG_STOP         | /* DAC Software Trigger.                                */
                                 BSP_DAC_HI_PWR_MODE          | /* DAC Power Mode.                                      */
                                 BSP_DAC_BUF_WTR_MARK_INT_DIS | /* DAC Buffer Watermark Interrupt Disabled.             */
                                 BSP_DAC_BUF_TOP_PTR_INT_DIS  | /* DAC Buffer Read Pointer Top    Int Enabled.          */
                                 BSP_DAC_BUF_BOT_PTR_INT_DIS);  /* DAC Buffer Read Pointer Bottom Int Disabled.         */


    if (ref_volt == BSP_DAC_SEL_VREFO) {
        SIM_SCGC4 |= SIM_SCGC4_VREF_MASK;                       /* Enable Vref 0.                                       */
        VREF_SC    = BSP_DAC_VREF_SC_INT_MODE;                  /* Select internal mode.                                */
        while (!(VREF_SC & VREF_SC_VREFST_MASK)) 
        {
            ;                                                   /* Wait until the VREFSC is stable.                     */
        }
    }

                                                                /* DAC Control Register 1.                              */
    DAC_C1_REG(dacx_base_ptr) = (wtr_mark_mode |                /* Buffer Watermark Level.                              */
                                 buf_mode  |                    /* Buffer Work Mode.                                    */
                                 BSP_DAC_BUF_ENABLE);           /* Buffer Read Pointer Enabled.                         */

                                                                /* DAC Control Register 2.                              */
    DAC_C2_REG(dacx_base_ptr) = buf_init_pos |                  /* Buffer Read Pointer Initial Position.                */
                                buf_up_lim;                     /* Buffer Upper Limit.                                  */
}


/*
*********************************************************************************************************
*                                          BSP_DAC_BufInit()
*
* Description : Initializes the DAC's data buffer.
*
* Argument(s) : dacx_base_ptr    DAC Module's Base Address.
*               
* Return(s)   : none.
*
* Caller(s)   : bsp_dac.c.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_DAC_BufInit (DAC_MemMapPtr dacx_base_ptr)
{
    CPU_INT08U  i;


    if (dacx_base_ptr == DAC0_BASE_PTR) {
        for (i = 0; i < BSP_DAC_BUF_MAX_COLS; i++) {
#ifdef BSP_DAC_CFG_BPM_EN
            DAC_DATL_REG(DAC0_BASE_PTR, i) = App_DAC_DAC0_BufLo[0][i];
            DAC_DATH_REG(DAC0_BASE_PTR, i) = App_DAC_DAC0_BufHi[0][i];
#endif            
        }
    } else if (dacx_base_ptr == DAC1_BASE_PTR) {
        for (i = 0; i < BSP_DAC_BUF_MAX_COLS; i++) {
            DAC_DATL_REG(DAC1_BASE_PTR, i) = App_DAC_DAC1_BufLo[0][i];
            DAC_DATH_REG(DAC1_BASE_PTR, i) = App_DAC_DAC1_BufHi[0][i];
        }
    }    
}


/*
*********************************************************************************************************
*                                       BSP_DAC_PDB_TrigInit()
*
* Description : Configures the DAC to hardware triggered mode by the PDB (Programmable Delay Block).
*
* Argument(s) : none.
*               
* Return(s)   : none.
*
* Caller(s)   : bsp_dac.c.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_DAC_PDB_TrigInit (void)
{
    SIM_SCGC6     |= SIM_SCGC6_PDB_MASK;                        /* Enable system clock to PDB.                          */

    PDB0_SC = PDB_SC_CONT_MASK      |                           /* Contintuous mode enabled.                            */
              PDB_SC_PDBEN_MASK     |                           /* PDB enabled.                                         */
              PDB_SC_PDBIE_MASK     |                           /* PDB interrupt enabled.                               */
              PDB_SC_PRESCALER(0x3) |                           /* Sampling rate.                                       */
              PDB_SC_TRGSEL(0xF)    |                           /* Software trigger selected.                           */
              PDB_SC_MULT(0);                                   /* Sampling rate.                                       */
    
    PDB0_DACINT0   = 50000;                                     /* Effective after writting PDBSC_DACTOE = 1, DAC out   */
                                                                /* changes are base on the interval defined by this val */
    PDB0_DACINT1   = 50000;                                     /* Effective after writting PDBSC_DACTOE = 1, DAC out   */
                                                                /* changes are base on the interval defined by this val */
        
    PDB0_DACINTC0 |= PDB_INTC_TOE_MASK;                         /* DAC0 output delay from PDB Software trigger.         */
    PDB0_DACINTC1 |= PDB_INTC_TOE_MASK;                         /* DAC1 output delay from PDB Software trigger.         */
    
    PDB0_SC       |= PDB_SC_LDOK_MASK;                          /* Load the value assigned to PDB_DACINT0 to register.  */    
}


/*
*********************************************************************************************************
*                                    BSP_DAC_UpdateInterval()
*
* Description : Updates the DAC's interval register that for example specifies 
*               the frequency of the ECG signal which translates to higher or 
*               lower heart rate.
*
* Argument(s) : interval    interval value.
*               
* Return(s)   : none.
*
* Caller(s)   : bsp_dac.c.
*
* Note(s)     : Human heart rate ranges from 726 mHz (43 bpm) to 1.915 Hz (114 bpm).
*********************************************************************************************************
*/

void  BSP_DAC_UpdateInterval (CPU_INT08U dac_nbr, CPU_INT16U interval) 
{
    switch (dac_nbr) {
        case BSP_DAC_DAC0:
             PDB0_DACINT0 = BSP_DAC_DAC0_MIN_SPLS_DLY 
                          + (4 * interval);                     /* Update the DAC interval register.                    */
             break;

        case BSP_DAC_DAC1:             
             PDB0_DACINT1 = BSP_DAC_DAC1_MIN_SPLS_DLY 
                          + (4 * interval);                     /* Update the DAC interval register.                    */
             break;
             
        default:
             break;
    }
    PDB0_SC |= PDB_SC_LDOK_MASK;                                /* Load the value assigned to PDB_DACINT0 to register.  */
}


/*
*********************************************************************************************************
*                                   BSP_DAC_SwitchVoltage()
*
* Description : Updates the DAC's output amplitude.
*
* Argument(s) : amplitude    voltage selection
*               
* Return(s)   : none.
*
* Caller(s)   : bsp_dac.c.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_DAC_SwitchVoltage (CPU_INT08U dac_nbr, CPU_INT08U amplitude)
{
    DAC_MemMapPtr  dacx_base_ptr;


    switch (dac_nbr) {
        case BSP_DAC_DAC0:
             dacx_base_ptr = DAC0_BASE_PTR;                     /* Set the base address pointer for DAC0.               */
             break;

        case BSP_DAC_DAC1:
             dacx_base_ptr = DAC1_BASE_PTR;                     /* Set the base address pointer for DAC1.               */
             break;

        default:
             break;
    }

    switch (amplitude) {
        case BSP_DAC_AMPLITUDE_3000:                            /* 3V.                                                  */
             DAC_DATL_REG(dacx_base_ptr, 0) = 0xA1;
             DAC_DATH_REG(dacx_base_ptr, 0) = 0x0E;
             break;
               
        case BSP_DAC_AMPLITUDE_300:                             /* 300mV.                                               */
             DAC_DATL_REG(dacx_base_ptr, 0) = 0x72;
             DAC_DATH_REG(dacx_base_ptr, 0) = 0x01;
             break;
             
        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                   BSP_DAC_SetOutputVoltage()
*
* Description : Updates the DAC's output amplitude.
*
* Argument(s) : amplitude    voltage selection
*               
* Return(s)   : none.
*
* Caller(s)   : bsp_dac.c.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_DAC_SetOutputVoltage (CPU_INT08U dac_nbr, CPU_INT16U amplitude_cnts)
{
    DAC_MemMapPtr  dacx_base_ptr;


    switch (dac_nbr) {
        case BSP_DAC_DAC0:
             dacx_base_ptr = DAC0_BASE_PTR;                     /* Set the base address pointer for DAC0.               */
             break;

        case BSP_DAC_DAC1:
             dacx_base_ptr = DAC1_BASE_PTR;                     /* Set the base address pointer for DAC1.               */
             break;

        default:
             break;
    }

    DAC_DATL_REG(dacx_base_ptr, 0) = (amplitude_cnts & 0x00FF);
    DAC_DATH_REG(dacx_base_ptr, 0) = (amplitude_cnts & 0xFF00) >> 8;
}