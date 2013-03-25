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

/*  This was moved here to correct a software engineering defect.
 * These defines need to be private
 */
#define CONNECTOR_VERSION_1100   0x01010000UL
#define CONNECTOR_VERSION_1200   0x01020000UL
#define CONNECTOR_VERSION_1300   0x01030000UL

#include <stddef.h>

#include "connector_config.h"
#include "connector_api.h"

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200)
#include "connector_debug.h"
#endif

#include "connector_info.h"
#include "connector_def.h"

#if (CONNECTOR_VERSION <= CONNECTOR_VERSION_1200) && (!defined CONNECTOR_TRANSPORT_TCP)
#define CONNECTOR_TRANSPORT_TCP
#endif

#include "chk_config.h"
#include "bele.h"
#include "os_intf.h"

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
static connector_status_t connector_stop_callback(connector_data_t * const connector_ptr, connector_transport_t const transport, void * const user_context);
#endif

#if (defined CONNECTOR_DATA_POINTS)
#include "connector_data_point.h"
#endif

#if (defined CONNECTOR_TRANSPORT_TCP)
#include "connector_edp.h"
#endif

#if (defined CONNECTOR_TRANSPORT_UDP) || (defined CONNECTOR_TRANSPORT_SMS)
#include "connector_sm.h"
#endif

#ifdef CONNECTOR_NO_MALLOC
#include "connector_static_buffer.h"
#endif

#define DEVICE_ID_LENGTH 16

#define MAX_DECIMAL_DIGIT       9
#define MAX_HEXDECIMAL_DIGIT    0xF

static char const connector_signature[] = CONNECTOR_SW_VERSION;
static uint8_t connector_device_id[DEVICE_ID_LENGTH];
static connector_bool_t connector_got_device_id = connector_false;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200)
static connector_bool_t check_digit_array(uint8_t * const digits, size_t const length, uint8_t const max_digit)
{
    connector_bool_t isDigit = connector_true;
    size_t i;

    for (i=0; i < length; i++)
    {
        unsigned char const up_digit = (digits[i] >> 4) & 0xF;
        unsigned char const lo_digit = digits[i] & 0x0F;

        if (up_digit > max_digit || lo_digit > max_digit)
        {
            isDigit = connector_false;
            break;
        }
    }

    return isDigit;
}
#endif

static connector_status_t get_config_parameter(connector_data_t * const connector_ptr, connector_config_request_t const request)
{
    connector_status_t result = connector_working;
    connector_callback_status_t callback_status;
    connector_request_t request_id;
    void * store_at;
#if (!defined CONNECTOR_CONNECTION_TYPE) || (!defined CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND)
    void * pointer_data;
#endif
    size_t length = 0;

    request_id.config_request = request;
    switch (request)
    {
#if (!defined CONNECTOR_CONNECTION_TYPE)
        case connector_config_connection_type:
            store_at = &pointer_data;
            length = sizeof connector_ptr->connection_type;
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200) && (!defined CONNECTOR_DEVICE_ID_METHOD)
        case connector_config_device_id_method:
            store_at = &connector_ptr->device_id_method;
            length = sizeof connector_ptr->device_id_method;
            break;
#endif
        case connector_config_device_id:
            store_at = &connector_ptr->device_id;
            break;

        case connector_config_mac_addr:
            store_at = &connector_ptr->mac_addr;
            break;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200)
        case connector_config_imei_number:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_config_esn:
        case connector_config_meid:
            store_at = &connector_ptr->wan_id;
            if (request == connector_config_imei_number)
                length = CONNECTOR_GSM_IMEI_LENGTH;

#else
        {
            static uint8_t imei_number[CONNECTOR_GSM_IMEI_LENGTH];
            connector_ptr->wan_id = (uint8_t *)&imei_number;

            length = CONNECTOR_GSM_IMEI_LENGTH;
            store_at = &connector_ptr->wan_id;
        }
