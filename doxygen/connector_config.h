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
  *  @brief Cloud Connector configuration options
  *
  */
#ifndef __CONNECTOR_CONFIG_H_
#define __CONNECTOR_CONFIG_H_


/**
* @defgroup connector_config_options Cloud Connector Configuration Options
* @{
*
* @brief Defines the major blocks of functionality used in Cloud Connector
*/

/**
 *  When defined, the system is little endian.
 *
 *  @note For Big endian systems, this line should be commented out.
 */
#define CONNECTOR_LITTLE_ENDIAN

/**
 * When defined, Cloud Connector private library includes debug support.  Cloud Connector state
 * and debug data will be displayed using the @ref connector_debug_printf. See @ref debug_routine
 * for its implementation. Macro error testing is compiled and implemented using the
 * @ref ASSERT macro.
 *
 * This debug feature can be eliminated in a product release, which will greatly reduce memory codespace and
 * RAM needs, by commenting this line out in connector_config.h:
 *
 * @code
 * #define CONNECTOR_DEBUG
 * @endcode
 *
 * To this:
 * @code
 * //#define CONNECTOR_DEBUG
 * @endcode
 *
 * @see connector_debug_printf
 * @see ASSERT
 */
#define CONNECTOR_DEBUG

/**
 * When defined, Cloud Connector private library includes the @ref firmware_download
 * "Firmware Download Service".
 *
 * To disable the @ref firmware_download "Firmware Download Service" feature, comment
 * this line out in connector_config.h:
 *
 * @code
 * #define CONNECTOR_FIRMWARE_SERVICE
 * @endcode
 *
 * To this:
 * @code
 * //#define CONNECTOR_FIRMWARE_SERVICE
 * @endcode
 *
 * @see @ref firmware_support
 */
#define CONNECTOR_FIRMWARE_SERVICE

/**
 * When defined, Cloud Connector includes the @ref zlib "compression" support used with the
 * @ref data_service.
 *
 * By default, compression support is disabled.  To enable Optional Data Compression Support,
 * change this line in connector_config.h:
 * By default, optional @ref zlib "compression" support is disabled.
 *
 * To enable @ref zlib "compression", uncomment the define in connector_config.h:
 *
 * @code
 * //#define CONNECTOR_COMPRESSION
 * @endcode
 *
 * To this:
 * @code
 * #define CONNECTOR_COMPRESSION
 * @endcode
 *
 * @note When included, this requires the @ref zlib "zlib" library.
 *
 * @see @ref data_service
 * @see @ref CONNECTOR_DATA_SERVICE
 * @see @ref CONNECTOR_FILE_SYSTEM
 * @see @ref CONNECTOR_RCI_SERVICE
 *
 */
#define CONNECTOR_COMPRESSION

/**
 * If defined, Cloud Connector includes the @ref data_service.
 * To disable the @ref data_service feature, comment this line out in connector_config.h:
 *
 * @code
 * #define CONNECTOR_DATA_SERVICE
 * @endcode
 *
 * To this:
 * @code
 * //#define CONNECTOR_DATA_SERVICE
 * @endcode
 *
 * @see @ref data_service_support
 * @see @ref zlib
 * @see @ref CONNECTOR_COMPRESSION
 */
#define CONNECTOR_DATA_SERVICE

/**
* If defined, Cloud Connector includes the @ref data_point.
* To disable the @ref data_point feature, comment this line out in connector_config.h:
*
* @code
* #define CONNECTOR_DATA_POINTS
* @endcode
*
* To this:
* @code
* //#define CONNECTOR_DATA_POINTS
* @endcode
*
* @see @ref data_point
* @see @ref zlib
* @see @ref CONNECTOR_COMPRESSION
*/
#define CONNECTOR_DATA_POINTS

