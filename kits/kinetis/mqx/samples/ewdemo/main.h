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
#ifndef __main_h_
#define __main_h_
#include <mqx.h>
#include <bsp.h>

#include <rtcs.h>
#ifdef APPLICATION_HAS_SHELL
#include <sh_rtcs.h>
#endif
#include <ipcfg.h>

#define MAIN_TASK 1
#define CONNECTOR_TASK 2
#define CONNECTOR_FLASH_TASK 3
#define ADC_TASK 4
#define ACCEL_TASK 5
#define CONNECTOR_LED_TASK 6
#define CONNECTOR_TOUCH_TASK 7
#define CONNECTOR_BUTTON_TASK 8
#define CONNECTOR_UTILITY_1 9
#define CONNECTOR_UTILITY_2 10
#define CONNECTOR_APP_TASK 11
#define CONNECTOR_GPIO_TASK 12
#define CONNECTOR_CPU_USAGE_TASK 13

#define ENET_DEVICE 0

void adc_Task();
void accel_Task(uint_32 data);
void connector_led_task(unsigned long initial_data);
void connector_touch_pad_task(unsigned long initial_data);
void connector_button_task(unsigned long initial_data);
void connector_utility_task1(unsigned long initial_data);
void connector_utility_task2(unsigned long initial_data);
void connector_app_run_task(unsigned long initial_data);
void connector_gpio_pulse_task(unsigned long initial_data);
void connector_cpu_usage(unsigned long initial_data);
void main_task(uint_32 initial_data);

#endif /* __main_h_ */