#endif
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_config_wan_type:
            store_at = &connector_ptr->wan_type;
            length = sizeof connector_ptr->wan_type;
            break;
#endif

#if !(defined CONNECTOR_CLOUD_URL)
        case connector_config_server_url:
            store_at = &connector_ptr->device_cloud_url;
            length = SERVER_URL_LENGTH-1;
            break;
#endif

#if !(defined CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND)
        case connector_config_link_speed:
            store_at = &pointer_data;
            length = sizeof connector_ptr->link_speed;
            break;
#endif

#if !(defined CONNECTOR_WAN_PHONE_NUMBER_DIALED)
        case connector_config_phone_number:
            store_at = &connector_ptr->phone_dialed;
            break;
#endif

        default:
            ASSERT_GOTO(connector_false, done);
            break;
    }

    callback_status = connector_callback_no_request_data(connector_ptr->callback, connector_class_config, request_id, store_at, &length);
    switch (callback_status)
    {
    case connector_callback_continue:
        break;
    case connector_callback_busy:
        result = connector_pending;
        goto done;
    case connector_callback_unrecognized:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200) && (!defined CONNECTOR_DEVICE_ID_METHOD)
        if (request == connector_config_device_id_method)
        {
            break;
        }
#endif
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        if (request == connector_config_wan_type)
        {
            break;
        }
#endif

        /* fall thru */
    case connector_callback_abort:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
    case connector_callback_error:
#endif

        result = connector_abort;
        goto done;
    }

    switch (request)
    {
#if (!defined CONNECTOR_CONNECTION_TYPE)
        case connector_config_connection_type:
            if (length != sizeof connector_ptr->connection_type)
            {
                result = connector_invalid_data_size;
                goto error;
            }
            else if (pointer_data == NULL)
            {
                result = connector_invalid_data;
                goto error;
            }

            {
                connector_connection_type_t * connection_type = pointer_data;

                switch (*connection_type)
                {
                case connector_lan_connection_type:
                case connector_wan_connection_type:
                    connector_ptr->connection_type = *connection_type;
                    break;
                default:
                    result = connector_invalid_data_range;
                    break;
                }
            }
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200) && (!defined CONNECTOR_DEVICE_ID_METHOD)
        case connector_config_device_id_method:
            if (callback_status == connector_callback_unrecognized)
            {
                connector_ptr->device_id_method = connector_manual_device_id_method;
            }
            else if (length != sizeof connector_ptr->device_id_method)
            {
                result = connector_invalid_data_size;
            }
            switch (connector_ptr->device_id_method)
            {
            case connector_manual_device_id_method:
            case connector_auto_device_id_method:
                break;
            default:
                result = connector_invalid_data_range;
                break;
            }
            break;
#endif

        case connector_config_device_id:
            if (length != DEVICE_ID_LENGTH)
                result = connector_invalid_data_size;
            else if (connector_ptr->device_id == NULL)
                result = connector_invalid_data;
            break;

        case connector_config_mac_addr:
            if (length != MAC_ADDR_LENGTH)
            {
                result = connector_invalid_data_size;
            }
            else if (connector_ptr->mac_addr == NULL)
            {
                result = connector_invalid_data;
            }
            break;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200)
        case connector_config_imei_number:
            if (length != CONNECTOR_GSM_IMEI_LENGTH)
            {
                result = connector_invalid_data_size;
            }
            else if (connector_ptr->wan_id == NULL)
            {
                result = connector_invalid_data;
            }
            else if (check_digit_array(connector_ptr->wan_id, length, MAX_DECIMAL_DIGIT) != connector_true)
            {
                result = connector_invalid_data_range;
            }
            else
            {
                connector_ptr->wan_id_length = length;
            }
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_config_wan_type:
            if (callback_status == connector_callback_unrecognized)
            {
                connector_ptr->wan_type = connector_imei_wan_type;
            }

            switch (connector_ptr->wan_type)
            {
            case connector_imei_wan_type:
            case connector_esn_wan_type:
            case connector_meid_wan_type:
                break;
            default:
                result = connector_invalid_data_range;
                break;
            }
            break;
        case connector_config_meid:
        case connector_config_esn:
        {
            size_t hex_valid_length = (request == connector_config_esn)? CONNECTOR_ESN_HEX_LENGTH : CONNECTOR_MEID_HEX_LENGTH;

            if (connector_ptr->wan_id == NULL)
            {
                result = connector_invalid_data;
            }
            else if (length != hex_valid_length)
            {
                result = connector_invalid_data_size;
            }
            else
            {

                if (check_digit_array(connector_ptr->wan_id, length, MAX_HEXDECIMAL_DIGIT) != connector_true)
                {
                    result = connector_invalid_data_range;
                }
                else
                {
                    connector_ptr->wan_id_length = length;
                }
            }
            break;
        }
