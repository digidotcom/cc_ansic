#include <includes.h>

#include <connector.h>

connector_error_t connector_config(void);

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_TRACE_INFO(("connector_status: status update %d [%s]\n", status, status_message));
}

int application_start(void)
{
    OS_ERR      err_os;
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
      
    BSP_LED_Off(BSP_LED_ALL);                                   /* Turn off all LEDs.                                   */
    
	//Test without this 2->5 downs
    /* Delay 2 Second to let iDigiConnector connect to iDigi */
    OSTimeDlyHMSM(0, 0, 2, 0, OS_OPT_TIME_HMSM_NON_STRICT, &err_os);
    
    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        #define WAIT_FOR_10_MSEC    10
        static connector_dataservice_data_t ecc_data = {0};
        static CPU_BOOLEAN display_push_msg = DEF_TRUE;
        static unsigned char count = 0;
        static char buffer[] = "Device application data. Count xxxx.\n";
        size_t const buf_size = (sizeof buffer) - 1;

        /* Check SW1 PushButton status. A rise edge will trigger a data send to the cloud */
        if (BSP_StatusRd(BSP_PB_START) == DEF_OFF)
        {
            while (BSP_StatusRd(BSP_PB_START) == DEF_OFF)
                OSTimeDlyHMSM(0, 0, 0, WAIT_FOR_10_MSEC, OS_OPT_TIME_HMSM_STRICT, &err_os);

            BSP_LED_On(BSP_LED_ALL);    /* Turn on both led to indicate the request has been sent */
            APP_TRACE_INFO(("Sending data to the Device Cloud using connector_send_data...\n"));
            {
                size_t const bytes_copied = snprintf(buffer, buf_size, "Device application data. Count %d.\n", count);
                
                APP_TRACE_INFO(("%s", buffer));
                ecc_data.transport = connector_transport_tcp;
                ecc_data.data_ptr = buffer;
                ecc_data.length_in_bytes = bytes_copied;
                /* Following function will block until data is sent or timed out */
                // TODO: Create a task for each transmision ?
                ret = connector_send_data("test/test.txt", &ecc_data, NULL);
                display_push_msg = DEF_TRUE;
            }
        }
        else
        {
            if (display_push_msg)
            {
                APP_TRACE_INFO(("\nPush SW1 to send data to the Device Cloud.\n"));
                display_push_msg = DEF_FALSE;
            }

            OSTimeDlyHMSM(0, 0, 0, WAIT_FOR_10_MSEC, OS_OPT_TIME_HMSM_STRICT, &err_os);
            continue;
        }

        if (ret != connector_success)
        {
            APP_TRACE_INFO(("Send failed [%d]\n", ret));
            BSP_LED_Off(BSP_LED_YELLOW);    /* Turn on just orange led to indicate Failure */
        }
        else
        {
            APP_TRACE_INFO(("Send completed\n"));
            BSP_LED_Off(BSP_LED_ORANGE);    /* Turn on just yellow led to indicate Success */
            ecc_data.flags = CONNECTOR_FLAG_APPEND_DATA;;
            count++;
        }
    }
    
    return 0;
}