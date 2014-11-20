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
#define get_rci_global_error(rci, id)   rci->service_data->connector_ptr->rci_data.error_table[(id) - connector_rci_error_OFFSET]
static char const * get_rci_group_error(rci_t * const rci, unsigned int const id)
{
    connector_group_t const * const group = get_current_group(rci);
    unsigned int const index = (id - rci->service_data->connector_ptr->rci_data.global_error_count);

    ASSERT(id >= rci->service_data->connector_ptr->rci_data.global_error_count);
    ASSERT(index < group->errors.count);

    return group->errors.description[index];
}
#else
#define get_rci_global_error(rci, id)   ((void) (rci), (void) (id), NULL)
#define get_rci_group_error(rci, id)    ((void) (rci), (void) (id), NULL)
#endif

STATIC void rci_global_error(rci_t * const rci, unsigned int const id, char const * const hint)
{
    char const * const description = get_rci_global_error(rci, id);

    rci_error(rci, id, description, hint);
}

STATIC void rci_group_error(rci_t * const rci, unsigned int const id, char const * const hint)
{
    if (id < rci->service_data->connector_ptr->rci_data.global_error_count)
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
        rci->shared.callback_data.attribute.source = NULL;
        rci->shared.callback_data.attribute.compare_to = NULL;
#if (defined RCI_LEGACY_COMMANDS)
        rci->shared.callback_data.attribute.target = NULL;
#endif
        break;
    case connector_request_id_remote_config_session_end:
        break;
    case connector_request_id_remote_config_action_start:
        switch (rci->command.command_id)
        {
            case rci_command_query_setting:
            case rci_command_query_state:
            {
                unsigned int i;
                for (i=0; i < rci->command.attribute_count; i++)
                {
                    switch (rci->command.attribute[i].id.query)
                    {
                        case rci_query_command_attribute_id_source:
                            rci->shared.callback_data.attribute.source = rci->command.attribute[i].value;
                            break;
                        case rci_query_command_attribute_id_compare_to:
                            rci->shared.callback_data.attribute.compare_to = rci->command.attribute[i].value;
                            break;
                        case rci_query_command_attribute_id_count:
                            ASSERT_GOTO(0, done);
                            break;
                    }
                }
                break;
            }
#if (defined RCI_LEGACY_COMMANDS)
            case rci_command_do_command:
            {
                unsigned int i;
                for (i=0; i < rci->command.attribute_count; i++)
                {
                    switch (rci->command.attribute[i].id.do_command)
                    {
                        case rci_do_command_attribute_id_target:
                            rci->shared.callback_data.attribute.target = rci->command.attribute[i].value;
                            break;
                        case rci_do_command_attribute_id_count:
                            ASSERT_GOTO(0, done);
                            break;
                    }
                }
                break;
            }
#endif
            case rci_command_set_setting:
            case rci_command_set_state:
            case rci_command_query_descriptor:
#if (defined RCI_LEGACY_COMMANDS)
            case rci_command_reboot:
            case rci_command_set_factory_default:
#endif
                break;
        }         
        rci->shared.callback_data.response.element_value = &rci->shared.value;
        break;
    case connector_request_id_remote_config_action_end:
        break;

    case connector_request_id_remote_config_group_start:
        ASSERT(have_group_id(rci));
        ASSERT(have_group_index(rci));

        rci->shared.callback_data.group.id = get_group_id(rci);
        rci->shared.callback_data.group.index = get_group_index(rci);
#if (defined RCI_PARSER_USES_GROUP_NAMES)
        rci->shared.callback_data.group.name = get_current_group(rci)->name;
#endif
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
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
            rci->shared.callback_data.element.name = element->name;
#endif
        }

        rci->shared.callback_data.element.value = is_set_command(rci->shared.callback_data.action) ? &rci->shared.value : NULL;
        break;
    case connector_request_id_remote_config_configurations:
        ASSERT(remote_config_request != connector_request_id_remote_config_configurations);
        break;
