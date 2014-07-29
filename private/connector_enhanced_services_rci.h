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

STATIC connector_callback_status_t enhs_rci_set(enhs_metrics_t * const metrics_item, connector_remote_config_t const * const remote_config)
{
    connector_callback_status_t const status = connector_callback_continue;

    switch (remote_config->element.id)
    {
        case connector_setting_enhanced_services_path:
        {
            strncpy(metrics_item->path, remote_config->element.value->string_value, sizeof metrics_item->path);
            break;
        }
        case connector_setting_enhanced_services_sampling_interval:
        {
            metrics_item->sampling_interval = remote_config->element.value->unsigned_integer_value;
            metrics_item->sample_at = 0;
            break;
        }
        case connector_setting_enhanced_services_reporting_interval:
        {
            metrics_item->reporting_interval = remote_config->element.value->unsigned_integer_value;
            metrics_item->report_at = 0;
            break;
        }
        default:
        {
            ASSERT(remote_config->element.id < connector_setting_enhanced_services_COUNT);
            break;
        }
    }

    return status;
}

STATIC connector_callback_status_t enhs_rci_query(enhs_metrics_t const * const metrics_item, connector_remote_config_t * const remote_config)
{
    connector_callback_status_t const status = connector_callback_continue;

    switch (remote_config->element.id)
    {
        case connector_setting_enhanced_services_path:
        {
            remote_config->response.element_value->string_value = metrics_item->path;
            break;
        }
        case connector_setting_enhanced_services_sampling_interval:
        {
            remote_config->response.element_value->unsigned_integer_value = metrics_item->sampling_interval;
            break;
        }
        case connector_setting_enhanced_services_reporting_interval:
        {
            remote_config->response.element_value->unsigned_integer_value = metrics_item->reporting_interval;
            break;
        }
        default:
        {
            ASSERT(remote_config->element.id <= connector_setting_enhanced_services_COUNT);
            break;
        }
    }

    return status;
}

STATIC connector_callback_status_t enhs_rci_handler(connector_data_t * const connector_ptr, connector_request_id_remote_config_t const request_id, void * const data)
{
    connector_callback_status_t const status = connector_callback_continue;

    switch (request_id)
    {
        case connector_request_id_remote_config_session_start:
        case connector_request_id_remote_config_session_end:
        case connector_request_id_remote_config_group_start:
        case connector_request_id_remote_config_group_end:
        case connector_request_id_remote_config_action_start:
        case connector_request_id_remote_config_action_end:
        case connector_request_id_remote_config_session_cancel:
        case connector_request_id_remote_config_configurations:
            break;

        case connector_request_id_remote_config_group_process:
        {
            connector_remote_config_t * const remote_config = data;
            unsigned int const group_index = remote_config->group.index - 1;
            enhs_metrics_t * const metrics_item = &connector_ptr->enhs.metrics[group_index];

            switch (remote_config->action)
            {
                case connector_remote_action_set:
                {
                    enhs_rci_set(metrics_item, remote_config);
                    break;
                }
                case connector_remote_action_query:
                {
                    enhs_rci_query(metrics_item, remote_config);
                    break;
                }
            }
            break;
        }
    }

    return status;
}
