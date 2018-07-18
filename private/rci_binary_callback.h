/*
 * Copyright (c) 2018 Digi International Inc.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
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
#define get_rci_global_error(rci, id)   rci->service_data->connector_ptr->rci_data->error_table[(id) - connector_rci_error_OFFSET]
static char const * get_rci_group_error(rci_t * const rci, unsigned int const id)
{
    connector_group_t const * const group = get_current_group(rci);
    unsigned int const index = id - (rci->service_data->connector_ptr->rci_data->global_error_count + 1);

    ASSERT(id >= rci->service_data->connector_ptr->rci_data->global_error_count + 1);
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
    if (id < (rci->service_data->connector_ptr->rci_data->global_error_count + 1))
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
        rci->shared.callback_data.attribute.source = rci_query_setting_attribute_source_current;
        rci->shared.callback_data.attribute.compare_to = rci_query_setting_attribute_compare_to_none;
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
            {
                unsigned int i;
                for (i=0; i < rci->shared.attribute_count; i++)
                {
                    switch (rci->command.attribute[i].id.query_setting)
                    {
                        case rci_query_setting_attribute_id_source:
                            rci->shared.callback_data.attribute.source = rci->command.attribute[i].value.enum_val;
                            break;
                        case rci_query_setting_attribute_id_compare_to:
                            rci->shared.callback_data.attribute.compare_to = rci->command.attribute[i].value.enum_val;
                            break;
                        case rci_query_setting_attribute_id_count:
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
                for (i=0; i < rci->shared.attribute_count; i++)
                {
                    switch (rci->command.attribute[i].id.do_command)
                    {
                        case rci_do_command_attribute_id_target:
                            rci->shared.callback_data.attribute.target = rci->command.attribute[i].value.string_val;
                            break;
                        case rci_do_command_attribute_id_count:
                            ASSERT_GOTO(0, done);
                            break;
                    }
                }
                break;
            }
#endif
            case rci_command_query_state:
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

	case connector_request_id_remote_config_group_instance_remove:
		ASSERT(get_group_collection_type(rci) == connector_collection_type_variable_dictionary);
		ASSERT(rci->shared.group.info.instance == 0);
		rci->shared.callback_data.group.collection_type = connector_collection_type_variable_dictionary;
		rci->shared.callback_data.group.item.key = rci->shared.group.info.keys.key_store;
		break;

    case connector_request_id_remote_config_group_start:
	{
		connector_collection_type_t const collection_type = get_group_collection_type(rci);
		if (collection_type == connector_collection_type_fixed_array || collection_type == connector_collection_type_variable_array)
		{
			rci->shared.callback_data.group.item.index = get_group_instance(rci);
		}
		else
		{
			unsigned int const instance = get_group_instance(rci);
			if (instance == 0)
			{
				rci->shared.callback_data.group.item.key = rci->shared.group.info.keys.key_store;
			}
			else
			{
				rci->shared.callback_data.group.item.key = rci->shared.group.info.keys.list[instance - 1];
			}
		}
	}
	/* intentional fall-through */

	case connector_request_id_remote_config_group_instances_lock:
	{
		connector_collection_type_t const collection_type = get_group_collection_type(rci);
		rci->shared.callback_data.group.collection_type = collection_type;
		ASSERT(have_group_id(rci));
#if (defined RCI_PARSER_USES_LIST)
		rci->shared.callback_data.list.depth = 0;
#endif
        rci->shared.callback_data.group.id = get_group_id(rci);
#if (defined RCI_PARSER_USES_COLLECTION_NAMES)
        rci->shared.callback_data.group.name = get_current_group(rci)->collection.name;
#endif
        break;
	}

	case connector_request_id_remote_config_group_instances_unlock:
		/* Data for last lock should still be present in callback_data */
		break;

	case connector_request_id_remote_config_group_instances_set:
	{
		connector_collection_type_t const collection_type = get_group_collection_type(rci);
		ASSERT(collection_type == connector_collection_type_variable_array || collection_type == connector_collection_type_variable_dictionary);
		if (collection_type == connector_collection_type_variable_array)
		{
			rci->shared.callback_data.group.item.count = rci->shared.group.info.keys.count;
		}
		else
		{
			rci->shared.callback_data.group.item.dictionary.entries = rci->shared.group.info.keys.count;
			rci->shared.callback_data.group.item.dictionary.keys = rci->shared.group.info.keys.list;
		}
		break;
	}

    case connector_request_id_remote_config_group_end:
        ASSERT(have_group_id(rci));
        ASSERT(have_group_instance(rci));
        break;

