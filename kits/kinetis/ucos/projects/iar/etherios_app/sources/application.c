#include <includes.h>

#include <connector.h>
#include <connector_bsp.h>

connector_error_t connector_config(void);

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_TRACE_INFO(("connector_status: status update %d [%s]\n", status, status_message));
}

int application_start(void)
{
    OS_ERR err_os = OS_ERR_NONE;
    connector_error_t ret;

    APP_TRACE_INFO(("application_start: calling connector_config\n"));
    connector_config();
    
    APP_TRACE_INFO(("application_start: calling connector_start\n"));
    ret = connector_start(connector_status);
    if (ret != connector_error_success)
    {
        APP_TRACE_INFO(("connector_start failed [%d]\n", ret));
        return -1;
    }
      
    Connector_BSP_LED_Off(BSP_LED_ALL);             /* Turn off all LEDs.                                   */
    
    while (DEF_TRUE && (err_os == OS_ERR_NONE)) {   /* Task body, always written as an infinite loop.       */
        Connector_BSP_LED_Toggle(2);      
        
        OSTimeDlyHMSM(0, 0, 0, 500, 
                      OS_OPT_TIME_HMSM_STRICT, 
                      &err_os);
    }
    
    return 0;
}