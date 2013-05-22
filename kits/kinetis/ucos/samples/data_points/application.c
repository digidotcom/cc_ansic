#include <includes.h>

#include <connector.h>

#define SAWTOOTH_SIGNAL_MAX 	10
#define SAWTOOTH_SIGNAL_MIN 	-10

connector_error_t connector_config(void);

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_TRACE_INFO(("connector_status: status update %d [%s]\n", status, status_message));
}

#if (APP_CFG_CLK == DEF_ENABLED)
CPU_BOOLEAN set_clk(void)
{
    CPU_BOOLEAN ret;
    
    CLK_DATE_TIME date_time;
    CLK_YR Yr;
    CLK_DAY Day;
    CLK_MONTH Month;
    CLK_HR Hr;
    CLK_MIN Min;
    CLK_SEC Sec;
    CLK_TZ_SEC tz_sec;
           
    ret = Clk_GetDateTime(&date_time);
    if (ret == DEF_OK)
    {
        APP_TRACE_INFO(("Current Date/Time:\n"));
        APP_TRACE_INFO(("Date = %u/%u/%u\n", date_time.Month, date_time.Day, date_time.Yr));
        APP_TRACE_INFO(("Time = %u:%u:%u\n", date_time.Hr, date_time.Min, date_time.Sec));
    }
    else
        APP_TRACE_INFO(("Error in Clk_GetDateTime\n"));
 
#if (APP_CFG_SNTP == DEF_ENABLED)
#else
    // TODO: Ask user for time?
    // Set a fix data/time as reference
    Yr = 2013;
    Month = 5;
    Day = 22;
    Hr = 12;
    Min = 23;
    Sec = 0;
    tz_sec = 0;
#endif
    
    ret = Clk_DateTimeMake(&date_time, Yr, Month, Day, Hr, Min, Sec, tz_sec);
    if (ret != DEF_OK)
        APP_TRACE_INFO(("Clk_DateTimeMake Failed\n"));
    ret = Clk_SetDateTime(&date_time);
    if (ret != DEF_OK)
        APP_TRACE_INFO(("Clk_GetDateTime Failed\n"));
    
    ret = Clk_GetDateTime(&date_time);
    if (ret == DEF_OK)
    {
        APP_TRACE_INFO(("New Date/Time:\n"));
        APP_TRACE_INFO(("Date = %u/%u/%u\n", date_time.Month, date_time.Day, date_time.Yr));
        APP_TRACE_INFO(("Time = %u:%u:%u\n", date_time.Hr, date_time.Min, date_time.Sec));
    }
    else
        APP_TRACE_INFO(("Error in Clk_GetDateTime\n"));
    
    return ret;
}
#endif

void fill_data_point(connector_data_point_t *point)
{
	static int signal_value = SAWTOOTH_SIGNAL_MIN;
	static int increasing = 1;
	
	point->description = "Sawtooth Signal";

	point->location.type =  connector_location_type_text;
	point->location.value.text.latitude = "42.27";
	point->location.value.text.longitude = "2.27";
	point->location.value.text.elevation = "391.00";
	
	point->next = NULL;
	point->quality.type = connector_quality_type_ignore;
	{
#if (APP_CFG_CLK == DEF_ENABLED)
        CLK_TS_SEC  ts_unix_sec;
        Clk_GetTS_Unix (&ts_unix_sec);
        point->time.source = connector_time_local_epoch_fractional;
		point->time.value.since_epoch_fractional.seconds = ts_unix_sec;
		point->time.value.since_epoch_fractional.milliseconds = 0;
#endif
	}
	
	point->data.type = connector_data_type_native;
	point->data.element.native.int_value = signal_value;
	
	if (increasing) {
		signal_value++;
		if (signal_value == SAWTOOTH_SIGNAL_MAX)
			increasing = 0;
	} else {
		signal_value--;
		if (signal_value == SAWTOOTH_SIGNAL_MIN)
			increasing = 1;
	}
}

int application_start(void)
{
    OS_ERR err_os = OS_ERR_NONE;
    connector_error_t ret;

    /* Set System Time using SNTPc so samples are uploaded to the cloud with
       correct timestamp.
    */
#if (APP_CFG_CLK == DEF_ENABLED)
    set_clk();  //TODO: maybe move below connector so tcp is ready
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

    // Wait connector connected
#if 0
    do {
        OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err_os);
    } while (get_connection_status() == 0);
#else
    OSTimeDlyHMSM(0, 0, 5, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err_os);
#endif
    
    BSP_LED_Off(BSP_LED_ALL);                       /* Turn off all LEDs.                                   */
    
    {
        static connector_request_data_point_single_t request_data_point = {0};
        static connector_data_point_t data_point = {0};
    	
    	request_data_point.forward_to = NULL;
    	request_data_point.path = "SawtoothSignal";
    	request_data_point.response_required = connector_false;
    	request_data_point.transport = connector_transport_tcp;
    	request_data_point.type = connector_data_point_type_integer;
    	request_data_point.unit = "Volts";
    	request_data_point.user_context = NULL;
    	
    	request_data_point.point = &data_point;
    	
    	for (;;) {
        	fill_data_point(&data_point);
        	APP_TRACE_INFO(("Sending sample %d at %d\n", 
                            request_data_point.point->data.element.native.int_value, 
                            request_data_point.point->time.value.since_epoch_fractional.seconds));
        	connector_send_data_point(&request_data_point);
            // Delay time between samples: 1 Second.
            OSTimeDlyHMSM(0, 0, 1, 0, OS_OPT_TIME_HMSM_STRICT, &err_os);
    	}
    }
    
    return 0;
}