/*
* Copyright (c) 2011, 2012 Digi International Inc.,
* All rights not expressly granted are reserved.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this file,
* You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
* =======================================================================
*/

#include "main.h"
#include "connector_config.h"

#if defined(APPLICATION_FIRMWARE_SERVICE_FULL)
#include "connector_api.h"
#include "platform.h"
#include "connector_debug.h"

#if !defined DEVICE_HAS_FLASH_PART_0 && !defined DEVICE_HAS_FLASH_PART_1
#define	DEVICE_HAS_FLASH_PART_0
#endif
#ifndef FLASH_PART_0_LEN
#define FLASH_PART_0_LEN                          0xFFFFFFFF
#endif
#ifndef FLASH_PART_0_TYPE
#define FLASH_PART_0_TYPE                         ".*\\.S19"
#endif
#ifndef FLASH_PART_0_DESC
#define FLASH_PART_0_DESC                         "SRec Image"
#endif
#ifndef CONNECTOR_FIRMWARE_VERSION
#define CONNECTOR_FIRMWARE_VERSION                          {0,0,0,1}
#endif

typedef struct {
	connector_firmware_version_t    version;
    uint32_t    code_size;
    char        * name_spec;
    char        * description;
} firmware_list_t;

/* list of all supported firmware target info */
static firmware_list_t firmware_list[] = {
    /* version     code_size     name_spec          description */
#ifdef DEVICE_HAS_FLASH_PART_0
    {CONNECTOR_FIRMWARE_VERSION, FLASH_PART_0_LEN, FLASH_PART_0_TYPE, FLASH_PART_0_DESC},
#endif
#ifdef DEVICE_HAS_FLASH_PART_1
    {FLASH_PART_1_VER, FLASH_PART_1_LEN, FLASH_PART_1_TYPE, FLASH_PART_1_DESC}
#endif
};

static uint32_t flash_taskid;

int firmware_download_started = 0;
static size_t total_image_size = 0;
int FlashWriteInProgress = 0;
char *download_buffer = NULL;
uint32_t image_data_length;

static connector_callback_status_t app_firmware_target_count(connector_firmware_count_t * const target_info)
{
    connector_callback_status_t status = connector_callback_continue;

    target_info->count = asizeof(firmware_list);

    return status;
}

static connector_callback_status_t app_firmware_target_info(connector_firmware_info_t * const request_info)
{
    connector_callback_status_t status = connector_callback_continue;
    firmware_list_t * firmware_info;

    ASSERT(request_info->target_number <= asizeof(firmware_list));

    firmware_info = &firmware_list[request_info->target_number];

    request_info->version.major = firmware_info->version.major;
    request_info->version.minor = firmware_info->version.minor;
    request_info->version.revision = firmware_info->version.revision;
    request_info->version.build = firmware_info->version.build;

    request_info->description = firmware_info->description;

    return status;
}

static connector_callback_status_t app_firmware_download_request(connector_firmware_download_start_t * const download_info)
{
    connector_callback_status_t status = connector_callback_continue;

    if (download_info == NULL)
    {
        APP_DEBUG("app_firmware_download_request ERROR: iDigi passes incorrect parameters\n");
        status = connector_callback_abort;
        goto done;
    }
    if (firmware_download_started)
    {
        APP_DEBUG("app_firmware_download_request ERROR: Firmware Download already started! \n");
        goto done;
    }

    /* Create the iDigi Flash Task */
    flash_taskid = ecc_create_task(FLASH_TASK, 0);
    if (flash_taskid <= 0)
    {
        APP_DEBUG("app_firmware_download_request ERROR: Flash task cannot be created\n");
        status = connector_callback_abort;
        goto done;
    }

    /* Create buffer to be used by Flash Task */
    #define FLASH_DOWNLOAD_BUFFER_SIZE 2048
    download_buffer = (char *) ecc_malloc(FLASH_DOWNLOAD_BUFFER_SIZE);
    if (download_buffer == NULL)
    {
        APP_DEBUG("app_firmware_download_request ERROR: Malloc failure creating download buffer\n");
        status = connector_callback_abort;
        goto done;
    }

    APP_DEBUG("target = %d\n",         download_info->target_number);
    APP_DEBUG("filename = %s\n",       download_info->filename);

    total_image_size = 0;
    firmware_download_started = 1;

done:
    return status;
}

