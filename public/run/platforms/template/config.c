/*
 * Copyright (c) 2011, 2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

 /**
  * @file
  *  @brief Configuration routines for the iDigi Connector.
  *
  */
#include <stdio.h>
#include <ctype.h>

#include "connector_config.h"
#include "connector_api.h"
#include "connector_debug.h"
#include "platform.h"

/**
 * @brief   Get the IP address of the device
 *
 * This routine assigns a pointer to the IP address of the device in *ip_address
 * along with the size of the IP address which must be either a 4-octet value for
 * IPv4 or a 6-octet value for IPv6.
 *
 * @param [out] ip_address  Pointer to memory containing IP address
 * @param [out] size Size of the IP address in bytes
 *
 * @retval connector_callback_continue  IP address was successfully returned
 * @retval connector_callback_abort     Could not get IP address and abort iDigi connector.
 *
 * @see @ref ip_address API Configuration Callback
 */
static connector_callback_status_t app_get_ip_address(uint8_t const ** ip_address, size_t * const size)
{
    /* Remove this #error statement once you modify this routine to return the correct IP address */
#error "Specify device IP address. Set size to 4 (bytes) for IPv4 or 16 (bytes) for IPv6"

    UNUSED_ARGUMENT(ip_address);
    UNUSED_ARGUMENT(size);

    return connector_callback_continue;
}

/**
 * @brief   Get the MAC address of the device
 *
 * This routine assigns a pointer to the MAC address of the device in *mac_address along
 * with the size.
 *
 * @param [out] mac_address  Pointer to memory containing IP address
 * @param [out] size Size of the MAC address in bytes (6 bytes).
 *
 * @retval connector_callback_continue  MAC address was successfully returned
 * @retval connector_callback_abort     Could not get the MAC address and abort iDigi connector.
 *
 * @see @ref mac_address API Configuration Callback
 */
static connector_callback_status_t app_get_mac_addr(uint8_t const ** mac_address, size_t * const size)
{
    #define MAC_ADDR_LENGTH     6

    /* MAC address used in this sample */
    static uint8_t const device_mac_addr[MAC_ADDR_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#error "Specify device MAC address for LAN connection"

    *mac_address = device_mac_addr;
    *size = sizeof device_mac_addr;

    return connector_callback_continue;
}

/**
 * @brief   Get the iDigi device ID
 *
 * This routine is called to get a unique device ID which is used to identify the device.
 *
 * Device IDs are a globally unique identifier for iDigi clients.  The Device ID is a
 * 16-octet value derived from the MAC address of a network interface on the client.
 * The mapping from MAC address to Device ID consists of inserting "FFFF" in the middle
 * of the MAC and setting all other bytes of the Device ID to 0.
 * For Example:
 * MAC Address 12:34:56:78:9A:BC, would map to a Device ID: 00000000-123456FF-FF789ABC.
 * If a client has more than one network interface, it does not matter to the iDigi Device Cloud which
 * network interface MAC is used for the basis of the Device ID.  If the MAC is read
 * directly from the network interface to generate the client's Device ID, care must be
 * taken to always use the same network interface's MAC since there is a unique mapping
 * between a device and a Device ID.
 *
 * The pointer ID is filled in with the address of the memory location which contains the
 * device ID, size is filled in with the size of the device ID.
. *
 * @param [out] id  Pointer to memory containing the device ID
 * @param [out] size Size of the device ID in bytes (16 bytes)
 *
 * @retval connector_callback_continue  Device ID was successfully returned.
 * @retval connector_callback_abort     Could not get the device ID and abort iDigi connector.
 *
 * @see @ref device_id API Configuration Callback
 */
static connector_callback_status_t app_get_device_id(uint8_t const ** id, size_t * const size)
{
    #define DEVICE_ID_LENGTH    16

    static uint8_t device_id[DEVICE_ID_LENGTH] = {0};

    uint8_t const * mac_addr;
    size_t mac_size;
    connector_callback_status_t status;

#error  "Specify device id"

    /* This sample uses the MAC address to format the device ID */
    status  = app_get_mac_addr(&mac_addr, &mac_size);

    if (status == connector_callback_continue)
    {
        device_id[8] = mac_addr[0];
        device_id[9] = mac_addr[1];
        device_id[10] = mac_addr[2];
        device_id[11] = 0xFF;
        device_id[12] = 0xFF;
        device_id[13] = mac_addr[3];
        device_id[14] = mac_addr[4];
        device_id[15] = mac_addr[5];

        *id   = device_id;
        *size = sizeof device_id;
    }

    return status;
}

/**
 * @brief   Get the iDigi vendor ID
 *
 * This routine assigns a pointer to the vendor ID which is a unique code identifying
 * the manufacturer of a device. Vendor IDs are assigned to manufacturers by iDigi Device Cloud.
 *
 * @param [out] id  Pointer to memory containing the device ID
 * @param [out] size Size of the vendor ID in bytes (4 bytes)
 *
 * @retval connector_callback_continue  Vendor ID was successfully returned.
 * @retval connector_callback_abort     Could not get the vendor ID and abort iDigi connector.
 *
 * @see @ref vendor_id API Configuration Callback
 * @see @ref connector_config_vendor_id
 * @see @ref connector_vendor_id "Obtaining an iDigi Vendor ID"
 *
 * @note This routine is not needed if you define @b CONNECTOR_VENDOR_ID configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_vendor_id(uint8_t const ** id, size_t * const size)
{
#error  "Specify vendor id"

    #define VENDOR_ID_LENGTH    4

    static uint8_t const device_vendor_id[VENDOR_ID_LENGTH] = {0x00, 0x00, 0x00, 0x00};

    *id   = device_vendor_id;
    *size = sizeof device_vendor_id;

    return connector_callback_continue;
}

/**
 * @brief   Get the device type
 *
 * This routine returns a pointer to the device type which is an iso-8859-1 encoded string.
 * This string should be chosen by the device manufacture as a name that uniquely
 * identifies this model of device  to the server. When the server finds two devices
 * with the same device type, it can infer that they are the same product and
 * product scoped data may be shared among all devices with this device type.
 * A device's type cannot be an empty string, nor contain only whitespace.
 *
 * @param [out] type  Pointer to memory containing the device type
 * @param [out] size Size of the device type in bytes (Maximum is 63 bytes)
 *
 * @retval connector_callback_continue  Device type was successfully returned.
 * @retval connector_callback_abort     Could not get the device type and abort iDigi connector.
 *
 * @see @ref device_type API Configuration Callback
 * @see @ref CONNECTOR_DEVICE_TYPE
 *
 * @note This routine is not needed if you define @ref CONNECTOR_DEVICE_TYPE configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_device_type(char const ** type, size_t * const size)
{
#error "Specify device type"
    static char const device_type[] = "Linux Sample";

    /* Return pointer to device type. */
    *type = (char *)device_type;
    *size = sizeof device_type -1;

    return connector_callback_continue;
}

