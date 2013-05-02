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

int application_start(void)
{
   size_t const points_per_message = 5;
   void * const points = app_allocate_data_points(points_per_message);
   size_t points_sent = 0;
   size_t busy_count = 0;
   void *handle = NULL;
   int ret;
   
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
   
   handle = get_connector_handle();
   
   if (points == NULL)  goto error;

   for(;;)
   {
		int const point_interval_in_seconds = 5000;
		size_t const current_index = points_sent % points_per_message;
		connector_status_t const status = app_send_data_point(handle, points, current_index);

		switch (status)
		{
			case connector_init_error:
			case connector_service_busy:
			case connector_unavailable:
			{
				int const point_delay_in_seconds = 10000;

				if (++busy_count > points_per_message) goto done;
				APP_DEBUG(".");
				_time_delay(point_delay_in_seconds);
				break;
			}

			case connector_success:
				points_sent++;
				busy_count = 0;
				break;

			default:
				APP_DEBUG("Failed to send data point:%" PRIsize ", status:%d\n", points_sent, status);
				goto done;
		}

		_time_delay(point_interval_in_seconds);
	}

done:
	app_free_data_points(points);

error:
	APP_DEBUG("Data point sample is exited!\n");
	return 1;
}