static connector_callback_status_t app_firmware_image_data(connector_firmware_download_data_t * const image_data)
{
    connector_callback_status_t status = connector_callback_continue;

    if (image_data == NULL)
    {
        APP_DEBUG("app_firmware_image_data ERROR: iDigi passes incorrect parameters\n");
        status = connector_callback_abort;
        goto done;
    }

    if (!firmware_download_started)
    {
        APP_DEBUG("app_firmware_image_data:no firmware download request started\n");
        status = connector_callback_abort;
        goto done;
    }
    
    if (FlashWriteInProgress)
    {
        status = connector_callback_busy;
        goto done;
    }

    APP_DEBUG("target = %d\n", image_data->target_number);
    APP_DEBUG("offset = 0x%04X\n", image_data->image.offset);
    APP_DEBUG("data = %p\n", image_data->image.data);
    total_image_size += image_data->image.bytes_used;
    APP_DEBUG("length = %zu (total = %zu)\n", image_data->image.bytes_used, total_image_size);
    
    /*
     * Copy the data so it can be used by the Flash Thread
     */
    memcpy(download_buffer, image_data->image.data, image_data->image.bytes_used);
    
    image_data_length = (unsigned long)image_data->image.bytes_used;
    
    FlashWriteInProgress = 1;

done:
    return status;
}

static connector_callback_status_t app_firmware_download_complete(connector_firmware_download_complete_t * const download_complete)
{
    connector_callback_status_t status = connector_callback_continue;
    unsigned long result;

    if (download_complete == NULL)
    {
        APP_DEBUG("app_firmware_download_complete Error: iDigi passes incorrect parameters\n");
        status = connector_callback_abort;
        goto done;
    }
    
    if (FlashWriteInProgress)
    {
        status = connector_callback_busy;
        goto done;
    }

    if (!firmware_download_started)
    {
        APP_DEBUG("app_firmware_download_complete:no firmware download request started\n");
        download_complete->status = connector_firmware_download_not_complete;
        goto done;
    }

    download_complete->status = connector_firmware_status_success;

    firmware_download_started = 0;

    result = ecc_destroy_task(flash_taskid);

    if (result == 0)
    {
        APP_DEBUG("app_firmware_download_complete: target    = %d\n",    download_complete->target_number);
    }
    else
    {               	
        APP_DEBUG("app_firmware_download_complete: _task_destroy(taskid) failed [%d]\n", result);
    }
    
    if (download_buffer)
    	ecc_free(download_buffer);

done:
    return status;
}

static connector_callback_status_t app_firmware_download_abort(connector_firmware_download_abort_t const * const abort_data)
{
    connector_callback_status_t status = connector_callback_continue;
    unsigned long result;

    /* Server is aborting firmware update */
    APP_DEBUG("app_firmware_download_abort\n");
    firmware_download_started = 0;

    if (abort_data == NULL)
    {
        APP_DEBUG("app_firmware_download_abort Error: iDigi passes incorrect parameters\n");
        status = connector_callback_abort;
        goto done;
    }
    
    result = ecc_destroy_task(flash_taskid);

    if (result == 0)
    {
        APP_DEBUG("app_firmware_download_abort: flash task is destroyed\n");
    }
    else
    {               	
        APP_DEBUG("app_firmware_download_abort: _task_destroy(taskid) failed [%d]\n", result);
    }
    
    if (download_buffer)
    	ecc_free(download_buffer);

done:
    return status;
}

static connector_callback_status_t app_firmware_reset(connector_firmware_reset_t const * const reset_data)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(reset_data);
    
    if (FlashWriteInProgress)
    {
        status = connector_callback_busy;
        goto done;
    }
    
    /* Server requests firmware reboot */
    APP_DEBUG("app_firmware_reset\n");
    
    ecc_software_reset(); // Should never return

done:
    return status;
}

connector_callback_status_t app_firmware_handler(connector_request_id_firmware_t const request_id,
                                                  void * const data)
{
    connector_callback_status_t status = connector_callback_unrecognized;

    switch (request_id)
    {
    case connector_request_id_firmware_target_count:
        status = app_firmware_target_count(data);
        break;

    case connector_request_id_firmware_info:
        status = app_firmware_target_info(data);
        break;

    case connector_request_id_firmware_download_start:
        status = app_firmware_download_request(data);
        break;

    case connector_request_id_firmware_download_data:
        status = app_firmware_image_data(data);
        break;

    case connector_request_id_firmware_download_complete:
        status = app_firmware_download_complete(data);
        break;

    case connector_request_id_firmware_download_abort:
        status =  app_firmware_download_abort(data);
        break;

    case connector_request_id_firmware_target_reset:
        status =  app_firmware_reset(data);
        break;

    }

    return status;
}
#endif /* APPLICATION_FIRMWARE_SERVICE_FULL */
