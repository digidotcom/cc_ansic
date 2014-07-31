#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "connector_api.h"

/* #define DEBUG */

#if !(defined UNUSED_ARGUMENT)
#define UNUSED_ARGUMENT(a)  (void)(a)
#endif

#if (defined DEBUG)
#define PRINT_FUNCTION_NAME()   do{printf("Function: %s\n", __FUNCTION__);}while(0)
#else
#define PRINT_FUNCTION_NAME()
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
connector_callback_status_t cc_dev_health_load_metrics(dev_health_metrics_config_t * const metrics_array, size_t array_size)
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
        size_t i;

        for (i = 0; i < array_size; i++)
        {
            metrics_array[i].path[0] = '\0';
            metrics_array[i].reporting_interval = 0;
            metrics_array[i].sampling_interval = 0;
        }
    }

    return status;
}

connector_callback_status_t cc_dev_health_save_metrics(dev_health_metrics_config_t const * const metrics_array, size_t array_size)
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


uint32_t cc_dev_health_get_posix_time(void)
{
    time_t const posix_time = time(NULL);

    return posix_time;
}

connector_bool_t cc_dev_health_get_net_latency_min(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_net_latency_avg(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_net_latency_max(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


unsigned int cc_dev_health_get_mobile_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 1;
}

connector_bool_t cc_dev_health_get_mobile_module(unsigned int const index, connector_geojson_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "{\"topic\" : \"battery\",\"status\": \"reset\"}";
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_status(unsigned int const index, char const * * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "Status";
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_rssi(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_ecio(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_rsrp(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_rsrq(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_sinr(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_snr(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_registration(unsigned int const index, char const * * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "Registration";
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_lac(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_tac(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_session(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_temperature(unsigned int const index, float * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 25.3;
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_sim0(unsigned int const index, connector_geojson_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "{\"topic\" : \"battery\",\"status\": \"reset\"}";
    return connector_true;
}


connector_bool_t cc_dev_health_get_mobile_sim0_status(unsigned int const index, char const * * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "sim0 status";
    return connector_true;
}

unsigned int cc_dev_health_get_eth_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 4;
}

connector_bool_t cc_dev_health_get_eth_tx_bytes(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_eth_tx_packets(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_eth_rx_bytes(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_eth_rx_packets(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_eth_dropped_packets(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


unsigned int cc_dev_health_get_wifi_instances(void)
{
    PRINT_FUNCTION_NAME();

    return 1;
}

connector_bool_t cc_dev_health_get_wifi_radio(unsigned int const index, connector_geojson_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "{\"topic\" : \"battery\",\"status\": \"reset\"}";
    return connector_true;
}


connector_bool_t cc_dev_health_get_wifi_status(unsigned int const index, char const * * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "wifi status";
    return connector_true;
}


connector_bool_t cc_dev_health_get_wifi_rssi(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_wifi_rate(unsigned int const index, float * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 1.159;
    return connector_true;
}


connector_bool_t cc_dev_health_get_wifi_clients(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}



connector_bool_t cc_dev_health_get_system_mem_free(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_mem_used(unsigned int const index, uint64_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 64;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_reboots(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_msg_free(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_msg_min(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_buf_free(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_buf_used(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_bigbuf_free(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}


connector_bool_t cc_dev_health_get_system_bigbuf_used(unsigned int const index, int32_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = 32;
    return connector_true;
}

connector_bool_t cc_dev_health_get_gps_location(unsigned int const index, connector_geojson_t * const value)
{
    UNUSED_ARGUMENT(index);

    PRINT_FUNCTION_NAME();
    *value = "{\"type\": \"Point\",\"coordinates\": [-102.3046875, 36.1733569352216]}";
    return connector_true;
}


