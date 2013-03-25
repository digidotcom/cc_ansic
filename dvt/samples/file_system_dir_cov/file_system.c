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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "connector_api.h"
#include "platform.h"
#include "connector_config.h"
#include "connector_debug.h"

#if !defined CONNECTOR_FILE_SYSTEM 
#error "Please define CONNECTOR_FILE_SYSTEM in connector_config.h to run this sample" 
#endif

#if CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH > 460
#error The maximum supported CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH is 460
#endif


#if defined APP_ENABLE_MD5
#include <openssl/md5.h>

#define APP_MD5_BUFFER_SIZE 1024

typedef struct
{
    MD5_CTX md5;
    char buf[APP_MD5_BUFFER_SIZE];
    unsigned int flags;
    int fd;

} app_md5_ctx;
#endif

#ifndef APP_MIN_VALUE
#define APP_MIN_VALUE(a,b) (((a)<(b))?(a):(b))
#endif

typedef struct
{
    DIR * dirp;
    struct dirent dir_entry;

} app_dir_data_t;


static connector_callback_status_t app_process_file_error(connector_file_error_data_t * const error_data, long int const errnum)
{
    connector_callback_status_t status = connector_callback_continue;

    error_data->errnum = (void *) errnum;

    switch(errnum)
    {
        case EACCES:
        case EPERM:
        case EROFS:
            error_data->error_status = connector_file_permision_denied;
            break;

        case ENOMEM:
        case ENAMETOOLONG:
            error_data->error_status = connector_file_out_of_memory;
            break;

        case ENOENT:
        case ENODEV:
        case EBADF:
            error_data->error_status = connector_file_path_not_found;
            break;

        case EINVAL:
        case ENOSYS:
        case ENOTDIR:
        case EISDIR:
            error_data->error_status = connector_file_invalid_parameter;
            break;

#if EAGAIN != EWOULDBLOCK
        case EWOULDBLOCK:
#endif
        case EAGAIN:
            status = connector_callback_busy;
            break;

        case ENOSPC:
            error_data->error_status = connector_file_insufficient_storage_space;
            break;

        default:
            error_data->error_status = connector_file_unspec_error;
            break;
    }
    return status;
}

static int app_convert_file_open_mode(int const oflag)
{
#if (CONNECTOR_O_RDONLY == O_RDONLY) && (CONNECTOR_O_WRONLY == O_WRONLY) && (CONNECTOR_O_RDWR == O_RDWR) && \
    (CONNECTOR_O_CREAT == O_CREAT)   && (CONNECTOR_O_APPEND == O_APPEND) && (CONNECTOR_O_TRUNC == O_TRUNC)

    return oflag;
#else
    int result = 0;

    if (oflag & CONNECTOR_O_WRONLY) result |= O_WRONLY;
    if (oflag & CONNECTOR_O_RDWR)   result |= O_RDWR;
    if (oflag & CONNECTOR_O_APPEND) result |= O_APPEND;
    if (oflag & CONNECTOR_O_CREAT)  result |= O_CREAT;
    if (oflag & CONNECTOR_O_TRUNC)  result |= O_TRUNC;

    if ((oflag & (CONNECTOR_O_WRONLY | CONNECTOR_O_RDWR)) == 0)
        result |= O_RDONLY;

    return result;
#endif
}

