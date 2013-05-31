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
*
*                                          Freescale Kinetis
*                                           featured on the
*                                        Freescale TWR-K53N512
*                                          Evaluation Board
*
* Filename      : bsp.c
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#define  BSP_MODULE
#include <includes.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  BSP_CPU_CLK_FREQ             50000000ul                /* CPU Clock Frequency.                                 */
#define  BSP_SYSTICK_PERIOD            2000000ul                /* System Tick Timer Period of 2 MHz.                   */


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

static  CPU_INT32U  BSP_CPU_ClkFreq_MHz;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  BSP_PLL_Init        (void);
static  void  BSP_LED_Init        (void);
static  void  BSP_TestPointsInit  (void);
static  void  BSP_StatusInit      (void);


/*
*********************************************************************************************************
*                                             REGISTERS
*********************************************************************************************************
*/

#define  DWT_CR      *(CPU_REG32 *)0xE0001000                   /* Data Watchpoint and Trace (DWT) Control Register     */
#define  DWT_CYCCNT  *(CPU_REG32 *)0xE0001004                   /* Data Watchpoint and Trace (DWT) Cycle Count Register */
#define  DEM_CR      *(CPU_REG32 *)0xE000EDFC
#define  DBGMCU_CR   *(CPU_REG32 *)0xE0042004                   /* Located in the PPB area only.                        */


/*
*********************************************************************************************************
*                                            REGISTER BITS
*********************************************************************************************************
*/

#define  DBGMCU_CR_TRACE_IOEN_MASK       0x10                   /* Located in the PPB area only.                        */
#define  DBGMCU_CR_TRACE_MODE_ASYNC      0x00                   /* Located in the PPB area only.                        */
#define  DBGMCU_CR_TRACE_MODE_SYNC_01    0x40                   /* Located in the PPB area only.                        */
#define  DBGMCU_CR_TRACE_MODE_SYNC_02    0x80                   /* Located in the PPB area only.                        */
#define  DBGMCU_CR_TRACE_MODE_SYNC_04    0xC0                   /* Located in the PPB area only.                        */
#define  DBGMCU_CR_TRACE_MODE_MASK       0xC0                   /* Located in the PPB area only.                        */

#define  DEM_CR_TRCENA                   (1 << 24)

#define  DWT_CR_CYCCNTENA                (1 <<  0)


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if ((CPU_CFG_TS_TMR_EN          != DEF_ENABLED) && \
     (APP_CFG_PROBE_OS_PLUGIN_EN == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN          == 1))
#error  "CPU_CFG_TS_EN                  illegally #define'd in 'cpu.h'"
#error  "                              [MUST be  DEF_ENABLED] when    "
#error  "                               using uC/Probe COM modules    "
#endif


/*
*********************************************************************************************************
*                                          BSP_PDB0_ISR()
*
* Description : Handle the PDB # 0 interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_PDB0_ISR (void)
{
    PDB0_SC &= ~PDB_SC_PDBIF_MASK;                              /* Clear interrupt mask.                                */
}


/*
*********************************************************************************************************
*                                          BSP_FTM0_ISR()
*
* Description : Handle the FTM # 0 interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_FTM0_ISR (void)
{
    if (DEF_BIT_IS_SET(FTM0_C4SC, FTM_CnSC_CHF_MASK)) {    
        if (FTM0_C4V > 0) {
            GPIOC_PTOR = DEF_BIT_11;                            /* Toggle the PWM signal that drives the motor.         */
        } else {
            GPIOC_PSOR = DEF_BIT_11;                            /* Set the output pin to turn off the motor.            */
        }
        FTM0_C4SC &= ~FTM_CnSC_CHF_MASK;                        /* Clear interrupt mask.                                */
    } else if (DEF_BIT_IS_SET(FTM0_C5SC, FTM_CnSC_CHF_MASK)) {    
        if (FTM0_C5V > 0) {
            GPIOC_PTOR = DEF_BIT_10;                            /* Toggle the PWM signal that drives the valve.         */
        } else {
            GPIOC_PSOR = DEF_BIT_10;                            /* Set the output pin to close the valve.               */
        }
        FTM0_C5SC &= ~FTM_CnSC_CHF_MASK;                        /* Clear interrupt mask.                                */
    }
}