/**
 * @brief   Get the iDigi server URL
 *
 * This routine assigns a pointer to the ASCII null-terminated string of the iDigi
 * Device Cloud FQDN, this is typically developer.idig.com.
 *
 * @param [out] url  Pointer to memory containing the URL
 * @param [out] size Size of the server URL in bytes (Maximum is 63 bytes)
 *
 * @retval connector_callback_continue  The URL type was successfully returned.
 * @retval connector_callback_abort     Could not get the URL and abort iDigi connector.
 *
 * @see @ref server_url API Configuration Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_CLOUD_URL configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_server_url(char const ** url, size_t * const size)
{
#error "Specify iDigi Server URL"
    static char const connector_server_url[] = "my.idigi.com";

    /* Return pointer to device type. */
    *url = connector_server_url;
    *size = sizeof connector_server_url -1;

    return connector_callback_continue;
}

/**
 * @brief   Get the connection type
 *
 * This routine specifies the connection type as @ref connector_lan_connection_type or
 * @ref connector_wan_connection_type. Fill in the type parameter with the address of the
 * connector_connection_type_t.
 *
 * @param [out] type  Pointer to memory containing the @ref connector_connection_type_t
 *
 * @retval connector_callback_continue  The connection type was successfully returned.
 * @retval connector_callback_abort     Could not get connection type and abort iDigi connector.
 *
 * @see @ref connection_type API Configuration Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_CONNECTION_TYPE configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_connection_type(connector_connection_type_t const ** type)
{
#error "Specify LAN or WAN connection type"

    /* Return pointer to connection type */
    static connector_connection_type_t const device_connection_type = connector_lan_connection_type;

    *type = &device_connection_type;

    return connector_callback_continue;
}

/**
 * @brief   Get the link speed
 *
 * This routine assigns the link speed for WAN connection type. If connection type is LAN,
 * iDigi Connector will not request link speed configuration.
 *
 * @param [out] speed Pointer to memory containing the link speed
 * @param [out] size Size of the link speed in bytes
 *
 * @retval connector_callback_continue  The link speed was successfully returned.
 * @retval connector_callback_abort     Could not get the link speed and abort iDigi connector
 *
 * @see @ref link_speed API Configuration Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_link_speed(uint32_t const ** speed, size_t * const size)
{
#error "Specify link speed for WAN connection type"

    UNUSED_ARGUMENT(speed);
    UNUSED_ARGUMENT(size);
    return connector_callback_continue;
}

/**
 * @brief   Get the WAN phone number
 *
 * This routine assigns the phone number dialed for WAN connection type,
 * including any dialing prefixes. It's a variable length, non null-terminated string.
 * If connection type is LAN, iDigi Connector will not request phone number.
 *
 * @param [out] number  Pointer to memory containing the phone number
 * @param [out] size Size of the phone number in bytes
 *
 * @retval connector_callback_continue  The phone number was successfully returned.
 * @retval connector_callback_abort     Could not get the phone number and abort iDigi connector.
 *
 * @see @ref phone_number API Configuration Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_WAN_PHONE_NUMBER_DIALED configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_phone_number(char const ** number, size_t * const size)
{
#error "Specify phone number dialed for WAN connection type"
    /*
     * Return pointer to phone number for WAN connection type.
     */
    UNUSED_ARGUMENT(number);
    UNUSED_ARGUMENT(size);
    return connector_callback_continue;
}

