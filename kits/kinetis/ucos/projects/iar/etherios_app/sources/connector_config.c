#include <includes.h>

#include <connector.h>

/* Following hook will be called during initialitation if 
   CONNECTOR_VENDOR_ID is not defined in connector_config.h
   It request device_vendor_id through the serial port.
*/
#if !(defined CONNECTOR_VENDOR_ID)
extern uint32_t /*CPU_INT32U*/ device_vendor_id;
#define MAX_VENDOR_ID_STR   10
void app_config_vendor_id(void)
{
    CPU_CHAR device_vendor_id_str[MAX_VENDOR_ID_STR+1]; 
    
 ask_again:
    APP_TRACE_INFO(("app_config_vendor_id: Type desired device_vendor_id in 0xAABBCCDD format:\n"));
    BSP_Ser_RdStr(device_vendor_id_str, MAX_VENDOR_ID_STR);
    
    device_vendor_id = Str_ParseNbr_Int32U(device_vendor_id_str, NULL, DEF_NBR_BASE_HEX);
    if ((device_vendor_id == DEF_INT_32U_MAX_VAL) || (device_vendor_id == 0))
    {
        APP_TRACE_INFO(("app_config_vendor_id: Wrong device_vendor_id\n"));
        goto ask_again;
    }
   
    APP_TRACE_INFO(("\napp_config_vendor_id: device_vendor_id: 0x%x\n", device_vendor_id));
}
#endif

/* Following hook will be called during initialitation if 
   CONNECTOR_CLOUD_URL is not defined in connector_config.h
   It request connector_cloud_url through the serial port.
*/
#if !(defined CONNECTOR_CLOUD_URL)
#if !(defined CLOUD_URL_LENGTH)
#define CLOUD_URL_LENGTH   64
#endif
extern char connector_cloud_url[];
void app_config_device_cloud_url(void)
{
    APP_TRACE_INFO(("app_config_device_cloud_url: Type desired connector_cloud_url:\n"));
    BSP_Ser_RdStr(connector_cloud_url, CLOUD_URL_LENGTH);
    
    APP_TRACE_INFO(("\napp_config_device_cloud_url: connector_cloud_url: %s\n", connector_cloud_url));
}
#endif

connector_error_t connector_config(void)
{    
#if !(defined CONNECTOR_VENDOR_ID)
    app_config_vendor_id();
#endif
#if !(defined CONNECTOR_CLOUD_URL)
    app_config_device_cloud_url();
#endif
    
    return connector_error_success;
}