/**
 * If defined, Cloud Connector includes the @ref file_system.
 * To enable the @ref file_system feature, uncomment this line in connector_config.h:
 *
 * @code
 * // #define CONNECTOR_FILE_SYSTEM
 * @endcode
 *
 * To this:
 * @code
 * #define CONNECTOR_FILE_SYSTEM
 * @endcode
 *
 * @see @ref file_system_support
 * @see @ref zlib
 * @see @ref CONNECTOR_COMPRESSION
 */
#define CONNECTOR_FILE_SYSTEM

/**
 * If defined, Cloud Connector includes the @ref rci_service.
 * To disable the @ref rci_service feature, comment this line out in connector_config.h:
 *
 * @code
 * #define CONNECTOR_RCI_SERVICE
 * @endcode
 *
 * To this:
 * @code
 * // #define CONNECTOR_RCI_SERVICE
 * @endcode
 *
 * @see @ref rci_service
 * @see @ref app_get_remote_configuration_support()
 * @see @ref CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH
 * @see @ref zlib
 * @see @ref CONNECTOR_COMPRESSION
 */
#define CONNECTOR_RCI_SERVICE

/**
 * This is used to define the maximum content length in bytes of an element's value for the @ref rci_service,
 * Cloud Connector includes the @ref rci_service that allows user remote configuration, control, and information
 * exchange between a device and Device Cloud.
 * See @ref CONNECTOR_RCI_SERVICE to enable or disable the service.
 *
 * @code
 * #define CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH 256
 * @endcode
 *
 * @see @ref CONNECTOR_RCI_SERVICE
 */
#define CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH    256

/**
* If defined, Cloud Connector includes the @ref cli_request.
* To disable the @ref cli_request feature, comment this line out in connector_config.h:
*
* @code
* #define CONNECTOR_SM_CLI
* @endcode
*
* To this:
* @code
* //#define CONNECTOR_SM_CLI
* @endcode
*
* @see @ref shortmessaging
* @see @ref zlib
* @see @ref CONNECTOR_COMPRESSION
*/
#define CONNECTOR_SM_CLI

/**
* If defined, Cloud Connector includes the ability to receive multiple short messages in
* one packet. If there are more messages pending in Device Cloud then user will be notified with
* @ref pending_data. To retreive pending mesages, user has to send a request (can be @ref ping_request)
* to Device Cloud. If there is no @ref pending_data callback then that means no pending messages at this
* point. This mecahnism is very helpfull for battary operated devices, where they can go back to sleep
* as soon as they get the response without any @ref pending_data.
* To disable the this functionality, comment this line out in connector_config.h:
*
* @code
* #define CONNECTOR_SM_BATTERY
* @endcode
*
* To this:
* @code
* //#define CONNECTOR_SM_BATTERY
* @endcode
*
* @see @ref shortmessaging
* @see @ref pending_data
* @see @ref zlib
* @see @ref CONNECTOR_COMPRESSION
*/
#define CONNECTOR_SM_BATTERY


/**
 * This is used to define the maximum length in bytes of the full file path on the device, supported by the @ref file_system.
 * This length includes an ending null-character.
 *
 * For example, if the device receives a request to list the directory '/mydir' and the '/mydir' directory has an entry
 * 'myfile', the full path to the directory entry is '/mydir/myfile'. This full path '/mydir/myfile' has a length of 14 bytes,
 * including an ending null-character.
 *
 * @note System limitation: CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH must be <= 460
 *
 * See @ref CONNECTOR_FILE_SYSTEM to enable or disable the service.
 *
 * @code
 * #define CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH 256
 * @endcode
 *
 * @see @ref CONNECTOR_FILE_SYSTEM
 * @see @ref file_system_readdir "Read a directory" callback
 */
#define CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH   256

/**
 * When defined, Cloud Connector private library does not use dynamic memory allocations,
 * static memory buffers are used instead. This eliminates the possibility of memory fragmentation.
 *
 * When no dynamic RAM option is used, @ref CONNECTOR_MSG_MAX_TRANSACTION must be defined and it should be
 * less or equal to 32. The number of simultaneous data service transactions, initiated by the device
 * is 1.
 *
 * To enable no dynamic RAM feature, uncomment this line in connector_config.h:
 *
 * @code
 * //#define CONNECTOR_NO_MALLOC
 * @endcode
 *
 * To this:
 * @code
 * #define CONNECTOR_NO_MALLOC
 * @endcode
 *
 */
