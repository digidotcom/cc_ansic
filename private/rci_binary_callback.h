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

#define state_call(rci, value)  ((rci)->parser.state = (value))

STATIC connector_bool_t is_set_command(connector_remote_action_t const action)
{
    return connector_bool(action == connector_remote_action_set);

}

STATIC void rci_error(rci_t * const rci, unsigned int const id, char const * const description, char const * const hint)
{
    rci->shared.callback_data.error_id = id;
    rci->shared.callback_data.response.error_hint = hint;

    rci->error.description = description;
}

#if defined RCI_PARSER_USES_ERROR_DESCRIPTIONS
static char const * get_rci_group_error(rci_t * const rci, unsigned int const id)
{
    connector_group_t const * const group = get_current_group(rci);
    connector_remote_config_data_t const * const rci_data = &rci->service_data->connector_ptr->rci_data;
    unsigned int const index = (id - rci_data->global_error_count);

    ASSERT(id >= rci_data->global_error_count);
    ASSERT(index < group->errors.count);

    return group->errors.description[index];
}
#else
static char const * get_rci_group_error(rci_t * const rci, unsigned int const id)
{
    UNUSED_PARAMETER(rci);
    UNUSED_PARAMETER(id);
    return NULL;
}
#endif

STATIC void rci_global_error(rci_t * const rci, unsigned int const id, char const * const hint)
{
    connector_remote_config_data_t const * const rci_data = &rci->service_data->connector_ptr->rci_data;
    char const * const description = rci_data->error_table[id] - connector_rci_error_OFFSET;

    rci_error(rci, id, description, hint);
}

STATIC void rci_group_error(rci_t * const rci, unsigned int const id, char const * const hint)
{
    connector_remote_config_data_t const * const rci_data = &rci->service_data->connector_ptr->rci_data;

    if (id < rci_data->global_error_count)
    {
        rci_global_error(rci, id, hint);
    }
    else
    {
        char const * const description = get_rci_group_error(rci, id);

        rci_error(rci, id, description, hint);
    }
}

STATIC connector_bool_t pending_rci_callback(rci_t * const rci)
{
    connector_bool_t const pending = connector_bool(rci->callback.status == connector_callback_busy);

    return pending;
}

STATIC void trigger_rci_callback(rci_t * const rci, connector_request_id_remote_config_t const remote_config_request)
{
    switch (remote_config_request)
    {
    case connector_request_id_remote_config_session_cancel:
        break;

    case connector_request_id_remote_config_session_start:
    case connector_request_id_remote_config_session_end:
    case connector_request_id_remote_config_action_start:
    case connector_request_id_remote_config_action_end:
        break;

    case connector_request_id_remote_config_group_start:
        ASSERT(have_group_id(rci));
        ASSERT(have_group_index(rci));

        rci->shared.callback_data.group.id = get_group_id(rci);
        rci->shared.callback_data.group.index = get_group_index(rci);
        break;

    case connector_request_id_remote_config_group_end:
        ASSERT(have_group_id(rci));
        ASSERT(have_group_index(rci));
        break;

    case connector_request_id_remote_config_group_process:
        ASSERT(have_group_id(rci));
        ASSERT(have_group_index(rci));
        ASSERT(have_element_id(rci));

        rci->shared.callback_data.element.id = get_element_id(rci);
        {
            connector_group_element_t const * const element = get_current_element(rci);

            rci->shared.callback_data.element.type = element->type;
        }

        rci->shared.callback_data.element.value = is_set_command(rci->shared.callback_data.action) ? &rci->shared.value : NULL;
        break;
    case connector_request_id_remote_config_configurations:
        ASSERT(remote_config_request != connector_request_id_remote_config_configurations);
        break;
    }

    rci->callback.request.remote_config_request = remote_config_request;
    rci->callback.status = connector_callback_busy;
}

