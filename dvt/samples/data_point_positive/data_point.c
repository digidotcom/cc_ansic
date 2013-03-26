/*
 * Copyright (c) 2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
#include <time.h>
#include <sys/time.h>
#include "connector_config.h"
#include "connector_api.h"
#include "platform.h"

static void free_dp_ptr(connector_data_point_request_t * const dp_ptr)
{
    if (dp_ptr == NULL) goto done;

    if (dp_ptr->data_point_path != NULL)
    {
        APP_DEBUG("free_dp_ptr: Freeing %s\n", dp_ptr->data_point_path);
        free(dp_ptr->data_point_path);
        dp_ptr->data_point_path = NULL;
    }

    if (dp_ptr->dp_type == connector_data_point_type_binary)
    {
        if (dp_ptr->content.binary.point != NULL)
        {
            free(dp_ptr->content.binary.point);
            dp_ptr->content.binary.point = NULL;
        }
    }
    else
    {
        if (dp_ptr->content.advanced.points != NULL)
        {
            free(dp_ptr->content.advanced.points);
            dp_ptr->content.advanced.points = NULL;
        }
    }

    free(dp_ptr);

done:
    return;
}

static connector_data_point_t * get_advanced_points(connector_data_point_type_t const type)
{
    #define APP_MAX_POINTS  6
    size_t const num_of_points = (rand() % APP_MAX_POINTS) + 1;
    size_t const bytes_needed = (num_of_points * sizeof(connector_data_point_t));
    connector_data_point_t * data_points = malloc(bytes_needed);
    connector_data_point_t * point = data_points;
    struct timeval first_reading_time;
    size_t test_case = 0;

    if (point == NULL)
    {
        APP_DEBUG("get_advanced_points: Failed to malloc data_points [%zu bytes]\n", bytes_needed);
        goto error;
    }

    do
    {
        switch (type)
        {
            case connector_data_point_type_integer:
                if (test_case == 1)
                {
                    static char value[sizeof(int) * 3];

                    point->data.type = connector_data_type_ascii;
                    snprintf(value, sizeof value, "%d", rand());
                    point->data.element.ascii = value;
                }
                else
                {
                    point->data.type = connector_data_type_binary;
                    point->data.element.int_value = rand();
                }
                break;

            case connector_data_point_type_long:
                if (test_case == 1)
                {
                    static char value[sizeof(long) * 3];

                    point->data.type = connector_data_type_ascii;
                    snprintf(value, sizeof value, "%ld", lrand48());
                    point->data.element.ascii = value;
                }
                else
                {
                    point->data.type = connector_data_type_binary;
                    point->data.element.long_value = lrand48();
                }
                break;

            case connector_data_point_type_float:
                #if (defined FLOATING_POINT_SUPPORTED)
                if (test_case == 1)
                {
                    point->data.type = connector_data_type_binary;
                    point->data.element.float_value = (float)drand48();
                }
                else
                #endif
                {
                    static char value[APP_MAX_POINTS][sizeof(float) * 2];

                    point->data.type = connector_data_type_ascii;
                    snprintf(value[test_case], sizeof value[test_case], "%f", (float)drand48()); /* this also requires floating point, works for DVT */
                    point->data.element.ascii = value[test_case];
                }
                break;

            case connector_data_point_type_double:
                #if (defined FLOATING_POINT_SUPPORTED)
                if (test_case == 1)
                {
                    point->data.type = connector_data_type_binary;
                    point->data.element.double_value = drand48();
                }
                else
                #endif
                {
                    static char value[APP_MAX_POINTS][sizeof(double) * 2];

                    point->data.type = connector_data_type_ascii;
                    snprintf(value[test_case], sizeof value[test_case], "%lf", drand48()); /* this also requires floating point, works for DVT */
                    point->data.element.ascii = value[test_case];
                }
                break;

            case connector_data_point_type_string:
            {
                static char * value[APP_MAX_POINTS] = {"Hello World!", "", "?", "\"Hi\"", "\nLine Feed\n", "Line\twith\ttabs"};

                point->data.type = connector_data_type_ascii;
                point->data.element.string_value = value[test_case];
                break;
            }

            default:
                APP_DEBUG("get_advanced_points: Unsupported type [%d]\n", type);
                goto error;
        }

        /* update time */
        if (test_case > 0)
        {
            size_t const time_case = (rand() % (connector_time_relative + 1));
            struct timeval current_time;
            int const status = gettimeofday(&current_time, NULL);

            ASSERT(status == 0);
            switch (time_case)
            {
                case connector_time_local_epoch:
                    point->time.source = connector_time_local_epoch;
                    point->time.value.seconds_since_epoch = current_time.tv_sec;
                    break;

                case connector_time_local_iso8601:
                {
                    #define MAX_ISO_8601_BYTES  20
                    static char value[connector_data_point_type_binary][APP_MAX_POINTS][MAX_ISO_8601_BYTES];
                    struct tm * const tmval = localtime(&current_time.tv_sec);

                    ASSERT(tmval != NULL);
                    snprintf(value[type][test_case-1], sizeof value[type][test_case-1], "%04d-%02d-%02d %02d:%02d:%02d",
                             tmval->tm_year+1900, tmval->tm_mon+1, tmval->tm_mday, tmval->tm_hour, tmval->tm_min, tmval->tm_sec);
                    point->time.value.iso8601_string = value[type][test_case-1];
                    break;
                }

                case connector_time_relative:
                {
                    time_t delta_sec = current_time.tv_sec - first_reading_time.tv_sec;
                    uint32_t delta_usec;

                    if (current_time.tv_usec < first_reading_time.tv_usec)
                    {
                        uint32_t const microseconds_to_seconds = 1000000;

                        ASSERT(delta_sec > 0);
                        delta_usec = (current_time.tv_usec + microseconds_to_seconds) - first_reading_time.tv_usec;
                        delta_sec--;
                    }
                    else
                    {
                        delta_usec = current_time.tv_usec - first_reading_time.tv_usec;
                    }

                    {
                        uint32_t const msec = (delta_sec * 1000) + (delta_usec / 1000);

                        point->time.source = connector_time_relative;
                        point->time.value.relative_msec = msec;
                    }

                    break;
                }

                default:
                    point->time.source = connector_time_server;
                    break;
            }
        }
        else
        {
            int const status = gettimeofday(&first_reading_time, NULL);

            ASSERT(status == 0);
            point->time.source = connector_time_local_epoch;
            point->time.value.seconds_since_epoch = first_reading_time.tv_sec;
        }

        /* update location */
        {
            size_t const location_case = (rand() % (connector_location_type_ascii + 1));

            switch(location_case)
            {
                case connector_location_type_ascii:
                {
                    static char latitude_str[] = "42.32";
                    static char longitude_str[] = "71.23";
                    static char elevation_str[] = "63.95";

                    point->location.type = connector_location_type_ascii;
                    point->location.value.ascii.latitude = latitude_str;
                    point->location.value.ascii.longitude = longitude_str;
                    point->location.value.ascii.elevation = elevation_str;
                    break;
                }

                #if (defined FLOATING_POINT_SUPPORTED)
                case connector_location_type_float:
                    point->location.type = connector_location_type_float;
                    point->location.value.binary.latitude = 42.22;
                    point->location.value.binary.longitude = 71.14;
                    point->location.value.binary.elevation = 60.67;
                    break;
                #endif

                default:
                    point->location.type = connector_location_type_ignore;
                    break;
            }
        }

        if (!(rand() % 3))
        {
            static char dp_description[] = "iC DVT data point";

            point->description = dp_description;
        }
        else
            point->description = NULL;

        {
            int const quality_value = rand();

            point->quality = ((quality_value % 3) == 0) ? quality_value : 0;
        }

        test_case++;
        if (test_case < num_of_points)
        {
            point->next = point + 1;
            point++;
        }
        else
        {
            point->next = NULL;
            goto done;
        }

    } while (test_case < num_of_points);

