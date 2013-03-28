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

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include "connector_api.h"
#include "platform.h"
#include "network_dns.h"

#if defined CONNECTOR_TRANSPORT_UDP

static connector_callback_status_t app_network_udp_close(connector_close_request_t const * const close_data,
                                                 connector_auto_connect_type_t * const is_to_reconnect)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_network_handle_t * const fd = close_data->network_handle;

    *is_to_reconnect = app_connector_reconnect(connector_class_id_network_udp, close_data->status);

    if (close(*fd) < 0)
    {
        APP_DEBUG("network_tcp_close: close() failed, errno %d\n", errno);
    }

    {
        int * user_fd = (int *)fd;
        *user_fd = -1;
    }

    return status;
}


/*
 * This routine reads a specified number of bytes from the iDigi Device Cloud.  This
 * function must not block. If it encounters EAGAIN  error, return
 * connector_callback_busy and iDigi connector will ignore the read_data and read_length
 * and continue calling this function.
 */
static connector_callback_status_t app_network_udp_receive(connector_read_request_t const * const read_data,
                                                       size_t * const read_length)
{
    connector_callback_status_t status = connector_callback_continue;
    int ccode;

    *read_length = 0;

    ccode = read(*read_data->network_handle, read_data->buffer, read_data->length);

    if (ccode < 0)
    {
        int const err = errno;
        /* An error of some sort occurred: handle it appropriately. */
        if (err == EAGAIN)
        {
            status = connector_callback_busy;
        }
        else
        {
            APP_DEBUG("app_network_udp_receive: recv() failed, errno %d\n", err);
            /* if not timeout (no data) return an error */
            app_dns_cache_invalidate(connector_class_id_network_udp);
            status = connector_callback_error;
        }
    }

    *read_length = (size_t)ccode;

    return status;
}

/*
 * Sends data to the iDigi Device Cloud, this routine must not block.  If it encounters
 * EAGAIN  error, return connector_callback_busy and iDigi connector will ignore the
 * sent_length and continue calling this function.
 */
static connector_callback_status_t app_network_udp_send(connector_write_request_t const * const write_data,
                                                size_t * const sent_length)
{
    connector_callback_status_t status = connector_callback_continue;

    int const bytes_sent = write(*write_data->network_handle, write_data->buffer, write_data->length);
    if (bytes_sent < 0)
    {
        int const err = errno;
        if (err == EAGAIN)
        {
            status = connector_callback_busy;
        }
        else
        {
            status = connector_callback_error;
            APP_DEBUG("app_network_udp_send: send() failed, errno %d\n", err);
            app_dns_cache_invalidate(connector_class_id_network_udp);
        }
    }
    *sent_length = (size_t)bytes_sent;

    return status;
}

static int  app_udp_create_socket(void)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd >= 0)
    {
        int enabled = 1;

        if (ioctl(fd, FIONBIO, &enabled) < 0)
        {
            APP_DEBUG("ioctl: FIONBIO failed, errno %d\n", errno);
            close(fd);
            fd = -1;
        }
    }
    else
    {
        APP_DEBUG("Could not open UDP socket, errno %d\n", errno);
    }

    return fd;
}

static connector_callback_status_t app_udp_connect(int const fd, in_addr_t const ip_addr)
{
    connector_callback_status_t status = connector_callback_continue;
    struct sockaddr_in sin = {0};

    memcpy(&sin.sin_addr, &ip_addr, sizeof sin.sin_addr);
    sin.sin_port   = htons(CONNECTOR_UDP_PORT);
    sin.sin_family = AF_INET;

    if (connect(fd, &sin, sizeof(sin)) < 0)
    {
        APP_DEBUG("network_udp_open: connect() failed, errno %d\n", errno);
        status = connector_callback_error;
    }
    return status;
}

static connector_callback_status_t app_network_udp_open(char const * const server_name,
                                                    size_t const length,
                                                    connector_network_handle_t ** network_handle)
{
    connector_callback_status_t status = connector_callback_continue;
    in_addr_t ip_addr;
    static int fd = -1;

    *network_handle = &fd;

    status = app_dns_resolve(connector_class_id_network_udp, server_name, length, &ip_addr);
    if (status != connector_callback_continue)
    {
        APP_DEBUG("app_network_udp_open: Can't resolve DNS for %s\n", server_name);
        goto done;
    }

    if (fd == -1)
    {
        fd = app_udp_create_socket();
        if (fd == -1)
        {
            status = connector_callback_error;
            goto done;
        }

        status = app_udp_connect(fd, ip_addr);
    }

    if ((status == connector_callback_error) && (fd >= 0))
    {
        close(fd);
        fd = -1;
    }

done:
    return status;
}


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
#endif