#endif

#if (!defined CONNECTOR_CLOUD_URL)
        case connector_config_server_url:
            if ((length == 0) || (length > SERVER_URL_LENGTH-1))
            {
                result =  connector_invalid_data_size;
            }
            else if (connector_ptr->device_cloud_url == NULL)
            {
                result = connector_invalid_data;
            }
            else
            {
                connector_ptr->device_cloud_url_length = length;
            }
            break;
#endif

#if (!defined CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND)
        case connector_config_link_speed:
        {
            uint32_t * const link_speed = pointer_data;

            if (link_speed == NULL)
            {
                result = connector_invalid_data;
            }
            else if (length != sizeof connector_ptr->link_speed)
            {
                result = connector_invalid_data_size;
            }
            else
            {
                connector_ptr->link_speed = *link_speed;
            }

            break;
        }
#endif

#if (!defined CONNECTOR_WAN_PHONE_NUMBER_DIALED)
        case connector_config_phone_number:
            if (connector_ptr->phone_dialed == NULL)
            {
                result = connector_invalid_data;
            }
            else if (length == 0)
            {
                result = connector_invalid_data_size;
            }
            else
            {
                connector_ptr->phone_dialed_length = length;
            }
            break;
#endif

        default:
            ASSERT_GOTO(connector_false, done);
            break;
    }

#if (!defined CONNECTOR_CONNECTION_TYPE)
error:
#endif
    if (result != connector_working)
    {
        notify_error_status(connector_ptr->callback, connector_class_config, request_id, result);
        result = connector_abort;
    }

done:
    return result;
}

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200)
static connector_status_t get_wan_device_id(connector_data_t * const connector_ptr, uint8_t * const device_id, connector_config_request_t config_request)
{

enum {
    imei_device_id_prefix = 1,
    esn_hex_device_id_prefix,
    esn_dec_device_id_prefix,
    meid_hex_device_id_prefix,
    meid_dec_device_id_prefix
};

    connector_status_t result;

    result = get_config_parameter(connector_ptr, config_request);
    if (result == connector_working)
    {
        uint8_t * dst = device_id + (DEVICE_ID_LENGTH - connector_ptr->wan_id_length);

#if (defined CONNECTOR_DEBUG) && (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        {
            char * wan_string = NULL;

            switch (connector_ptr->wan_type)
            {
            case connector_imei_wan_type:
                wan_string = "IMEI";
                break;
            case connector_esn_wan_type:
                wan_string = "ESN";
                break;
            case connector_meid_wan_type:
                wan_string = "MEID";
                break;
            }
            connector_debug_hexvalue(wan_string, connector_ptr->wan_id, connector_ptr->wan_id_length);
        }
#endif
        memcpy(dst, connector_ptr->wan_id, connector_ptr->wan_id_length);

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        switch (connector_ptr->wan_type)
        {
        case connector_imei_wan_type:
            device_id[1] = imei_device_id_prefix;
            break;
        case connector_esn_wan_type:
            device_id[1] = esn_hex_device_id_prefix;
            break;
        case connector_meid_wan_type:
            device_id[1] = meid_hex_device_id_prefix;
            break;
        }
#else
        device_id[1] = imei_device_id_prefix;
#endif

        connector_ptr->device_id = device_id;
    }

    return result;
}
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200)
static connector_status_t get_device_id_method(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

#if (defined CONNECTOR_DEVICE_ID_METHOD)
    UNUSED_PARAMETER(connector_ptr);
    connector_ptr->device_id_method = CONNECTOR_DEVICE_ID_METHOD;
#else
    result = get_config_parameter(connector_ptr, connector_config_device_id_method);
#endif

    return result;
}
#endif

