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

static connector_status_t notify_error_status(connector_callback_t const callback, connector_class_id_t const class_number, connector_request_id_t const request_number, connector_status_t const status)
{
    connector_status_t result = connector_working;

#if (defined CONNECTOR_DEBUG)
    connector_error_status_t err_status;
    connector_request_id_t request_id;

    request_id.config_request = connector_request_id_config_error_status;
    err_status.class_id = class_number;
    err_status.request_id = request_number;
    err_status.status = status;

    {
        connector_callback_status_t const callback_status = connector_callback(callback, connector_class_id_config, request_id, &err_status);
        switch (callback_status)
        {
            case connector_callback_continue:
                break;
            default:
                result = connector_abort;
                break;
        }
    }

#else
    UNUSED_PARAMETER(callback);
    UNUSED_PARAMETER(class_number);
    UNUSED_PARAMETER(request_number);
    UNUSED_PARAMETER(status);
#endif
    return result;
}

static connector_status_t get_config_device_id(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_callback_status_t callback_status;
    connector_request_id_t request_id;
    connector_config_pointer_data_t device_id;

    device_id.bytes_required = DEVICE_ID_LENGTH;

    request_id.config_request = connector_request_id_config_device_id;

    callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &device_id);
    switch (callback_status)
    {
    case connector_callback_continue:
        if (device_id.bytes_required != DEVICE_ID_LENGTH)
            result = connector_invalid_data_size;
        else if (device_id.data == NULL)
            result = connector_invalid_data;
        else
            connector_ptr->device_id = device_id.data;
        break;
    case connector_callback_busy:
        result = connector_pending;
        break;
    case connector_callback_unrecognized:
    case connector_callback_abort:
    case connector_callback_error:
        result = connector_abort;
        goto done;
    }

    if (result != connector_working)
    {
        notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, result);
        result = connector_abort;
    }

done:
    return result;
}

#if !(defined CONNECTOR_CLOUD_URL)
static connector_status_t get_config_server_url(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_callback_status_t callback_status;
    connector_config_pointer_string_t   server_url;
    connector_request_id_t request_id;

    request_id.config_request = connector_request_id_config_server_url;


    callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &server_url);
    switch (callback_status)
    {
    case connector_callback_continue:
        if ((server_url.bytes_in_string == 0) || (server_url.bytes_in_string > SERVER_URL_LENGTH-1))
        {
            result =  connector_invalid_data_size;
        }
        else if (server_url.string == NULL)
        {
            result = connector_invalid_data;
        }
        else
        {
            connector_ptr->device_cloud_url_length = server_url.bytes_in_string;
            connector_ptr->device_cloud_url = server_url.string;
        }
        break;

    case connector_callback_busy:
    case connector_callback_unrecognized:
    case connector_callback_abort:
    case connector_callback_error:
        result = connector_abort;
        goto done;
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

static connector_status_t get_config_connection_type(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

#if (defined CONNECTOR_CONNECTION_TYPE)
    connector_ptr->connection_type = CONNECTOR_CONNECTION_TYPE;
#else
    connector_config_connection_type_t  config_connection;
    connector_callback_status_t callback_status;
    connector_request_id_t request_id;

    request_id.config_request = connector_request_id_config_connection_type;
    callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &config_connection);

    switch (callback_status)
    {
    case connector_callback_continue:

        switch (config_connection.type)
        {
        case connector_lan_connection_type:
        case connector_wan_connection_type:
            connector_ptr->connection_type = config_connection.type;
            break;

        default:
            notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data_range);
            result = connector_abort;
            break;
        }
        break;

    case connector_callback_busy:
        result = connector_pending;
        break;

    case connector_callback_unrecognized:
    case connector_callback_abort:
    case connector_callback_error:
        result = connector_abort;
        break;
    }
#endif

    return result;
}

static connector_status_t get_config_mac_addr(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

    if (connector_ptr->mac_addr == NULL)
    {
        connector_callback_status_t callback_status;
        connector_request_id_t request_id;
        connector_config_pointer_data_t mac_addr;

        mac_addr.bytes_required = MAC_ADDR_LENGTH;

        request_id.config_request = connector_request_id_config_mac_addr;

        callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &mac_addr);
        switch (callback_status)
        {
        case connector_callback_continue:
            if (mac_addr.bytes_required != MAC_ADDR_LENGTH)
                result = connector_invalid_data_size;
            else if (mac_addr.data == NULL)
                result = connector_invalid_data;
            else
                connector_ptr->mac_addr = mac_addr.data;
            break;

        case connector_callback_busy:
        case connector_callback_unrecognized:
        case connector_callback_abort:
        case connector_callback_error:
            result = connector_abort;
            goto done;
        }

        if (result != connector_working)
        {
            notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, result);
            result = connector_abort;
        }
    }

done:
    return result;
}

#if !(defined CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND)
static connector_status_t get_config_link_speed(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    connector_callback_status_t callback_status;
    connector_request_id_t request_id;
    connector_config_link_speed_t config_link;

    request_id.config_request = connector_request_id_config_link_speed;

    callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &config_link);
    switch (callback_status)
    {
    case connector_callback_continue:
        connector_ptr->link_speed = config_link.speed;
        break;
    case connector_callback_busy:
    case connector_callback_unrecognized:
    case connector_callback_abort:
    case connector_callback_error:
        result = connector_abort;
        break;
    }
    return result;
}
#endif

