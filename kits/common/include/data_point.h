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

#ifndef DATA_SERVICE_H
#define DATA_SERVICE_H

#include "connector_config.h"
#include "connector_api_data_point.h"

connector_callback_status_t app_data_point_handler(connector_request_id_data_point_t const request, void  * const data);
#endif



