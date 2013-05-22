/*
 * Copyright (c) 2012, 2013 Digi International Inc.,
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
#include "platform.h"
#include "connector.h"

#define SAWTOOTH_SIGNAL_MAX 	10
#define SAWTOOTH_SIGNAL_MIN 	-10
#define TIME_BETWEEN_SAMPLES	1000 /* Milliseconds */

static void connector_status(connector_error_t const status, char const * const status_message)
{
    APP_DEBUG("connector_status: status update %d [%s]\n", status, status_message);
}

void fill_data_point(connector_request_data_point_single_t *data_point)
{
	connector_data_point_t *point = data_point->point;
	static int signal_value = SAWTOOTH_SIGNAL_MIN;
	static int increasing = 1;
	
	point->description = "Sawtooth Signal";

	point->location.type =  connector_location_type_text;
	point->location.value.text.latitude = "42.27";
	point->location.value.text.longitude = "2.27";
	point->location.value.text.elevation = "391.00";
	
	point->next = NULL;
	point->quality.type = connector_quality_type_ignore;
	{
		TIME_STRUCT time;
		_time_get(&time);
		point->time.source = connector_time_local_epoch_fractional;
		point->time.value.since_epoch_fractional.seconds = time.SECONDS;
		point->time.value.since_epoch_fractional.milliseconds = time.MILLISECONDS;
	}
	
	point->data.type = connector_data_type_native;
	point->data.element.native.int_value = signal_value;
	
	if (increasing) {
		signal_value++;
		if (signal_value == SAWTOOTH_SIGNAL_MAX)
			increasing = 0;
	} else {
		signal_value--;
		if (signal_value == SAWTOOTH_SIGNAL_MIN)
			increasing = 1;
	}
}

int application_start(void)
{
    connector_error_t ret;
    int status = -1;

    /* Set hour */
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
    
    APP_DEBUG("application_start: calling connector_start\n");
    ret = connector_start(connector_status);
    if (ret != connector_success)
    {
        APP_DEBUG("connector_start failed [%d]\n", ret);
        goto error;
    }

    {
    	connector_request_data_point_single_t *data_point = _mem_alloc(sizeof *data_point);
    	connector_callback_status_t status;
    	
    	data_point->forward_to = NULL;
    	data_point->path = "SawtoothSignal";
    	data_point->response_required = connector_false;
    	data_point->transport = connector_transport_tcp;
    	data_point->type = connector_data_point_type_integer;
    	data_point->unit = "Volts";
    	data_point->user_context = NULL;
    	
    	data_point->point = _mem_alloc(sizeof *data_point->point);
    	
    	for (;;) {
        	fill_data_point(data_point);
        	APP_DEBUG("Sending sample %d at %d\n", data_point->point->data.element.native.int_value, data_point->point->time.value.since_epoch_fractional.seconds);
        	status = connector_send_data_point(data_point);
        	if (status == connector_init_error) {
        		APP_DEBUG("Connector not ready yet\n");
        	}
        	_time_delay(TIME_BETWEEN_SAMPLES);
    	}
    }
    
    status = 0;
error:
    return status;
}
