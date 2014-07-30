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

uint32_t cc_dev_health_get_posix_time(void);
connector_bool_t cc_dev_health_get_net_latency_min(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_net_latency_avg(unsigned int const index, int32_t * const value);
connector_bool_t cc_dev_health_get_net_latency_max(unsigned int const index, int32_t * const value);

unsigned int cc_dev_health_get_mobile_instances(void);

connector_bool_t cc_dev_health_get_mobile_module(unsigned int const index, connector_json_t * const value);
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
connector_bool_t cc_dev_health_get_mobile_sim0(unsigned int const index, connector_json_t * const value);
connector_bool_t cc_dev_health_get_mobile_sim0_status(unsigned int const index, char const * * const value);

unsigned int cc_dev_health_get_eth_instances(void);

connector_bool_t cc_dev_health_get_eth_tx_bytes(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_tx_packets(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_rx_bytes(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_rx_packets(unsigned int const index, uint64_t * const value);
connector_bool_t cc_dev_health_get_eth_dropped_packets(unsigned int const index, uint64_t * const value);

unsigned int cc_dev_health_get_wifi_instances(void);

connector_bool_t cc_dev_health_get_wifi_radio(unsigned int const index, connector_json_t * const value);
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

connector_bool_t cc_dev_health_get_gps_location(unsigned int const index, connector_json_t * const value);

#endif

#endif