static connector_status_t get_mac_addr(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

    if (connector_ptr->mac_addr == NULL)
    {
        result = get_config_parameter(connector_ptr, connector_config_mac_addr);
    }
    return result;
}

static connector_status_t get_connection_type(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
#if (defined CONNECTOR_CONNECTION_TYPE)
    connector_ptr->connection_type = CONNECTOR_CONNECTION_TYPE;
#else
    result = get_config_parameter(connector_ptr, connector_config_connection_type);
#endif
    return result;
}

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
static connector_status_t get_wan_type(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

#if (defined CONNECTOR_WAN_TYPE)
    connector_ptr->wan_type = CONNECTOR_WAN_TYPE;
#else
    result = get_config_parameter(connector_ptr, connector_config_wan_type);
#endif

    return result;
}
#endif

#if !(defined CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND)
static connector_status_t get_link_speed(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    result = get_config_parameter(connector_ptr, connector_config_link_speed);

    return result;
}
#endif

#if !(defined CONNECTOR_WAN_PHONE_NUMBER_DIALED)
static connector_status_t get_phone_number(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

    if (connector_ptr->phone_dialed == NULL)
    {
        result = get_config_parameter(connector_ptr, connector_config_phone_number);
    }

    return result;
}
#endif


#if (CONNECTOR_VERSION < CONNECTOR_VERSION_1200)
static connector_status_t manage_device_id(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

    if (connector_got_device_id)
    {
        goto done;
    }

    result = get_config_parameter(connector_ptr, connector_config_device_id);
    COND_ELSE_GOTO(result == connector_working, error);
    memcpy(connector_device_id, connector_ptr->device_id, sizeof connector_device_id);

done:
    connector_ptr->device_id = connector_device_id;
    connector_got_device_id = connector_true;
    connector_debug_hexvalue("Device ID ", connector_ptr->device_id, DEVICE_ID_LENGTH);

error:
    return result;
}

