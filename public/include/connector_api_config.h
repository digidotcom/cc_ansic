/*
 * Copyright (c) 2011, 2013 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
#ifndef CONNECTOR_API_CONFIG_H
#define CONNECTOR_API_CONFIG_H

/**
* @defgroup connector_request_id_config_t Configuration Requests
* @{
*/
/**
* Configuration Request Id passed to the application's callback for requesting configuration data.
* The class id for this connector_request_id_config_t is connector_class_id_config.
*/
typedef enum {
    connector_request_id_config_device_id,         /**< Requesting callback to return a unique device id which is used to identify the device.*/
    connector_request_id_config_vendor_id,         /**< Requesting callback to return a unique vendor id identifying the manufacturer of a device. */
    connector_request_id_config_device_type,       /**< Requesting callback to return device type that identifies the type of the device  */
    connector_request_id_config_server_url,        /**< Requesting callback to return iDigi Device Cloud URL which will be connected to */
    connector_request_id_config_connection_type,   /**< Requesting callback to return LAN or WAN connection type */
    connector_request_id_config_mac_addr,          /**< Requesting callback to return device's MAC addresss */
    connector_request_id_config_link_speed,        /**< Requesting callback to return link speed for WAN connection type */
    connector_request_id_config_phone_number,      /**< Requesting callback to return phone number dialed for WAN connection type */
    connector_request_id_config_tx_keepalive,      /**< Requesting callback to return server's Tx keep alive interval in seconds */
    connector_request_id_config_rx_keepalive,      /**< Requesting callback to return server's Rx keep alive interval in seconds */
    connector_request_id_config_wait_count,        /**< Requesting callback to return the number of intervals of not receiving a keep alive message after which a connection should be considered lost. */
    connector_request_id_config_ip_addr,           /**< Requesting callback to return device's IP address */
    connector_request_id_config_error_status,      /**< Error status notification which tells callback that error is encountered. */
    connector_request_id_config_firmware_facility, /**< Requesting callback to return whether firmware facility is supported or not. */
    connector_request_id_config_data_service,      /**< Requesting callback to return whether data service is supported or not. */
    connector_request_id_config_file_system,       /**< Requesting callback to return whether file system is supported or not. */
    connector_request_id_config_remote_configuration, /**< Requesting callback to return whether remote configuration is supported or not. */
    connector_request_id_config_max_transaction,   /**< Requesting callback to obtain maximum messaging sessions supported by client. */
    connector_request_id_config_device_id_method,  /**< Requesting callback to obtain method on how device id is generated */
    connector_request_id_config_imei_number,       /**< Requesting callback to return GSM IMEI number */
    connector_request_id_config_network_tcp,       /**< Requesting callback to return whether connection over TCP need to start now. */
    connector_request_id_config_network_udp,       /**< Requesting callback to return whether connection over UDP need to start now. */
    connector_request_id_config_network_sms,       /**< Requesting callback to return whether connection over SMS need to start now. */
    connector_request_id_config_wan_type,          /**< Requesting callback to return WAN type used. It tells iDigi connector whether it's GSM IMEI, CDMA ESN or CDMA MEID. */
    connector_request_id_config_esn,               /**< Requesting callback to return CDMA ESN (Electronic Serial Number) */
    connector_request_id_config_meid,              /**< Requesting callback to return CDMA MEID (Mobile Equipment Identifier) */
    connector_request_id_config_identity_verification,     /**< Requesting callback to return identity verification form. */
    connector_request_id_config_password,          /**< Requesting callback to return password for the password identity verification form */
    connector_request_id_config_sms_service_id     /**< Requesting callback to return service ID to be used as a preamble in each message */
} connector_request_id_config_t;
/**
* @}
*/

/**
* @defgroup connector_config_pointer_data_t Configuration Data Pointer
* @{
*/
/**
* Application Configuration Data Pointer
*
* @see connector_request_id_config_device_id callback
*/
typedef struct {
    uint8_t * data;         /**< Pointer to application device ID */
    size_t bytes_required;  /**< Number of bytes needed for application device ID */
} connector_config_pointer_data_t;
/**
* @}
*/


