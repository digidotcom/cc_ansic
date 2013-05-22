/*
 * Copyright (c) 2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

#include "parsing_utils.h"
#include "connector_config.h"
#include "connector.h"


/* Following hook will be called during initialitation if 
   CONNECTOR_VENDOR_ID is not defined in connector_config.h
   It request device_vendor_id through the serial port.
*/
#if !(defined CONNECTOR_VENDOR_ID)
extern uint32_t /*CPU_INT32U*/ device_vendor_id;
#define MAX_VENDOR_ID_STR   10
void app_config_vendor_id(void)
{
    char device_vendor_id_str[MAX_VENDOR_ID_STR+1]; 
    char *device_vendor_id_str_p = device_vendor_id_str;
    uint8_t got_vendor_id = 0;
    
    do {
        _io_printf("app_config_vendor_id: Type desired device_vendor_id in 0xAABBCCDD format:\n");
		get_line(&device_vendor_id_str_p, MAX_VENDOR_ID_STR);
		got_vendor_id = vendor_parse(&device_vendor_id, device_vendor_id_str);
    } while (!got_vendor_id);
    
    _io_printf("\napp_config_vendor_id: device_vendor_id: 0x%x\n", device_vendor_id);
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
	char *connector_cloud_url_p = connector_cloud_url;
	uint8_t got_server_url = 0;
	
	do {
	    _io_printf("app_config_device_cloud_url: Type desired connector_cloud_url:\n");
	    get_line(&connector_cloud_url_p, CLOUD_URL_LENGTH);
		got_server_url = server_url_parse(connector_cloud_url);
	} while (!got_server_url);

    _io_printf("\napp_config_device_cloud_url: connector_cloud_url: %s\n", connector_cloud_url);
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