/*
*********************************************************************************************************
*                                          BSP_FTM1_ISR()
*
* Description : Handle the FTM # 1 interrupt.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : This is an ISR.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_FTM1_ISR (void)
{
    if (DEF_BIT_IS_SET(FTM1_C0SC, FTM_CnSC_CHF_MASK)) {    
        if (FTM1_C0V > 0) {
            DAC_C0_REG(DAC1_BASE_PTR) |= DAC_C0_DACSWTRG_MASK;  /* Advance the read buffer pointer one position.        */
        }
        FTM1_C0SC &= ~FTM_CnSC_CHF_MASK;                        /* Clear interrupt mask.                                */
    }
}


/*
*********************************************************************************************************
*                                               BSP_Init()
*
* Description : Initialize the Board Support Package (BSP).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function SHOULD be called before any other BSP function is called.
*
*               (2) CPU instruction / data tracing requires the use of the following pins :
*                   (a) (1) Aysynchronous     :  PB[3]
*                       (2) Synchronous 1-bit :  PE[3:2]
*                       (3) Synchronous 2-bit :  PE[4:2]
*                       (4) Synchronous 4-bit :  PE[6:2]
*
*                   (b) The uC-Eval board MAY utilize the following pins depending on the application :
*                       (1) PE[5], MII_INT
*                       (1) PE[6], SDCard_Detection
*
*                   (c) The application may wish to adjust the trace bus width depending on I/O
*                       requirements.
*********************************************************************************************************
*/

void  BSP_Init (void)
{
    volatile  CPU_INT08U   dummy;


    WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;                  /* Disable the watchdog.                                */

    DWT_CR |= (CPU_INT32U)0x00000001;                           /* Enable Cortex-M4's DWT CYCCNT reg.                   */

    BSP_IntInit();                                              /* Initialize Interrupts.                               */

                                                                /* Setup the System Tick Timer.                         */
    dummy     = SYST_CSR;                                       /* Clear any pending interrupts.                        */
    SYST_RVR  = BSP_SYSTICK_PERIOD;                             /* Set period.                                          */
    SYST_CSR |= SysTick_CSR_CLKSOURCE_MASK;                     /* Clock source - System Clock.                         */
    SYST_CSR |= SysTick_CSR_TICKINT_MASK;                       /* Enable interrupt.                                    */
    SYST_CSR |= SysTick_CSR_ENABLE_MASK;                        /* Start Sys Timer.                                     */

//    SIM_SOPT2 |= SIM_SOPT2_MCGCLKSEL_MASK;                      /* RTC 32 kHz oscillator drives MCG clock.              */

    BSP_CPU_ClkFreq_MHz = BSP_CPU_ClkFreq() / (CPU_INT32U)1000000;
    BSP_CPU_ClkFreq_MHz = BSP_CPU_ClkFreq_MHz;                  /* Surpress compiler warning BSP_CPU_ClkFreq_MHz    ... */
                                                                /* ... set and not used.                                */

    BSP_PLL_Init();                                             /* Initialize the PLL clock.                            */
    
    BSP_LED_Init();                                             /* Initialize the I/Os for the LEDs.                    */

    BSP_TestPointsInit();                                       /* Initialize the I/Os for some Test Points.            */
    
    BSP_StatusInit();                                           /* Initialize the status inputs.                        */

#ifdef TRACE_EN                                                 /* See project / compiler preprocessor options.         */
    DBGMCU_CR |=  DBGMCU_CR_TRACE_IOEN_MASK;                    /* Enable tracing (see Note #2).                        */
    DBGMCU_CR &= ~DBGMCU_CR_TRACE_MODE_MASK;                    /* Clr trace mode sel bits.                             */
    DBGMCU_CR |=  DBGMCU_CR_TRACE_MODE_SYNC_04;                 /* Cfg trace mode to synch 4-bit.                       */
#endif
}


/*
*********************************************************************************************************
*                                            BSP_CPU_ClkFreq()
*
* Description : Read CPU registers to determine the CPU clock frequency of the chip.
*
* Argument(s) : none.
*
* Return(s)   : The CPU clock frequency, in Hz.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT32U  BSP_CPU_ClkFreq (void)
{
    return ((CPU_INT32U)BSP_CPU_CLK_FREQ);
}



/*
*********************************************************************************************************
*                                         PLL INITIALIZATION
*
* Description : This function is called to initialize the PLL.
*
* Arguments   : none
*********************************************************************************************************
*/

