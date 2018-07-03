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

STATIC void traverse_rci_command(rci_t * const rci)
{
    trigger_rci_callback(rci, connector_request_id_remote_config_action_start);

    set_rci_output_state(rci, rci_output_state_command_id);
    state_call(rci, rci_parser_state_output);

}
STATIC void traverse_group_id(rci_t * const rci)
{
    trigger_rci_callback(rci, connector_request_id_remote_config_group_start);

    set_rci_output_state(rci, rci_output_state_group_id);
    state_call(rci, rci_parser_state_output);
}

#if (defined RCI_PARSER_USES_LIST)
STATIC void traverse_list_id(rci_t * const rci)
{
	trigger_rci_callback(rci, connector_request_id_remote_config_list_start);
	invalidate_element_id(rci);
	set_rci_output_state(rci, rci_output_state_list_id);
    state_call(rci, rci_parser_state_output);
}
#endif

STATIC void traverse_element_id(rci_t * const rci)
{

    connector_item_t const * const element = get_current_element(rci);

#if (defined RCI_PARSER_USES_LIST)
	ASSERT(element->type != connector_element_type_list);
#endif

    if ((rci->shared.callback_data.action == connector_remote_action_query) &&
        (element->data.element->access == connector_element_access_write_only))
    {
        goto done;
    }

    trigger_rci_callback(rci, connector_request_id_remote_config_element_process);
    set_rci_output_state(rci, rci_output_state_field_id);
    state_call(rci, rci_parser_state_output);

done:
    return;
}

#if (defined RCI_LEGACY_COMMANDS)
STATIC void traverse_command_do_command(rci_t * const rci)
{
    trigger_rci_callback(rci, connector_request_id_remote_config_do_command);

    set_rci_output_state(rci, rci_output_state_do_command_payload);
    state_call(rci, rci_parser_state_output);

    return;
}

STATIC void traverse_command_reboot(rci_t * const rci)
{
    trigger_rci_callback(rci, connector_request_id_remote_config_reboot);

    state_call(rci, rci_parser_state_input);

    return;
}

STATIC void traverse_command_set_factory_default(rci_t * const rci)
{
    trigger_rci_callback(rci, connector_request_id_remote_config_set_factory_def);

    state_call(rci, rci_parser_state_input);

    return;
}
#endif

STATIC void traverse_element_end(rci_t * const rci)
{
#if (defined RCI_PARSER_USES_LIST)
	if (get_list_depth(rci) > 0)
	{
		trigger_rci_callback(rci, connector_request_id_remote_config_list_end);
		set_element_id(rci, get_current_list_id(rci));
		/* traverse_all_element_instances() needs the list info so don't invalidate it if we will be transitioning to that state */
		if (get_list_depth(rci) < get_query_depth(rci) || (get_list_depth(rci) == get_query_depth(rci) && !should_traverse_all_list_instances(rci))) 
		{
			invalidate_current_list_id(rci);
			invalidate_current_list_index(rci);
			decrement_list_depth(rci);
		}
	}
	else
#endif
	{
    	trigger_rci_callback(rci, connector_request_id_remote_config_group_end);
	}

    set_rci_output_state(rci, rci_output_state_field_terminator);
    state_call(rci, rci_parser_state_output);
}

STATIC void traverse_group_end(rci_t * const rci)
{
    trigger_rci_callback(rci, connector_request_id_remote_config_action_end);
    set_rci_output_state(rci, rci_output_state_group_terminator);
    state_call(rci, rci_parser_state_output);
}

#if (defined RCI_PARSER_USES_LIST)
STATIC connector_bool_t traverse_all_list_instances(rci_t * const rci)
{
	connector_bool_t done = connector_false;

	if (!have_current_list_index(rci))
	{
		set_current_list_index(rci, 1);
	}
	else
	{
		connector_collection_t const * const info = get_current_collection_info(rci);
		if (get_current_list_count(rci) > get_current_list_index(rci))
        {
            /* next instances */
            increment_current_list_index(rci);
        }
		else
		{
			if (get_list_depth(rci) > get_query_depth(rci))
			{
				set_rci_traverse_state(rci, rci_traverse_state_all_elements);
			}
			else
			{
            	done = connector_true;
			}
			set_element_id(rci, get_current_list_id(rci));
			invalidate_current_list_id(rci);
			invalidate_current_list_index(rci);
			decrement_list_depth(rci);
			goto done;
		}
	}

	traverse_list_id(rci);
	set_rci_traverse_state(rci, rci_traverse_state_all_elements);

done:
	return done;
}
#endif

STATIC void traverse_element(rci_t * const rci)
{
#if (defined RCI_PARSER_USES_LIST)
    connector_item_t const * const element = get_current_element(rci);
	if (element->type == connector_element_type_list)
	{
		increment_list_depth(rci);
		set_current_list_id(rci, get_element_id(rci));
		invalidate_current_list_index(rci);
		traverse_all_list_instances(rci);
	}
	else
#endif
	{
		traverse_element_id(rci);
	}
}