#define CONNECTOR_NO_MALLOC

/**
 * If defined, Cloud Connector includes the TCP transport.
 * To disable this feature, comment this line out in connector_config.h:
 *
 * @code
 * #define CONNECTOR_TRANSPORT_TCP
 * @endcode
 *
 * To this:
 * @code
 * // #define CONNECTOR_TRANSPORT_TCP
 * @endcode
 *
 * @see @ref network_tcp_start
 * @see @ref app_start_network_tcp()
 */
#define CONNECTOR_TRANSPORT_TCP

/**
* If defined, Cloud Connector includes the UDP transport.
* To disable this feature, comment this line out in connector_config.h:
*
* @code
* #define CONNECTOR_TRANSPORT_UDP
* @endcode
*
* To this:
* @code
* // #define CONNECTOR_TRANSPORT_UDP
* @endcode
*
* @see @ref network_udp_start
* @see @ref app_start_network_udp()
*/
#define CONNECTOR_TRANSPORT_UDP

/**
 * This is used to enable support for file sizes larger than 2
 * gigabyte in file system listing requests. On linux platform 
 * the following must be added to Makefile: 
 *  
 * CFLAGS += D_FILE_OFFSET_BITS=64 
 *
 * @code
 * #define CONNECTOR_FILE_SYSTEM_HAS_LARGE_FILES
 * @endcode
 *
 * @see @ref file_system_stat callback 
 * @see @ref file_system_stat_dir_entry callback  
 * @see @ref CONNECTOR_FILE_SYSTEM 
 */
#define CONNECTOR_FILE_SYSTEM_HAS_LARGE_FILES

/**
 * @}*/

#ifdef ENABLE_COMPILE_TIME_DATA_PASSING

