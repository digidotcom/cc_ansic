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
#include "remote_config.h"

#if !defined CONNECTOR_RCI_SERVICE
#error "Must define CONNECTOR_RCI_SERVICE in connector_config.h to run this sample"
#endif

typedef enum {
    no_fail,
    fail_start_session,
    fail_start_action,
    fail_start_group,
    fail_process_group,
    fail_end_group,
    fail_end_action,
    fail_end_session
} session_fail_state_t;

session_fail_state_t session_fail_state = no_fail;

static connector_callback_status_t app_process_session_start(connector_remote_config_t * const remote_config)
{

    APP_DEBUG("app_process_session_start\n");

    if (session_fail_state == fail_start_session)
    {
        response->error_id = connector_global_error_memory_fail;
        remote_config->response.error_hint = NULL;
        session_fail_state = no_fail;
    }

    return connector_callback_continue;
}

static connector_callback_status_t app_process_session_end(connector_remote_config_t * const remote_config)
{
    APP_DEBUG("app_process_session_end\n");

    if (session_fail_state == fail_end_session)
    {
        response->error_id = connector_global_error_memory_fail;
        remote_config->response.error_hint = NULL;
        session_fail_state = no_fail;
    }

    return connector_callback_continue;
}

static connector_callback_status_t app_process_action_start(connector_remote_config_t * const remote_config)
{

    APP_DEBUG("app_process_action_start\n");

    if (session_fail_state == fail_start_action)
    {
        response->error_id = connector_global_error_memory_fail;
        remote_config->response.error_hint = NULL;
        session_fail_state = no_fail;
    }

    return connector_callback_continue;
}

static connector_callback_status_t app_process_action_end(connector_remote_group_request_t const * const request,
                                                      connector_remote_group_response_t * const response)
{
    UNUSED_ARGUMENT(request);
    APP_DEBUG("app_process_action_end\n");

    if (session_fail_state == fail_end_action)
    {
        response->error_id = connector_global_error_memory_fail;
        remote_config->response.error_hint = NULL;
        session_fail_state = no_fail;
    }

    return connector_callback_continue;
}

static connector_callback_status_t app_process_group_start(connector_remote_group_request_t const * const request,
                                                       connector_remote_group_response_t * const response)
{
    UNUSED_ARGUMENT(request);

    if (session_fail_state == fail_start_group)
    {
        response->error_id = connector_global_error_memory_fail;
        remote_config->response.error_hint = NULL;
        session_fail_state = no_fail;
    }

    return connector_callback_continue;
}

static connector_callback_status_t app_process_group_process(connector_remote_group_request_t const * const request,
                                                         connector_remote_group_response_t * const response)
{

    if (session_fail_state == fail_process_group)
    {
        response->error_id = connector_global_error_load_fail;
        remote_config->response.error_hint = "Hint";
        session_fail_state = no_fail;
        goto done;
    }

    if (remote_config->action == connector_remote_action_query)
    {
        remote_config->response.element_value->string_value = "my data";
        goto done;
    }

    switch (remote_config->element.id)
    {
        case connector_setting_test_error_start_session:
            session_fail_state = fail_start_session;
            break;
        case connector_setting_test_error_start_action:
            session_fail_state = fail_start_action;
            break;
        case connector_setting_test_error_start_group:
            session_fail_state = fail_start_group;
            break;
        case connector_setting_test_error_process_group:
            session_fail_state = fail_process_group;
            break;
        case connector_setting_test_error_end_group:
            session_fail_state = fail_end_group;
            break;
        case connector_setting_test_error_end_action:
            session_fail_state = fail_end_action;
            break;
        case connector_setting_test_error_end_session:
            session_fail_state = fail_end_session;
            break;
    }

done:
    return connector_callback_continue;
}

static connector_callback_status_t app_process_group_end(connector_remote_group_request_t const * const request,
                                                     connector_remote_group_response_t * const response)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request);
    if (session_fail_state == fail_end_group)
    {
        response->error_id = connector_global_error_memory_fail;
        remote_config->response.error_hint = NULL;
        session_fail_state = no_fail;
    }

    return status;
}

static connector_callback_status_t app_process_session_cancel(void const * const context)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(context);

    APP_DEBUG("app_process_session_cancel\n");

    return status;
}

connector_callback_status_t app_remote_config_handler(connector_request_id_remote_config_t const request,
                                                      void const * const request_data, size_t const request_length,
                                                      void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {
    case connector_request_id_remote_config_session_start:
        status = app_process_session_start(response_data);
        break;

    case connector_request_id_remote_config_action_start:
        status = app_process_action_start(request_data, response_data);
        break;

    case connector_request_id_remote_config_group_start:
        status = app_process_group_start( request_data, response_data);
        break;

    case connector_request_id_remote_config_group_process:
        status = app_process_group_process(request_data, response_data);
        break;

    case connector_request_id_remote_config_group_end:
        status = app_process_group_end(request_data, response_data);
        break;

    case connector_request_id_remote_config_action_end:
        status = app_process_action_end(request_data, response_data);
        break;

    case connector_request_id_remote_config_session_end:
        status = app_process_session_end(response_data);
        break;

    case connector_request_id_remote_config_session_cancel:
        status = app_process_session_cancel(request_data);
        break;
    default:
        APP_DEBUG("app_remote_config_handler: unknown request id %d\n", request);
        break;
    }

    return status;
}