#else
static connector_status_t manage_device_id(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

    if (connector_got_device_id)
    {
        connector_ptr->device_id_method = connector_manual_device_id_method;
        goto done;
    }
    result = get_device_id_method(connector_ptr);
    COND_ELSE_GOTO(result == connector_working, error);

    switch (connector_ptr->device_id_method)
    {
        case connector_manual_device_id_method:
            result = get_config_parameter(connector_ptr, connector_config_device_id);
            COND_ELSE_GOTO(result == connector_working, error);
            memcpy(connector_device_id, connector_ptr->device_id, sizeof connector_device_id);
            break;

        case connector_auto_device_id_method:
        {
            result = get_connection_type(connector_ptr);
            COND_ELSE_GOTO(result == connector_working, error);

            switch (connector_ptr->connection_type)
            {
                case connector_lan_connection_type:
                {
                    result = get_mac_addr(connector_ptr);
                    COND_ELSE_GOTO(result == connector_working, error);

                    connector_device_id[8] = connector_ptr->mac_addr[0];
                    connector_device_id[9] = connector_ptr->mac_addr[1];
                    connector_device_id[10] = connector_ptr->mac_addr[2];
                    connector_device_id[11] = 0xFF;
                    connector_device_id[12] = 0xFF;
                    connector_device_id[13] = connector_ptr->mac_addr[3];
                    connector_device_id[14] = connector_ptr->mac_addr[4];
                    connector_device_id[15] = connector_ptr->mac_addr[5];
                }
                break;

                case connector_wan_connection_type:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
                    /* TODO: get wan type */
                    result = get_wan_type(connector_ptr);
                    COND_ELSE_GOTO(result == connector_working, error);

                    switch (connector_ptr->wan_type)
                    {
                    case connector_imei_wan_type:
                        result = get_wan_device_id(connector_ptr, connector_device_id, connector_config_imei_number);
                        break;
                    case connector_esn_wan_type:
                        result = get_wan_device_id(connector_ptr, connector_device_id, connector_config_esn);
                        break;
                    case connector_meid_wan_type:
                        result = get_wan_device_id(connector_ptr, connector_device_id, connector_config_meid);
                        break;
                    }
#else
                    result = get_wan_device_id(connector_ptr, connector_device_id, connector_config_imei_number);
#endif
                    break;
            }
            break;
        }
    }

    COND_ELSE_GOTO(result == connector_working, error);

done:
    connector_ptr->device_id = connector_device_id;
    connector_got_device_id = connector_true;
    connector_debug_hexvalue("Device ID ", connector_ptr->device_id, DEVICE_ID_LENGTH);

error:
    return result;
}
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)

static connector_status_t connector_stop_callback(connector_data_t * const connector_ptr, connector_transport_t const transport, void * const user_context)
{
    connector_status_t result = connector_working;

    connector_initiate_stop_request_t stop_request;

    connector_request_t request_id;
    request_id.status_request = connector_status_stop_completed;

    stop_request.transport = transport;
    stop_request.user_context = user_context;

    {
        connector_callback_status_t const status =  connector_callback_no_response(connector_ptr->callback, connector_class_status, request_id, &stop_request, sizeof stop_request);

        switch (status)
        {
        case connector_callback_continue:
        case connector_callback_unrecognized:
            break;
        case connector_callback_busy:
            result = connector_pending;
            break;
        default:
            result = connector_abort;
            break;
        }
    }

    connector_debug_printf("connector_stop_callback: %s\n", transport_status_to_string(transport));
    return result;
}

#endif
#define CONNECTOR_IS_STOP(state, value)    ((state) == (value))

static connector_bool_t is_connector_stopped(connector_data_t * const connector_ptr, connector_close_status_t const close_status)
{
    int count = 0;
    connector_transport_state_t wait_state = (close_status == connector_close_status_device_stopped) ? connector_transport_idle : connector_transport_terminate;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)

#if (defined CONNECTOR_TRANSPORT_UDP)
    if (!connector_bool(CONNECTOR_IS_STOP(connector_ptr->sm_udp.transport.state, wait_state))) count++;
#endif

#if (defined CONNECTOR_TRANSPORT_SMS)
    if (!connector_bool(CONNECTOR_IS_STOP(connector_ptr->sm_sms.transport.state, wait_state))) count++;
#endif

#endif /* else (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) */

#if (defined CONNECTOR_TRANSPORT_TCP)
    if (!connector_bool(CONNECTOR_IS_STOP(edp_get_active_state(connector_ptr), wait_state))) count++;
