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

char python_file_buffer[256];
static char filename[64];

/*
 * Routine to get the IP address, you will need to modify this routine for your
 * platform.
 */
static connector_callback_status_t app_get_ip_address(uint8_t ** ip_address, size_t *size)
{
    int             fd = -1;
    connector_callback_status_t  status= connector_callback_abort;
    char            *buf = malloc(MAX_INTERFACES*sizeof(struct ifreq));
    struct ifconf   conf;
    unsigned int    entries = 0;
    unsigned int    i;
    static struct in_addr  ip_addr; /* Used to store the IP address */

/* Remove this //#error statement once you modify this routine to return the correct IP address */
//#error "Specify device IP address. Set size to 4 (bytes) for IPv4 or 16 (bytes) for IPv6"


    APP_DEBUG("Entering app_get_ip_address\n");

    if (buf == NULL)
    {
        APP_DEBUG("get_ip_address malloc failed\n");
        goto error;
    }
    conf.ifc_len = MAX_INTERFACES*sizeof(struct ifreq);
    conf.ifc_buf = buf;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1)
    {
        perror("socket");
        goto error;
    }

    if( ioctl(fd, SIOCGIFCONF , &conf) == -1)
    {
        APP_DEBUG("get_ip_address: Error using ioctl SIOCGIFCONF.\n");
        goto error;
    }

    entries = conf.ifc_len / sizeof(struct ifreq);

    APP_DEBUG("get_ip_address: Looking for current device IP address: found [%d] entries\n", entries);

    if (entries == 0)
    {
        goto error;
    }
    for( i = 0; i < entries; i++)
    {
        struct ifreq * req = &conf.ifc_req[i];
        struct sockaddr_in * sa = cast_for_alignment(struct sockaddr_in *, &req->ifr_addr);

        APP_DEBUG("get_ip_address: %d: Interface name [%s]\tIP Address [%s]\n", i+1, req->ifr_name, inet_ntoa(sa->sin_addr));
        if (sa->sin_addr.s_addr != htonl(INADDR_LOOPBACK))
        {
            ip_addr = sa->sin_addr;
            break;
        }
    }

    /* Fill in the size and IP address */
    *size       = sizeof ip_addr.s_addr;

    *ip_address = (uint8_t *)&ip_addr.s_addr;
    status = connector_callback_continue;

error:
    if (fd != -1)
    {
        close(fd);
    }
    if (buf != NULL)
    {
        free(buf);
    }
    return status;
}

