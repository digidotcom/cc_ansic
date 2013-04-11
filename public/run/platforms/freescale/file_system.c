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



#include "connector_config.h"

#ifdef FILE_SYSTEM_SDCARD
#include "sdcard_task.h"
#endif
#ifdef FILE_SYSTEM_USB
#include "MFS_USB.h"
#include "USB_File.h"
#endif

#include "platform.h"
#include <errno.h>



#ifdef CONNECTOR_FILE_SYSTEM

#if CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH > 460
#error The maximum supported CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH is 460
#endif

#if !defined(FILE_SYSTEM_USB) && !defined(FILE_SYSTEM_SDCARD)
#error "You must define where the Filesyste is located (FILE_SYSTEM_SDCARD or FILE_SYSTEM_USB)"
#endif

#if !defined CONNECTOR_FILE_SYSTEM 
#error "Please define CONNECTOR_FILE_SYSTEM in connector_config.h to run this sample" 
#endif

#if CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH > 460
#error The maximum supported CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH is 460
#endif

#ifndef APP_MIN_VALUE
#define APP_MIN_VALUE(a,b) (((a)<(b))?(a):(b))
#endif

static uint_32 mfs_date_to_epoch(MFS_DATE_TIME_PARAM_PTR date)
{
	TIME_STRUCT epoch;
	DATE_STRUCT mqx_date;
	
	mqx_date.YEAR = ((*date->DATE_PTR & MFS_MASK_YEAR) >> MFS_SHIFT_YEAR) + 1980; /* MS-DOS has different reference! */
	mqx_date.MONTH = (*date->DATE_PTR & MFS_MASK_MONTH) >> MFS_SHIFT_MONTH;
	mqx_date.DAY = (*date->DATE_PTR & MFS_MASK_DAY) >> MFS_SHIFT_DAY;
	mqx_date.HOUR = (*date->TIME_PTR & MFS_MASK_HOURS) >> MFS_SHIFT_HOURS;
	mqx_date.MINUTE = (*date->TIME_PTR & MFS_MASK_MINUTES) >> MFS_SHIFT_MINUTES;
	mqx_date.SECOND = ((*date->TIME_PTR & MFS_MASK_SECONDS) >> MFS_SHIFT_SECONDS) * 2; /* Resolution of seconds is 2 */
	mqx_date.MILLISEC = 0;
	
	if (_time_from_date(&mqx_date, &epoch)) {
		return epoch.SECONDS;	
	} else {
		return ~0; /* Error ocurred, probably invalid date*/
	}
}

#if 0
static connector_callback_status_t app_process_file_strerror(connector_file_data_response_t * response_data)
{
    size_t strerr_size = 0;

    connector_file_error_data_t * error_data = response_data->error;
    long int errnum = (long int)error_data->errnum;

    if (errnum != 0)
    {
        char * err_str = strerror(errnum);
        char * ptr = response_data->data_ptr;

        strerr_size = APP_MIN_VALUE(strlen(err_str), response_data->size_in_bytes);
        memcpy(ptr, err_str, strerr_size);
    }

    response_data->size_in_bytes = strerr_size;

    return connector_callback_continue;
}

static connector_callback_status_t app_process_file_msg_error(connector_file_error_request_t const * const request_data,
                                                          connector_file_response_t * const response_data)
{
    UNUSED_ARGUMENT(request_data);
    UNUSED_ARGUMENT(response_data);
    APP_DEBUG("Message Error %d\n", request_data->message_status);

    // All application resources, used in the session, must be released in this callback
    return connector_callback_continue;
}
#endif

static connector_callback_status_t app_process_file_hash(connector_file_system_hash_t * const data)
{

    // app_process_file_hash() should not be called if APP_ENABLE_MD5 is not defined

    memset(data->hash_value, 0, data->bytes_requested);
    return connector_callback_continue;
}