/**
 * @defgroup connector_config_data_options Hard Coded Application Configuration options
 * @{
 *
 *  @brief Hard Coded Application Configuration options is an optional method where configuration is pulled from hardcode
 *  defined at compile time instead of the @ref connector_callback_t "Application callback" method.
 *
 *  If the following configurations in @ref connector_config.h are defined, the code to make the @ref connector_callback_t "application callbacks"
 *  is uncompiled and the hard-coded values are used instead:
 *
 * @code
 *     #define CONNECTOR_DEVICE_TYPE                          "Cloud Connector Linux Sample"
 *     #define CONNECTOR_CLOUD_URL                            "login.etherios.com"
 *     #define CONNECTOR_TX_KEEPALIVE_IN_SECONDS              75
 *     #define CONNECTOR_RX_KEEPALIVE_IN_SECONDS              75
 *     #define CONNECTOR_WAIT_COUNT                           10
 *     #define CONNECTOR_VENDOR_ID                            0x00000000
 *     #define CONNECTOR_MSG_MAX_TRANSACTION                  1
 *     #define CONNECTOR_CONNECTION_TYPE                      connector_connection_type_lan
 *     #define CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND    0
 *     #define CONNECTOR_WAN_PHONE_NUMBER_DIALED              "0123456789"
 *     #define CONNECTOR_FIRMWARE_SUPPORT
 *     #define CONNECTOR_DATA_SERVICE_SUPPORT
 *     #define CONNECTOR_FILE_SYSTEM_SUPPORT
 *     #define CONNECTOR_NETWORK_TCP_START                    connector_connect_auto
 *     #define CONNECTOR_WAN_TYPE                             connector_wan_type_imei
 *     #define CONNECTOR_IDENTITY_VERIFICATION                connector_identity_verification_simple
 * @endcode
 *
 * This option is useful for reducing Cloud Connector code space for those applications that are sensitive to memory usage.  The following
 * table describes the each define and the callback cross reference:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 * <th class="title">Configuration Defines</td>
 * <th class="title">Description</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_DEVICE_TYPE @htmlonly</td>
 * <td>Device type in iso-8859-1 encoded string to identify the device.
 * See @endhtmlonly @ref device_type @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_CLOUD_URL @htmlonly </td>
 * <td>Device Cloud FQDN.
 * See @endhtmlonly @ref device_cloud_url @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_TX_KEEPALIVE_IN_SECONDS @htmlonly </td>
 * <td>TX Keepalive interval in seconds between 5 and 7200 seconds.
 * See @endhtmlonly @ref tx_keepalive @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_RX_KEEPALIVE_IN_SECONDS @htmlonly </td>
 * <td>RX Keepalive interval in seconds between 5 and 7200 seconds.
 * See @endhtmlonly @ref rx_keepalive @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_WAIT_COUNT @htmlonly </td>
 * <td>Maximum number of consecutive missing keepalive messages for a
 * connection to be considered lost.  Valid range is greater than 1 and less than 64.
 * See @endhtmlonly @ref wait_count @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_VENDOR_ID @htmlonly </td>
 * <td>Vendor ID from Device Cloud Account.
 * See @endhtmlonly @ref vendor_id @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_MSG_MAX_TRANSACTION @htmlonly </td>
 * <td>Maximum simultaneous transactions for data service receiving message.
 * See @endhtmlonly @ref max_msg_transactions @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_CONNECTION_TYPE @htmlonly </td>
 * <td> @endhtmlonly @ref connector_connection_type_lan @htmlonly for LAN connection or
 * @endhtmlonly @ref connector_connection_type_wan @htmlonly WAN connection.
 * See @endhtmlonly @ref connection_type @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND @htmlonly </td>
 * <td>Link speed for WAN connection.
 * See @endhtmlonly @ref link_speed @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_WAN_PHONE_NUMBER_DIALED @htmlonly </td>
 * <td>String represetnation of a WAN cellular phone connection.
 * See @endhtmlonly @ref phone_number @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_FIRMWARE_SUPPORT @htmlonly </td>
 * <td>If defined it enables firmware download capability.
 * See @endhtmlonly @ref firmware_support @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_DATA_SERVICE_SUPPORT @htmlonly </td>
 * <td>If defined it enables data service capability.
 * See @endhtmlonly @ref data_service_support @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_NETWORK_TCP_START @htmlonly </td>
 * <td>If defined it enables and starts TCP.
 * See @endhtmlonly @ref network_tcp_start @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_WAN_TYPE @htmlonly </td>
 * <td> @endhtmlonly @ref connector_wan_type_imei  for IMEI ,
 * @ref connector_wan_type_esn for ESN  or @ref connector_wan_type_meid for MEID
 * WAN type. See @ref wan_type @htmlonly</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref CONNECTOR_IDENTITY_VERIFICATION @htmlonly </td>
 * <td> @endhtmlonly @ref identity_verification @htmlonly for
 * simple or password identity verification form.
 * See @endhtmlonly @ref identity_verification @htmlonly</td>
 * </tr>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 *
 *
 */
/**
 * When defined, this string is hardcode for the @ref device_type instead of the application framework
 * function @ref app_get_device_type (called via the @ref connector_request_id_config_device_type @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_device_type when CONNECTOR_DEVICE_TYPE is defined.
 *
 * @see @ref device_type
 * @see @ref app_get_device_type
 */
#define CONNECTOR_DEVICE_TYPE                             "Cloud Connector Linux Sample"

/**
 * When defined, this string hardcode for the @ref device_cloud_url instead of the application framework
 * function @ref app_get_device_cloud_url (called via the @ref connector_request_id_config_device_cloud_url @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_device_cloud_url when CONNECTOR_CLOUD_URL is defined.
 *
 * @see @ref device_cloud_url
 * @see @ref app_get_device_cloud_url()
 */
#define CONNECTOR_CLOUD_URL                               "login.etherios.com"