/* MAC address used in this sample */
static uint8_t device_mac_addr[MAC_ADDR_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static connector_callback_status_t app_get_mac_addr(uint8_t ** addr, size_t * size)
{
//#error "Specify device MAC address for LAN connection"

    APP_DEBUG("entering app_get_mac_addr\n");

    *addr = device_mac_addr;
    *size = sizeof device_mac_addr;

    return connector_callback_continue;
}

static uint8_t device_id[DEVICE_ID_LENGTH] = {0};

static connector_callback_status_t app_get_device_id(uint8_t ** id, size_t * size)
{
    uint8_t * mac_addr;
    size_t mac_size;

//#error  "Specify device id"

    APP_DEBUG("Entering app_get_device_id\n");

    /* This sample uses the MAC address to format the device ID */
    app_get_mac_addr(&mac_addr, &mac_size);

    device_id[8] = device_mac_addr[0];
    device_id[9] = device_mac_addr[1];
    device_id[10] = device_mac_addr[2];
    device_id[11] = 0xFF;
    device_id[12] = 0xFF;
    device_id[13] = device_mac_addr[3];
    device_id[14] = device_mac_addr[4];
    device_id[15] = device_mac_addr[5];

    *id   = device_id;
    *size = sizeof device_id;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_vendor_id(uint8_t ** id, size_t * size)
{
//#error  "Specify vendor id"
    static const uint8_t device_vendor_id[VENDOR_ID_LENGTH] = {0x00, 0x00, 0x00, 0x00};
    *id = (uint8_t *)device_vendor_id;
    *size = sizeof device_vendor_id;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_device_type(char ** type, size_t * size)
{
//#error "Specify device type"
    static char const device_type[] = "Linux Application";

    /* Return pointer to device type. */
    *type = (char *)device_type;
    *size = sizeof device_type -1;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_server_url(char ** url, size_t * size)
{
//#error "Specify iDigi Server URL"

    static char const connector_server_url[] = "my.idigi.com";

    /* Return pointer to device type. */
    *url = (char *)connector_server_url;
    *size = sizeof connector_server_url -1;

    return connector_callback_continue;
}

connector_connection_type_t  device_connection_type = connector_wan_connection_type;

static connector_callback_status_t app_get_connection_type(connector_connection_type_t ** type)
{
//#error "Specify LAN or WAN connection type"

    /* Return pointer to connection type */

    *type = &device_connection_type;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_link_speed(uint32_t **speed, size_t * size)
{
//#error "Specify link speed for WAN connection type"
    static uint32_t const link_speed = 0;
    *speed = (uint32_t *)&link_speed;
    *size = sizeof link_speed;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_phone_number(char ** number, size_t * size)
{
//#error "Specify phone number dialed for WAN connection type"
    /*
     * Return pointer to phone number for WAN connection type.
     */
    static char const phone_number[] = "32075";
    *number = (char *)phone_number;
    *size = sizeof phone_number -1;

    return connector_callback_continue;
}

/* Keep alives are from the prospective of the server */
/* This keep alive is sent from the server to the device */
static connector_callback_status_t app_get_tx_keepalive_interval(uint16_t **interval, size_t * size)
{
//#error "Specify server to device TX keepalive interval in seconds"

#define DEVICE_TX_KEEPALIVE_INTERVAL_IN_SECONDS     90
    /* Return pointer to Tx keepalive interval in seconds */
    static uint16_t device_tx_keepalive_interval = DEVICE_TX_KEEPALIVE_INTERVAL_IN_SECONDS;
    *interval = &device_tx_keepalive_interval;
    *size = sizeof device_tx_keepalive_interval;

    return connector_callback_continue;
}


/* This keep alive is sent from the device to the server  */
static connector_callback_status_t app_get_rx_keepalive_interval(uint16_t **interval, size_t * size)
{
//#error "Specify server to device RX keepalive interval in seconds"
#define DEVICE_RX_KEEPALIVE_INTERVAL_IN_SECONDS     60
    /* Return pointer to Rx keepalive interval in seconds */
    static uint16_t device_rx_keepalive_interval = DEVICE_RX_KEEPALIVE_INTERVAL_IN_SECONDS;
    *interval = &device_rx_keepalive_interval;
    *size = sizeof device_rx_keepalive_interval;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_wait_count(uint16_t **count, size_t * size)
{
//#error "Specify the number of times that not receiving keepalive messages from server is allowed"
#define DEVICE_WAIT_COUNT     5
    /*
     * Return pointer to wait count (number of times not receiving Tx keepalive
     * from server is allowed).
     */
    static uint16_t device_wait_count = DEVICE_WAIT_COUNT;
    *count = &device_wait_count;
    *size = sizeof device_wait_count;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_firmware_support(connector_service_supported_status_t * is_supported)
{
    *is_supported = connector_service_supported;
    return connector_callback_continue;

}

static connector_callback_status_t app_get_data_service_support(connector_service_supported_status_t * is_supported)
{
    *is_supported = connector_service_supported;
    return connector_callback_continue;
}

static connector_callback_status_t  app_get_file_system_support(connector_service_supported_status_t * is_supported)
{
    *is_supported = connector_service_supported;
    return connector_callback_continue;
}

static connector_callback_status_t  app_get_remote_configuration_support(connector_service_supported_status_t * is_supported)
{
    *is_supported = connector_service_supported;
    return connector_callback_continue;
}

static connector_callback_status_t app_get_max_message_transactions(unsigned int * max_transactions)
{
#define CONNECTOR_MAX_MSG_TRANSACTIONS   1

    *max_transactions =  CONNECTOR_MAX_MSG_TRANSACTIONS;
    return connector_callback_continue;
}

connector_device_id_method_t device_id_method = connector_auto_device_id_method;

static connector_callback_status_t app_get_device_id_method(connector_device_id_method_t * const method)
{

    *method = device_id_method;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_imei_number(uint8_t ** const imei_number, size_t * size)
{
#define APP_IMEI_LENGTH 8

//#error "Specify the IMEI number for WAN connection type if app_get_device_id_method returns connector_auto_device_id_method and app_get_device_id_method returns connector_imei_wan_type"
    /* Each nibble corresponds a decimal digit.
     * Most upper nibble must be 0.
     */
    static char const app_imei_number_string[] = "000000-00-000000-0";
    static uint8_t app_imei_number[APP_IMEI_LENGTH] = {0};
    int i = sizeof app_imei_number_string -1;
    int index = sizeof app_imei_number -1;

    while (i > 0)
    {
        int n = 0;

        app_imei_number[index] = 0;

        /* loop 2 times here for 2 digits (1 bytes) */
        while (n < 2 && i > 0)
        {
            i--;
            if (app_imei_number_string[i] != '-')
            {
                ASSERT(isdigit(app_imei_number_string[i]));
                app_imei_number[index] += ((app_imei_number_string[i] - '0') << (n * 4));
                n++;
            }
        }
        index--;
    }

    *imei_number = app_imei_number;
    *size = sizeof app_imei_number;

    APP_DEBUG("app_get_imei_number\n");

    return connector_callback_continue;
}

connector_auto_connect_type_t connector_connect_tcp = connector_manual_connect;

static connector_callback_status_t app_start_network_tcp(connector_auto_connect_type_t * const connect_type)
{
    *connect_type = connector_connect_tcp;

    return connector_callback_continue;
}

static connector_callback_status_t app_start_network_udp(connector_auto_connect_type_t * const connect_type)
{
    *connect_type = connector_auto_connect;

    return connector_callback_continue;
}

static connector_callback_status_t app_start_network_sms(connector_auto_connect_type_t * const connect_type)
{
    *connect_type = connector_auto_connect;

    return connector_callback_continue;
}

connector_wan_type_t connector_wan_type = connector_imei_wan_type;

static connector_callback_status_t app_get_wan_type(connector_wan_type_t * const type)
{
//#error "Specify connector_imei_wan_type for IMEI, connector_esn_wan_type for ESN, or connector_meid_wan_type for MEID WAN type"

    *type = connector_wan_type;

    return connector_callback_continue;
}


static connector_callback_status_t app_get_esn(uint8_t ** const esn_number, size_t * size)
{
#define APP_ESN_HEX_LENGTH 4
#define APP_ESN_HEX_STRING_LENGTH 8

//#error "Specify the ESN number for WAN connection type if app_get_device_id_method returns connector_auto_device_id_method and app_get_device_id_method returns connector_esn_wan_type."
    /* Each nibble corresponds a decimal digit.
     * Most upper nibble must be 0.
     */
    static char const app_esn_hex_string[APP_ESN_HEX_STRING_LENGTH] = "00000000";
    static uint8_t app_esn_hex[APP_ESN_HEX_LENGTH] = {0};
    int i = sizeof app_esn_hex_string -1;
    int index = sizeof app_esn_hex -1;

    while (i > 0)
    {
        int n = 0;

        app_esn_hex[index] = 0;

        /* loop 2 times here for 2 digits (1 bytes) */
        while (n < 2 && i > 0)
        {
            i--;
            if (app_esn_hex_string[i] != '-')
            {
                ASSERT(isdigit(app_esn_hex_string[i]));
                app_esn_hex[index] += ((app_esn_hex_string[i] - '0') << (n * 4));
                n++;
            }
        }
        index--;
    }

    *esn_number = app_esn_hex;
    *size = sizeof app_esn_hex;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_meid(uint8_t ** const meid_number, size_t * size)
{
#define APP_MEID_HEX_LENGTH 8
#define APP_MEID_HEX_STRING_LENGTH 15

//#error "Specify the MEID number for WAN connection type if app_get_device_id_method returns connector_auto_device_id_method and app_get_device_id_method returns connector_meid_wan_type."
    /* Each nibble corresponds a decimal digit.
     * Most upper nibble must be 0.
     */
    static char const app_meid_hex_string[APP_MEID_HEX_STRING_LENGTH] = "000000000000000";
    static uint8_t app_meid_hex[APP_MEID_HEX_LENGTH] = {0};
    int i = sizeof app_meid_hex_string -1;
    int index = sizeof app_meid_hex -1;

    while (i > 0)
    {
        int n = 0;

        app_meid_hex[index] = 0;

        /* loop 2 times here for 2 digits (1 bytes) */
        while (n < 2 && i > 0)
        {
            i--;
            if (app_meid_hex_string[i] != '-')
            {
                ASSERT(isdigit(app_meid_hex_string[i]));
                app_meid_hex[index] += ((app_meid_hex_string[i] - '0') << (n * 4));
                n++;
            }
        }
        index--;
    }

    *meid_number = app_meid_hex;
    *size = sizeof app_meid_hex;

    return connector_callback_continue;
}

connector_identity_verification_t connector_identity_verification = connector_simple_identity_verification;

static connector_callback_status_t app_get_identity_verification(connector_identity_verification_t * const identity)
{
//#error "Specify connector_identity_verification for simple or password identify verification form"

    *identity = connector_identity_verification;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_password(char const ** password, size_t * const size)
{
//#error "Specify password for password identity verification form"
    static  char const connector_password[] = "";

    /* Return pointer to password. */
    *password = connector_password;
    *size = sizeof connector_password -1;

    return connector_callback_continue;
}

static connector_callback_status_t app_get_sms_service_id(char const ** const service_id, size_t * const size)
{
//#error "Specify SMS service id. It is optional, set *service_id to NULL and *size to 0 if not used"
    static  char const sms_service_id[] = "IDGP";

    /* Return pointer to service_id. */
    *service_id = sms_service_id;
    *size = sizeof sms_service_id -1;

    return connector_callback_continue;
}

/* End of iDigi Connector configuration routines */

/*
 * This routine is called when a configuration error is encountered by the iDigi Connector.
 * This is currently used as a debug tool for finding configuration errors.
 */
#define enum_to_case(name)  case name:  result = #name;             break

static char const * app_class_to_string(connector_class_id_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_class_config);
        enum_to_case(connector_class_operating_system);
        enum_to_case(connector_class_firmware);
        enum_to_case(connector_class_id_data_service);
        enum_to_case(connector_class_remote_config_service);
        enum_to_case(connector_class_id_file_system);
        enum_to_case(connector_class_network_tcp);
        enum_to_case(connector_class_id_network_udp);
        enum_to_case(connector_class_id_network_sms);
        enum_to_case(connector_class_id_status);
        enum_to_case(connector_class_id_short_message);
    }
    return result;
}

static char const * app_config_class_to_string(connector_config_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_config_device_id);
        enum_to_case(connector_config_vendor_id);
        enum_to_case(connector_config_device_type);
        enum_to_case(connector_config_server_url);
        enum_to_case(connector_config_connection_type);
        enum_to_case(connector_config_mac_addr);
        enum_to_case(connector_config_link_speed);
        enum_to_case(connector_config_phone_number);
        enum_to_case(connector_config_tx_keepalive);
        enum_to_case(connector_config_rx_keepalive);
        enum_to_case(connector_config_wait_count);
        enum_to_case(connector_config_ip_addr);
        enum_to_case(connector_config_error_status);
        enum_to_case(connector_config_firmware_facility);
        enum_to_case(connector_config_data_service);
        enum_to_case(connector_config_file_system);
        enum_to_case(connector_config_remote_configuration);
        enum_to_case(connector_config_max_transaction);
        enum_to_case(connector_config_device_id_method);
        enum_to_case(connector_config_imei_number);
        enum_to_case(connector_config_network_tcp);
        enum_to_case(connector_config_network_udp);
        enum_to_case(connector_config_network_sms);
        enum_to_case(connector_config_wan_type);
        enum_to_case(connector_config_esn);
        enum_to_case(connector_config_meid);
        enum_to_case(connector_config_identity_verification);
        enum_to_case(connector_config_password);
        enum_to_case(connector_config_sms_service_id);
    }
    return result;
}

static char const * app_network_class_to_string(connector_network_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_network_open);
        enum_to_case(connector_network_send);
        enum_to_case(connector_network_receive);
        enum_to_case(connector_network_close);
    }
    return result;
}

static char const * app_os_class_to_string(connector_os_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_os_malloc);
        enum_to_case(connector_os_free);
        enum_to_case(connector_os_system_up_time);
        enum_to_case(connector_os_yield);
        enum_to_case(connector_os_reboot);
    }
    return result;
}

static char const * app_firmware_class_to_string(connector_firmware_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_firmware_target_count);
        enum_to_case(connector_firmware_version);
        enum_to_case(connector_firmware_code_size);
        enum_to_case(connector_firmware_description);
        enum_to_case(connector_firmware_name_spec);
        enum_to_case(connector_firmware_download_request);
        enum_to_case(connector_firmware_binary_block);
        enum_to_case(connector_firmware_download_complete);
        enum_to_case(connector_firmware_download_abort);
        enum_to_case(connector_firmware_target_reset);
    }
    return result;
}

static char const * app_remote_config_class_to_string(connector_remote_config_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_remote_config_session_start);
        enum_to_case(connector_remote_config_action_start);
        enum_to_case(connector_remote_config_group_start);
        enum_to_case(connector_remote_config_group_process);
        enum_to_case(connector_remote_config_group_end);
        enum_to_case(connector_remote_config_action_end);
        enum_to_case(connector_remote_config_session_end);
        enum_to_case(connector_remote_config_session_cancel);
    }
    return result;
}

