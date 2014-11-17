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

#include "rci_binary_support.h"
#include "rci_binary_debug.h"
#include "rci_binary_util.h"
#include "rci_binary_buffer.h"
#include "rci_binary_string.h"
#include "rci_binary_command.h"
#include "rci_binary_group.h"
#include "rci_binary_element.h"
#include "rci_binary_callback.h"
#include "rci_binary_output.h"
#include "rci_binary_input.h"
#include "rci_binary_traverse.h"
#include "rci_binary_error.h"

STATIC connector_bool_t rci_action_session_start(rci_t * const rci, rci_service_data_t * service_data)
{
    ASSERT(rci->service_data == NULL);
    rci->service_data = service_data;
    ASSERT(rci->service_data != NULL);

    rci_set_buffer(&rci->buffer.input, &rci->service_data->input);
    rci_set_buffer(&rci->buffer.output, &rci->service_data->output);
#if defined RCI_DEBUG
    memset(rci->service_data->output.data, 0, rci->service_data->output.bytes);
#endif

    rci->input.destination = rci_buffer_position(&rci->buffer.input);
    reset_input_content(rci);

    invalidate_group_id(rci);
    invalidate_group_index(rci);
    invalidate_element_id(rci);

    rci->shared.callback_data.response.element_value = &rci->shared.value;

    rci->status = rci_status_busy;
    rci->error.command_error = connector_false;
    rci->input.flag = 0;

    trigger_rci_callback(rci, rci_command_callback_set_query_setting_state, connector_request_id_remote_config_session_start);
    set_rci_input_state(rci, rci_input_state_command_id);
    state_call(rci, rci_parser_state_input);

#if (!defined CONNECTOR_NO_MALLOC)
    {
        static char const max_mac_value[] = "FF:FF:FF:FF:FF:FF";
        size_t const rci_input_start_size = sizeof max_mac_value;
        connector_data_t * const connector_ptr = rci->service_data->connector_ptr;
        connector_status_t const connector_status = malloc_data(connector_ptr, rci_input_start_size, (void **)&rci->input.storage);

        switch (connector_status)
        {
            case connector_working:
                break;
            default:
                ASSERT(connector_status == connector_working);
                break;
        }
        rci->input.storage_len = rci_input_start_size;
    }
#endif

    set_rci_error_state(rci, rci_error_state_id);

    return connector_true;
}

STATIC connector_bool_t rci_action_session_active(rci_t * const rci)
{
    connector_bool_t success = connector_true;

    switch (rci->status)
    {
        case rci_status_error:
        case rci_status_complete:
        {
            rci->status = rci_status_internal_error;
            /* no break; */
        }

        case rci_status_internal_error:
        {
            success = connector_false;
            break;
        }

        case rci_status_busy:
        {
            break;
        }

        case rci_status_more_input:
        {
            connector_debug_print_buffer("rci_binary more request", rci->service_data->input.data, rci->service_data->input.bytes);
            rci_set_buffer(&rci->buffer.input, &rci->service_data->input);
            if (!destination_in_storage(rci))
            {
                rci->input.destination = rci_buffer_position(&rci->buffer.input);
                reset_input_content(rci);
            }
            rci->status = rci_status_busy;
            break;
        }

        case rci_status_flush_output:
        {
            rci_set_buffer(&rci->buffer.output, &rci->service_data->output);
#if defined RCI_DEBUG
            memset(rci->service_data->output.data, 0, rci->service_data->output.bytes);
#endif
            rci->status = rci_status_busy;
            break;
        }
    }

    return success;
}

STATIC connector_bool_t rci_action_session_lost(rci_t * const rci)
{
    trigger_rci_callback(rci, rci_command_callback_set_query_setting_state, connector_request_id_remote_config_session_cancel);
    {
        connector_bool_t const success = rci_callback(rci);
        ASSERT(success); UNUSED_VARIABLE(success);
    }

    rci->status = rci_status_complete;

    return connector_false;
}


STATIC rci_status_t rci_binary(connector_data_t * const connector_ptr, rci_session_t const action, rci_service_data_t * service_data)
{
#if (defined CONNECTOR_NO_MALLOC)
    static rci_t rci;
    rci_t * p_rci = &rci;

    UNUSED_PARAMETER(connector_ptr);
#else
    rci_t * p_rci = connector_ptr->p_rci;
#endif

    {
        connector_bool_t success;

        switch (action)
        {
        case rci_session_start:
            if (p_rci == NULL)
            {

#if (!defined CONNECTOR_NO_MALLOC)
                connector_status_t const connector_status = malloc_data(connector_ptr, sizeof *p_rci, (void **)&p_rci);

                switch (connector_status)
                {
                    case connector_working:
                        connector_ptr->p_rci = p_rci;
                        break;
                    default:
                        ASSERT(connector_status == connector_working);
                        break;
                }
#endif
            }

            ASSERT(p_rci != NULL);
            success = rci_action_session_start(p_rci, service_data);
            break;

        case rci_session_active:
            ASSERT(p_rci != NULL);
            success = rci_action_session_active(p_rci);
            break;

        case rci_session_lost:
            ASSERT(p_rci != NULL);
            success = rci_action_session_lost(p_rci);
            break;

        default:
            success = connector_false;
            break;
        }

        if (!success) goto done;
    }

    if (pending_rci_callback(p_rci))
    {
        connector_remote_config_t * const remote_config = &p_rci->shared.callback_data;

        if (!rci_callback(p_rci))
            goto done;

        if (remote_config->error_id != connector_success)
        {
            rci_group_error(p_rci, remote_config->error_id, remote_config->response.error_hint);
            goto done;
        }
    }

    switch (p_rci->parser.state)
    {
    case rci_parser_state_input:
        rci_parse_input(p_rci);
        break;

    case rci_parser_state_output:
        rci_generate_output(p_rci);
        break;

    case rci_parser_state_traverse:
        rci_traverse_data(p_rci);
        break;

    case rci_parser_state_error:
        rci_generate_error(p_rci);
        break;
    }

done:

    switch (p_rci->status)
    {
    case rci_status_busy:
        break;
    case rci_status_more_input:
        connector_debug_line("Need more input");
        break;
    case rci_status_flush_output:
        p_rci->service_data->output.bytes = rci_buffer_used(&p_rci->buffer.output);
        break;
    case rci_status_complete:
        p_rci->service_data->output.bytes = rci_buffer_used(&p_rci->buffer.output);
        /* no break; */
    case rci_status_internal_error:
    case rci_status_error:
        p_rci->service_data = NULL;
        break;
    }

    return p_rci->status;
}