error:
    if (data_points != NULL)
    {
        free(data_points);
        data_points = NULL;
    }

done:
    return data_points;
}

static void * get_binary_point(size_t * bytes)
{
    size_t const rand_bytes = (rand() % 16384) + 1;
    unsigned char * data_ptr = malloc(rand_bytes);

    if (data_ptr != NULL)
    {
        size_t i;

        for (i = 0; i < rand_bytes; i++)
            data_ptr[i] = rand() % UCHAR_MAX;

        *bytes = rand_bytes;
    }
    else
    {
        APP_DEBUG("get_binary_point: Failed to malloc binary point %zu bytes\n", rand_bytes);
    }

    return data_ptr;
}

static char * get_path_name(connector_transport_t const transport, connector_data_point_type_t const type)
{
    size_t const path_max_size = sizeof("_dp_tcp_double_");
    static char * app_transport[] = {"tcp", "udp", "sms"};
    static char * app_type[] = {"int", "long", "float", "double", "string", "binary"};
    char * path_name = malloc(path_max_size);

    ASSERT(transport < asizeof(app_transport));
    ASSERT(type < asizeof(app_type));

    if (path_name == NULL)
    {
        APP_DEBUG("get_path_name: Failed to malloc path_name [%zu]\n", path_max_size);
        goto error;
    }

    snprintf(path_name, path_max_size, "dp_%s_%s", app_transport[transport], app_type[type]);

error:
    return path_name;
}

typedef struct
{
    connector_data_point_request_t * dp_ptr;
    connector_data_point_type_t data_type;
    size_t request_count;
    size_t response_count;
    connector_transport_t transport;
} dvt_dp_t;

static dvt_dp_t dvt_dp_info[] = {
    #if (defined CONNECTOR_TRANSPORT_TCP)
    {NULL, connector_data_point_type_integer, 0, 0, connector_transport_tcp},
    #endif
    #if (defined CONNECTOR_TRANSPORT_UDP)
    {NULL, connector_data_point_type_integer, 0, 0, connector_transport_udp},
    #endif
    #if (defined CONNECTOR_TRANSPORT_SMS)
    {NULL, connector_data_point_type_integer, 0, 0, connector_transport_sms}
    #endif
};