#endif

    return connector_bool(count == 0);
}

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
static void abort_idigi(connector_data_t * const connector_ptr)
{
    connector_status_t status;

#if (defined CONNECTOR_TRANSPORT_UDP) ||(defined CONNECTOR_TRANSPORT_SMS)
    status = sm_initiate_action(connector_ptr, connector_initiate_terminate, NULL);

#if (defined CONNECTOR_TRANSPORT_UDP)
    connector_ptr->sm_udp.close.status = connector_close_status_abort;
#endif

#if (defined CONNECTOR_TRANSPORT_SMS)
    connector_ptr->sm_sms.close.status = connector_close_status_abort;
#endif
    if (status != connector_success)
        connector_debug_printf("abort_idigi: sm_initiate_action returns error %d\n", status);
#endif


#if (defined CONNECTOR_TRANSPORT_TCP)
    status = edp_initiate_action(connector_ptr, connector_initiate_terminate, NULL, NULL);
    edp_set_close_status(connector_ptr, connector_close_status_abort);
    if (status != connector_success)
        connector_debug_printf("abort_idigi: edp_initiate_action returns error %d\n", status);
#endif

    connector_ptr->stop.state = connector_state_abort_by_callback;
    return;
}
#endif


connector_handle_t connector_init(connector_callback_t const callback)
{

    connector_data_t * connector_handle = NULL;
    connector_status_t status;

#if (defined CONNECTOR_SW_DESCRIPTION)
    connector_debug_printf("iDigi Connector v%s %s\n", CONNECTOR_SW_VERSION, CONNECTOR_SW_DESCRIPTION);
#else
    connector_debug_printf("iDigi Connector v%s\n", CONNECTOR_SW_VERSION);
#endif

    {
        void * handle;

#if (defined CONNECTOR_NO_MALLOC)
        status = malloc_data_buffer(NULL, sizeof *connector_handle, named_buffer_id(connector_data), &handle);
#else
        status = malloc_cb(callback, sizeof *connector_handle, &handle);
#endif

        COND_ELSE_GOTO(status == connector_working, done);
        connector_handle = handle;
    }

    connector_handle->callback = callback;

    status = manage_device_id(connector_handle);
    COND_ELSE_GOTO(status == connector_working, error);

    /* make a copy of the cloud url */
#if (defined CONNECTOR_TRANSPORT_TCP) || (defined CONNECTOR_TRANSPORT_UDP)
#if (defined CONNECTOR_CLOUD_URL)
    {
        static char const connector_device_cloud_url[]= CONNECTOR_CLOUD_URL;
        connector_handle->device_cloud_url = (char *)connector_device_cloud_url;
        connector_handle->device_cloud_url_length = sizeof connector_device_cloud_url -1;
    }
#else
    status = get_config_parameter(connector_handle, connector_config_server_url);
    COND_ELSE_GOTO(status == connector_working, error);
#endif
#endif /* (defined CONNECTOR_TRANSPORT_TCP) || (defined CONNECTOR_TRANSPORT_UDP) */

#if (defined CONNECTOR_TRANSPORT_TCP)
    status = connector_edp_init(connector_handle);
    COND_ELSE_GOTO(status == connector_working, error);
    connector_handle->first_running_network = connector_network_tcp;
#endif

    #if (defined CONNECTOR_TRANSPORT_UDP) || (defined CONNECTOR_TRANSPORT_SMS)
    status = connector_sm_init(connector_handle);
    COND_ELSE_GOTO(status == connector_working, error);
    #endif

    connector_handle->signature = connector_signature;
    goto done;

error:
    free_data_buffer(connector_handle, named_buffer_id(connector_data), connector_handle);
    connector_handle = NULL;

done:
    return connector_handle;
}


