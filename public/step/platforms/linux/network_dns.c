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

#if defined (CONNECTOR_TRANSPORT_TCP) || (defined CONNECTOR_TRANSPORT_UDP)

#define APP_DNS_CACHE_TIMEOUT   (24 * 3600)

typedef struct
{
    char name[64];
    in_addr_t ip_addr;
    unsigned long sys_time;
    int is_redirected;
} app_dns_cache_t;

static app_dns_cache_t app_dns_cache = {"", INADDR_NONE, 0, 0};
#define app_dns_is_redirected(class_id) ((class_id) == connector_class_network_udp ? 0 : app_dns_cache.is_redirected)

static int app_dns_cache_is_valid(connector_class_t const class_id,
                                   char const * const server_name,
                                   size_t const length,
                                   in_addr_t * const ip_addr)
{
    int valid = 0;

    if (!app_dns_is_redirected(class_id))
    {
        if ((app_dns_cache.ip_addr != INADDR_NONE) && (memcmp(app_dns_cache.name, server_name, length) == 0))
        {
            unsigned long elapsed_time;

            app_os_get_system_time(&elapsed_time);
            if ((elapsed_time - app_dns_cache.sys_time) < APP_DNS_CACHE_TIMEOUT)
            {
                *ip_addr = app_dns_cache.ip_addr;
                valid = 1;
            }
        }
    }

    return valid;
}

static void app_dns_cache_update(connector_class_t const class_id,
                                 char const * const server_name,
                                 size_t const length,
                                 in_addr_t const ip_addr)
{
    if (!app_dns_is_redirected(class_id))
    {
        memcpy(app_dns_cache.name, server_name, length);
        app_dns_cache.ip_addr = ip_addr;
        app_os_get_system_time(&app_dns_cache.sys_time);
    }
}

static int app_dns_resolve_name(char const * const domain_name, in_addr_t * const ip_addr)
{
    int ret = -1;
    struct addrinfo *res_list;
    struct addrinfo *res;

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


void app_dns_set_redirected(connector_class_t const class_id, int const state)
{
    UNUSED_ARGUMENT(class_id);;
    app_dns_cache.is_redirected = state;
}

void app_dns_cache_invalidate(connector_class_t const class_id)
{
    if (!app_dns_is_redirected(class_id))
    {
        app_dns_cache.ip_addr = INADDR_NONE;
    }
}

connector_callback_status_t app_dns_resolve(connector_class_t const class_id,
                                       char const * const domain_name,
                                       size_t const length,
                                       in_addr_t * const ip_addr)
{
    char server_name[64];
    connector_callback_status_t status = connector_callback_error;

    if ((domain_name == NULL) || (ip_addr == NULL))
    {
        status = connector_callback_abort;
        goto done;
    }

    if (length >= asizeof(server_name))
    {
        APP_DEBUG("app_dns_resolve: server name length [%zu]\n", length);
        status = connector_callback_abort;
        goto done;
    }

    memcpy(server_name, domain_name, length);
    server_name[length] = '\0';

    *ip_addr = inet_addr(server_name);

    if (*ip_addr == INADDR_NONE)
    {
        if (!app_dns_cache_is_valid(class_id, server_name, length + 1, ip_addr))
        {
            if (app_dns_resolve_name(server_name, ip_addr) == 0)
            {
                app_dns_cache_update(class_id, server_name, length, *ip_addr);
            }
            else
            {
                APP_DEBUG("app_dns_resolve: Can't resolve DNS for %s\n", server_name);
                goto done;
            }
        }
    }
    status = connector_callback_continue;

done:
    return status;
}
#endif

