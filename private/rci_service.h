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
#define CONNECTOR_RCI_PARSER_INTERNAL_DATA
#include "remote_config.h"


#if (defined CONNECTOR_BINARY_RCI_SERVICE)
#include "rci_binary.h"
#else
#include "rci_parser.h"
#endif

static uint32_t rci_get_firmware_target_zero_version(void)
{
#if (defined CONNECTOR_BINARY_RCI_SERVICE)
    return connector_rci_config_data.firmware_target_zero_version;
#else
    return FIRMWARE_TARGET_ZERO_VERSION;
#endif
}

static void set_rci_service_error(msg_service_request_t * const service_request, connector_msg_error_t const error_code)
{
    service_request->error_value = error_code;
    service_request->service_type = msg_service_type_error;
}

static connector_status_t rci_service_callback(connector_data_t * const connector_ptr, msg_service_request_t * const service_request)
{
    connector_status_t status = connector_working;
    connector_msg_error_t error_status = connector_msg_error_none;
    msg_session_t * session;
    rci_service_data_t * service_data;

    ASSERT_GOTO(connector_ptr != NULL, done);
    ASSERT_GOTO(service_request != NULL, done);

    session = service_request->session;
    ASSERT_GOTO(session != NULL, done);

    service_data = session->service_context;

    switch (service_request->service_type)
    {
    case msg_service_type_need_data:
    case msg_service_type_have_data:
    {
        rci_status_t rci_status;
        rci_session_t parser_action = rci_session_active;

        ASSERT_GOTO(service_request->have_data != NULL, done);
        ASSERT_GOTO(service_request->need_data != NULL, done);

        if (session->service_context == NULL)
        {
            /* 1st time here so let's allocate service context memory for rci parser */
            void * ptr;

            status = malloc_data_buffer(connector_ptr, sizeof *service_data, named_buffer_id(msg_service), &ptr);
            if (status != connector_working)
            {
                if (status != connector_pending)
                {
                    error_status = connector_msg_error_cancel;
                }
                goto done;
            }

            service_data = ptr;
            session->service_context = service_data;

            MsgSetStart(service_request->need_data->flags);

#if (defined CONNECTOR_BINARY_RCI_SERVICE)
            connector_debug_hexvalue("rci_service.request", service_request->have_data->data_ptr, service_request->have_data->length_in_bytes);
#else
            connector_debug_printf("rci_service.request: %d %.*s\n", service_request->have_data->length_in_bytes, service_request->have_data->length_in_bytes, service_request->have_data->data_ptr);
#endif
            parser_action = rci_session_start;
        }

        ASSERT(service_data != NULL);

        service_data->connector_ptr = connector_ptr;

        service_data->input.data = service_request->have_data->data_ptr;
        service_data->input.bytes = service_request->have_data->length_in_bytes;

        service_data->output.data = service_request->need_data->data_ptr;
        service_data->output.bytes = service_request->need_data->length_in_bytes;

#if (defined CONNECTOR_BINARY_RCI_SERVICE)
        service_data->input.flags = service_request->have_data->flags;
        service_data->output.flags = service_request->need_data->flags;
#endif
        rci_status = rci_parser(parser_action, service_data);

        switch (rci_status)
        {
        case rci_status_complete:
            status = connector_working;
            MsgSetLastData(service_request->need_data->flags);
            MsgSetSendNow(service_request->need_data->flags);
#if (defined CONNECTOR_BINARY_RCI_SERVICE)
            connector_debug_hexvalue("rci_service.response", service_data->output.data, service_data->output.bytes);
#else
            connector_debug_printf("rci_service.response: %d %.*s\n", service_data->output.bytes, service_data->output.bytes, service_data->output.data);
#endif
            service_request->need_data->length_in_bytes = service_data->output.bytes;
            break;

        case rci_status_busy:
            status = connector_pending;
            break;

        case rci_status_more_input:
            status = connector_working;
            break;

        case rci_status_flush_output:
            MsgSetSendNow(service_request->need_data->flags);
#if (defined CONNECTOR_BINARY_RCI_SERVICE)
            connector_debug_hexvalue("rci_service.response", service_data->output.data, service_data->output.bytes);
#else
            connector_debug_printf("rci_service.response: %d %.*s\n", service_data->output.bytes, service_data->output.bytes, service_data->output.data);
#endif
            service_request->need_data->length_in_bytes = service_data->output.bytes;
            status = connector_active;
            break;

        case rci_status_error:
            status = connector_abort;

            /* no break; */
        case rci_status_internal_error:
            error_status = connector_msg_error_cancel;
            break;
        }
        break;
    }
    case msg_service_type_error:
    {
        rci_status_t const rci_status = rci_parser(rci_session_lost, service_data);

        switch (rci_status)
        {
        case rci_status_complete:
            status = connector_working;
            break;

        case rci_status_busy:
            status = connector_pending;
            break;

        case rci_status_more_input:
        case rci_status_flush_output:
            ASSERT(connector_false);
            break;

        case rci_status_error:
            status = connector_abort;
            break;

        case rci_status_internal_error:
            /* just ignore */
            break;

        }
        break;
    }
    case msg_service_type_free:
        status = free_data_buffer(connector_ptr, named_buffer_id(msg_service), session->service_context);
        break;

    default:
        ASSERT(connector_false);
        break;

    }

done:
    if (error_status != connector_msg_error_none)
    {
        set_rci_service_error(service_request, error_status);
    }

    return status;
}

static connector_status_t connector_facility_rci_service_cleanup(connector_data_t * const connector_ptr)
{
#if (defined CONNECTOR_BINARY_RCI_SERVICE)
    msg_service_id_t const service_id = msg_service_id_brci;
#else
    msg_service_id_t const service_id = msg_service_id_rci;
#endif
    return msg_cleanup_all_sessions(connector_ptr, service_id);
}

static connector_status_t connector_facility_rci_service_delete(connector_data_t * const connector_ptr)
{
#if (defined CONNECTOR_BINARY_RCI_SERVICE)
    msg_service_id_t const service_id = msg_service_id_brci;
#else
    msg_service_id_t const service_id = msg_service_id_rci;
#endif
    return msg_delete_facility(connector_ptr, service_id);
}

static connector_status_t connector_facility_rci_service_init(connector_data_t * const connector_ptr, unsigned int const facility_index)
{
    connector_status_t result;

#if (defined CONNECTOR_BINARY_RCI_SERVICE)
    msg_service_id_t const service_id = msg_service_id_brci;
#else
    msg_service_id_t const service_id = msg_service_id_rci;
#endif

#if defined connector_remote_config_configurations
    {

        connector_request_t request_id;
        connector_callback_status_t callback_status;

        request_id.remote_config_request = (connector_remote_config_request_t)connector_remote_config_configurations;
        callback_status = connector_callback_no_request_data(connector_ptr->callback, connector_class_remote_config_service, request_id,
                                                &connector_rci_config_data, NULL);
        switch (callback_status)
        {
            case connector_callback_unrecognized:
                result = connector_idle;
                goto done;

            case connector_callback_continue:
                ASSERT(connector_rci_config_data.group_table != NULL);
                if (connector_rci_config_data.global_error_count < connector_rci_error_COUNT) connector_rci_config_data.global_error_count = connector_rci_error_COUNT;
                break;

            default:
                result = connector_abort;
                goto done;

        }

    }
#endif

    result = msg_init_facility(connector_ptr, facility_index, service_id, rci_service_callback);

#if defined connector_remote_config_configurations
done:
#endif
    return result;
}
