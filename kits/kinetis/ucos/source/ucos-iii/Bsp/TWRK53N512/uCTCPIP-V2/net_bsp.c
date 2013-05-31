/*
*********************************************************************************************************
*                                            uC/TCP-IP V2
*                                      The Embedded TCP/IP Suite
*                                            EXAMPLE CODE
*
*                          (c) Copyright 2003-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*
*               uC/TCP-IP is provided in source form to registered licensees ONLY.  It is 
*               illegal to distribute this source code to any third party unless you receive 
*               written permission by an authorized Micrium representative.  Knowledge of 
*               the source code may NOT be used to develop a similar product.  However, 
*               please feel free to use any application code labeled as 'EXAMPLE CODE' in 
*               your application products.  Example code may be used as is, in whole or in 
*               part, or may be used as a reference only.
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
*                            NETWORK BOARD SUPPORT PACKAGE (BSP) FUNCTIONS
*

*                                Freescale TWR-K53N512 Evaluation Board
*
* Filename      : net_bsp.c
* Version       : V2.10
* Programmer(s) : AF
*                 ITJ
*********************************************************************************************************
* Note(s)       : (1) Assumes the following versions (or more recent) of software modules are included in
*                     the project build :
*
*                     (a) uC/TCP-IP V2.02
*                     (b) uC/OS-II  V2.86 or
*                         uC/OS-III V3.00.0
*                     (c) uC/LIB    V1.27
*                     (d) uC/CPU    V1.20
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define    NET_BSP_MODULE
#include  <net.h>
#include  <bsp.h>


/*$PAGE*/
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
*                                       LOCAL GLOBAL VARIABLES
*
* Note(s) : (1) (a) Each network device maps to a unique network interface number.
*
*               (b) Instances of network devices' interface number SHOULD be named using the following 
*                   convention :
*
*                       <Board><Device>[Number] _IF_Nbr
*
*                           where
*                                   <Board>         Development board name
*                                   <Device>        Network device name (or type)
*                                   [Number]        Network device number for each specific instance 
*                                                       of device (optional if the development board 
*                                                       does NOT support multiple instances of the 
*                                                       specific device)
*
*                   For example, the network device interface number variable for the #2 MACB Ethernet 
*                   controller on an Atmel AT91SAM92xx should be named 'AT91SAM92xx_MACB_2_IF_Nbr'.
*
*               (c) Network device interface number variables SHOULD be initialized to 'NET_IF_NBR_NONE'.
*********************************************************************************************************
*/

static  NET_IF_NBR  KxxN512_0_IF_Nbr = NET_IF_NBR_NONE;


/*$PAGE*/
/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*
* Note(s) : (1) Device driver BSP functions may be arbitrarily named.  However, it is recommended that 
*               device BSP functions be named using the names provided below.
*
*               (a) (1) BSP interface functions SHOULD be named using the following convention :
*
*                           NetDev_<Function Name>[Number] ()
*
*                               where
*                                       <Function Name>     Network device BSP function name
*                                       [Number]            Network device number for each specific instance 
*                                                               of device (optional if the development board 
*                                                               does NOT support multiple instances of the 
*                                                               specific device)
*
*                       For example, the NetDev_CfgClk() function for the #2 MACB Ethernet controller 
*                       on an Atmel AT91SAM92xx should be named NetDev_CfgClk2{}.
*
*
*                   (2) BSP-level device ISR handlers SHOULD be named using the following convention :
*
*                           NetDev_ISR_Handler<Device><Type>[Number] ()
*
*                               where
*                                       <Device>            Network device name or type
*                                       <Type>              Network device interrupt type (optional, 
*                                                               if generic interrupt type or unknown)
*                                       [Number]            Network device number for each specific instance 
*                                                               of device (optional if the development board 
*                                                               does NOT support multiple instances of the 
*                                                               specific device)
*
*               (b) All BSP function prototypes should be located within the development board's network 
*                   BSP C source file ('net_bsp.c') & be declared as static functions to prevent name 
*                   clashes with other network protocol suite BSP functions/files.
*********************************************************************************************************
*/

#if (NET_VERSION >= 21000u)
static  void        NetDev_CfgClk                (NET_IF   *pif,
                                                  NET_ERR  *perr);

