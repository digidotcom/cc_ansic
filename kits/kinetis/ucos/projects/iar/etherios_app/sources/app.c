/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*                                               uC/TCP-IP
*
*                             (c) Copyright 2011; Micrium, Inc.; Weston, FL
*                   All rights reserved.  Protected by international copyright laws.
*
*      This medical application example is provided solely as a reference to help engineers use Micriµm
*      and Freescale Semiconductor products.
*
*      There are no express or implied copyright licenses granted hereunder to design or fabricate any
*      medical devices based on the information in this file.
*
*      Micriµm and Freescale Semiconductor make no warranty, representation or guarantee regarding the
*      suitability of this example for any particular purpose, nor does Micriµm and Freescale
*      Semiconductor assume any liability arising out of the application or use of any example design,
*      and specifically disclaims any and all liability, including without limitation consequential or 
*      incidental damages. 
*
*      The following medical application example is not designed, intended, or authorized for use as a
*      component in systems intended to support or sustain life, or for any other application in which
*      the failure of the system could create a situation where personal injury or death may occur.
*
*      You should always consult your physician or other healthcare professionals for specific 
*      advice regarding any medical or health condition.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                           EXAMPLE CODE #1
*                                   Static IP Address Configuration
*
*                                          Freescale Kinetis
*                                               on the
*                                        Freescale TWR-KxxN512
*                                          Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : JPB
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>

#include <connector.h>

/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_IP_ADDR_STR_UNKNOWN                        "xxx.xxx.xxx.xxx"

/*
*********************************************************************************************************
*                                         LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB   AppTaskStartTCB; 
static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];

#if (APP_CFG_DNS == DEF_ENABLED)
static  NET_IP_ADDR  App_IP_DNS_Srvr;
#endif

extern  const  NET_DEV_CFG_ETHER  NetDev_Cfg_KxxN512_0;         /* Ethernet     configuration                           */
extern  const  NET_PHY_CFG_ETHER  NetPhy_Cfg_0;                 /* Ethernet PHY configuration                           */


/*
*********************************************************************************************************
*                                            LOCAL MACRO'S
*********************************************************************************************************
*/

#define  APP_TASK_STOP();                             { while (DEF_ON) { \
                                                            ;            \
                                                        }                \
                                                      }


#define  APP_TEST_FAULT(err_var, err_code)            { if ((err_var) != (err_code)) {   \
                                                            APP_TASK_STOP();             \
                                                        }                                \
                                                      }


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart  (void *p_arg);

static  void  AppTaskCreate (void);

static  void  AppTCPIP_Init (void);

#if (APP_CFG_IP_MODE_DHCP == DEF_ENABLED)
static  void  AppDHCPcInit     (NET_IF_NBR);
#endif

int application_start(void);

void AppGetRunTimeParameters(void);

NET_ERR app_set_mac_addr(NET_IF_NBR if_nbr);

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int  main (void)
{
    OS_ERR  err_os;


    BSP_IntDisAll();                                            /* Disable all interrupts.                              */

    OSInit(&err_os);                                            /* Init uC/OS-III.                                      */

    APP_TEST_FAULT(err_os, OS_ERR_NONE);                        /* Handle the error.                                    */

    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR )AppTaskStart, 
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE)APP_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err_os);

    APP_TEST_FAULT(err_os, OS_ERR_NONE);                        /* Handle the error.                                    */

    OSStart(&err_os);                                           /* Start multitasking (i.e. give control to uC/OS-III). */

    APP_TEST_FAULT(err_os, OS_ERR_NONE);                        /* Handle the error.                                    */
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err_os;

   (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions.                            */
    
    CPU_Init();                                                 /* Initialize the uC/CPU services.                      */

    cpu_clk_freq = BSP_CPU_ClkFreq();                           /* Determine SysTick reference freq.                    */                                                                        
    cnts         = cpu_clk_freq / 
                  (CPU_INT32U)OSCfg_TickRate_Hz;                /* Determine nbr SysTick increments.                    */
    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err_os);                            /* Compute CPU capacity with no task running.           */
#endif

    BSP_Ser_Init(APP_CFG_SERIAL_BAUDRATE);                      /* Initialize serial port.                              */
    
    Mem_Init();                                                 /* Initialize mem mgmt module, necessary for TCP-IP.    */
    
    AppGetRunTimeParameters();
    
    AppTCPIP_Init();                                            /* Initialize uC/TCP-IP and associated appliations.     */
    
#if (APP_CFG_DNS == DEF_ENABLED)
    DNSc_Init(App_IP_DNS_Srvr);                                 /* Initialize DNS  client.                              */
#endif
      
    AppTaskCreate();                                            /* Create application tasks.                            */
    
    APP_TRACE_INFO(("Calling Cloud Connector application_start\n"));
    application_start();    /* Will not return */

    return;
}

