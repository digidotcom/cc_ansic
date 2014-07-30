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

typedef connector_bool_t (* dev_health_query_fn_t)(unsigned int const index, void * const value);
typedef unsigned int (* dev_health_get_instances_fn_t)(void);

typedef enum {
    DEV_HEALTH_TYPE_NONE,
    DEV_HEALTH_TYPE_INT32,
    DEV_HEALTH_TYPE_UINT64,
    DEV_HEALTH_TYPE_FLOAT,
    DEV_HEALTH_TYPE_STRING,
    DEV_HEALTH_TYPE_JSON,
    DEV_HEALTH_TYPE_GEOJSON
} dev_health_value_type_t;

typedef struct {
    char const * name;
    dev_health_value_type_t type;
    dev_health_query_fn_t getter;
} dev_health_item_t;

typedef enum {
    ITEMS,
    SUBGROUPS
} dev_health_path_type_t;

typedef struct dev_health_path_group {
    char const * name;
    dev_health_get_instances_fn_t multi_instance;

    struct {
        dev_health_path_type_t type;
        union {
            struct {
                dev_health_item_t const * const * array;
                unsigned int size;
            } items;
            struct {
                struct dev_health_path_group const * const * array;
                unsigned int size;
            } subgroups;
        } data;
    } child;
} dev_health_path_group_t;