#if (defined RCI_LEGACY_COMMANDS)
    case connector_request_id_remote_config_do_command:
        /* Provide request */
        rci->shared.callback_data.element.value = &rci->shared.value;
        /* Provide response pointer */
        rci->shared.callback_data.response.element_value = &rci->command.do_command.response_value;
        rci->shared.callback_data.response.element_value->string_value = NULL;
        goto done;
        break;
#endif
    case connector_request_id_remote_config_reboot:
    case connector_request_id_remote_config_set_factory_def:
        goto done;
        break;
    }

done:
    rci->callback.request.remote_config_request = remote_config_request;
    rci->callback.status = connector_callback_busy;
}

STATIC connector_bool_t rci_callback(rci_t * const rci)
{
    connector_bool_t callback_complete;
    connector_remote_config_t * remote_config = &rci->shared.callback_data;
    connector_remote_config_cancel_t remote_cancel;
    void * callback_data = NULL;
    connector_request_id_remote_config_t const remote_config_request = rci->callback.request.remote_config_request;

    switch (remote_config_request)
    {
        case connector_request_id_remote_config_session_start:
#if (defined RCI_PARSER_USES_GROUP_NAMES)
            rci->shared.callback_data.group.name = NULL;
#endif
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
            rci->shared.callback_data.element.name = NULL;
#endif
            /* intentional fall through */
        case connector_request_id_remote_config_session_end:
        case connector_request_id_remote_config_action_start:
        case connector_request_id_remote_config_action_end:
        case connector_request_id_remote_config_group_start:
            rci->output.group_skip = connector_false;
            /* intentional fall through */
        case connector_request_id_remote_config_group_end:
        case connector_request_id_remote_config_group_process:
            rci->output.element_skip = connector_false;
            remote_config->error_id = connector_success;
            remote_config->response.error_hint = NULL;
            callback_data = remote_config;
            break;

        case connector_request_id_remote_config_session_cancel:
        {
            remote_cancel.user_context = remote_config->user_context;
            callback_data =  &remote_cancel;
            break;
        }
        case connector_request_id_remote_config_configurations:
            ASSERT(remote_config_request != connector_request_id_remote_config_configurations);
            callback_complete = connector_false;
            goto done;
#if (defined RCI_LEGACY_COMMANDS)
        case connector_request_id_remote_config_do_command:
        case connector_request_id_remote_config_reboot:
        case connector_request_id_remote_config_set_factory_def:
            remote_config->error_id = connector_success;
            callback_data = remote_config;
            break;
#endif
    }


    switch (remote_config_request)
    {
        default:
            if (remote_config_request == connector_request_id_remote_config_group_process && rci->output.group_skip == connector_true)
            {
                rci->callback.status = connector_callback_continue;
            }
            else
            {
#if (defined CONNECTOR_DEVICE_HEALTH)
                rci->callback.status = enhs_rci_handler(rci->service_data->connector_ptr, rci->callback.request, callback_data);
#else
                rci->callback.status = connector_callback(rci->service_data->connector_ptr->callback, connector_class_id_remote_config, rci->callback.request, callback_data, rci->service_data->connector_ptr->context);
#endif
            }
            break;

#if (defined RCI_LEGACY_COMMANDS)
        case connector_request_id_remote_config_do_command:
        {
#if (defined CONNECTOR_DEVICE_HEALTH)
            rci->callback.status = enhs_rci_handler(rci->service_data->connector_ptr, rci->callback.request, callback_data);
#else
            rci->callback.status = connector_callback(rci->service_data->connector_ptr->callback, connector_class_id_remote_config, rci->callback.request, callback_data, rci->service_data->connector_ptr->context);
#endif
            if (rci->callback.status == connector_callback_error) 
            {
                rci_global_error(rci, connector_rci_error_do_command_failed, RCI_NO_HINT);
                set_rci_command_error(rci);
                state_call(rci, rci_parser_state_error);

                rci->callback.status = connector_callback_continue;
            }
            break;
        }

        case connector_request_id_remote_config_reboot:
        {
#if (defined CONNECTOR_DEVICE_HEALTH)
            rci->callback.status = enhs_rci_handler(rci->service_data->connector_ptr, rci->callback.request, callback_data);
#else
            rci->callback.status = connector_callback(rci->service_data->connector_ptr->callback, connector_class_id_remote_config, rci->callback.request, callback_data, rci->service_data->connector_ptr->context);
#endif
            if (rci->callback.status == connector_callback_continue) 
            {
                connector_request_id_t request_id;
                connector_data_t * const connector_ptr = rci->service_data->connector_ptr;

                connector_class_id_t class_id = connector_class_id_operating_system;
                request_id.os_request = connector_request_id_os_reboot;
                rci->callback.status = connector_callback(connector_ptr->callback, class_id, request_id, NULL, connector_ptr->context);
            }

            if (rci->callback.status == connector_callback_error) 
            {
                rci_global_error(rci, connector_rci_error_reboot_failed, RCI_NO_HINT);
                set_rci_command_error(rci);
                state_call(rci, rci_parser_state_error);

                rci->callback.status = connector_callback_continue;
            }
            break;
        }
        case connector_request_id_remote_config_set_factory_def:
        {
#if (defined CONNECTOR_DEVICE_HEALTH)
            rci->callback.status = enhs_rci_handler(rci->service_data->connector_ptr, rci->callback.request, callback_data);
#else
            rci->callback.status = connector_callback(rci->service_data->connector_ptr->callback, connector_class_id_remote_config, rci->callback.request, callback_data, rci->service_data->connector_ptr->context);
#endif

            if (rci->callback.status == connector_callback_error) 
            {
                rci_global_error(rci, connector_rci_error_set_factory_default_failed, RCI_NO_HINT);
                set_rci_command_error(rci);
                state_call(rci, rci_parser_state_error);

                rci->callback.status = connector_callback_continue;
            }
            break;
        }
#endif
    }

    switch (rci->callback.status)
    {
    case connector_callback_abort:
        callback_complete = connector_false;
        rci->status = rci_status_error;
        break;

    case connector_callback_continue:
        callback_complete = connector_true;

        if ((connector_rci_error_id_t)remote_config->error_id == connector_rci_error_not_available)
        {
            switch (remote_config_request)
            {
                case connector_request_id_remote_config_group_process:
                    rci->output.element_skip = connector_true;
                    break;

                case connector_request_id_remote_config_group_start:
                    rci->output.group_skip = connector_true;
                    break;

                default:
                    /* Invalid error_id for this callback */
                    ASSERT(0);
                    break;
            }

            remote_config->error_id = connector_success;
        }
        break;

    case connector_callback_busy:
        callback_complete = connector_false;
        break;

    default:
        callback_complete = connector_false;
        rci->status = rci_status_error;
        break;
    }

    if (callback_complete)
    {
        if (destination_in_storage(rci))
        {
            rci->input.destination = rci->buffer.input.current;
            reset_input_content(rci);
        }

#if (defined RCI_PARSER_USES_GROUP_NAMES) || (defined RCI_PARSER_USES_ELEMENT_NAMES)
/* TODO */
        switch (remote_config_request)
        {
            case connector_request_id_remote_config_group_end:
#if (defined RCI_PARSER_USES_GROUP_NAMES)
                rci->shared.callback_data.group.name = NULL;
#endif
                break;
            case connector_request_id_remote_config_group_process:
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
                rci->shared.callback_data.element.name = NULL;
#endif
                break;
            case connector_request_id_remote_config_session_start:
                break;
            case connector_request_id_remote_config_session_end:
            case connector_request_id_remote_config_action_start:
                break;
            case connector_request_id_remote_config_action_end:
                rci->shared.callback_data.attribute.source = NULL;
                rci->shared.callback_data.attribute.compare_to = NULL;
#if (defined RCI_LEGACY_COMMANDS)
                rci->shared.callback_data.attribute.target = NULL;
#endif
                break;
            case connector_request_id_remote_config_group_start:
            case connector_request_id_remote_config_session_cancel:
            case connector_request_id_remote_config_configurations:
                break;
        }
#endif
    }

done:
    return callback_complete;
}


