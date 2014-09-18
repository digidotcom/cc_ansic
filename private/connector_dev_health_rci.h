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

STATIC connector_callback_status_t enhs_rci_set(dev_health_metrics_config_t * const metrics_item, connector_remote_config_t const * const remote_config)
{
    connector_callback_status_t const status = connector_callback_continue;

    switch (remote_config->element.id)
    {
        case connector_setting_enhanced_services_path:
        {
            strncpy(metrics_item->path, remote_config->element.value->string_value, sizeof metrics_item->path);
            metrics_item->path[sizeof metrics_item->path - 1] = '\0';
            break;
        }
        case connector_setting_enhanced_services_sampling_interval:
        {
            metrics_item->sampling_interval = remote_config->element.value->unsigned_integer_value;
            break;
        }
        case connector_setting_enhanced_services_reporting_interval:
        {
            metrics_item->reporting_interval = remote_config->element.value->unsigned_integer_value;
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

STATIC connector_callback_status_t enhs_rci_query(dev_health_metrics_config_t const * const metrics_item, connector_remote_config_t * const remote_config)
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

STATIC connector_callback_status_t simple_enhs_rci_set(dev_health_simple_metric_t * const simple_metrics, connector_remote_config_t const * const remote_config)
{
    connector_callback_status_t const status = connector_callback_continue;

    switch (remote_config->element.id)
    {
        case connector_setting_simple_enhanced_services_eth_on:
        case connector_setting_simple_enhanced_services_mobile_on:
        /*case connector_setting_simple_enhanced_services_wifi_on:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_on:
        {
            simple_metrics->on = remote_config->element.value->on_off_value;
            break;
        }
        case connector_setting_simple_enhanced_services_eth_sampling:
        case connector_setting_simple_enhanced_services_mobile_sampling:
        /*case connector_setting_simple_enhanced_services_wifi_sampling:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_sampling:
        {
            simple_metrics->sampling_interval = remote_config->element.value->unsigned_integer_value;
            break;
        }
        case connector_setting_simple_enhanced_services_eth_reporting:
        case connector_setting_simple_enhanced_services_mobile_reporting:
        /*case connector_setting_simple_enhanced_services_wifi_reporting:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_reporting:
        {
            simple_metrics->reporting_interval = remote_config->element.value->unsigned_integer_value;
            break;
        }
        default:
        {
            ASSERT(remote_config->element.id < connector_setting_simple_enhanced_services_COUNT);
            break;
        }
    }

    return status;
}

STATIC connector_callback_status_t simple_enhs_rci_query(dev_health_simple_metric_t const * const simple_metrics, connector_remote_config_t * const remote_config)
{
    connector_callback_status_t const status = connector_callback_continue;

    switch (remote_config->element.id)
    {
        case connector_setting_simple_enhanced_services_eth_on:
        case connector_setting_simple_enhanced_services_mobile_on:
        /*case connector_setting_simple_enhanced_services_wifi_on:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_on:
        {
            remote_config->response.element_value->on_off_value = simple_metrics->on ? connector_on : connector_off;
            break;
        }
        case connector_setting_simple_enhanced_services_eth_sampling:
        case connector_setting_simple_enhanced_services_mobile_sampling:
        /*case connector_setting_simple_enhanced_services_wifi_sampling:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_sampling:
        {
            remote_config->response.element_value->unsigned_integer_value = simple_metrics->sampling_interval;
            break;
        }
        case connector_setting_simple_enhanced_services_eth_reporting:
        case connector_setting_simple_enhanced_services_mobile_reporting:
        /*case connector_setting_simple_enhanced_services_wifi_reporting:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_reporting:
        {
            remote_config->response.element_value->unsigned_integer_value = simple_metrics->reporting_interval;
            break;
        }
        default:
        {
            ASSERT(remote_config->element.id < connector_setting_simple_enhanced_services_COUNT);
            break;
        }
    }

    return status;
}

STATIC connector_callback_status_t enhs_rci_handler(connector_data_t * const connector_ptr, connector_request_id_remote_config_t const request_id, void * const data)
{
    connector_remote_config_t * const remote_config = data;
    connector_callback_status_t status = connector_callback_continue;

    switch (request_id)
    {
        case connector_request_id_remote_config_session_start:
        case connector_request_id_remote_config_session_end:
        case connector_request_id_remote_config_group_start:
        case connector_request_id_remote_config_group_end:
        case connector_request_id_remote_config_action_start:
        case connector_request_id_remote_config_session_cancel:
        case connector_request_id_remote_config_configurations:
            break;

        case connector_request_id_remote_config_group_process:
        {
            switch (remote_config->group.id)
            {
                case connector_setting_enhanced_services:
                {
                    unsigned int const group_index = remote_config->group.index - 1;
                    dev_health_metrics_config_t * const metrics_config_item = &connector_ptr->dev_health.metrics.config[group_index];

                    switch (remote_config->action)
                    {
                        case connector_remote_action_set:
                        {
                            enhs_rci_set(metrics_config_item, remote_config);
                            break;
                        }
                        case connector_remote_action_query:
                        {
                            enhs_rci_query(metrics_config_item, remote_config);
                            break;
                        }
                    }
                    break;
                }
                case connector_setting_simple_enhanced_services:
                {
                    dev_health_simple_metric_t * simple_metrics_config = NULL;

                    switch (remote_config->element.id)
                    {
                        case connector_setting_simple_enhanced_services_eth_on:
                        case connector_setting_simple_enhanced_services_eth_sampling:
                        case connector_setting_simple_enhanced_services_eth_reporting:
                            simple_metrics_config = &connector_ptr->dev_health.simple_metrics.config.eth;
                            break;
                        case connector_setting_simple_enhanced_services_mobile_on:
                        case connector_setting_simple_enhanced_services_mobile_sampling:
                        case connector_setting_simple_enhanced_services_mobile_reporting:
                            simple_metrics_config = &connector_ptr->dev_health.simple_metrics.config.mobile;
                            break;
                        /* TODO: IC4C-402 */
                        /*
                        case connector_setting_simple_enhanced_services_wifi_on:
                        case connector_setting_simple_enhanced_services_wifi_sampling:
                        case connector_setting_simple_enhanced_services_wifi_reporting:
                            simple_metrics_config = &connector_ptr->dev_health.simple_metrics.config.wifi;
                            break;
                        */
                        case connector_setting_simple_enhanced_services_sys_on:
                        case connector_setting_simple_enhanced_services_sys_sampling:
                        case connector_setting_simple_enhanced_services_sys_reporting:
                            simple_metrics_config = &connector_ptr->dev_health.simple_metrics.config.sys;
                            break;
                        default:
                            ASSERT_GOTO(remote_config->element.id < connector_setting_simple_enhanced_services_COUNT, done);
                            break;
                    }

                    switch (remote_config->action)
                    {
                        case connector_remote_action_set:
                        {
                            simple_enhs_rci_set(simple_metrics_config, remote_config);
                            break;
                        }
                        case connector_remote_action_query:
                        {
                            simple_enhs_rci_query(simple_metrics_config, remote_config);
                            break;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case connector_request_id_remote_config_action_end:
        {
            switch (remote_config->group.id)
            {
                case connector_setting_enhanced_services:
                {
                    switch (remote_config->action)
                    {
                        case connector_remote_action_set:
                        {
                            status = cc_dev_health_save_metrics(connector_ptr->dev_health.metrics.config, asizeof(connector_ptr->dev_health.metrics.config));
                            break;
                        }
                        case connector_remote_action_query:
                        {
                            break;
                        }
                    }
                    break;
                }
                case connector_setting_simple_enhanced_services:
                {
                    switch (remote_config->action)
                    {
                        case connector_remote_action_set:
                        {
                            status = cc_dev_health_simple_config_save(&connector_ptr->dev_health.simple_metrics.config);
                            break;
                        }
                        case connector_remote_action_query:
                        {
                            break;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
    }
done:
    return status;
}