#if (defined RCI_PARSER_USES_LIST)

	case connector_request_id_remote_config_list_instance_remove:
	{
		unsigned int const index = get_list_depth(rci) - 1;
		ASSERT(get_current_list_collection_type(rci) == connector_collection_type_variable_dictionary);
		ASSERT(get_current_list_instance(rci) == 0);
		rci->shared.callback_data.list.level[index].collection_type = connector_collection_type_variable_dictionary;
		rci->shared.callback_data.list.level[index].item.key = rci->shared.list.level[index].info.keys.key_store;
		break;
	}

	case connector_request_id_remote_config_list_start:
	{
		unsigned int const index = get_list_depth(rci) - 1;
		connector_collection_type_t const collection_type = get_current_list_collection_type(rci);
		if (collection_type == connector_collection_type_fixed_array || collection_type == connector_collection_type_variable_array)
		{
			rci->shared.callback_data.list.level[index].item.index = rci->shared.list.level[index].info.instance;
		}
		else
		{
			unsigned int const instance = rci->shared.list.level[index].info.instance;
			if (instance ==  0)
			{
				rci->shared.callback_data.list.level[index].item.key = rci->shared.list.level[index].info.keys.key_store;
			}
			else
			{
				rci->shared.callback_data.list.level[index].item.key = rci->shared.list.level[index].info.keys.list[instance - 1];
			}
		}
	}
	/* intentional fall-through */

	case connector_request_id_remote_config_list_instances_lock:
		ASSERT(have_group_id(rci));
		ASSERT(get_list_depth(rci) > 0 && get_list_depth(rci) <= RCI_LIST_MAX_DEPTH);
		
		rci->shared.callback_data.list.depth = get_list_depth(rci);
		{
			unsigned int const index = get_list_depth(rci) - 1;
			rci->shared.callback_data.list.level[index].collection_type = get_current_list_collection_type(rci);;
			rci->shared.callback_data.list.level[index].id = rci->shared.list.level[index].id;
			#if (defined RCI_PARSER_USES_COLLECTION_NAMES)
			{
				connector_collection_t const * const list = get_current_collection_info(rci);
				rci->shared.callback_data.list.level[index].name = list->name;
			}
			#endif
		}
		break;

	case connector_request_id_remote_config_list_end:
		ASSERT(have_current_list_instance(rci));
		ASSERT(have_current_list_id(rci));
		/* Intentional fall-through */

	case connector_request_id_remote_config_list_instances_unlock:
		rci->shared.callback_data.list.depth = get_list_depth(rci);
		break;

	case connector_request_id_remote_config_list_instances_set:
	{
		unsigned int index = get_list_depth(rci) - 1;
		connector_collection_type_t const collection_type = get_current_list_collection_type(rci);
		ASSERT(collection_type == connector_collection_type_variable_array || collection_type == connector_collection_type_variable_dictionary);
		if (collection_type == connector_collection_type_variable_array)
		{
			rci->shared.callback_data.list.level[index].item.count = rci->shared.list.level[index].info.keys.count;
		}
		else
		{
			rci->shared.callback_data.list.level[index].item.dictionary.entries = rci->shared.list.level[index].info.keys.count;
			rci->shared.callback_data.list.level[index].item.dictionary.keys = rci->shared.list.level[index].info.keys.list;
		}
		break;
	}

#endif
    case connector_request_id_remote_config_element_process:
        ASSERT(have_group_id(rci));
        ASSERT(have_group_instance(rci));
        ASSERT(have_element_id(rci));

#if (defined RCI_PARSER_USES_LIST)
		rci->shared.callback_data.list.depth = get_list_depth(rci);
#endif
        rci->shared.callback_data.element.id = get_element_id(rci);
        {
            connector_item_t const * const element = get_current_element(rci);

            rci->shared.callback_data.element.type = element->type;
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
            rci->shared.callback_data.element.name = element->data.element->name;
#endif
        }

        rci->shared.callback_data.element.value = is_set_command(rci->shared.callback_data.action) ? &rci->shared.value : NULL;
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

    case connector_request_id_remote_config_reboot:
    case connector_request_id_remote_config_set_factory_def:
        goto done;
        break;
