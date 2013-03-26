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

#include <stddef.h>

#include "connector_edp_util.h"

#include "connector_tcp_send.h"
#include "connector_tcp_recv.h"
#include "connector_tcp_open.h"
#include "connector_tcp_close.h"

#include "connector_cc.h"

#if (defined CONNECTOR_DATA_SERVICE) || (defined CONNECTOR_FILE_SYSTEM) || (defined CONNECTOR_RCI_SERVICE)
#include "connector_msg.h"
#endif
#if (defined CONNECTOR_DATA_SERVICE)
#include "connector_ds.h"
#endif
#if (defined CONNECTOR_FILE_SYSTEM)
#include "connector_fs.h"
#endif

#if (defined CONNECTOR_FIRMWARE_SERVICE)
#include "connector_firmware.h"
#endif

#include "layer.h"

static connector_status_t get_connection_type(connector_data_t * const connector_ptr);
static connector_status_t get_link_speed(connector_data_t * const connector_ptr);
static connector_status_t get_phone_number(connector_data_t * const connector_ptr);

static connector_status_t edp_get_config_parameter(connector_data_t * const connector_ptr, connector_config_request_t const config_request)
{
    connector_status_t result = connector_working;

    void * store_at = NULL;

#if !(defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS) || !(defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS) || !(defined CONNECTOR_WAIT_COUNT)
    void * pointer_data;
#endif

    size_t length = 0;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) && !(defined CONNECTOR_NETWORK_TCP_START)
    connector_auto_connect_type_t  connect_type = connector_auto_connect;
#endif

    switch (config_request)
    {
#if !(defined CONNECTOR_VENDOR_ID)
        case connector_config_vendor_id:
            store_at = &connector_ptr->edp_data.config.vendor_id;
            length = VENDOR_ID_LENGTH;
            break;
#endif

#if !(defined CONNECTOR_DEVICE_TYPE)
        case connector_config_device_type:
            store_at = &connector_ptr->edp_data.config.device_type;
            break;
#endif

        case connector_config_ip_addr:
            store_at = &connector_ptr->edp_data.config.ip_addr;
            break;

#if !(defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
        case connector_config_rx_keepalive:
            store_at = &pointer_data;
            break;
#endif

#if !(defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS)
        case connector_config_tx_keepalive:
            store_at = &pointer_data;
            break;
#endif

#if !(defined CONNECTOR_WAIT_COUNT)
        case connector_config_wait_count:
            store_at = &pointer_data;
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) && !(defined CONNECTOR_NETWORK_TCP_START)
        case connector_config_network_tcp:
            store_at = &connect_type;
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) &&  !(defined CONNECTOR_IDENTITY_VERIFICATION)
        case connector_config_identity_verification:
            store_at = &connector_ptr->edp_data.config.identity_verification;
            length = sizeof connector_ptr->edp_data.config.identity_verification;
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_config_password:
            store_at = &connector_ptr->edp_data.config.password;
            break;
#endif

        default:
            ASSERT_GOTO(connector_false, done);
            break;
    }

    {
        connector_callback_status_t status;
        connector_request_t request_id;

        request_id.config_request = config_request;
        status = connector_callback(connector_ptr->callback, connector_class_config, request_id, store_at);

        switch (status)
        {
        case connector_callback_continue:
            break;

        case connector_callback_busy:
            /* treat as an error since callback cannot return busy for configuration data */
            result = connector_abort;
            goto error;

        case connector_callback_abort:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_callback_error:
#endif
            result = connector_abort;
            goto done;

        case connector_callback_unrecognized:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) && !(defined CONNECTOR_NETWORK_TCP_START)
            if (config_request == connector_config_network_tcp)
            {
                connect_type = connector_auto_connect;
                break;
            }
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) && !(defined CONNECTOR_IDENTITY_VERIFICATION)
            if (config_request == connector_config_identity_verification)
            {
                length = sizeof connector_ptr->edp_data.config.identity_verification;
                connector_ptr->edp_data.config.identity_verification = connector_simple_identity_verification;
                break;
            }
