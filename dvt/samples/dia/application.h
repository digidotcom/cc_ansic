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
#ifndef __APPLICATION_H_
#define __APPLICATION_H_

typedef int app_bool_t;
#define app_true   1
#define app_false  0

#define APP_USE_SM_UDP

#if defined APP_USE_SM_TCP && !defined CONNECTOR_TRANSPORT_TCP
#error Please define CONNECTOR_TRANSPORT_TCP in connector_config.h
#endif

#if defined APP_USE_SM_UDP && !defined CONNECTOR_TRANSPORT_UDP
#error Please define CONNECTOR_TRANSPORT_UDP in connector_config.h
#endif

#if defined APP_USE_SM_SMS && !defined CONNECTOR_TRANSPORT_SMS
#error Please define CONNECTOR_TRANSPORT_SMS in connector_config.h
#endif

#if !defined APP_USE_SM_TCP && !defined APP_USE_SM_UDP && !defined APP_USE_SM_SMS
#error Please define APP_USE_SM_TCP or APP_USE_SM_UDP or APP_USE_SM_SMS
#endif

#if defined APP_USE_SM_TCP
#define APP_TRANSPORT connector_transport_tcp
#elif defined APP_USE_SM_UDP
#define APP_TRANSPORT connector_transport_udp
#elif defined APP_USE_SM_SMS
#define APP_TRANSPORT connector_transport_sms
#endif

extern connector_callback_status_t app_data_service_handler(connector_data_service_request_t const request, void * const data);
extern connector_callback_status_t app_sm_handler(connector_request_id_sm_t const request, void * const data);
extern connector_callback_status_t app_status_handler(connector_status_request_t const request, void * const data);

extern connector_status_t app_send_dia_data(connector_handle_t handle);
extern connector_status_t app_send_ping(connector_handle_t handle);

#endif
