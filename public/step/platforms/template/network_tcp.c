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

#include "connector_api.h"
#include "platform.h"

static connector_callback_status_t app_network_tcp_open(ichar const * const host_name, size_t const length, connector_network_handle_t ** network_handle)
{
    connector_callback_status_t rc = connector_callback_continue;

    UNUSED_ARGUMENT(host_name);
    UNUSED_ARGUMENT(length);
    UNUSED_ARGUMENT(network_handle);

    return rc;
}

static connector_callback_status_t app_network_tcp_send(connector_write_request_t const * const write_data, size_t * const sent_length)
{
    connector_callback_status_t rc = connector_callback_continue;

    UNUSED_ARGUMENT(write_data);
    UNUSED_ARGUMENT(sent_length);

    return rc;
}

static connector_callback_status_t app_network_tcp_receive(connector_read_request_t const * const read_data, size_t * const read_length)
{
    connector_callback_status_t rc = connector_callback_continue;

    UNUSED_ARGUMENT(read_data);
    UNUSED_ARGUMENT(read_length);
    return rc;
}

static connector_callback_status_t app_network_tcp_close(connector_close_request_t const * const close_data, connector_auto_connect_type_t * const is_to_reconnect)
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
connector_callback_status_t app_network_tcp_handler(connector_network_request_t const request,
                                            void const * const request_data, size_t const request_length,
                                            void * response_data, size_t * const response_length)
{
    connector_callback_status_t status;

    UNUSED_ARGUMENT(request_length);


    switch (request)
    {
    case connector_network_open:
        status = app_network_tcp_open(request_data, request_length, response_data);
        *response_length = sizeof(connector_network_handle_t);
        break;

    case connector_network_send:
        status = app_network_tcp_send(request_data, response_data);
        break;

    case connector_network_receive:
        status = app_network_tcp_receive(request_data, response_data);
        break;

    case connector_network_close:
        status = app_network_tcp_close(request_data, response_data);
        break;

    default:
        APP_DEBUG("app_network_tcp_handler: unrecognized callback request [%d]\n", request);
        status = connector_callback_unrecognized;
        break;

    }

    return status;
}



/**
 * @endcond
 */