/**
 * When defined, this string hardcode for the @ref tx_keepalive instead of the application framework
 * function @ref  app_get_tx_keepalive_interval() (called via the @ref connector_request_id_config_tx_keepalive @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref  app_get_tx_keepalive_interval() when CONNECTOR_TX_KEEPALIVE_IN_SECONDS is defined.
 *
 * @see @ref tx_keepalive
 * @see @ref  app_get_tx_keepalive_interval()
 */
#define CONNECTOR_TX_KEEPALIVE_IN_SECONDS                 75

/**
 * When defined, this string hardcode for the @ref rx_keepalive instead of the application framework
 * function @ref app_get_rx_keepalive_interval() (called via the @ref connector_request_id_config_rx_keepalive @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_rx_keepalive_interval() when CONNECTOR_RX_KEEPALIVE_IN_SECONDS is defined.
 *
 * @see @ref rx_keepalive
 * @see @ref app_get_rx_keepalive_interval()
 */
#define CONNECTOR_RX_KEEPALIVE_IN_SECONDS                 75

/**
 * When defined, this string hardcode for the @ref wait_count instead of the application framework
 * function @ref app_get_wait_count() (called via the @ref connector_request_id_config_wait_count @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_wait_count() when CONNECTOR_RX_KEEPALIVE_IN_SECONDS is defined.
 *
 * @see @ref wait_count
 * @see @ref app_get_wait_count()
 */
#define CONNECTOR_WAIT_COUNT                              10

/**
 * When defined, this 4 byte value is hardcode for the @ref vendor_id instead of the application framework
 * function @ref app_get_vendor_id() (called via the @ref connector_request_id_config_vendor_id @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_wait_count() when CONNECTOR_RX_KEEPALIVE_IN_SECONDS is defined.
 *
 * @see @ref vendor_id
 * @see @ref app_get_vendor_id()
 * @see @ref connector_vendor_id "Obtaining an Device Cloud Vendor ID"
 */
#define CONNECTOR_VENDOR_ID                            0x00000000

/**
 * When defined, this string hardcode for the @ref max_msg_transactions instead of the application framework
 * function @ref app_get_max_message_transactions() (called via the @ref connector_request_id_config_max_transaction @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_max_message_transactions() when CONNECTOR_MSG_MAX_TRANSACTION is defined.
 *
 * @see @ref max_msg_transactions
 * @see @ref app_get_max_message_transactions()
 */
#define CONNECTOR_MSG_MAX_TRANSACTION                  1

/**
 * When defined, this string hardcode for the @ref connection_type instead of the application framework
 * function @ref app_get_connection_type() (called via the @ref connector_request_id_config_connection_type @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_connection_type() when CONNECTOR_CONNECTION_TYPE is defined.  The
 * values for this define are limited to @ref connector_connection_type_lan or @ref connector_connection_type_wan.
 *
 * @see @ref connection_type
 * @see @ref app_get_connection_type()
 * @see @ref connector_connection_type_t
 */
#define CONNECTOR_CONNECTION_TYPE                      connector_connection_type_lan

/**
 * When defined, this string hardcode for the @ref link_speed instead of the application framework
 * function @ref app_get_link_speed() (called via the @ref connector_request_id_config_link_speed @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_link_speed() when CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND is defined.
 *
 * @see @ref link_speed
 * @see @ref app_get_link_speed()
 */
#define CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND    0

/**
 * When defined, this string hardcode for the @ref phone_number instead of the application framework
 * function @ref app_get_phone_number() (called via the @ref connector_request_id_config_phone_number @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_phone_number() when CONNECTOR_WAN_PHONE_NUMBER_DIALED is defined.
 *
 * @see @ref phone_number
 * @see @ref app_get_phone_number()
 */
#define CONNECTOR_WAN_PHONE_NUMBER_DIALED              "0123456789"

