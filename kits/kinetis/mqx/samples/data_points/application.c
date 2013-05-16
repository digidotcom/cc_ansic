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
#include <mqx.h>
#include <rtcs.h>
#include <bsp.h>
#include <lwevent.h>
#include <io_gpio.h>
#include "platform.h"
#include "connector.h"

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_DEBUG("idigi_status: status update %d [%s]\n", status, status_message);
}

void *get_connector_handle(void);
void *app_allocate_data_points(size_t const points_count);

connector_bool_t get_connection_status(void);

void fill_data_point(connector_data_point_t * const point)
{
    point->data.type = connector_data_type_native;
    point->data.element.native.int_value = rand() % 100;
    {
        TIME_STRUCT current_time;
        _time_get(&current_time);

        point->time.source = connector_time_local_epoch_fractional;
        point->time.value.since_epoch_fractional.seconds = current_time.SECONDS;
        point->time.value.since_epoch_fractional.milliseconds = 0;
    }

    {
        static char latitude_str[] = "26.78";
        static char longitude_str[] = "60.44";
        static char elevation_str[] = "90";

        point->location.type = connector_location_type_text;
        point->location.value.text.latitude = latitude_str;
        point->location.value.text.longitude = longitude_str;
        point->location.value.text.elevation = elevation_str;
    }

    {
        static char dp_description[] = "Water temperature";

        point->description = dp_description;
    }

    point->quality.type = connector_quality_type_ignore;
    point->next = NULL; /* Pointer to the next connector_data_point_t, NULL for end of list */
}


int application_start(void)
{
   void *handle = NULL;
   int ret;
   connector_data_point_t * const point = _mem_alloc(sizeof *point);
   
   if (SNTP_oneshot(IPADDR(84,77,40,132), 3000) != RTCS_OK) {
		printf("SNTP_oneshot failed!\n");
		printf("Setting time to 1/1/2013\n");
		{
			TIME_STRUCT time;
			time.SECONDS =  1356998400; /* 01/01/2013 00:00:00 */
			time.MILLISECONDS = 0;
			_time_set(&time);
		}
   }
   
   APP_DEBUG("application_start: calling idigi_connector_start\n");
   ret = connector_start(connector_status);
   if (ret != connector_success)
   {
       APP_DEBUG("idigi_connector_start failed [%d]\n", ret);
       goto error;
   }
   
   do {
	    _time_delay(1000);
   } while (get_connection_status() != connector_true);
   
   srand(0x5EBA5);

   for(;;)
   {
		#define WAIT_INTERVAL 5000 /* 5 second */
		connector_status_t status;
		connector_request_data_point_single_t dp_stream;
		
		dp_stream.transport = connector_transport_tcp;
		dp_stream.forward_to = NULL;
		dp_stream.path = "Temperature";
		dp_stream.point = point;
		dp_stream.unit = "Celsius";
		dp_stream.user_context = NULL;
		dp_stream.type = connector_data_point_type_integer;
		dp_stream.response_required = connector_true;
		
		fill_data_point(point);
		
		APP_DEBUG("Sending Data point %d to %s\n", dp_stream.point->data.element.native.int_value, dp_stream.path);
		status = connector_send_data_point(&dp_stream);

		switch (status)
		{
			case connector_init_error:
				APP_DEBUG("connector_send_data_point: returned connector_init_error\n");
				break;
			case connector_service_busy:
				APP_DEBUG("connector_send_data_point: returned connector_service_busy\n");
				break;
			case connector_unavailable:
				APP_DEBUG("connector_send_data_point: returned connector_unavailable\n");
				break;
			case connector_success:
				APP_DEBUG("Data Point uploaded successfully\n");
				break;
			default:
				APP_DEBUG("Failed to send data point! status: %d\n", status);
				goto done;
		}
		_time_delay(WAIT_INTERVAL);
	}

done:

error:
	APP_DEBUG("Data point sample is exited!\n");
	return 1;
}