/**
 * @brief   Get the TX keepalive interval
 *
 * This routine assigns the TX keepalive interval in seconds. This indicates how
 * often the iDigi Device Cloud sends a keepalive message to the device to verify the
 * device is still operational. Keepalive messages are from the prospective of the iDigi Device Cloud,
 * this keepalive is sent from the iDigi Device Cloud to the device. The value must be between 5 and 7200 seconds.
 *
 * @param [out] interval  Pointer to memory containing the keep alive interval
 * @param [out] size Size of memory buffer, containing the keep alive interval in bytes (this must be 2 bytes).
 *
 * @retval connector_callback_continue  The keep alive interval was successfully returned.
 * @retval connector_callback_abort     Could not get the keep alive interval and abort iDigi connector.
 *
 * @see @ref tx_keepalive API Configuration Callback
 *
 * @note This routine is not needed if you define @ref CONNECTOR_TX_KEEPALIVE_IN_SECONDS configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_tx_keepalive_interval(uint16_t const ** interval, size_t * const size)
{
#error "Specify server to device TX keepalive interval in seconds"

#define    DEVICE_TX_KEEPALIVE_INTERVAL_IN_SECONDS    45
    /* Return pointer to Tx keepalive interval in seconds */
    static uint16_t const device_tx_keepalive_interval = DEVICE_TX_KEEPALIVE_INTERVAL_IN_SECONDS;

    *interval = &device_tx_keepalive_interval;
    *size = sizeof device_tx_keepalive_interval;

    return connector_callback_continue;
}

/**
 * @brief   Get the RX keepalive interval
 *
 * This routine assigns the RX keepalive interval in seconds. This indicates how
 * often the iDigi Connector device sends keepalive messages to the iDigi Device Cloud. Keepalive
 * messages are from the prospective of the iDigi Device Cloud, this keepalive is sent from the
 * device to the iDigi Device Cloud. The value must be between 5 and 7200 seconds.
 *
 * @param [out] interval  Pointer to memory containing the keep alive interval
 * @param [out] size Size of memory buffer, containing the keep alive interval in bytes (this must be 2 bytes).
 *
 * @retval connector_callback_continue  The keep alive interval was successfully returned.
 * @retval connector_callback_abort     Could not get the keep alive interval and abort iDigi connector.
 *
 * @see @ref rx_keepalive API Configuration Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_RX_KEEPALIVE_IN_SECONDS configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_rx_keepalive_interval(uint16_t const ** interval, size_t * const size)
{
#error "Specify server to device RX keepalive interval in seconds"
#define    DEVICE_RX_KEEPALIVE_INTERVAL_IN_SECONDS    45
    /* Return pointer to Rx keepalive interval in seconds */
    static uint16_t const device_rx_keepalive_interval = DEVICE_RX_KEEPALIVE_INTERVAL_IN_SECONDS;

    *interval = &device_rx_keepalive_interval;
    *size = sizeof device_rx_keepalive_interval;

    return connector_callback_continue;
}

/**
 * @brief   Get the wait count
 *
 * This routine assigns the number of times that not receiving a keepalive message
 * from the iDigi Device Cloud will indicate that the connection is considered lost.
 * This must be a 2-octet integer value between 2 to 64 counts.
 *
 * @param [out] count  Pointer to memory containing the wait count
 * @param [out] size Size of memory buffer, containing the wait count in bytes (this must be 2 bytes).
 *
 * @retval connector_callback_continue  The wait count was successfully returned.
 * @retval connector_callback_abort     Could not get the wait count and abort iDigi connector.
 *
 * @see @ref wait_count API Configuration Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_WAIT_COUNT configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_wait_count(uint16_t const ** count, size_t * const size)
{
#error "Specify the number of times that not receiving keepalive messages from server is allowed"
#define    DEVICE_WAIT_COUNT    3
    /*
     * Return pointer to wait count (number of times not receiving Tx keepalive
     * from server is allowed).
     */
    static uint16_t const device_wait_count = DEVICE_WAIT_COUNT;

    *count = &device_wait_count;
    *size = sizeof device_wait_count;

    return connector_callback_continue;
}

