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
#ifndef CONNECTOR_API_DEVICE_HEALTH_H_
#define CONNECTOR_API_DEVICE_HEALTH_H_

#if (defined CONNECTOR_DEVICE_HEALTH)

#define DEV_HEALTH_MAX_STREAM_ID_LEN               32

typedef struct {
    char path[DEV_HEALTH_MAX_STREAM_ID_LEN];
    unsigned long sampling_interval;
    unsigned long reporting_interval;
} dev_health_metrics_config_t;

connector_callback_status_t cc_dev_health_load_metrics(dev_health_metrics_config_t * const metrics_array, unsigned int array_size);
connector_callback_status_t cc_dev_health_save_metrics(dev_health_metrics_config_t const * const metrics_array, unsigned int array_size);

char * cc_dev_health_malloc_string(size_t size);
void cc_dev_health_free_string(char const * const string);

uint32_t cc_dev_health_get_posix_time(void);
connector_bool_t cc_dev_health_get_net_latency_min(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_net_latency_avg(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_net_latency_max(unsigned int const index, int32_t * const value);

connector_bool_t cc_dev_health_get_net_transactions_count(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_net_drop_count(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_net_oos_count(unsigned int const index, int32_t * const value);


unsigned int cc_dev_health_get_mobile_instances(void);

void cc_dev_health_get_mobile_module_modem_id(unsigned int const index, char * * const value);
void cc_dev_health_get_mobile_module_cell_id(unsigned int const index, char * * const value);
void cc_dev_health_get_mobile_module_network(unsigned int const index, char * * const value);
void cc_dev_health_get_mobile_module_sims(unsigned int const index, unsigned int * const value);
void cc_dev_health_get_mobile_module_active_sim(unsigned int const index, unsigned int * const value);

void cc_dev_health_get_mobile_net_sim_iccid(unsigned int const index, char * * const value);
void cc_dev_health_get_mobile_net_sim_imsi(unsigned int const index, char * * const value);
void cc_dev_health_get_mobile_net_sim_phone_num(unsigned int const index, char * * const value);

connector_bool_t cc_dev_health_mobile_module_present(unsigned int const index);
connector_bool_t cc_dev_health_get_mobile_net_present(unsigned int const index);

connector_bool_t cc_dev_health_get_mobile_net_status(unsigned int const index, char * * const value);
connector_bool_t cc_dev_health_get_mobile_net_rssi(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_ecio(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_rsrp(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_rsrq(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_sinr(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_snr(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_registration(unsigned int const index, char * * const value);
connector_bool_t cc_dev_health_get_mobile_net_lac(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_tac(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_session(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_mobile_net_temperature(unsigned int const index, float * const value);

unsigned int cc_dev_health_get_eth_instances(void);

connector_bool_t cc_dev_health_get_eth_tx_bytes(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_tx_packets(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_tx_dropped(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_tx_overruns(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_rx_bytes(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_rx_packets(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_rx_dropped(unsigned int const index, uint64_t * const value);

connector_bool_t cc_dev_health_get_eth_link_down_count(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_link_down_duration(unsigned int const index, uint64_t * const value);

unsigned int cc_dev_health_get_wifi_instances(void);

void cc_dev_health_get_wifi_radio_mode(unsigned int const index, char * * const value);
void cc_dev_health_get_wifi_radio_ssid(unsigned int const index, char * * const value);
void cc_dev_health_get_wifi_radio_channel(unsigned int const index, unsigned int * const value);
void cc_dev_health_get_wifi_radio_protocol(unsigned int const index, char * * const value);
connector_bool_t cc_dev_health_get_wifi_radio_present(unsigned int const index);

connector_bool_t cc_dev_health_get_wifi_status(unsigned int const index, char * * const value);
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

void cc_dev_health_get_gps_location_latitude(unsigned int const index, float * const value);
void cc_dev_health_get_gps_location_longitude(unsigned int const index, float * const value);
connector_bool_t cc_dev_health_get_gps_location_present(unsigned int const index);

#endif

#endif
