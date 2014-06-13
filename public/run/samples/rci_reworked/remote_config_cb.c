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
#include "connector_api.h"
#include "remote_config.h"

#define UNUSED_PARAMETER(a)  (void)(a)

connector_callback_status_t app_remote_config_handler(connector_request_id_remote_config_t const request_id, void * const data)
{
    UNUSED_PARAMETER(data);
    UNUSED_PARAMETER(request_id);

    return connector_callback_continue;
}

connector_callback_status_t rci_session_start_cb(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_session_end_cb(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_setting_serial_start(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_end(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_setting_serial_baud_get(rci_info_t * const info, connector_setting_serial_baud_id_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = connector_setting_serial_baud_2400;
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_baud_set(rci_info_t * const info, connector_setting_serial_baud_id_t const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_parity_get(rci_info_t * const info, connector_setting_serial_parity_id_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = connector_setting_serial_parity_even;
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_parity_set(rci_info_t * const info, connector_setting_serial_parity_id_t const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_databits_get(rci_info_t * const info, uint32_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = 7;
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_databits_set(rci_info_t * const info, uint32_t const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_xbreak_get(rci_info_t * const info, connector_on_off_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = connector_on;
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_xbreak_set(rci_info_t * const info, connector_on_off_t const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_serial_txbytes_get(rci_info_t * const info, uint32_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = 123;
    return connector_callback_continue;
}


connector_callback_status_t rci_setting_ethernet_start(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_end(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_setting_ethernet_ip_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "192.168.1.1";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_ip_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_subnet_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "192.168.1.1";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_subnet_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_gateway_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "192.168.1.1";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_gateway_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_dhcp_get(rci_info_t * const info, connector_bool_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = connector_true;
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_dhcp_set(rci_info_t * const info, connector_bool_t const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_dns_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "192.168.1.1";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_dns_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_duplex_get(rci_info_t * const info, connector_setting_ethernet_duplex_id_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = connector_setting_ethernet_duplex_auto;
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_ethernet_duplex_set(rci_info_t * const info, connector_setting_ethernet_duplex_id_t const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_time_start(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_time_end(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_setting_device_time_curtime_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "2002-05-30T09:30:10-0600";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_time_curtime_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_start(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_end(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_setting_device_info_version_get(rci_info_t * const info, uint32_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = 0x20101010;
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_product_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "String";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_product_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_model_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "String";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_model_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_company_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "String";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_company_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_desc_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "String";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_device_info_desc_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_system_start(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_system_end(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_setting_system_description_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "String";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_system_description_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_system_contact_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "String";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_system_contact_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_system_location_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "String";
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_system_location_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_devicesecurity_start(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_devicesecurity_end(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_setting_devicesecurity_identityVerificationForm_get(rci_info_t * const info, connector_setting_devicesecurity_identityVerificationForm_id_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = connector_setting_devicesecurity_identityVerificationForm_simple;
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_devicesecurity_identityVerificationForm_set(rci_info_t * const info, connector_setting_devicesecurity_identityVerificationForm_id_t const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_setting_devicesecurity_password_set(rci_info_t * const info, char const * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_state_device_state_start(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_state_device_state_end(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_state_device_state_system_up_time_get(rci_info_t * const info, uint32_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = 1402643208;
    return connector_callback_continue;
}


connector_callback_status_t rci_state_device_state_signed_integer_get(rci_info_t * const info, int32_t * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = -1;
    return connector_callback_continue;
}

connector_callback_status_t rci_state_device_state_signed_integer_set(rci_info_t * const info, int32_t const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    UNUSED_PARAMETER(value);
    return connector_callback_continue;
}

connector_callback_status_t rci_state_gps_stats_start(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}

connector_callback_status_t rci_state_gps_stats_end(rci_info_t * const info)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    return connector_callback_continue;
}


connector_callback_status_t rci_state_gps_stats_latitude_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "51.23";
    return connector_callback_continue;
}


connector_callback_status_t rci_state_gps_stats_longitude_get(rci_info_t * const info, char const * * const value)
{
    UNUSED_PARAMETER(info);
    printf("    Called '%s'\n", __FUNCTION__);
    *value = "12.3";
    return connector_callback_continue;
}