/**
 * @brief   Get firmware update support
 *
 * This routine tells iDigi Connector whether firmware update capability is supported or not.
 * If firmware update is not supported, callback for connector_class_firmware
 * class will not be executed.
 *
 * @param [out] isSupported  Pointer memory where callback writes connector_service_supported if firmware update is supported or
 *                            connector_service_unsupported  if firmware update is not supported.
 *
 * @retval connector_callback_continue  The firmware update support was successfully returned.
 * @retval connector_callback_abort     Could not get the firmware update support and abort iDigi connector.
 *
 * @see @ref firmware_support API Configuration Callback
 *
 * @note This routine is not called if you define @b CONNECTOR_FIRMWARE_SUPPORT configuration in @ref connector_config.h.
 * @note This CONNECTOR_FIRMWARE_SUPPORT indicates application supports firmware download. See @ref connector_config_data_options
 *
 * @note See @ref CONNECTOR_FIRMWARE_SERVICE to include firmware access facility code in iDigi Connector.
 */
static connector_callback_status_t app_get_firmware_support(connector_service_supported_status_t * const isSupported)
{
    *isSupported = connector_service_supported;

    return connector_callback_continue;
}

/**
 * @brief  Get data service support
 *
 * This routine tells iDigi Connector whether the data service facility is supported or not.
 * If you plan on sending data to/from the iDigi Device Cloud set this to connector_service_supported.
 *
 * @param [out] isSupported  Pointer memory where callback writes connector_service_supported if data service is supported or
 *                            connector_service_unsupported  if data service is not supported.
 *
 * @retval connector_callback_continue  The data service support was successfully returned.
 * @retval connector_callback_abort     Could not get the data service support and abort iDigi connector.
 *
 * @see @ref data_service_support API Configuration Callback
 *
 * @note This routine is not called if you define @b CONNECTOR_DATA_SERVICE_SUPPORT configuration in @ref connector_config.h.
 * @note This CONNECTOR_DATA_SERVICE_SUPPORT indicates application supports data service. See @ref connector_config_data_options
 *
 * @note See @ref CONNECTOR_DATA_SERVICE to include data service code in iDigi Connector.
 * @note See @ref CONNECTOR_COMPRESSION for data service transferring compressed data.
 */
static connector_callback_status_t app_get_data_service_support(connector_service_supported_status_t * const isSupported)
{
    *isSupported = connector_service_supported;

    return connector_callback_continue;
}

/**
 * @brief   Get file system support
 *
 * This routine tells iDigi Connector whether the file system facility is supported or not.
 * If you plan to access device files from the iDigi Device Cloud set this to connector_service_supported.
 *
 * @param [out] isSupported  Pointer memory where callback writes connector_service_supported if file system is supported or
 *                            connector_service_unsupported  if file system is not supported.
 *
 * @retval connector_callback_continue  The file system support was successfully returned.
 * @retval connector_callback_abort     Could not get the file system support and abort iDigi connector.
 *
 * @see @ref file_system_support API Configuration Callback
 *
 * @note This routine is not called if you define @b CONNECTOR_FILE_SYSTEM_SUPPORT configuration in @ref connector_config.h.
 * @note This CONNECTOR_FILE_SYSTEM_SUPPORT indicates application supports file system. See @ref connector_config_data_options
 *
 * @note See @ref CONNECTOR_FILE_SYSTEM to include file system code in iDigi Connector.
 * @note See @ref CONNECTOR_COMPRESSION for file system transferring compressed data.
 */
static connector_callback_status_t app_get_file_system_support(connector_service_supported_status_t * const isSupported)
{
    *isSupported = connector_service_supported;

    return connector_callback_continue;
}

/**
 * @brief   Get the remote configuration support
 *
 * This routine tells iDigi connector whether the remote configuration service is supported or not.
 * If you plan on accessing device data configurations through the iDigi Device Cloud set
 * this to connector_service_supported.
 *
 * @param [out] isSupported  Pointer memory where callback writes connector_service_supported if the remote configuration is supported or
 *                            connector_service_unsupported  if the remote configuration is not supported.
 *
 * @retval connector_callback_continue  The remote configuration support was successfully returned.
 * @retval connector_callback_abort     Could not get the remote configuration support and abort iDigi connector.
 *
 * @note @b CONNECTOR_RCI_SERVICE must be defined in connector_config.h
 * @note @b CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH is used to define maximum length of an element including the name of element in @ref connector_config.h.
 * @note This routine is not called if you define @b CONNECTOR_REMOTE_CONFIGURATION_SUPPORT configuration in @ref connector_config.h.
 * @note This CONNECTOR_REMOTE_CONFIGURATION_SUPPORT indicates application supports remote configuration. See @ref connector_config_data_options
 *
 */