/**
* @defgroup connector_config_vendor_t  Vendor ID
* @{
*/
/**
* Vendor ID for @see connector_request_id_config_vendor_id callback
*/
typedef struct {
    uint32_t id;    /**< Vendor ID */
} connector_config_vendor_id_t;
/**
* @}
*/


/**
* @defgroup connector_config_name_data_t Configuration Name Pointer
* @{
*/
/**
* Application Name Point
*
* @see connector_request_id_config_device_type callback
* @see connector_request_id_config_server_url callback
*/
typedef struct {
    char * name;           /**< Pointer to application string */
    size_t bytes_in_name;  /**< Length of device type string  */
} connector_config_pointer_name_t;
/**
* @}
*/

/**
* @defgroup connector_connection_type_t iDigi connection types
* @{
*/
/**
* Connection type for @see connector_config_connection_type callback
*/
typedef enum {
   connector_lan_connection_type,   /**< LAN connection type for Ethernet or WiFi */
   connector_wan_connection_type    /**< WAN connection type for PPP over Modem */
} connector_connection_type_t;
/**
* @}
*/

/**
* @defgroup connector_config_connection_type_t Connector Connection types
* @{
*/
/**
* Connection type for @see connector_request_id_config_connection_type callback
*/
typedef struct {
    connector_connection_type_t type; /**< Types of connections */
} connector_config_connection_type_t;
/**
* @}
*/


/**
* @defgroup connector_config_link_speed_t  Link Speed
* @{
*/
/**
* Link Speed for WAN connection type for @see connector_request_id_config_link_speed callback
*/
typedef struct {
    uint32_t speed;        /**< Link speed value */
} connector_config_link_speed_t;
/**
* @}
*/


/**
* @defgroup connector_config_keepalive_t Keep-Alive Interval
* @{
*/
/**
* Keep-alive interval
*
* @see connector_request_id_config_tx_keepalive callback
* @see connector_request_id_config_rx_keepalive callback
*/
typedef struct {
    uint16_t interval;       /**< Keep-alive interval value */
} connector_config_keepalive_t;
/**
* @}
*/


/**
* @defgroup connector_config_wait_count_t Wait Count
* @{
*/
/**
* Wait count for TX Keep-alive interval
*
* @see connector_request_id_config_tx_keepalive callback
*/
typedef struct {
    uint16_t count;       /**< wait count */
} connector_config_wait_count_t;
/**
* @}
*/


/**
* @defgroup connector_config_ip_addr_t Device IP Address
* @{
*/
/**
* Device IP Address for @ref connector_request_id_config_ip_addr callback
*/
typedef struct {
    uint8_t * addr;         /**< Pointer to device's IP address */
    size_t bytes_in_addr;  /**< Number of bytes of the IP address */
} connector_config_ip_addr_t;
/**
* @}
*/


/**
* @defgroup connector_service_supported_status_t Service Support
* @{
*/
/**
* Service supported status which is used in the application's callback
* telling iDigi connector whether application supports a service or not.
* @see @ref firmware_support
* @see @ref data_service_support
*/
typedef enum {
    connector_service_unsupported,  /**< Service is not supported */
    connector_service_supported     /**< Service is supported */
} connector_service_supported_status_t;
/**
* @}
*/


/**
* @defgroup connector_config_support_status_t Supported Status of a Facility or Service
* @{
*/
/**
* Support status for @ref connector_request_id_config_firmware_facility,
* @ref connector_request_id_config_data_service, or
* @ref connector_request_id_config_file_system callback
*/
typedef struct {
    connector_service_supported_status_t status; /**< Application supported status  */
} connector_config_supported_status_t;
/**
* @}
*/


/**
* @defgroup connector_device_id_method_t Device ID Method Types
* @{
*/
/**
 * Device ID method returned by the application's callback for @ref connector_config_device_id_method callback.
 */
