#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "connector_api.h"

 /*#define DEBUG*/

#if !(defined UNUSED_ARGUMENT)
#define UNUSED_ARGUMENT(a)  (void)(a)
#endif

#if (defined DEBUG)
#define PRINT_FUNCTION_NAME()   do{printf("Function: %s\n", __FUNCTION__);}while(0)
#define PRINT_INDEXES()         do{printf("Mobile %d Net %d\n", indexes->upper, indexes->lower);}while(0)
#define PRINT_TECH()            do{printf("Mobile tech %dG\n", (int)tech + 2);}while(0)
#else
#define PRINT_FUNCTION_NAME()
#define PRINT_INDEXES()         UNUSED_ARGUMENT(indexes)
#define PRINT_TECH()            UNUSED_ARGUMENT(tech)
#endif

#define DEVICE_HEALTH_FILENAME  "dev_health.cfg"

static int get_executable_path(char * const path, unsigned int max_size)
{
    pid_t pid = getpid();
    ssize_t error;
    static char const executable_name[] = "connector";
    char proc_path[PATH_MAX] = {0};

    sprintf(proc_path, "/proc/%d/exe", pid);
    error = readlink(proc_path, path, max_size);

    if (error == -1)
    {
        perror("readlink");
        goto done;
    }
    else
    {
        char * const end_of_path = path + strlen(path) - (sizeof executable_name - 1);
        *end_of_path = '\0';
    }

done:
    return error;
}
connector_callback_status_t cc_dev_health_load_metrics(dev_health_metrics_config_t * const metrics_array, unsigned int array_size)
{
    connector_callback_status_t status = connector_callback_continue;
    char dev_health_path[PATH_MAX] = {0};

    get_executable_path(dev_health_path, sizeof dev_health_path);
    strcat(dev_health_path, DEVICE_HEALTH_FILENAME);

    if (access(dev_health_path, F_OK) != -1)
    {
        FILE *file;
        unsigned int bytes_read;

        file = fopen(dev_health_path, "r");
        bytes_read = fread(metrics_array, sizeof metrics_array[0], array_size, file);
        if (bytes_read == (sizeof metrics_array[0]) * array_size)
        {
            printf("Error while reading %s\n", dev_health_path);
        }
        fclose(file);
    }
    else
    {
        /* File does not exist */
        unsigned int i;

        for (i = 0; i < array_size; i++)
        {
            metrics_array[i].path[0] = '\0';
            metrics_array[i].reporting_interval = 0;
            metrics_array[i].sampling_interval = 0;
        }
    }

    return status;
}

connector_callback_status_t cc_dev_health_save_metrics(dev_health_metrics_config_t const * const metrics_array, unsigned int array_size)
{
    connector_callback_status_t status = connector_callback_continue;
    char dev_health_path[PATH_MAX] = {0};
    FILE *file;
    unsigned int bytes_read;

    get_executable_path(dev_health_path, sizeof dev_health_path);
    strcat(dev_health_path, DEVICE_HEALTH_FILENAME);

    file = fopen(dev_health_path, "w");
    bytes_read = fwrite(metrics_array, sizeof metrics_array[0], array_size, file);
    if (bytes_read == (sizeof metrics_array[0]) * array_size)
    {
        printf("Error while reading %s\n", dev_health_path);
    }
    fclose(file);

    return status;
}


char * cc_dev_health_malloc_string(size_t size)
{
    return malloc(size);
}

void cc_dev_health_free_string(char const * const string)
{
    free((void *)string);
}

uint32_t cc_dev_health_get_posix_time(void)
{
    time_t const posix_time = time(NULL);

    return posix_time;
}