#endif

            /* error callback must support the rest of configuration request */
            result =  connector_abort;
            goto error;
        }
    }

    if (store_at == NULL)
    {
        result = connector_invalid_data;
        goto error;
    }


    switch (config_request)
    {
#if !(defined CONNECTOR_VENDOR_ID)
        case connector_config_vendor_id:
        {
            uint32_t const max_vendor_id = CONNECTOR_MAX_VENDOR_ID_NUMBER;
            uint32_t * const vendor_id = connector_ptr->edp_data.config.vendor_id;

            if (length != VENDOR_ID_LENGTH)
                result = connector_invalid_data_size;

            else if (vendor_id == NULL)
                result = connector_invalid_data;

            else if ((*vendor_id == 0) || (*vendor_id >= max_vendor_id))
                result = connector_invalid_data_range;

            break;
        }
#endif

#if !(defined CONNECTOR_DEVICE_TYPE)
        case connector_config_device_type:
            if (connector_ptr->edp_data.config.device_type == NULL)
                result = connector_invalid_data;
            else if ((length == 0) || (length > DEVICE_TYPE_LENGTH))
                result = connector_invalid_data_size;
            else
                connector_ptr->edp_data.config.device_type_length = length;

            break;
#endif

        case connector_config_ip_addr:
        {
            #define CC_ZERO_IP_ADDR         0x00000000
            #define CC_BROADCAST_IP_ADDR    0xFFFFFFFF

            if (connector_ptr->edp_data.config.ip_addr == NULL)
            {
                result = connector_invalid_data;
                goto error;
            }
            if ((length != CC_IPV6_ADDRESS_LENGTH) && (length != CC_IPV4_ADDRESS_LENGTH))
            {
                result =  connector_invalid_data_size;
                goto error;
            }

            if (length == CC_IPV4_ADDRESS_LENGTH)
            {
                static uint32_t const zero_ip_addr = CC_ZERO_IP_ADDR;
                static uint32_t const broadcast_ip_addr = CC_BROADCAST_IP_ADDR;

                uint32_t const ip =  LoadBE32(connector_ptr->edp_data.config.ip_addr);

                if ((ip == LoadBE32(&zero_ip_addr)) || (ip == LoadBE32(&broadcast_ip_addr)))
                {
                    result = connector_invalid_data_range;
                    goto error;
                }
            }

            connector_ptr->edp_data.config.ip_addr_length = length;

            break;
        }

#if !(defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS) || !(defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
        case connector_config_tx_keepalive:
        case connector_config_rx_keepalive:
        {
            uint16_t const * const value = pointer_data;
#if !(defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS) && !(defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
            connector_bool_t const is_tx = connector_bool(config_request == connector_config_tx_keepalive);
            uint16_t const min_interval = is_tx ? MIN_TX_KEEPALIVE_INTERVAL_IN_SECONDS : MIN_RX_KEEPALIVE_INTERVAL_IN_SECONDS;
            uint16_t const max_interval = is_tx ? MAX_TX_KEEPALIVE_INTERVAL_IN_SECONDS : MAX_RX_KEEPALIVE_INTERVAL_IN_SECONDS;

#elif !(defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS)
            uint16_t const min_interval = MIN_TX_KEEPALIVE_INTERVAL_IN_SECONDS;
            uint16_t const max_interval = MAX_TX_KEEPALIVE_INTERVAL_IN_SECONDS;

#elif !(defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
            uint16_t const min_interval = MIN_RX_KEEPALIVE_INTERVAL_IN_SECONDS;
            uint16_t const max_interval = MAX_RX_KEEPALIVE_INTERVAL_IN_SECONDS;
#endif
            if (value == NULL)
            {
                result = connector_invalid_data;
                goto error;
            }
            if ((*value < min_interval) || (*value > max_interval))
            {
                result = connector_invalid_data_range;
                goto error;
            }
            if (length != sizeof *value)
            {
                result = connector_invalid_data_size;
                goto error;
            }

            {
#if !defined(CONNECTOR_TX_KEEPALIVE_IN_SECONDS) && !defined(CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
                uint16_t * const keepalive_store_at = is_tx ? &connector_ptr->edp_data.config.tx_keepalive_interval : &connector_ptr->edp_data.config.rx_keepalive_interval;
#elif !defined(CONNECTOR_TX_KEEPALIVE_IN_SECONDS)
                uint16_t * const keepalive_store_at = &connector_ptr->edp_data.config.tx_keepalive_interval;
#elif !defined(CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
                uint16_t * const keepalive_store_at = &connector_ptr->edp_data.config.rx_keepalive_interval;
#endif
                *keepalive_store_at = *value;
            }
            break;
        }
#endif

#if !(defined CONNECTOR_WAIT_COUNT)
        case connector_config_wait_count:
        {
            uint16_t const * const value = pointer_data;

            if (value == NULL)
            {
                result = connector_invalid_data;
            }

            else if ((*value < WAIT_COUNT_MIN) ||
                     (*value > WAIT_COUNT_MAX))
            {
                result = connector_invalid_data_range;
            }
            else if (length != sizeof *value)
            {
                result = connector_invalid_data_size;
            }
            else
            {
                connector_ptr->edp_data.config.wait_count = *value;
            }
            break;
        }
#endif


#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) && !(defined CONNECTOR_NETWORK_TCP_START)
        case connector_config_network_tcp:
            switch (connect_type)
            {
            case connector_auto_connect:
            case connector_manual_connect:
                connector_ptr->edp_data.connect_type = connect_type;
                connector_ptr->edp_data.stop.connect_action = connect_type;
                break;
            default:
                result = connector_invalid_data_range;
                break;
            }
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) &&  !(defined CONNECTOR_IDENTITY_VERIFICATION)
        case connector_config_identity_verification:
            if (length != sizeof connector_ptr->edp_data.config.identity_verification)
            {
                result = connector_invalid_data_size;
                goto error;
            }
            switch (connector_ptr->edp_data.config.identity_verification)
            {
            case connector_simple_identity_verification:
            case connector_password_identity_verification:
                break;
            default:
                result = connector_invalid_data_range;
                break;
            }
            break;
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_config_password:
            if (connector_ptr->edp_data.config.password == NULL)
            {
                result = connector_invalid_data;
            }
            else if (length > UINT16_MAX)
            {
                result = connector_invalid_data_size;
            }
            else
            {
                connector_ptr->edp_data.config.password_length = length;
            }
            break;