static  void        NetDev_CfgIntCtrl            (NET_IF   *pif,
                                                  NET_ERR  *perr);
#if 0
static  void        NetDev_CfgGPIO_MII           (NET_IF   *pif,
                                                  NET_ERR  *perr);
#endif

static  void        NetDev_CfgGPIO_RMII          (NET_IF   *pif,
                                                  NET_ERR  *perr);

static  CPU_INT32U  NetDev_ClkFreqGet            (NET_IF   *pif,
                                                  NET_ERR  *perr);
#endif

static  void        NetBSP_ISR_HandlerKxxN512_0(void);


/*$PAGE*/
/*
*********************************************************************************************************
*                                            LOCAL TABLES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                    NETWORK DEVICE BSP INTERFACE
*
* Note(s) : (1) Device board-support package (BSP) interface structures are used by the device driver to 
*               call specific devices' BSP functions via function pointer instead of by name.  This enables 
*               the network protocol suite to compile & operate with multiple instances of multiple devices 
*               & drivers.
*
*           (2) In most cases, the BSP interface structure provided below SHOULD suffice for most devices' 
*               BSP functions exactly as is with the exception that BSP interface structures' names MUST be 
*               unique & SHOULD clearly identify the development board, device name, & possibly the specific 
*               device number (if the development board supports multiple instances of any given device).
*
*               (a) BSP interface structures SHOULD be named using the following convention :
*
*                       NetDev_BSP_<Board><Device>[Number] {}
*
*                           where
*                                   <Board>         Development board name
*                                   <Device>        Network device name (or type)
*                                   [Number]        Network device number for each specific instance 
*                                                       of device (optional if the development board 
*                                                       does NOT support multiple instances of the 
*                                                       specific device)
*
*                   For example, the BSP interface structure for the #2 MACB Ethernet controller on an 
*                   Atmel AT91SAM92xx should be named NetDev_BSP_AT91SAM92xx_MACB_2{}.
*
*               (b) The API structure MUST also be externally declared in the development board's network 
*                   BSP header file ('net_bsp.h') with the exact same name & type.
*********************************************************************************************************
*/

#if (NET_VERSION >= 21000u)                                                 /* KxxN512 BSP fnct ptrs :                  */
const  NET_DEV_BSP_ETHER  NetDev_BSP_KxxN512   = { NetDev_CfgClk,           /*   Cfg clk(s)                             */
                                                   NetDev_CfgIntCtrl,       /*   Cfg int ctrl(s)                        */
                                                   NetDev_CfgGPIO_RMII,     /*   Cfg GPIO                               */
                                                   NetDev_ClkFreqGet        /*   Get clk freq                           */
                                                 };
#endif


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*$PAGE*/
/*
*********************************************************************************************************
*********************************************************************************************************
*                                   NETWORK DEVICE DRIVER FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           NetDev_CfgClk()
*
* Description : Configure clocks for the specified interface/device.
*
* Argument(s) : pif         Pointer to network interface to configure.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device clock successfully configured.
*                               NET_DEV_ERR_INVALID_CFG         Invalid device/PHY configuration.
*
* Return(s)   : none.
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : (1) The external Ethernet Phy may be clocked using several different sources.  The
*                   resulting Phy clock MUST be 25MHz for MII and 50MHz for RMII :
*
*                   (a) RCC_MCO_NoClock    External Phy clocked directly via external crystal.
*                   (b) RCC_MCO_XT1        External Phy clocked by HSE   via MCO output.
*                   (c) RCC_MCO_PLL3CLK    External Phy clocked by PLL3  via MCO output.
*********************************************************************************************************
*/

#if (NET_VERSION >= 21000u)
static  void  NetDev_CfgClk (NET_IF   *pif,
                             NET_ERR  *perr)
{
   (void)&pif;                                                  /* Prevent 'variable unused' compiler warning.          */

   
    SIM_SCGC2 |= SIM_SCGC2_ENET_MASK;
    MPU_CESR   = 0;         
   
   *perr = NET_DEV_ERR_NONE;
}
#endif


/*
*********************************************************************************************************
*                                         NetDev_CfgIntCtrl()
*
* Description : Configure interrupt controller for the specified interface/device.
*
* Argument(s) : pif         Pointer to network interface to configure.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device interrupt(s) successfully configured.
*
* Return(s)   : none.
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : none.
*
* Note(s)     : (1) Phy interrupts NOT supported when using the Generic Ethernet Phy driver.
*********************************************************************************************************
*/

