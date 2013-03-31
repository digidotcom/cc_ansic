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

#if defined CONNECTOR_TRANSPORT_TCP

/* Global structure of connected interface */
static struct sockaddr_in interface_addr;

connector_callback_status_t app_get_interface_ip_address(uint8_t ** ip_address, size_t * size)
{
    *size       = sizeof(interface_addr.sin_addr.s_addr);
    *ip_address = (uint8_t *)&interface_addr.sin_addr.s_addr;

    return connector_callback_continue;
}

static connector_callback_status_t app_network_tcp_close(connector_close_request_t const * const close_data,
                                                     connector_connect_auto_type_t * const is_to_reconnect)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_network_handle_t * const fd = close_data->network_handle;

#if 0
    {
        struct linger ling_opt;
        ling_opt.l_onoff = 1;
        ling_opt.l_linger = 1;

        if setsockopt(*fd, SOL_SOCKET, SO_LINGER, &ling_opt, sizeof(ling_opt) ) < 0)
        {
            APP_DEBUG("network_tcp_close: setsockopt fails: errno %d\n", errno);
        }
    }
#endif

    app_dns_set_redirected(connector_class_id_network_tcp, close_data->status == connector_close_status_server_redirected);

    *is_to_reconnect = app_connector_reconnect(connector_class_id_network_tcp, close_data->status);

    if (close(*fd) < 0)
    {
        APP_DEBUG("network_tcp_close: close() failed, fd %d, errno %d\n", *fd, errno);
    }
    else
        APP_DEBUG("network_tcp_close: fd %d\n", *fd);

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
static connector_callback_status_t app_network_tcp_receive(connector_read_request_t const * const read_data,
                                                       size_t * const read_length)
{
    connector_callback_status_t status = connector_callback_continue;
    int ccode;

    *read_length = 0;

    ccode = read(*read_data->network_handle, read_data->buffer, read_data->length);

    if (ccode == 0)
    {
        /* EOF on input: the connection was closed. */
        APP_DEBUG("network_receive: EOF on socket\n");
        errno = ECONNRESET;
        status = connector_callback_error;
        goto done;
    }

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
            APP_DEBUG("network_receive: recv() failed, errno %d\n", err);
            /* if not timeout (no data) return an error */
            app_dns_cache_invalidate(connector_class_id_network_tcp);
            status = connector_callback_error;
        }
    }

    *read_length = (size_t)ccode;

done:
    return status;
}

/*
 * Sends data to the iDigi Device Cloud, this routine must not block.  If it encounters
 * EAGAIN  error, return connector_callback_busy and iDigi connector will ignore the
 * sent_length and continue calling this function.
 */
static connector_callback_status_t app_network_tcp_send(connector_write_request_t const * const write_data,
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
            APP_DEBUG("app_network_tcp_send: send() failed, errno %d\n", err);
            app_dns_cache_invalidate(connector_class_id_network_tcp);
        }
    }
    *sent_length = (size_t)bytes_sent;

    return status;
}


static int app_tcp_create_socket(void)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd >= 0)
    {
        int enabled = 1;

        if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enabled, sizeof(enabled)) < 0)
        {
            APP_DEBUG("open_socket: setsockopt SO_KEEPALIVE failed, errno %d\n", errno);
        }

        if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enabled, sizeof(enabled)) < 0)
        {
            APP_DEBUG("open_socket: setsockopt TCP_NODELAY failed, errno %d\n", errno);
        }

        if (ioctl(fd, FIONBIO, &enabled) < 0)
        {
            APP_DEBUG("ioctl: FIONBIO failed, errno %d\n", errno);
            close(fd);
            fd = -1;
        }
    }
    else
    {
        APP_DEBUG("Could not open tcp socket, errno %d\n", errno);
    }

    return fd;
}