connector_status_t connector_step(connector_handle_t const handle)
{
    connector_status_t result = connector_init_error;
    connector_data_t * const connector_ptr = handle;

    ASSERT_GOTO(handle != NULL, done);
    if (connector_ptr->signature != connector_signature) goto done;

    switch (connector_ptr->stop.state)
    {
        case connector_state_running:
            break;
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_state_stop_by_initiate_action:
            if (is_connector_stopped(connector_ptr, connector_close_status_device_stopped))
            {
                result = connector_stop_callback(connector_ptr, connector_transport_all, connector_ptr->stop.user_context);
                if (result == connector_abort)
                {
                    goto error;
                }
                else if (result == connector_working)
                {
                    connector_ptr->stop.state = connector_state_running;
                }
            }
            break;
        case connector_state_abort_by_callback:
#endif
        case connector_state_terminate_by_initiate_action:
            if (is_connector_stopped(connector_ptr, connector_close_status_device_terminated))
            {
                connector_got_device_id = connector_false;
                connector_ptr->signature = NULL;
                free_data_buffer(connector_ptr, named_buffer_id(connector_data), connector_ptr);
                connector_debug_printf("connector_step: free iDigi connector\n");

                result = (connector_ptr->stop.state == connector_state_terminate_by_initiate_action) ? connector_device_terminated : connector_abort;
                goto done;
            }
            break;
        default:
            break;
    }


    {
        connector_network_type_t i;
        connector_network_type_t current_network = connector_ptr->first_running_network;

        result = connector_idle;

        for (i=connector_network_tcp; i < connector_network_count && result == connector_idle; i++)
        {
            switch (current_network)
            {
#if (defined CONNECTOR_TRANSPORT_TCP)
            case connector_network_tcp:
                result = connector_edp_step(connector_ptr);
                break;
#endif

#if (defined CONNECTOR_TRANSPORT_UDP)
            case connector_network_udp:
                result = connector_udp_step(connector_ptr);
                break;
#endif

#if (defined CONNECTOR_TRANSPORT_SMS)
            case connector_network_sms:
                result = connector_sms_step(connector_ptr);
                break;
#endif
            default:
                break;
            }

            current_network++;
            if (current_network == connector_network_count) current_network = connector_network_tcp;

        }

        connector_ptr->first_running_network = (result == connector_idle) ? connector_ptr->first_running_network + 1: current_network; /* next network type */
        if (connector_ptr->first_running_network == connector_network_count) connector_ptr->first_running_network = connector_network_tcp;
    }

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
error:
    if ((result == connector_abort) &&
        (connector_ptr->stop.state != connector_state_terminate_by_initiate_action) &&
        (connector_ptr->stop.state != connector_state_abort_by_callback))
    {
        /* let's abort idigi */
        abort_idigi(connector_ptr);
    }
    /* wait until SMS/UDP/TCP are in terminated state */
    if ((result == connector_abort) || (result == connector_device_terminated)) result = connector_working;

#else
    /* wait until TCP is in termianted state */
    if (result == connector_device_terminated) result = connector_working;

#endif

done:
#if (CONNECTOR_VERSION <= CONNECTOR_VERSION_1200)
    if (result == connector_idle || result == connector_pending || result == connector_working || result == connector_active)
    {
        result = connector_success;
    }
#endif
    return result;
}

connector_status_t connector_run(connector_handle_t const handle)
{
    connector_status_t rc;

    do {
        rc = connector_step(handle);

        if (rc == connector_idle || rc == connector_working || rc == connector_pending || rc == connector_active || rc == connector_success)
        {
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
            if (yield_process(handle, rc) != connector_working)
            {
                abort_idigi(handle);
                rc = connector_success;
            }
#else
            sleep_timeout(handle);
#endif
        }

    } while (rc == connector_idle || rc == connector_working || rc == connector_pending || rc == connector_active || rc == connector_success);

    return rc;
}

connector_status_t connector_initiate_action(connector_handle_t const handle, connector_initiate_request_t const request, void const * const request_data, void  * const response_data)
{
    connector_status_t result = connector_init_error;
    connector_data_t * connector_ptr = (connector_data_t *)handle;

    ASSERT_GOTO(handle != NULL, done);

    if (connector_ptr->signature != connector_signature) goto done;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)

#if !(defined CONNECTOR_TRANSPORT_TCP) && !(defined CONNECTOR_TRANSPORT_UDP) && !(defined CONNECTOR_TRANSPORT_SMS)
#error "Error: One of the transport (TCP, UDP or SMS) must be defined in connector_config.h."
#endif