#if (NET_VERSION >= 21000u)
static  void  NetDev_CfgIntCtrl (NET_IF   *pif,
                                 NET_ERR  *perr)
{
    KxxN512_0_IF_Nbr = pif->Nbr;                                /* Configure this device's BSP instance with specific   */ 
                                                                /* interface number.                                    */

    BSP_IntVectSet(BSP_INT_ID_ENET_TRANSMIT, NetBSP_ISR_HandlerKxxN512_0);
    BSP_IntEn(BSP_INT_ID_ENET_TRANSMIT);
    BSP_IntVectSet(BSP_INT_ID_ENET_RECEIVE, NetBSP_ISR_HandlerKxxN512_0);
    BSP_IntEn(BSP_INT_ID_ENET_RECEIVE);
    BSP_IntVectSet(BSP_INT_ID_ENET_ERROR, NetBSP_ISR_HandlerKxxN512_0);
    BSP_IntEn(BSP_INT_ID_ENET_ERROR);

   *perr = NET_DEV_ERR_NONE;
}
#endif




/*
*********************************************************************************************************
*                                          NetDev_CfgGPIO_RMII()
*
* Description : Configure general-purpose I/O (GPIO) for the specified interface/device.
*
* Argument(s) : pif         Pointer to network interface to configure.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device GPIO successfully configured.
*                               NET_DEV_ERR_INVALID_CFG         Invalid device/PHY configuration.
*
* Return(s)   : none.
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if (NET_VERSION >= 21000u)
static  void  NetDev_CfgGPIO_RMII (NET_IF   *pif,
                                   NET_ERR  *perr)
{
   (void)&pif;                                                  /* Prevent 'variable unused' compiler warning.          */

   
    SIM_SCGC5    |= SIM_SCGC5_PORTA_MASK;                       /* Clock Gate Control: Enable the PORT A Clock.         */

    PORTA_PCR12  |= PORT_PCR_SRE_MASK                           /* Slow slew rate.                                      */
                 |  PORT_PCR_ODE_MASK                           /* Open Drain Enable.                                   */
                 |  PORT_PCR_DSE_MASK;                          /* High drive strength.                                 */
    
    PORTA_PCR12   = PORT_PCR_MUX(4);                            /* Alternative 4 (RMII0).                               */

    PORTA_PCR13  |= PORT_PCR_SRE_MASK                           /* Slow slew rate.                                      */
                 |  PORT_PCR_ODE_MASK                           /* Open Drain Enable.                                   */
                 |  PORT_PCR_DSE_MASK;                          /* High drive strength.                                 */
    
    PORTA_PCR13   = PORT_PCR_MUX(4);                            /* Alternative 4 (RMII0).                               */
    
    PORTA_PCR14  |= PORT_PCR_SRE_MASK                           /* Slow slew rate.                                      */
                 |  PORT_PCR_ODE_MASK                           /* Open Drain Enable.                                   */
                 |  PORT_PCR_DSE_MASK;                          /* High drive strength.                                 */
    
    PORTA_PCR14   = PORT_PCR_MUX(4);                            /* Alternative 4 (RMII0).                               */
    
    PORTA_PCR15  |= PORT_PCR_SRE_MASK                           /* Slow slew rate.                                      */
                 |  PORT_PCR_ODE_MASK                           /* Open Drain Enable.                                   */
                 |  PORT_PCR_DSE_MASK;                          /* High drive strength.                                 */

    PORTA_PCR15   = PORT_PCR_MUX(4);                            /* Alternative 4 (RMII0).                               */

    PORTA_PCR16  |= PORT_PCR_SRE_MASK                           /* Slow slew rate.                                      */
                 |  PORT_PCR_ODE_MASK                           /* Open Drain Enable.                                   */
                 |  PORT_PCR_DSE_MASK;                          /* High drive strength.                                 */

    PORTA_PCR16   = PORT_PCR_MUX(4);                            /* Alternative 4 (RMII0).                               */
    
    PORTA_PCR17  |= PORT_PCR_SRE_MASK                           /* Slow slew rate.                                      */
                 |  PORT_PCR_ODE_MASK                           /* Open Drain Enable.                                   */
                 |  PORT_PCR_DSE_MASK;                          /* High drive strength.                                 */

    PORTA_PCR17   = PORT_PCR_MUX(4);                            /* Alternative 4 (RMII0).                               */


    SIM_SCGC5    |= SIM_SCGC5_PORTB_MASK;                       /* Clock Gate Control: Enable the PORT B Clock.         */

    PORTB_PCR0   |= PORT_PCR_SRE_MASK                           /* Slow slew rate.                                      */
                 |  PORT_PCR_ODE_MASK                           /* Open Drain Enable.                                   */
                 |  PORT_PCR_DSE_MASK;                          /* High drive strength.                                 */
    
    PORTB_PCR0    = PORT_PCR_MUX(4);                            /* Alternative 4 (RMII0).                              */    

    PORTB_PCR1   |= PORT_PCR_SRE_MASK                           /* Slow slew rate.                                      */
                 |  PORT_PCR_ODE_MASK                           /* Open Drain Enable.                                   */
                 |  PORT_PCR_DSE_MASK;                          /* High drive strength.                                 */
    
    PORTB_PCR1    = PORT_PCR_MUX(4);                            /* Alternative 4 (RMII0).                              */    
   
    
   *perr = NET_DEV_ERR_NONE;
}
#endif