static connector_callback_status_t app_process_file_stat(connector_file_system_stat_t * const data)
{
    connector_file_system_statbuf_t *file_stat = &data->statbuf;
    connector_callback_status_t status = connector_callback_continue;
	MFS_FILE_ATTR_PARAM attributes_param;
	uchar attributes = 0;
    _mqx_int result;
    char full_path[50] = {0};
    
    strcpy(full_path, filesystem_info->FS_NAME);
    strcat(full_path, data->path);
    
    attributes_param.PATHNAME = full_path;
    attributes_param.ATTRIBUTE_PTR = &attributes;
    
    result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_GET_FILE_ATTR, &attributes_param);
    if (result < 0) {
		APP_DEBUG("SP:%s:%s:%d:ERROR:IO_IOCTL_GET_FILE_ATTR: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
		goto done;
    }

    APP_DEBUG ("SP: Attributes of %s: %s%s%s%s%s%s\n",
    		full_path,
			(attributes & MFS_ATTR_READ_ONLY) ? "R/O ":"",
			(attributes & MFS_ATTR_HIDDEN_FILE) ? "HID ":"",
			(attributes & MFS_ATTR_SYSTEM_FILE) ? "SYS ":"",
			(attributes & MFS_ATTR_VOLUME_NAME) ? "VOL ":"",
			(attributes & MFS_ATTR_DIR_NAME) ? "DIR ":"",
			(attributes & MFS_ATTR_ARCHIVE) ? "ARC ":"");

    if (attributes & (MFS_ATTR_DIR_NAME) || attributes == 0) {
    	file_stat->flags = connector_file_system_file_type_is_dir;
    	file_stat->file_size = 0;
    	file_stat->last_modified = 0; /* Directories don't have this field */
    } else if (attributes & MFS_ATTR_ARCHIVE) {
        MQX_FILE_PTR file;

    	file = _io_fopen(full_path, "r");
		if (file == NULL) {
			//status = app_process_file_error(response_data->error, errno);
			APP_DEBUG("SP:%s:%s:%d:ERROR:failed to open \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, full_path);
		}
		file_stat->file_size = file->SIZE;
		file_stat->flags = connector_file_system_file_type_is_reg;
		{
			uint_16 date, time;
			uint32_t epoch_time = 0;
			MFS_DATE_TIME_PARAM file_date;
			
			file_date.DATE_PTR = &date;
			file_date.TIME_PTR = &time;
			
			result = _io_ioctl(file, IO_IOCTL_GET_DATE_TIME, (uint_32 *) &file_date);
			if (result < 0) {
				APP_DEBUG("SP:%s:%s:%d:ERROR:IO_IOCTL_GET_DATE_TIME: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
				goto done;
			}
			
			file_stat->last_modified = mfs_date_to_epoch(&file_date);
		}
		_io_fclose(file);
    } else {
    	file_stat->flags = 0;
		file_stat->file_size = 0;
		file_stat->last_modified = 0; /* Directories don't have this field */
    }
	data->hash_algorithm.actual = connector_file_system_hash_none;
done:
    return status;
}

static connector_callback_status_t app_process_file_stat_dir_entry(connector_file_system_stat_dir_entry_t * const data)
{
    connector_file_system_statbuf_t *file_stat = &data->statbuf;
    connector_callback_status_t status = connector_callback_continue;
	MFS_FILE_ATTR_PARAM attributes_param;
	uchar attributes = 0;
    _mqx_int result;
    char full_path[50] = {0};
    APP_DEBUG("SP:%s\n", __FUNCTION__);
    strcpy(full_path, filesystem_info->FS_NAME);
    strcat(full_path, data->path);
    
    attributes_param.PATHNAME = full_path;
    attributes_param.ATTRIBUTE_PTR = &attributes;
    
    result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_GET_FILE_ATTR, &attributes_param);
    if (result < 0) {
		APP_DEBUG("SP:%s:%s:%d:ERROR:IO_IOCTL_GET_FILE_ATTR: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
		goto done;
    }

    APP_DEBUG ("SP: Attributes of %s: %s%s%s%s%s%s\n",
    		full_path,
			(attributes & MFS_ATTR_READ_ONLY) ? "R/O ":"",
			(attributes & MFS_ATTR_HIDDEN_FILE) ? "HID ":"",
			(attributes & MFS_ATTR_SYSTEM_FILE) ? "SYS ":"",
			(attributes & MFS_ATTR_VOLUME_NAME) ? "VOL ":"",
			(attributes & MFS_ATTR_DIR_NAME) ? "DIR ":"",
			(attributes & MFS_ATTR_ARCHIVE) ? "ARC ":"");

    if (attributes & (MFS_ATTR_DIR_NAME) || attributes == 0) {
    	file_stat->flags = connector_file_system_file_type_is_dir;
    	file_stat->file_size = 0;
    	file_stat->last_modified = 0; /* Directories don't have this field */
    } else if (attributes & MFS_ATTR_ARCHIVE) {
        MQX_FILE_PTR file;

    	file = _io_fopen(full_path, "r");
		if (file == NULL) {
			//status = app_process_file_error(response_data->error, errno);
			APP_DEBUG("SP:%s:%s:%d:ERROR:failed to open \"%s\"\n", __FILE__, __FUNCTION__, __LINE__, full_path);
		}
		file_stat->file_size = file->SIZE;
		file_stat->flags = connector_file_system_file_type_is_reg;
		{
			uint_16 date, time;
			uint32_t epoch_time = 0;
			MFS_DATE_TIME_PARAM file_date;
			
			file_date.DATE_PTR = &date;
			file_date.TIME_PTR = &time;
			
			result = _io_ioctl(file, IO_IOCTL_GET_DATE_TIME, (uint_32 *) &file_date);
			if (result < 0) {
				APP_DEBUG("SP:%s:%s:%d:ERROR:IO_IOCTL_GET_DATE_TIME: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
				goto done;
			}
			
			file_stat->last_modified = mfs_date_to_epoch(&file_date);
		}
		_io_fclose(file);
    } else {
    	file_stat->flags = 0;
		file_stat->file_size = 0;
		file_stat->last_modified = 0; /* Directories don't have this field */
    }
done:
    return status;
}

#define	CONNECTOR_FS_FIRST_SEARCH	((void *) 0xFF)
#define	CONNECTOR_FS_NEXT_SEARCH	((void *) 0x00)
static connector_callback_status_t app_process_file_opendir(connector_file_system_opendir_t * const data)
{
    connector_callback_status_t status = connector_callback_continue;
    _mqx_int result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_CHANGE_CURRENT_DIR, (uint_32 *)data->path);

    if (result < 0) {
    	//status = app_process_file_error(response_data->error, errno);
		APP_DEBUG("SP:%s:%s:%d:ERROR:IO_IOCTL_CHANGE_CURRENT_DIR: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
    } else {
    	MFS_SEARCH_PARAM *search_param = _mem_alloc(sizeof(MFS_SEARCH_PARAM));
    	MFS_SEARCH_DATA *search_data = _mem_alloc(sizeof(MFS_SEARCH_DATA));
    	
    	if (search_param == NULL) {
    		APP_DEBUG("SP:%s:%s:%d:ERROR:failed to allocate memmory for MFS_SEARCH_PARAM\n", __FILE__, __FUNCTION__, __LINE__);
    		goto done;
    	} else if (search_data == NULL) {
    		APP_DEBUG("SP:%s:%s:%d:ERROR:failed to allocate memmory for MFS_SEARCH_DATA\n", __FILE__, __FUNCTION__, __LINE__);
    		goto done;
    	}
    	search_param->ATTRIBUTE = MFS_SEARCH_ANY;
    	search_param->WILDCARD = "*";
    	search_param->SEARCH_DATA_PTR = search_data;
    	APP_DEBUG("SP:%s:search_param -> %p\n", __FUNCTION__, search_param);
    	data->user_context = CONNECTOR_FS_FIRST_SEARCH;
    	data->handle = search_param;
		APP_DEBUG("SP:%s:%s:%d: Opened %s\n", __FILE__, __FUNCTION__, __LINE__, data->path);
    }

done:
    return status;
}

static connector_callback_status_t app_process_file_readdir(connector_file_system_readdir_t * const data)
{
    connector_callback_status_t status = connector_callback_continue;
    _mqx_int result;
    MFS_SEARCH_PARAM *search_param = data->handle;
	MFS_SEARCH_DATA *search_data = search_param->SEARCH_DATA_PTR;
	
	if (data->user_context == CONNECTOR_FS_FIRST_SEARCH) {
		APP_DEBUG("SP:%s:search_param -> %p\n", __FUNCTION__, search_param);
		result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_FIND_FIRST_FILE, (uint_32_ptr)search_param);
		data->user_context = CONNECTOR_FS_NEXT_SEARCH;
	} else {
		result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_FIND_NEXT_FILE, search_data);
	}
	
	if (result != MFS_NO_ERROR) {
		APP_DEBUG("No more directory entries\n");
		//data->size_in_bytes = 0;
	} else {
		size_t name_length;
		MFS_GET_LFN_STRUCT long_filename_struct;
		
		long_filename_struct.PATHNAME = search_data->NAME;
		long_filename_struct.LONG_FILENAME = data->entry_name;
		long_filename_struct.SEARCH_DATA_PTR = NULL;
		result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_GET_LFN, (uint_32 *)&long_filename_struct);
		if (result != MFS_NO_ERROR) {
			/* It's not a long filename */	
			memcpy(data->entry_name, search_data->NAME, strlen(search_data->NAME) + 1);
		}

		name_length = strlen(data->entry_name) + 1;
		//data->bytes_available = name_length;
		APP_DEBUG("SP:%s:%s:%d: read entry  %s\n", __FILE__, __FUNCTION__, __LINE__, data->entry_name);
	}
	
    return status;
}

