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

#define JSON_MAX_SIZE   128

STATIC unsigned int count_special_chars(char const * const string)
{
    unsigned int i;
    unsigned int escaped_chars = 0;

    for (i = 0; string[i] != '\0'; i++)
    {
        if (string[i] == '"' || string[i] == '\\')
        {
            escaped_chars++;
        }
    }

    return escaped_chars;
}

STATIC void escape_string(char * * const string)
{
    unsigned int escape_chars;

    ASSERT(*string != NULL);
    escape_chars = count_special_chars(*string);

    if (escape_chars)
    {
        unsigned int const new_size = strlen(*string) + escape_chars + 1;
        char * string_escaped = cc_dev_health_malloc_string(new_size);

        ASSERT(string_escaped != NULL);
        dp_process_string(*string, string_escaped, new_size, NULL, connector_false, connector_true);
        cc_dev_health_free_string(*string);
        *string = string_escaped;
    }
}

STATIC connector_bool_t cc_dev_health_get_mobile_module_json(unsigned int const index, connector_json_t * const value)
{
    connector_bool_t const present = cc_dev_health_mobile_module_present(index);

    if (present)
    {
        char * const module_json = cc_dev_health_malloc_string(JSON_MAX_SIZE);
        static const char module_json_format[] = "{\"modem_id\":\"%s\",\"cell_id\":\"%s\", \"network\":\"%s\",\"sims\":%u,\"active_sim\":%u}";
        char * modem_id = NULL;
        char * cell_id = NULL;
        char * network = NULL;
        unsigned int sims;
        unsigned int active_sim;
        int json_size;

        ASSERT(module_json != NULL);
        cc_dev_health_get_mobile_module_modem_id(index, &modem_id);
        cc_dev_health_get_mobile_module_cell_id(index, &cell_id);
        cc_dev_health_get_mobile_module_network(index, &network);
        cc_dev_health_get_mobile_module_sims(index, &sims);
        cc_dev_health_get_mobile_module_active_sim(index, &active_sim);

        ASSERT(modem_id != NULL);
        ASSERT(cell_id != NULL);
        ASSERT(network != NULL);

        escape_string(&modem_id);
        escape_string(&cell_id);
        escape_string(&network);

        json_size = sprintf(module_json, module_json_format, modem_id, cell_id, network, sims, active_sim);
        UNUSED_VARIABLE(json_size); /* Prevent variable set but not used in non-debug */
        ASSERT(json_size <= JSON_MAX_SIZE);

        cc_dev_health_free_string(modem_id);
        cc_dev_health_free_string(cell_id);
        cc_dev_health_free_string(network);

        *value = module_json;
    }

    return present;
}

STATIC connector_bool_t cc_dev_health_get_mobile_sim0_json(unsigned int const index, connector_json_t * const value)
{
    connector_bool_t const present = cc_dev_health_get_mobile_sim0_present(index);

    if (present)
    {
        char * const sim0_json = cc_dev_health_malloc_string(JSON_MAX_SIZE);
        static const char sim0_json_format[] = "{\"iccid\":\"%s\",\"imsi\":\"%s\",\"phone_num\":\"%s\"}";
        char * iccid = NULL;
        char * imsi = NULL;
        char * phone_num = NULL;
        int json_size;

        ASSERT(sim0_json != NULL);

        cc_dev_health_get_sim0_iccid(index, &iccid);
        cc_dev_health_get_sim0_imsi(index, &imsi);
        cc_dev_health_get_sim0_phone_num(index, &phone_num);

        ASSERT(iccid != NULL);
        ASSERT(imsi != NULL);
        ASSERT(phone_num != NULL);

        escape_string(&iccid);
        escape_string(&imsi);
        escape_string(&phone_num);

        json_size = sprintf(sim0_json, sim0_json_format, iccid, imsi, phone_num);
        UNUSED_VARIABLE(json_size); /* Prevent variable set but not used in non-debug */
        ASSERT(json_size <= JSON_MAX_SIZE);

        cc_dev_health_free_string(iccid);
        cc_dev_health_free_string(imsi);
        cc_dev_health_free_string(phone_num);

        *value = sim0_json;
    }

    return present;
}

STATIC connector_bool_t cc_dev_health_get_wifi_radio_json(unsigned int const index, connector_geojson_t * const value)
{
    connector_bool_t const present = cc_dev_health_get_wifi_radio_present(index);

    if (present)
    {
        char * const radio_json = cc_dev_health_malloc_string(JSON_MAX_SIZE);
        static const char radio_json_format[] = "{\"mode\":\"%s\",\"ssid\":\"%s\",\"channel\":%u,\"protocol\":\"%s\"}";
        char * mode = NULL;
        char * ssid = NULL;
        char * protocol = NULL;
        unsigned int channel = 0;
        int json_size;

        ASSERT(radio_json != NULL);

        cc_dev_health_get_wifi_radio_mode(index, &mode);
        cc_dev_health_get_wifi_radio_ssid(index, &ssid);
        cc_dev_health_get_wifi_radio_protocol(index, &protocol);
        cc_dev_health_get_wifi_radio_channel(index, &channel);

        ASSERT(mode != NULL);
        ASSERT(ssid != NULL);
        ASSERT(protocol != NULL);

        escape_string(&mode);
        escape_string(&ssid);
        escape_string(&protocol);

        json_size = sprintf(radio_json, radio_json_format, mode, ssid, channel, protocol);
        UNUSED_VARIABLE(json_size); /* Prevent variable set but not used in non-debug */
        ASSERT(json_size <= JSON_MAX_SIZE);

        cc_dev_health_free_string(mode);
        cc_dev_health_free_string(ssid);
        cc_dev_health_free_string(protocol);

        *value = radio_json;
    }

    return present;
}

STATIC connector_bool_t cc_dev_health_get_gps_location_geojson(unsigned int const index, connector_geojson_t * const value)
{
    connector_bool_t const present = cc_dev_health_get_gps_location_present(index);

    if (present)
    {
        char * const location_json = cc_dev_health_malloc_string(JSON_MAX_SIZE);
        static const char gps_location_json_format[] = "{\"type\":\"Point\",\"coordinates\":[%f,%f]}";
        float latitude;
        float longitude;
        int json_size;

        ASSERT(location_json != NULL);
        cc_dev_health_get_gps_location_latitude(index, &latitude);
        cc_dev_health_get_gps_location_longitude(index, &longitude);

        json_size = sprintf(location_json, gps_location_json_format, latitude, longitude);
        UNUSED_VARIABLE(json_size); /* Prevent variable set but not used in non-debug */
        ASSERT(json_size <= JSON_MAX_SIZE);

        *value = location_json;
    }

    return present;
}

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

static const dev_health_item_t dev_health_mobile_module = {"module", DEV_HEALTH_TYPE_JSON, (dev_health_query_fn_t)cc_dev_health_get_mobile_module_json};
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
static const dev_health_item_t dev_health_mobile_sim0 = {"sim0", DEV_HEALTH_TYPE_JSON, (dev_health_query_fn_t)cc_dev_health_get_mobile_sim0_json};
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

static const dev_health_item_t dev_health_wifi_radio = {"radio", DEV_HEALTH_TYPE_JSON, (dev_health_query_fn_t)cc_dev_health_get_wifi_radio_json};
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

static const dev_health_item_t dev_health_gps_location = {"location", DEV_HEALTH_TYPE_GEOJSON, (dev_health_query_fn_t)cc_dev_health_get_gps_location_geojson};

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