#if !(defined CONNECTOR_WAN_PHONE_NUMBER_DIALED)
static connector_status_t get_config_phone_number(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
    if (connector_ptr->phone_dialed == NULL)
    {
        connector_callback_status_t callback_status;
        connector_request_id_t request_id;
        connector_config_pointer_string_t phone_number;

        request_id.config_request = connector_request_id_config_phone_number;

        callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &phone_number);
        switch (callback_status)
        {
        case connector_callback_continue:
            if (phone_number.string == NULL)
            {
                result = connector_invalid_data;
            }
            else if (phone_number.bytes_in_string == 0)
            {
                result = connector_invalid_data_size;
            }
            else
            {
                connector_ptr->phone_dialed_length = phone_number.bytes_in_string;
                connector_ptr->phone_dialed = phone_number.string;
            }
            break;
        case connector_callback_busy:
        case connector_callback_unrecognized:
        case connector_callback_abort:
        case connector_callback_error:
            result = connector_abort;
            goto done;
        }

        if (result != connector_working)
        {
            notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, result);
            result = connector_abort;
        }
    }

done:
    return result;
}
#endif

static connector_status_t get_config_device_id_method(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

#if (defined CONNECTOR_DEVICE_ID_METHOD)
    connector_ptr->device_id_method = CONNECTOR_DEVICE_ID_METHOD;
#else
    connector_callback_status_t callback_status;

    connector_request_id_t request_id;
    connector_config_device_id_method_t device_id;

    request_id.config_request = connector_request_id_config_device_id_method;
    callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &device_id);
    switch (callback_status)
    {
    case connector_callback_continue:
        switch (device_id.method)
        {
        case connector_manual_device_id_method:
        case connector_auto_device_id_method:
            connector_ptr->device_id_method = device_id.method;
            break;

        default:
            notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data_range);
            result = connector_abort;
            break;
        }
        break;

    case connector_callback_unrecognized:
        connector_ptr->device_id_method = connector_manual_device_id_method;
        break;

    case connector_callback_busy:
    case connector_callback_abort:
    case connector_callback_error:
        result = connector_abort;
        break;
    }
#endif

    return result;
}

#define MAX_DECIMAL_DIGIT       9
#define MAX_HEXADECIMAL_DIGIT    0xF

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

static connector_status_t get_config_wan_id(connector_data_t * const connector_ptr,
                                            connector_request_id_config_t const config_request_id)
{
    connector_status_t result = connector_working;
    connector_callback_status_t callback_status;
    connector_config_pointer_data_t request_data;
    size_t  bytes_required;
    uint8_t max_digits ;

    connector_request_id_t request_id;

    switch (config_request_id)
    {
        case connector_request_id_config_imei_number:
            bytes_required = CONNECTOR_GSM_IMEI_LENGTH;
            max_digits = MAX_DECIMAL_DIGIT;
            break;

        case connector_request_id_config_esn:
            bytes_required = CONNECTOR_ESN_HEX_LENGTH;
            max_digits = MAX_HEXADECIMAL_DIGIT;
            break;

        case connector_request_id_config_meid:
            bytes_required = CONNECTOR_MEID_HEX_LENGTH;
            max_digits = MAX_HEXADECIMAL_DIGIT;
            break;

        default:
            ASSERT(connector_false);
            break;
    }

    request_data.bytes_required = bytes_required;
    request_id.config_request = config_request_id;

    callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &request_data);
    switch (callback_status)
    {
    case connector_callback_continue:
        if (request_data.data == NULL)
        {
            result = connector_invalid_data;
        }
        else if (request_data.bytes_required != bytes_required)
        {
            result = connector_invalid_data_size;
        }
        else if (check_digit_array(request_data.data, request_data.bytes_required, max_digits) != connector_true)
        {
            result = connector_invalid_data_range;
        }
        else
        {
            connector_ptr->wan_id = request_data.data;
            connector_ptr->wan_id_length = bytes_required;
        }
        break;
    case connector_callback_busy:
    case connector_callback_unrecognized:
    case connector_callback_abort:
    case connector_callback_error:
        result = connector_abort;
        goto done;
    }

   if (result != connector_working)
    {
        notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, result);
        result = connector_abort;
    }

done:
    return result;
}

static connector_status_t get_config_wan_type(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;
#if (defined CONNECTOR_WAN_TYPE)
    connector_ptr->wan_type = CONNECTOR_WAN_TYPE;
#else
    connector_callback_status_t callback_status;
    connector_request_id_t request_id;
    connector_config_wan_type_t config_wan;


    request_id.config_request = connector_request_id_config_wan_type;
    callback_status = connector_callback(connector_ptr->callback, connector_class_id_config, request_id, &config_wan);
    switch (callback_status)
    {
    case connector_callback_continue:
        switch (config_wan.type)
        {
        case connector_imei_wan_type:
        case connector_esn_wan_type:
        case connector_meid_wan_type:
            connector_ptr->wan_type = config_wan.type;
            break;
        default:
            notify_error_status(connector_ptr->callback, connector_class_id_config, request_id, connector_invalid_data_range);
            result = connector_abort;
            break;
        }
        break;
    case connector_callback_busy:
    case connector_callback_unrecognized:
    case connector_callback_abort:
    case connector_callback_error:
        result = connector_abort;
        break;
    }
#endif
    return result;
}


