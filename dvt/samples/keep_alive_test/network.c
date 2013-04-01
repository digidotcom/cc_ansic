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

#define MT_TYPE_OFFSET      0
#define MT_TYPE_KEEP_ALIVE  0x0030

int dvt_send_keepalive_count = 0;
int dvt_fw_keepalive_count = 0;
int dvt_fw_complete_called = 0;
int start_put_request = 0;


static int app_dns_resolve_name(char const * const domain_name, in_addr_t * const ip_addr)
{
    int ret = -1;
    struct addrinfo *res_list;
    struct addrinfo *res;

    if ((domain_name == NULL) || (ip_addr == NULL))
        goto done;

    {
        struct addrinfo hint = {0};
        int error;

        hint.ai_socktype = SOCK_STREAM;
        hint.ai_family   = AF_INET;
        error = getaddrinfo(domain_name, NULL, &hint, &res_list);
        if (error != 0)
        {
            APP_DEBUG("dns_resolve_name: DNS resolution failed for [%s]\n", domain_name);
            goto done;
        }
    }

    /* loop over all returned results and look for a IPv4 address */
    for (res = res_list; res; res = res->ai_next)
    {
        if (res->ai_family == PF_INET)
        {
            struct sockaddr_in * const sa = cast_for_alignment(struct sockaddr_in *, res->ai_addr);
            struct in_addr const ipv4_addr = sa->sin_addr;

            *ip_addr = ipv4_addr.s_addr;
            APP_DEBUG("dns_resolve_name: ip address = [%s]\n", inet_ntoa(ipv4_addr));
            ret = 0;
            break;
        }
    }

    freeaddrinfo(res_list);

done:
    return ret;
}

static connector_callback_status_t app_network_connect(char const * const host_name, size_t const length, connector_network_handle_t ** network_handle)
{
    connector_callback_status_t rc = connector_callback_error;
    static int fd = -1;

    if (fd == -1)
    {
        in_addr_t ip_addr;

        {
            char server_name[64];

            if (length >= asizeof(server_name))
            {
                APP_DEBUG("app_connect_to_server: server name length [%zu]\n", length);
                goto done;
            }

            memcpy(server_name, host_name, length);
            server_name[length] = '\0';

            /*
             * Check if it's a dotted-notation IP address, if it's a domain name,
             * attempt to resolve it.
             */
            ip_addr = inet_addr(server_name);
            if (ip_addr == INADDR_NONE)
            {
                if (app_dns_resolve_name(server_name, &ip_addr) != 0)
                {
                    APP_DEBUG("network_connect: Can't resolve DNS for %s\n", server_name);
                    goto done;
                }
            }
        }

        fd = socket(AF_INET, SOCK_STREAM, 0);
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
        }
        else
        {
            APP_DEBUG("Could not open socket, errno %d\n", errno);
            goto done;
        }

        {
            int opt=1;

            if (ioctl(fd, FIONBIO, &opt) < 0)
            {
                APP_DEBUG("ioctl: FIONBIO failed, errno %d\n", errno);
                goto done;
            }
        }

        {
            struct sockaddr_in sin = {0};

            memcpy(&sin.sin_addr, &ip_addr, sizeof sin.sin_addr);
            sin.sin_port   = htons(CONNECTOR_PORT);
            sin.sin_family = AF_INET;

            if (connect(fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            {
                int err = errno;
                switch (err)
                {
                case EAGAIN:
                case EINPROGRESS:
                    break;
                default:
                    APP_DEBUG("network_connect: connect() failed, errno %d\n", err);
                    goto done;
                }
            }
        }
    }

    {
        struct timeval timeout = {0};
        fd_set read_set;
        fd_set write_set;
        int ccode;

        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);
        write_set = read_set;

        timeout.tv_sec = 30;
        ccode = select(fd+1, &read_set, &write_set, NULL, &timeout);
        if (ccode <= 0)
        {
            APP_DEBUG("network_connect: select returned %d\n", ccode);
            goto done;
        }

        /* Check whether the socket is now writable (connection succeeded). */
        if (FD_ISSET(fd, &write_set))
        {
            /* We expect "socket writable" when the connection succeeds. */
            /* If we also got a "socket readable" we have an error. */
            if (FD_ISSET(fd, &read_set))
            {
                APP_DEBUG("network_connect: error to connect to %.*s server\n", (int)length, host_name);
                goto done;
            }
            *network_handle = &fd;
            rc = connector_callback_continue;
            APP_DEBUG("network_connect: connected to [%.*s] server\n", (int)length, host_name);
        }
    }

done:
    if ((rc == connector_callback_error) && (fd >= 0))
    {
        close(fd);
        fd = -1;
    }

    return rc;
}

