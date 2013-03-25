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
#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <ipcfg.h>
#include <select.h>
#include <errno.h>
#include <connector_api.h>
#include <connector_types.h>
#include <platform.h>
#include <connector_debug.h>
#include <os_support.h>

int connector_network_receive_failures = 0;
int connector_network_send_failures = 0;
int connector_connect_to_connector_successes = 0;
int connector_connect_to_connector_failures = 0;

static int socket_fd = RTCS_SOCKET_ERROR;

static boolean dns_resolve_name(char const * const name, _ip_address * const ip_addr)
{
    boolean result = FALSE;
    size_t tries = 3;

    APP_DEBUG("dns_resolve_name: DNS Address : %d.%d.%d.%d\n",
        IPBYTES(ipcfg_get_dns_ip(IPCFG_default_enet_device, 0)));

    #define ONE_SECOND_DELAY    1000
    /* Try three times to get name */
    do
    {
        result = RTCS_resolve_ip_address((char_ptr)name, ip_addr, NULL, 0);

        if (result) break;

        APP_DEBUG("Failed - name not resolved\n");
        _time_delay(ONE_SECOND_DELAY);

    } while (--tries > 0);

    return result;
}

static boolean set_socket_options(int const fd)
{
    #define SOCKET_BUFFER_SIZE 512
    #define SOCKET_TIMEOUT_MSEC 1000
    boolean success = FALSE;
    uint_32 option = TRUE;

    if(setsockopt(fd, SOL_TCP, OPT_RECEIVE_NOWAIT, &option, sizeof option) != RTCS_OK)
    {
        APP_DEBUG("set_non_blocking_socket: setsockopt OPT_RECEIVE_NOWAIT failed\n");
        goto error;
    }

    /* Reduce buffer size of socket to save memory */
    option = SOCKET_BUFFER_SIZE;
    if (setsockopt(socket_fd, SOL_TCP, OPT_TBSIZE, &option, sizeof option) != RTCS_OK)
    {
        APP_DEBUG("network_connect: setsockopt OPT_TBSIZE failed\n");
        goto error;
    }

    if (setsockopt(socket_fd, SOL_TCP, OPT_RBSIZE, &option, sizeof option) != RTCS_OK)
    {
        APP_DEBUG("network_connect: setsockopt OPT_RBSIZE failed\n");
        goto error;
    }

    /* set a socket timeout */
    option = SOCKET_TIMEOUT_MSEC;
    if (setsockopt(socket_fd, SOL_TCP, OPT_TIMEWAIT_TIMEOUT, &option, sizeof option) != RTCS_OK)
    {
        APP_DEBUG("network_connect: setsockopt OPT_TIMEWAIT_TIMEOUT failed\n");
        goto error;
    }

    success = TRUE;

error:
    return success;
}

static connector_callback_status_t app_network_connect(char const * const host_name, size_t const length, connector_network_handle_t ** network_handle)
{
    connector_callback_status_t status = connector_callback_abort;
    sockaddr_in addr;

    if (socket_fd == RTCS_SOCKET_ERROR)
    {
        _ip_address ip_addr;

        if (!dns_resolve_name(host_name, &ip_addr))
            goto error;

        APP_DEBUG("network_connect: Resolved IP Address: %d.%d.%d.%d\n", IPBYTES(ip_addr));

        socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd == RTCS_SOCKET_ERROR)
        {
            APP_DEBUG("Could not open socket\n");
            goto done;
        }

        set_socket_options(socket_fd);

        {
            struct timeval timeout = {0};
            fd_set read_set;
            fd_set write_set;
            struct sockaddr_in sin = {0};
            uint_32 result;

            /* Allow binding to any address */
            addr.sin_family      = AF_INET;
            addr.sin_port        = 0;
            addr.sin_addr.s_addr = INADDR_ANY;

            if (bind(socket_fd, &addr, sizeof addr) != RTCS_OK)
            {
                APP_DEBUG("Error in binding socket %08x\n", errno);
                goto error;
            }

            APP_DEBUG("Connecting to %s...\n", host_name);

            /* Connect to device */
            addr.sin_port        = CONNECTOR_PORT;
            addr.sin_addr.s_addr = ip_addr;
            result = connect(socket_fd, &addr, sizeof addr);
            if (result != RTCS_OK)
            {
                connector_connect_to_connector_failures ++;
                APP_DEBUG("network_connect: connect() failed %lx\n", result);
                goto error;
            }

            FD_ZERO(&read_set);
            FD_SET(socket_fd, &read_set);
            write_set = read_set;
            timeout.tv_sec = 30;
            
            APP_DEBUG("network_connect: calling select\n");

            if (select(socket_fd+1, &read_set, &write_set, NULL, &timeout) <= 0)
            {
                APP_DEBUG("network_connect: error select failed\n");
                goto error;
            }
            
            APP_DEBUG("network_connect: select returned\n");

            /* Check whether the socket is now writable (connection succeeded). */
            if (FD_ISSET(socket_fd, &write_set))
            {
                /* We expect "socket writable" when the connection succeeds. */
                /* If we also got a "socket readable" we have an error. */
                if (FD_ISSET(socket_fd, &read_set))
                {
                    APP_DEBUG("network_connect: error socket is readable\n");
                    goto error;
                }
            }
            else
            {
                APP_DEBUG("network_connect: error socket is not writable\n");
                goto error;
            }
        }
    }

    connector_connect_to_connector_successes ++;
    *network_handle = &socket_fd;
    status = connector_callback_continue;
    APP_DEBUG("network_connect: 3 way handshake, connected to [%.*s] server\n", (int)length, host_name);
    goto done;

