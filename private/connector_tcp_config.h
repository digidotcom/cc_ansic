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


#if !(defined CONNECTOR_VENDOR_ID)
static connector_status_t get_config_vendor_id(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_config_vendor_id_t config_vendor;

    {
        connector_callback_status_t status;
        connector_request_t request_id;

        request_id.config_request = connector_request_id_config_vendor_id;
        status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &config_vendor);

        switch (status)
        {
        case connector_callback_continue:
            {
                uint32_t const max_vendor_id = CONNECTOR_MAX_VENDOR_ID_NUMBER;

                if ((config_vendor.id == 0) || (config_vendor.id >= max_vendor_id))
                {
                    notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data_range);
                    result = connector_abort;
                }
                else
                    connector_ptr->edp_data.config.vendor_id = config_vendor.id;

            }
            break;

        case connector_callback_busy:
        case connector_callback_abort:
        case connector_callback_error:
        case connector_callback_unrecognized:
            result =  connector_abort;
            break;
        }
    }

    return result;
}
#endif

#if !(defined CONNECTOR_DEVICE_TYPE)
static connector_status_t get_config_device_type(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_config_pointer_name_t device_type;
    connector_request_t request_id;

    {
        connector_callback_status_t status;

        request_id.config_request = connector_request_id_config_device_type;
        status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &device_type);

        switch (status)
        {
        case connector_callback_continue:
            if (device_type.name == NULL)
                result = connector_invalid_data;
            else if ((device_type.bytes_in_name == 0) || (device_type.bytes_in_name > DEVICE_TYPE_LENGTH))
                result = connector_invalid_data_size;
            else
            {
                connector_ptr->edp_data.config.device_type_length = device_type.bytes_in_name;
                connector_ptr->edp_data.config.device_type = device_type.name;
            }
            break;

        case connector_callback_busy:
        case connector_callback_abort:
        case connector_callback_error:
        case connector_callback_unrecognized:
            result =  connector_abort;
            goto done;
        }
    }

    if (result != connector_working)
    {
        notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, result);
        result = connector_abort;
    }

done:
    return result;
}
#endif

#if !(defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS) || !(defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS)
static connector_status_t get_config_keepalive(connector_data_t * const connector_ptr, connector_request_id_config_t const config_request)
{
    connector_status_t result = connector_working;
    connector_config_keepalive_t keepalive;

    ASSERT((config_request == connector_request_id_config_rx_keepalive) || (config_request == connector_request_id_config_tx_keepalive));
    {
        connector_callback_status_t status;
        connector_request_t request_id;

        request_id.config_request = config_request;
        status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &keepalive);

        switch (status)
        {
            case connector_callback_continue:
            {

#if !defined(CONNECTOR_TX_KEEPALIVE_IN_SECONDS) && !defined(CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
                connector_bool_t const is_tx = connector_bool(config_request == connector_request_id_config_tx_keepalive);
                uint16_t const min_interval = is_tx ? MIN_TX_KEEPALIVE_INTERVAL_IN_SECONDS : MIN_RX_KEEPALIVE_INTERVAL_IN_SECONDS;
                uint16_t const max_interval = is_tx ? MAX_TX_KEEPALIVE_INTERVAL_IN_SECONDS : MAX_RX_KEEPALIVE_INTERVAL_IN_SECONDS;

#elif !(defined CONNECTOR_TX_KEEPALIVE_IN_SECONDS)
                uint16_t const min_interval = MIN_TX_KEEPALIVE_INTERVAL_IN_SECONDS;
                uint16_t const max_interval = MAX_TX_KEEPALIVE_INTERVAL_IN_SECONDS;

#elif !(defined CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
                uint16_t const min_interval = MIN_RX_KEEPALIVE_INTERVAL_IN_SECONDS;
                uint16_t const max_interval = MAX_RX_KEEPALIVE_INTERVAL_IN_SECONDS;
#endif
                if ((keepalive.interval < min_interval) || (keepalive.interval > max_interval))
                {
                    notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data_range);
                    result = connector_abort;
                    goto done;
                }

                {
#if !defined(CONNECTOR_TX_KEEPALIVE_IN_SECONDS) && !defined(CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
                    uint16_t * const keepalive_store_at = is_tx ? &connector_ptr->edp_data.config.tx_keepalive_interval : &connector_ptr->edp_data.config.rx_keepalive_interval;
#elif !defined(CONNECTOR_TX_KEEPALIVE_IN_SECONDS)
                    uint16_t * const keepalive_store_at = &connector_ptr->edp_data.config.tx_keepalive_interval;
#elif !defined(CONNECTOR_RX_KEEPALIVE_IN_SECONDS)
                    uint16_t * const keepalive_store_at = &connector_ptr->edp_data.config.rx_keepalive_interval;
#endif
                    *keepalive_store_at = keepalive.interval;
                }
                break;
            }
            case connector_callback_busy:
            case connector_callback_abort:
            case connector_callback_error:
            case connector_callback_unrecognized:
                result =  connector_abort;
                goto done;
        }
    }

done:
    return result;
}
#endif