static  void  BSP_PLL_Init (void)
{
#if (CPU_CFG_CRITICAL_METHOD == CPU_CRITICAL_METHOD_STATUS_LOCAL)
    CPU_SR  cpu_sr;
#endif


    CPU_CRITICAL_ENTER();
    
    MCG_C2 = 0;                                                 /* Enable external oscillator                           */

    MCG_C1 = MCG_C1_CLKS(2) | MCG_C1_FRDIV(3);                  /* Select external oscilator and Reference Divider and  */
                                                                /* clear IREFS to start ext osc                         */
                                                                /* CLKS=2, FRDIV=3, IREFS=0, IRCLKEN=0, IREFSTEN=0      */
    
    while (MCG_S & MCG_S_IREFST_MASK) {};                       /* Wait for Reference clock Status bit to clear         */
    
                                                                /* Wait for clock status bits to show clock             */
                                                                /* source is ext ref clk                                */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2) {}; 
    
    MCG_C5 = MCG_C5_PRDIV(0x18);                                /* PLL Ref Divider, PLLCLKEN=0, PLLSTEN=0, PRDIV=0x18   */
    
    MCG_C6 = 0;                                                 /* Ensure MCG_C6 is at the reset default of 0           */
    
                                                                /* Set system options dividers                          */
    SIM_CLKDIV1 =   SIM_CLKDIV1_OUTDIV1(BSP_CORE_DIV - 1)    |  /* core/system clock                                    */
                    SIM_CLKDIV1_OUTDIV2(BSP_BUS_DIV - 1)     |  /* peripheral clock                                     */
                    SIM_CLKDIV1_OUTDIV3(BSP_FLEXBUS_DIV - 1) |  /* FlexBus clock driven to the external pin (FB_CLK).   */
                    SIM_CLKDIV1_OUTDIV4(BSP_FLASH_DIV - 1);     /* flash clock                                          */
    
                                                                /* Set the VCO divider and enable the PLL               */
    MCG_C6 = MCG_C6_PLLS_MASK | MCG_C6_VDIV(BSP_CLOCK_MUL - 24); 

    while (!(MCG_S & MCG_S_PLLST_MASK)) {};                     /* wait for PLL status bit to set                       */
    while (!(MCG_S & MCG_S_LOCK_MASK)) {};                      /* Wait for LOCK bit to set                             */

    MCG_C1 &= ~MCG_C1_CLKS_MASK;                                /* Transition into PEE by setting CLKS to 0             */

                                                                /* Wait for clock status bits to update                 */
    while (((MCG_S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x3) {};

    CPU_CRITICAL_EXIT();
}



/*
*********************************************************************************************************
*********************************************************************************************************
*                                              LED FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             BSP_LED_Init()
*
* Description : Initialize the I/O for the LEDs
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_LED_Init (void)
{    
    SIM_SCGC5   |= SIM_SCGC5_PORTC_MASK;                        /* Clock Gate Control: Enable the PORT C Clock.         */

    PORTC_PCR7  |= PORT_PCR_SRE_MASK                            /* Slow slew rate.                                      */
                |  PORT_PCR_ODE_MASK                            /* Open Drain Enable.                                   */
                |  PORT_PCR_DSE_MASK;                           /* High drive strength.                                 */
                   
    PORTC_PCR7   = PORT_PCR_MUX(1);                             /* Alternative 1 (GPIO).                                */

    PORTC_PCR8  |= PORT_PCR_SRE_MASK                            /* Slow slew rate.                                      */
                |  PORT_PCR_ODE_MASK                            /* Open Drain Enable.                                   */
                |  PORT_PCR_DSE_MASK;                           /* High drive strength.                                 */
                   
    PORTC_PCR8   = PORT_PCR_MUX(1);                             /* Alternative 1 (GPIO).                                */
    
    GPIOC_PDDR   = BSP_GPIOC_LED_ORANGE                         /* Output enable.                                       */
                |  BSP_GPIOC_LED_YELLOW; 

    BSP_LED_Off(BSP_LED_ALL);                                   /* Turn off all the LEDs.                               */

    PORTC_PCR10 |= PORT_PCR_SRE_MASK                            /* Slow slew rate.                                      */
                |  PORT_PCR_ODE_MASK                            /* Open Drain Enable.                                   */
                |  PORT_PCR_DSE_MASK;                           /* High drive strength.                                 */
                   
    PORTC_PCR10  = PORT_PCR_MUX(1);                             /* Alternative 1 (GPIO).                                */

    PORTC_PCR11 |= PORT_PCR_SRE_MASK                            /* Slow slew rate.                                      */
                |  PORT_PCR_ODE_MASK                            /* Open Drain Enable.                                   */
                |  PORT_PCR_DSE_MASK;                           /* High drive strength.                                 */
                   
    PORTC_PCR11  = PORT_PCR_MUX(1);                             /* Alternative 1 (GPIO).                                */
    
    GPIOC_PDDR  |= DEF_BIT_10                                   /* Output enable.                                       */
                |  DEF_BIT_11;
    
    GPIOC_PSOR   = DEF_BIT_10                                   /* Turn off Motor and Valve.                            */
                |  DEF_BIT_11;
}