connector_bool_t cc_dev_health_get_net_latency_min(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_net_latency_avg(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_net_latency_max(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_net_transactions_count(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_net_drop_count(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_net_oos_count(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


unsigned int cc_dev_health_get_mobile_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 2;
}

unsigned int cc_dev_health_get_mobile_net_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 2;
}

#define MAX_STRING  64

void cc_dev_health_get_mobile_module_modem_id(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "My \"Modem\" \\ID\\");
}

void cc_dev_health_get_mobile_module_imei(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "123456-789-0");
}

connector_bool_t cc_dev_health_get_mobile_net_cell_id(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "My Cell ID");
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_network(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "AT&T");
    return connector_true;
}

void cc_dev_health_get_mobile_module_sims(connector_indexes_t const * const indexes, unsigned int * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 2;
}

connector_bool_t cc_dev_health_get_mobile_active_sim(connector_indexes_t const * const indexes, unsigned int * const value)
{
    UNUSED_ARGUMENT(indexes);
    PRINT_FUNCTION_NAME();
    *value = 1;
    return connector_true;
}

connector_bool_t cc_dev_health_mobile_module_present(connector_indexes_t const * const indexes)
{
    UNUSED_ARGUMENT(indexes);
    PRINT_FUNCTION_NAME();
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_info_present(connector_indexes_t const * const indexes)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_status(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);

    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    strcpy(*value, "Status");
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_rssi(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_ecio(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_rsrp(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_rsrq(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_sinr(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_snr(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


void cc_dev_health_get_mobile_net_status_registration(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "<Registration>");
}

void cc_dev_health_get_mobile_net_status_attachment(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "<Attachment>");
}


connector_bool_t cc_dev_health_get_mobile_net_lac(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_tac(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_session(connector_indexes_t const * const indexes, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_net_rxbytes(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_txbytes(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_latency_min(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_latency_avg(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_latency_max(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_transactions_count(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_fdrop_count(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_losspercent(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_drop_count(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_oos_count(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_net_uptime(connector_indexes_t const * const indexes, mobile_network_tech_t tech, int32_t * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();
    PRINT_TECH();

    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_mobile_temperature(connector_indexes_t const * const indexes, float * const value)
{
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    *value = 25.3;
    return connector_true;
}

void cc_dev_health_get_mobile_net_info_iccid(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    strcpy(*value, "My ICCID");
}

void cc_dev_health_get_mobile_net_info_imsi(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    strcpy(*value, "My IMSI");
}

void cc_dev_health_get_mobile_net_info_phone_num(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);

    PRINT_FUNCTION_NAME();
    PRINT_INDEXES();

    strcpy(*value, "My Phone Num");
}

unsigned int cc_dev_health_get_eth_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 4;
}

connector_bool_t cc_dev_health_get_eth_tx_bytes(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_eth_tx_packets(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}

connector_bool_t cc_dev_health_get_eth_tx_dropped(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}

connector_bool_t cc_dev_health_get_eth_tx_overruns(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}

connector_bool_t cc_dev_health_get_eth_rx_bytes(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}

connector_bool_t cc_dev_health_get_eth_rx_packets(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}

connector_bool_t cc_dev_health_get_eth_rx_dropped(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}

connector_bool_t cc_dev_health_get_eth_link_down_count(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}

connector_bool_t cc_dev_health_get_eth_link_down_duration(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


unsigned int cc_dev_health_get_wifi_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 1;
}

connector_bool_t cc_dev_health_get_wifi_radio_present(connector_indexes_t const * const indexes)
{
    UNUSED_ARGUMENT(indexes);
    return connector_true;
}

void cc_dev_health_get_wifi_radio_mode(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "client");
}

void cc_dev_health_get_wifi_radio_ssid(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);

    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "SSID");
}

void cc_dev_health_get_wifi_radio_channel(connector_indexes_t const * const indexes, unsigned int * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 3;
}

void cc_dev_health_get_wifi_radio_protocol(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);

    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "Wifi Protocol");
}


connector_bool_t cc_dev_health_get_wifi_status(connector_indexes_t const * const indexes, char * * const value)
{
    *value = cc_dev_health_malloc_string(MAX_STRING);
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    strcpy(*value, "wifi status");
    return connector_true;
}


connector_bool_t cc_dev_health_get_wifi_rssi(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_wifi_rate(connector_indexes_t const * const indexes, float * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 1.159;
    return connector_true;
}


connector_bool_t cc_dev_health_get_wifi_clients(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}



connector_bool_t cc_dev_health_get_system_mem_free(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_mem_used(connector_indexes_t const * const indexes, uint64_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_reboots(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_msg_free(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_msg_min(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_buf_free(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_buf_used(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_bigbuf_free(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_bigbuf_used(connector_indexes_t const * const indexes, int32_t * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_gps_location_present(connector_indexes_t const * const indexes)
{
    UNUSED_ARGUMENT(indexes);
    return connector_true;
}

void cc_dev_health_get_gps_location_latitude(connector_indexes_t const * const indexes, float * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = -26.47;
}

void cc_dev_health_get_gps_location_longitude(connector_indexes_t const * const indexes, float * const value)
{
    UNUSED_ARGUMENT(indexes);

    PRINT_FUNCTION_NAME();
    *value = 60.27;
}
