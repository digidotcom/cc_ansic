#include <includes.h>

#include <connector.h>

connector_error_t connector_config(void);

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_TRACE_INFO(("connector_status: status update %d [%s]\n", status, status_message));
}

#if (APP_CFG_CLK == DEF_ENABLED)
#ifndef CLK_TS_NTP
typedef CLK_TS_SEC CLK_TS_NTP;  //Added temporarily
#endif

CPU_INT32U SNTPc_GetLocalTime_s (void)
{
    CLK_TS_NTP ts_ntp;
    Clk_GetTS_NTP(&ts_ntp);
    return ((CPU_INT32U)ts_ntp);
}

CPU_BOOLEAN set_clk(void)
{
    CPU_BOOLEAN ret;    
    CLK_DATE_TIME date_time;
           
    ret = Clk_GetDateTime(&date_time);
    if (ret == DEF_OK)
    {
        APP_TRACE_INFO(("Current Date/Time:\n"));
        APP_TRACE_INFO(("Date = %u/%u/%u  ", date_time.Month, date_time.Day, date_time.Yr));
        APP_TRACE_INFO(("Time = %u:%u:%u\n", date_time.Hr, date_time.Min, date_time.Sec));
    }
    else
        APP_TRACE_INFO(("Error in Clk_GetDateTime\n"));
 
#if (APP_CFG_SNTP == DEF_ENABLED)
    {
        CLK_TS_NTP ntp_time;
        SNTP_PKT sntp_pkt;
        NET_IP_ADDR   App_IP_NTP_Srvr = 0x544D2884;      //Hardcoded SNTP server IP
        
        ret = SNTPc_ReqRemoteTime(App_IP_NTP_Srvr, &sntp_pkt);
        if (ret == DEF_OK) 
        {
            ntp_time = SNTPc_GetRemoteTime_s(&sntp_pkt);
            //Clk_SetTS_NTP(&ntp_time);
            ret = Clk_TS_NTP_ToDateTime(ntp_time, 
                                        CLK_CFG_TZ_DFLT_SEC,    /* TimeZone configured in clk_sfg.h */
                                        &date_time);
            if (ret != DEF_OK)
                APP_TRACE_INFO(("Clk_TS_NTP_ToDateTime Failed\n"));
        }
        else
        {
            APP_TRACE_INFO(("SNTPc_ReqRemoteTime Failed. Date/Time not set\n"));
            return ret;
        }
    }
#else
    {
        CLK_YR Yr;
        CLK_DAY Day;
        CLK_MONTH Month;
        CLK_HR Hr;
        CLK_MIN Min;
        CLK_SEC Sec;
        CLK_TZ_SEC tz_sec;

        // TODO: Ask user for time?
        // Set a fix data/time as reference
        Yr = 2013;
        Month = 5;
        Day = 22;
        Hr = 17;
        Min = 00;
        Sec = 0;
        tz_sec = 7200; /* UTC+2 */

        ret = Clk_DateTimeMake(&date_time, Yr, Month, Day, Hr, Min, Sec, tz_sec);
        if (ret != DEF_OK)
            APP_TRACE_INFO(("Clk_DateTimeMake Failed\n"));
    }
#endif

    ret = Clk_SetDateTime(&date_time);
    if (ret != DEF_OK)
        APP_TRACE_INFO(("Clk_SetDateTime Failed\n"));

    ret = Clk_GetDateTime(&date_time);
    if (ret == DEF_OK)
    {
        APP_TRACE_INFO(("New Date/Time:\n"));
        APP_TRACE_INFO(("Date = %u/%u/%u  ", date_time.Month, date_time.Day, date_time.Yr));
        APP_TRACE_INFO(("Time = %u:%u:%u\n", date_time.Hr, date_time.Min, date_time.Sec));
    }
    else
        APP_TRACE_INFO(("Error in Clk_GetDateTime\n"));
    
    return ret;
}
#endif

int application_start(void)
{
    OS_ERR err_os = OS_ERR_NONE;
    connector_error_t ret;

#if (APP_CFG_CLK == DEF_ENABLED)
    /* Set System Time using SNTPc so files are uploaded to the cloud with
       correct timestamp.
    */
    set_clk();
#endif
    
    APP_TRACE_INFO(("application_start: calling connector_config\n"));
    connector_config();
        
    APP_TRACE_INFO(("application_start: calling connector_start\n"));
    ret = connector_start(connector_status);
    if (ret != connector_error_success)
    {
        APP_TRACE_INFO(("connector_start failed [%d]\n", ret));
        return -1;
    }
    
    BSP_LED_Off(BSP_LED_ALL);                       /* Turn off all LEDs.                                   */
    
    while (DEF_TRUE && (err_os == OS_ERR_NONE)) {   /* Task body, always written as an infinite loop.       */
        BSP_LED_Toggle(2);      
        
        OSTimeDlyHMSM(0, 0, 0, 500, 
                      OS_OPT_TIME_HMSM_STRICT, 
                      &err_os);
    }
    
    return 0;
}