static char const * app_file_system_class_to_string(connector_file_system_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_file_system_open);
        enum_to_case(connector_file_system_read);
        enum_to_case(connector_file_system_write);
        enum_to_case(connector_file_system_lseek);
        enum_to_case(connector_file_system_ftruncate);
        enum_to_case(connector_file_system_close);
        enum_to_case(connector_file_system_rm);
        enum_to_case(connector_file_system_stat);
        enum_to_case(connector_file_system_opendir);
        enum_to_case(connector_file_system_readdir);
        enum_to_case(connector_file_system_closedir);
        enum_to_case(connector_file_system_strerror);
        enum_to_case(connector_file_system_msg_error);
        enum_to_case(connector_file_system_hash);
    }
    return result;
}

static char const * app_data_service_class_to_string(connector_data_service_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_data_service_put_request);
        enum_to_case(connector_data_service_device_request);
        enum_to_case(connector_data_service_dp_response);
    }
    return result;
}

static char const * app_status_class_to_string(connector_status_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_status_ping_response);
        enum_to_case(connector_status_ping_request);
        enum_to_case(connector_status_tcp);
        enum_to_case(connector_status_stop_completed);
}
    return result;
}

static char const * app_sm_class_to_string(connector_sm_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_sm_cli_request);
        enum_to_case(connector_sm_server_to_device_config);
        enum_to_case(connector_sm_device_to_server_config);
        enum_to_case(connector_sm_more_data);
        enum_to_case(connector_sm_opaque_response);
    }
    return result;
}