static connector_service_supported_status_t app_get_remote_configuration_support(connector_service_supported_status_t * const isSupported)
{
    *isSupported = connector_service_supported;

    return connector_callback_continue;
}

/**
 * @brief   Get maximum transactions
 *
 * This routine tells iDigi Connector the maximum simultaneous transactions for data service
 * to receive messages from the iDigi Device Cloud.
 *
 * @param [out] transCount  Pointer memory where callback writes the maximum simultaneous transaction.
 *                           Writes 0 for unlimited transactions.
 *
 * @retval connector_callback_continue  The maximum simultaneous transactions was successfully returned.
 * @retval connector_callback_abort     Could not get the maximum simultaneous transactions and abort iDigi connector.
 *
 * @see @ref max_msg_transactions API Configuration Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_MSG_MAX_TRANSACTION configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_max_message_transactions(unsigned int * const transCount)
{
	/**
	 */
#define    CONNECTOR_MAX_MSG_TRANSACTIONS    1

    *transCount = CONNECTOR_MAX_MSG_TRANSACTIONS;

    return connector_callback_continue;
}

/**
 * @brief   Get device id method
 *
 * This routine tells iDigi Connector how to obtain a device ID.
 *
 * @param [out] method  Pointer memory where callback writes:
 *                      @li @a @b digi_auto_device_id_method: to generate device ID from
 *                             - @ref mac_address callback for @ref connector_lan_connection_type connection type or
 *                             - @ref imei_number callback for @ref connector_wan_connection_type connection type.
 *                      @li @a @b connector_manual_device_id_method: to obtain device ID from @ref device_id callback.
 *
 * @retval connector_callback_continue  The device ID method was successfully returned.
 * @retval connector_callback_abort     Could not get the device ID method and abort iDigi connector.
 *
 * @see @ref connection_type API Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_DEVICE_ID_METHOD configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_device_id_method(connector_device_id_method_t * const method)
{

    *method = connector_auto_device_id_method;

    return connector_callback_continue;
}

/**
 * @brief   Get IMEI number
 *
 * This routine returns IMEI number. This routine is called when @ref device_id_method callback returns
 * @ref connector_auto_device_id_method for WAN connection type and @ref wan_type callback returns
 * @ref connector_imei_wan_type.
 *
 * @param [out] imei_number  Pointer to memory which contains 14 IMEI decimal digits plus one check digit.
 *                           Each nibble corresponds a decimal digit and most upper nibble must be 0.
 * @param [out] size         Size of the imei_number in bytes. It should be 8 bytes.
 *
 * @retval connector_callback_continue  The IMEI number was successfully returned.
 * @retval connector_callback_abort     Could not get the IMEI number and abort iDigi connector.
 *
 * @see @ref device_id_method API Callback
 * @see @ref connection_type API Callback
 * @see @ref wan_type API Callback
 *
 */