connector_bool_t cc_dev_health_get_net_latency_min(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_net_latency_avg(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_net_latency_max(unsigned int const index, int32_t * const value);

unsigned int cc_dev_health_get_mobile_instances(void);

connector_bool_t cc_dev_health_get_mobile_module(unsigned int const index, json_t * const value);
connector_bool_t cc_dev_health_get_mobile_status(unsigned int const index, char const * * const value);
connector_bool_t cc_dev_health_get_mobile_rssi(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_ecio(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_rsrp(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_rsrq(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_sinr(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_snr(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_registration(unsigned int const index, char const * * const value);
connector_bool_t cc_dev_health_get_mobile_lac(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_tac(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_session(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_temperature(unsigned int const index, float * const value);
connector_bool_t cc_dev_health_get_mobile_sim0(unsigned int const index, json_t * const value);
connector_bool_t cc_dev_health_get_mobile_sim0_status(unsigned int const index, char const * * const value);

unsigned int cc_dev_health_get_eth_instances(void);

connector_bool_t cc_dev_health_get_eth_tx_bytes(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_tx_packets(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_rx_bytes(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_rx_packets(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_dropped_packets(unsigned int const index, uint64_t * const value);

unsigned int cc_dev_health_get_wifi_instances(void);

connector_bool_t cc_dev_health_get_wifi_radio(unsigned int const index, json_t * const value);
connector_bool_t cc_dev_health_get_wifi_status(unsigned int const index, char const * * const value);
connector_bool_t cc_dev_health_get_wifi_rssi(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_wifi_rate(unsigned int const index, float * const value);
connector_bool_t cc_dev_health_get_wifi_clients(unsigned int const index, int32_t * const value);

connector_bool_t cc_dev_health_get_system_mem_free(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_system_mem_used(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_system_reboots(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_system_msg_free(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_system_msg_min(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_system_buf_free(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_system_buf_used(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_system_bigbuf_free(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_system_bigbuf_used(unsigned int const index, int32_t * const value);

connector_bool_t cc_dev_health_get_gps_location(unsigned int const index, json_t * const value);

static const dev_health_item_t dev_health_net_latency_min = {"min", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_net_latency_min};
static const dev_health_item_t dev_health_net_latency_avg = {"avg", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_net_latency_avg};
static const dev_health_item_t dev_health_net_latency_max = {"max", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_net_latency_max};

static dev_health_item_t const * const dev_health_net_latency_elements[] =
{
    &dev_health_net_latency_min,
    &dev_health_net_latency_avg,
    &dev_health_net_latency_max
};

static dev_health_path_group_t const dev_health_net_latency =
{
    "latency",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_net_latency_elements,
                asizeof(dev_health_net_latency_elements)
            }
        }
    }
};

static dev_health_path_group_t const * const dev_health_net_levels[] =
{
    &dev_health_net_latency
};

static dev_health_path_group_t const dev_health_root_group_net =
{
    "net",
    NULL,
    {
        SUBGROUPS,
        {
            {
                (dev_health_item_t const * const *)dev_health_net_levels,
                asizeof(dev_health_net_levels)
            }
        }
    }
};

static const dev_health_item_t dev_health_mobile_module = {"module", DEV_HEALTH_TYPE_JSON, (dev_health_query_fn_t)cc_dev_health_get_mobile_module};
static const dev_health_item_t dev_health_mobile_status = {"status", DEV_HEALTH_TYPE_STRING, (dev_health_query_fn_t)cc_dev_health_get_mobile_status};
static const dev_health_item_t dev_health_mobile_rssi = {"rssi", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_rssi};
static const dev_health_item_t dev_health_mobile_ecio = {"ecio", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_ecio};
static const dev_health_item_t dev_health_mobile_rsrp = {"rsrp", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_rsrp};
static const dev_health_item_t dev_health_mobile_rsrq = {"rsrq", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_rsrq};
static const dev_health_item_t dev_health_mobile_sinr = {"sinr", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_sinr};
static const dev_health_item_t dev_health_mobile_snr = {"snr", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_snr};
static const dev_health_item_t dev_health_mobile_registration = {"registration", DEV_HEALTH_TYPE_STRING, (dev_health_query_fn_t)cc_dev_health_get_mobile_registration};
static const dev_health_item_t dev_health_mobile_lac = {"lac", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_lac};
static const dev_health_item_t dev_health_mobile_tac = {"tac", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_tac};
static const dev_health_item_t dev_health_mobile_session = {"session", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_mobile_session};
static const dev_health_item_t dev_health_mobile_temperature = {"temperature", DEV_HEALTH_TYPE_FLOAT, (dev_health_query_fn_t)cc_dev_health_get_mobile_temperature};
static const dev_health_item_t dev_health_mobile_sim0 = {"sim0", DEV_HEALTH_TYPE_JSON, (dev_health_query_fn_t)cc_dev_health_get_mobile_sim0};
static const dev_health_item_t dev_health_mobile_sim0_status = {"sim0_status", DEV_HEALTH_TYPE_STRING, (dev_health_query_fn_t)cc_dev_health_get_mobile_sim0_status};

static dev_health_item_t const * const dev_health_mobile_elements[] =
{
    &dev_health_mobile_module,
    &dev_health_mobile_status,
    &dev_health_mobile_rssi,
    &dev_health_mobile_ecio,
    &dev_health_mobile_rsrp,
    &dev_health_mobile_rsrq,
    &dev_health_mobile_sinr,
    &dev_health_mobile_snr,
    &dev_health_mobile_registration,
    &dev_health_mobile_lac,
    &dev_health_mobile_tac,
    &dev_health_mobile_session,
    &dev_health_mobile_temperature,
    &dev_health_mobile_sim0,
    &dev_health_mobile_sim0_status
};

static dev_health_path_group_t const dev_health_root_group_mobile =
{
    "mobile",
    cc_dev_health_get_mobile_instances,
    {
        ITEMS,
        {
            {
                dev_health_mobile_elements,
                asizeof(dev_health_mobile_elements)
            }
        }
    }
};

static const dev_health_item_t dev_health_eth_tx_bytes = {"bytes", DEV_HEALTH_TYPE_UINT64, (dev_health_query_fn_t)cc_dev_health_get_eth_tx_bytes};
static const dev_health_item_t dev_health_eth_tx_packets = {"packets", DEV_HEALTH_TYPE_UINT64, (dev_health_query_fn_t)cc_dev_health_get_eth_tx_packets};

static dev_health_item_t const * const dev_health_eth_tx_elements[] =
{
    &dev_health_eth_tx_bytes,
    &dev_health_eth_tx_packets
};

static dev_health_path_group_t const dev_health_eth_tx =
{
    "tx",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_eth_tx_elements,
                asizeof(dev_health_eth_tx_elements)
            }
        }
    }
};

static const dev_health_item_t dev_health_eth_rx_bytes = {"bytes", DEV_HEALTH_TYPE_UINT64, (dev_health_query_fn_t)cc_dev_health_get_eth_rx_bytes};
static const dev_health_item_t dev_health_eth_rx_packets = {"packets", DEV_HEALTH_TYPE_UINT64, (dev_health_query_fn_t)cc_dev_health_get_eth_rx_packets};

static dev_health_item_t const * const dev_health_eth_rx_elements[] =
{
    &dev_health_eth_rx_bytes,
    &dev_health_eth_rx_packets
};

static dev_health_path_group_t const dev_health_eth_rx =
{
    "rx",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_eth_rx_elements,
                asizeof(dev_health_eth_rx_elements)
            }
        }
    }
};

static const dev_health_item_t dev_health_eth_dropped_packets = {"packets", DEV_HEALTH_TYPE_UINT64, (dev_health_query_fn_t)cc_dev_health_get_eth_dropped_packets};

static dev_health_item_t const * const dev_health_eth_dropped_elements[] =
{
    &dev_health_eth_dropped_packets
};

static dev_health_path_group_t const dev_health_eth_dropped =
{
    "dropped",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_eth_dropped_elements,
                asizeof(dev_health_eth_dropped_elements)
            }
        }
    }
};

static dev_health_path_group_t const * const dev_health_eth_levels[] =
{
    &dev_health_eth_tx,
    &dev_health_eth_rx,
    &dev_health_eth_dropped
};

static dev_health_path_group_t const dev_health_root_group_eth =
{
    "eth",
    cc_dev_health_get_eth_instances,
    {
        SUBGROUPS,
        {
            {
                (dev_health_item_t const * const *)dev_health_eth_levels,
                asizeof(dev_health_eth_levels)
            }
        }
    }
};

static const dev_health_item_t dev_health_wifi_radio = {"radio", DEV_HEALTH_TYPE_JSON, (dev_health_query_fn_t)cc_dev_health_get_wifi_radio};
static const dev_health_item_t dev_health_wifi_status = {"status", DEV_HEALTH_TYPE_STRING, (dev_health_query_fn_t)cc_dev_health_get_wifi_status};
static const dev_health_item_t dev_health_wifi_rssi = {"rssi", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_wifi_rssi};
static const dev_health_item_t dev_health_wifi_rate = {"rate", DEV_HEALTH_TYPE_FLOAT, (dev_health_query_fn_t)cc_dev_health_get_wifi_rate};
static const dev_health_item_t dev_health_wifi_clients = {"clients", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_wifi_clients};

static dev_health_item_t const * const dev_health_wifi_elements[] =
{
    &dev_health_wifi_radio,
    &dev_health_wifi_status,
    &dev_health_wifi_rssi,
    &dev_health_wifi_rate,
    &dev_health_wifi_clients
};

static dev_health_path_group_t const dev_health_root_group_wifi =
{
    "wifi",
    cc_dev_health_get_wifi_instances,
    {
        ITEMS,
        {
            {
                dev_health_wifi_elements,
                asizeof(dev_health_wifi_elements)
            }
        }
    }
};

static const dev_health_item_t dev_health_system_mem_free = {"free", DEV_HEALTH_TYPE_UINT64, (dev_health_query_fn_t)cc_dev_health_get_system_mem_free};
static const dev_health_item_t dev_health_system_mem_used = {"used", DEV_HEALTH_TYPE_UINT64, (dev_health_query_fn_t)cc_dev_health_get_system_mem_used};

static dev_health_item_t const * const dev_health_system_mem_elements[] =
{
    &dev_health_system_mem_free,
    &dev_health_system_mem_used
};

static dev_health_path_group_t const dev_health_system_mem =
{
    "mem",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_system_mem_elements,
                asizeof(dev_health_system_mem_elements)
            }
        }
    }
};

static const dev_health_item_t dev_health_system_reboots_items = {"", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_system_reboots};

static dev_health_item_t const * const dev_health_system_reboots_elements[] =
{
    &dev_health_system_reboots_items,
};

static dev_health_path_group_t const dev_health_system_reboots =
{
    "reboots",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_system_reboots_elements,
                asizeof(dev_health_system_reboots_elements)
            }
        }
    }
};

static const dev_health_item_t dev_health_system_msg_free = {"free", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_system_msg_free};
static const dev_health_item_t dev_health_system_mem_min = {"min", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_system_msg_min};

static dev_health_item_t const * const dev_health_system_msg_elements[] =
{
    &dev_health_system_msg_free,
    &dev_health_system_mem_min
};

static dev_health_path_group_t const dev_health_system_msg =
{
    "msg",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_system_msg_elements,
                 asizeof(dev_health_system_msg_elements)
            }
        }

    }
};

static const dev_health_item_t dev_health_system_buf_free = {"free", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_system_buf_free};
static const dev_health_item_t dev_health_system_buf_used = {"used", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_system_buf_used};

static dev_health_item_t const * const dev_health_system_buf_elements[] =
{
    &dev_health_system_buf_free,
    &dev_health_system_buf_used
};

static dev_health_path_group_t const dev_health_system_buf =
{
    "buf",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_system_buf_elements,
                asizeof(dev_health_system_buf_elements)
            }
        }
    }
};