static size_t const dvt_dp_transport_count = asizeof(dvt_dp_info);

static connector_data_point_request_t * get_data_point(dvt_dp_t * dvt_ptr)
{
    connector_data_point_request_t * dp_ptr = malloc(sizeof(connector_data_point_request_t));

    if (dp_ptr == NULL)
    {
        APP_DEBUG("app_send_data_point: Failed to malloc dp_ptr\n");
        goto error;
    }

    dp_ptr->data_point_path = NULL;
    dp_ptr->transport = dvt_ptr->transport;
    dp_ptr->dp_type = dvt_ptr->data_type;

    if (dp_ptr->dp_type == connector_data_point_type_binary)
    {
        dp_ptr->content.binary.point = get_binary_point(&dp_ptr->content.binary.bytes);
        if (dp_ptr->content.binary.point == NULL) goto error;
    }
    else
    {
        static char * units[] = {"mph", "lb", NULL, "\'F", ""};

        dp_ptr->content.advanced.points = get_advanced_points(dp_ptr->dp_type);
        if (dp_ptr->content.advanced.points == NULL) goto error;
        dp_ptr->content.advanced.unit = units[dp_ptr->dp_type];
        /* TODO: Add appropriate forward to */
        dp_ptr->content.advanced.forward_to = NULL;
    }

    dp_ptr->data_point_path = get_path_name(dp_ptr->transport, dp_ptr->dp_type);
    if (dp_ptr->data_point_path != NULL) goto error;

    if (dp_ptr->transport != connector_transport_tcp)
    {
        static unsigned int current_flag = CONNECTOR_DATA_RESPONSE_NOT_NEEDED;

        current_flag ^= CONNECTOR_DATA_RESPONSE_NOT_NEEDED;
        dp_ptr->flags = current_flag;
    }
    else
        dp_ptr->flags = 0;

    dp_ptr->user_context = dp_ptr;
    goto done;

error:
    free_dp_ptr(dp_ptr);
    dp_ptr = NULL;

done:
    return dp_ptr;
}

connector_status_t app_send_data_point(connector_handle_t const handle)
{
    connector_status_t status = connector_idle;
    static size_t current_transport = 0;
    size_t transport = current_transport;
    dvt_dp_t * dvt_ptr = &dvt_dp_info[transport];

    do
    {
        transport++;
        if (transport == dvt_dp_transport_count) transport = 0;

        if (dvt_ptr->request_count < connector_data_point_type_binary)
        {
            if (dvt_ptr->dp_ptr == NULL)
                dvt_ptr->dp_ptr = get_data_point(dvt_ptr);

            if (dvt_ptr->dp_ptr != NULL) break;
        }

        dvt_ptr = &dvt_dp_info[transport];

    } while (transport != current_transport);

    current_transport = transport;
    if (dvt_ptr->request_count == connector_data_point_type_binary)
        goto done;

    status = connector_initiate_action(handle, connector_initiate_data_point, dvt_ptr->dp_ptr);
    switch (status)
    {
        case connector_success:
            dvt_ptr->request_count++;
            dvt_ptr->dp_ptr = NULL; /* allow next data point message */
            break;

        case connector_service_busy:
        case connector_init_error:
        case connector_unavailable:
            break;  /* try to send again later */

        default:
            free_dp_ptr(dvt_ptr->dp_ptr);
            dvt_ptr->dp_ptr = NULL;
            break;
    }

done:
    return status;
}

connector_callback_status_t app_data_service_handler(connector_data_service_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);
    UNUSED_ARGUMENT(response_data);

    if (request_data == NULL)
    {
        APP_DEBUG("Error: Empty request_data in data point response!");
        status = connector_callback_error;
        goto error;
    }

    switch (request)
    {
        case connector_data_service_dp_response:
        {
            connector_message_status_response_t const * const dp_response = request_data;
            connector_data_point_request_t * const dp_ptr = (connector_data_point_request_t *)dp_response->user_context;

            if (dp_ptr == NULL)
            {
                APP_DEBUG("Error: Received null data point context\n");
                status = connector_callback_error;
                goto error;
            }

            APP_DEBUG("Received response %d for %s\n", dp_response->status, dp_ptr->data_point_path);
            free_dp_ptr(dp_ptr);
            break;
        }

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            status = connector_callback_unrecognized;
            break;
    }

error:
    return status;
}


connector_callback_status_t app_sm_handler(connector_sm_request_t const request,
                                       void const * const request_data, size_t const request_length,
                                       void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_data);
    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {
        case connector_sm_opaque_response:
        {
            connector_sm_opaque_response_t * const response = response_data;
    
            APP_DEBUG("Received %zu opaque bytes on id %d\n", response->bytes, response->id);
            break;
        }

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}

