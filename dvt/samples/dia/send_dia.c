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
#include "connector_api.h"
#include "platform.h"
#include "application.h"

#define DIA_CHANNEL_CNT 2

typedef struct
{
    connector_message_dia_channel_t channel_info[DIA_CHANNEL_CNT];
    app_bool_t initialized;

} app_dia_data_t;

static app_bool_t app_session_complete = app_true;


#ifdef APP_USE_SM_UDP
connector_status_t app_send_ping(connector_handle_t handle)
{
    static connector_message_status_request_t request; /* idigi connector will hold this until reply received or send completes */
    connector_status_t status;

    request.transport = connector_transport_udp;
    request.user_context = &request;
    request.flags = 0;
    status = connector_initiate_action(handle, connector_initiate_status_message, &request, NULL);
    APP_DEBUG("Sent ping [%d].\n", status);

    return status;
}
#endif


static void app_init_dia_data(app_dia_data_t * const dia_data)
{
    connector_message_dia_channel_t * channel = &dia_data->channel_info[0];

    channel->name = "sensor0.noise_level";
    channel->unit = "dB";
    channel->type = connector_dia_data_type_unsigned_int;
    channel->content_type = connector_dia_content_type_binary;
    channel->time.source = connector_time_local;

    channel++;

    channel->name = "sensor1.temperature";
    channel->unit = "F";
    channel->type = connector_dia_data_type_signed_int;
    channel->content_type = connector_dia_content_type_binary;
    channel->time.source = connector_time_local;

     dia_data->initialized = app_true;

}

static void app_get_noise_level(unsigned int * noise)
{
    *noise = 10 + rand()%75;
}

static void app_get_temperature(int * temperature)
{
    if (*temperature == 0)
        *temperature = 40;

    if (*temperature < 70)
        *temperature += rand()%2;
    else 
        *temperature = 68 + rand()%2;
}

static void app_fill_time(connector_message_dia_channel_t * const channel)
{
     time_t uptime = time(NULL);

#if defined APP_USE_SM_TCP

    static char timestamp[sizeof "yyyy-mm-dd hh:mm:ss"];
    struct tm *tmval = gmtime(&uptime);

    sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d", tmval->tm_year+1900, tmval->tm_mon+1, tmval->tm_mday,
                                                        tmval->tm_hour, tmval->tm_min, tmval->tm_sec);
    APP_DEBUG("Time[UTC]: %s\n", timestamp);

  
    channel->time.value.iso8601_string = timestamp;
#else
    channel->time.value.seconds_since_epoch = uptime;
#endif
}

connector_status_t app_send_dia_data(connector_handle_t handle)
{
    connector_status_t status;
    static connector_message_dia_request_t dia_request; /* idigi connector will hold this until we get a response/error callback */
    static app_dia_data_t dia_data;
    connector_message_dia_channel_t * channel;

    static unsigned int noise;
    static int temperature;

    if (!dia_data.initialized)
        app_init_dia_data(&dia_data);

    if (!app_session_complete)
    {
#if APP_TRANSPORT == connector_transport_tcp

       APP_DEBUG("Previous dia session pending\n");
       status = connector_service_busy;
#else
       static connector_message_status_request_t request;

       request.transport = APP_TRANSPORT;
       request.flags = 0;
       request.user_context = &dia_data;

       APP_DEBUG("Previous dia session pending, cancel it\n");
       status = connector_initiate_action(handle, connector_initiate_session_cancel, &request, NULL);
       if (status == connector_success) 
           status = connector_service_busy;
       else
           APP_DEBUG("connector_initiate_session_cancel returned %d\n", status);
#endif
       goto done;
    }

    app_get_noise_level(&noise);
    app_get_temperature(&temperature);

    channel = &dia_data.channel_info[0];

    dia_request.channels = channel;
    dia_request.transport = APP_TRANSPORT;
    dia_request.flags = 0;
    dia_request.user_context = &dia_data;

    channel->data = &noise;
    channel->bytes = sizeof noise;
    app_fill_time(channel);
    channel->next = &dia_data.channel_info[1];

    channel++;

    channel->data = &temperature;
    channel->bytes = sizeof temperature;

    app_fill_time(channel);
    channel->next = NULL;

    app_session_complete = app_false;

    status = connector_initiate_action(handle, connector_initiate_dia_message, &dia_request, NULL);
    APP_DEBUG("Sent dia data %s %u, %s %d, status %d\n", 
        dia_data.channel_info[0].name, noise, dia_data.channel_info[1].name, temperature, status);

    if (status != connector_success) 
        app_session_complete = app_true;
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


    switch (request)
    {
        case connector_data_service_dia_response:
        {
            connector_message_status_response_t const * const dia_response = request_data;
            app_dia_data_t const * const dia_data = dia_response->user_context;

           app_session_complete = app_true;
            APP_DEBUG("Received response %d for %s, %s\n", dia_response->status, dia_data->channel_info[0].name, dia_data->channel_info[1].name);
            break;
        }

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}


connector_callback_status_t app_sm_handler(connector_sm_request_t const request,
                                       void const * const request_data, size_t const request_length,
                                       void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_data);
    UNUSED_ARGUMENT(response_data);
    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {

    case connector_sm_session_complete:
            app_session_complete = app_true;
            APP_DEBUG("connector_sm_send_complete\n");
            break;

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

connector_callback_status_t app_status_handler(connector_status_request_t const request,
                                           void const * const request_data, size_t const request_length,
                                           void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    switch (request)
    {
        case connector_status_ping_response:
        {
            connector_message_status_response_t const * const status_response = request_data;

            APP_DEBUG("Received ping response [%d].\n", status_response->status);
            break;
        }

        case connector_status_ping_request:
        {
            connector_status_t * const status = response_data;

            APP_DEBUG("Received ping request.\n");
            *status = connector_success;
            break;
        }

        default:
            APP_DEBUG("Request not supported in this sample: %d\n", request);
            status = connector_callback_unrecognized;
            break;
    }

    return status;
}