static connector_callback_status_t app_get_imei_number(uint8_t ** const imei_number, size_t * size)
{
#define APP_IMEI_LENGTH 8

#error "Specify the IMEI number for WAN connection type if app_get_device_id_method returns connector_auto_device_id_method and app_get_device_id_method returns connector_imei_wan_type"
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

/**
 * @brief  Start Network TCP
 *
 * This routine tells iDigi Connector whether it starts TCP automatically or manually.
 * You need to call connector_initiate_action with connector_initiate_transport_start to start TCP if manually starts TCP.
 *
 * @param [out] auto_start  Pointer memory where callback writes connector_auto_connect to start TCP automatically or
 *                            connector_manual_connect to start TCP manually.
 *
 * @retval connector_callback_continue  TCP was successfully started automatically or manually.
 * @retval connector_callback_abort     Abort iDigi connector.
 *
 * @see @ref network_tcp_start API Configuration Callback
 *
 * @note This routine is not called if you define @b CONNECTOR_TRANSPORT_TCP and @b CONNECTOR_NETWORK_TCP_START configuration in @ref connector_config.h.
 * @note This CONNECTOR_TRANSPORT_TCP indicates application supports network TCP.
 *
 * @note See @ref CONNECTOR_TRANSPORT_TCP to include TCP code in iDigi Connector.
 */
static connector_callback_status_t app_start_network_tcp(connector_auto_connect_type_t * const auto_start)
{
    *auto_start = connector_auto_connect;

    return connector_callback_continue;
}

/**
* @brief  Start Network UDP
*
* This routine tells iDigi Connector whether it starts UDP automatically or manually.
* You need to call connector_initiate_action with connector_initiate_transport_start to start UDP if manually is selected
* when this callback is called.
*
* @param [out] auto_start  Pointer to memory where callback writes connector_auto_connect to start UDP automatically or
*                          connector_manual_connect to start UDP manually.
*
* @retval connector_callback_continue  UDP configuration was successfully set to start automatically or manually.
* @retval connector_callback_abort     Abort iDigi connector.
*
* @see @ref network_udp_start API Configuration Callback
*
* @note This routine is not called if you define @b CONNECTOR_TRANSPORT_UDP configuration in @ref connector_config.h.
* @note This CONNECTOR_TRANSPORT_UDP indicates application supports network UDP.
*
* @note See @ref CONNECTOR_TRANSPORT_UDP to include UDP code in iDigi Connector.
*/
static connector_callback_status_t app_start_network_udp(connector_auto_connect_type_t * const auto_start)
{
    *auto_start = connector_auto_connect;

    return connector_callback_continue;
}

/**
* @brief  Start Network SMS
*
* This routine tells iDigi Connector whether it starts the SMS automatically or manually.
* You need to call connector_initiate_action with connector_initiate_transport_start to start SMS if manually is selected
* when this callback is called.
*
* @param [out] auto_start  Pointer to memory where callback writes connector_auto_connect to start SMS automatically or
*                          connector_manual_connect to start SMS manually.
*
* @retval connector_callback_continue  SMS configuration was successfully set to start automatically or manually.
* @retval connector_callback_abort     Abort iDigi connector.
*
* @see @ref network_sms_start API Configuration Callback
*
* @note This routine is not called if you define @b CONNECTOR_TRANSPORT_SMS configuration in @ref connector_config.h.
* @note This CONNECTOR_TRANSPORT_SMS indicates application supports network SMS.
*
* @note See @ref CONNECTOR_TRANSPORT_SMS to include SMS code in iDigi Connector.
*/
static connector_callback_status_t app_start_network_sms(connector_auto_connect_type_t * const auto_start)
{
    *auto_start = connector_auto_connect;

    return connector_callback_continue;
}

/**
 * @brief   Get the WAN type
 *
 * This routine specifies the WAN type as @ref connector_imei_wan_type, @ref connector_esn_wan_type, or
 * @ref connector_meid_wan_type. Fill in the type parameter with the address of the
 * connector_wan_type_t.
 *
 * @param [out] type  Pointer to memory containing the @ref connector_connection_type_t
 *
 * @retval connector_callback_continue  The WAN type was successfully returned.
 * @retval connector_callback_abort     Could not get WAN type and abort iDigi connector.
 *
 * @see @ref wan_type API Configuration Callback
 *
 * @note This routine is not needed if you define @b CONNECTOR_WAN_TYPE configuration in @ref connector_config.h.
 * See @ref connector_config_data_options
 */
static connector_callback_status_t app_get_wan_type(connector_wan_type_t * const type)
{
#error "Specify connector_imei_wan_type for IMEI, connector_esn_wan_type for ESN, or connector_meid_wan_type for MEID WAN type"

    *type = connector_imei_wan_type;
    APP_DEBUG("app_get_wan_type\n");

    return connector_callback_continue;
}


/**
 * @brief   Get ESN number
 *
 * This routine returns ESN number. This routine is called when @ref device_id_method callback returns
 * @ref connector_auto_device_id_method for WAN connection type and @ref wan_type callback returns
 * @ref connector_esn_wan_type.
 *
 *
 * @param [out] esn_number  Pointer to memory which contains 8 ESN hexadecimal.
 *                           Each nibble corresponds a hexadecimal or decimal digit.
 * @param [out] size         Size of the esn_number in bytes. It should be 4 bytes.
 *
 * @retval connector_callback_continue  The ESN number was successfully returned.
 * @retval connector_callback_abort     Could not get the ESN number and abort iDigi connector.
 *
 * @see @ref device_id_method API Callback
 * @see @ref connection_type API Callback
 * @see @ref wan_type API Callback
 *
 */
static connector_callback_status_t app_get_esn(uint8_t ** const esn_number, size_t * size)
{
#define APP_ESN_HEX_LENGTH 4

#error "Specify the ESN number for WAN connection type if app_get_device_id_method returns connector_auto_device_id_method and app_get_device_id_method returns connector_esn_wan_type."
    /* Each nibble corresponds a decimal digit.
     */
    static char const app_esn_hex_string[] = "00000000";
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
                uint8_t value = 0;

                if (isdigit(app_esn_hex_string[i]))
                    value = (app_esn_hex_string[i] - '0');
                else if (isxdigit(app_esn_hex_string[i]))
                    value = (tolower(app_esn_hex_string[i]) - 'a') + 0xa;

                app_esn_hex[index] += (value << (n * 4));
                n++;
            }
        }
        index--;
    }

    *esn_number = app_esn_hex;
    *size = sizeof app_esn_hex;

    return connector_callback_continue;
}