static connector_callback_status_t app_network_send(connector_write_request_t const * const write_data,
                                            size_t * const sent_length)
{
    connector_callback_status_t rc = connector_callback_continue;
    int ccode;

    if ((dvt_fw_complete_called) && (write_data->length > 2))
    {
        #define MT_TYPE_PAYLOAD     0x0040
        #define FACILITY_OFFSET     6
        #define FW_FACILITY_CODE    0x0070
        #define TARGET_LIST_OFFSET  8
        #define TARGET_LIST_OPCODE  0
        unsigned char const * buffer = write_data->buffer;
        unsigned short const mt_type = (buffer[MT_TYPE_OFFSET] << 8) + buffer[MT_TYPE_OFFSET + 1];

        if (mt_type == MT_TYPE_KEEP_ALIVE)
            dvt_send_keepalive_count++;
        else if ((mt_type == MT_TYPE_PAYLOAD) && (write_data->length > TARGET_LIST_OFFSET))
        {
            unsigned short const fac_code = (buffer[FACILITY_OFFSET] << 8) + buffer[FACILITY_OFFSET + 1];

            if ((fac_code == FW_FACILITY_CODE) && (buffer[TARGET_LIST_OFFSET] == TARGET_LIST_OPCODE))
                dvt_fw_keepalive_count++;
        }
    }

    ccode = send(*write_data->network_handle, write_data->buffer,write_data->length, 0);
    if (ccode < 0)
    {
        int err = errno;
        if (err == EAGAIN)
        {
            rc = connector_callback_busy;
        }
        else
        {
            rc = connector_callback_error;
            APP_DEBUG("network_send: send() failed, err %d\n", err);
        }
    }
    *sent_length = ccode;

    return rc;
}

static connector_callback_status_t app_network_receive(connector_read_request_t const * const read_data, size_t * const read_length)
{
    connector_callback_status_t rc = connector_callback_continue;
    struct timeval timeout;
    fd_set read_set;
    int ccode, err;

    timeout.tv_sec = read_data->timeout;
    timeout.tv_usec = 0;

    *read_length = 0;

    FD_ZERO(&read_set);
    FD_SET(*read_data->network_handle, &read_set);

    /* Blocking point for iDigi Connector */
    ccode = select(*read_data->network_handle+1, &read_set, NULL, NULL, &timeout);
    if (ccode < 0)
    {
        APP_DEBUG("app_network_receive: select returned %d\n", ccode);
        goto done;
    }

    if (ccode == 0)
    {
        rc = connector_callback_busy;
        goto done;
    }
    ccode = recv(*read_data->network_handle, read_data->buffer, read_data->length, 0);

    if (ccode == 0)
    {
        /* EOF on input: the connection was closed. */
        APP_DEBUG("network_receive: EOF on socket\r\n");
        errno = ECONNRESET;
        rc = connector_callback_error;
    }
    else if (ccode < 0)
    {
        /* An error of some sort occurred: handle it appropriately. */
        err = errno;
        if (err == EAGAIN)
        {
            rc = connector_callback_busy;
        }
        else
        {
            APP_DEBUG("network_receive: recv() failed, errno %d\n", err);
            /* if not timeout (no data) return an error */
            rc = connector_callback_error;
        }
    }
    else
    {
        if ((dvt_fw_complete_called) && (ccode >= 2))
        {
            unsigned char * buffer = read_data->buffer;
            unsigned short const mt_type = (buffer[MT_TYPE_OFFSET] << 8) + buffer[MT_TYPE_OFFSET + 1];

            if (mt_type == MT_TYPE_KEEP_ALIVE)
            {
                APP_DEBUG("Discarding tx keepalive receive packet\n");
                ccode = recv(*read_data->network_handle, buffer, read_data->length, 0);
                goto done;
            }
        }
    }

    *read_length = (size_t)ccode;
done:

    return rc;
}

static connector_callback_status_t app_network_close(connector_close_request_t const * const close_data,
                                                 connector_connect_auto_type_t * const is_to_reconnect)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_network_handle_t * const fd = close_data->network_handle;
#if 0
    struct linger ling_opt;
    ling_opt.l_onoff = 1;
    ling_opt.l_linger = 1;

    if (setsockopt(*fd, SOL_SOCKET, SO_LINGER, &ling_opt, sizeof(ling_opt) ) < 0)
    {
        APP_DEBUG("network_tcp_close: setsockopt fails\n");
        if (errno == EAGAIN)
        {
            status = connector_callback_busy;
            goto done;
        }
    }
#endif
    if (close(*fd) < 0)
    {
        APP_DEBUG("network_tcp_close: close() failed, errno %d\n", errno);
    }

    {
        int * user_fd = (int *)fd;
        *user_fd = -1;
    }
    if (close_data->status == connector_close_status_no_keepalive)
    {
        start_put_request = 1;
        dvt_fw_complete_called = 0;
    }

    *is_to_reconnect = app_connector_reconnect(connector_class_id_network_tcp, close_data->status);

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
        status = app_network_close(request_data, response_data);
        break;

    default:
        APP_DEBUG("app_network_tcp_handler: unrecognized callback request [%d]\n", request);
        status = connector_callback_unrecognized;
        break;

    }

    return status;
}

