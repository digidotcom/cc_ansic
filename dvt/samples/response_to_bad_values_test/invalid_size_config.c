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
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ctype.h>
#include "errno.h"

#include "connector_config.h"
#include "connector_api.h"
#include "platform.h"
#include "application.h"

/* iDigi Connector Configuration routines */

#define MAX_INTERFACES      128
#define DEVICE_ID_LENGTH    16
#define VENDOR_ID_LENGTH    4
#define MAC_ADDR_LENGTH     6


/*
 * Routine to get the IP address, you will need to modify this routine for your
 * platform.
 */

connector_callback_status_t app_get_invalid_size_ip_address(void ** ip_address, size_t *size)
{
    static uint8_t invalid_size_ip_address;

    APP_DEBUG("Entering app_get_invalid_size_forced_error_ip_address\n");

    *ip_address = &invalid_size_ip_address;
    *size = sizeof invalid_size_ip_address;

    return connector_callback_continue;
}

/* MAC address used in this sample */
static connector_callback_status_t app_get_invalid_size_mac_addr(uint8_t ** addr, size_t * size)
{
    static uint8_t invalid_size_mac_addr;

    APP_DEBUG("entering app_get_invalid_size_mac_addr\n");

    *addr = &invalid_size_mac_addr;
    *size = sizeof invalid_size_mac_addr;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_device_id(uint8_t ** id, size_t * size)
{
    static uint8_t invalid_size_device_id;

    *id   = &invalid_size_device_id;
    *size = sizeof invalid_size_device_id;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_vendor_id(uint8_t ** id, size_t * size)
{
    static uint8_t invalid_size_vendor_id;

    *id   = &invalid_size_vendor_id;
    *size = sizeof invalid_size_vendor_id;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_device_type(char ** type, size_t * size)
{
    static char invalid_size_device_type;

    *type = &invalid_size_device_type;
    *size = 0;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_size_server_url(char ** url, size_t * size)
{
//#error "Specify iDigi Server URL"

    static char  invalid_size_server_url;

    /* Return pointer to device type. */
    *url = &invalid_size_server_url;
    *size = 0;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_connection_type(connector_connection_type_t ** type)
{

    /* No invalid_size for connection type so return good connection type */
    *type = &device_connection_type;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_link_speed(uint32_t **speed, size_t * size)
{
    static uint8_t invalid_size_link_speed;

    *speed = cast_for_alignment(uint32_t *, &invalid_size_link_speed);
    *size = sizeof invalid_size_link_speed;


    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_phone_number(char ** number, size_t * size)
{
    static char invalid_size_phone_number;

    *number = &invalid_size_phone_number;
    *size = 0;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_tx_keepalive_interval(uint16_t **interval, size_t * size)
{

    static uint8_t invalid_size_tx_keepalive_interval;

    *interval = cast_for_alignment(uint16_t *, &invalid_size_tx_keepalive_interval);
    *size = sizeof invalid_size_tx_keepalive_interval;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_rx_keepalive_interval(uint16_t **interval, size_t * size)
{
    static uint8_t invalid_size_rx_keepalive_interval;

    *interval = cast_for_alignment(uint16_t *, &invalid_size_rx_keepalive_interval);
    *size = sizeof invalid_size_rx_keepalive_interval;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_size_wait_count(uint16_t **count, size_t * size)
{
    static uint8_t invalid_size_wait_count;

    *count = cast_for_alignment(uint16_t *, &invalid_size_wait_count);
    *size = sizeof invalid_size_wait_count;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_firmware_support(connector_service_supported_status_t * is_supported)
{
    /* not invalid size so return valid response */
    *is_supported = connector_service_supported;
    return connector_callback_continue;

}

static connector_callback_status_t app_get_invalid_size_data_service_support(connector_service_supported_status_t * is_supported)
{
    /* not invalid size so return valid response */
    *is_supported = connector_service_supported;
    return connector_callback_continue;
}

static connector_callback_status_t  app_get_invalid_size_file_system_support(connector_service_supported_status_t * is_supported)
{
    /* not invalid size so return valid response */
    *is_supported = connector_service_supported;
    return connector_callback_continue;
}

static connector_callback_status_t  app_get_invalid_size_remote_configuration_support(connector_service_supported_status_t * is_supported)
{
    /* not invalid size so return valid response */
    *is_supported = connector_service_supported;
    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_max_message_transactions(unsigned int * max_transactions)
{
#define CONNECTOR_MAX_MSG_TRANSACTIONS   1
    /* not invalid size so return valid response */
    *max_transactions =  CONNECTOR_MAX_MSG_TRANSACTIONS;
    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_size_device_id_method(connector_device_id_method_t * const method)
{

    /* not invalid size so return valid response */
    *method = device_id_method;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_imei_number(uint8_t ** const imei_number, size_t * size)
{
#define APP_INVALID_SIZE_IMEI_LENGTH   4 /* 8 is right size */

    static uint8_t invalid_size_imei_number[APP_INVALID_SIZE_IMEI_LENGTH];

    *imei_number = invalid_size_imei_number;
    *size = sizeof invalid_size_imei_number;


    return connector_callback_continue;
}

static connector_callback_status_t app_start_network_tcp(connector_auto_connect_type_t * const connect_type)
{
    /* not invalid size so return valid response */
    *connect_type = connector_auto_connect;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_wan_type(connector_wan_type_t * const type)
{

    /* not invalid size so return valid response */
    *type = connector_imei_wan_type;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_size_esn(uint8_t ** const esn_number, size_t * size)
{
#define APP_INVALID_SIZE_ESN_HEX_LENGTH 2

    static uint8_t invalid_size_esn[APP_INVALID_SIZE_ESN_HEX_LENGTH];

    *esn_number = invalid_size_esn;
    *size = sizeof invalid_size_esn;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_meid(uint8_t ** const meid_number, size_t * size)
{
#define APP_INVALID_SIZE_MEID_HEX_LENGTH 4
    static uint8_t invalid_size_meid[APP_INVALID_SIZE_MEID_HEX_LENGTH];

    *meid_number = invalid_size_meid;
    *size = sizeof invalid_size_meid;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_size_identity_verification(connector_identity_verification_t * const identity)
{

    /* not invalid size so return valid response */
    *identity = connector_simple_identity_verification;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_size_password(char const ** password, size_t * const size)
{
    static  char const connector_password[] = "";

    /* Return pointer to password. */
    *password = connector_password;
    *size = sizeof connector_password -1;

    return connector_callback_continue;
}


/* End of iDigi Connector configuration routines */


/*
 * Configuration callback routine.
 */
connector_callback_status_t app_invalid_size_config_handler(connector_config_request_t const request,
                                              void const * const request_data,
                                              size_t const request_length,
                                              void * response_data,
                                              size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_unrecognized;

    UNUSED_ARGUMENT(request_length);

    switch (request)
    {
    case connector_config_device_id:
        APP_DEBUG("app_config_handler:connector_config_device_id\n");
        status = app_get_invalid_size_device_id(response_data, response_length);
        break;

    case connector_config_mac_addr:
        APP_DEBUG("app_config_handler:connector_config_mac_addr\n");
        status = app_get_invalid_size_mac_addr(response_data, response_length);
        break;

    case connector_config_vendor_id:
        APP_DEBUG("app_config_handler:connector_config_vendor_id\n");
        status = app_get_invalid_size_vendor_id(response_data, response_length);
        break;

    case connector_config_device_type:
        APP_DEBUG("app_config_handler:connector_config_vendor_id\n");
        status = app_get_invalid_size_device_type(response_data, response_length);
        break;

    case connector_config_server_url:
        APP_DEBUG("app_config_handler:connector_config_server_url\n");
        status = app_get_invalid_size_server_url(response_data, response_length);
        break;

    case connector_config_connection_type:
        APP_DEBUG("app_config_handler:connector_config_connection_type\n");
        status = app_get_invalid_size_connection_type(response_data);
        break;

    case connector_config_link_speed:
        APP_DEBUG("app_config_handler:connector_config_link_speed\n");
        status = app_get_invalid_size_link_speed(response_data, response_length);
        break;

    case connector_config_phone_number:
        APP_DEBUG("app_config_handler:connector_config_phone_number\n");
        status = app_get_invalid_size_phone_number(response_data, response_length);
        break;

    case connector_config_tx_keepalive:
        APP_DEBUG("app_config_handler:connector_config_tx_keepalive\n");
        status = app_get_invalid_size_tx_keepalive_interval(response_data, response_length);
        break;

    case connector_config_rx_keepalive:
        APP_DEBUG("app_config_handler:connector_config_rx_keepalive\n");
        status = app_get_invalid_size_rx_keepalive_interval(response_data, response_length);
        break;

    case connector_config_wait_count:
        APP_DEBUG("app_config_handler:connector_config_wait_count\n");
        status = app_get_invalid_size_wait_count(response_data, response_length);
        break;

    case connector_config_ip_addr:
        APP_DEBUG("app_config_handler:connector_config_ip_addr\n");
        status = app_get_invalid_size_ip_address(response_data, response_length);
        break;

    case connector_config_error_status:
        APP_DEBUG("app_config_handler:connector_config_error_status\n");
        app_config_error(request_data);
        status = connector_callback_continue;
        break;

    case connector_config_firmware_facility:
        APP_DEBUG("app_config_handler:connector_config_firmware_support\n");
        status = app_get_invalid_size_firmware_support(response_data);
        break;

    case connector_config_data_service:
        APP_DEBUG("app_config_handler:connector_config_data_service_support\n");
        status = app_get_invalid_size_data_service_support(response_data);
        break;

    case connector_config_file_system:
        status = app_get_invalid_size_file_system_support(response_data);
        break;

    case connector_config_remote_configuration:
        status = app_get_invalid_size_remote_configuration_support(response_data);
        break;

    case connector_config_max_transaction:
        APP_DEBUG("app_config_handler:connector_config_max_message_transactions\n");
        status = app_get_invalid_size_max_message_transactions(response_data);
        break;

    case connector_config_device_id_method:
        status = app_get_invalid_size_device_id_method(response_data);
        break;

     case connector_config_network_tcp:
         status = app_start_network_tcp(response_data);
         break;

     case connector_config_imei_number:
         status = app_get_invalid_size_imei_number(response_data, response_length);
         break;

     case connector_config_wan_type:
         status = app_get_invalid_size_wan_type(response_data);
         break;

     case connector_config_esn:
         status = app_get_invalid_size_esn(response_data, response_length);
         break;

     case connector_config_meid:
         status = app_get_invalid_size_meid(response_data, response_length);
         break;

     case connector_config_identity_verification:
         status = app_get_invalid_size_identity_verification(response_data);
         break;

     case connector_config_password:
         status = app_get_invalid_size_password(response_data, response_length);
         break;

    default:
        APP_DEBUG("connector_invalid_size_config_callback: unknown configuration request= %d\n", request);
        break;
    }

    return status;
}

