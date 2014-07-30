#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "connector_api.h"

/* #define DEBUG */

typedef char const * json_t;
typedef char const * geojson_t;

#if !(defined UNUSED_ARGUMENT)
#define UNUSED_ARGUMENT(a)  (void)(a)
#endif

#if (defined DEBUG)
#define PRINT_FUNCTION_NAME()   do{printf("Function: %s\n", __FUNCTION__);}while(0)
#else
#define PRINT_FUNCTION_NAME()
#endif

connector_bool_t cc_enhs_get_net_latency_min(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_net_latency_avg(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_net_latency_max(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


unsigned int enhs_get_mobile_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 1;
}

connector_bool_t cc_enhs_get_mobile_module(unsigned int const index, json_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "{\"modem_id\": \"<modem id>\",\"cell_id\": \"<cell ID>\",\"network\": \"verizon,at&t,...\",\"sims\": 2,\"active_sim\": 1}";
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_status(unsigned int const index, char const * * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "Status";
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_rssi(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_ecio(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_rsrp(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_rsrq(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_sinr(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_snr(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_registration(unsigned int const index, char const * * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "Registration";
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_lac(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_tac(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_session(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_temperature(unsigned int const index, float * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 25.3;
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_sim0(unsigned int const index, json_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "{\"iccid\": \"<iccid>\",\"imsi\": \"<imsi>\",\"phone_num\": \"<phone number>\"}";
    return connector_true;
}


connector_bool_t cc_enhs_get_mobile_sim0_status(unsigned int const index, char const * * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "sim0 status";
    return connector_true;
}

unsigned int enhs_get_eth_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 4;
}

connector_bool_t cc_enhs_get_eth_tx_bytes(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_enhs_get_eth_tx_packets(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_enhs_get_eth_rx_bytes(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_enhs_get_eth_rx_packets(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_enhs_get_eth_dropped_packets(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


unsigned int enhs_get_wifi_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 1;
}

connector_bool_t cc_enhs_get_wifi_radio(unsigned int const index, json_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "{\"mode\": \"<client_or_access_point>\",\"ssid\": \"<ssid>\",\"channel\": <channel_number>,\"protocol\": \"<protocol_B_G_N_A)\"}";
    return connector_true;
}


connector_bool_t cc_enhs_get_wifi_status(unsigned int const index, char const * * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "wifi status";
    return connector_true;
}


connector_bool_t cc_enhs_get_wifi_rssi(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_wifi_rate(unsigned int const index, float * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 1.159;
    return connector_true;
}


connector_bool_t cc_enhs_get_wifi_clients(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}



connector_bool_t cc_enhs_get_system_mem_free(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_enhs_get_system_mem_used(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_enhs_get_system_reboots(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_system_msg_free(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_system_msg_min(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_system_buf_free(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_system_buf_used(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_system_bigbuf_free(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_enhs_get_system_bigbuf_used(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}

connector_bool_t cc_enhs_get_gps_location(unsigned int const index, geojson_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "{\"type\": \"Point\",\"coordinates\": [-102.3046875, 36.1733569352216]}";
    return connector_true;
}