/*
*********************************************************************************************************
*                                             BSP_LED_On()
*
* Description : Turn ON any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turns ON ALL the LEDs.
*                       1    turns ON user ORANGE LED on the board.
*                       2    turns ON user YELLOW LED on the board.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_On (CPU_INT08U led)
{
    switch (led) {
        case BSP_LED_ALL:
             GPIOC_PCOR = BSP_GPIOC_LED_ORANGE
                        | BSP_GPIOC_LED_YELLOW;
             break;

        case BSP_LED_ORANGE:
             GPIOC_PCOR = BSP_GPIOC_LED_ORANGE;
             break;

        case BSP_LED_YELLOW:
             GPIOC_PCOR = BSP_GPIOC_LED_YELLOW;
             break;

        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                              BSP_LED_Off()
*
* Description : Turn OFF any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turns ON ALL the LEDs.
*                       1    turns ON user ORANGE LED on the board.
*                       2    turns ON user YELLOW LED on the board.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Off (CPU_INT08U led)
{
    switch (led) {
        case BSP_LED_ALL:
             GPIOC_PSOR = BSP_GPIOC_LED_ORANGE
                        | BSP_GPIOC_LED_YELLOW;
             break;

        case BSP_LED_ORANGE:
             GPIOC_PSOR = BSP_GPIOC_LED_ORANGE;
             break;

        case BSP_LED_YELLOW:
             GPIOC_PSOR = BSP_GPIOC_LED_YELLOW;
             break;

        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                            BSP_LED_Toggle()
*
* Description : TOGGLE any or all the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       0    turns ON ALL the LEDs.
*                       1    turns ON user ORANGE LED on the board.
*                       2    turns ON user YELLOW LED on the board.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_LED_Toggle (CPU_INT08U led)
{
    switch (led) {
        case BSP_LED_ALL:
             GPIOC_PTOR = BSP_GPIOC_LED_ORANGE
                        | BSP_GPIOC_LED_YELLOW;
             break;

        case BSP_LED_ORANGE:
             GPIOC_PTOR = BSP_GPIOC_LED_ORANGE;
             break;

        case BSP_LED_YELLOW:
             GPIOC_PTOR = BSP_GPIOC_LED_YELLOW;
             break;

        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                            BSP_LED_Status()
*
* Description : Reads the status any of the LEDs on the board.
*
* Argument(s) : led     The ID of the LED to control:
*
*                       1    ORANGE LED on the board.
*                       2    YELLOW LED on the board.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_LED_Status (CPU_INT08U led)
{
    CPU_BOOLEAN  ret;


    switch (led) {
        case BSP_LED_ORANGE:
             ret = !(GPIOC_PDOR & BSP_GPIOC_LED_ORANGE);
             break;

        case BSP_LED_YELLOW:
             ret = !(GPIOC_PDOR & BSP_GPIOC_LED_YELLOW);
             break;

        default:
             ret = DEF_OFF;
             break;
    }
    
    return ret;
}


/*
*********************************************************************************************************
*                                          BSP_TestPointsInit()
*
* Description : Initialize the I/O for the Test Points.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_TestPointsInit (void)
{
    SIM_SCGC5  |= SIM_SCGC5_PORTB_MASK;                         /* Clock Gate Control: Enable the PORT B Clock.         */

    PORTB_PCR2 |= PORT_PCR_SRE_MASK                             /* Slow slew rate.                                      */
               |  PORT_PCR_ODE_MASK                             /* Open Drain Enable.                                   */
               |  PORT_PCR_DSE_MASK;                            /* High drive strength.                                 */

    PORTB_PCR2  = PORT_PCR_MUX(1);                              /* Alternative 1 (GPIO).                                */

    PORTB_PCR3 |= PORT_PCR_SRE_MASK                             /* Slow slew rate.                                      */
               |  PORT_PCR_ODE_MASK                             /* Open Drain Enable.                                   */
               |  PORT_PCR_DSE_MASK;                            /* High drive strength.                                 */

    PORTB_PCR3  = PORT_PCR_MUX(1);                              /* Alternative 1 (GPIO).                                */

    GPIOB_PDDR |= DEF_BIT_02                                    /* Output Enable.                                       */
               |  DEF_BIT_03;                                   /* Output Enable.                                       */
}


