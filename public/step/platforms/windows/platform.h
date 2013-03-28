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

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include <stdio.h>
#include "connector_debug.h"

void connector_debug_printf(char const * const format, ...);
#define APP_DEBUG   connector_debug_printf

#define UNUSED_ARGUMENT(x)     ((void)x)


extern connector_callback_status_t app_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id,
                                    void const * const request_data, size_t const request_length,
                                    void * response_data, size_t * const response_length);

/* Callbacks for this platform */
extern connector_callback_status_t app_os_handler(connector_os_request_t const request,
                           void const * const request_data, size_t const request_length,
                           void * response_data, size_t * const response_length);

extern connector_callback_status_t app_network_handler(connector_class_id_t const class_id,
                                                   connector_network_request_t const request,
                                                   void const * const request_data, size_t const request_length,
                                                   void * response_data, size_t * const response_length);

extern connector_callback_status_t app_config_handler(connector_config_request_t const request,
                                              void const * const request_data,
                                              size_t const request_length,
                                              void * response_data,
                                              size_t * const response_length);

extern connector_callback_status_t app_file_system_handler(connector_file_system_request_t const request,
                                                void const * const request_data,
                                                size_t const request_length,
                                                void * const response_data,
                                                size_t * const response_length);

extern int application_step(connector_handle_t handle);

extern connector_callback_status_t app_os_get_system_time(unsigned long * const uptime);
extern connector_callback_status_t app_os_yield(connector_status_t const * const status);
extern int app_get_edp_conn_ip_address(uint8_t const ** ip_address, size_t * const size);

#define APP_SSL_CA_CERT   "public/certificates/idigi-ca-cert-public.crt"
/* #define APP_SSL_CA_CERT   "../../../public/certificates/idigi-ca-cert-public.crt" */

#endif /* _PLATFORM_H */