/*
*********************************************************************************************************
*                                          NetDev_CfgGPIO_MII()
*
* Description : Configure general-purpose I/O (GPIO) for the specified interface/device.
*
* Argument(s) : pif         Pointer to network interface to configure.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device GPIO successfully configured.
*                               NET_DEV_ERR_INVALID_CFG         Invalid device/PHY configuration.
*
* Return(s)   : none.
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

#if 0
#if (NET_VERSION >= 21000u)
static  void  NetDev_CfgGPIO_MII (NET_IF   *pif,
                                  NET_ERR  *perr)
{
   (void)&pif;                                                  /* Prevent 'variable unused' compiler warning.          */

    PORTB_PCR0  = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_MDIO                           */
    PORTB_PCR1  = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_MDC                            */    
    PORTA_PCR14 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_RXDV                           */
//    PORTA_PCR5  = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_RXER                           */
    PORTA_PCR12 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_RXD1                           */
    PORTA_PCR13 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_RXD0                           */
    PORTA_PCR15 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_TXEN                           */
    PORTA_PCR16 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_TXD0                           */
    PORTA_PCR17 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_TXD1                           */
    PORTA_PCR11 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_RXCLK                          */
    PORTA_PCR25 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_TXCLK                          */
    PORTA_PCR9  = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_RXD3                           */
    PORTA_PCR10 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_RXD2                           */  
    PORTA_PCR28 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_TXER                           */
    PORTA_PCR24 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_TXD2                           */
    PORTA_PCR26 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_TXD3                           */
    PORTA_PCR27 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_CRS                            */
    PORTA_PCR29 = PORT_PCR_MUX(4);                              /* Configure MACNET MII0_COL                            */
    
   *perr = NET_DEV_ERR_NONE;
}
#endif
#endif

/*
*********************************************************************************************************
*                                         NetDev_ClkFreqGet()
*
* Description : Get device clock frequency.
*
* Argument(s) : pif         Pointer to network interface to get clock frequency.
*               ---         Argument validated in NetDev_Init().
*
*               perr        Pointer to variable that will receive the return error code from this function :
*
*                               NET_DEV_ERR_NONE                Device clock frequency successfully returned.
*
* Return(s)   : MAC Device clock divider frequency (in Hz).
*
* Caller(s)   : NetDev_Init().
*
* Note(s)     : (1) The MDC input clock frequency is used by the device driver to determine which MDC
*                   divider to select in order to reduce the MDC / MII bus frequency to the ~2MHz range.
*********************************************************************************************************
*/

#if (NET_VERSION >= 21000u)
static  CPU_INT32U  NetDev_ClkFreqGet (NET_IF   *pif,
                                       NET_ERR  *perr)
{
    CPU_INT32U  clk_freq;


    (void)&pif;                                                 /* Prevent 'variable unused' compiler warning.          */

    clk_freq = 2 * BSP_CPU_ClkFreq();
//    clk_freq = periph_clk_khz;
    
   *perr     = NET_DEV_ERR_NONE;

    return (clk_freq);
}
#endif