/**
 * @brief   Get MEID number
 *
 * This routine returns MEID number. This routine is called when @ref device_id_method callback returns
 * @ref connector_auto_device_id_method for WAN connection type and @ref wan_type callback returns
 * @ref connector_meid_wan_type.
 *
 *
 * @param [out] meid_number  Pointer to memory which contains 14 MEID hexadecimal. check digit is not included.
 *                           Each nibble corresponds a hexadecimal or decimal digit.
 * @param [out] size         Size of the esn_number in bytes. It should be 7 bytes.
 *
 * @retval connector_callback_continue  The MEID number was successfully returned.
 * @retval connector_callback_abort     Could not get the MEID number and abort iDigi connector.
 *
 * @see @ref device_id_method API Callback
 * @see @ref connection_type API Callback
 * @see @ref wan_type API Callback
 *
 */
static connector_callback_status_t app_get_meid(uint8_t ** const meid_number, size_t * size)
{
#define APP_MEID_HEX_LENGTH 7

#error "Specify the MEID number for WAN connection type if app_get_device_id_method returns connector_auto_device_id_method and app_get_device_id_method returns connector_meid_wan_type."
    /* Each nibble corresponds a decimal digit.
     */
    static char const app_meid_hex_string[] = "00000000000000";
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
                uint8_t value = 0;

                if (isdigit(app_meid_hex_string[i]))
                    value = (app_meid_hex_string[i] - '0');
                else if (isxdigit(app_meid_hex_string[i]))
                    value = (tolower(app_meid_hex_string[i]) - 'a') + 0xa;
                app_meid_hex[index] += (value << (n * 4));
                n++;
            }
        }
        index--;
    }

    *meid_number = app_meid_hex;
    *size = sizeof app_meid_hex;

    return connector_callback_continue;
}

/**
 * @brief   Get identity verification form
 *
 * This routine returns identity verification form which allows the device to determine that it is
 * communicating with the iDigi Device Cloud, and allows the iDigi Device Cloud to determine and
 * verify the Device ID of the device it is communicating with for TCP transport.
 *
 * @note The identity verification form in this function should be referenced by devicesecurity setting
 * when @ref rci_service is supported and devicesecurity setting is defined in configuration file (see @ref rci_tool).
 *
 * @note If password identity verification form is used, @ref app_get_password will be called to obtain
 * the password.
 *
 * @param [out] identity  Pointer to memory when identity verification form will be written to.
 *
 * @retval connector_callback_continue  The identity verification form was successfully returned.
 * @retval connector_callback_abort     Could not get the identity verification form and abort iDigi connector.
 *
 * @see @ref app_get_password API Callback
 *
 */
static connector_callback_status_t app_get_identity_verification(connector_identity_verification_t * const identity)
{
#error "Specify connector_identity_verification for simple or password identify verification form"

    *identity = connector_simple_identity_verification;

    return connector_callback_continue;
}

/**
 * @brief   Get password for password identity verification form
 *
 * This routine returns the password which the iDigi Device Cloud verifies the password to
 * its stored password when password identity verification form is used. See @ref app_get_identity_verification.
 *
 * @param [out] password  Pointer to password.
 * @param [out] size      Length of the password in bytes.
 *
 * @retval connector_callback_continue  The password was successfully returned.
 * @retval connector_callback_abort     Could not get the password and abort iDigi connector.
 *
 * @see @ref app_get_identity_verification API Callback
 *
 */
static connector_callback_status_t app_get_password(char const ** password, size_t * const size)
{
#error "Specify password for password identity verification form"
    static  char const connector_password[] = "";

    /* Return pointer to password. */
    *password = connector_password;
    *size = sizeof connector_password -1;

    return connector_callback_continue;
}

/**
* @brief   Get SMS service ID (shared code)
*
* This routine returns the service ID to be used when communicating with the iDigi Device Cloud via SMS.
* The service ID are special short codes that use an identifier as part of the message that allows more
* than one user to share a short code. In case the service ID is not used, set the content of service_id
* to NULL and the content of size to 0.
*
* @param [out] service_id  Pointer to store the service ID.
* @param [out] size        Pointer to store the service ID length in bytes.
*
* @retval connector_callback_continue  The service ID was successfully returned.
* @retval connector_callback_abort     Could not get the service ID and abort iDigi connector.
*
*/
static connector_callback_status_t app_get_sms_service_id(char const ** const service_id, size_t * const size)
{
#error "Specify SMS service id. It is optional, set *service_id to NULL and *size to 0 if not used"
    static  char const sms_service_id[] = "IDGP";

    /* Return pointer to service_id. */
    *service_id = sms_service_id;
    *size = sizeof sms_service_id -1;

    return connector_callback_continue;
}

/* End of iDigi connector configuration routines */

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
        enum_to_case(connector_request_id_status_tcp);
        enum_to_case(connector_request_id_status_stop_completed);
    }
    return result;
}

