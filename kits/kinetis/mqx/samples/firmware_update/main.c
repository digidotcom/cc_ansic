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
#include "main.h"
#include "platform.h"
#include "os_support.h"
#include "connector_config.h"

#if !BSPCFG_ENABLE_FLASHX && (defined CONNECTOR_FIRMWARE_SERVICE)
#error This application requires BSPCFG_ENABLE_FLASHX defined non-zero in user_config.h. Please recompile BSP with this option.
#endif

TASK_TEMPLATE_STRUCT MQX_template_list[] =
{ 
/*  Task number, Entry point, Stack, Pri, String, Auto? */
   {MAIN_TASK, Main_task, 2048, 9, "main", MQX_AUTO_START_TASK},
   {CONNECTOR_TASK, connector_thread, CONNECTOR_THREAD_STACK_SIZE, 10, "Cloud Connector", 0},
#ifdef FILE_SYSTEM_SDCARD
   {SDCARD_TASK, sdcard_task,  2048, 11, "SDcard Task", MQX_AUTO_START_TASK},
#endif
#ifdef FILE_SYSTEM_USB
   {USB_TASK, USB_task, 2048, 12, "USB Task", MQX_AUTO_START_TASK},
#endif
#if (defined CONNECTOR_FIRMWARE_SERVICE)
    {FLASH_TASK, flash_task, 4096, 10, "Flash Task", 0},
#endif
   {0,           0,           0,     0,   0,      0,                 }
};


static uint_32 network_start(network_start_params_t *params)
{
    uint8_t * mac_addr = NULL;
    uint_32 result = RTCS_create();

    if (result != RTCS_OK) 
    {
        APP_DEBUG("RTCS failed to initialize, error = %X", result);
        goto error;
    }

    {
		#define MAC_ADDR_LENGTH	6
        size_t size;
        connector_config_pointer_data_t config_data = {NULL, MAC_ADDR_LENGTH};
        
        if (app_get_mac_addr(&config_data) != connector_callback_continue)
        {
            APP_DEBUG("Failed to get device MAC address");
            goto error;
        }

        mac_addr = config_data.data;
        APP_DEBUG("MAC Address: %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X\r\n",
                  (mac_addr)[0], (mac_addr)[1], (mac_addr)[2], (mac_addr)[3], (mac_addr)[4], (mac_addr)[5]);
    }

    result = ipcfg_init_device (ENET_DEVICE, mac_addr);
    if (result != RTCS_OK)
    {
        APP_DEBUG("Failed to initialize Ethernet device, error = %X", result);
        goto error;
    }

    APP_DEBUG("Waiting for Ethernet cable plug in ... ");
    while(!ipcfg_get_link_active(ENET_DEVICE)) {};
    APP_DEBUG("Cable connected\n");

    if (params->ip_type == ip_static) {
		APP_DEBUG("Setting static IP address ... \n");
	    APP_DEBUG("    IP Address      : %d.%d.%d.%d\n",IPBYTES(params->ip_data.ip));
	    APP_DEBUG("    Subnet Address  : %d.%d.%d.%d\n",IPBYTES(params->ip_data.mask));
	    APP_DEBUG("    Gateway Address : %d.%d.%d.%d\n",IPBYTES(params->ip_data.gateway));
	    APP_DEBUG("    DNS Address     : %d.%d.%d.%d\n",IPBYTES(params->dns_server));
	    ipcfg_add_dns_ip(ENET_DEVICE, params->dns_server);
		result = ipcfg_bind_staticip (ENET_DEVICE, &params->ip_data);
    } else {
        APP_DEBUG("Contacting DHCP server ... ");
        result = ipcfg_bind_dhcp_wait(ENET_DEVICE, FALSE, &params->ip_data);    	
    }
    
    if (result != IPCFG_ERROR_OK) 
    {
        APP_DEBUG("\nRTCS failed to bind interface with IPv4, error = %X", result);
        goto error;
    }
    else
    {
        APP_DEBUG("Done\n");
    }

    ipcfg_get_ip(ENET_DEVICE, &params->ip_data);
    APP_DEBUG("\nIP Address      : %d.%d.%d.%d\n",IPBYTES(params->ip_data.ip));
    APP_DEBUG("\nSubnet Address  : %d.%d.%d.%d\n",IPBYTES(params->ip_data.mask));
    APP_DEBUG("\nGateway Address : %d.%d.%d.%d\n",IPBYTES(params->ip_data.gateway));
    APP_DEBUG("\nDNS Address     : %d.%d.%d.%d\n",IPBYTES(ipcfg_get_dns_ip(ENET_DEVICE,0)));
    result = RTCS_OK;
    
error:
    return result;
}

void read_mac(void);
void read_server_url(void);
void read_vendorid(void);
void read_ipconfig(network_start_params_t *netwrok_parameters);

/*TASK*-----------------------------------------------------
* 
* Task Name    : Main_task
* Comments     :
*    This starts Cloud Connector
*
*END*-----------------------------------------------------*/

void Main_task(uint_32 initial_data)
{
    uint_32 result;
    network_start_params_t network_parameters;
    
    /* Read MAC, Vendor ID and server through UART */
    read_mac();
    read_ipconfig(&network_parameters);
    read_vendorid();
    read_server_url();
    result = network_start(&network_parameters);

    if (result == RTCS_OK)
    {
        int const status = application_start();

        if (status != 0)
        {
            APP_DEBUG("application_start failed %d\n", status);
        }
    }

error:
    _task_block();
}

/* EOF */