static connector_callback_status_t app_process_file_closedir(connector_file_system_close_t * const data)
{
    MFS_SEARCH_PARAM *search_param = data->handle;
	MFS_SEARCH_DATA *search_data = search_param->SEARCH_DATA_PTR;
	_mqx_int result;

	result = _mem_free(search_param);
	if (result < 0) {
		//status = app_process_file_error(response_data->error, errno);
		APP_DEBUG("SP:%s:%s:%d:ERROR:_io_fclose: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
	}
	result = _mem_free(search_data);
	if (result < 0) {
		//status = app_process_file_error(response_data->error, errno);
		APP_DEBUG("SP:%s:%s:%d:ERROR:_io_fclose: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
	}
	APP_DEBUG("SP:%s:%s:%d: Closed\n", __FILE__, __FUNCTION__, __LINE__);
    return connector_callback_continue;
}


static connector_callback_status_t app_process_file_get_error(connector_file_system_get_error_t * const data)
{
    long int errnum = (long int)data->errnum;

    data->bytes_used = 0;

    if (errnum != 0)
    {
        char * err_str = strerror(errnum);

        data->bytes_used = APP_MIN_VALUE(strlen(err_str), data->bytes_available);
        memcpy(data->buffer, err_str, data->bytes_used);
    }

    switch(errnum)
    {
        case EACCES:
        case EPERM:
        case EROFS:
            data->error_status = connector_file_system_permision_denied;
            break;

        case ENOMEM:
        case ENAMETOOLONG:
            data->error_status = connector_file_system_out_of_memory;
            break;

        case ENOENT:
        case ENODEV:
        case EBADF:
            data->error_status = connector_file_system_path_not_found;
            break;

        case EINVAL:
        case ENOSYS:
        case ENOTDIR:
        case EISDIR:
            data->error_status = connector_file_system_invalid_parameter;
            break;

        case ENOSPC:
            data->error_status = connector_file_system_insufficient_storage_space;
            break;

        default:
            data->error_status = connector_file_system_unspec_error;
            break;
    }

    return connector_callback_continue;
}


static char const * app_convert_file_open_mode(int const oflag)
{ /* TODO, check this */
    if ((oflag & (CONNECTOR_FILE_O_WRONLY | CONNECTOR_FILE_O_RDWR)) == 0) {
        return "r";
    } else if (oflag & CONNECTOR_FILE_O_WRONLY) {
    	return"w";
    } else if (oflag & CONNECTOR_FILE_O_RDWR) {
    	return "r+";
    } else if (oflag & CONNECTOR_FILE_O_APPEND) {
    	return "a";
    } else if (oflag & CONNECTOR_FILE_O_CREAT) {
    	return "a+";
    } else if (oflag & CONNECTOR_FILE_O_TRUNC) {
    	return "w+";
    } else {
    	return "r";
    }
}

static connector_callback_status_t app_process_file_open(connector_file_system_open_t * const data)
{
    connector_callback_status_t status = connector_callback_continue;
    char const * oflag = app_convert_file_open_mode(data->oflag);

    MQX_FILE_PTR fd;
    char full_path[50] = {0};
    
    strcpy(full_path, filesystem_info->FS_NAME);
    strcat(full_path, data->path);
    
    fd = _io_fopen(full_path, oflag);
    		
    if (fd == NULL)
    {
        //status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("SP:%s:%s:%d:ERROR:_io_fopen returned: %ld\n", __FILE__, __FUNCTION__, __LINE__, fd);
    }

    APP_DEBUG("Open %s, %s, returned %ld\n", data->path, oflag, fd);

    data->handle = (void *) fd;
    data->user_context = NULL;
    
    return status;
}

static int app_convert_lseek_origin(int const origin)
{
#if 1//(CONNECTOR_SEEK_SET == SEEK_SET) && (CONNECTOR_SEEK_CUR == SEEK_CUR) && (CONNECTOR_SEEK_END == SEEK_END)

    return origin;
#else
    int result;

    switch(origin)
    {
    case CONNECTOR_SEEK_SET:
        result = SEEK_SET;
        break;
    case CONNECTOR_SEEK_END:
        result = SEEK_END;
        break;
    case CONNECTOR_SEEK_CUR:
    default:
        result = SEEK_CUR;
        break;
    }

    return result;
#endif
}

static connector_callback_status_t app_process_file_lseek(connector_file_system_lseek_t * const data)
{
    connector_callback_status_t status = connector_callback_continue;
    MQX_FILE_PTR fd = data->handle;
    int origin;
    _mqx_int offset;
    
    switch(data->origin)
	{
		case connector_file_system_seek_set:
			origin = 0;//SEEK_SET;
			break;

		case connector_file_system_seek_end:
			origin = 1;//SEEK_END;
			break;

		case connector_file_system_seek_cur:
		default:
			origin = 2;//SEEK_CUR;
			break;
	}

    offset = fseek(fd, data->requested_offset, origin);
    if (offset < 0)
    {
        //status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("SP:%s:%s:%d:ERROR:fseek returned: %ld\n", __FILE__, __FUNCTION__, __LINE__, fd);
    }
	data->resulting_offset = (connector_file_offset_t) offset;

	APP_DEBUG("lseek fd %ld, offset %d, origin %d returned %d", fd, data->requested_offset, data->origin, data->resulting_offset);
	
	if (offset < 0)
		APP_DEBUG(", errno %d\n", errno);
	else 
		APP_DEBUG("\n");

    return status;
}

static connector_callback_status_t app_process_file_ftruncate(connector_file_system_truncate_t * const data)
{
#if SP
    connector_callback_status_t status = connector_callback_continue;
    MQX_FILE_PTR fd = data->handle;
    _mqx_int result = ftruncate(fd, data->length);

    if (result < 0)
    {
        //status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("SP:%s:%s:%d:ERROR:truncate returned: %ld\n", __FILE__, __FUNCTION__, __LINE__, fd);
    }
    APP_DEBUG("ftruncate %ld, %ld returned %d\n", fd, data->length, result);

    return status;
#endif
    APP_DEBUG("SP:%s:%s:%d:ERROR:app_process_file_ftruncate not implemented yet!!\n", __FILE__, __FUNCTION__, __LINE__);
    return connector_callback_continue;
}

static connector_callback_status_t app_process_file_remove(connector_file_system_remove_t * const data)
{
    connector_callback_status_t status = connector_callback_continue;
	uchar attributes;
	MFS_FILE_ATTR_PARAM attributes_param;
    _mqx_int result;
    char full_path[50] = {0};
    
    strcpy(full_path, filesystem_info->FS_NAME);
    strcat(full_path, data->path);
    
    attributes_param.PATHNAME = full_path;
    attributes_param.ATTRIBUTE_PTR = &attributes;

    /* Read attributes to determine if it is a regular file or a subdirectory */
    result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_GET_FILE_ATTR, &attributes_param);
    if (result != MFS_NO_ERROR) {
		APP_DEBUG("SP:%s:%s:%d:ERROR:IO_IOCTL_GET_FILE_ATTR: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
		goto done;
    }

    if (attributes & MFS_ATTR_ARCHIVE) {
    	result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_DELETE_FILE, (void *)data->path);
    } else if (attributes & MFS_ATTR_DIR_NAME) {
    	result = _io_ioctl(filesystem_info->FS_FD_PTR, IO_IOCTL_REMOVE_SUBDIR, (void *)data->path);
    } else {
    	/* TODO, set errors properly */
    	APP_DEBUG("SP:%s:%s:%d: Permision denied %s\n", __FILE__, __FUNCTION__, __LINE__, full_path);
    	//data->error->error_status = connector_file_permision_denied;
    	goto done;
    }
    
    if (result != MFS_NO_ERROR)
    {
        //status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("SP:%s:%s:%d:ERROR:_io_ioctl %s returned: %ld\n", __FILE__, __FUNCTION__, __LINE__, attributes & MFS_ATTR_ARCHIVE ? "IO_IOCTL_DELETE_FILE" : "IO_IOCTL_REMOVE_SUBDIR", result);
        goto done;
    }    


done:
    APP_DEBUG("unlink %s returned %d\n", data->path, result);
    return status;
}

static connector_callback_status_t app_process_file_read(connector_file_system_read_t * const data)
{
    connector_callback_status_t status = connector_callback_continue;
    MQX_FILE_PTR fd = data->handle;

    _mqx_int result = _io_read(fd, data->buffer, data->bytes_available);

    if (result < 0)
    {
        //status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("SP:%s:%s:%d:ERROR:_io_read returned: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
        APP_DEBUG("read %ld, %zu, returned %d, errno %d\n", fd, data->bytes_available, result, errno);
        goto done;
    }

    APP_DEBUG("read %ld, %zu, returned %d\n", fd, data->bytes_available, result);
    data->bytes_used = result;

done:
    return status;
}

static connector_callback_status_t app_process_file_write(connector_file_system_write_t * const data)
{
    connector_callback_status_t status = connector_callback_continue;
    MQX_FILE_PTR fd = data->handle;

    _mqx_int result = _io_write(fd, (void *)data->buffer, data->bytes_available);
    
    if (result < 0)
    {
     //  status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("SP:%s:%s:%d:ERROR:_io_write returned: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
        APP_DEBUG("write %ld, %zu, returned %d, errno %d\n", fd, data->bytes_available, result, errno);
        goto done;
    }

    APP_DEBUG("write %ld, %zu, returned %d\n", fd, data->bytes_available, result);

    data->bytes_used = result;
    
done:
    return status;
}

static connector_callback_status_t app_process_file_close(connector_file_system_close_t * const data)
{
    connector_callback_status_t status = connector_callback_continue;
    MQX_FILE_PTR fd = data->handle;
    int result = _io_fclose(fd);

    if (result < 0 && errno == EIO)
    {
        //status = app_process_file_error(response_data->error, EIO);
        APP_DEBUG("SP:%s:%s:%d:ERROR:_io_fclose returned: %ld\n", __FILE__, __FUNCTION__, __LINE__, result);
    }

    APP_DEBUG("close %ld returned %d\n", fd, result);

    // All application resources, used in the session, must be released in this callback

    return status;
}

static connector_callback_status_t app_process_file_session_error(connector_file_system_session_error_t * const data)
{
     APP_DEBUG("Session Error %d\n", data->session_error);

    /* All application resources, used in the session, must be released in this callback */
#if 0
     if (data->user_context != NULL)
    {
        app_md5_ctx * ctx = data->user_context;

        if (ctx->fd >= 0)
            close(ctx->fd);

        free(data->user_context);
        data->user_context = NULL;
    }
#endif
    return connector_callback_continue;
}

connector_callback_status_t app_file_system_handler(connector_request_id_file_system_t const request, void * const data)
{
    connector_callback_status_t status = connector_callback_continue;

    switch (request)
    {
        case connector_request_id_file_system_open:
            status = app_process_file_open(data);
            break;

        case connector_request_id_file_system_read:
            status = app_process_file_read(data);
            break;

        case connector_request_id_file_system_write:
            status = app_process_file_write(data);
            break;

        case connector_request_id_file_system_lseek:
            status = app_process_file_lseek(data);
            break;

        case connector_request_id_file_system_close:
            status = app_process_file_close(data);
            break;

        case connector_request_id_file_system_ftruncate:
            status = app_process_file_ftruncate(data);
            break;

        case connector_request_id_file_system_remove:
            status = app_process_file_remove(data);
            break;

        case connector_request_id_file_system_stat:
            status = app_process_file_stat(data);
            break;

        case connector_request_id_file_system_stat_dir_entry:
            status = app_process_file_stat_dir_entry(data);
            break;

        case connector_request_id_file_system_opendir:
            status = app_process_file_opendir(data);
            break;

        case connector_request_id_file_system_readdir:
            status = app_process_file_readdir(data);
            break;

        case connector_request_id_file_system_closedir:
            status = app_process_file_closedir(data);
            break;

        case connector_request_id_file_system_get_error:
            status = app_process_file_get_error(data);
            break;

        case connector_request_id_file_system_hash:
            status = app_process_file_hash(data);
            break;

        case connector_request_id_file_system_session_error:
            status = app_process_file_session_error(data);
            break;

        default:
            status = connector_callback_unrecognized;
            APP_DEBUG("Unsupported file system request %d\n", request);
    }

    return status;
}
#endif /* #ifdef CONNECTOR_FILE_SYSTEM */
