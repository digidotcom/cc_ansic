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

#ifndef CHK_CONFIG_H_
#define CHK_CONFIG_H_

#if (defined CONNECTOR_TRANSPORT_TCP)

#if (defined CONNECTOR_VENDOR_ID)
#if (CONNECTOR_VENDOR_ID > CONNECTOR_MAX_VENDOR_ID_NUMBER) || (CONNECTOR_VENDOR_ID == 0)
    #error "Invalid CONNECTOR_VENDOR_ID in connector_config.h"
#endif
#endif

#if (defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS)
#if (CONNECTOR_TX_KEEPALIVE_IN_SECONDS < MIN_TX_KEEPALIVE_INTERVAL_IN_SECONDS) || (CONNECTOR_TX_KEEPALIVE_IN_SECONDS > MAX_TX_KEEPALIVE_INTERVAL_IN_SECONDS)
    #error "Invalid CONNECTOR_TX_KEEPALIVE_IN_SECONDS value in connector_config.h."
#endif
#endif

#if (defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
#if (CONNECTOR_RX_KEEPALIVE_IN_SECONDS < MIN_RX_KEEPALIVE_INTERVAL_IN_SECONDS) || (CONNECTOR_RX_KEEPALIVE_IN_SECONDS > MAX_RX_KEEPALIVE_INTERVAL_IN_SECONDS)
    #error "Invalid CONNECTOR_RX_KEEPALIVE_IN_SECONDS value in connector_config.h."
#endif
#endif

#if (defined CONNECTOR_WAIT_COUNT)
#if (CONNECTOR_WAIT_COUNT < WAIT_COUNT_MIN) || (CONNECTOR_WAIT_COUNT > WAIT_COUNT_MAX)
    #error "Invalid CONNECTOR_WAIT_COUNT value in connector_config.h."
#endif
#endif

#if (defined CONNECTOR_MSG_MAX_TRANSACTION)
#if (CONNECTOR_MSG_MAX_TRANSACTION < 0 || CONNECTOR_MSG_MAX_TRANSACTION > CONNECTOR_MAX_TRANSACTIONS_LIMIT)
    #error "Invalid CONNECTOR_MSG_MAX_TRANSACTION value in connector_config.h"
#endif
#endif

#endif

#if (defined CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH)
#if CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH > SIZE_MAX
    #error "Invalid CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH, it must be lower than SIZE_MAX"
#endif
#endif

#endif

#if (defined CONNECTOR_TRANSPORT_UDP)
#if CONNECTOR_SM_UDP_MAX_SESSIONS > CONNECTOR_SM_MAX_SESSIONS_LIMIT
    #error "CONNECTOR_SM_UDP_MAX_SESSIONS over Range"
#endif
#if CONNECTOR_SM_UDP_MAX_RX_SEGMENTS > CONNECTOR_SM_MAX_RX_SEGMENTS_LIMIT
    #error "CONNECTOR_SM_UDP_MAX_RX_SEGMENTS over Range"
#endif
#endif

#if (defined CONNECTOR_TRANSPORT_SMS)
#if CONNECTOR_SM_SMS_MAX_SESSIONS > CONNECTOR_SM_MAX_SESSIONS_LIMIT
    #error "CONNECTOR_SM_SMS_MAX_SESSIONS over Range"
#endif
#if CONNECTOR_SM_SMS_MAX_RX_SEGMENTS > CONNECTOR_SM_MAX_RX_SEGMENTS_LIMIT
    #error "CONNECTOR_SM_SMS_MAX_RX_SEGMENTS over Range"
#endif
#endif

#if (defined CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS) && (CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS > 1) && (!defined CONNECTOR_SM_MULTIPART)
    #error "You must define CONNECTOR_SM_MULTIPART in order to set CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS bigger than 1"
#endif

#if (defined CONNECTOR_DEVICE_HEALTH)
#if !(defined CONNECTOR_TRANSPORT_TCP)
    #error "You must define CONNECTOR_TRANSPORT_TCP in order to enable CONNECTOR_DEVICE_HEALTH"
#endif
#if !(defined CONNECTOR_DATA_SERVICE)
    #error "You must define CONNECTOR_DATA_SERVICE in order to enable CONNECTOR_DEVICE_HEALTH"
#endif
#if !(defined CONNECTOR_RCI_SERVICE)
    #error "You must define CONNECTOR_RCI_SERVICE in order to enable CONNECTOR_DEVICE_HEALTH"
#endif
#if (defined CONNECTOR_NO_MALLOC)
    #error "CONNECTOR_DEVICE_HEALTH are not available for CONNECTOR_NO_MALLOC version"
#endif
#if !(defined CONNECTOR_HAS_64_BIT_INTEGERS)
    #error "You must define CONNECTOR_HAS_64_BIT_INTEGERS in order to enable CONNECTOR_DEVICE_HEALTH"
#endif
#if !(defined FLOATING_POINT_SUPPORTED)
    #error "You must define FLOATING_POINT_SUPPORTED in order to enable CONNECTOR_DEVICE_HEALTH"
#endif
#endif