#if !(defined CONNECTOR_WAIT_COUNT)
static connector_status_t get_config_wait_count(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_config_wait_count_t wait_count;

    {
        connector_callback_status_t status;
        connector_request_t request_id;

        request_id.config_request = connector_request_id_config_wait_count;
        status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &wait_count);

        switch (status)
        {
        case connector_callback_continue:
            if ((wait_count.count < WAIT_COUNT_MIN) ||
                     (wait_count.count > WAIT_COUNT_MAX))
            {
                notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data_range);
                result = connector_abort;
            }
            else
            {
                connector_ptr->edp_data.config.wait_count = wait_count.count;
            }
            break;

        case connector_callback_busy:
        case connector_callback_abort:
        case connector_callback_error:
        case connector_callback_unrecognized:
            result = connector_abort;
            break;
        }
    }
    return result;
}
#endif

#define CC_IPV6_ADDRESS_LENGTH 16
#define CC_IPV4_ADDRESS_LENGTH 4

static connector_status_t get_config_ip_addr(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_config_ip_addr_t config_ip;
    connector_request_t request_id;

    {
        connector_callback_status_t status;

        request_id.config_request = connector_request_id_config_ip_addr;
        status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &config_ip);

        switch (status)
        {
        case connector_callback_continue:
        {
            #define CC_ZERO_IP_ADDR         0x00000000
            #define CC_BROADCAST_IP_ADDR    0xFFFFFFFF

            if (config_ip.addr == NULL)
            {
                result = connector_invalid_data;
                goto error;
            }
            if ((config_ip.bytes_in_addr != CC_IPV6_ADDRESS_LENGTH) && (config_ip.bytes_in_addr != CC_IPV4_ADDRESS_LENGTH))
            {
                result =  connector_invalid_data_size;
                goto error;
            }

            if (config_ip.bytes_in_addr == CC_IPV4_ADDRESS_LENGTH)
            {
                static uint32_t const zero_ip_addr = CC_ZERO_IP_ADDR;
                static uint32_t const broadcast_ip_addr = CC_BROADCAST_IP_ADDR;

                uint32_t const ip =  LoadBE32(config_ip.addr);

                if ((ip == LoadBE32(&zero_ip_addr)) || (ip == LoadBE32(&broadcast_ip_addr)))
                {
                    result = connector_invalid_data_range;
                    goto error;
                }
            }

            connector_ptr->edp_data.config.ip_addr_length = config_ip.bytes_in_addr;
            connector_ptr->edp_data.config.ip_addr = config_ip.addr;
            break;
        }

        case connector_callback_busy:
        case connector_callback_abort:
        case connector_callback_error:
        case connector_callback_unrecognized:
            result =  connector_abort;
            goto done;
        }
    }


error:
    if (result != connector_working)
    {
        notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, result);
        result = connector_abort;
    }

done:
    return result;
}


static connector_status_t get_config_connect_status(connector_data_t * const connector_ptr,
                                                        connector_request_id_config_t const config_request_id,
                                                        connector_config_connect_status_t * const config_connect)
{
    connector_status_t result = connector_working;

    ASSERT(config_connect != NULL);
    ASSERT((config_request_id == connector_request_id_config_network_tcp) ||
           (config_request_id == connector_request_id_config_network_udp) ||
           (config_request_id == connector_request_id_config_network_sms));

    config_connect->status = connector_auto_connect;

    {
        connector_callback_status_t status;
        connector_request_t request_id;

        request_id.config_request = config_request_id;
        status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, config_connect);

        switch (status)
        {
        case connector_callback_continue:
            switch (config_connect->status)
            {
            case connector_auto_connect:
            case connector_manual_connect:
                break;
            default:
                notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data_range);
                result = connector_abort;
                break;
            }
            break;

        case connector_callback_busy:
        case connector_callback_abort:
        case connector_callback_error:
            result = connector_abort;
            break;

        case connector_callback_unrecognized:
            break;
        }
    }
    return result;
}

#if !(defined CONNECTOR_IDENTITY_VERIFICATION)
static connector_status_t get_config_identity_verification(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_config_identity_verification_t config_identity;


    {
        connector_callback_status_t status;
        connector_request_t request_id;

        request_id.config_request = connector_request_id_config_identity_verification;
        status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &config_identity);

        switch (status)
        {
        case connector_callback_continue:
            switch (config_identity.type)
            {
            case connector_simple_identity_verification:
            case connector_password_identity_verification:
                connector_ptr->edp_data.config.identity_verification = config_identity.type;
                break;
            default:
                notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data_range);
                result = connector_abort;
                break;
            }
            break;

        case connector_callback_busy:
        case connector_callback_abort:
        case connector_callback_error:
            result = connector_abort;
            break;

        case connector_callback_unrecognized:
            connector_ptr->edp_data.config.identity_verification = connector_simple_identity_verification;
            break;
        }
    }
    return result;
}
#endif

static connector_status_t get_config_password(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_config_pointer_name_t config_password;


    {
        connector_callback_status_t status;
        connector_request_t request_id;

        request_id.config_request = connector_request_id_config_password;
        status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &config_password);

        switch (status)
        {
        case connector_callback_continue:
            if (config_password.name == NULL)
            {
                notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data);
                result = connector_abort;
            }
            else
            {
                connector_ptr->edp_data.config.password_length = config_password.bytes_in_name;
                connector_ptr->edp_data.config.password = config_password.name;

            }
            break;

        case connector_callback_busy:
        case connector_callback_abort:
        case connector_callback_error:
        case connector_callback_unrecognized:
            result = connector_abort;
            break;
        }
    }

    return result;
}