STATIC connector_bool_t rci_callback(rci_t * const rci)
{
    connector_bool_t callback_complete;
    connector_remote_config_t * remote_config = &rci->shared.callback_data;
    unsigned int const element = remote_config->element.id;
    unsigned int const group = remote_config->group.id;
    unsigned int const group_type = remote_config->group.type;
    connector_remote_action_t const action = remote_config->action;
    connector_remote_config_data_t const * const rci_data = &rci->service_data->connector_ptr->rci_data;
    connector_remote_group_table_t const * const connector_group_table = rci_data->group_table;
    rci_function_t cb_function = NULL;

    switch(rci->callback.request.remote_config_request)
    {
        case connector_request_id_remote_config_group_start:
            cb_function = connector_group_table[group_type].groups[group].start_cb;
            break;
        case connector_request_id_remote_config_group_end:
            cb_function = connector_group_table[group_type].groups[group].end_cb;
            break;
        case connector_request_id_remote_config_group_process:
        {
            switch (action)
            {
                case connector_remote_action_set:
                    cb_function = connector_group_table[group_type].groups[group].elements.data[element].set_cb;
                    break;
                case connector_remote_action_query:
                    cb_function = connector_group_table[group_type].groups[group].elements.data[element].get_cb;
                    break;
            }
            break;
        }
        case connector_request_id_remote_config_session_start:
        case connector_request_id_remote_config_session_end:
        case connector_request_id_remote_config_session_cancel:
        case connector_request_id_remote_config_action_start:
        case connector_request_id_remote_config_action_end:
        case connector_request_id_remote_config_configurations:
            rci->callback.status = connector_callback_continue;
            break;
    }

    if (cb_function != NULL)
    {
        switch(rci->callback.request.remote_config_request)
        {
            case connector_request_id_remote_config_group_start:
            case connector_request_id_remote_config_group_end:
                rci->callback.status = cb_function(NULL);
                break;
            case connector_request_id_remote_config_group_process:
            {
                switch (action)
                {
                    case connector_remote_action_set:
                    {
                        switch (connector_group_table[group_type].groups[group].elements.data[element].type)
                        {
                            case connector_element_type_string:
                            case connector_element_type_multiline_string:
                            case connector_element_type_password:
                            case connector_element_type_ipv4:
                            case connector_element_type_fqdnv4:
                            case connector_element_type_datetime:
                                rci->callback.status = cb_function(NULL, &remote_config->element.value->string_value);
                                break;
                            case connector_element_type_int32:
                                rci->callback.status = cb_function(NULL, &remote_config->element.value->signed_integer_value);
                                break;
                            case connector_element_type_uint32:
                            case connector_element_type_0x_hex32:
                                rci->callback.status = cb_function(NULL, &remote_config->element.value->unsigned_integer_value);
                                break;

                            case connector_element_type_enum:
                                rci->callback.status = cb_function(NULL, &remote_config->element.value->enum_value);
                                break;
                            case connector_element_type_on_off:
                                rci->callback.status = cb_function(NULL, &remote_config->element.value->on_off_value);
                                break;
                            case connector_element_type_boolean:
                                rci->callback.status = cb_function(NULL, &remote_config->element.value->boolean_value);
                                break;
                            default:
                                rci->callback.status = cb_function(NULL);
                                break;
                        }
                        break;
                    }
                    case connector_remote_action_query:
                    {
                        switch (connector_group_table[group_type].groups[group].elements.data[element].type)
                        {
                            case connector_element_type_string:
                            case connector_element_type_multiline_string:
                            case connector_element_type_password:
                            case connector_element_type_ipv4:
                            case connector_element_type_fqdnv4:
                            case connector_element_type_datetime:
                                rci->callback.status = cb_function(NULL, &remote_config->response.element_value->string_value);
                                break;
                            case connector_element_type_int32:
                                rci->callback.status = cb_function(NULL, &remote_config->response.element_value->signed_integer_value);
                                break;
                            case connector_element_type_uint32:
                            case connector_element_type_0x_hex32:
                                rci->callback.status = cb_function(NULL, &remote_config->response.element_value->unsigned_integer_value);
                                break;

                            case connector_element_type_enum:
                                rci->callback.status = cb_function(NULL, &remote_config->response.element_value->enum_value);
                                break;
                            case connector_element_type_on_off:
                                rci->callback.status = cb_function(NULL, &remote_config->response.element_value->on_off_value);
                                break;
                            case connector_element_type_boolean:
                                rci->callback.status = cb_function(NULL, &remote_config->response.element_value->boolean_value);
                                break;
                            default:
                                rci->callback.status = cb_function(NULL);
                                break;
                        }
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    switch (rci->callback.status)
    {
    case connector_callback_abort:
        callback_complete = connector_false;
        rci->status = rci_status_error;
        break;

    case connector_callback_continue:
        callback_complete = connector_true;
#if 0
        if (response_data->error_id != connector_success)
        {
            switch (rci->callback.request.remote_config_request)
            {
            case connector_request_id_remote_config_session_end:
                callback_complete = connector_false;
                rci->status = rci_status_internal_error;
                break;
            default:
                break;
            }
        }
#endif
        break;

    case connector_callback_busy:
        callback_complete = connector_false;
        break;

    default:
        callback_complete = connector_false;
        rci->status = rci_status_error;
        break;
    }

    return callback_complete;
}


