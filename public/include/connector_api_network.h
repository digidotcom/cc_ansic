/*
 * Copyright (c) 2013 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

#ifndef CONNECTOR_API_NETWORK_H
#define CONNECTOR_API_NETWORK_H


/**
* @defgroup connector_request_id_network_t Network Request IDs
* @{
*/
/**
* Network Request ID passed to the application's callback for network interface.
* The class id for this connector_network_request_t is connector_class_network_tcp,
* connector_class_id_network_udp.
*/
typedef enum {
    connector_request_id_network_open,     /**< Requesting callback to set up and make connection to the iDigi Device Cloud */
    connector_request_id_network_send,     /**< Requesting callback to send data to the iDigi Device Cloud */
    connector_request_id_network_receive,  /**< Requesting callback to receive data from the iDigi Device Cloud */
    connector_request_id_network_close     /**< Requesting callback to close the iDigi Device Cloud connection */
} connector_request_id_network_t;
/**
* @}
*/

/**
* @defgroup connector_network_open_data_t Network Open Data Structure
* @{
*/
/**
* Network open data structure for @ref connector_network_open callback which is called to open and connect to the iDigi Device Cloud.
*/
typedef struct  {
    char const * device_cloud_url;          /**< Pointer to Etherios Cloud URL */
    connector_network_handle_t * handle;    /**< Application defined network handle associated with the connection */
} connector_network_open_data_t;
/**
* @}
*/


/**
* @defgroup connector_network_send_data_t Network Send Data Structure
* @{
*/
/**
* Send data structure for @ref connector_network_send callback which is called to send data to the iDigi Device Cloud.
*/
typedef struct  {
    connector_network_handle_t * handle;    /**< Pointer to network handle associated with a connection through the connector_network_open callback */
    uint8_t const * buffer;                 /**< Pointer to data to be sent */
    size_t bytes_available;                 /**< Number of bytes to send in the buffer */
    size_t bytes_used;                      /**< Number of bytes sent */
} connector_network_send_data_t;
/**
* @}
*/

/**
* @defgroup connector_network_receive_data_t Network Receive Request
* @{
*/
/**
* Read request structure for connector_network_receive callback which is called to receive
* a specified number of bytes data from the iDigi Device Cloud.
*/
typedef struct  {
    connector_network_handle_t * handle;    /**< Pointer to network handle associated with a connection through the connector_network_open callback */
    uint8_t * buffer;                       /**< Pointer to memory where callback writes received data to */
    size_t bytes_available;                 /**< Number of bytes available in the buffer */
    size_t bytes_used;                      /**< Number of bytes received and copied to the buffer */
} connector_network_receive_data_t;
/**
* @}
*/

/**
* @defgroup connector_close_status_t iDigi Connection Close Status Values
* @{
*/
/**
* Reasons for @ref connector_network_close callback which is called to close the connection to the iDigi Device Cloud.
*/
typedef enum {
    connector_close_status_server_disconnected = 1,    /**< iDigi connector received a disconnect from the server. */
    connector_close_status_server_redirected,          /**< iDigi connector is redirected to different server. */
    connector_close_status_device_terminated,          /**< iDigi connector is terminated via @ref connector_initiate_action
                                                        iDigi connector will terminate all active messages or requests and free all memory.
                                                        @ref connector_connect_auto_type_t returned status from the close callback will be ignored. */
    connector_close_status_device_stopped,             /**< iDigi connector is stopped via @ref connector_initiate_action */
    connector_close_status_no_keepalive,               /**< iDigi connector has not received keep alive messages from the server */
    connector_close_status_abort,                      /**< iDigi connector is aborted either it encountered fatal error or callback aborted iDigi connector.
                                                        iDigi connector will terminate all active messages or requests and free all memory.
                                                        @ref connector_connect_auto_type_t returned status from the close callback will be ignored. */
    connector_close_status_device_error                /**< iDigi connector received error from callback which requires to close the connection. */
} connector_close_status_t;
/**
* @}
*/

/**
* @defgroup connector_close_data_t Network Close Request
* @{
*/
/**
* Close request structure for @ref connector_network_close callback which is called to close the connection to the iDigi Device Cloud.
*/
typedef struct  {
    connector_network_handle_t * handle;    /**< Pointer to network handle associated with a connection through the connector_network_open callback */
    connector_close_status_t  status;               /**< Reason for closing the network handle */

    connector_bool_t  reconnect;                     /**< connector_true - reconnect, connector_false - don't reconnect */
} connector_network_close_data_t;
/**
* @}
*/

#endif /* CONNECTOR_API_NETWORK_H */