/**
 * When defined, this string hardcode for the @ref firmware_support instead of the application framework
 * function @ref app_get_firmware_support() (called via the @ref connector_request_id_config_firmware_facility @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_firmware_support() when CONNECTOR_FIRMWARE_SUPPORT is defined.
 * @note This define is only used when @ref CONNECTOR_FIRMWARE_SERVICE is defined in @ref connector_config.h.
 *
 * @see @ref firmware_support
 * @see @ref app_get_firmware_support()
 * @see @ref CONNECTOR_FIRMWARE_SERVICE
 */
#define CONNECTOR_FIRMWARE_SUPPORT

/**
 * When defined, this string hardcode for the @ref data_service_support instead of the application framework
 * function @ref app_get_data_service_support() (called via the @ref connector_request_id_config_data_service @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_data_service_support() when CONNECTOR_DATA_SERVICE_SUPPORT is defined.
 * @note This define is only used when @ref CONNECTOR_DATA_SERVICE is defined in @ref connector_config.h.
 *
 * @see @ref data_service_support
 * @see @ref app_get_data_service_support()
 * @see @ref CONNECTOR_DATA_SERVICE
 */
#define CONNECTOR_DATA_SERVICE_SUPPORT

/**
 * When defined, this string hardcode for the @ref file_system_support instead of the application framework
 * function @ref app_get_file_system_support() (called via the @ref connector_request_id_config_file_system @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_file_system_support() when CONNECTOR_FILE_SYSTEM_SUPPORT is defined.
 * @note This define is only used when @ref CONNECTOR_FILE_SYSTEM is defined in @ref connector_config.h.
 *
 * @see @ref file_system_support
 * @see @ref app_get_file_system_support()
 * @see @ref CONNECTOR_FILE_SYSTEM
 */
#define CONNECTOR_FILE_SYSTEM_SUPPORT


/**
 * When defined, this string hardcode for the remote_configuration_support instead of the application framework
 * function @ref app_get_remote_configuration_support() (called via the @ref connector_request_id_config_remote_configuration @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_remote_configuration_support() when CONNECTOR_REMOTE_CONFIGURATION_SUPPORT is defined.
 * @note This define is only used when @ref CONNECTOR_RCI_SERVICE is defined a non-zero in @ref connector_config.h.
 *
 * @see @ref rci_support
 * @see @ref app_get_remote_configuration_support()
 * @see @ref CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH
 */
#define CONNECTOR_REMOTE_CONFIGURATION_SUPPORT

/**
 * When defined, this string hardcode for the @ref device_id_method instead of the application framework
 * function @ref app_get_device_id_method() (called via the @ref connector_request_id_config_device_id_method @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_device_id_method() when CONNECTOR_DEVICE_ID_METHOD is defined.  The
 * values for this define are limited to @ref connector_device_id_method_auto or @ref connector_device_id_method_manual.
 *
 * @see @ref device_id_method
 * @see @ref app_get_device_id_method()
 * @see @ref connector_device_id_method_t
 */
#define CONNECTOR_DEVICE_ID_METHOD connector_device_id_method_auto

/**
 * When defined, this string hardcode for the @ref network_tcp_start instead of the application framework
 * function @ref app_start_network_tcp() (called via the @ref connector_request_id_config_network_tcp @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_start_network_tcp() when CONNECTOR_NETWORK_TCP_START is defined. The
 * values for this define are limited to @ref connector_connect_auto or @ref connector_connect_manual.
 * @note This define is only used when @ref CONNECTOR_TRANSPORT_TCP is defined in @ref connector_config.h.
 *
 * @see @ref network_tcp_start
 * @see @ref connector_connect_auto_type_t
 * @see @ref app_start_network_tcp()
 * @see @ref CONNECTOR_TRANSPORT_TCP
 */
#define CONNECTOR_NETWORK_TCP_START                 connector_connect_auto