static int app_convert_lseek_origin(int const origin)
{
#if (CONNECTOR_SEEK_SET == SEEK_SET) && (CONNECTOR_SEEK_CUR == SEEK_CUR) && (CONNECTOR_SEEK_END == SEEK_END)

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


#if defined APP_ENABLE_MD5
static app_md5_ctx * app_allocate_md5_ctx(unsigned int const flags, connector_file_error_data_t * const error_data)
{
    app_md5_ctx * ctx = malloc(sizeof *ctx);

    if (ctx != NULL)
    {
        ctx->flags = flags;
        ctx->fd    = -1;
    }
    else
    {
        app_process_file_error(error_data, ENOMEM);
        APP_DEBUG("app_allocate_md5_ctx: malloc fails\n");
    }
    return ctx;
}

static connector_callback_status_t app_process_file_msg_error(connector_file_error_request_t const * const request_data,
                                                          connector_file_response_t * const response_data)
{
    UNUSED_ARGUMENT(request_data);
    APP_DEBUG("Message Error %d\n", request_data->message_status);

    // All application resources, used in the session, must be released in this callback
    if (response_data->user_context != NULL)
    {
        app_md5_ctx * ctx = response_data->user_context;

        if (ctx->fd >= 0)
            close(ctx->fd);

        free(response_data->user_context);
        response_data->user_context = NULL;
    }
    return connector_callback_continue;
}

static connector_callback_status_t app_process_file_hash(connector_file_path_request_t const * const request_data,
                                                     connector_file_data_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    app_md5_ctx * ctx = response_data->user_context;
    int ret;

    if (ctx == NULL)
    {   
        ASSERT(0);
        goto error;
    }

    if (ctx->fd < 0)
    {
        ctx->fd = open(request_data->path, O_RDONLY);
        APP_DEBUG("app_process_file_hash: Open %s, returned %d\n", request_data->path, ctx->fd);

        if (ctx->fd < 0)
        {
            ASSERT(0);
            goto error;
        }

        MD5_Init(&ctx->md5);
    }

    while ((ret = read (ctx->fd, ctx->buf, sizeof ctx->buf)) > 0)
    {
        MD5_Update(&ctx->md5, ctx->buf, ret);
    }
    if (ret == -1 && errno == EAGAIN)
    {
        status = connector_callback_busy;
        goto done;
    }

    APP_DEBUG("app_process_file_hash: Close %d\n", ctx->fd);
    close (ctx->fd);
    ctx->fd = -1;

    if (ret == 0)
    {
        MD5_Final (response_data->data_ptr, &ctx->md5);
        goto done;
    }

error:
    memset(response_data->data_ptr, 0, response_data->size_in_bytes);

done:
    if (ctx != NULL && status == connector_callback_continue)
    {
        // free md5 context here,  if ls was issued a single file
        if ((ctx->flags & CONNECTOR_FILE_IS_DIR) == 0)
        {
            free(response_data->user_context);
            response_data->user_context = NULL;
        }
    }
    return status;
}
#else

static connector_callback_status_t app_process_file_msg_error(connector_file_error_request_t const * const request_data,
                                                          connector_file_response_t * const response_data)
{
    UNUSED_ARGUMENT(request_data);
    UNUSED_ARGUMENT(response_data);
    APP_DEBUG("Message Error %d\n", request_data->message_status);

    // All application resources, used in the session, must be released in this callback
    return connector_callback_continue;
}

static connector_callback_status_t app_process_file_hash(connector_file_path_request_t const * const request_data,
                                                     connector_file_data_response_t * const response_data)
{
    UNUSED_ARGUMENT(request_data);

    // app_process_file_hash() should not be called if APP_ENABLE_MD5 is not defined
    ASSERT(0);

    memset(response_data->data_ptr, 0, response_data->size_in_bytes);
    return connector_callback_continue;
}
#endif

static connector_callback_status_t app_process_file_stat(connector_file_stat_request_t const * const request_data,
                                                     connector_file_stat_response_t * const response_data)
{
    struct stat statbuf;
    connector_file_stat_t * pstat = &response_data->statbuf;
    connector_callback_status_t status = connector_callback_continue;

    int const result = stat(request_data->path, &statbuf);

    if (result < 0)
    {
        status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("stat for %s returned %d, errno %d\n", request_data->path, result, errno);
        goto done;
    }

    APP_DEBUG("stat for %s returned %d, filesize %ld\n", request_data->path, result, statbuf.st_size);

    pstat->flags = 0;
    pstat->file_size = statbuf.st_size;
    pstat->last_modified = statbuf.st_mtime;
    pstat->hash_alg = connector_file_hash_none;

    if (S_ISDIR(statbuf.st_mode))
       pstat->flags |= CONNECTOR_FILE_IS_DIR;
    else
    if (S_ISREG(statbuf.st_mode))
       pstat->flags |= CONNECTOR_FILE_IS_REG;

#if defined APP_ENABLE_MD5
    /*
     * If ls was issued for a directory
     * - app_process_file_stat() is called with the requested hash algorithm once for this directory.
     * - app_process_file_stat() is called with connector_file_hash_none for each directory entry.
     */
    switch (request_data->hash_alg)
    {
        case connector_file_hash_best:
        case connector_file_hash_md5:
            if (pstat->flags != 0)
            {
                pstat->hash_alg = connector_file_hash_md5;
                if (response_data->user_context == NULL)
                    response_data->user_context = app_allocate_md5_ctx(pstat->flags, response_data->error);
            }
            break;


        default:
            break;
    }
#endif
done:
    return status;
}

static connector_callback_status_t app_process_file_opendir(connector_file_path_request_t const * const request_data,
                                                        connector_file_open_response_t * const response_data)
{

    connector_callback_status_t status = connector_callback_continue;
    DIR * dirp;

    errno = 0;
    dirp = opendir(request_data->path);

    if (dirp != NULL)
    {
        app_dir_data_t * dir_data = malloc (sizeof *dir_data);

        if (dir_data != NULL)
        {
            response_data->handle = dir_data;

            dir_data->dirp = dirp;
            APP_DEBUG("opendir for %s returned %p\n", request_data->path, (void *) dirp);
        }
        else
        {
            APP_DEBUG("app_process_file_opendir: malloc fails %s\n", request_data->path);
            status = app_process_file_error(response_data->error, ENOMEM);
            closedir(dirp);
        }
    }
    else
        status = app_process_file_error(response_data->error, errno);

    return status;
}

static connector_callback_status_t app_process_file_closedir(connector_file_request_t const * const request_data,
                                                         connector_file_response_t * const response_data)
{
    app_dir_data_t * dir_data = request_data->handle;

    ASSERT(dir_data != NULL);
    APP_DEBUG("closedir %p\n", (void *) dir_data->dirp);

    closedir(dir_data->dirp);
    free(dir_data);

    // All application resources, used in the session, must be released in this callback

#if defined APP_ENABLE_MD5
    if (response_data->user_context != NULL)
    {
        // free md5 context here, if ls was issued a directory
        free(response_data->user_context);
        response_data->user_context = NULL;
    }
#else
    UNUSED_ARGUMENT(response_data);
#endif
    return connector_callback_continue;
}

static connector_callback_status_t app_process_file_readdir(connector_file_request_t const * const request_data,
                                                        connector_file_data_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    app_dir_data_t * dir_data = request_data->handle;
    struct dirent  * result = NULL;

    // Read next directory entry
    int rc = readdir_r(dir_data->dirp, &dir_data->dir_entry, &result);

    // error
    if (rc != 0)
    {
        status = app_process_file_error(response_data->error, rc);
        APP_DEBUG("readdir_r returned %d\n", rc);
        goto done;
    }

    // finished with the directory
    if (result == NULL)
    {
        APP_DEBUG("No more directory entries %d\n", rc);
        response_data->size_in_bytes = 0;
        goto done;
    }

    // read valid entry
    {
        size_t const name_len = strlen(result->d_name);

        APP_DEBUG("readdir_r returned directory entry name %s\n", result->d_name);

        if(name_len < response_data->size_in_bytes)
        {
            memcpy(response_data->data_ptr, result->d_name, name_len + 1);
            response_data->size_in_bytes = name_len + 1;
        }
        else
        {
            APP_DEBUG("directory entry name too long\n");
            status = app_process_file_error(response_data->error, ENAMETOOLONG);
        }
    }

done:
    return status;
}


static connector_callback_status_t app_process_file_open(connector_file_open_request_t const * const request_data,
                                                     connector_file_open_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    int const oflag = app_convert_file_open_mode(request_data->oflag);

    // 0664 = read,write owner + read,write group + read others
    long int const fd = open(request_data->path, oflag, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);

    if (fd < 0)
    {
        status = app_process_file_error(response_data->error, errno);
    }

    APP_DEBUG("Open %s, %d, returned %ld\n", request_data->path, oflag, fd);

    response_data->handle = (void *) fd;
    response_data->user_context = NULL;

    return status;
}


static connector_callback_status_t app_process_file_lseek(connector_file_lseek_request_t const * const request_data,
                                                      connector_file_lseek_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    int const origin = app_convert_lseek_origin(request_data->origin);
    long int const fd = (long int) request_data->handle;

    long int const offset = lseek(fd, request_data->offset, origin);

    response_data->offset = offset;

    if (offset < 0)
    {
        status = app_process_file_error(response_data->error, errno);
    }

    APP_DEBUG("lseek fd %ld, offset %ld, origin %d returned %ld\n", 
                fd, request_data->offset, request_data->origin, offset);

    return status;
}

static connector_callback_status_t app_process_file_ftruncate(connector_file_ftruncate_request_t const * const request_data,
                                                          connector_file_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    long int const fd = (long int) request_data->handle;

    int const result = ftruncate(fd, request_data->length);

    if (result < 0)
    {
        status = app_process_file_error(response_data->error, errno);
    }

    APP_DEBUG("ftruncate %ld, %ld returned %d\n", fd, request_data->length, result);

    return status;
}

static connector_callback_status_t app_process_file_rm(connector_file_path_request_t const * const request_data,
                                                   connector_file_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;

    int const result = unlink(request_data->path);

    if (result < 0)
    {
        status = app_process_file_error(response_data->error, errno);
    }

    APP_DEBUG("unlink %s returned %d\n", request_data->path, result);

    return status;
}

static connector_callback_status_t app_process_file_read(connector_file_request_t const * const request_data,
                                                     connector_file_data_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    long int const fd = (long int) request_data->handle;

    int const result = read(fd, response_data->data_ptr, response_data->size_in_bytes);

    if (result < 0)
    {
        status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("read %ld, %zu, returned %d, errno %d\n", fd, response_data->size_in_bytes, result, errno);
        goto done;
    }

    APP_DEBUG("read %ld, %zu, returned %d\n", fd, response_data->size_in_bytes, result);
    response_data->size_in_bytes = result;

done:
    return status;
}

static connector_callback_status_t app_process_file_write(connector_file_write_request_t const * const request_data,
                                                      connector_file_write_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    long int const fd = (long int) request_data->handle;

    int const result = write(fd, request_data->data_ptr, request_data->size_in_bytes);

    if (result < 0)
    {
        status = app_process_file_error(response_data->error, errno);
        APP_DEBUG("write %ld, %zu, returned %d, errno %d\n", fd, request_data->size_in_bytes, result, errno);
        goto done;
    }

    APP_DEBUG("write %ld, %zu, returned %d\n", fd, request_data->size_in_bytes, result);

    response_data->size_in_bytes = result;

done:
    return status;
}

static connector_callback_status_t app_process_file_close(connector_file_request_t const * const request_data,
                                                      connector_file_response_t * const response_data)
{
    connector_callback_status_t status = connector_callback_continue;
    long int const fd = (long int) request_data->handle;
    int const result = close(fd);

    if (result < 0 && errno == EIO)
    {
        status = app_process_file_error(response_data->error, EIO);
    }

    APP_DEBUG("close %ld returned %d\n", fd, result);

    // All application resources, used in the session, must be released in this callback

    return status;
}

typedef enum 
{
    action_none,
    return_abort,
    return_busy,
    return_error,
    return_unrecognized,
    return_invalid_code,
    set_user_cancel,
    set_io_error,
    set_io_error_no_errno,
    set_bad_resp_length,
    set_null_handle,
    set_zero_datalength,
    set_invalid_datalength,
    set_invalid_data,
    partial_process,
    partial_process2,
    set_file_flags_zero

} test_action_t;

typedef enum{
    no_cleanup,
    do_cleanup
} dvt_cleanup_type_t;

static struct 
{
 
    char * path;
    int    callback_cnt;
    dvt_cleanup_type_t cleanup;
    connector_file_system_request_t callback_id;
    test_action_t  action;

} test_table[] = {

    { "dvt_fs_stat1_abort",             1, do_cleanup, connector_file_system_stat, return_abort},
    { "dvt_fs_stat1_unrecognized",      1, do_cleanup, connector_file_system_stat, return_unrecognized},
    { "dvt_fs_stat1_reterror",          1, do_cleanup, connector_file_system_stat, return_error},
    { "dvt_fs_stat1_retinvalid",        1, do_cleanup, connector_file_system_stat, return_invalid_code},
    { "dvt_fs_stat1_user_cancel",       1, do_cleanup, connector_file_system_stat, set_user_cancel},
    { "dvt_fs_stat1_io_error",          1, do_cleanup, connector_file_system_stat, set_io_error},
    { "dvt_fs_stat1_io_error_no_errno", 1, do_cleanup, connector_file_system_stat, set_io_error_no_errno},
    { "dvt_fs_stat1_bad_length",        1, do_cleanup, connector_file_system_stat, set_bad_resp_length},
    { "dvt_fs_stat1_busy",              1, no_cleanup, connector_file_system_stat, return_busy},
    { "dvt_fs_stat1_not_regdir",        1, do_cleanup, connector_file_system_stat, set_file_flags_zero},
    { "dvt_fs_stat1_badhash",           1, do_cleanup, connector_file_system_stat, set_invalid_data},

    { "dvt_fs_opendir_abort",             1, do_cleanup, connector_file_system_opendir, return_abort},
    { "dvt_fs_opendir_unrecognized",      1, do_cleanup, connector_file_system_opendir, return_unrecognized},
    { "dvt_fs_opendir_reterror",          1, do_cleanup, connector_file_system_opendir, return_error},
    { "dvt_fs_opendir_retinvalid",        1, do_cleanup, connector_file_system_opendir, return_invalid_code},
    { "dvt_fs_opendir_null_handle",       1, do_cleanup, connector_file_system_opendir, set_null_handle},
    { "dvt_fs_opendir_user_cancel",       1, do_cleanup, connector_file_system_opendir, set_user_cancel},
    { "dvt_fs_opendir_io_error",          1, do_cleanup, connector_file_system_opendir, set_io_error},
    { "dvt_fs_opendir_io_error_no_errno", 1, do_cleanup, connector_file_system_opendir, set_io_error_no_errno},
    { "dvt_fs_opendir_bad_length",        1, do_cleanup, connector_file_system_opendir, set_bad_resp_length},
    { "dvt_fs_opendir_busy",              1, no_cleanup, connector_file_system_opendir, return_busy},    
 
    { "dvt_fs_readdir_abort",             1, no_cleanup, connector_file_system_readdir, return_abort},
    { "dvt_fs_readdir_abort2",            2, no_cleanup, connector_file_system_readdir, return_abort},
    { "dvt_fs_readdir_unrecognized",      1, no_cleanup, connector_file_system_readdir, return_unrecognized},
    { "dvt_fs_readdir_reterror",          1, no_cleanup, connector_file_system_readdir, return_error},
    { "dvt_fs_readdir_retinvalid",        1, no_cleanup, connector_file_system_readdir, return_invalid_code},
    { "dvt_fs_readdir_user_cancel",       1, no_cleanup, connector_file_system_readdir, set_user_cancel},
    { "dvt_fs_readdir_user_cancel2",      3, no_cleanup, connector_file_system_readdir, set_user_cancel},
    { "dvt_fs_readdir_io_error",          1, no_cleanup, connector_file_system_readdir, set_io_error},
    { "dvt_fs_readdir_io_error2",         3, no_cleanup, connector_file_system_readdir, set_io_error},
    { "dvt_fs_readdir_io_error_no_errno", 1, no_cleanup, connector_file_system_readdir, set_io_error_no_errno},
    { "dvt_fs_readdir_bad_length",        1, no_cleanup, connector_file_system_readdir, set_bad_resp_length},
    { "dvt_fs_readdir_busy",              1, no_cleanup, connector_file_system_readdir, return_busy},
    { "dvt_fs_readdir_busy2",             2, no_cleanup, connector_file_system_readdir, return_busy},
    { "dvt_fs_readdir_bad_datalength",    1, no_cleanup, connector_file_system_readdir, set_invalid_datalength},
    { "dvt_fs_readdir_zero_datalength",   1, no_cleanup, connector_file_system_readdir, set_zero_datalength},
                                             
    { "dvt_fs_stat3_abort",             3, no_cleanup, connector_file_system_stat, return_abort},
    { "dvt_fs_stat3_unrecognized",      3, no_cleanup, connector_file_system_stat, return_unrecognized},
    { "dvt_fs_stat3_reterror",          3, no_cleanup, connector_file_system_stat, return_error},
    { "dvt_fs_stat3_retinvalid",        3, no_cleanup, connector_file_system_stat, return_invalid_code},
    { "dvt_fs_stat3_user_cancel",       3, no_cleanup, connector_file_system_stat, set_user_cancel},
    { "dvt_fs_stat2_io_error",          2, no_cleanup, connector_file_system_stat, set_io_error},
    { "dvt_fs_stat2_io_error_no_errno", 2, no_cleanup, connector_file_system_stat, set_io_error_no_errno},
    { "dvt_fs_stat3_io_error",          3, no_cleanup, connector_file_system_stat, set_io_error},
    { "dvt_fs_stat3_io_error_no_errno", 3, no_cleanup, connector_file_system_stat, set_io_error_no_errno},
    { "dvt_fs_stat3_bad_length",        3, no_cleanup, connector_file_system_stat, set_bad_resp_length},
    { "dvt_fs_stat3_busy",              3, no_cleanup, connector_file_system_stat, return_busy},


    { "dvt_fs_hash_abort",             1, no_cleanup, connector_file_system_hash, return_abort},
    { "dvt_fs_hash_unrecognized",      1, no_cleanup, connector_file_system_hash, return_unrecognized},
    { "dvt_fs_hash_reterror",          1, no_cleanup, connector_file_system_hash, return_error},
    { "dvt_fs_hash_retinvalid",        1, no_cleanup, connector_file_system_hash, return_invalid_code},
    { "dvt_fs_hash_user_cancel",       1, no_cleanup, connector_file_system_hash, set_user_cancel},
    { "dvt_fs_hash_io_error",          2, no_cleanup, connector_file_system_hash, set_io_error},
    { "dvt_fs_hash_io_error_no_errno", 2, no_cleanup, connector_file_system_hash, set_io_error_no_errno},
    { "dvt_fs_hash_bad_length",        1, no_cleanup, connector_file_system_hash, set_bad_resp_length},
    { "dvt_fs_hash_busy",              1, no_cleanup, connector_file_system_hash, return_busy},
    { "dvt_fs_hash_bad_datalength",    1, no_cleanup, connector_file_system_hash, set_invalid_datalength},

    { "dvt_fs_hash_file_abort",             1, do_cleanup, connector_file_system_hash, return_abort},
    { "dvt_fs_hash_file_unrecognized",      1, do_cleanup, connector_file_system_hash, return_unrecognized},
    { "dvt_fs_hash_file_reterror",          1, do_cleanup, connector_file_system_hash, return_error},
    { "dvt_fs_hash_file_retinvalid",        1, do_cleanup, connector_file_system_hash, return_invalid_code},
    { "dvt_fs_hash_file_user_cancel",       1, do_cleanup, connector_file_system_hash, set_user_cancel},
    { "dvt_fs_hash_file_io_error",          1, do_cleanup, connector_file_system_hash, set_io_error},
    { "dvt_fs_hash_file_io_error_no_errno", 1, do_cleanup, connector_file_system_hash, set_io_error_no_errno},
    { "dvt_fs_hash_file_bad_length",        1, do_cleanup, connector_file_system_hash, set_bad_resp_length},
    { "dvt_fs_hash_file_busy",              1, no_cleanup, connector_file_system_hash, return_busy},
    { "dvt_fs_hash_file_bad_datalength",    1, do_cleanup, connector_file_system_hash, set_invalid_datalength},

    { "dvt_fs_strerr_abort",            2, do_cleanup, connector_file_system_strerror, return_abort},
    { "dvt_fs_strerr_bad_datalen",      2, do_cleanup, connector_file_system_strerror, set_invalid_datalength},
    { "dvt_fs_strerr_unrecognized",     2, do_cleanup, connector_file_system_strerror, return_unrecognized},
    { "dvt_fs_strerr_retinvalid",       2, do_cleanup, connector_file_system_strerror, return_invalid_code},
    { "dvt_fs_strerr_bad_length",       2, do_cleanup, connector_file_system_strerror, set_bad_resp_length},
    { "dvt_fs_strerr_zero_datalen",     2, do_cleanup, connector_file_system_strerror, set_zero_datalength},

    { "dvt_fs_closedir_abort",             1, do_cleanup, connector_file_system_closedir, return_abort},
    { "dvt_fs_closedir_busy",              1, no_cleanup, connector_file_system_closedir, return_busy},
    { "dvt_fs_closedir_unrecognized",      1, do_cleanup, connector_file_system_closedir, return_unrecognized},
    { "dvt_fs_closedir_reterror",          1, do_cleanup, connector_file_system_closedir, return_error},
    { "dvt_fs_closedir_retinvalid",        1, do_cleanup, connector_file_system_closedir, return_invalid_code},
    { "dvt_fs_closedir_user_cancel",       1, do_cleanup, connector_file_system_closedir, set_user_cancel},
    { "dvt_fs_closedir_io_error",          1, do_cleanup, connector_file_system_closedir, set_io_error},
    { "dvt_fs_closedir_io_error_no_errno", 1, do_cleanup, connector_file_system_closedir, set_io_error_no_errno},
    { "dvt_fs_closedir_bad_length",        1, do_cleanup, connector_file_system_closedir, set_bad_resp_length},

    { "dvt_fs_rm_abort",             1, do_cleanup, connector_file_system_rm, return_abort},
    { "dvt_fs_rm_busy",              1, no_cleanup, connector_file_system_rm, return_busy},
    { "dvt_fs_rm_unrecognized",      1, do_cleanup, connector_file_system_rm, return_unrecognized},
    { "dvt_fs_rm_reterror",          1, do_cleanup, connector_file_system_rm, return_error},
    { "dvt_fs_rm_retinvalid",        1, do_cleanup, connector_file_system_rm, return_invalid_code},
    { "dvt_fs_rm_user_cancel",       1, do_cleanup, connector_file_system_rm, set_user_cancel},
    { "dvt_fs_rm_io_error",          1, do_cleanup, connector_file_system_rm, set_io_error},
    { "dvt_fs_rm_io_error_no_errno", 1, do_cleanup, connector_file_system_rm, set_io_error_no_errno},
    { "dvt_fs_rm_bad_length",        1, do_cleanup, connector_file_system_rm, set_bad_resp_length},

    { "dvt_fs_truncate_abort",             1, no_cleanup, connector_file_system_ftruncate, return_abort},
    { "dvt_fs_truncate_busy",              1, no_cleanup, connector_file_system_ftruncate, return_busy},
    { "dvt_fs_truncate_unrecognized",      1, no_cleanup, connector_file_system_ftruncate, return_unrecognized},
    { "dvt_fs_truncate_reterror",          1, no_cleanup, connector_file_system_ftruncate, return_error},
    { "dvt_fs_truncate_retinvalid",        1, no_cleanup, connector_file_system_ftruncate, return_invalid_code},
    { "dvt_fs_truncate_user_cancel",       1, no_cleanup, connector_file_system_ftruncate, set_user_cancel},
    { "dvt_fs_truncate_io_error",          1, no_cleanup, connector_file_system_ftruncate, set_io_error},
    { "dvt_fs_truncate_io_error_no_errno", 1, no_cleanup, connector_file_system_ftruncate, set_io_error_no_errno},
    { "dvt_fs_truncate_bad_length",        1, no_cleanup, connector_file_system_ftruncate, set_bad_resp_length},

};

typedef enum
{
    dir_test_type,
    file_ls_test_type,
    rm_test_type,
    truncate_test_type
} dvt_test_t;

static struct 
{
    char filename[256];
    int  callback_cnt;
    int  new_test;
    dvt_cleanup_type_t cleanup;
    dvt_test_t test_type;
    
} dvt_data = {"", 0, 1, 0};

static void dvt_init_test(char const * name)
{
    strcpy(dvt_data.filename, name);

    dvt_data.new_test = 0;
    dvt_data.callback_cnt = 0;
    dvt_data.cleanup = no_cleanup;
    dvt_data.test_type = dir_test_type;
    if (strstr(name, "dvt_fs_hash_file")) 
       dvt_data.test_type = file_ls_test_type;
    else
    if (strstr(name, "dvt_fs_rm")) 
       dvt_data.test_type = rm_test_type;
    else
    if (strstr(name, "dvt_fs_truncate")) 
       dvt_data.test_type = truncate_test_type;

    APP_DEBUG("dvt_init_test: %s\n", name);
}

static void dvt_cleanup_test(void)
{
    dvt_data.new_test = 1;
    dvt_data.cleanup = no_cleanup;
    dvt_data.test_type = dir_test_type;
    APP_DEBUG("dvt_cleanup_test\n");
}


static test_action_t dvt_find_test(connector_file_system_request_t const request_id,
                                   void const * const request_data)
{

    test_action_t action = action_none;
    unsigned int i;

    if (dvt_data.new_test && request_id == connector_file_system_open)
    {
        connector_file_stat_request_t const * file_request = (connector_file_stat_request_t  *)  request_data;
        char const * ptr;

        ptr = strstr(file_request->path, "dvt_fs_stat");
        if (ptr == NULL)
            ptr = strstr(file_request->path, "dvt_fs_opendir");
        if (ptr == NULL)
            ptr = strstr(file_request->path, "dvt_fs_readdir");
        if (ptr == NULL)
            ptr = strstr(file_request->path, "dvt_fs_hash");
        if (ptr == NULL)
            ptr = strstr(file_request->path, "dvt_fs_strerr");
        if (ptr == NULL)
            ptr = strstr(file_request->path, "dvt_fs_closedir");
        if (ptr == NULL)
            ptr = strstr(file_request->path, "dvt_fs_rm");
        if (ptr == NULL)
            ptr = strstr(file_request->path, "dvt_fs_truncate");

        if (ptr == NULL)
            goto done;

        dvt_init_test(ptr);
    }

    for (i = 0; i < sizeof test_table / sizeof test_table[0]; i++) 
    {
        if ((test_table[i].callback_id == request_id) && 
            strcmp(dvt_data.filename, test_table[i].path) == 0) 
        {
            dvt_data.callback_cnt++;
            if (dvt_data.callback_cnt == test_table[i].callback_cnt)
            {
                action = test_table[i].action;
                dvt_data.cleanup = test_table[i].cleanup;
            }
            APP_DEBUG("dvt_find_test,  request_id %d, action %d, callback_cnt %d\n", request_id, action, dvt_data.callback_cnt);
            break;
        }
    }

done:
    return action;
}

static int dvt_pre_test(connector_file_system_request_t const request_id,
                        void const * const request_data, 
                        void * const response_data, 
                        size_t * const response_length,
                        connector_callback_status_t * status)
{
    const size_t MAX_DATA_LENGTH= 20000;
    connector_file_response_t * file_response = (connector_file_response_t *) response_data;
    int result = -1;

    test_action_t action = dvt_find_test(request_id, request_data);

    switch (action)
    {
        case return_abort:
            *status = connector_callback_abort;
            break;

        case return_busy:
            *status = connector_callback_busy;
            break;
 
        case return_unrecognized:
            *status = connector_callback_unrecognized;
            break;

        case return_error:
            *status = connector_callback_error;
            break;

        case return_invalid_code:
            *status = (connector_callback_status_t) -1;
            break;

        case set_user_cancel:
            file_response->error->error_status = connector_file_user_cancel;
            break;

        case set_io_error:
            file_response->error->error_status = connector_file_permision_denied;
            file_response->error->errnum = (void *) EACCES;
            break;

        case set_io_error_no_errno:
            file_response->error->error_status = connector_file_permision_denied;
            file_response->error->errnum = NULL;
            break;

        case set_bad_resp_length:
            (*response_length)--;
            break;

        case set_null_handle:
             break;

    case set_zero_datalength:
        {
            connector_file_data_response_t * const response = (connector_file_data_response_t *)response_data;
            response->size_in_bytes = 0;
            break;
        }

        case set_invalid_datalength:
        {
            connector_file_data_response_t * const response = (connector_file_data_response_t *)response_data;
            response->size_in_bytes = MAX_DATA_LENGTH;
            break;
        }


        case set_file_flags_zero:
        {
            connector_file_stat_response_t *resp = (connector_file_stat_response_t *)response_data;
            connector_file_stat_request_t const * request = (connector_file_stat_request_t const *)request_data;
            connector_file_stat_t * pstat = &resp->statbuf;
            pstat->flags = 0;
            pstat->file_size = 0;
            pstat->last_modified = 0;
            pstat->hash_alg = request->hash_alg;
            pstat->flags = 0;
            break;
        }

    case set_invalid_data:
        if (request_id == connector_file_system_stat)
        {
            connector_file_stat_response_t *resp = (connector_file_stat_response_t *)response_data;
            connector_file_stat_t * pstat = &resp->statbuf;
            pstat->flags = 0;
            pstat->file_size = 0;
            pstat->last_modified = 0;
            pstat->hash_alg = 100;
            pstat->flags = 0;
         }
         else
             result = 0;
        break;       

        case action_none:
        default:
            result = 0;
    }

    if (dvt_data.cleanup == do_cleanup)
    {
        dvt_cleanup_test();
    }


    return result;
}


connector_callback_status_t app_file_system_handler(connector_file_system_request_t const request,
                                                void const * const request_data, size_t const request_length,
                                                void * const response_data, size_t * const response_length)
{
    connector_callback_status_t status = connector_callback_continue;

    UNUSED_ARGUMENT(request_length);
    UNUSED_ARGUMENT(response_length);

    if (dvt_pre_test(request, request_data, response_data, response_length, &status) != 0)
        goto done;

    switch (request)
    {
        case connector_file_system_open:
            status = app_process_file_open(request_data, response_data);
            break;

        case connector_file_system_read:
            status = app_process_file_read(request_data, response_data);
            break;

        case connector_file_system_write:
            status = app_process_file_write(request_data, response_data);
            break;

        case connector_file_system_lseek:
            status = app_process_file_lseek(request_data, response_data);
            break;

        case connector_file_system_close:
            status = app_process_file_close(request_data, response_data);
            if (dvt_data.test_type == truncate_test_type && dvt_data.new_test == 0 && 
                status != connector_callback_busy) 
            {
                dvt_cleanup_test();
            }
            break;

        case connector_file_system_ftruncate:
            status = app_process_file_ftruncate(request_data, response_data);
            break;

        case connector_file_system_rm:
            status = app_process_file_rm(request_data, response_data);
            if (dvt_data.test_type == rm_test_type && dvt_data.new_test == 0 && 
                status != connector_callback_busy) 
            {
                dvt_cleanup_test();
            }
            break;

        case connector_file_system_stat:
            status = app_process_file_stat(request_data, response_data);
            break;

        case connector_file_system_opendir:
            status = app_process_file_opendir(request_data, response_data);
            break;

        case connector_file_system_readdir:
            status = app_process_file_readdir(request_data, response_data);
            break;

        case connector_file_system_closedir:
            status = app_process_file_closedir(request_data, response_data);
            if (dvt_data.test_type == dir_test_type && dvt_data.new_test == 0 && 
                status != connector_callback_busy) 
            {
                dvt_cleanup_test();
            }
            break;

        case connector_file_system_strerror:
            status = app_process_file_strerror(response_data);
            break;

        case connector_file_system_hash:
            status = app_process_file_hash(request_data, response_data);
            if (dvt_data.test_type == file_ls_test_type && dvt_data.new_test == 0 && 
                status != connector_callback_busy) 
            {
                dvt_cleanup_test();
            }
            break;

        case connector_file_system_msg_error:
            status = app_process_file_msg_error(request_data, response_data);
            break;

        default:
            APP_DEBUG("Unsupported file system request %d\n", request);
    }

done:
    return status;
}