#endif

        default:
             break;
    }

error:

    if (result != connector_working)
    {
        connector_request_t request_id;

        request_id.config_request = config_request;
        connector_debug_printf("edp_get_config_parameter: config failed request id = %d, error = %d\n", config_request, result);
        notify_error_status(connector_ptr->callback, connector_class_config, request_id, result);
        result = connector_abort;
    }

done:
    return result;
}

static void edp_get_device_cloud(connector_data_t * const connector_ptr)
{
    memcpy(connector_ptr->edp_data.config.server_url, connector_ptr->device_cloud_url, connector_ptr->device_cloud_url_length);
    connector_ptr->edp_data.config.server_url[connector_ptr->device_cloud_url_length] = 0x0;
    connector_ptr->edp_data.config.server_url_length = connector_ptr->device_cloud_url_length;
}

static connector_status_t edp_config_init(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1200)
    if (connector_ptr->device_id_method == connector_manual_device_id_method)
#endif
    {
        /* then we have to obtain connection type from the callback */
        result = get_connection_type(connector_ptr);
        COND_ELSE_GOTO(result == connector_working, done);
    }

    switch (connector_ptr->connection_type)
    {
    case connector_lan_connection_type:
        break;
    case connector_wan_connection_type:
#if (!defined CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND)
        result = get_link_speed(connector_ptr);
        COND_ELSE_GOTO(result == connector_working, done);
#endif

#if (!defined CONNECTOR_WAN_PHONE_NUMBER_DIALED)
        result = get_phone_number(connector_ptr);
        COND_ELSE_GOTO(result == connector_working, done);
#endif

        break;
    }

#if (CONNECTOR_VERSION <= CONNECTOR_VERSION_1200)
    edp_set_active_state(connector_ptr, connector_transport_open);

#elif (defined CONNECTOR_NETWORK_TCP_START)
    if (CONNECTOR_NETWORK_TCP_START == connector_auto_connect)
    {
        edp_set_active_state(connector_ptr, connector_transport_open);
    }
    connector_ptr->edp_data.stop.connect_action = CONNECTOR_NETWORK_TCP_START;
#endif

    {
        size_t i;

        static connector_config_request_t const edp_request_id_table[] = {
#if !(defined CONNECTOR_VENDOR_ID)
                connector_config_vendor_id,
#endif

#if !(defined CONNECTOR_DEVICE_TYPE)
                connector_config_device_type,
#endif
#if !(defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
                connector_config_rx_keepalive,
#endif
#if !(defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS)
                connector_config_tx_keepalive,
#endif
#if !(defined CONNECTOR_WAIT_COUNT)
                connector_config_wait_count,
#endif
#if !(defined CONNECTOR_NETWORK_TCP_START) && (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
                connector_config_network_tcp,
#endif
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) &&  !(defined CONNECTOR_IDENTITY_VERIFICATION)
                connector_config_identity_verification,
#endif

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
                connector_config_password,
#endif
        };

        for (i=0; i < asizeof(edp_request_id_table); i++)
        {
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
            if (edp_request_id_table[i] == connector_config_password)
            {
#if (defined CONNECTOR_IDENTITY_VERIFICATION)
                connector_identity_verification_t const identity_verification = CONNECTOR_IDENTITY_VERIFICATION;
#else
                connector_identity_verification_t const identity_verification = connector_ptr->edp_data.config.identity_verification;
#endif
                if (identity_verification == connector_simple_identity_verification)
                {
                    /* skip password configuration for simple identity verification */
                    continue;
                }
            }
#endif
            result = edp_get_config_parameter(connector_ptr, edp_request_id_table[i]);
            COND_ELSE_GOTO(result == connector_working, done);
        }
    }

