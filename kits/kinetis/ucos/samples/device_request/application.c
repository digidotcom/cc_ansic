#include <includes.h>

#include <connector.h>
#include <connector_bsp.h>

connector_error_t connector_config(void);

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_TRACE_INFO(("connector_status: status update %d [%s]\n", status, status_message));
}

/*
 * Turn on and off LED outputs
 */
static void set_led_state(CPU_INT08U led, CPU_BOOLEAN state)
{
    if (state)
        Connector_BSP_LED_On(led);
    else
        Connector_BSP_LED_Off(led);
}

static CPU_INT08U get_led_pin(char const * const ledString)
{
    CPU_INT08U led = -1;

    if (!strcmp(ledString, "LED1"))
        led = 1;
    else if (!strcmp(ledString, "LED2"))
        led = 2;
    else
    {
        APP_TRACE_INFO(("Unknown target [%s]\n", ledString));
    }

    return led;
}

static connector_app_error_t device_request_callback(char const * const target, connector_dataservice_data_t * const request_data)
{
    connector_app_error_t status = connector_app_invalid_parameter;
    CPU_INT08U const led = get_led_pin(target);

    if (request_data->error != connector_success)
    {
        APP_TRACE_INFO(("devcie_request_callback: target [%s], error [%d]\n", target, request_data->error));
        goto error;
    }

    if (led != ((CPU_INT08U)-1))
    {
        char const * const stateStr = request_data->data_ptr;
        CPU_BOOLEAN const state = (Str_Str_N(stateStr, "ON", request_data->length_in_bytes) != NULL) ? DEF_TRUE : DEF_FALSE;

        APP_TRACE_INFO(("Turning %s %s\n", state ? "ON" : "OFF", target));
        set_led_state(led, state);
        status = connector_app_success;
    }
    else
    {
    	status = connector_app_unknown_target;
    }
    
    

error:
    return status;
}

static size_t device_response_callback(char const * const target, connector_dataservice_data_t * const response_data)
{
    CPU_INT08U const led = get_led_pin(target);
    char const * const status = (led != NULL) ? "Success" : "Failed";
    size_t bytes_to_copy = strlen(status);

    memcpy(response_data->data_ptr, status, bytes_to_copy);
    response_data->more_data = connector_false;

    APP_TRACE_INFO(("%s action is %s\n", target, status));

    return bytes_to_copy;
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
      
    Connector_BSP_LED_Off(0);    /* Turn off all LEDs. */
    
    APP_TRACE_INFO(("application_start: calling connector_register_device_request_callbacks\n"));
    ret = connector_register_device_request_callbacks(device_request_callback, device_response_callback, NULL, NULL);
    if (ret != connector_success)
    {
        APP_TRACE_INFO(("connector_register_device_request_callbacks failed [%d]\n", ret));
        return -1;
    }
    
    return 0;
}