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
 *  @brief Rountines which implement Etherios Cloud Connector network interface for
 *  @ref CONNECTOR_TRANSPORT_UDP.
 */
#include "connector_api.h"
#include "platform.h"

/**
 * @brief   Open a network socket to communicate the iDigi Device Cloud
 *
 * This routine opens a UDP socket and attempts to to resolve the name for 
 * the iDigi Device Cloud server. It sets the resolved peer address using
 * UDP connect().
 *  
 * @param [in] host_name  FQDN of iDigi Device Cloud server
 * @param [in] length  Number of bytes in the host_name
 * @param [out] network_handle This is filled in with the value of a handle which
 * is passed to subsequent networking calls. The @ref connector_network_handle_t "connector_network_handle_t"
 * is defined in public\include\connector_types.h.
 *
 * @retval connector_callback_continue	The routine has successfully opened a socket and resolved server's name. 
 * @retval connector_callback_busy 		The routine will be called again to complete open.
 * @retval connector_callback_error     The operation failed, Etherios Cloud Connector
 *                                  will exit @ref connector_run "connector_run()" or @ref connector_step "connector_step()".
 * @retval connector_callback_abort     The application aborts Etherios Cloud Connector.
 *
 * @see @ref open "Network API callback Open"
 */
static connector_callback_status_t app_network_udp_open(char const * const host_name, size_t const length, connector_network_handle_t ** network_handle)
{
    connector_callback_status_t rc = connector_callback_continue;

    UNUSED_ARGUMENT(host_name);
    UNUSED_ARGUMENT(length);
    UNUSED_ARGUMENT(network_handle);

    return rc;
}

/**
 * @brief   Send data to the iDigi Device Cloud
 *
 * This routine sends data to the iDigi Device Cloud. This function must not block.
 * If it encounters EAGAIN or EWOULDBLOCK error, 0 bytes must be returned and Etherios Cloud Connector
 * will continue calling this function. If successful connector_callback_continue is returned.
 * If the data could not be sent connector_callback_busy is returned, otherwise connector_callback_abort is returned. 
 *  
 * @param [in] write_data  Pointer to a connector_write_request_t structure
 * @param [out] sent_length Filled in with the number of bytes sent.
 *
 * @retval connector_callback_status_t
 *
 * @see @ref send API Network Callback
 */
static connector_callback_status_t app_network_udp_send(connector_write_request_t const * const write_data, size_t * const sent_length)
{
    connector_callback_status_t rc = connector_callback_continue;

    UNUSED_ARGUMENT(write_data);
    UNUSED_ARGUMENT(sent_length);

    return rc;
}

/**
 * @brief   Receive data from the iDigi Device Cloud
 *
 * This routine reads a specified number of bytes from the the iDigi Device Cloud.
 * This function blocks up to the timeout value specified. If no data is received
 * and the timeout has expired 0 bytes must be returned and Etherios Cloud Connector will continue
 * calling this function. 
 *  
 * @param [in] read_data  Pointer to a connector_read_request_t structure
 * @param [out] read_length Filled in with the number of bytes received.
 *
 * @retval connector_callback_status_t
 *
 * @note When running in a multithreaded model this is where Etherios Cloud Connector will
 * relinquish control, the user should sleep until data is received up to the
 * timeout given.
 *
 * @see @ref receive API Network Callback
 */
static connector_callback_status_t app_network_udp_receive(connector_read_request_t const * const read_data, size_t * const read_length)
{
    connector_callback_status_t rc = connector_callback_continue;

    UNUSED_ARGUMENT(read_data);
    UNUSED_ARGUMENT(read_length);
    return rc;
}

/**
 * @brief   Close the network socket to the iDigi Device Cloud
 *
 * This callback requests an application to close it's network handle.
 *
 * @param [in] 	close_data 		Pointer to a connector_close_request_t structure, which contains the network handle and reason of closing
 * @param [out] is_to_reconnect	Instructs the iDigi Connector what to do after close completes.  When set to connector_auto_connect,
 * 								the iDigi Connector will restart the UDP transport without exiting connector_run() or connector_step().  When
 * 								set to connector_manual_connect, connector_run() or connector_step() will terminate.
 *
 * @retval connector_callback_continue	The callback has successfully closed the network device.
 * @retval connector_callback_busy 		The network device is busy and cannot be closed.
 *
 * @see connector_callback_status_t
 * @see @ref close API Network Callback
 */
static connector_callback_status_t app_network_udp_close(connector_close_request_t const * const close_data, connector_auto_connect_type_t * const is_to_reconnect)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(close_data);
    UNUSED_ARGUMENT(is_to_reconnect);


    return status;
}


/**
 * @cond DEV
 */



/*
 *  Callback routine to handle all networking related calls.
 */
connector_callback_status_t app_network_udp_handler(connector_network_request_t const request,
                                            void const * const request_data, size_t const request_length,
                                            void * response_data, size_t * const response_length)
{
    connector_callback_status_t status;

    UNUSED_ARGUMENT(request_length);

    switch (request)
    {
    case connector_network_open:
        status = app_network_udp_open(request_data, request_length, response_data);
        *response_length = sizeof(connector_network_handle_t);
        break;

    case connector_network_send:
        status = app_network_udp_send(request_data, response_data);
        break;

    case connector_network_receive:
        status = app_network_udp_receive(request_data, response_data);
        break;

    case connector_network_close:
        status = app_network_udp_close(request_data, response_data);
        break;

    default:
        APP_DEBUG("app_network_udp_handler: unrecognized callback request [%d]\n", request);
        status = connector_callback_unrecognized;
        break;

    }

    return status;
}


/**
 * @endcond
 */