done:
    return result;
}


connector_status_t connector_edp_init(connector_data_t * const connector_ptr)
{

    connector_status_t result = connector_working;

    edp_reset_initial_data(connector_ptr);
    connector_ptr->edp_data.facilities.list = NULL;
    connector_ptr->edp_data.facilities.supported_mask = 0;

    edp_set_active_state(connector_ptr, connector_transport_idle);
    edp_set_initiate_state(connector_ptr, connector_transport_idle);

    result = edp_config_init(connector_ptr);
    if (result != connector_working)
    {
        goto done;
    }

    result = edp_layer_get_supported_facilities(connector_ptr);
    if (result != connector_working)
    {
        goto done;
    }

    result = edp_layer_initialize_facilities(connector_ptr);
    if (result != connector_working)
    {
        goto done;
    }

    edp_get_device_cloud(connector_ptr);

done:
    return result;
}


connector_status_t connector_edp_step(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_idle;
    connector_transport_state_t init_active_state = edp_get_active_state(connector_ptr);

    while (result == connector_idle)
    {
        switch (edp_get_active_state(connector_ptr))
        {
        case connector_transport_idle:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
            if (connector_ptr->edp_data.stop.connect_action == connector_auto_connect)
            {
                edp_set_active_state(connector_ptr, connector_transport_open);
            }
#endif
            goto check_state;

        case connector_transport_open:
            result = edp_tcp_open_process(connector_ptr);
            break;

        case connector_transport_send:
        case connector_transport_receive:

            switch (edp_get_active_state(connector_ptr))
            {
                case connector_transport_send:
                    result = edp_tcp_send_process(connector_ptr);

                    if (edp_get_active_state(connector_ptr) == connector_transport_send)
                    {
                        edp_set_active_state(connector_ptr, connector_transport_receive);
                    }
                   break;
                case connector_transport_receive:
                    result = edp_tcp_receive_process(connector_ptr);

                    if (edp_get_active_state(connector_ptr) == connector_transport_receive)
                    {
                        edp_set_active_state(connector_ptr, connector_transport_send);
                    }
                    break;
                default:
                    break;
            }

            break;
        case connector_transport_close:
        case connector_transport_terminate:
            result = edp_close_process(connector_ptr);
            break;

        case connector_transport_redirect:
            result = edp_redirect_process(connector_ptr);
           break;
        }

        if (result == connector_device_terminated
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
            || result == connector_abort
#endif
            )
        {
            connector_debug_printf("connector_edp_step: done with status = %d\n", result);
            goto done;
        }


        /* done all cases */
        if (edp_get_active_state(connector_ptr) == init_active_state) break;
    }

check_state:
    switch (edp_get_initiate_state(connector_ptr))
    {
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case connector_transport_close:
            switch (edp_get_active_state(connector_ptr))
            {
                case connector_transport_idle:
                    connector_ptr->edp_data.stop.connect_action = connector_manual_connect;
                    edp_set_initiate_state(connector_ptr, connector_transport_idle);

                    if (edp_is_stop_request(connector_ptr))
                    {
                        result = connector_stop_callback(connector_ptr, connector_transport_tcp, connector_ptr->edp_data.stop.user_context);

                        if (result == connector_pending)
                        {
                            /* restore the initiate state */
                            edp_set_initiate_state(connector_ptr, connector_transport_close);
                        }
                        else if (result == connector_working)
                        {
                            edp_clear_stop_request(connector_ptr);
                        }
                    }
                    /* exit edp so let connector_step does next step */
                    goto done;

                case connector_transport_close:
                    connector_ptr->edp_data.stop.connect_action = connector_manual_connect;
                    /* no break */
                case connector_transport_terminate:
                    break;

                default:
                    if (!tcp_is_send_active(connector_ptr) && result == connector_idle)
                    {
                        edp_set_stop_condition(connector_ptr, connector_stop_immediately);
                    }
                    if (edp_is_stop_immediately(connector_ptr))
                    {
                        connector_ptr->edp_data.stop.connect_action = connector_manual_connect;
                        edp_set_close_status(connector_ptr, connector_close_status_device_stopped);
                        edp_set_active_state(connector_ptr, connector_transport_close);
                    }
                    break;
            }
            break;
        case connector_transport_open:
            switch (edp_get_active_state(connector_ptr))
            {
                case connector_transport_close:
                case connector_transport_idle:
                    edp_set_active_state(connector_ptr, connector_transport_open);
                    break;
                case connector_transport_terminate:
                    break;
                default:
                    edp_set_initiate_state(connector_ptr, connector_transport_idle);
                    break;
            }
            break;
#endif
        case connector_transport_terminate:
            edp_set_close_status(connector_ptr, connector_close_status_device_terminated);
            edp_set_active_state(connector_ptr, connector_transport_close);
            break;
        default:
            break;
    }

done:
    return result;
}

