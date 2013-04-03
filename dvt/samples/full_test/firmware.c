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
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "connector_api.h"
#include "platform.h"
#include "connector_dvt.h"

#define DVT_FW_UNKNOWN_FILE_SIZE   ((uint32_t)-1)

static dvt_data_t dvt_data_list[dvt_case_last] =
{
    {dvt_case_fw_download_denied,       {0x15,00,00,00}, dvt_state_init, "Download Denied",  ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_invalid_size,          {0x16,00,00,00}, dvt_state_init, "Download Invalid Size",     ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_invalid_version,       {0x17,00,00,00}, dvt_state_init, "Download Invalid Version",  ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_unauthenticated,       {0x18,00,00,00}, dvt_state_init, "Download Unauthenticated",  ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_not_allowed,           {0x19,00,00,00}, dvt_state_init, "Download Not Allowed",      ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_configured_to_reject,  {0x1A,00,00,00}, dvt_state_init, "Download Configured to Reject", ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_encountered_error,     {0x1B,00,00,00}, dvt_state_init, "Encountered Error", ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_user_abort,            {0x1E,00,00,00}, dvt_state_init, "User Abort",       ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_device_error,          {0x1F,00,00,00}, dvt_state_init, "Device Error",     ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_invalid_offset,        {0x20,00,00,00}, dvt_state_init, "Invalid Offset",   ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_invalid_data,          {0x21,00,00,00}, dvt_state_init, "Invalid Data",     ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE},
    {dvt_case_fw_hardware_error,        {0x22,00,00,00}, dvt_state_init, "Hardware Error",   ".*\\.[Ee][Xx][Nn]", "firmware.bin", DVT_FW_UNKNOWN_FILE_SIZE}
};

static dvt_data_t * dvt_current_ptr = NULL;

static connector_callback_status_t app_firmware_target_count(connector_firmware_count_t * const target_info)
{
    connector_callback_status_t status = connector_callback_continue;

    target_info->count = dvt_case_last;

    return status;
}

static connector_callback_status_t app_firmware_target_info(connector_firmware_info_t * const request_info)
{
    connector_callback_status_t status = connector_callback_continue;
    dvt_data_t * firmware_info;

    ASSERT(request_info->target_number < dvt_case_last);

    firmware_info = &dvt_data_list[request_info->target_number];

    request_info->version.major = firmware_info->version.major;
    request_info->version.minor = firmware_info->version.minor;
    request_info->version.revision = firmware_info->version.revision;
    request_info->version.build = firmware_info->version.build;

    request_info->description = firmware_info->description;
    request_info->filespec = firmware_info->filespec;

    return status;
}

static connector_callback_status_t app_firmware_download_request(connector_firmware_download_start_t * const download_info)
{
    connector_callback_status_t status = connector_callback_continue;

    if (download_info == NULL)
    {
        APP_DEBUG("firmware_download_request ERROR: Invalid parameters\n");
        status = connector_callback_abort;
        goto error;
    }

    if (dvt_current_ptr != NULL)
    {
        APP_DEBUG("firmware_download_request ERROR: In progress target : %d\n", dvt_current_ptr->target);
        download_info->status = connector_firmware_status_device_error;
        goto error;
    }

    APP_DEBUG("target = %d\n", download_info->target_number);
    APP_DEBUG("filename = %s\n", download_info->filename);
    if (download_info->target_number > dvt_case_last)
    {
        APP_DEBUG("firmware_download_request ERROR: In progress target : %d\n", dvt_current_ptr->target);
        download_info->status = connector_firmware_status_user_abort;
        goto error;
    }

    dvt_current_ptr = &dvt_data_list[download_info->target_number];
    if (dvt_current_ptr->target != download_info->target_number)
    {
        download_info->status = connector_firmware_status_device_error;
        goto error;
    }

    dvt_current_ptr->state = dvt_state_init;
    dvt_current_ptr->file_size = 0;

    switch (download_info->target_number)
    {
    case dvt_case_fw_download_denied:
        download_info->status = connector_firmware_status_download_denied;
        break;

    case dvt_case_fw_invalid_size:
        download_info->status = connector_firmware_status_download_invalid_size;
        break;

    case dvt_case_fw_invalid_version:
        download_info->status = connector_firmware_status_download_invalid_version;
        break;

    case dvt_case_fw_unauthenticated:
        download_info->status = connector_firmware_status_download_unauthenticated;
        break;

    case dvt_case_fw_not_allowed:
        download_info->status = connector_firmware_status_download_not_allowed;
        break;

    case dvt_case_fw_configured_to_reject:
        download_info->status = connector_firmware_status_download_configured_to_reject;
        break;

    case dvt_case_fw_encountered_error:
        download_info->status = connector_firmware_status_encountered_error;
        break;

    default:
        dvt_current_ptr->state = dvt_state_fw_download_progress;
        download_info->status = connector_firmware_status_success;
        break;
    }

error:
    if ((dvt_current_ptr != NULL) && (download_info->status != connector_firmware_status_success))
        dvt_current_ptr = NULL;
    return status;
}