error:
    shutdown(socket_fd, FLAG_ABORT_CONNECTION);
    socket_fd = RTCS_SOCKET_ERROR;

done:
    return status;
}

/*
 * Send data to the iDigi Device Cloud, this routine must not block.  If it encounters
 * EAGAIN or EWOULDBLOCK error, 0 bytes must be returned and iDigi Connector will continue
 * calling this function.
 */
static connector_callback_status_t app_network_send(connector_write_request_t const * const write_data,
                                                size_t * const sent_length)
{
    connector_callback_status_t status = connector_callback_continue;
    uint32_t bytes_sent;

    bytes_sent = send(*write_data->network_handle, (char _PTR_)write_data->buffer, write_data->length, 0);
    if (bytes_sent == RTCS_ERROR)
    {
        status = connector_callback_abort;
        APP_DEBUG("network_send: send() failed RTCS error [%lx]\n", RTCS_geterror(*write_data->network_handle));
        shutdown(socket_fd, FLAG_ABORT_CONNECTION);
        socket_fd = RTCS_SOCKET_ERROR;
        connector_network_send_failures ++;
    }
    else
    {
        *sent_length = bytes_sent;
        if (bytes_sent == 0)
            status = connector_callback_busy;
    }

    return status;
}

/*
 * This routine reads a specified number of bytes from the iDigi Device Cloud.  This
 * function must not block. If it encounters EAGAIN or EWOULDBLOCK error, 0
 * bytes must be returned and iDigi Connector will continue calling this function.
 */
static connector_callback_status_t app_network_receive(connector_read_request_t const * const read_data, size_t * const read_length)
{
    connector_callback_status_t status = connector_callback_continue;
    uint_32 bytes_read;

    *read_length = 0;
    bytes_read = recv(*read_data->network_handle, (char *)read_data->buffer, (int)read_data->length, 0);
    if (bytes_read == RTCS_ERROR)
    {
        APP_DEBUG("network_receive: Error, recv() failed RTCS error [%lx]\n", RTCS_geterror(*read_data->network_handle));
        shutdown(socket_fd, FLAG_ABORT_CONNECTION);
        socket_fd = RTCS_SOCKET_ERROR;
        status = connector_callback_abort;
        connector_network_receive_failures ++;
    }
    else
    {
        *read_length = bytes_read;
        if (bytes_read == 0)
            status = connector_callback_busy;
    }

    if (*read_length == 0)
        _time_delay(1);

    return status;
}

static connector_callback_status_t app_network_close(connector_network_handle_t * const fd)
{
    uint_32 result;
    
    ASSERT(*fd == socket_fd);

    // Note: this does a graceful close - like linger
    result = shutdown(*fd, FLAG_CLOSE_TX);
    if (result != RTCS_OK)
    {
        APP_DEBUG("network_close: failed, code = %lx\n", result);
    }

    socket_fd = RTCS_SOCKET_ERROR;

error:
    return connector_callback_continue;
}

static connector_callback_status_t app_server_disconnected(void)
{
    APP_DEBUG("Disconnected from server\n");
    /* if connector_run or connector_step is called again,
    * it will reconnect to the iDigi Device Cloud.
    */
    return connector_callback_continue;
}

static connector_callback_status_t app_server_reboot(void)
{
    APP_DEBUG("Reboot from server\n");

    /* should not return from rebooting the system */
    ic_software_reset();

    return connector_callback_continue;
}

/*
 *  Callback routine to handle all networking related calls.
 */
connector_callback_status_t app_network_handler(connector_network_request_t const request,
                                            void const * const request_data, size_t const request_length,
                                            void * const response_data, size_t * const response_length)
{
    connector_callback_status_t status;

    UNUSED_ARGUMENT(request_length);

    switch (request)
    {
        case connector_network_open:
            status = app_network_connect(request_data, request_length, response_data);
            *response_length = sizeof(connector_network_handle_t);
            break;

        case connector_network_send:
            status = app_network_send(request_data, response_data);
            break;

        case connector_network_receive:
            status = app_network_receive(request_data, response_data);
            break;

        case connector_network_close:
            status = app_network_close((connector_network_handle_t * const)request_data);
            break;

        case connector_network_disconnected:
            status = app_server_disconnected();
            break;

        case connector_network_reboot:
            status = app_server_reboot();
            break;

        default:
            APP_DEBUG("app_network_handler: unrecognized callback request [%d]\n", request);
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
