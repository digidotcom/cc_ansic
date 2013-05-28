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
      
    Connector_BSP_LED_Off(0);                         /* Turn off all LEDs.                                   */
        
    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.       */
        #define WAIT_FOR_10_MSEC    10
        static connector_dataservice_data_t ecc_data = {0};
        static CPU_BOOLEAN display_push_msg = DEF_TRUE;
        static unsigned char count = 0;
        static char buffer[] = "Device application data. Count xxxx.\n";
        size_t const buf_size = (sizeof buffer) - 1;

        /* Check SW1 PushButton status. A rise edge will trigger a data send to the cloud */
        if (Connector_BSP_Status_Read(0) == DEF_OFF)
        {
            while (Connector_BSP_Status_Read(0) == DEF_OFF)
                app_os_delay(WAIT_FOR_10_MSEC);

            Connector_BSP_LED_On(0);    /* Turn on both led to indicate the request has been sent */
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

            app_os_delay(WAIT_FOR_10_MSEC);
            continue;
        }

        if (ret != connector_error_success)
        {
            APP_TRACE_INFO(("Send failed [%d]\n", ret));
            Connector_BSP_LED_Off(2);    /* Turn on just led1 to indicate Failure */
        }
        else
        {
            APP_TRACE_INFO(("Send completed\n"));
            Connector_BSP_LED_Off(1);    /* Turn on just led2 to indicate Success */
            ecc_data.flags = CONNECTOR_FLAG_APPEND_DATA;;
            count++;
        }
    }
    
    return 0;
}