static connector_callback_status_t app_firmware_image_data(connector_firmware_download_data_t * const image_data)
{
    connector_callback_status_t status = connector_callback_continue;

    if (image_data == NULL)
    {
        APP_DEBUG("firmware_image_data: invalid parameter\n");
        status = connector_callback_abort;
        goto error;
    }

    if (dvt_current_ptr == NULL)
    {
        image_data->status = connector_firmware_status_device_error;
        goto error;
    }

    if (dvt_current_ptr->state != dvt_state_fw_download_progress)
    {
        APP_DEBUG("firmware_image_data: invalid DVT state [%d]\n", dvt_current_ptr->state);
        image_data->status = connector_firmware_status_device_error;
        goto error;
    }

    APP_DEBUG("target = %d, offset = 0x%04X, length = %zu\n", image_data->target_number, image_data->image.offset, image_data->image.bytes_used);

    switch (image_data->target_number)
    {
    case dvt_case_fw_user_abort:
        image_data->status = connector_firmware_status_user_abort;
        break;

    case dvt_case_fw_device_error:
        image_data->status = connector_firmware_status_device_error;
        break;

    case dvt_case_fw_invalid_offset:
        image_data->status = connector_firmware_status_invalid_offset;
        break;

    case dvt_case_fw_invalid_data:
        image_data->status = connector_firmware_status_invalid_data;
        break;

    case dvt_case_fw_hardware_error:
        image_data->status = connector_firmware_status_hardware_error;
        break;

    default:
        image_data->status = connector_firmware_status_success;
        goto done;
    }

error:
    dvt_current_ptr = NULL;

done:
    return status;
}

static connector_callback_status_t app_firmware_download_complete(connector_firmware_download_complete_t * const complete_response)
{
    connector_callback_status_t status = connector_callback_continue;

    if (complete_response == NULL)
    {
        APP_DEBUG("firmware_download_complete Error: Invalid parameters\n");
        status = connector_callback_abort;
        goto done;
    }

    APP_DEBUG("Download complete: target    = %d\n", complete_response->target_number);

    if (dvt_current_ptr == NULL)
    {
        complete_response->status = connector_firmware_download_not_complete;
        goto done;
    }

    if (dvt_current_ptr->state != dvt_state_fw_download_progress)
    {
        APP_DEBUG("firmware_download_complete: invalid DVT state [%d]\n", dvt_current_ptr->state);
        complete_response->status = connector_firmware_download_not_complete;
    }
    else
    {
        complete_response->status = connector_firmware_download_success;
    }


done:
    dvt_current_ptr = NULL;
    return status;
}

static connector_callback_status_t app_firmware_download_abort(connector_firmware_download_abort_t const * const abort_data)
{
    connector_callback_status_t   status = connector_callback_continue;

    /* Server is aborting firmware update */
    APP_DEBUG("firmware_download_abort\n");
    if (abort_data == NULL)
    {
        APP_DEBUG("firmware_download_abort Error: Invalid parameters\n");
        status = connector_callback_abort;

    }

    dvt_current_ptr = NULL;
    return status;
}

static connector_callback_status_t app_firmware_reset(connector_firmware_reset_t const * const reset_data)
{
    connector_callback_status_t   status = connector_callback_continue;

    UNUSED_ARGUMENT(reset_data);
    /* Server requests firmware reboot */
    APP_DEBUG("firmware_reset\n");

    if (dvt_current_ptr->state == dvt_state_fw_download_complete)
        dvt_current_ptr->state = dvt_state_reset_called;

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

