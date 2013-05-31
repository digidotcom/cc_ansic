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

#include "connector_api.h"

#if (defined CONNECTOR_DATA_POINTS)
#include "platform.h"
#include "os_support.h"
#include "data_point.h"
#include "connector_debug.h"

connector_callback_status_t process_data_point_response(connector_data_point_response_t * const resp_ptr)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_app_send_data_point_t * const data_point_info = (connector_app_send_data_point_t * const)resp_ptr->user_context;

    if (resp_ptr->hint != NULL)
    {
    	APP_DEBUG("process_data_point_response: Hint: %s\n", resp_ptr->hint);
    }
    
    ecc_set_event(ECC_DATA_POINTS_EVENT, data_point_info->event_bit);
    
    return status;
}

connector_callback_status_t process_data_point_status(connector_data_point_status_t * const status_ptr)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_app_send_data_point_t * const data_point_info = (connector_app_send_data_point_t * const)status_ptr->user_context;

    APP_DEBUG("Received data point error [%d]\n", status_ptr->status);
    ecc_set_event(ECC_DATA_POINTS_EVENT, data_point_info->event_bit);

    return status;
}

connector_callback_status_t app_data_point_handler(connector_request_id_data_point_t const request_id, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request_id)
    {
        case connector_request_id_data_point_single_response:
        {
            connector_data_point_response_t * const resp_ptr = data;

            status = process_data_point_response(resp_ptr);
            break;
        }

        case connector_request_id_data_point_single_status:
        {
            connector_data_point_status_t * const status_ptr = data;
            status = process_data_point_status(status_ptr);
            break;
        }

        default:
            APP_DEBUG("Data point callback: Request not supported: %d\n", request_id);
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
#endif /* defined CONNECTOR_DATA_POINTS */
