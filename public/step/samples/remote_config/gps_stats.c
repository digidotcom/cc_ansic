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
#include "connector_config.h"
#include "connector_api.h"
#include "platform.h"
#include "remote_config_cb.h"

#define GPS_STATS_LOCATION_STRING_LENGTH 15

typedef struct {
    char latitude[GPS_STATS_LOCATION_STRING_LENGTH];
    char longitude[GPS_STATS_LOCATION_STRING_LENGTH];
} gps_location_t;

static gps_location_t gps_data = { "44.932017", "-93.461594"};

connector_callback_status_t app_gps_stats_group_get(connector_remote_group_request_t const * const request, connector_remote_group_response_t * const response)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request->element.id)
    {
    case connector_state_gps_stats_latitude:
        response->element_data.element_value->string_value = gps_data.latitude;
        break;
    case connector_state_gps_stats_longitude:
        response->element_data.element_value->string_value = gps_data.longitude;
        break;
    default:
        ASSERT(0);
        goto done;
    }

done:
    return status;
}