STATIC connector_bool_t traverse_all_elements(rci_t * const rci)
{
    connector_bool_t done = connector_false;

	if (!have_element_id(rci))
	{
		set_element_id(rci, 0);
	}
	else
	{
		connector_collection_t const * const info = get_current_collection_info(rci);
        unsigned int const id = get_element_id(rci) + 1;

        if (id < info->item.count)
        {
            set_element_id(rci, id);
        }
        else
        {
#if (defined RCI_PARSER_USES_LIST)
			ASSERT(get_list_depth(rci) >= get_query_depth(rci));
			if (get_list_depth(rci) == 0)
			{
#endif
				if (should_traverse_all_group_instances(rci) || should_traverse_all_groups(rci))
				{
					set_rci_traverse_state(rci, rci_traverse_state_all_group_instances);
				}
				else
				{
            		done = connector_true;
				}
#if (defined RCI_PARSER_USES_LIST)
			}
			else if (get_list_depth(rci) >= get_query_depth(rci))
			{
				if (rci->shared.flag != 0 || get_list_depth(rci) > get_query_depth(rci))
					set_rci_traverse_state(rci, rci_traverse_state_all_list_instances);
				else
					done = connector_true;
			}
#endif
			traverse_element_end(rci);
			
			goto done;
        }
	}
	
	traverse_element(rci);

done:
	return done;
}

STATIC connector_bool_t traverse_all_group_instances(rci_t * const rci)
{
    connector_bool_t done = connector_false;
	
	if (!have_group_index(rci))
	{
		set_group_index(rci, 1);
	}
	else
	{
		if (rci->shared.group.info.keys.count > get_group_index(rci))
        {
            /* next instances */
            increment_group_index(rci);
        }
		else
		{
			if (should_traverse_all_groups(rci))
			{
				set_rci_traverse_state(rci, rci_traverse_state_all_groups);
			}
			else
			{
				done = connector_true;
			}
			goto done;
		}
	}

	invalidate_element_id(rci);
	traverse_group_id(rci);
	set_rci_traverse_state(rci, rci_traverse_state_all_elements);

done:
	return done;
}

STATIC connector_bool_t traverse_all_groups(rci_t * const rci)
{
    connector_bool_t done = connector_false;

    connector_remote_config_data_t const * const rci_data = rci->service_data->connector_ptr->rci_data;
    connector_remote_group_table_t const * const table = rci_data->group_table + rci->shared.callback_data.group.type;

    if (table->count == 0)
    {
        traverse_group_end(rci);
        goto done;
    }
    else if (!have_group_id(rci))
    {
        set_group_id(rci, 0);
    }
    else
    {
        unsigned int const id = get_group_id(rci) + 1;
        if (id == table->count)
        {
            /* done all groups */
            traverse_group_end(rci);
            done = connector_true;
			goto done;
        }
        else
        {
            /* next group */
            increment_group_id(rci);
        }
    }

	invalidate_group_index(rci);
	traverse_all_group_instances(rci);

done:
    return done;
}

STATIC void rci_traverse_data(rci_t * const rci)
{
	connector_bool_t done_state = connector_true;
	
#if (defined RCI_DEBUG)
    connector_debug_line("traverse: %s", rci_traverse_state_t_as_string(rci->traverse.state));
#if (defined RCI_PARSER_USES_LIST)
	connector_debug_line("flags: all_groups: %u, all_group_instances: %u, all_list_instances: %u; list depth: %u", 
						 should_traverse_all_groups(rci), should_traverse_all_group_instances(rci), should_traverse_all_list_instances(rci),
						 get_list_depth(rci));
#else
	connector_debug_line("flags: all_groups: %u, all_group_instances: %u", should_traverse_all_groups(rci), should_traverse_all_group_instances(rci));
#endif
#endif

    switch (rci->traverse.state)
    {
        case rci_traverse_state_none:
            if (get_rci_input_state(rci) == rci_input_state_done)
                state_call(rci, rci_parser_state_output);
            else
                state_call(rci, rci_parser_state_input);
            break;

        case rci_traverse_state_command_id:
            traverse_rci_command(rci);
            break;

        case rci_traverse_state_all_groups:
            done_state = traverse_all_groups(rci);
            break;

        case rci_traverse_state_group_id:
            traverse_group_id(rci);
            break;

        case rci_traverse_state_all_group_instances:
            done_state = traverse_all_group_instances(rci);
            break;

#if (defined RCI_PARSER_USES_LIST)
		case rci_traverse_state_list_id:
			traverse_list_id(rci);
			break;

		case rci_traverse_state_all_list_instances:
			done_state = traverse_all_list_instances(rci);
			break;
#endif

        case rci_traverse_state_element_id:
            traverse_element_id(rci);
            break;

        case rci_traverse_state_all_elements:
            done_state = traverse_all_elements(rci);
            break;

        case rci_traverse_state_element_end:
            traverse_element_end(rci);
            break;

        case rci_traverse_state_group_end:
            traverse_group_end(rci);
            break;
#if (defined RCI_LEGACY_COMMANDS)
        case rci_traverse_state_command_do_command:
            traverse_command_do_command(rci);
            break;
        case rci_traverse_state_command_reboot:
            traverse_command_reboot(rci);
            break;
        case rci_traverse_state_command_set_factory_default:
            traverse_command_set_factory_default(rci);
            break;
#endif
    }

    if (done_state)
    {
        set_rci_traverse_state(rci, rci_traverse_state_none);
    }

    return;
}