static uint8_t device_mac_addr[NET_IF_ETHER_ADDR_SIZE] = {0};

void AppGetRunTimeParameters(void)
{
    CPU_CHAR user_mac[NET_IF_802x_ADDR_SIZE_STR];
    NET_ERR err_net; 
    
#ifdef CONNECTOR_MAC_ADDRESS
    // User defined CONNECTOR_MAC_ADDRESS in connector.h so no need to ask for it
    Str_Copy_N(user_mac,
               CONNECTOR_MAC_ADDRESS,
               NET_IF_802x_ADDR_SIZE_STR);
    APP_TRACE_INFO(("AppGetParameters: Using MAC=%s\n",CONNECTOR_MAC_ADDRESS));
#else
ask_again:
    APP_TRACE_INFO(("AppGetParameters: Type desired MAC address:\n"));
    BSP_Ser_RdStr(user_mac, NET_IF_802x_ADDR_SIZE_STR);
#endif
    
    NetASCII_Str_to_MAC(user_mac,
                   (CPU_INT08U*)&device_mac_addr[0],
                   &err_net);
    if (err_net != NET_ASCII_ERR_NONE) 
    {
        APP_TRACE_INFO(("AppGetParameters: Wrong MAC\n"));
#ifndef CONNECTOR_MAC_ADDRESS
        goto ask_again;
#endif
    }
    
    /* TODO: For clients using prebuild binary image, ask on the serial port for custom parameters like:
                CONNECTOR_MAC_ADDRESS
                CONNECTOR_DEVICE_TYPE
                CONNECTOR_VENDOR_ID
    */  
}

NET_ERR app_set_mac_addr(NET_IF_NBR if_nbr)
{
    NET_IF *pif;
    NET_IF_DATA_802x  *p_if_data;
    NET_ERR err_net; 
        
    pif = NetIF_Get (if_nbr, &err_net);
    if (err_net != NET_IF_ERR_NONE)
        goto done;
             
    p_if_data = (NET_IF_DATA_802x *)pif->IF_Data;
    NET_UTIL_VAL_COPY(&p_if_data->HW_Addr[0],
                        device_mac_addr,
                        NET_IF_802x_ADDR_SIZE);
        
done:
    return err_net;
}

/*
*********************************************************************************************************
*                                      AppTCPIP_Init()
*
* Description : This function is called by AppTaskStart() and is responsible for initializing uC/TCP-IP:
*               - Initialize network protocol suite
*               - Add interface device
*               - Configure IP address
*
* Arguments   : perr    Pointer to variable to store NET_ERR return code.
*
* Returns     : none
********************************************************************************  *************************
*/
static  void  AppTCPIP_Init (void)
{
    NET_IF_NBR   if_nbr;
#if (APP_CFG_IP_MODE_DHCP == DEF_DISABLED)
    NET_IP_ADDR  ip;
    NET_IP_ADDR  msk;
    NET_IP_ADDR  gateway;
    CPU_BOOLEAN  cfg_success;
#endif
    NET_ERR      err_net;
    CPU_CHAR     addr_ip_str[NET_ASCII_LEN_MAX_ADDR_IP];
    
    err_net = Net_Init();                                       /* Initialize uC/TCP-IP.                                */

    APP_TEST_FAULT(err_net, NET_ERR_NONE);                      /* Handle the error.                                    */

                                                                /* Add interface device.                                */
    if_nbr  = NetIF_Add((void    *)&NetIF_API_Ether,            /* Ethernet  interface API.                             */
                        (void    *)&NetDev_API_MACNET,          /* KxxN512   device API.                                */
                        (void    *)&NetDev_BSP_KxxN512,         /* KxxN512   device BSP.                                */
                        (void    *)&NetDev_Cfg_KxxN512_0,       /* KxxN512   device configuration.                      */
                        (void    *)&NetPhy_API_Generic,         /* Generic   Phy API.                                   */
                        (void    *)&NetPhy_Cfg_0,               /* KxxN512   PHY configuration.                         */
                        (NET_ERR *)&err_net);

    APP_TEST_FAULT(err_net, NET_IF_ERR_NONE);                   /* Handle the error.                                    */
    
    app_set_mac_addr(if_nbr);

    APP_TRACE_INFO(("Starting the interface device..."));
    
    NetIF_Start(if_nbr, &err_net);

    APP_TEST_FAULT(err_net, NET_IF_ERR_NONE);
    
    APP_TRACE_INFO(("Ok.\r\n"));

#if (APP_CFG_IP_MODE_DHCP == DEF_ENABLED)
    APP_TRACE_INFO(("Configuring IP Address in DHCP Mode...\r\n"));
    AppDHCPcInit(if_nbr);                                       /* Configure the IP address in DHCP mode.               */
#else
    APP_TRACE_INFO(("Configuring IP Address in Static Mode..."));

                                                                /* Configure IP address in static mode.                 */
    ip      = NetASCII_Str_to_IP((CPU_CHAR *)APP_CFG_IP_ADDRESS_STR, &err_net);
    msk     = NetASCII_Str_to_IP((CPU_CHAR *)APP_CFG_IP_MASK_STR,    &err_net);
    gateway = NetASCII_Str_to_IP((CPU_CHAR *)APP_CFG_IP_GATEWAY_STR, &err_net);
    App_IP_DNS_Srvr = NetASCII_Str_to_IP((CPU_CHAR *)APP_CFG_IP_DNS_SRVR_STR, &err_net);

    cfg_success = NetIP_CfgAddrAdd(if_nbr, ip, msk, gateway, &err_net);

    APP_TEST_FAULT(err_net, NET_IP_ERR_NONE);
    
    if (cfg_success == DEF_OK) {
        APP_TRACE_INFO(("Ok.\r\n"));

        APP_TRACE_INFO(("IP Address: "));
        APP_TRACE_INFO((APP_CFG_IP_ADDRESS_STR));
        APP_TRACE_INFO(("\r\n"));

        APP_TRACE_INFO(("Subnet Mask: "));
        APP_TRACE_INFO((APP_CFG_IP_MASK_STR));
        APP_TRACE_INFO(("\r\n"));

        APP_TRACE_INFO(("Default Gateway: "));
        APP_TRACE_INFO((APP_CFG_IP_GATEWAY_STR));
        APP_TRACE_INFO(("\r\n"));
    } else {
        APP_TRACE_INFO(("Error.\r\n"));
    }
#endif    
    
#if (APP_CFG_DNS == DEF_ENABLED)
    APP_TRACE_INFO(("DNS server "));
    if (App_IP_DNS_Srvr != NET_IP_ADDR_NONE) {
      
        NetASCII_IP_to_Str((NET_IP_ADDR) App_IP_DNS_Srvr,
                           (CPU_CHAR  *) addr_ip_str,
                           (CPU_BOOLEAN) DEF_NO,
                           (NET_ERR   *)&err_net);
        APP_TEST_FAULT(err_net, NET_ASCII_ERR_NONE);
        APP_TRACE_INFO(("= %s\n\r",    &addr_ip_str[0]));
    } else {
        APP_TRACE_INFO(("unavailable\n\r"));
    }
#endif    
}