#endif
    }

done:
    rci->callback.request.remote_config_request = remote_config_request;
    rci->callback.status = connector_callback_busy;
}

#if !(defined CONNECTOR_NO_MALLOC)
STATIC connector_status_t free_rci_internal_data(connector_data_t * const connector_ptr)
{
    connector_status_t status = connector_working;

    if (connector_ptr->rci_internal_data != NULL)
    {
        if (connector_ptr->rci_internal_data->input.storage != NULL)
        {
            status = free_data(connector_ptr, connector_ptr->rci_internal_data->input.storage);
            ASSERT_GOTO(status == connector_working, done);
        }
        status = free_data(connector_ptr, connector_ptr->rci_internal_data);
        ASSERT_GOTO(status == connector_working, done);

        connector_ptr->rci_internal_data = NULL;
    }

done:
    return status;
}
#endif

STATIC connector_bool_t rci_callback(rci_t * const rci)
{
    connector_bool_t callback_complete;
    connector_remote_config_t * const remote_config = &rci->shared.callback_data;
    connector_remote_config_cancel_t remote_cancel;
    void * callback_data = NULL;
    connector_request_id_remote_config_t const remote_config_request = rci->callback.request.remote_config_request;

    switch (remote_config_request)
    {
        case connector_request_id_remote_config_session_start:
#if (defined RCI_PARSER_USES_COLLECTION_NAMES)
			rci->shared.callback_data.group.name = NULL;

#if (defined RCI_PARSER_USES_LIST)
			{
				int i;
				for (i = 0; i < RCI_LIST_MAX_DEPTH; i++)
				{
		       		rci->shared.callback_data.list.level[i].name = NULL;
				}
			}
#endif
#endif
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
            rci->shared.callback_data.element.name = NULL;
#endif
            /* intentional fall through */
        case connector_request_id_remote_config_session_end:
        case connector_request_id_remote_config_action_start:
        case connector_request_id_remote_config_action_end:
		case connector_request_id_remote_config_group_instances_lock:
		case connector_request_id_remote_config_list_instances_lock:
			if (get_list_depth(rci) <= rci->output.skip_depth)
			{
				SET_RCI_SHARED_FLAG(rci, RCI_SHARED_FLAG_SKIP_COLLECTION, connector_false);
			}
            /* intentional fall through */
		case connector_request_id_remote_config_group_instances_set:
		case connector_request_id_remote_config_group_instance_remove:
		case connector_request_id_remote_config_group_start:
		case connector_request_id_remote_config_list_instances_set:
		case connector_request_id_remote_config_list_instance_remove:
		case connector_request_id_remote_config_list_start:
			if (get_list_depth(rci) <= rci->output.skip_depth && !RCI_SHARED_FLAG_IS_SET(rci, RCI_SHARED_FLAG_SKIP_COLLECTION))
			{
				rci->output.skip_depth = INVALID_DEPTH;
			}
			/* intentional fall through */
		case connector_request_id_remote_config_group_end:
		case connector_request_id_remote_config_group_instances_unlock:
		case connector_request_id_remote_config_list_end:
		case connector_request_id_remote_config_list_instances_unlock:
        case connector_request_id_remote_config_element_process:
            rci->output.element_skip = connector_false;
            remote_config->error_id = connector_success;
            remote_config->response.compare_matches = connector_false;
            remote_config->response.error_hint = NULL;
            callback_data = remote_config;
            break;

        case connector_request_id_remote_config_session_cancel:
        {
            remote_cancel.user_context = remote_config->user_context;
            callback_data =  &remote_cancel;
            break;
        }
#if (defined RCI_LEGACY_COMMANDS)
        case connector_request_id_remote_config_do_command:
        case connector_request_id_remote_config_reboot:
        case connector_request_id_remote_config_set_factory_def:
            remote_config->error_id = connector_success;
            remote_config->response.compare_matches = connector_false;
            callback_data = remote_config;
            break;
#endif
    }

	if (get_list_depth(rci) < rci->output.skip_depth ||
		(get_list_depth(rci) == rci->output.skip_depth &&
			((RCI_SHARED_FLAG_IS_SET(rci, RCI_SHARED_FLAG_SKIP_COLLECTION) &&
				(remote_config_request == connector_request_id_remote_config_list_instances_unlock || remote_config_request == connector_request_id_remote_config_group_instances_unlock))
			||	(remote_config_request == connector_request_id_remote_config_list_end && remote_config_request != connector_request_id_remote_config_group_end))))
	{
		rci->callback.status = connector_callback(rci->service_data->connector_ptr->callback,
												  connector_class_id_remote_config,
												  rci->callback.request,
												  callback_data,
												  rci->service_data->connector_ptr->context);
	}
	else
	{
        rci->callback.status = connector_callback_continue;
    }		

    switch (remote_config_request)
    {
#if (defined RCI_LEGACY_COMMANDS)

        case connector_request_id_remote_config_reboot:
        {
            if (rci->callback.status == connector_callback_continue && remote_config->error_id == connector_success) 
            {
                connector_request_id_t request_id;
                connector_data_t * const connector_ptr = rci->service_data->connector_ptr;

                request_id.os_request = connector_request_id_os_reboot;
                rci->callback.status = connector_callback(connector_ptr->callback, connector_class_id_operating_system, request_id, NULL, connector_ptr->context);
            }
        }
        /* fall through */
#endif
		case connector_request_id_remote_config_group_instances_lock:
		case connector_request_id_remote_config_group_instances_set:
		case connector_request_id_remote_config_list_instances_lock:
		case connector_request_id_remote_config_list_instances_set:
#if (defined RCI_LEGACY_COMMANDS)
        case connector_request_id_remote_config_do_command:
        case connector_request_id_remote_config_set_factory_def:
#endif
            if (remote_config->error_id != connector_success)
            {
                rci_global_error(rci, remote_config->error_id, remote_config->response.error_hint);
                set_rci_command_error(rci);
                state_call(rci, rci_parser_state_error);
            }
            break;
		case connector_request_id_remote_config_group_instance_remove:
		case connector_request_id_remote_config_group_instances_unlock:
		case connector_request_id_remote_config_list_instance_remove:
		case connector_request_id_remote_config_list_instances_unlock:
			/* What happens if there is an error in any of the four callbacks above? Do we care? */
        case connector_request_id_remote_config_session_start:
        case connector_request_id_remote_config_session_end:
        case connector_request_id_remote_config_action_start:
        case connector_request_id_remote_config_action_end:
        case connector_request_id_remote_config_group_start:
        case connector_request_id_remote_config_element_process:
#if (defined RCI_PARSER_USES_LIST)
		case connector_request_id_remote_config_list_start:
		case connector_request_id_remote_config_list_end:
#endif
        case connector_request_id_remote_config_group_end:
        case connector_request_id_remote_config_session_cancel:
            break;
    }

    switch (rci->callback.status)
    {
    case connector_callback_abort:
        callback_complete = connector_false;
        rci->status = rci_status_error;
        break;

    case connector_callback_continue:
        callback_complete = connector_true;

        if (remote_config->response.compare_matches)
        {
            ASSERT(rci->shared.callback_data.action == connector_remote_action_query);
            ASSERT(rci->shared.callback_data.group.type == connector_remote_group_setting);
            ASSERT(rci->shared.callback_data.attribute.compare_to != rci_query_setting_attribute_compare_to_none);

            switch (remote_config_request)
            {
                case connector_request_id_remote_config_element_process:
                    rci->output.element_skip = connector_true;
                    break;

                case connector_request_id_remote_config_group_start:
                    rci->output.skip_depth = 0;
                    break;

#if (defined RCI_PARSER_USES_LIST)
                case connector_request_id_remote_config_list_start:
                    rci->output.skip_depth = get_list_depth(rci);
                    break;
#endif

                default:
                    /* Invalid error_id for this callback */
                    ASSERT(0);
                    break;
            }
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

#if (defined RCI_PARSER_USES_COLLECTION_NAMES) || (defined RCI_PARSER_USES_ELEMENT_NAMES)
        switch (remote_config_request)
        {
			case connector_request_id_remote_config_group_instances_set:
				if (remote_config->error_id != connector_success || get_list_depth(rci) >= rci->output.skip_depth)
				{
					rci->shared.group.info.keys.count = 0;
				}
				break;
            case connector_request_id_remote_config_group_instances_lock:
				if (remote_config->group.collection_type == connector_collection_type_variable_array)
				{
		            if (!should_set_count(rci) || remote_config->response.item.count == rci->shared.group.info.keys.count ||
						(RCI_SHARED_FLAG_IS_SET(rci, RCI_SHARED_FLAG_DONT_SHRINK) && remote_config->response.item.count > rci->shared.group.info.keys.count))
					{
						if (remote_config->error_id == connector_success && get_list_depth(rci) < rci->output.skip_depth)
							rci->shared.group.info.keys.count = remote_config->response.item.count;
						else
							rci->shared.group.info.keys.count = 0;
						SET_RCI_SHARED_FLAG(rci, RCI_SHARED_FLAG_SET_COUNT, connector_false);
					}
					SET_RCI_SHARED_FLAG(rci, RCI_SHARED_FLAG_DONT_SHRINK, connector_false);
				}
				else
				{
					if (remote_config->error_id == connector_success && get_list_depth(rci) < rci->output.skip_depth)
					{
						rci->shared.group.info.keys.count = remote_config->response.item.dictionary.entries;
						rci->shared.group.info.keys.list = remote_config->response.item.dictionary.keys;
					}
					else
					{
						rci->shared.group.info.keys.count = 0;
						rci->shared.group.info.keys.list = NULL;
					}
				}
				rci->shared.group.lock = get_group_id(rci);
				break;
			case connector_request_id_remote_config_group_instances_unlock:
#if (defined RCI_PARSER_USES_COLLECTION_NAMES)
                rci->shared.callback_data.group.name = NULL;
#endif
				rci->shared.group.lock = INVALID_ID;
				break;
            case connector_request_id_remote_config_group_end:
                break;
            case connector_request_id_remote_config_element_process:
#if (defined RCI_PARSER_USES_ELEMENT_NAMES)
                rci->shared.callback_data.element.name = NULL;
#endif
                break;
#if (defined RCI_PARSER_USES_LIST)
			case connector_request_id_remote_config_list_instances_set:
				if (remote_config->error_id != connector_success || get_list_depth(rci) >= rci->output.skip_depth)
				{
					set_current_list_count(rci, 0);
				}
				break;
			case connector_request_id_remote_config_list_instances_lock:
			{
				if (remote_config->list.level[remote_config->list.depth - 1].collection_type == connector_collection_type_variable_array)
				{
		            if (!should_set_count(rci) || remote_config->response.item.count == get_current_list_count(rci) ||
						(RCI_SHARED_FLAG_IS_SET(rci, RCI_SHARED_FLAG_DONT_SHRINK) && remote_config->response.item.count > get_current_list_count(rci)))
					{
						if (remote_config->error_id == connector_success && get_list_depth(rci) < rci->output.skip_depth)
							set_current_list_count(rci, remote_config->response.item.count);
						else
							set_current_list_count(rci, 0);
						SET_RCI_SHARED_FLAG(rci, RCI_SHARED_FLAG_SET_COUNT, connector_false);
					}
					SET_RCI_SHARED_FLAG(rci, RCI_SHARED_FLAG_DONT_SHRINK, connector_false);
				}
				else
				{
					if (remote_config->error_id == connector_success && get_list_depth(rci) < rci->output.skip_depth)
					{
						set_current_list_count(rci, remote_config->response.item.dictionary.entries);
						set_current_list_key_list(rci, remote_config->response.item.dictionary.keys);
					}
					else
					{
						set_current_list_count(rci, 0);
						set_current_list_key_list(rci, NULL);
					}
				}
				set_current_list_lock(rci, get_current_list_id(rci));
				break;
			}
			case connector_request_id_remote_config_list_start:
			case connector_request_id_remote_config_list_end:
				break;
			case connector_request_id_remote_config_list_instances_unlock:
#if (defined RCI_PARSER_USES_COLLECTION_NAMES)
                rci->shared.callback_data.list.level[rci->shared.callback_data.list.depth - 1].name = NULL;
#endif
				invalidate_list_lock(rci, rci->shared.callback_data.list.depth - 1);
				break;
#endif
            case connector_request_id_remote_config_session_start:
            case connector_request_id_remote_config_session_end:
            case connector_request_id_remote_config_action_start:
            case connector_request_id_remote_config_action_end:
            case connector_request_id_remote_config_group_start:
            case connector_request_id_remote_config_session_cancel:
#if (defined RCI_LEGACY_COMMANDS)
            case connector_request_id_remote_config_do_command:
            case connector_request_id_remote_config_reboot:
            case connector_request_id_remote_config_set_factory_def:
#endif
                break;
        }
#endif
    }

    return callback_complete;
}


