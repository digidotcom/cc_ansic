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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "connector_api.h"
#include "platform.h"

typedef struct {
    uint32_t    version;
    uint32_t    code_size;
    char        * name_spec;
    char        * description;
} firmware_list_t;

/* list of all supported firmware target info */
static firmware_list_t firmware_list[] = {
    /* version     code_size     name_spec          description */
    {0x01000000, -1, ".*\\.a",            "Library Image"}, /* any *.a files */
    {0x00000100, -1, ".*\\.[bB][iI][nN]", "Binary Image" }  /* any *.bin files */
};
static uint16_t firmware_list_count = asizeof(firmware_list);

static unsigned long dvt_timing_in_seconds = 0;

static int firmware_download_started = 0;
static size_t total_image_size = 0;

int firmware_timing_expired(unsigned long expired_timing_in_seconds)
{
    int timingExpired = 1;
    unsigned long current_time;

    if (dvt_timing_in_seconds == 0)
    {
        app_os_get_system_time(&dvt_timing_in_seconds);
    }

    app_os_get_system_time(&current_time);


    if ((current_time - dvt_timing_in_seconds) < expired_timing_in_seconds)
    {
        /* not expired */
        timingExpired = 0;
    }
    else
    {
        /* reset timing */
        dvt_timing_in_seconds = 0;
    }

    return timingExpired;
}

static connector_callback_status_t app_firmware_download_request(connector_fw_download_request_t const * const download_info, connector_fw_status_t * download_status)
{
    connector_callback_status_t status = connector_callback_continue;

    if ((download_info == NULL) || (download_status == NULL))
    {
        APP_DEBUG("firmware_download_request ERROR: iDigi passes incorrect parameters\n");
        status = connector_callback_abort;
        goto done;
    }
    if (firmware_download_started)
    {
        *download_status = connector_fw_device_error;
        goto done;
    }

    APP_DEBUG("target = %d\n",         download_info->target);
    APP_DEBUG("filename = %s\n",       download_info->filename);

    total_image_size = 0;
    firmware_download_started = 1;

    *download_status = connector_fw_success;

done:
    return status;
}

static connector_callback_status_t app_firmware_image_data(connector_fw_image_data_t const * const image_data, connector_fw_status_t * data_status)
{
    connector_callback_status_t status = connector_callback_continue;

    if (image_data == NULL)
    {
        APP_DEBUG("firmware_image_data: invalid parameter\n");
        status = connector_callback_abort;
        goto done;
    }

    APP_DEBUG("target = %d\n", image_data->target);
    APP_DEBUG("offset = 0x%04X\n", image_data->offset);
    APP_DEBUG("data = %p\n", image_data->data);
    total_image_size += image_data->length;
    APP_DEBUG("length = %zu (total = %zu)\n", image_data->length, total_image_size);


    *data_status = connector_fw_success;
done:
    return status;
}

static connector_callback_status_t app_firmware_download_complete(connector_fw_download_complete_request_t const * const complete_request, connector_fw_download_complete_response_t * complete_response)
{
    connector_callback_status_t status = connector_callback_continue;

    if ((complete_request == NULL) || (complete_response == NULL))
    {
        APP_DEBUG("firmware_download_complete Error: iDigi passes incorrect parameters\n");
        status = connector_callback_abort;
        goto done;
    }

    if (firmware_timing_expired(90) == 0)
    {
        status = connector_callback_busy;
        goto done;
    }


    APP_DEBUG("target    = %d\n",    complete_request->target);
    APP_DEBUG("code size = %u\n",    complete_request->code_size);
    APP_DEBUG("checksum  = 0x%x\n", (unsigned)complete_request->checksum);

    complete_response->status = connector_fw_download_success;
    /* use the same version since we are not really updating the code */
    complete_response->version = firmware_list[complete_request->target].version;

    if (complete_request->code_size != total_image_size)
    {
        APP_DEBUG("firmware_download_complete: actual image size (%u) != the code size received (%zu)\n",
                      complete_request->code_size, total_image_size);
    }

    firmware_download_started = 0;

done:
    return status;
}

static connector_callback_status_t app_firmware_download_abort(connector_fw_download_abort_t const * const abort_data)
{
    connector_callback_status_t   status = connector_callback_continue;

    /* Server is aborting firmware update */
    APP_DEBUG("firmware_download_abort\n");
    if (abort_data == NULL)
    {
        APP_DEBUG("firmware_download_abort Error: iDigi passes incorrect parameters\n");
        status = connector_callback_abort;
        goto done;
    }

done:
    return status;
}

static connector_callback_status_t app_firmware_reset(connector_fw_config_t const * const reset_data)
{
    extern connector_handle_t connector_handle;
    extern pthread_t application_thread;
    extern unsigned int put_file_active_count;

    connector_callback_status_t status = connector_callback_busy;


    UNUSED_ARGUMENT(reset_data);
    APP_DEBUG("firmware_reset\n");

    if (put_file_active_count > 0)
    {
        /* let's terminate iDigi Connector and free all memory used in iDigi Connector.
         *
         */
        connector_initiate_action(connector_handle, connector_initiate_terminate, NULL);
        pthread_cancel(application_thread);
        status = connector_callback_continue;
    }

    return status;
}

connector_callback_status_t app_firmware_handler(connector_firmware_request_t const request,
                                                  void const * const request_data, size_t const request_length,
                                                  void * response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;
    connector_fw_config_t * config = (connector_fw_config_t *)request_data;

    UNUSED_ARGUMENT(request_length);

    switch (request)
    {
    case connector_firmware_target_count:
    {
        uint16_t * count = response_data;
        /* return total number of firmware update targets */
        *count = firmware_list_count;
        break;
    }
    case connector_firmware_version:
    {
        uint32_t * version = response_data;
        /* return the target version number */
        *version = firmware_list[config->target].version;
        break;
    }
    case connector_firmware_code_size:
    {
        /* Return the target code size */
        uint32_t * code_size = response_data;
        *code_size = firmware_list[config->target].code_size;
        break;
    }
    case connector_firmware_description:
    {
        /* return pointer to firmware target description */
        char ** description = response_data;
        *description = firmware_list[config->target].description;
        *response_length = strlen(firmware_list[config->target].description);
       break;
    }
    case connector_firmware_name_spec:
    {
        /* return pointer to firmware target description */
        char ** name_spec = response_data;
        *name_spec = firmware_list[config->target].name_spec;
        *response_length = strlen(firmware_list[config->target].name_spec);
        break;
    }
    case connector_firmware_download_request:
        status = app_firmware_download_request(request_data, response_data);
        break;

    case connector_firmware_binary_block:
        status = app_firmware_image_data(request_data, response_data);
        break;

    case connector_firmware_download_complete:
        status = app_firmware_download_complete(request_data, response_data);
        break;

    case connector_firmware_download_abort:
        status =  app_firmware_download_abort(request_data);
        break;

    case connector_firmware_target_reset:
        status =  app_firmware_reset(request_data);
        break;

    }

    return status;
}
