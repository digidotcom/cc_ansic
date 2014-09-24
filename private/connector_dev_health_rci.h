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
        case connector_setting_enhanced_services_sample_rate:
        {
            metrics_item->sample_rate = remote_config->element.value->unsigned_integer_value;
            break;
        }
        case connector_setting_enhanced_services_report_rate:
        {
            metrics_item->report_rate = remote_config->element.value->unsigned_integer_value;
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
        case connector_setting_enhanced_services_sample_rate:
        {
            remote_config->response.element_value->unsigned_integer_value = metrics_item->sample_rate;
            break;
        }
        case connector_setting_enhanced_services_report_rate:
        {
            remote_config->response.element_value->unsigned_integer_value = metrics_item->report_rate;
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
        case connector_setting_simple_enhanced_services_eth_metrics:
        case connector_setting_simple_enhanced_services_mobile_metrics:
        /*case connector_setting_simple_enhanced_services_wifi_metrics:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_metrics:
        {
            simple_metrics->metrics = remote_config->element.value->on_off_value;
            break;
        }
        case connector_setting_simple_enhanced_services_eth_sample_rate:
        case connector_setting_simple_enhanced_services_mobile_sample_rate:
        /*case connector_setting_simple_enhanced_services_wifi_sample_rate:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_sample_rate:
        {
            simple_metrics->sample_rate = remote_config->element.value->unsigned_integer_value;
            break;
        }
        case connector_setting_simple_enhanced_services_eth_report_rate:
        case connector_setting_simple_enhanced_services_mobile_report_rate:
        /*case connector_setting_simple_enhanced_services_wifi_report_rate:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_report_rate:
        {
            simple_metrics->report_rate = remote_config->element.value->unsigned_integer_value;
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
        case connector_setting_simple_enhanced_services_eth_metrics:
        case connector_setting_simple_enhanced_services_mobile_metrics:
        /*case connector_setting_simple_enhanced_services_wifi_metrics:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_metrics:
        {
            remote_config->response.element_value->on_off_value = simple_metrics->metrics ? connector_on : connector_off;
            break;
        }
        case connector_setting_simple_enhanced_services_eth_sample_rate:
        case connector_setting_simple_enhanced_services_mobile_sample_rate:
        /*case connector_setting_simple_enhanced_services_wifi_sample_rate:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_sample_rate:
        {
            remote_config->response.element_value->unsigned_integer_value = simple_metrics->sample_rate;
            break;
        }
        case connector_setting_simple_enhanced_services_eth_report_rate:
        case connector_setting_simple_enhanced_services_mobile_report_rate:
        /*case connector_setting_simple_enhanced_services_wifi_report_rate:*/ /* TODO: IC4C-402 */
        case connector_setting_simple_enhanced_services_sys_report_rate:
        {
            remote_config->response.element_value->unsigned_integer_value = simple_metrics->report_rate;
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

STATIC connector_callback_status_t enhs_rci_handler(connector_data_t * const connector_ptr, connector_request_id_t const request, void * const data)
{
    connector_remote_config_t * const remote_config = data;
    connector_callback_status_t status = connector_callback_continue;
    connector_request_id_remote_config_t request_id = request.remote_config_request;

    switch (request_id)
    {
        case connector_request_id_remote_config_session_start:
        case connector_request_id_remote_config_session_end:
        case connector_request_id_remote_config_action_start:
        case connector_request_id_remote_config_session_cancel:
        case connector_request_id_remote_config_configurations:
            status = connector_callback(connector_ptr->callback, connector_class_id_remote_config, request, data, connector_ptr->context);
            break;
        case connector_request_id_remote_config_group_start:
        case connector_request_id_remote_config_group_end:
            switch (remote_config->group.id)
            {
                case connector_setting_enhanced_services:
                case connector_setting_simple_enhanced_services:
                    break;
                default:
                    status = connector_callback(connector_ptr->callback, connector_class_id_remote_config, request, data, connector_ptr->context);
                    break;
            }
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
                        case connector_setting_simple_enhanced_services_eth_metrics:
                        case connector_setting_simple_enhanced_services_eth_sample_rate:
                        case connector_setting_simple_enhanced_services_eth_report_rate:
                            simple_metrics_config = &connector_ptr->dev_health.simple_metrics.config.eth;
                            break;
                        case connector_setting_simple_enhanced_services_mobile_metrics:
                        case connector_setting_simple_enhanced_services_mobile_sample_rate:
                        case connector_setting_simple_enhanced_services_mobile_report_rate:
                            simple_metrics_config = &connector_ptr->dev_health.simple_metrics.config.mobile;
                            break;
                        /* TODO: IC4C-402 */
                        /*
                        case connector_setting_simple_enhanced_services_wifi_metrics:
                        case connector_setting_simple_enhanced_services_wifi_sample_rate:
                        case connector_setting_simple_enhanced_services_wifi_report_rate:
                            simple_metrics_config = &connector_ptr->dev_health.simple_metrics.config.wifi;
                            break;
                        */
                        case connector_setting_simple_enhanced_services_sys_metrics:
                        case connector_setting_simple_enhanced_services_sys_sample_rate:
                        case connector_setting_simple_enhanced_services_sys_report_rate:
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
                    status = connector_callback(connector_ptr->callback, connector_class_id_remote_config, request, data, connector_ptr->context);
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
                            cc_dev_health_save_metrics(connector_ptr->dev_health.metrics.config, asizeof(connector_ptr->dev_health.metrics.config));
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
                            cc_dev_health_simple_config_save(&connector_ptr->dev_health.simple_metrics.config);
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
            status = connector_callback(connector_ptr->callback, connector_class_id_remote_config, request, data, connector_ptr->context);
            break;
        }
    }
done:
    return status;
}
