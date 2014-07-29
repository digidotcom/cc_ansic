/*
 * Copyright (c) 2014 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

typedef char const * json_t;
typedef char const * geojson_t;

typedef connector_bool_t (* enhs_query_fn_t)(unsigned int const index, void * const value);
typedef unsigned int (* enhs_get_instances_fn_t)(void);

typedef enum {
    ENHS_TYPE_NONE,
    ENHS_TYPE_INT32,
    ENHS_TYPE_UINT64,
    ENHS_TYPE_FLOAT,
    ENHS_TYPE_STRING,
    ENHS_TYPE_JSON,
    ENHS_TYPE_GEOJSON
} enhs_value_type_t;

typedef struct {
    char const * name;
    enhs_value_type_t type;
    enhs_query_fn_t getter;
} enhs_item_t;

typedef enum {
    ITEMS,
    SUBGROUPS
} enhs_path_type_t;

typedef struct enhs_path_group {
    char const * name;
    enhs_get_instances_fn_t multi_instance;

    struct {
        enhs_path_type_t type;
        union {
            struct {
                enhs_item_t const * const * array;
                unsigned int size;
            } items;
            struct {
                struct enhs_path_group const * const * array;
                unsigned int size;
            } subgroups;
        } data;
    } child;
} enhs_path_group_t;

connector_bool_t cc_enhs_get_net_latency_min(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_net_latency_avg(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_net_latency_max(unsigned int const index, int32_t * const value);

unsigned int enhs_get_mobile_instances(void);

connector_bool_t cc_enhs_get_mobile_module(unsigned int const index, json_t * const value);
connector_bool_t cc_enhs_get_mobile_status(unsigned int const index, char const * * const value);
connector_bool_t cc_enhs_get_mobile_rssi(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_ecio(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_rsrp(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_rsrq(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_sinr(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_snr(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_registration(unsigned int const index, char const * * const value);
connector_bool_t cc_enhs_get_mobile_lac(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_tac(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_session(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_mobile_temperature(unsigned int const index, float * const value);
connector_bool_t cc_enhs_get_mobile_sim0(unsigned int const index, json_t * const value);
connector_bool_t cc_enhs_get_mobile_sim0_status(unsigned int const index, char const * * const value);

unsigned int enhs_get_eth_instances(void);

connector_bool_t cc_enhs_get_eth_tx_bytes(unsigned int const index, uint64_t * const value);
connector_bool_t cc_enhs_get_eth_tx_packets(unsigned int const index, uint64_t * const value);
connector_bool_t cc_enhs_get_eth_rx_bytes(unsigned int const index, uint64_t * const value);
connector_bool_t cc_enhs_get_eth_rx_packets(unsigned int const index, uint64_t * const value);
connector_bool_t cc_enhs_get_eth_dropped_packets(unsigned int const index, uint64_t * const value);

unsigned int enhs_get_wifi_instances(void);

connector_bool_t cc_enhs_get_wifi_radio(unsigned int const index, json_t * const value);
connector_bool_t cc_enhs_get_wifi_status(unsigned int const index, char const * * const value);
connector_bool_t cc_enhs_get_wifi_rssi(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_wifi_rate(unsigned int const index, float * const value);
connector_bool_t cc_enhs_get_wifi_clients(unsigned int const index, int32_t * const value);

connector_bool_t cc_enhs_get_system_mem_free(unsigned int const index, uint64_t * const value);
connector_bool_t cc_enhs_get_system_mem_used(unsigned int const index, uint64_t * const value);
connector_bool_t cc_enhs_get_system_reboots(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_system_msg_free(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_system_msg_min(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_system_buf_free(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_system_buf_used(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_system_bigbuf_free(unsigned int const index, int32_t * const value);
connector_bool_t cc_enhs_get_system_bigbuf_used(unsigned int const index, int32_t * const value);

connector_bool_t cc_enhs_get_gps_location(unsigned int const index, json_t * const value);

static const enhs_item_t enhs_net_latency_min = {"min", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_net_latency_min};
static const enhs_item_t enhs_net_latency_avg = {"avg", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_net_latency_avg};
static const enhs_item_t enhs_net_latency_max = {"max", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_net_latency_max};

static enhs_item_t const * const enhs_net_latency_elements[] =
{
    &enhs_net_latency_min,
    &enhs_net_latency_avg,
    &enhs_net_latency_max
};

static enhs_path_group_t const enhs_net_latency =
{
    "latency",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_net_latency_elements,
                asizeof(enhs_net_latency_elements)
            }
        }
    }
};

static enhs_path_group_t const * const enhs_net_levels[] =
{
    &enhs_net_latency
};

static enhs_path_group_t const enhs_root_group_net =
{
    "net",
    NULL,
    {
        SUBGROUPS,
        {
            {
                (enhs_item_t const * const *)enhs_net_levels,
                asizeof(enhs_net_levels)
            }
        }
    }
};

static const enhs_item_t enhs_mobile_module = {"module", ENHS_TYPE_JSON, (enhs_query_fn_t)cc_enhs_get_mobile_module};
static const enhs_item_t enhs_mobile_status = {"status", ENHS_TYPE_STRING, (enhs_query_fn_t)cc_enhs_get_mobile_status};
static const enhs_item_t enhs_mobile_rssi = {"rssi", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_rssi};
static const enhs_item_t enhs_mobile_ecio = {"ecio", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_ecio};
static const enhs_item_t enhs_mobile_rsrp = {"rsrp", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_rsrp};
static const enhs_item_t enhs_mobile_rsrq = {"rsrq", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_rsrq};
static const enhs_item_t enhs_mobile_sinr = {"sinr", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_sinr};
static const enhs_item_t enhs_mobile_snr = {"snr", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_snr};
static const enhs_item_t enhs_mobile_registration = {"registration", ENHS_TYPE_STRING, (enhs_query_fn_t)cc_enhs_get_mobile_registration};
static const enhs_item_t enhs_mobile_lac = {"lac", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_lac};
static const enhs_item_t enhs_mobile_tac = {"tac", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_tac};
static const enhs_item_t enhs_mobile_session = {"session", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_mobile_session};
static const enhs_item_t enhs_mobile_temperature = {"temperature", ENHS_TYPE_FLOAT, (enhs_query_fn_t)cc_enhs_get_mobile_temperature};
static const enhs_item_t enhs_mobile_sim0 = {"sim0", ENHS_TYPE_JSON, (enhs_query_fn_t)cc_enhs_get_mobile_sim0};
static const enhs_item_t enhs_mobile_sim0_status = {"sim0_status", ENHS_TYPE_STRING, (enhs_query_fn_t)cc_enhs_get_mobile_sim0_status};

static enhs_item_t const * const enhs_mobile_elements[] =
{
    &enhs_mobile_module,
    &enhs_mobile_status,
    &enhs_mobile_rssi,
    &enhs_mobile_ecio,
    &enhs_mobile_rsrp,
    &enhs_mobile_rsrq,
    &enhs_mobile_sinr,
    &enhs_mobile_snr,
    &enhs_mobile_registration,
    &enhs_mobile_lac,
    &enhs_mobile_tac,
    &enhs_mobile_session,
    &enhs_mobile_temperature,
    &enhs_mobile_sim0,
    &enhs_mobile_sim0_status
};

static enhs_path_group_t const enhs_root_group_mobile =
{
    "mobile",
    enhs_get_mobile_instances,
    {
        ITEMS,
        {
            {
                enhs_mobile_elements,
                asizeof(enhs_mobile_elements)
            }
        }
    }
};

static const enhs_item_t enhs_eth_tx_bytes = {"bytes", ENHS_TYPE_UINT64, (enhs_query_fn_t)cc_enhs_get_eth_tx_bytes};
static const enhs_item_t enhs_eth_tx_packets = {"packets", ENHS_TYPE_UINT64, (enhs_query_fn_t)cc_enhs_get_eth_tx_packets};

static enhs_item_t const * const enhs_eth_tx_elements[] =
{
    &enhs_eth_tx_bytes,
    &enhs_eth_tx_packets
};

static enhs_path_group_t const enhs_eth_tx =
{
    "tx",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_eth_tx_elements,
                asizeof(enhs_eth_tx_elements)
            }
        }
    }
};

static const enhs_item_t enhs_eth_rx_bytes = {"bytes", ENHS_TYPE_UINT64, (enhs_query_fn_t)cc_enhs_get_eth_rx_bytes};
static const enhs_item_t enhs_eth_rx_packets = {"packets", ENHS_TYPE_UINT64, (enhs_query_fn_t)cc_enhs_get_eth_rx_packets};

static enhs_item_t const * const enhs_eth_rx_elements[] =
{
    &enhs_eth_rx_bytes,
    &enhs_eth_rx_packets
};

static enhs_path_group_t const enhs_eth_rx =
{
    "rx",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_eth_rx_elements,
                asizeof(enhs_eth_rx_elements)
            }
        }
    }
};

static const enhs_item_t enhs_eth_dropped_packets = {"packets", ENHS_TYPE_UINT64, (enhs_query_fn_t)cc_enhs_get_eth_dropped_packets};

static enhs_item_t const * const enhs_eth_dropped_elements[] =
{
    &enhs_eth_dropped_packets
};

static enhs_path_group_t const enhs_eth_dropped =
{
    "dropped",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_eth_dropped_elements,
                asizeof(enhs_eth_dropped_elements)
            }
        }
    }
};

static enhs_path_group_t const * const enhs_eth_levels[] =
{
    &enhs_eth_tx,
    &enhs_eth_rx,
    &enhs_eth_dropped
};

static enhs_path_group_t const enhs_root_group_eth =
{
    "eth",
    enhs_get_eth_instances,
    {
        SUBGROUPS,
        {
            {
                (enhs_item_t const * const *)enhs_eth_levels,
                asizeof(enhs_eth_levels)
            }
        }
    }
};

static const enhs_item_t enhs_wifi_radio = {"radio", ENHS_TYPE_JSON, (enhs_query_fn_t)cc_enhs_get_wifi_radio};
static const enhs_item_t enhs_wifi_status = {"status", ENHS_TYPE_STRING, (enhs_query_fn_t)cc_enhs_get_wifi_status};
static const enhs_item_t enhs_wifi_rssi = {"rssi", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_wifi_rssi};
static const enhs_item_t enhs_wifi_rate = {"rate", ENHS_TYPE_FLOAT, (enhs_query_fn_t)cc_enhs_get_wifi_rate};
static const enhs_item_t enhs_wifi_clients = {"clients", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_wifi_clients};

static enhs_item_t const * const enhs_wifi_elements[] =
{
    &enhs_wifi_radio,
    &enhs_wifi_status,
    &enhs_wifi_rssi,
    &enhs_wifi_rate,
    &enhs_wifi_clients
};

static enhs_path_group_t const enhs_root_group_wifi =
{
    "wifi",
    enhs_get_wifi_instances,
    {
        ITEMS,
        {
            {
                enhs_wifi_elements,
                asizeof(enhs_wifi_elements)
            }
        }
    }
};

static const enhs_item_t enhs_system_mem_free = {"free", ENHS_TYPE_UINT64, (enhs_query_fn_t)cc_enhs_get_system_mem_free};
static const enhs_item_t enhs_system_mem_used = {"used", ENHS_TYPE_UINT64, (enhs_query_fn_t)cc_enhs_get_system_mem_used};

static enhs_item_t const * const enhs_system_mem_elements[] =
{
    &enhs_system_mem_free,
    &enhs_system_mem_used
};

static enhs_path_group_t const enhs_system_mem =
{
    "mem",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_system_mem_elements,
                asizeof(enhs_system_mem_elements)
            }
        }
    }
};

static const enhs_item_t enhs_system_reboots_items = {"", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_system_reboots};

static enhs_item_t const * const enhs_system_reboots_elements[] =
{
    &enhs_system_reboots_items,
};

static enhs_path_group_t const enhs_system_reboots =
{
    "reboots",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_system_reboots_elements,
                asizeof(enhs_system_reboots_elements)
            }
        }
    }
};

static const enhs_item_t enhs_system_msg_free = {"free", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_system_msg_free};
static const enhs_item_t enhs_system_mem_min = {"min", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_system_msg_min};

static enhs_item_t const * const enhs_system_msg_elements[] =
{
    &enhs_system_msg_free,
    &enhs_system_mem_min
};

static enhs_path_group_t const enhs_system_msg =
{
    "msg",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_system_msg_elements,
                 asizeof(enhs_system_msg_elements)
            }
        }

    }
};

static const enhs_item_t enhs_system_buf_free = {"free", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_system_buf_free};
static const enhs_item_t enhs_system_buf_used = {"used", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_system_buf_used};

static enhs_item_t const * const enhs_system_buf_elements[] =
{
    &enhs_system_buf_free,
    &enhs_system_buf_used
};

static enhs_path_group_t const enhs_system_buf =
{
    "buf",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_system_buf_elements,
                asizeof(enhs_system_buf_elements)
            }
        }
    }
};

static const enhs_item_t enhs_system_bigbuf_free = {"free", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_system_bigbuf_free};
static const enhs_item_t enhs_system_bigbuf_used = {"used", ENHS_TYPE_INT32, (enhs_query_fn_t)cc_enhs_get_system_bigbuf_used};

static enhs_item_t const * const enhs_system_bigbuf_elements[] =
{
    &enhs_system_bigbuf_free,
    &enhs_system_bigbuf_used
};

static enhs_path_group_t const enhs_system_bigbuf =
{
    "bigbuf",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_system_bigbuf_elements,
                asizeof(enhs_system_bigbuf_elements)
            }
        }
    }
};

static enhs_path_group_t const * const enhs_system_groups[] =
{
    &enhs_system_mem,
    &enhs_system_reboots,
    &enhs_system_msg,
    &enhs_system_buf,
    &enhs_system_bigbuf
};

static enhs_path_group_t const enhs_root_group_system =
{
    "system",
    NULL,
    {
        SUBGROUPS,
        {
            {
                (enhs_item_t const * const *)enhs_system_groups,
                asizeof(enhs_system_groups)
            }
        }
    }
};

static const enhs_item_t enhs_gps_location = {"location", ENHS_TYPE_GEOJSON, (enhs_query_fn_t)cc_enhs_get_gps_location};

static enhs_item_t const * const enhs_gps_elements[] =
{
    &enhs_gps_location
};

static enhs_path_group_t const enhs_root_group_gps =
{
    "gps",
    NULL,
    {
        ITEMS,
        {
            {
                enhs_gps_elements,
                asizeof(enhs_gps_elements)
            }
        }
    }
};

static enhs_path_group_t const * const enhs_root_groups[] =
{
    &enhs_root_group_net,
    &enhs_root_group_mobile,
    &enhs_root_group_eth,
    &enhs_root_group_wifi,
    &enhs_root_group_system,
    &enhs_root_group_gps
};