/*
*********************************************************************************************************
*                                            BSP_StatusInit()
*
* Description : Initialize the status port(s)
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : BSP_Init()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_StatusInit (void)
{                   
    PORTC_PCR5   = PORT_PCR_MUX(1);                             /* Alternative 1 (GPIO).                                */
    PORTC_PCR5  |= PORT_PCR_PS_MASK;                            /* Internal Pull Up Resitor  Selected.                  */
    PORTC_PCR5  |= PORT_PCR_PE_MASK;                            /* Internal Pull    Resistor Enabled.                   */
    PORTC_PCR5  |= PORT_PCR_PFE_MASK;                           /* Passive Low Pass Filter Enabled.                     */

    PORTC_PCR13  = PORT_PCR_MUX(1);                             /* Alternative 1 (GPIO).                                */
    PORTC_PCR13 |= PORT_PCR_PS_MASK;                            /* Internal Pull Up Resitor  Selected.                  */
    PORTC_PCR13 |= PORT_PCR_PE_MASK;                            /* Internal Pull    Resistor Enabled.                   */
    PORTC_PCR13 |= PORT_PCR_PFE_MASK;                           /* Passive Low Pass Filter Enabled.                     */
}


/*
*********************************************************************************************************
*                                            BSP_StatusRd()
*
* Description : Get the current status of a status input
*
* Argument(s) : id    is the status you want to get.
*
* Return(s)   : DEF_ON    if the status is asserted
*               DEF_OFF   if the status is negated
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  BSP_StatusRd (CPU_INT08U  id)
{
    CPU_BOOLEAN    bit_val;


    switch (id) {
        case BSP_PB_START:
             if ((GPIOC_PDIR & BSP_GPIOC_PB_START) == BSP_GPIOC_PB_START) {
                 bit_val = DEF_OFF;
             } else {
                 bit_val = DEF_ON;
             }
             return (bit_val);

        case BSP_PB_STOP:
             if ((GPIOC_PDIR & BSP_GPIOC_PB_STOP) == BSP_GPIOC_PB_STOP) {
                 bit_val = DEF_OFF;
             } else {
                 bit_val = DEF_ON;
             }
             return (bit_val);

        default:
             return ((CPU_BOOLEAN)DEF_OFF);
    }
}


/*
*********************************************************************************************************
*                                            BSP_FTM1_Init()
*
* Description : Initialize the FlexTimer in PWM mode.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_FTM1_Init (void)
{
    SIM_SCGC6 |= SIM_SCGC6_FTM1_MASK;                           /* Clock Gate Control: Enable the FTM 1 Clock.          */
    
    FTM1_SC    =  FTM_SC_CPWMS_MASK |                           /* FTM 0 Status and Control                             */
                  DEF_BIT_03 |                                  /* Center-Aligned PWM Selected.                         */
                  FTM_SC_PS(0);                                 /* System Clock source selected.                        */

    FTM1_C0SC  = FTM_CnSC_MSB_MASK |                            /* FTM 0 Ch 4 Status and Control.                       */
                 FTM_CnSC_ELSB_MASK | 
                 FTM_CnSC_CHIE_MASK;                            /* Edge-Aligned PWM.                                    */

    FTM1_CNTIN = 0;                                             /* Counter Initial Value.                               */
    FTM1_CNT   = 0;                                             /* Counter.                                             */
    
    FTM1_C0V   = 0x00000500;                                    /* Channel Value.                                       */
    FTM1_MOD   = 0x00005000;                                    /* Modulo.                                              */
    
    BSP_IntEn(BSP_INT_ID_FTM1);                                 /* Enable Interrupts on FTM1.                           */

    FTM1_MODE  = FTM_MODE_WPDIS_MASK;                           /* Write Protection Disabled.                           */
}