#if (defined CONNECTOR_SHORT_MESSAGE)
static char const * app_sm_class_to_string(connector_sm_request_t const value)
{
    char const * result = NULL;
    switch (value)
    {
        enum_to_case(connector_request_id_sm_ping_request);
        enum_to_case(connector_request_id_sm_ping_response);
        enum_to_case(connector_request_id_sm_cli_request);
        enum_to_case(connector_request_id_sm_cli_response);
        enum_to_case(connector_request_id_sm_more_data);
        enum_to_case(connector_request_id_sm_opaque_response);
    }
    return result;
}
#endif

static char const * app_status_error_to_string(connector_status_t const value)
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

/**
 * @brief   Error status notification
 *
 * This routine is called when iDigi Connector encounters an error. This is used as
 * a debug tool for finding configuration or keepalive error.
 *
 * The error_data argument contains class id, request id, and error status.
 *
 * @note If @ref CONNECTOR_DEBUG is not defined in connector_config.h, iDigi Connector will
 * not call this callback to notify any error encountered.
 *
 * @retval connector_callback_continue  No Error.
 * @retval connector_callback_abort     Abort iDigi connector.
 *                                  @note This function may be called again to free other pointer even if it has returned connector_callback_abort.
 *
 * @see @ref error_status API Configuration Callback
 */
static connector_callback_status_t app_config_error(connector_error_status_t const * const error_data)
{
    APP_DEBUG("app_config_error: Class: %s (%d) ", app_class_to_string(error_data->class_id), error_data->class_id);

    switch (error_data->class_id)
    {
    case connector_class_config:
        APP_DEBUG("Request: %s (%d) ", app_config_class_to_string(error_data->request_id.config_request), error_data->request_id.config_request);
        break;
    case connector_class_network_tcp:
    case connector_class_id_network_udp:
    case connector_class_id_network_sms:
        APP_DEBUG("Request: %s (%d) ", app_network_class_to_string(error_data->request_id.network_request), error_data->request_id.network_request);
        break;
    case connector_class_operating_system:
        APP_DEBUG("Request: %s (%d) ", app_os_class_to_string(error_data->request_id.os_request), error_data->request_id.os_request);
        break;
    case connector_class_firmware:
        APP_DEBUG("Request: %s (%d) ", app_firmware_class_to_string(error_data->request_id.firmware_request), error_data->request_id.firmware_request);
        break;
    case connector_class_id_data_service:
        APP_DEBUG("Request: %s (%d) ", app_data_service_class_to_string(error_data->request_id.data_service_request), error_data->request_id.data_service_request);
        break;
    case connector_class_id_file_system:
        APP_DEBUG("Request: %s (%d) ", app_file_system_class_to_string(error_data->request_id.file_system_request), error_data->request_id.file_system_request);
        break;
    case connector_class_remote_config_service:
        APP_DEBUG("Request: %s (%d) ", app_remote_config_class_to_string(error_data->request_id.remote_config_request), error_data->request_id.remote_config_request);
        break;
    case connector_class_id_status:
        APP_DEBUG("Request: %s (%d) ", app_status_class_to_string(error_data->request_id.status_request), error_data->request_id.status_request);
        break;
#if (defined CONNECTOR_SHORT_MESSAGE)
    case connector_class_id_short_message:
        APP_DEBUG("Request: %s (%d) ", app_sm_class_to_string(error_data->request_id.sm_request), error_data->request_id.sm_request);
        break;
#endif
    }

    APP_DEBUG("Error status: %s (%d)\n", app_status_error_to_string(error_data->status), error_data->status);

    return connector_callback_continue;

}


/**
 * @cond DEV
 */
/*
 * Configuration callback routine.
 */
connector_callback_status_t app_config_handler(connector_config_request_t const request,
                                              void const * const request_data,
                                              size_t const request_length,
                                              void * response_data,
                                              size_t * const response_length)
{
    connector_callback_status_t status;

    UNUSED_ARGUMENT(request_length);

    switch (request)
    {
    case connector_config_device_id:
        status = app_get_device_id(response_data, response_length);
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

    case connector_config_mac_addr:
        status = app_get_mac_addr(response_data, response_length);
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
        status = app_config_error(request_data);
        break;

    case connector_config_firmware_facility:
        status = app_get_firmware_support(response_data);
        break;

    case connector_config_data_service:
        status = app_get_data_service_support(response_data);
        break;

    case connector_config_max_transaction:
        status = app_get_max_message_transactions(response_data);
        break;

    case connector_config_remote_configuration:
        status = app_get_remote_configuration_support(response_data);
        break;

    case connector_config_file_system:
        status = app_get_file_system_support(response_data);
        break;

    case connector_config_device_id_method:
        status = app_get_device_id_method(response_data);
        break;

     case connector_config_imei_number:
         status = app_get_imei_number(response_data, response_length);
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
        status = connector_callback_unrecognized;
        break;

    }
    return status;
}
/**
 * @endcond
 */
