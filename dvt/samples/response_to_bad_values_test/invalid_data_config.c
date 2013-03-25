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

static connector_callback_status_t app_get_invalid_data_ip_address(uint8_t ** ip_address, size_t *size)
{
    static uint32_t invalid_data_ip_address = 0;
    *ip_address = (uint8_t *)&invalid_data_ip_address;
    *size = sizeof invalid_data_ip_address;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_mac_addr(uint8_t ** addr, size_t * size)
{
//#error "Specify device MAC address for LAN connection"

    APP_DEBUG("Entering app_get_invalid_data_mac_addr\n");

    *addr = NULL;
    *size = MAC_ADDR_LENGTH;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_data_device_id(uint8_t ** id, size_t * size)
{

    APP_DEBUG("Entering app_get_invalid_data_forced_error_device_id \n");

    *id = NULL;
    *size = DEVICE_ID_LENGTH;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_vendor_id(uint8_t ** id, size_t * size)
{
    static uint8_t const invalid_data_vendor_id[VENDOR_ID_LENGTH] = {0x00, 0x00, 0x00, 0x00};
    APP_DEBUG("Entering app_get_invalid_data_forced_error_vendor_id \n");

    *id = (uint8_t *)&invalid_data_vendor_id;
    *size = VENDOR_ID_LENGTH;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_device_type(char ** type, size_t * size)
{
    APP_DEBUG("Entering app_get_invalid_data_forced_error_device_type\n");

    *type = NULL;
    *size = 10;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_server_url(char ** url, size_t * size)
{

    /* Return pointer to device type. */
    *url = (char *)NULL;
    *size = 10;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_data_connection_type(connector_connection_type_t ** type)
{
    /* Return pointer to connection type */
    static connector_connection_type_t  invalid_data_connection_type = (connector_connection_type_t) -1;

    *type = &invalid_data_connection_type;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_data_link_speed(uint32_t **speed, size_t * size)
{
    *speed = NULL;
    *size = 0;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_phone_number(char ** number, size_t * size)
{
    /*
     * Return pointer to phone number for WAN connection type.
     */
    *number = NULL;
    *size = 0;
    return connector_callback_continue;
}

/* Keep alives are from the prospective of the server */
/* This keep alive is sent from the server to the device */
static connector_callback_status_t app_get_invalid_data_tx_keepalive_interval(uint16_t **interval, size_t * size)
{
    UNUSED_ARGUMENT(size);

    /* Return pointer to Tx keepalive interval in seconds */
    *interval = NULL;

    return connector_callback_continue;
}

/* This keep alive is sent from the device to the server  */
static connector_callback_status_t app_get_invalid_data_rx_keepalive_interval(uint16_t **interval, size_t * size)
{
    UNUSED_ARGUMENT(size);

    *interval = NULL;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_data_wait_count(uint16_t **count, size_t * size)
{
    UNUSED_ARGUMENT(size);
    *count = NULL;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_firmware_support(connector_service_supported_status_t * is_supported)
{
    *is_supported = (connector_callback_status_t)-1;
    return connector_callback_continue;

}

static connector_callback_status_t app_get_invalid_data_data_service_support(connector_service_supported_status_t * is_supported)
{
    *is_supported = (connector_callback_status_t)-1;
    return connector_callback_continue;
}

static connector_callback_status_t  app_get_invalid_data_file_system_support(connector_service_supported_status_t * is_supported)
{
    *is_supported = (connector_callback_status_t)-1;
    return connector_callback_continue;
}

static connector_callback_status_t  app_get_invalid_data_remote_configuration_support(connector_service_supported_status_t * is_supported)
{
    *is_supported = (connector_callback_status_t)-1;
    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_max_message_transactions(unsigned int * max_transactions)
{
#define CONNECTOR_MAX_MSG_TRANSACTIONS   256

    *max_transactions =  CONNECTOR_MAX_MSG_TRANSACTIONS;
    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_data_device_id_method(connector_device_id_method_t * const method)
{

    *method = (connector_device_id_method_t)-1;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_imei_number(uint8_t ** const imei_number, size_t * size)
{
#define APP_MEID_HEX_LENGTH 8

    *imei_number = NULL;
    *size = APP_MEID_HEX_LENGTH;

    return connector_callback_continue;
}

static connector_callback_status_t app_start_network_tcp(connector_auto_connect_type_t * const connect_type)
{
    *connect_type = (connector_auto_connect_type_t)-1;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_wan_type(connector_wan_type_t * const type)
{
    *type = (connector_wan_type_t)-1;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_data_esn(uint8_t ** const esn_number, size_t * size)
{
    UNUSED_ARGUMENT(size);

    *esn_number = NULL;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_meid(uint8_t ** const meid_number, size_t * size)
{
    UNUSED_ARGUMENT(size);

    *meid_number = NULL;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_invalid_data_identity_verification(connector_identity_verification_t * const identity)
{

    *identity = (connector_identity_verification_t)-1;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_invalid_data_password(char const ** password, size_t * const size)
{
    *password = NULL;
    *size = 0;

    return connector_callback_continue;
}


/* End of iDigi Connector configuration routines */

/*
 * Configuration callback routine.
 */
connector_callback_status_t app_invalid_data_config_handler(connector_config_request_t const request,
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
        status = app_get_invalid_data_device_id(response_data, response_length);
        break;

    case connector_config_mac_addr:
        APP_DEBUG("app_config_handler:connector_config_mac_addr\n");
        status = app_get_invalid_data_mac_addr(response_data, response_length);
        break;

    case connector_config_vendor_id:
        APP_DEBUG("app_config_handler:connector_config_vendor_id\n");
        status = app_get_invalid_data_vendor_id(response_data, response_length);
        break;

    case connector_config_device_type:
        APP_DEBUG("app_config_handler:connector_config_vendor_id\n");
        status = app_get_invalid_data_device_type(response_data, response_length);
        break;

    case connector_config_server_url:
        APP_DEBUG("app_config_handler:connector_config_server_url\n");
        status = app_get_invalid_data_server_url(response_data, response_length);
        break;

    case connector_config_connection_type:
        APP_DEBUG("app_config_handler:connector_config_connection_type\n");
        status = app_get_invalid_data_connection_type(response_data);
        break;

    case connector_config_link_speed:
        APP_DEBUG("app_config_handler:connector_config_link_speed\n");
        status = app_get_invalid_data_link_speed(response_data, response_length);
        break;

    case connector_config_phone_number:
        APP_DEBUG("app_config_handler:connector_config_phone_number\n");
        status = app_get_invalid_data_phone_number(response_data, response_length);
        break;

    case connector_config_tx_keepalive:
        APP_DEBUG("app_config_handler:connector_config_tx_keepalive\n");
        status = app_get_invalid_data_tx_keepalive_interval(response_data, response_length);
        break;

    case connector_config_rx_keepalive:
        APP_DEBUG("app_config_handler:connector_config_rx_keepalive\n");
        status = app_get_invalid_data_rx_keepalive_interval(response_data, response_length);
        break;

    case connector_config_wait_count:
        APP_DEBUG("app_config_handler:connector_config_wait_count\n");
        status = app_get_invalid_data_wait_count(response_data, response_length);
        break;

    case connector_config_ip_addr:
        APP_DEBUG("app_config_handler:connector_config_ip_addr\n");
        status = app_get_invalid_data_ip_address(response_data, response_length);
        break;

    case connector_config_error_status:
        APP_DEBUG("app_config_handler:connector_config_error_status\n");
        app_config_error(request_data);
        status = connector_callback_continue;
        break;

    case connector_config_firmware_facility:
        APP_DEBUG("app_config_handler:connector_config_firmware_support\n");
        status = app_get_invalid_data_firmware_support(response_data);
        break;

    case connector_config_data_service:
        APP_DEBUG("app_config_handler:connector_config_data_service_support\n");
        status = app_get_invalid_data_data_service_support(response_data);
        break;

    case connector_config_file_system:
        status = app_get_invalid_data_file_system_support(response_data);
        break;

    case connector_config_remote_configuration:
        status = app_get_invalid_data_remote_configuration_support(response_data);
        break;

    case connector_config_max_transaction:
        APP_DEBUG("app_config_handler:connector_config_max_message_transactions\n");
        status = app_get_invalid_data_max_message_transactions(response_data);
        break;

    case connector_config_device_id_method:
        status = app_get_invalid_data_device_id_method(response_data);
        break;

     case connector_config_network_tcp:
         status = app_start_network_tcp(response_data);
         break;

     case connector_config_imei_number:
         status = app_get_invalid_data_imei_number(response_data, response_length);
         break;

     case connector_config_wan_type:
         status = app_get_invalid_data_wan_type(response_data);
         break;

     case connector_config_esn:
         status = app_get_invalid_data_esn(response_data, response_length);
         break;

     case connector_config_meid:
         status = app_get_invalid_data_meid(response_data, response_length);
         break;

     case connector_config_identity_verification:
         status = app_get_invalid_data_identity_verification(response_data);
         break;

     case connector_config_password:
         status = app_get_invalid_data_password(response_data, response_length);
         break;

    default:
        APP_DEBUG("connector_config_callback: unknown configuration request= %d\n", request);
        break;
    }

    return status;
}