char const * app_status_error_to_string(connector_status_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_success);
        enum_to_case(connector_init_error);
        enum_to_case(connector_abort);
        enum_to_case(connector_invalid_data_size);
        enum_to_case(connector_invalid_data_range);
        enum_to_case(connector_keepalive_error);
        enum_to_case(connector_invalid_data);
        enum_to_case(connector_device_terminated);
        enum_to_case(connector_service_busy);
        enum_to_case(connector_invalid_response);
        enum_to_case(connector_no_resource);
        enum_to_case(connector_unavailable);
        enum_to_case(connector_idle);
        enum_to_case(connector_working);
        enum_to_case(connector_pending);
        enum_to_case(connector_active);
        enum_to_case(connector_device_error);
        enum_to_case(connector_open_error);

        enum_to_case(connector_invalid_payload_packet);
        enum_to_case(connector_bad_version);
        enum_to_case(connector_exceed_timeout);
    }
    return result;
}

/*
 * This routine is called when a configuration error is encountered by the iDigi connector.
 * This is currently used as a debug tool for finding configuration errors.
 */
void app_config_error(connector_error_status_t const * const error_data)
{
    char const * request_name = NULL;
    int request_id = 0;

    switch (error_data->class_id)
    {
    case connector_class_config:
        request_name = app_config_class_to_string(error_data->request_id.config_request);
        request_id = error_data->request_id.config_request;
        break;
    case connector_class_network_tcp:
    case connector_class_id_network_udp:
    case connector_class_id_network_sms:
        request_name = app_network_class_to_string(error_data->request_id.network_request);
        request_id = error_data->request_id.network_request;
        break;
    case connector_class_operating_system:
        request_name = app_os_class_to_string(error_data->request_id.os_request);
        request_id = error_data->request_id.os_request;
        break;
    case connector_class_firmware:
        request_name = app_firmware_class_to_string(error_data->request_id.firmware_request);
        request_id = error_data->request_id.firmware_request;
        break;
    case connector_class_id_data_service:
        request_name = app_data_service_class_to_string(error_data->request_id.data_service_request);
        request_id = error_data->request_id.data_service_request;
        break;
    case connector_class_id_file_system:
        request_name = app_file_system_class_to_string(error_data->request_id.file_system_request);
        request_id = error_data->request_id.file_system_request;
        break;
    case connector_class_remote_config_service:
        request_name = app_remote_config_class_to_string(error_data->request_id.remote_config_request);
        request_id = error_data->request_id.remote_config_request;
           break;
    case connector_class_id_status:
        request_name = app_status_class_to_string(error_data->request_id.status_request);
        request_id = error_data->request_id.status_request;
        break;
    case connector_class_id_short_message:
        request_name = app_sm_class_to_string(error_data->request_id.sm_request);
        request_id = error_data->request_id.sm_request;
        break;
    }

    APP_DEBUG("app_config_error: Class: %s (%d) Request: %s (%d) Error Status: %s (%d)\n",
                                 app_class_to_string(error_data->class_id), error_data->class_id,
                                 request_name, request_id,
                                 app_status_error_to_string(error_data->status), error_data->status);

    snprintf(python_file_buffer, sizeof(python_file_buffer), "%s,%s,", request_name, app_status_error_to_string(error_data->status));
    write_python_result_file(python_file_buffer);
}