/*
*********************************************************************************************************
*                                          AppDHCPcInit()
*
* Description : Initialize DHCP client for specified interface.
*
* Argument(s) : if_nbr      Interface number to start DHCP management.
*
* Return(s)   : none.
*
* Caller(s)   : App_TCPIP_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (APP_CFG_IP_MODE_DHCP == DEF_ENABLED)
static  void  AppDHCPcInit (NET_IF_NBR  if_nbr)
{
    DHCPc_OPT_CODE    req_param[DHCPc_CFG_PARAM_REQ_TBL_SIZE];
    CPU_BOOLEAN       cfg_done;
    CPU_BOOLEAN       dly;
    DHCPc_STATUS      dhcp_status;
    NET_IP_ADDRS_QTY  addr_ip_tbl_qty;
    NET_IP_ADDR       addr_ip_tbl[NET_IP_CFG_IF_MAX_NBR_ADDR];
    NET_IP_ADDR       addr_ip;
    CPU_CHAR          addr_ip_str[NET_ASCII_LEN_MAX_ADDR_IP];
    NET_ERR           err_net;
    OS_ERR            err_os;
    DHCPc_ERR         err_dhcp_app;
#if (APP_CFG_DNS == DEF_ENABLED)
    CPU_INT08U        opt_buf[5 * sizeof(NET_IP_ADDR)];
    CPU_INT16U        opt_buf_len;
#endif
                                                                /* Initialize the DHCP client.                          */
    APP_TRACE_INFO(("Initialize DHCP client..."));

    err_dhcp_app = DHCPc_Init();

    APP_TEST_FAULT(err_dhcp_app, DHCPc_ERR_NONE);

    APP_TRACE_INFO(("Ok\r\n"));
                                                                /* Start interface's DHCP client.                       */
    req_param[0] = DHCP_OPT_DOMAIN_NAME_SERVER;

    APP_TRACE_INFO(("Starting DHCP client..."));
    
    DHCPc_Start((NET_IF_NBR      ) if_nbr,
                (DHCPc_OPT_CODE *)&req_param[0],
                (CPU_INT08U      ) 1u,
                (DHCPc_ERR      *)&err_dhcp_app);

    APP_TEST_FAULT(err_dhcp_app, DHCPc_ERR_NONE);

    APP_TRACE_INFO(("Ok\n\r"));
    
    APP_TRACE_INFO(("Processing...\r\n"));

                                                                /* Check interface's DHCP configuration status.         */
    dhcp_status = DHCP_STATUS_NONE;
    cfg_done    = DEF_NO;
    dly         = DEF_NO;

    while (cfg_done != DEF_YES) {
        if (dly == DEF_YES) {
            OSTimeDlyHMSM((CPU_INT16U) 0u,
                          (CPU_INT16U) 0u,
                          (CPU_INT16U) 0u,
                          (CPU_INT16U) 100u,
                          (OS_OPT    ) OS_OPT_TIME_HMSM_STRICT,
                          (OS_ERR   *)&err_os);
        }

        dhcp_status = DHCPc_ChkStatus(if_nbr, &err_dhcp_app);
        switch (dhcp_status) {
            case DHCP_STATUS_CFGD:
                 APP_TRACE_INFO(("DHCP address configured\n\r"));
                 cfg_done = DEF_YES;
                 break;


            case DHCP_STATUS_CFGD_NO_TMR:
                 APP_TRACE_INFO(("DHCP address configured (no timer)\n\r"));
                 cfg_done = DEF_YES;
                 break;


            case DHCP_STATUS_CFGD_LOCAL_LINK:
                 APP_TRACE_INFO(("DHCP address configured (link-local)\n\r"));
                 cfg_done = DEF_YES;
                 break;


            case DHCP_STATUS_FAIL:
                 APP_TRACE_INFO(("DHCP address configuration failed\n\r"));
                 cfg_done = DEF_YES;
                 break;


            case DHCP_STATUS_CFG_IN_PROGRESS:
            default:
                 dly = DEF_YES;
                 break;
        }
    }



                                                                /* Display interface's address.                         */
    if (dhcp_status != DHCP_STATUS_FAIL) {

        addr_ip_tbl_qty = sizeof(addr_ip_tbl) / sizeof(NET_IP_ADDR);

       (void)NetIP_GetAddrHost((NET_IF_NBR        ) if_nbr,
                               (NET_IP_ADDR      *)&addr_ip_tbl[0],
                               (NET_IP_ADDRS_QTY *)&addr_ip_tbl_qty,
                               (NET_ERR          *)&err_net);
        switch (err_net) {
            case NET_IP_ERR_NONE:
                 addr_ip = addr_ip_tbl[0];
                 NetASCII_IP_to_Str((NET_IP_ADDR) addr_ip,
                                    (CPU_CHAR  *) addr_ip_str,
                                    (CPU_BOOLEAN) DEF_NO,
                                    (NET_ERR   *)&err_net);
                 APP_TEST_FAULT(err_net, NET_ASCII_ERR_NONE);
                 break;


            case NET_IF_ERR_INVALID_IF:
            case NET_IP_ERR_NULL_PTR:
            case NET_IP_ERR_ADDR_CFG_IN_PROGRESS:
            case NET_IP_ERR_ADDR_TBL_SIZE:
            case NET_IP_ERR_ADDR_NONE_AVAIL:
            default:
                (void)Str_Copy_N((CPU_CHAR *)&addr_ip_str[0],
                                 (CPU_CHAR *) APP_IP_ADDR_STR_UNKNOWN,
                                 (CPU_SIZE_T) sizeof(addr_ip_str));
                 break;
        }

        APP_TRACE_INFO(("DHCP IP Address = "));
        APP_TRACE_INFO((&addr_ip_str[0]));
        APP_TRACE_INFO(("\n\r"));
      
#if (APP_CFG_DNS == DEF_ENABLED)
                                                                                /* ---- GET IF's DHCPc OPTs/PARAMs ---- */
        opt_buf_len = sizeof(opt_buf);
        DHCPc_GetOptVal((NET_IF_NBR    ) if_nbr,
                        (DHCPc_OPT_CODE) DHCP_OPT_DOMAIN_NAME_SERVER,           /* Get DNS server.                      */
                        (CPU_INT08U   *)&opt_buf[0],
                        (CPU_INT16U   *)&opt_buf_len,
                        (DHCPc_ERR    *)&err_dhcp_app);
        if (err_dhcp_app == DHCPc_ERR_NONE) {
            if (opt_buf_len >= sizeof(addr_ip)) {
                Mem_Copy((void     *)&addr_ip,
                         (void     *)&opt_buf[0],
                         (CPU_SIZE_T) sizeof(addr_ip));
                addr_ip = NET_UTIL_NET_TO_HOST_32(addr_ip);
                if (addr_ip != NET_IP_ADDR_NONE) {
                    App_IP_DNS_Srvr = addr_ip;
                }
            }
        }
#endif
    }
}
#endif

/*
*********************************************************************************************************
*                                        CREATE APPLICATION TASKS
*
* Description:  This function creates the application tasks.
*
* Arguments  :  none
*
* Returns    :  none
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
}