/*
*********************************************************************************************************
*                                            BSP_PWM_Init()
*
* Description : Initialize the FlexTimer in PWM mode.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_PWM_Init (void)
{
    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK;                          /* Clock Gate Control: Enable the PORT D Clock.         */

    PORTA_PCR11 = PORT_PCR_MUX(0x3);                            /* MUX Alternative 3: FTM 2 Ch 1.                       */

    SIM_SCGC3 |= SIM_SCGC3_FTM2_MASK;                           /* Clock Gate Control: Enable the FTM 2 Clock.          */
    
    FTM2_SC    =  FTM_SC_CPWMS_MASK |                           /* FTM 0 Status and Control                             */
                  DEF_BIT_03 |                                  /* Center-Aligned PWM Selected.                         */
                  FTM_SC_PS(0);                                 /* System Clock source selected.                        */

    FTM2_C1SC  = FTM_CnSC_MSB_MASK |                            /* FTM 0 Ch 4 Status and Control.                       */
                 FTM_CnSC_ELSB_MASK;                            /* Edge-Aligned PWM.                                    */
    
    FTM2_CNTIN = 0;                                             /* Counter Initial Value.                               */
    FTM2_CNT   = 0;                                             /* Counter.                                             */
    FTM2_MOD   = 0x00000500;                                    /* Modulo.                                              */    

    FTM2_C1V   = 0x00000125;                                    /* Channel Value.                                       */

    SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK;                          /* Clock Gate Control: Enable the PORT D Clock.         */

    PORTD_PCR4 = PORT_PCR_MUX(0x4);                             /* MUX Alternative 4: FTM 0 Ch 4.                       */
    PORTD_PCR5 = PORT_PCR_MUX(0x4);                             /* MUX Alternative 4: FTM 0 Ch 5.                       */

    SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;                           /* Clock Gate Control: Enable the FTM 0 Clock.          */
    
    FTM0_SC    =  FTM_SC_CPWMS_MASK |                           /* FTM 0 Status and Control                             */
                  DEF_BIT_03 |                                  /* Center-Aligned PWM Selected.                         */
                  FTM_SC_PS(0);                                 /* System Clock source selected.                        */

    FTM0_C4SC  = FTM_CnSC_MSB_MASK |                            /* FTM 0 Ch 4 Status and Control.                       */
                 FTM_CnSC_ELSB_MASK | 
                 FTM_CnSC_CHIE_MASK;                            /* Edge-Aligned PWM.                                    */

    FTM0_C5SC  = FTM_CnSC_MSB_MASK |                            /* FTM 0 Ch 5 Status and Control.                       */
                 FTM_CnSC_ELSB_MASK | 
                 FTM_CnSC_CHIE_MASK;                            /* Edge-Aligned PWM.                                    */
    
    FTM0_CNTIN = 0;                                             /* Counter Initial Value.                               */
    FTM0_CNT   = 0;                                             /* Counter.                                             */
    FTM0_MOD   = 0x00000500;                                    /* Modulo.                                              */

    FTM0_C4V   = 0x00000000;                                    /* Channel Value.                                       */
    FTM0_C5V   = 0x00000000;                                    /* Channel Value.                                       */

    GPIOC_PSOR = DEF_BIT_10;
    GPIOC_PSOR = DEF_BIT_11;
    
    BSP_IntEn(BSP_INT_ID_FTM0);                                 /* Enable Interrupts on FTM0.                           */
}


/*
*********************************************************************************************************
*                                            Op Amps Init
*
* Description : Initialize the MCU's Operational Amplifiers in General-Purpose Mode.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_OPAMP_Init (CPU_INT08U module, CPU_INT08U mode, CPU_INT08U gain)
{
    SIM_SCGC1 |= SIM_SCGC1_OPAMP_MASK;                          /* Clock Gate Control: Enable OpAmps.                   */

    switch (module) {
        case BSP_OPAMP_MODULE_0:
             OPAMP0_C1  = BSP_OPAMP_GainTbl[gain];              /* Gain selection ignored in general-purpose mode.      */
             OPAMP0_C2 |= (OPAMP_C2_AMPNSEL(0) | 
                           OPAMP_C2_AMPPSEL(0));                /* Negative input: OP0_DM0 and Positive input: OP0_DP0. */
             OPAMP0_C0 |= (OPAMP_C0_EN_MASK | 
                           OPAMP_C0_MODE(mode));                /* OpAmp 0 Enabled in general purpose mode.             */
             break;

        case BSP_OPAMP_MODULE_1:
             OPAMP1_C1  = BSP_OPAMP_GainTbl[gain];              /* Gain selection ignored in general-purpose mode.      */
             OPAMP1_C2 |= (OPAMP_C2_AMPNSEL(0) | 
                           OPAMP_C2_AMPPSEL(0));                /* Negative input: OP1_DM0 and Positive input: OP1_DP0. */
             OPAMP1_C0 |= (OPAMP_C0_EN_MASK | 
                           OPAMP_C0_MODE(mode));                /* OpAmp 1 Enabled and Non-Inverted PGA mode.           */
             break;
    }
}