#if (!defined CONNECTOR_TRANSPORT_TCP)
    UNUSED_PARAMETER(response_data);
#endif

    switch (request)
    {
    case connector_initiate_terminate:

#if (defined CONNECTOR_TRANSPORT_TCP)
        result = edp_initiate_action(connector_ptr, request, request_data, response_data);
        COND_ELSE_GOTO(result == connector_success, done);
#endif

#if (defined CONNECTOR_TRANSPORT_UDP) || (defined CONNECTOR_TRANSPORT_SMS)
        result = sm_initiate_action(connector_ptr, request, request_data);
        COND_ELSE_GOTO(result == connector_success, done);
#endif

        connector_ptr->stop.state = connector_state_terminate_by_initiate_action;
        result = connector_success;
        break;

#if (defined CONNECTOR_DATA_POINTS)
    case connector_initiate_data_point:
        result = dp_initiate_data_point(request_data);
        break;

    case connector_initiate_binary_point:
        result = dp_initiate_binary_point(request_data);
        break;
#endif

   default:
        if (connector_ptr->stop.state == connector_state_terminate_by_initiate_action)
        {
            result = connector_device_terminated;
            goto done;
        }
#if 0
        if (request != connector_initiate_transport_stop &&
            connector_ptr->stop.state == connector_state_stop_by_initiate_action)
        {
            /* we are in closing all transport */
            result = connector_service_busy;
            goto done;
        }
#endif
        {
            connector_transport_t const * const transport = request_data;

            result = connector_unavailable;

            if (transport == NULL)
            {
                result = connector_invalid_data;
                goto done;
            }

            switch (*transport)
            {
            case connector_transport_all:
                if (request != connector_initiate_transport_stop)
                {
                    result = connector_invalid_data;
                    goto done;
                }

                if (connector_ptr->stop.state != connector_state_running)
                {
                    /* already in close state */
                    result = (connector_ptr->stop.state == connector_state_terminate_by_initiate_action) ? connector_device_terminated: connector_service_busy;
                    goto done;
                }

                /* no break */

#if (defined CONNECTOR_TRANSPORT_UDP) || (defined CONNECTOR_TRANSPORT_SMS)
#if (defined CONNECTOR_TRANSPORT_UDP)
            case connector_transport_udp:
#endif
#if (defined CONNECTOR_TRANSPORT_SMS)
            case connector_transport_sms:
#endif
                result = sm_initiate_action(connector_ptr, request, request_data);

                if (*transport != connector_transport_all)  break;
                else if (result != connector_success) break;
                else if (request != connector_initiate_transport_stop) break;
                /* no break; */
#endif

#if (defined CONNECTOR_TRANSPORT_TCP)
            case connector_transport_tcp:
                result = edp_initiate_action(connector_ptr, request, request_data, response_data);

                if (*transport != connector_transport_all)  break;
                else if (result != connector_success) break;
                else if (request != connector_initiate_transport_stop) break;
#endif
                {
                    connector_initiate_stop_request_t const * const stop_request = request_data;
                    connector_ptr->stop.condition = stop_request->condition;
                    connector_ptr->stop.user_context = stop_request->user_context;
                    connector_ptr->stop.state = connector_state_stop_by_initiate_action;
                    result = connector_success;
                }
                break;

            default:
                result = connector_invalid_data;
                goto done;
            }
        }
    }
#else


    switch (request)
    {
    case connector_initiate_terminate:
        connector_ptr->stop.state = connector_state_terminate_by_initiate_action;
        break;
    default:
        if (connector_ptr->stop.state == connector_state_terminate_by_initiate_action)
        {
            result = connector_device_terminated;
            goto done;
        }
        break;
    }
    result = edp_initiate_action(connector_ptr, request, request_data, response_data);

    if (result != connector_success)
        connector_ptr->stop.state = connector_state_running;

#endif

done:
    return result;
}