/*
 * Configuration callback routine.
 */
connector_callback_status_t app_config_handler(connector_config_request_t const request,
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
        status = app_get_device_id(response_data, response_length);
        break;

    case connector_config_mac_addr:
        status = app_get_mac_addr(response_data, response_length);
        break;

    case connector_config_vendor_id:
        status = app_get_vendor_id(response_data, response_length);
        break;

    case connector_config_device_type:
        status = app_get_device_type(response_data, response_length);
        break;

    case connector_config_server_url:
        status = app_get_server_url(response_data, response_length);
        break;

    case connector_config_connection_type:
        status = app_get_connection_type(response_data);
        break;

    case connector_config_link_speed:
        status = app_get_link_speed(response_data, response_length);
        break;

    case connector_config_phone_number:
        status = app_get_phone_number(response_data, response_length);
        break;

    case connector_config_tx_keepalive:
        status = app_get_tx_keepalive_interval(response_data, response_length);
        break;

    case connector_config_rx_keepalive:
        status = app_get_rx_keepalive_interval(response_data, response_length);
        break;

    case connector_config_wait_count:
        status = app_get_wait_count(response_data, response_length);
        break;

    case connector_config_ip_addr:
        status = app_get_ip_address(response_data, response_length);
        break;

    case connector_config_error_status:
        app_config_error(request_data);
        status = connector_callback_continue;
        break;

    case connector_config_firmware_facility:
        status = app_get_firmware_support(response_data);
        break;

    case connector_config_data_service:
        status = app_get_data_service_support(response_data);
        break;

    case connector_config_file_system:
        status = app_get_file_system_support(response_data);
        break;

    case connector_config_remote_configuration:
        status = app_get_remote_configuration_support(response_data);
        break;

    case connector_config_max_transaction:
        status = app_get_max_message_transactions(response_data);
        break;

    case connector_config_device_id_method:
        status = app_get_device_id_method(response_data);
        break;

     case connector_config_network_tcp:
         status = app_start_network_tcp(response_data);
         break;

     case connector_config_network_udp:
         status = app_start_network_udp(response_data);
         break;

     case connector_config_network_sms:
         status = app_start_network_sms(response_data);
         break;

     case connector_config_imei_number:
         status = app_get_imei_number(response_data, response_length);
         break;

     case connector_config_wan_type:
         status = app_get_wan_type(response_data);
         break;

     case connector_config_esn:
         status = app_get_esn(response_data, response_length);
         break;

     case connector_config_meid:
         status = app_get_meid(response_data, response_length);
         break;

     case connector_config_identity_verification:
         status = app_get_identity_verification(response_data);
         break;

     case connector_config_password:
         status = app_get_password(response_data, response_length);
         break;

     case connector_config_sms_service_id:
         status = app_get_sms_service_id(response_data, response_length);
         break;

    default:
        APP_DEBUG("connector_config_callback: unknown configuration request= %d\n", request);
        break;
    }

    return status;
}

void write_python_result_file(char *file_buffer)
{
    FILE * fp = NULL;
    uint8_t * id;
    size_t size;

    app_get_device_id(&id, &size);
    snprintf(filename, sizeof(filename), "%02X%02X%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X-%02X%02X%02X%02X.txt",
             device_id[0], device_id[1], device_id[2], device_id[3], device_id[4], device_id[5], device_id[6], device_id[7],
             device_id[8], device_id[9], device_id[10], device_id[11], device_id[12], device_id[13], device_id[14], device_id[15]);
    fp = fopen(filename, "a");

    if (fp == NULL)
    {
        APP_DEBUG("app_config_error: Failed to open %s\n", filename);
    }
    else
    {
        APP_DEBUG("app_config_error: writing file %s\n", filename);
        size = fwrite(file_buffer, 1, strlen(file_buffer), fp);
        fclose(fp);
    }
}