/*
*********************************************************************************************************
*                                       Transimpedance Amps Init
*
* Description : Initialize the Transimpedance Amplifiers.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_TRIAMP_Init (void)
{
    SIM_SCGC1  |= SIM_SCGC1_TRIAMP_MASK;                        /* Clock Gate Control: Enable TriAmps.                  */

    TRIAMP0_C0  = TRIAMP_C0_TRIAMPEN_MASK;                      /* TriAmp0 Enabled.                                     */
    TRIAMP1_C0  = TRIAMP_C0_TRIAMPEN_MASK;                      /* TriAmp1 Enabled.                                     */
}


/*
*********************************************************************************************************
*                                            BSP_BPM_CloseValve()
*
* Description : Closes the solenoid valve by energizing the coil via the PWM module assuming 
*               the solenoid valve is Normally Opened (N.O.).
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_BPM_CloseValve (CPU_INT08U percentage)
{
    CPU_INT16U  duty_cycle;


                                                                /* Calculates the counts for the specified duty cycle.  */
    duty_cycle = (CPU_INT16U)((CPU_FP32)FTM2_MOD * 
                              (CPU_FP32)((CPU_FP32)percentage / 
                                         (CPU_FP32)100));
    FTM2_C1V     = duty_cycle;
    PORTA_PCR11  = PORT_PCR_MUX(3);                             /* Enables PWM output through FTM2 channel 1.           */
    FTM2_SC     |= FTM_SC_CLKS(1);                              /* System clock on FTM2, start generating PWM.          */
}


/*
*********************************************************************************************************
*                                            BSP_BPM_StartMotor()
*
* Description : Start pumping air into the cuff by starting the motor via the PWM module.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_BPM_StartMotor (void)
{
    FTM0_CNT    = 0;                                            /* Counter.                                             */
    FTM0_C4SC  |= FTM_CnSC_CHIE_MASK;                           /* Enable interrupts from this channel.                 */
    FTM0_C4V    = 0x00000010;                                   /* 50% duty cycle.                                      */
    GPIOC_PCOR  = DEF_BIT_11;                                   /* Clear the pin low to turn on the motor.              */
}


/*
*********************************************************************************************************
*                                            BSP_BPM_StopMotor()
*
* Description : Stops pumping air into the cuff by stopping the motor.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_BPM_StopMotor (void)
{
    FTM0_C4SC  &= ~FTM_CnSC_CHIE_MASK;                          /* Disable interrupts from this channel.                */
    FTM0_CNT    =  0;                                           /* Counter.                                             */
    FTM0_C4V    =  0x00000000;                                  /* 0% duty cycle.                                       */
    GPIOC_PSOR  =  DEF_BIT_11;                                  /* Set the pin high to turn off the motor.              */
}


/*
*********************************************************************************************************
*                             Select a pulse oxymetry sensor's analog channel
*
* Description : Switches between the IR and Red LED analog channel by setting or clearing
*               the control signal of a MUX.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_POX_MUX_Select (CPU_INT08U wavelength)
{     
    if (wavelength == 1) {
        GPIOC_PSOR = DEF_BIT_11;
    } else if (wavelength == 2){
        GPIOC_PCOR = DEF_BIT_11;
    }
}


/*
*********************************************************************************************************
*                                   Turn on pulse oxymetry sensor's LED
*
* Description : Turns on the pulse oximetry sensor's LED by turning on the PWM output.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_POX_LED_On (CPU_INT08U intensity)
{
    CPU_INT16U  duty_cycle;
    
                                                                /* Calculates the counts for the specified intensity.   */
    duty_cycle = (CPU_INT16U)((CPU_FP32)FTM2_MOD * 
                              (CPU_FP32)((CPU_FP32)intensity / 
                                         (CPU_FP32)100));
    FTM2_C1V     = duty_cycle;
    PORTA_PCR11  = PORT_PCR_MUX(3);                             /* Enables PWM output through FTM2 channel 1.           */
    FTM2_SC     |= FTM_SC_CLKS(1);                              /* System clock on FTM2, start generating PWM.          */
}


/*
*********************************************************************************************************
*                                   Turn off pulse oxymetry sensor's LED
*
* Description : Turns off the pulse oximetry sensor's LED by turning off the PWM output.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : application
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  BSP_POX_LED_Off (void)
{
    PORTA_PCR11  =  PORT_PCR_MUX(1);                            /* Set pin as GPIO.                                     */
    GPIOA_PDDR  |= (1 << 11);                                   /* Set pin as output.                                   */
    GPIOA_PCOR  |= (1 << 11);                                   /* Clear the output pin.                                */
    FTM2_SC     &= ~FTM_SC_CLKS_MASK;                           /* Turn off the clock for FTM2.                         */

    FTM2_CNT = 0;                                               /* Resets the counter.                                  */
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           OS PROBE FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           OSProbe_TmrInit()
*
* Description : Select & initialize a timer for use with the uC/Probe Plug-In for uC/OS-III.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : OSProbe_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if ((APP_CFG_PROBE_OS_PLUGIN_EN == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN          == 1))
void  OSProbe_TmrInit (void)
{
}
#endif