connector_status_t edp_initiate_action(connector_data_t * const connector_ptr, connector_initiate_request_t const request, void const * const request_data)
{
    connector_status_t result = connector_init_error;

    switch (request)
    {
    case connector_initiate_terminate:
        UNUSED_PARAMETER(request_data);
        edp_set_initiate_state(connector_ptr, connector_transport_terminate);
        result = connector_success;
        break;

#if (defined CONNECTOR_DATA_SERVICE)
    case connector_initiate_send_data:
        if (edp_get_edp_state(connector_ptr) == edp_communication_connect_server) goto done;

        switch (edp_get_active_state(connector_ptr))
        {
        case connector_transport_idle:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
            result = connector_unavailable;
#endif
            goto done;

        case connector_transport_close:
            if (edp_get_initiate_state(connector_ptr) == connector_transport_open)
            {
                break;
            }
            /* no break  to return error */
        case connector_transport_redirect:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
            result = connector_unavailable;
            goto done;
#endif
            /* no break to return error */
        case connector_transport_terminate:
#if (CONNECTOR_VERSION <= CONNECTOR_VERSION_1200)
            result = connector_server_disconnected;
#endif
            goto done;

        case connector_transport_open:
        case connector_transport_send:
        case connector_transport_receive:
            switch (edp_get_initiate_state(connector_ptr))
            {
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
                case connector_transport_close:
                    result = connector_unavailable;
                    goto done;
#endif
                default:
                    break;
            }
            break;
        }
        result = data_service_initiate(connector_ptr, request_data);
        break;
#endif /* CONNECTOR_DATA_SERVICE */

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
    case connector_initiate_transport_stop:
        switch (edp_get_active_state(connector_ptr))
        {
        case connector_transport_open:
        case connector_transport_send:
        case connector_transport_receive:
        case connector_transport_redirect:
            break;

        case connector_transport_idle:
        case connector_transport_close:
            /* Return busy if start initiate action was called and cannot stop now */
            if (edp_get_initiate_state(connector_ptr) == connector_transport_open)
            {
                result = connector_service_busy;
                goto done;
            }
            break;

        default:
             goto done;
        }

        if (edp_get_initiate_state(connector_ptr) == connector_transport_close &&
            edp_is_stop_request(connector_ptr))
        {
            /* already requesting stop before */
            result = connector_service_busy;
        }
        else
        {
            connector_initiate_stop_request_t const * const stop_request = request_data;

            edp_set_stop_condition(connector_ptr, stop_request->condition);

            if (stop_request->transport == connector_transport_tcp)
            {
                edp_set_stop_request(connector_ptr, stop_request->user_context);
            }

            edp_set_initiate_state(connector_ptr, connector_transport_close);
            result = connector_success;
        }
        break;

    case connector_initiate_transport_start:
        if (request_data == NULL)
        {
            result = connector_invalid_data;
            goto done;
        }
        switch (edp_get_active_state(connector_ptr))
        {
        case connector_transport_open:
        case connector_transport_send:
        case connector_transport_receive:
        case connector_transport_redirect:
            /* Return busy if stop initiate action was called and cannot start now */
            result = (edp_get_initiate_state(connector_ptr) == connector_transport_close) ? connector_service_busy : connector_success;
            break;

        case connector_transport_idle:
        case connector_transport_close:
            if (edp_get_initiate_state(connector_ptr) == connector_transport_close)
            {
                result = connector_service_busy;
            }
            else
            {
                edp_set_initiate_state(connector_ptr, connector_transport_open);
                edp_get_device_cloud(connector_ptr);
                result = connector_success;
            }
            break;

        case connector_transport_terminate:
            break;;
        }

        break;
#endif /* (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300) */

    default:
        ASSERT(connector_false);
        result = connector_abort;
        break;
    }

#if (defined CONNECTOR_DATA_SERVICE) || (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
done:
#endif
    return result;
}