typedef enum {
    connector_auto_device_id_method,                  /**< Callback returns this type telling iDigi connector to
                                                       generate the device ID from MAC address (See @ref connector_config_mac_addr callback)
                                                       for LAN connection type or genearte the device ID according to the @ref wan_type.
                                                       @note iDigi connector will not call @ref connector_config_device_id callback. */
    connector_manual_device_id_method                 /**< Callback returns this type telling iDigi connector to retrieve device ID from @ref connector_config_device_id callback */
} connector_device_id_method_t;
/**
* @}
*/


/**
* @defgroup connector_config_device_id_method_t Device ID Method
* @{
*/
/**
 * Device ID method returned by the application's callback for @ref connector_request_id_config_device_id_method callback.
 */
typedef struct {
    connector_device_id_method_t method;    /**< Device ID method */

} connector_config_device_id_method_t;
/**
* @}
*/


/**
* @defgroup connector_auto_connect_type_t Action on Network Close
* @{
*/
/**
* Response to @ref connector_network_close callback which is called to close the connection to the iDigi Device Cloud.
*/
typedef enum {

    connector_auto_connect,     /**< Connect to the iDigi Device Cloud automatically */
    connector_manual_connect    /**< Connect to the iDigi Device Cloud manually */

} connector_auto_connect_type_t;
/**
* @}
*/


/**
* @defgroup connector_config_connect_status_t Connect status
* @{
*/
/**
* Connect status for the connector to make connection to Etherios Device Cloud automatically or manually
*
* @see @ref connector_request_id_config_network_tcp,
* @see @ref connector_request_id_config_network_upd, or
* @see @ref connector_request_id_config_network_tc
*/
typedef struct {
    connector_auto_connect_type_t status;   /**< Connect status */

} connector_config_connect_status_t;
/**
* @}
*/


/**
* @defgroup connector_wan_type_t iDigi WAN types for WAN connection type
* @{
*/
/**
 * WAN connection type for @ref connector_config_wan_type callback.
 */
typedef enum {
    connector_imei_wan_type,    /**< IMEI number for GSM network */
    connector_esn_wan_type,     /**< ESN for CDMA network */
    connector_meid_wan_type     /**< MEID for CDMA network */
} connector_wan_type_t;
/**
* @}
*/


/**
* @defgroup connector_config_wan_type_t Device Configuration WAN Type
* @{
*/
/**
* Device WAN Type
*
**/
typedef struct {
    connector_wan_type_t type;   /**< WAN type */

} connector_config_wan_type_t;
/**
* @}
*/


/**
* @defgroup connector_identity_verification_t  Identity Verification Form Types
* @{
*/
/**
 * Identify verification returned by the application's callback for @ref connector_request_id_config_identity_verification callback.
 */
typedef enum {
    connector_simple_identity_verification,         /**< Callback returns this form for simple identity verification. iDigi connector just transmits the
                                                     device ID and the iDigi Device Cloud URL. */
    connector_password_identity_verification        /**< Callback returns this form for password identity verification. iDigi connector exchanges a pre-shared
                                                     secret password with the iDigi Device Cloud. The password must match the password set in the iDigi Device Cloud.
                                                     @note iDigi connector will call @ref connector_request_id_config_password callback for the password. */
} connector_identity_verification_t;
/**
* @}
*/


/**
* @defgroup connector_config_identity_verification_t Device Configuration Identity Verification
* @{
*/
/**
* Device Identity Verification
*
**/
typedef struct {
    connector_identity_verification_t type;   /**< Identity verification type */

} connector_config_identity_verification_t;
/**
* @}
*/


/**
* @defgroup connector_config_max_transaction_t Device Configuration Maximum Transaction
* @{
*/
/**
* Device maximum tranaction
*
**/
typedef struct {
    uint8_t count;   /**< Maximum transaction count */

} connector_config_max_transaction_t;
/**
* @}
*/


#endif /* CONNECTOR_API_CONFIG_H */