static connector_callback_status_t app_tcp_connect(int const fd, in_addr_t const ip_addr)
{

    struct sockaddr_in sin = {0};
    connector_callback_status_t status = connector_callback_continue;

    memcpy(&sin.sin_addr, &ip_addr, sizeof sin.sin_addr);
    sin.sin_port   = htons(CONNECTOR_PORT);
    sin.sin_family = AF_INET;

    APP_DEBUG("app_tcp_connect: fd %d\n", fd);

    if (connect(fd, &sin, sizeof(sin)) < 0)
    {
        int const err = errno;
        switch (err)
        {
        case EINTR:
        case EAGAIN:
        case EINPROGRESS:
            status = connector_callback_busy;
            break;

        default:
            APP_DEBUG("app_tcp_connect: connect() failed, fd %d, errno %d\n", fd, err);
            status = connector_callback_error;
        }
    }

    return status;
}

static connector_callback_status_t app_is_tcp_connect_complete(int const fd)
{
    connector_callback_status_t status = connector_callback_busy;
    struct timeval timeout = {0};
    fd_set read_set, write_set;
    int rc;

    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    write_set = read_set;

    rc = select(fd+1, &read_set, &write_set, NULL, &timeout);
    if (rc < 0)
    {
        if (errno != EINTR) {
            APP_DEBUG("app_is_tcp_connect_complete: select on fd %d returned %d, errno %d\n", fd, rc, errno);
            status = connector_callback_error;
        }
    }
    else
    /* Check whether the socket is now writable (connection succeeded). */
    if (rc > 0 && FD_ISSET(fd, &write_set))
    {
        /* We expect "socket writable" when the connection succeeds. */
        /* If we also got a "socket readable" we have an error. */
        if (FD_ISSET(fd, &read_set))
        {
            APP_DEBUG("app_is_tcp_connect_complete: FD_ISSET for read, fd %d\n", fd);
            status = connector_callback_error;

        }
        else
        {
            status = connector_callback_continue;
        }
    }
    return status;
}

static connector_callback_status_t app_network_tcp_open(char const * const server_name,
                                                    size_t const length,
                                                    connector_network_handle_t ** network_handle)
{
#define APP_CONNECT_TIMEOUT 30

    static unsigned long connect_time;
    static int fd = -1;
    socklen_t interface_addr_len;

    connector_callback_status_t status = connector_callback_error;

    *network_handle = &fd;

    if (fd == -1)
    {
        in_addr_t ip_addr;

        status = app_dns_resolve(connector_class_id_network_tcp, server_name, length, &ip_addr);
        if (status != connector_callback_continue)
        {
            APP_DEBUG("app_network_tcp_open: Can't resolve DNS for %s\n", server_name);
            goto done;
        }

        fd = app_tcp_create_socket();
        if (fd == -1)
        {
            status = connector_callback_error;
            goto done;
        }

        app_os_get_system_time(&connect_time);
        status = app_tcp_connect(fd, ip_addr);
        if (status != connector_callback_continue)
            goto error;
    }

    /* Get socket info of connected interface */
    interface_addr_len = sizeof(interface_addr);
    if (getsockname(fd, &interface_addr, &interface_addr_len))
    {
        APP_DEBUG("network_connect: getsockname error, errno %d\n", errno);
        goto done;
    }

    status = app_is_tcp_connect_complete(fd);
    if (status == connector_callback_continue)
    {
         APP_DEBUG("app_network_tcp_open: connected to %s\n", server_name);
         goto done;
    }

    if (status == connector_callback_busy)
    {
        unsigned long elapsed_time;

        app_os_get_system_time(&elapsed_time);
        elapsed_time -= connect_time;

        if (elapsed_time >= APP_CONNECT_TIMEOUT)
        {
            APP_DEBUG("app_network_tcp_open: failed to connect withing 30 seconds\n");
            status = connector_callback_error;
        }
    }

error:
    if (status == connector_callback_error)
    {
        APP_DEBUG("app_network_tcp_open: failed to connect to %s\n", server_name);
        app_dns_set_redirected(connector_class_id_network_tcp, 0);

        if (fd >= 0)
        {
            close(fd);
            fd = -1;
        }
    }

done:
    return status;
}


/*
 *  Callback routine to handle all networking related calls.
 */
connector_callback_status_t app_network_tcp_handler(connector_request_id_network_t const request,
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
#endif