/*
*********************************************************************************************************
*                                            OSProbe_TmrRd()
*
* Description : Read the current counts of a 16-bit free running timer.
*
* Argument(s) : none.
*
* Return(s)   : The 16-bit counts of the timer in a 32-bit variable.
*
* Caller(s)   : OSProbe_TimeGetCycles().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if ((APP_CFG_PROBE_OS_PLUGIN_EN == DEF_ENABLED) && \
     (OS_PROBE_HOOKS_EN          == 1))
CPU_INT32U  OSProbe_TmrRd (void)
{
    return ((CPU_INT32U)DWT_CYCCNT);
}
#endif


/*
*********************************************************************************************************
*                                          CPU_TS_TmrInit()
*
* Description : Initialize & start CPU timestamp timer.
*
* Argument(s) : none.
*
* Return(s)   : Number of left-shifts to scale & return timer as (32-bit) 'CPU_TS' data type
*                 (see Note #1a1), if necessary.
*
*               0 (see Note #1a2), otherwise.
*
* Caller(s)   : CPU_TS_Init().
*
* Note(s)     : (1) (a) Timer count values MUST be scaled & returned via (32-bit) 'CPU_TS' data type.
*
*                       (1) If timer used has less bits, left-shift timer values until the most
*                           significant bit of the timer value is shifted into         the most
*                           significant bit of the return timestamp data type.
*                       (2) If timer used has more bits, truncate   timer values' higher-order
*                           bits greater than  the return timestamp data type.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                   (c) When applicable, timer period SHOULD be less than the typical measured time
*                       but MUST be less than the maximum measured time; otherwise, timer resolution
*                       inadequate to measure desired times.
*
*                   See also 'CPU_TS_TmrRd()  Note #1'.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_TmrInit (void)
{
    CPU_INT32U  cpu_clk_freq_hz;
      
      
    DEM_CR         |= (CPU_INT32U)DEM_CR_TRCENA;                /* Enable Cortex-M4's DWT CYCCNT reg.                   */
    DWT_CYCCNT      = (CPU_INT32U)0u;
    DWT_CR         |= (CPU_INT32U)DWT_CR_CYCCNTENA;

    cpu_clk_freq_hz = BSP_CPU_ClkFreq();    
    CPU_TS_TmrFreqSet(cpu_clk_freq_hz);
}
#endif


/*
*********************************************************************************************************
*                                           CPU_TS_TmrRd()
*
* Description : Get current CPU timestamp timer count value.
*
* Argument(s) : none.
*
* Return(s)   : (32-bit) Timestamp timer count (see Notes #1a & #1b).
*
* Caller(s)   : CPU_TS_Init(),
*               CPU_TS_UpdateHandler(),
*               CPU_IntDisMeasStart(),
*               CPU_IntDisMeasStop().
*
* Note(s)     : (1) (a) Timer count values MUST be returned via (32-bit) 'CPU_TS' data type.
*
*                       (1) If timer used has less bits, left-shift timer values until the most
*                           significant bit of the timer value is shifted into         the most
*                           significant bit of the return timestamp data type.
*                       (2) If timer used has more bits, truncate   timer values' higher-order
*                           bits greater than  the return timestamp data type.
*
*                   (b) Timer SHOULD be an 'up'  counter whose values increase with each time count.
*
*                       (1) If timer is a 'down' counter whose values decrease with each time count,
*                           then the returned timer value MUST be ones-complemented.
*
*                   (c) (1) When applicable, the amount of time measured by CPU timestamps is
*                           calculated by the following equation :
*
*                               Time measured  =  Timer period  *  Number timer counts
*
*                                   where
*
*                                       Timer period            Timer's period in some units of
*                                                                   (fractional) seconds
*                                       Number timer counts     Number of timer counts measured
*                                       Time measured           Amount of time measured, in same
*                                                                   units of (fractional) seconds
*                                                                   as the Timer period
*
*                       (2) Timer period SHOULD be less than the typical measured time but MUST be less
*                           than the maximum measured time; otherwise, timer resolution inadequate to
*                           measure desired times.
*********************************************************************************************************
*/

#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
CPU_TS  CPU_TS_TmrRd (void)
{
    return ((CPU_TS)DWT_CYCCNT);
}
#endif