static const dev_health_item_t dev_health_system_bigbuf_free = {"free", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_system_bigbuf_free};
static const dev_health_item_t dev_health_system_bigbuf_used = {"used", DEV_HEALTH_TYPE_INT32, (dev_health_query_fn_t)cc_dev_health_get_system_bigbuf_used};

static dev_health_item_t const * const dev_health_system_bigbuf_elements[] =
{
    &dev_health_system_bigbuf_free,
    &dev_health_system_bigbuf_used
};

static dev_health_path_group_t const dev_health_system_bigbuf =
{
    "bigbuf",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_system_bigbuf_elements,
                asizeof(dev_health_system_bigbuf_elements)
            }
        }
    }
};

static dev_health_path_group_t const * const dev_health_system_groups[] =
{
    &dev_health_system_mem,
    &dev_health_system_reboots,
    &dev_health_system_msg,
    &dev_health_system_buf,
    &dev_health_system_bigbuf
};

static dev_health_path_group_t const dev_health_root_group_system =
{
    "system",
    NULL,
    {
        SUBGROUPS,
        {
            {
                (dev_health_item_t const * const *)dev_health_system_groups,
                asizeof(dev_health_system_groups)
            }
        }
    }
};

static const dev_health_item_t dev_health_gps_location = {"location", DEV_HEALTH_TYPE_GEOJSON, (dev_health_query_fn_t)cc_dev_health_get_gps_location};

static dev_health_item_t const * const dev_health_gps_elements[] =
{
    &dev_health_gps_location
};

static dev_health_path_group_t const dev_health_root_group_gps =
{
    "gps",
    NULL,
    {
        ITEMS,
        {
            {
                dev_health_gps_elements,
                asizeof(dev_health_gps_elements)
            }
        }
    }
};

static dev_health_path_group_t const * const dev_health_root_groups[] =
{
    &dev_health_root_group_net,
    &dev_health_root_group_mobile,
    &dev_health_root_group_eth,
    &dev_health_root_group_wifi,
    &dev_health_root_group_system,
    &dev_health_root_group_gps
};