/*
*********************************************************************************************************
*                                   NetBSP_ISR_HandlerKxxN512_0()
*
* Description : BSP-level ISR handler for device receive & transmit interrupts.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : CPU &/or device interrupts.
*
* Note(s)     : (1) (a) Each device interrupt, or set of device interrupts, MUST be handled by a 
*                       unique BSP-level ISR handler which maps each specific device interrupt to 
*                       its corresponding network interface ISR handler.
*
*                   (b) BSP-level device ISR handlers SHOULD be named using the following convention :
*
*                           NetDev_ISR_Handler<Dev><Type>[Nbr]()
*
*                               where
*                                   (1) Dev         Network device name or type
*                                   (2) Type        Network device interrupt type (optional, 
*                                                       if generic interrupt type or unknown)
*                                   (3) Nbr         Network device number (optional; 
*                                                       see 'NETWORK DEVICE BSP INTERFACE  Note #2a3')
*********************************************************************************************************
*/

static  void  NetBSP_ISR_HandlerKxxN512_0 (void)
{
    NET_IF_NBR        if_nbr;
    NET_DEV_ISR_TYPE  type;
    NET_ERR           err;


    if_nbr = KxxN512_0_IF_Nbr;                                  /* See Note #2b3.                                       */
    type   = NET_DEV_ISR_TYPE_UNKNOWN;                          /* See Note #2b2.                                       */

    NetIF_ISR_Handler(if_nbr, type, &err);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      NETWORK MODULE FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                          NetUtil_TS_Get()
*
* Description : Get current Internet Timestamp.
*
*               (1) "The Timestamp is a right-justified, 32-bit timestamp in milliseconds since midnight
*                    UT [Universal Time]" (RFC #791, Section 3.1 'Options : Internet Timestamp').
*
*               (2) The developer is responsible for providing a real-time clock with correct time-zone
*                   configuration to implement the Internet Timestamp.
*
*
* Argument(s) : none.
*
* Return(s)   : Internet Timestamp.
*
* Caller(s)   : various.
*
*               This function is an INTERNAL network protocol suite function but MAY be called by
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

NET_TS  NetUtil_TS_Get (void)
{
    NET_TS  ts;


    /* $$$$ Insert code to return Internet Timestamp (see Notes #1 & #2).   */

    ts = NET_TS_NONE;

    return (ts);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         NetUtil_TS_Get_ms()
*
* Description : Get current millisecond timestamp.
*
*               (1) (a) (1) Although RFC #2988, Section 4 states that "there is no requirement for the
*                           clock granularity G used for computing [TCP] RTT measurements ... experience
*                           has shown that finer clock granularities (<= 100 msec) perform somewhat
*                           better than more coarse granularities".
*
*                       (2) (A) RFC #2988, Section 2.4 states that "whenever RTO is computed, if it is
*                               less than 1 second then the RTO SHOULD be rounded up to 1 second".
*
*                           (B) RFC #1122, Section 4.2.3.1 states that "the recommended ... RTO ... upper
*                               bound should be 2*MSL" where RFC #793, Section 3.3 'Sequence Numbers :
*                               Knowing When to Keep Quiet' states that "the Maximum Segment Lifetime
*                               (MSL) is ... to be 2 minutes".
*
*                               Therefore, the required upper bound is :
*
*                                   2 * MSL = 2 * 2 minutes = 4 minutes = 240 seconds
*
*                   (b) Therefore, the developer is responsible for providing a timestamp clock with
*                       adequate resolution to satisfy the clock granularity (see Note #1a1) & adequate
*                       range to satisfy the minimum/maximum TCP RTO values  (see Note #1a2).
*
* Argument(s) : none.
*
* Return(s)   : Timestamp, in milliseconds.
*
* Caller(s)   : NetIF_PerfMonHandler(),
*               NetTCP_RxPktValidate(),
*               NetTCP_TxPktPrepareHdr().
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by
*               application function(s).
*
* Note(s)     : (2) (a) To avoid  timestamp calculation overflow, timestamps are updated by continually
*                       summing OS time tick delta differences converted into milliseconds :
*
*                                  Total                       [                        1000 ms/sec  ]
*                           (A)  Timestamp  =    Summation     [ (time  - time   )  *  ------------- ]
*                                 (in ms)     i = 1 --> i = N  [      i       i-1       M ticks/sec  ]
*
*
*                                   where
*                                           time        Instantaneous time value (in OS ticks/second)
*                                           M           Number of OS time ticks per second
*
*
*                       (1) However, multiplicative overflow is NOT totally avoided if the product of
*                           the OS time tick delta difference & the constant time scalar (i.e. 1000
*                           milliseconds per second) overflows the integer data type :
*
*                           (A)                 (time_delta * time_scalar)  >=  2^N
*
*                                   where
*                                           time_delta      Calculated time delta difference
*                                           time_scalar     Constant   time scalar (e.g. 1000 ms/1 sec)
*                                           N               Number of data type bits (e.g. 32)
*
*
*                   (b) To ensure timestamp calculation accuracy, timestamp calculations sum timestamp
*                       integer remainders back into total accumulated timestamp :
*
*                                  Total                         [                        1000 ms/sec  ]
*                           (A)  Timestamp  =      Summation     [ (time  - time   )  *  ------------- ]
*                                 (in ms)       i = 1 --> i = N  [      i       i-1       M ticks/sec  ]
*
*                                                                [                                     ]
*                                                  Summation     [ (time  - time   )  *   1000 ms/sec  ]  modulo  (M ticks/sec)
*                                               i = 1 --> i = N  [      i       i-1                    ]
*                                           +  ---------------------------------------------------------------------------------
*
*                                                                                M ticks/sec
*
*
*                                   where
*                                           time        Instantaneous time value (in OS ticks/second)
*                                           M           Number of OS time ticks per second
*
*
*                       (1) However, these calculations are required only when the OS time ticks per
*                           second rate is not an integer multiple of the constant time scalar (i.e.
*                           1000 milliseconds per second).
*********************************************************************************************************
*/
/*$PAGE*/
#if (OS_VERSION >= 300u)
#define  NET_BSP_TICK_RATE_HZ       OS_CFG_TICK_RATE_HZ
#else
#define  NET_BSP_TICK_RATE_HZ       OS_TICKS_PER_SEC
#endif


NET_TS_MS  NetUtil_TS_Get_ms (void)
{
#if (NET_BSP_TICK_RATE_HZ > 0u)
#if (!( (DEF_TIME_NBR_mS_PER_SEC >= NET_BSP_TICK_RATE_HZ) &&     \
       ((DEF_TIME_NBR_mS_PER_SEC %  NET_BSP_TICK_RATE_HZ) == 0u)))
    static  NET_TS_MS    ts_ms_delta_rem_tot = 0u;
            NET_TS_MS    ts_ms_delta_rem_ovf;
            NET_TS_MS    ts_ms_delta_rem;
            NET_TS_MS    ts_ms_delta_num;
#endif
            NET_TS_MS    ts_ms_delta;
    static  CPU_BOOLEAN  ts_active           = DEF_NO;
    static  CPU_INT32U   os_tick_prev        = 0u;
            CPU_INT32U   os_tick_delta;
#endif
            CPU_INT32U   os_tick_cur;
    static  NET_TS_MS    ts_ms_tot           = 0u;
#if (OS_VERSION >= 300u)
            OS_ERR       os_err;
#endif

                                                                        /* Get cur OS time (in ticks).                  */
#if (OS_VERSION >= 300u)
    os_tick_cur = (CPU_INT32U)OSTimeGet(&os_err);
   (void)&os_err;
#else
    os_tick_cur = (CPU_INT32U)OSTimeGet();
#endif

#if (NET_BSP_TICK_RATE_HZ > 0u)
    if (ts_active == DEF_YES) {                                         /* If active, calc & update ts :                */
        os_tick_delta        =  os_tick_cur - os_tick_prev;             /*     Calc time delta (in OS ticks).           */

#if ( (DEF_TIME_NBR_mS_PER_SEC >= NET_BSP_TICK_RATE_HZ) &&    \
     ((DEF_TIME_NBR_mS_PER_SEC %  NET_BSP_TICK_RATE_HZ) == 0u))
                                                                        /*     Calc   ts delta (in ms).                 */
        ts_ms_delta          = (NET_TS_MS)(os_tick_delta  * (DEF_TIME_NBR_mS_PER_SEC / NET_BSP_TICK_RATE_HZ));
        ts_ms_tot           += (NET_TS_MS) ts_ms_delta;                 /*     Update ts tot   (in ms) [see Note #2a].  */

#else
                                                                        /*     Calc   ts delta (in ms) [see Note #2a1]. */
        ts_ms_delta_num      = (NET_TS_MS)(os_tick_delta   * DEF_TIME_NBR_mS_PER_SEC);
        ts_ms_delta          = (NET_TS_MS)(ts_ms_delta_num / NET_BSP_TICK_RATE_HZ);
        ts_ms_tot           += (NET_TS_MS) ts_ms_delta;                 /*     Update ts tot   (in ms) [see Note #2a].  */
                                                                        /*     Calc   ts delta rem ovf (in ms) ...      */
        ts_ms_delta_rem      = (NET_TS_MS)(ts_ms_delta_num % NET_BSP_TICK_RATE_HZ);
        ts_ms_delta_rem_tot +=  ts_ms_delta_rem;
        ts_ms_delta_rem_ovf  =  ts_ms_delta_rem_tot / NET_BSP_TICK_RATE_HZ;
        ts_ms_delta_rem_tot -= (ts_ms_delta_rem_ovf * NET_BSP_TICK_RATE_HZ);
        ts_ms_tot           +=  ts_ms_delta_rem_ovf;                    /* ... & adj  ts tot by ovf    (see Note #2b).  */
#endif

    } else {
        ts_active = DEF_YES;
    }

    os_tick_prev  = os_tick_cur;                                        /* Save cur time for next ts update.            */

#else
    ts_ms_tot += (NET_TS_MS)os_tick_cur;
#endif


    return (ts_ms_tot);
}


/*$PAGE*/
/*
*********************************************************************************************************
*********************************************************************************************************
*                            TRANSMISSION CONTROL PROTOCOL LAYER FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        NetTCP_InitTxSeqNbr()
*
* Description : Initialize the TCP Transmit Initial Sequence Counter, 'NetTCP_TxSeqNbrCtr'.
*
*               (1) Possible initialization methods include :
*
*                   (a) Time-based initialization is one preferred method since it more appropriately
*                       provides a pseudo-random initial sequence number.
*                   (b) Hardware-generated random number initialization is NOT a preferred method since it
*                       tends to produce a discrete set of pseudo-random initial sequence numbers--often
*                       the same initial sequence number.
*                   (c) Hard-coded initial sequence number is NOT a preferred method since it is NOT random.
*
*                   See also 'net_tcp.h  NET_TCP_TX_GET_SEQ_NBR()  Note #1'.
*
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Caller(s)   : NetTCP_Init().
*
*               This function is an INTERNAL network protocol suite function & SHOULD NOT be called by
*               application function(s).
*
* Note(s)     : none.
*********************************************************************************************************
*/

#ifdef  NET_TCP_MODULE_PRESENT
void  NetTCP_InitTxSeqNbr (void)
{
    /* $$$$ Insert code to initialize TCP Transmit Initial Sequence Counter (see Note #1).  */

    NetTCP_TxSeqNbrCtr = NET_TCP_SEQ_NBR_NONE;
}
#endif


/*$PAGE*/
/*
*********************************************************************************************************
*********************************************************************************************************
*                               USER DATAGRAM PROTOCOL LAYER FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                      NetUDP_RxAppDataHandler()
*
* Description : Application-specific UDP connection handler function.
*
* Argument(s) : pbuf        Pointer to network buffer that received UDP datagram.
*               ----        Argument checked in NetUDP_Rx().
*
*               src_addr    Received UDP datagram's source      IP  address.
*
*               src_port    Received UDP datagram's source      UDP port.
*
*               dest_addr   Received UDP datagram's destination IP  address.
*
*               dest_port   Received UDP datagram's destination UDP port.
*
*               perr        Pointer to variable that will receive the return error code from this function
*                               (see Note #1b) :
*
*                               NET_APP_ERR_NONE                UDP datagram successfully received to application
*                                                                   connection(s).
*                               NET_ERR_RX                      Receive error; packet discarded.
*                               NET_ERR_RX_DEST                 Invalid destination; no application connection
*                                                                   available for received packet.
*
* Return(s)   : none.
*
* Caller(s)   : NetUDP_RxPktDemuxAppData().
*$PAGE*
* Note(s)     : (1) Application-specific UDP connection handler function is fully responsible for properly
*                   receiving, demultiplexing, & handling all UDP receive packets to the appropriate
*                   application connections.
*
*                   (a) (1) If the application-specific UDP connection handler function successfully
*                           demultiplexes UDP receive packets to an application connection, the handler
*                           function MUST at some point call NetUDP_RxAppData() to deframe the application
*                           data from the packet buffer(s) into an application array as well as copy any
*                           received IP options into an application IP options buffer.
*
*                           (A) The application-specific connection handler function may service the
*                               application data immediately within the handler function, in which case
*                               the application data SHOULD be serviced as quickly as possible since the
*                               network's global lock remains acquired for the full duration of the
*                               network receive.  Thus, no other network receives or transmits can occur
*                               while the application-specific handler function executes.
*
*                           (B) The application-specific connection handler function may delay servicing
*                               the application data by some other application-specific data servicing
*                               function(s), in which case the servicing function(s) MUST :
*
*                               (1) Acquire the network's global lock PRIOR to calling NetUDP_RxAppData()
*                               (2) Release the network's global lock AFTER    calling NetUDP_RxAppData()
*
*                               See 'net_udp.c  NetUDP_RxAppData()  Note #2'.
*
*                       (2) (A) (1) If NetUDP_RxAppDataHandler()          successfully demultiplexes the UDP
*                                   packets, it SHOULD eventually call NetUDP_RxAppData() to deframe the UDP
*                                   packet data.  If NetUDP_RxAppData() successfully deframes the UDP packet
*                                   application data :
*
*                                   (a) NetUDP_RxAppDataHandler() SHOULD return NET_APP_ERR_NONE, regardless
*                                       of whether the application handled the application data; ...
*                                   (b) but MUST NOT call NetUDP_RxPktFree() to free the UDP packet network
*                                       buffer(s) since NetUDP_RxAppData() frees the network buffer(s) [see
*                                       'NetUDP_RxAppData()  Note #1f'].
*
*                               (2) If NetUDP_RxAppDataHandler() does NOT successfully demultiplex   the UDP
*                                   packets :
*
*                                   (a) NetUDP_RxAppDataHandler() SHOULD return NET_ERR_RX_DEST, ...
*                                   (b) but must NOT free or discard the UDP packet network buffer(s) since
*                                       NetUDP_Rx() discards the network buffer(s).
*
*                               (3) If NetUDP_RxAppDataHandler() or NetUDP_RxAppData() fails for any other
*                                   reason(s) :
*
*                                   (a) NetUDP_RxAppDataHandler() SHOULD return NET_ERR_RX, ...
*                                   (b) but MUST call NetUDP_RxPktDiscard() to discard the UDP packet network
*                                       buffer(s).
*
*                           (B) FAILURE to appropriately call NetUDP_RxAppData() or NetUDP_RxPktDiscard() MAY
*                               result in lost network buffer(s).
*
*                   (b) Application-specific UDP connection handler function MUST return one of the following
*                       error codes ONLY [see 'Argument(s) : perr'] :
*
*                       (1) NET_APP_ERR_NONE  for     received UDP packets     destined to an available
*                               application connection & successfully deframed (see Note #1a2A1a)
*
*                       (2) NET_ERR_RX_DEST   for any received UDP packets NOT destined to an available
*                               application connection                         (see Note #1a2A2a)
*
*                       (3) NET_ERR_RX        for any other receive error      (see Note #1a2A3a)
*********************************************************************************************************
*/

#if ((NET_UDP_CFG_APP_API_SEL == NET_UDP_APP_API_SEL_APP     ) || \
     (NET_UDP_CFG_APP_API_SEL == NET_UDP_APP_API_SEL_SOCK_APP))
void  NetUDP_RxAppDataHandler (NET_BUF           *pbuf,
                               NET_IP_ADDR        src_addr,
                               NET_UDP_PORT_NBR   src_port,
                               NET_IP_ADDR        dest_addr,
                               NET_UDP_PORT_NBR   dest_port,
                               NET_ERR           *perr)
{
    /* $$$$ Insert code to demultiplex & handle UDP packets (see Note #1).  */

   *perr = NET_ERR_RX_DEST;
}
#endif

