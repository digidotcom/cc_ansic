/*! @page file_system File System
 *
 * @htmlinclude nav.html
 *
 * @section file_system_overview1 File System
 *
 *  -# @ref file_system_overview
 *  -# @ref file_system_context
 *  -# @ref file_system_term
 *  -# @ref file_system_open
 *  -# @ref file_system_lseek
 *  -# @ref file_system_read
 *  -# @ref file_system_write
 *  -# @ref file_system_ftruncate
 *  -# @ref file_system_close
 *  -# @ref file_system_rm
 *  -# @ref file_system_opendir
 *  -# @ref file_system_readdir
 *  -# @ref file_system_closedir
 *  -# @ref file_system_stat
 *  -# @ref file_system_hash
 *  -# @ref file_system_strerror
 *  -# @ref file_system_msg_error
 *
 * @section file_system_overview Overview
 *
 * The file system facility is an optional facility for applications to access files on the device 
 * remotely from the iDigi Device Cloud. The iDigi connector invokes the application-defined callbacks  
 * to read from a file, to write to a file, and to list files or directory entries.
 *
 * The largest allowed file to get from the device is 2MB - 1byte (2097151 bytes). 
 *
 * A typical application-defined callback sequence for reading file data by the iDigi Device Cloud would include:
 *  -# iDigi connector calls application-defined @ref file_system_open "connector_file_system_open" callback with read-only access.
 *  -# iDigi connector calls application-defined @ref file_system_read "connector_file_system_read" callback number of times, until
 *     the requested data amount is retrieved or the end of the file is reached.
 *  -# iDigi connector calls application-defined @ref file_system_close "connector_file_system_close" callback.
 *
 * A typical application-defined callback sequence for writing file data by the iDigi Device Cloud would include:
 *  -# iDigi connector calls application-defined @ref file_system_open "connector_file_system_open" callback with write-create access.
 *  -# iDigi connector calls application-defined @ref file_system_write "connector_file_system_write" callback number of times, untill all data,
 *     received from the iDigi Device Cloud, is written to the file.
 *  -# iDigi connector calls application-defined @ref file_system_close "connector_file_system_close" callback.
 *
 * In order to remove a file iDigi connector calls application-defined @ref file_system_rm "connector_file_system_rm" callback.
 *
 * A typical application-defined callback sequence to get listing for a single file would be:
 *  -# iDigi connector calls application-defined @ref file_system_stat "connector_file_system_stat" callback. 
 *  -# iDigi connector calls application-defined @ref file_system_hash "connector_file_system_hash" callback, if the requested hash value is supported.
 *
 * A typical application-defined callback sequence to get a directory listing would be:
 *  -# iDigi connector calls application-defined @ref file_system_stat "connector_file_system_stat" callback and learns that the path is a directory.
 *  -# iDigi connector calls application-defined @ref file_system_opendir "connector_file_system_opendir" callback.
 *  -# For each directory entry iDigi connector invokes application-defined callbacks:
 *      -# @ref file_system_readdir "connector_file_system_readdir" callback.
 *      -# @ref file_system_stat "connector_file_system_stat" callback.
 *      -# @ref file_system_hash "connector_file_system_hash" callback, if the requested hash value is supported.
 *  -# When all directory entries are processed, iDigi connector calls application-defined @ref file_system_closedir "connector_file_system_closedir" callback.
 *
 * @note See @ref file_system_support under Configuration to enable or disable file system.
 *
 * @section file_system_context Session Context
 *
 * All file system response structures have the <b><i>user_context</i></b> field. This field is provided to 
 * the application to identify the session and store session data between callbacks. 
 *
 * All application session memory must be released in the last callback of the session, typically 
 * @ref file_system_close "connector_file_system_close" or @ref file_system_closedir "connector_file_system_closedir" callbacks. This callback will be invoked if the file or 
 * directory was opened successfully, even if the session had an error or was canceled by the user.
 *
 * @section file_system_term Session Termination and Error Processing
 * 
 * All file system response sctuctures have the pointer to @ref connector_file_error_data_t "connector_file_error_data_t" structure, 
 * which contains:
 * @li error_status - error status of @ref connector_file_error_status_t "connector_file_error_status_t" type. 
 * @li errnum - Application defined error token.
 * 
 * The callback should use:
 * @li @ref connector_file_noerror status - in case of successful file operation.
 * @li @ref connector_file_user_cancel status - to cancel the session. 
 * @li Any other error status - to send the File System error message to the iDigi Device Cloud.
 *
 * The callback can set errnum to the system errno value. It will be used later in the @ref file_system_strerror "connector_file_system_strerror" callback
 * to send the error string to the iDigi Device Cloud. 
 *
 * Different scenarios for the session termination are described below.
 *
 * If the session is successful:
 *  -# iDigi connector calls @ref file_system_close "connector_file_system_close" or @ref file_system_closedir "connector_file_system_closedir" callback, 
 *     if there is an open file or directory.
 *  -# iDigi connector sends the last response to the iDigi Device Cloud.
 *
 * The callback aborts iDigi connector:
 *  -# The callback returns @ref connector_callback_abort status.
 *  -# iDigi connector calls @ref file_system_close "connector_file_system_close" or @ref file_system_closedir "connector_file_system_closedir" callback, 
 *     if there is an open file or directory.
 *  -# iDigi connector is aborted.
 *
 * The callback cancels the file system session:
 *  -# The callback returns @ref connector_callback_continue and sets error_status to @ref connector_file_user_cancel.
 *  -# iDigi connector calls @ref file_system_close "connector_file_system_close" or @ref file_system_closedir "connector_file_system_closedir" callback, 
 *     if there is an open file or directory.
 *  -# iDigi connector canceles the session.
 *
 * The callback encounters a file I/O error:
 *  -# The callback returns @ref connector_callback_continue and sets error status and errnum.
 *  -# iDigi connector calls @ref file_system_close "connector_file_system_close" or @ref file_system_closedir "connector_file_system_closedir" callback, 
 *     if there is an open file or directory.
 *  -# If iDigi connector has already sent part of file or directory data, it cancels the session. This is due to the fact 
 *     that it can't differentiate an error response from part of the data response.
 *  -# Otherwise iDigi connector calls @ref file_system_strerror "connector_file_system_strerror" callback and sends an error response to the iDigi Device Cloud.
 *
 * File system was notified of an error in the messaging layer:
 *  -# iDigi connector calls @ref file_system_close "connector_file_system_close" or @ref file_system_closedir "connector_file_system_closedir" callback, 
 *     if there is an open file or directory.
 *  -# iDigi connector calls @ref file_system_msg_error "connector_file_system_msg_error" callback.
 *  -# iDigi connector canceles the session.
 *
 * @section file_system_open Open a file
 *
 * Open a file for the specified path. 
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_open @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> [IN] pointer to @endhtmlonly @ref connector_file_open_request_t "connector_file_open_request_t" @htmlonly structure where:
 *     <ul>
 *       <li><b><i>path</i></b> is the file path to a null-terminated string (with maximum string length of @endhtmlonly @ref CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH @htmlonly).</li>
 *       <li><b><i>oflag</i></b> is the bitwise-inclusive OR of @endhtmlonly @ref file_open_flag_t @htmlonly.</li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Size of @endhtmlonly @ref connector_file_open_request_t "connector_file_open_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td> [OUT] pointer to @endhtmlonly @ref connector_file_open_response_t "connector_file_open_response_t" @htmlonly structure:
 *     <ul>
 *       <li><b><i>user_context</i></b> - Application-owned pointer.</li>
 *       <li><b><i>error</i></b> - Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                  used in case of file I/O error.</li>
 *       <li><b><i>handle</i></b> - Application defined file handle.</li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_open_response_t "connector_file_open_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>File opened successfully or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_open(connector_file_open_request_t const * const request_data, 
 *                                               connector_file_open_response_t * const response_data)
 * {
 *    connector_callback_status_t status = connector_callback_continue;
 *    int oflag = 0; 
 *    long int fd;
 *
 *    if (request_data->oflag & CONNECTOR_O_RDONLY) oflag |= O_RDONLY;
 *    if (request_data->oflag & CONNECTOR_O_WRONLY) oflag |= O_WRONLY;
 *    if (request_data->oflag & CONNECTOR_O_RDWR)   oflag |= O_RDWR;
 *    if (request_data->oflag & CONNECTOR_O_APPEND) oflag |= O_APPEND;
 *    if (request_data->oflag & CONNECTOR_O_CREAT)  oflag |= O_CREAT;
 *    if (request_data->oflag & CONNECTOR_O_TRUNC)  oflag |= O_TRUNC;
 *
 *    // 0664 = read,write owner + read,write group + read others
 *    fd = open(request_data->path, oflag, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH); 
 *
 *    if (fd < 0)
 *    {
 *        connector_file_error_data_t * error_data = response_data->error;
 *
 *        long int errnum = errno;
 *        error_data->errnum = (void *) errnum;
 *        
 *        switch (errnum)
 *        {
 *            case EACCES:
 *            case EPERM:
 *                error_data->error_status = connector_file_permision_denied;
 *                break;
 *            case ENOMEM:
 *                error_data->error_status = connector_file_out_of_memory;
 *                break;
 *            case ENOENT:
 *            case EISDIR:
 *            case EBADF:
 *                error_data->error_status = connector_file_path_not_found;
 *                break;
 *            case EINVAL:
 *                error_data->error_status = connector_file_invalid_parameter;
 *                break;
 *            case EAGAIN:
 *                status = connector_callback_busy;
 *                break;
 *            case ENOSPC:
 *            error_data->error_status = connector_file_insufficient_storage_space;
 *                break;
 *            default:
 *                error_data->error_status = connector_file_unspec_error;
 *        }
 *    }
 *    response_data->handle = (void *) fd;
 *
 *    return status;
 * }
 * @endcode
 *
 * @section file_system_lseek   Seek file position
 *
 * Set the file offset for an open file.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_lseek @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_lseek_request_t "connector_file_lseek_request_t" @htmlonly structure:
 *   <ul>
 *   <li><b><i>handle</i></b> - File handle.</li>
 *   <li><b><i>offset</i></b> - Offset in bytes relative to origin.</li>
 *   <li><b><i>origin</i></b> - CONNECTOR_SEEK_CUR, CONNECTOR_SEEK_SET, or CONNECTOR_SEEK_END, see
 *                             @endhtmlonly @ref connector_file_seek_origin_t @htmlonly</li>
 *   </ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_lseek_request_t "connector_file_lseek_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_lseek_response_t "connector_file_lseek_response_t" @htmlonly structure:
 *   <ul>
 *   <li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly 
 *                                   structure, used in case of file I/O error.</li>
 *   <li><b><i>offset</i></b> - [OUT] Resulting file offset.</li>
 *   </ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_lseek_response_t "connector_file_lseek_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>File offset set successfully or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly 
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_lseek(connector_file_lseek_request_t const * const request_data, 
 *                                                connector_file_lseek_response_t * const response_data)
 * {
 *    connector_callback_status_t status = connector_callback_continue;
 *    int origin;
 *    long int offset;
 *
 *    switch (request_data->origin)
 *    {
 *        case CONNECTOR_SEEK_SET:
 *           origin = SEEK_SET;
 *           break;
 *        case CONNECTOR_SEEK_END:
 *           origin = SEEK_END;
 *           break;
 *        case CONNECTOR_SEEK_CUR:
 *        default:
 *           origin = SEEK_CUR;
 *           break;
 *    }
 *
 *    offset = lseek((long int) request_data->handle, request_data->offset, origin);
 *
 *    response_data->offset = offset;
 *
 *    if (offset < 0)
 *    {
 *        connector_file_error_data_t * error_data = response_data->error;
 *
 *        long int errnum = errno;
 *        error_data->errnum = (void *) errnum;
 *        
 *        switch (errnum)
 *        {
 *            case EBADF:
 *                error_data->error_status = connector_file_path_not_found;
 *                break;
 *            case EINVAL:
 *                error_data->error_status = connector_file_invalid_parameter;
 *                break;
 *            case ENOSPC:
 *            case EFBIG:
 *                error_data->error_status = connector_file_insufficient_storage_space;
 *                break;
 *            case EAGAIN:
 *                status = connector_callback_busy;
 *                break;
 *            default:
 *                error_data->error_status = connector_file_unspec_error;
 *        }
 *    }
 *    return status;
 * }
 *
 * @endcode
 *
 * @section file_system_read    Read file data
 *
 * Read data from an open file.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_read @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_request_t "connector_file_request_t" @htmlonly structure:
 *  <ul>
 *   <li><b><i>handle</i></b> - File handle.</li>
 *   </ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_request_t "connector_file_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_data_response_t "connector_file_data_response_t" @htmlonly structure:
 *   <ul>
 *   <li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                   used in case of file I/O error.</li>
 *   <li><b><i>data_ptr</i></b> - Pointer to memory where the callback writes data.</li>
 *   <li><b><i>size_in_bytes</i></b> - [IN] Size of the memory buffer. [OUT] Number of bytes
 *                                    retrieved from the file and written to the memory buffer.</li>
 *   </ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_data_response_t "connector_file_data_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Reading from a file succeded or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly 
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_read(connector_file_request_t const * const request_data, 
 *                                               connector_file_data_response_t * const response_data)
 * {
 *    connector_callback_status_t status = connector_callback_continue;
 * 
 *    int result = read((long int) request_data->handle, response_data->data_ptr, response_data->size_in_bytes);
 *
 *    if (result >= 0)
 *    {
 *        response_data->size_in_bytes = result;
 *    }
 *    else
 *    {
 *        connector_file_error_data_t * error_data = response_data->error;
 *
 *        long int errnum = errno;
 *        error_data->errnum = (void *) errnum;
 *            
 *        switch (errnum)
 *        {
 *            case EBADF:
 *                error_data->error_status = connector_file_path_not_found;
 *                break;
 *            case EINVAL:
 *                error_data->error_status = connector_file_invalid_parameter;
 *                break;
 *            case EAGAIN:
 *                status = connector_callback_busy;
 *                break;
 *            default:
 *                error_data->error_status = connector_file_unspec_error;
 *        }
 *    }
 *    return status;
 * } 
 *
 * @endcode
 *
 * @section file_system_write   Write file data
 *
 * Write data to an open file.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_write @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_write_request_t "connector_file_write_request_t" @htmlonly structure:
 *   <ul>
 *   <li><b><i>handle</i></b> - File handle.</li>
 *   <li><b><i>data_ptr</i></b> - Pointer to data to write to the file.</li>
 *   <li><b><i>size_in_bytes</i></b> - Number of data bytes to write to the file.</li>
 *   </ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_write_request_t "connector_file_write_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_write_response_t "connector_file_write_response_t" @htmlonly structure:
 *   <ul>
 *   <li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                             used in case of file I/O error.</li>
 *   <li><b><i>size_in_bytes</i></b> - [OUT] Number of bytes actually written to the file.</li>
 *   </ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_write_response_t "connector_file_error_data_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Writing to a file succeded or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly 
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_write(connector_file_write_request_t const * const request_data, 
 *                                                connector_file_write_response_t * const response_data)
 * {
 *    connector_callback_status_t status = connector_callback_continue;
 * 
 *    int result = write((long int) request_data->handle, request_data->data_ptr, request_data->size_in_bytes);
 * 
 *    if (result >= 0)
 *    {
 *        response_data->size_in_bytes = result;
 *    }
 *    else
 *    if (result < 0)
 *    {
 *        connector_file_error_data_t * error_data = response_data->error;
 *
 *        long int errnum = errno;
 *        error_data->errnum = (void *) errnum;
 *            
 *        switch (errnum)
 *        {
 *            case EBADF:
 *                error_data->error_status = connector_file_path_not_found;
 *                break;
 *            case EINVAL:
 *                error_data->error_status = connector_file_invalid_parameter;
 *                break;
 *            case EAGAIN:
 *                status = connector_callback_busy;
 *                break;
 *            default:
 *                error_data->error_status = connector_file_unspec_error;
 *        }
 *    }
 *    return status;
 * } 
 *
 * @endcode
 *
 * @section file_system_ftruncate   Truncate a file
 *
 *
 * Truncate an open file to a specified length.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_ftruncate @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_ftruncate_request_t "connector_file_ftruncate_request_t" @htmlonly structure:
 *   <ul>
 *   <li><b><i>handle</i></b> - File handle.</li>
 *   <li><b><i>length</i></b> - Length in bytes to truncate a file to.</li>
 *   </ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_ftruncate_request_t "connector_file_ftruncate_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure:
 *   <ul>
 *   <li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                   used in case of file I/O error.</li>
 *   </ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>File truncated successfully or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly 
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_ftruncate(connector_file_ftruncate_request_t const * const request_data, 
 *                                                     connector_file_response_t * const response_data)
 * {
 *    connector_callback_status_t status = connector_callback_continue;
 * 
 *    int result = ftruncate((long int) request_data->handle, request_data->length);
 * 
 *    if (result < 0)
 *    {
 *        connector_file_error_data_t * error_data = response_data->error;
 *
 *        long int errnum = errno;
 *        error_data->errnum = (void *) errnum;
 *            
 *        switch (errnum)
 *        {
 *            case EBADF:
 *            case EINVAL:
 *            case EFBIG:
 *                error_data->error_status = connector_file_invalid_parameter;
 *                break;
 *            case EAGAIN:
 *                status = connector_callback_busy;
 *                break;
 *            default:
 *                error_data->error_status = connector_file_unspec_error;
 *        }
 *    }
 *    return status;
 * } 
 *
 * @endcode
 *
 * @section file_system_close   Close a file
 *
 * Close an open file.
 *
 * @note This callback must release all resources, used during the file system session.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_close @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_request_t "connector_file_request_t" @htmlonly structure:
 *   <ul><li><b><i>handle</i></b> - File handle.</li></ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_request_t "connector_file_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                   used in case of file I/O error.</li></ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>File closed successfully or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly 
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_close(connector_file_request_t const * const request_data, 
 *                                                connector_file_response_t * const response_data)
 * {
 *
 *     int result = close((long int) request_data->handle);
 *
 *     if (result < 0 && errno == EIO)
 *     {
 *         connector_file_error_data_t * error_data = response_data->error;

 *         error_data->errnum = (void *) EIO;
 *         error_data->error_status = connector_file_unspec_error;
 *     }
 *
 *     // All session resources must be released in this callback
 *     return connector_callback_continue;
 *  } 
 *
 * @endcode
 *
 * @section file_system_rm      Remove a file
 *
 * Remove a file for the specified path. 
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_rm @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_path_request_t "connector_file_path_request_t" @htmlonly structure:
 *   <ul><li><b><i>path</i></b> - File path is a null-terminated string.
 *                          See @endhtmlonly @ref CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH @htmlonly.</li></ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_path_request_t "connector_file_path_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                   used in case of file I/O error.</li></ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>File removed successfully or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_rm(connector_file_path_request_t const * const request_data, 
 *                                             connector_file_response_t * const response_data)
 * {
 *    connector_callback_status_t status = connector_callback_continue;
 *
 *     // Posix function to remove a file
 *     int result = unlink(request_data->path);
 *
 *     if (result < 0)
 *     {
 *          connector_file_error_data_t * error_data = response_data->error;
 *
 *          long int errnum = errno;
 *          error_data->errnum = (void *) errnum;
 *
 *          switch (errnum)
 *          {
 *              case EACCES:
 *              case EPERM:
 *                  error_data->error_status = connector_file_permision_denied;
 *                  break;
 *              case ENOMEM:
 *                  error_data->error_status = connector_file_out_of_memory;
 *                  break;
 *              case ENOENT:
 *              case EBADF:
 *                  error_data->error_status = connector_file_path_not_found;
 *                  break;
 *              case EINVAL:
 *              case EISDIR:
 *                  error_data->error_status = connector_file_invalid_parameter;
 *                  break;
 *              case EAGAIN:
 *                  status = connector_callback_busy;
 *                  break;
 *              default:
 *                  error_data->error_status = connector_file_unspec_error;
 *          }
 *     }
 *     return status;
 *  } 
 *
 * @endcode
 *
 * @section file_system_opendir     Open a directory
 *
 * Open a directory for the spesified path.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_opendir @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_path_request_t "connector_file_path_request_t" @htmlonly structure:
 *   <ul><li><b><i>path</i></b> - File path is a null-terminated string.
 *                          See @endhtmlonly @ref CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH @htmlonly.</li></ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_path_request_t "connector_file_path_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [OUT] pointer to @endhtmlonly @ref connector_file_open_response_t "connector_file_open_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                                            used in case of an error.</li>
 *   <li><b><i>handle</i></b> - Application defined directory handle.</li></ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_open_response_t "connector_file_open_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Directory opened successfully or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * 
 * Example:
 *
 * @code
 *
 * typedef struct
 * {
 *    DIR         * dirp;
 *    struct dirent dir_entry;
 * 
 * } app_dir_data_t;
 *
 * connector_callback_status_t app_process_file_opendir(connector_file_path_request_t const * const request_data,
 *                                                  connector_file_open_response_t * const response_data)
 * {
 * 
 *     connector_callback_status_t status = connector_callback_continue;
 *     connector_file_error_data_t * error_data = response_data->error;
 *     DIR * dirp;
 * 
 *     errno = 0;
 *     dirp = opendir(request_data->path);
 * 
 *     if (dirp != NULL)
 *     {
 *         void * ptr;
 *         app_dir_data_t * dir_data = malloc(sizeof *dir_data);
 *
 *         if (dir_data != NULL)
 *         {
 *             response_data->handle = dir_data;
 *             dir_data->dirp = dirp;
 *         }
 *         else
 *         {
 *             error_data->error_status = connector_file_out_of_memory;
 *             error_data->errnum = (void *) ENOMEM; 
 *             closedir(dirp);
 *         }
 *     }
 *     else
 *     {
 *         long int errnum = errno;
 *         error_data->errnum = (void *) errnum;
 *
 *         switch (errnum)
 *         {
 *             case EACCES:
 *                  error_data->error_status = connector_file_permision_denied;
 *                  break;
 *              case ENOMEM:
 *                  error_data->error_status = connector_file_out_of_memory;
 *                  break;
 *              case ENOENT:
 *              case ENOTDIR:
 *                  error_data->error_status = connector_file_path_not_found;
 *                  break;
 *              case EINVAL:
 *                  error_data->error_status = connector_file_invalid_parameter;
 *                  break;
 *              case EAGAIN:
 *                  status = connector_callback_busy;
 *                  break;
 *              default:
 *                  error_data->error_status = connector_file_unspec_error;
 *         }
 *     }
 *     return status;
 * }
 *
 * @endcode
 *
 * @section file_system_readdir     Read next directory entry
 *
 * The callbacks reads the next directory entry for the directory handle,
 * returned in the @ref file_system_opendir "connector_file_system_opendir" callback. 
 *
 * The callback writes the directory entry name to memory at the data_ptr address. When no more 
 * directory entries exist, the callback sets size_in_bytes to 0.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_readdir @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_request_t "connector_file_request_t" @htmlonly structure:
 *   <ul><li><b><i>handle</i></b> - Directory handle.</li></ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_request_t "connector_file_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_data_response_t "connector_file_data_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                          used in case of an error.</li>
 *   <li><b><i>data_ptr</i></b> - Pointer to memory where the callback writes the directory entry name,
 *                               a null-terminated string.</li>
 *   <li><b><i>size_in_bytes</i></b> - [IN] Size of the memory buffer. [OUT] Length of the ditectory entry name,
 *                                      including ending '\0'. 0 if no more directory entries exist.</li></ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_data_response_t "connector_file_data_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Next directory entry returned, no more directory entries exist, or an error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * 
 * Example:
 *
 * @code
 *
 * typedef struct
 * {
 *    DIR         * dirp;
 *    struct dirent dir_entry;
 * 
 * } app_dir_data_t;
 *
 *
 * connector_callback_status_t app_process_file_readdir(connector_file_request_t const * const request_data,
 *                                                  connector_file_data_response_t * const response_data)
 * {
 *     connector_callback_status_t status = connector_callback_continue;
 *     app_dir_data_t * dir_data = request_data->handle;
 *     connector_file_error_data_t * error_data = response_data->error;
 *     struct dirent  * result;
 *     int rc;
 * 
 *     // This sample does not skip "." and ".."
 * 
 *     rc = readdir_r(dir_data->dirp, &dir_data->dir_entry, &result);
 * 
 *     if (rc != 0)
 *     {
 *         // error
 *         error_data->errnum = (void *) rc;
 *
 *         switch (rc)
 *         {
 *              case ENOENT:
 *              case EBADF:
 *                 error_data->error_status = connector_file_path_not_found;
 *                 break;
 *              case EAGAIN:
 *                 status = connector_callback_busy;
 *                 break;
 *              case EOVERFLOW:
 *              default:
 *                 error_data->error_status = connector_file_unspec_error;
 *         }
 *     }
 *     else
 *     if (result == NULL)
 *     {
 *         // finished with the directory
 *         response_data->size_in_bytes = 0;
 *     }
 *     else
 *     {
 *         // valid entry
 *         size_t name_len = strlen(result->d_name);
 *         size_t buffer_size = response_data->size_in_bytes;
 * 
 *         if(name_len < buffer_size)
 *         {
 *             memcpy(response_data->data_ptr, result->d_name, name_len + 1);
 *             response_data->size_in_bytes = name_len;
 *         }
 *         else
 *         {
 *             connector_file_error_data_t * error_data = response_data->error;
 * 
 *             error_data->error_status = connector_file_out_of_memory;
 *             error_data->errnum = (void *) ENAMETOOLONG;
 *         }
 *     }
 *     return status;
 * }
 *
 * @endcode
 *
 * @section file_system_closedir    Close a directory
 *
 * Close a directory for the directory handle, returned in the @ref file_system_opendir "connector_file_system_opendir"
 * callback.
 *
 * @note This callback must release all resources, used during the file system session.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_closedir @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_request_t "connector_file_request_t" @htmlonly structure:
 *   <ul><li><b><i>handle</i></b> - Directory handle.</li></ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_request_t "connector_file_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                      used in case of an error.</li></ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Directory closed successfully or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * 
 * Example:
 *
 * @code
 *
 * typedef struct
 * {
 *    DIR         * dirp;
 *    struct dirent dir_entry;
 * 
 * } app_dir_data_t;
 *
 *
 * connector_callback_status_t app_process_file_closedir(connector_file_request_t const * const request_data,
 *                                                   connector_file_response_t * const response_data)
 * {
 *     app_dir_data_t * dir_data = request_data->handle;
 *
 *     closedir(dir_data->dirp);
 *     free(dir_data);
 *
 *     // All session resources must be released in this callback
 *     return connector_callback_continue;
 * }
 * 
 * @endcode
 *
 * @section file_system_stat        Get file status
 *
 * Get file status for the named file path.
 *
 * When called for a file, the callback returns the following information in the @ref connector_file_stat_t "connector_file_stat_t"
 * structure of the response:
 * @li File size
 * @li Last modified time
 * @li The @ref CONNECTOR_FILE_IS_REG flag set, if the path represents a regular file.
 * @li Hash algorithm of @ref connector_file_hash_algorithm_t "connector_file_hash_algorithm_t" type to be used for this file in a future
 * @ref file_system_hash "connector_file_system_hash" callback.
 *
 * When called for a directory, the callback returns the following information in the @ref connector_file_stat_t "connector_file_stat_t"
 * structure of the response:
 * @li Last modified time
 * @li The @ref CONNECTOR_FILE_IS_DIR flag set.
 * @li Hash algorithm of @ref connector_file_hash_algorithm_t "connector_file_hash_algorithm_t" type to be used for each regular file in this directory
 * in a separate @ref file_system_hash "connector_file_system_hash" callback. 
 *
 * Hash values support is optional.
 *
 * The following table shows the valid response for a suggested hash algorithm:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 * <th class="title">Suggested hash algorithm</th>
 * <th class="title">Actual hash algorithm</th>
 * </tr>
 * <tr>
 * <th> @endhtmlonly @ref connector_file_hash_best @htmlonly</th>
 * <td> @endhtmlonly @ref connector_file_hash_md5 @htmlonly, @endhtmlonly @ref connector_file_hash_crc32 @htmlonly,
 *                                           or @endhtmlonly @ref connector_file_hash_none @htmlonly </td>
 * </tr>
 * <tr>
 * <th> @endhtmlonly @ref connector_file_hash_md5 @htmlonly</th>
 * <td> @endhtmlonly @ref connector_file_hash_md5 @htmlonly or @endhtmlonly @ref connector_file_hash_none @htmlonly</td>
 * </tr>
 * <tr>
 * <th> @endhtmlonly @ref connector_file_hash_crc32 @htmlonly</th>
 * <td> @endhtmlonly @ref connector_file_hash_crc32 @htmlonly or @endhtmlonly @ref connector_file_hash_none @htmlonly</td>
 * </tr>
 * <tr>
 * <th> @endhtmlonly @ref connector_file_hash_none @htmlonly</th>
 * <td> @endhtmlonly @ref connector_file_hash_none @htmlonly</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Callback arguments:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_stat @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_stat_request_t "connector_file_stat_request_t" @htmlonly structure:
 *   <ul><li><b><i>path</i></b> - File path is a null-terminated string.
 *                          See @endhtmlonly @ref CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH @htmlonly.</li>
 *   <li><b><i>hash_alg</i></b> - Suggested hash algorithm of @endhtmlonly @ref connector_file_hash_algorithm_t "connector_file_hash_algorithm_t" 
 *                                @htmlonly type.</li></ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_stat_request_t "connector_file_stat_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_stat_response_t "connector_file_stat_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                  used in case of file error.</li>
 *   <li><b><i>statbuf</i></b> - [OUT] @endhtmlonly @ref connector_file_stat_t "connector_file_stat_t" @htmlonly structure where
 *                            the callback writes file status data.
 *
 *      <ul>
 *      <li><b><i>last_modified</i></b> Last modified time (seconds since 1970). If not supported, use 0.</li>
 *      <li><b><i>file_size</i></b> File size in bytes.</li>
 *      <li><b><i>flags</i></b> Non-zero if the file is a regular file or a directory, see @endhtmlonly @ref file_stat_flag_t @htmlonly.</li>
 *      <li><b><i>hash_alg</i></b> Actual hash algorithm: @endhtmlonly @ref connector_file_hash_crc32 @htmlonly,
 *                  @endhtmlonly @ref connector_file_hash_md5 @htmlonly, or @endhtmlonly @ref connector_file_hash_none @htmlonly.</li>
 *      </ul></li></ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_stat_response_t "connector_file_stat_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Status information returned or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_stat(connector_file_stat_request_t const * const request_data, 
 *                                               connector_file_stat_response_t * const response_data)
 * {
 *     struct stat statbuf;
 *     connector_file_stat_t * pstat = &response_data->statbuf;
 *     connector_callback_status_t status = connector_callback_continue;
 * 
 *     int result = stat(request_data->path, &statbuf);
 * 
 *     if (result == 0)
 *     {
 *         pstat->flags         = 0;
 *         pstat->file_size     = statbuf.st_size;
 *         pstat->last_modified = statbuf.st_mtime;
 *         pstat->hash_alg      = connector_file_hash_none;
 * 
 *         if (S_ISDIR(statbuf.st_mode))
 *            pstat->flags |= CONNECTOR_FILE_IS_DIR;
 *         else
 *         if (S_ISREG(statbuf.st_mode))
 *            pstat->flags |= CONNECTOR_FILE_IS_REG;
 *     }
 *     else
 *     {
 *         connector_file_error_data_t * error_data = response_data->error;
 *         long int errnum = errno;
 *         error_data->errnum = (void *) errnum;
 *
 *         switch (errnum)
 *         {
 *          case EACCES:
 *              error_data->error_status = connector_file_permision_denied;
 *               break;
 *           case ENAMETOOLONG:
 *               error_data->error_status = connector_file_out_of_memory;
 *               break;
 *           case ENOENT:
 *               error_data->error_status = connector_file_path_not_found;
 *               break;
 *           case ENOTDIR:
 *           case EBADF:
 *           case EINVAL:
 *               error_data->error_status = connector_file_invalid_parameter;
 *               break;
 *           case EAGAIN:
 *               status = connector_callback_busy;
 *               break;
 *           default:
 *               error_data->error_status = connector_file_unspec_error;
 *         }
 *    }
 *    return status;
 * }
 *
 * @endcode
 *
 * @section file_system_hash        Get file hash value
 * 
 * Get a file hash value for the specified path.
 *
 * Supported hash algorithms are md5 and crc32.
 *
 * An application callback might need more than one pass to calculate the hash value.
 * It can use user_context to point to intermidiate data and return
 * @ref connector_callback_busy. The callback will be repeated until it completes hash
 * calculations and returns @ref connector_callback_continue.
 *
 * If @ref file_system_hash "connector_file_system_hash" callback for a directory entry has a problem reading a file
 * and sets an error status, the session will terminate without sending any data for other
 * directory entries. In order to avoid this the callback may elect to return a zero
 * hash value and a non-error status.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_hash @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_stat_request_t "connector_file_stat_request_t" @htmlonly structure:
 *   <ul><li><b><i>path</i></b> - File path is a null-terminated string.
 *                          See @endhtmlonly @ref CONNECTOR_FILE_SYSTEM_MAX_PATH_LENGTH @htmlonly.</li>
 *   <li><b><i>hash_alg</i></b> - Hash algorithm @endhtmlonly @ref connector_file_hash_md5 @htmlonly
 *                               or @endhtmlonly @ref connector_file_hash_crc32 @htmlonly.</li></ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_stat_request_t "connector_file_stat_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_data_response_t "connector_file_data_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - [IN/OUT] Application-owned pointer.</li>
 *   <li><b><i>error</i></b> - [OUT] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure,
 *                                  used in case of file error.</li>
 *   <li><b><i>data_ptr</i></b> - Pointer to  memory, where the callback writes the hash value.</li>
 *   <li><b><i>size_in_bytes</i></b> - [IN] Size of memory buffer for hash value: 16 bytes for md5,
 *                                4 bytes for crc32.</li></ul>
 * </td>
 * </tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_data_response_t "connector_file_data_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Status information returned or error has occured</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Busy. The callback will be repeated
 * </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_hash(connector_file_stat_request_t const * const request_data, 
 *                                               connector_file_data_response_t * const response_data)
 * {
 *     connector_callback_status_t status = connector_callback_continue;
 *
 *     switch (request_data->hash_alg)
 *     {
 *          case connector_file_hash_crc32:
 *              status = app_calc_crc32(request_data->path, response_data);
 *              break;
 *
 *          case connector_file_hash_md5:
 *              status = app_calc_md5(request_data->path, response_data);
 *              break;
 *     }
 *     return status;
 * }
 *
 * @endcode
 *
 * @section file_system_strerror    Get error description
 *
 * Get error description string to send to the iDigi Device Cloud.
 *
 * iDigi connector invokes this this callback if an earlier callback has encountered a file I/O eror and
 * has set an error_status and errnum in @ref connector_file_error_data_t "connector_file_error_data_t" structure of the response.
 *
 * iDigi connector invokes this callback after calling the @ref file_system_close "connector_file_system_close" 
 * or the @ref file_system_closedir "connector_file_system_closedir" callback.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_strerror @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th> <td> NULL </td>
 * </tr>
 * <tr>
 * <td>request_length</td> <td> 0 </td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN/OUT] pointer to @endhtmlonly @ref connector_file_data_response_t "connector_file_data_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - NULL.</li>
 *   <li><b><i>error</i></b> [IN] Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure.
 *      <ul><li><b><i>- error_status</i></b> [IN] Error status of @endhtmlonly @ref connector_file_error_status_t "connector_file_error_status_t" @htmlonly type.</li>
 *      <li><b><i>- errnum</i></b> [IN] Used as an input for an error description string.</li></ul></li>
 *   <li><b><i>data_ptr</i></b> Pointer to memory where the callback writes an error description.
 *                               The error description is not a null-terminated string.</li>
 *   <li><b><i>size_in_bytes</i></b> [IN] Size of the memory buffer.<br></br>
 *                                   [OUT] Length of the error description string.</li></ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_data_response_t "connector_file_data_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Continue</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * 
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_strerror(connector_file_response_t * const response_data)
 * {
 *    size_t strerr_size = 0;
 * 
 *    connector_file_error_data_t * error_data = response_data->error;
 *    long int errnum = (long int) error_data->errnum;
 * 
 *    char * err_str = strerror(errnum);
 *    char * ptr = response_data->data_ptr;
 *
 *    strerr_size = strnlen(err_str, response_data->size_in_bytes);
 *    memcpy(ptr, err_str, strerr_size);
 *    ptr[strerr_size - 1] = '\0';
 * 
 *    response_data->size_in_bytes = strerr_size;
 *
 *    return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section file_system_msg_error   Inform of an iDigi connector error
 *
 * An error in a file system session might be caused by network communication problems,
 * session timeout, insufficient memory, etc.
 *
 * iDigi connector will invoke the @ref file_system_close "connector_file_system_close" 
 * or the @ref file_system_closedir "connector_file_system_closedir" callback after this callback.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr> 
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>class_id</td>
 * <td>@endhtmlonly @ref connector_class_id_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <td>request_id</td>
 * <td>@endhtmlonly @ref connector_file_system_msg_error @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_error_request_t "connector_file_error_request_t" @htmlonly structure:
 *   <ul><li><b><i>message_status</i></b> - @endhtmlonly @ref connector_msg_error_t @htmlonly.</li></ul>
 * </td></tr>
 * <tr>
 * <td>request_length</td>
 * <td> [IN] Size of @endhtmlonly @ref connector_file_error_request_t "connector_file_error_request_t" @htmlonly structure</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> [IN] pointer to @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure:
 *   <ul><li><b><i>user_context</i></b> - Set by a user in an earlier callback.</li>
 *   <li><b><i>error</i></b> - Pointer to a @endhtmlonly @ref connector_file_error_data_t "connector_file_error_data_t" @htmlonly structure.</li></ul>
 * </td></tr>
 * <tr>
 * <td>response_length</td>
 * <td>[OUT] Size of @endhtmlonly @ref connector_file_response_t "connector_file_response_t" @htmlonly structure</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr> 
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Continue</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * 
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_process_file_msg_error(connector_file_error_request_t const * const request_data, 
 *                                                    connector_file_response_t * response_data)
 * {
 *     UNUSED_ARGUMENT(response_data);
 *
 *     APP_DEBUG("Message Error %d\n", request_data->message_status);
 *
 *    // All session resources must be released in this callback
 *    return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @htmlinclude terminate.html
 */