/**
 * When defined, this string hardcode for the @ref network_udp_start instead of the application framework
 * function @ref app_start_network_udp() (called via the @ref connector_request_id_config_network_tcp @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_start_network_udp() when CONNECTOR_NETWORK_UDP_START is defined. The
 * values for this define are limited to @ref connector_connect_auto or @ref connector_connect_manual.
 * @note This define is only used when @ref CONNECTOR_TRANSPORT_UDP is defined in @ref connector_config.h.
 *
 * @see @ref network_udp_start
 * @see @ref connector_connect_auto_type_t
 * @see @ref app_start_network_udp()
 * @see @ref CONNECTOR_TRANSPORT_UDP
 */
#define CONNECTOR_NETWORK_UDP_START                 connector_connect_auto


/**
 * When defined, this string hardcode for the @ref wan_type instead of the application framework
 * function @ref app_get_wan_type() (called via the @ref connector_request_id_config_wan_type @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_wan_type() when CONNECTOR_WAN_TYPE is defined.  The
 * values for this define are limited to @ref connector_wan_type_imei, @ref connector_wan_type_esn or @ref connector_wan_type_meid.
 * @note This define is only used when @ref device_id_method is connector_device_id_method_auto and
 * @ref connection_type is @ref connector_connection_type_wan.
 *
 * @see @ref wan_type
 * @see @ref CONNECTOR_CONNECTION_TYPE
 * @see @ref CONNECTOR_DEVICE_ID_METHOD
 * @see @ref app_get_wan_type()
 */
#define CONNECTOR_WAN_TYPE                              connector_wan_type_imei

/**
 * When defined, this string hardcode for the @ref identity_verification instead of the application framework
 * function @ref app_get_identity_verification() (called via the @ref connector_request_id_config_identity_verification @ref connector_callback_t "callback" in config.c).
 *
 * @note There is no need to implement or port @ref app_get_identity_verification() when CONNECTOR_IDENTITY_VERIFICATION is defined.  The
 * values for this define are limited to @ref connector_identity_verification_simple, or @ref connector_identity_verification_password.
 *
 * @see @ref identity_verification
 * @see @ref app_get_identity_verification()
 * @see @ref connector_identity_verification_t
 */
#define CONNECTOR_IDENTITY_VERIFICATION                 connector_identity_verification_simple

#endif

/**
* If @ref CONNECTOR_TRANSPORT_UDP is defined, Cloud Connector will use the define below to set the
* maximum short message session at a time. If not set, Cloud Connector will use the default of 2. 
*
* @see @ref shortmessaging
* @see @ref CONNECTOR_TRANSPORT_UDP
*/
#define CONNECTOR_SM_MAX_SESSIONS                  4

/**
* If @ref CONNECTOR_TRANSPORT_UDP is defined, Cloud Connector will use the define below to set the
* maximum short message segments used per session. If not set, Cloud Connector will use the default of 1.
* User need to increase this value if they are planning to use short message to send larger data.
*
* @see @ref shortmessaging
* @see @ref CONNECTOR_TRANSPORT_UDP
*/
#define CONNECTOR_SM_MAX_SEGMENTS                  4


/**
* If @ref CONNECTOR_TRANSPORT_UDP is defined, Cloud Connector will use the define below to set the
* session timeout in seconds to this value. If not set, Cloud Connector will use the default of
* no timeout (0). Cloud Connector will wait for Device Cloud response (complete response) until
* this period. 
*
* @see @ref shortmessaging
* @see @ref CONNECTOR_TRANSPORT_UDP
*/
#define CONNECTOR_SM_TIMEOUT                    30


/**
 * When defined, the Cloud Connector compilation will expect the ANSI C99 stdint.h header file,
 * even though we're setup for an ANSI C89 environment.
 *
 * This define should get added to your makefile:
 *
 * @code
 * CFLAGS +=-std=c89 -DCONNECTOR_HAVE_STDINT_HEADER
 * @endcode
 *
 * @see @ref errors_due_to_C89_and_stdint
 * @see @ref resolving_compilation_issues
 *
 */
#define CONNECTOR_HAVE_STDINT_HEADER

/**
* @}
*/


#endif
