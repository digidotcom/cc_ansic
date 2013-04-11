/*! @page api_overview API Overview
 *
 * @htmlinclude nav.html
 *
 * @section api_overview API
 *  -# @ref api1_overview
 *  -# @ref initialization
 *  -# @ref send_data
 *  -# @ref register_device_request_callbacks
 *  -# @ref device_request_callback
 *  -# @ref device_response_callback
 *  -# @ref device_download_callback
 *  -# @ref device_reset_callback
 *
 * @section api1_overview API Overview
 *
 * The iDigi Connector API is used for communication between the client running on the
 * device and the iDigi Device Cloud.  The device can be controlled by the iDigi Device
 * Cloud via <a href="http://www.idigi.com/idigimanagerpro">iDigi Manager Pro</a> or by
 * using @ref web_services "iDigi Web Services".  When commands are received they
 * come in the form of an iDigi device request, for each request the device will be
 * asked to send back a response.  Device requests can be used to send commands to control a device.
 * This API is also used to push files onto the iDigi device cloud.
 * 
 * The API consists of the following routines, click on the name of the routine for more
 * details.
 * 
 * @htmlonly
 * <table class="apitable">
 * <tr>
 * <th class="title">Routine</th>
 * <th class="title">Description</th>
 * </tr>
 * <tr>
 * <td>@endhtmlonly idigi_connector_start() @htmlonly</td>
 * <td>Start the iDigi connector</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly idigi_send_data() @htmlonly</td>
 * <td>Send data to the iDigi Device Cloud</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly idigi_register_device_request_callbacks() @htmlonly</td>
 * <td>Registers the device request callbacks</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref idigi_device_request_callback_t "device_request_callback()" @htmlonly</td>
 * <td>Handles the device request callback</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref idigi_device_response_callback_t "device_response_callback()" @htmlonly</td>
 * <td>Prepares the response to the device request</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * 
 * @subsection initialization Initialization
 * 
 * The function idigi_connector_start() is called during startup to initialize the iDigi connector.
 * The @ref idigi_status_callback_t "status callback function" is passed as an argument and is used to pass the application status messages from
 * the iDigi Device Cloud. The idigi_connector_start() function must be called before all
 * other iDigi Connector APIs.
 *
 * Example:
 *
 * @code
 * ret = idigi_connector_start(idigi_status);
 * if (ret != idigi_connector_success)
 * {
 *    printf("%s: idigi_connector_start failed [%d]\n", __FUNCTION__, ret);
 *    goto error;
 * }
 * @endcode
 *
 * @subsection send_data Send Data
 * 
 * The function idigi_send_data() is used to write data to files on the iDigi Server.  The file is specified in the
 * path argument passed into this function.  The flags field is used to specify how the data
 * is written to the file.  The flag definitions are listed below:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 * <th class="title">Flag</th>
 * <th class="title">Description</th>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref IDIGI_FLAG_OVERWRITE_DATA @htmlonly</td>
 * <td>Overwrite the file with the new data</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref IDIGI_FLAG_ARCHIVE_DATA @htmlonly</td>
 * <td>Keep a history of the changes to the file</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref IDIGI_FLAG_APPEND_DATA @htmlonly</td>
 * <td>Append new data to the end of the file</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * 
 * Example:
 * @code
 * ic_data.data_ptr         = sample;
 * ic_data.length_in_bytes  = strlen(sample);
 * ic_data.flags            = 0;
 *
 * do
 * {
 *    ret = idigi_send_data("dia_file.xml", &ic_data, "text/xml");
 *    if (ret != idigi_connector_success)
 *    {
 *        sleep(2);
 *    }
 * } while (ret != idigi_connector_success);
 * @endcode
 *
 * @subsection register_device_request_callbacks Register Callbacks
 *
 * The function idigi_register_device_request_callbacks() is called to register routines which are called when
 * requests are received from the server.  The table below lists the callbacks passed into this routine:

 * @htmlonly
 * <table class="apitable">
 * <tr>
 * <th class="title">Callback</th>
 * <th class="title">Description</th>
 * </tr>
 * <tr>
 * <td>request_callback</td>
 * <td>Called with device request data from the iDigi Device Cloud</td>
 * </tr>
 * <tr>
 * <td>response_callback</td>
 * <td>Called to get the response to the previous device request</td>
 * </tr>
 * <tr>
 * <td>download_callback</td>
 * <td>Called with firmware download data, this can be NULL if not used.</td>
 * </tr>
 * <tr>
 * <td>reset_callback</td>
 * <td>Called when a device reset is requested, this can be NULL if not used.</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * 
 * Device requests are used to send messages to a device.  The iDigi Connector user can write a web services
 * application to send device requests to the device; the application running on the device then interprets the
 * incoming device requests.
 *
 * Example:
 *
 * @code
 *
 * ret = idigi_register_device_request_callbacks(device_request_callback, device_response_callback, 
                                                 device_download_callback, device_reset_callback);
 * if (ret != idigi_connector_success)
 * {
 *    printf("%s: idigi_register_device_request_callbacks failed [%d]\n", __FUNCTION__, ret);
 *    goto error;
 * }
 *
 * @endcode
 *
 * @subsection device_request_callback Device Request
 *
 * The user defined function @ref idigi_device_request_callback_t "device_request_callback()" parses the device request received
 * from the iDigi Device Cloud to determine if the target and the data associated with that target is valid.  If both are valid,
 * the user defined function @ref idigi_device_request_callback_t "device_request_callback()" then parses the request
 * received.
 *
 * Example:
 *
 * @code
 * idigi_app_error_t device_request_callback(char const * const target, idigi_connector_data_t * const request_data)
 * {
 *    idigi_app_error_t status=idigi_app_invalid_parameter;
 *    int i;
 *
 *    ok_response = false;
 *
 *    if (request_data->error != idigi_connector_success)
 *    {
 *        printf("devcie_request_callback: error [%d]\n", request_data->error);
 *        goto error;
 *    }
 *
 *    printf("device_request_callback: command [%s]\n", target);
 *
 *    for (i=0; command_table[i].name != NULL; i++)
 *    {
 *        if (strcmp(target, command_table[i].name) == 0)
 *        {   
 *            status = command_table[i].cmd_handler(request_data->data_ptr, request_data->length_in_bytes);
 *            ok_response = true;
 *            break;
 *        }
 *    }
 *
 *    status = idigi_app_success;
 *
 * error:
 *    return status;
 * }
 * @endcode
 *
 * @subsection device_response_callback Device Response
 *
 * The user defined function @ref idigi_device_response_callback_t "device_response_callback()" checks the result of the @ref
 * idigi_device_request_callback_t "device_request_callback()", and sets up the response_data based on that result.  The
 * Application needs to check the error value in the response_data before returning the response.
 *
 * Example
 * @code
 * size_t device_response_callback(char const * const target, idigi_connector_data_t * const response_data)
 * {
 *     static char rsp_string[] = "iDigi Connector device response OK\n";
 *     static char error_rsp_string[] = "Invalid Target\n";
 *     size_t len;
 *     size_t bytes_to_copy;
 * 
 *     if (response_data->error != idigi_connector_success)
 *     {
 *         printf("devcie_response_callback: error [%d]\n", response_data->error);
 *         goto error;
 *     }
 *     if (ok_response) 
 *     {
 *         len = strlen(rsp_string);
 *         bytes_to_copy = (len < response_data->length_in_bytes) ? len : response_data->length_in_bytes;
 *         memcpy(response_data->data_ptr, rsp_string, bytes_to_copy);
 *     }
 *     else
 *     {
 *         len = strlen(error_rsp_string);
 *         bytes_to_copy = (len < response_data->length_in_bytes) ? len : response_data->length_in_bytes;
 *         memcpy(response_data->data_ptr, error_rsp_string, bytes_to_copy);
 *     }
 *     response_data->flags = IDIGI_FLAG_LAST_DATA;
 *     response_data->length_in_bytes   = len;
 * 
 *     printf("devcie_response_callback: data- %s\n", rsp_string);
 * 
 * error:
 *     return bytes_to_copy;
 * }
 * @endcode 
 * @subsection device_download_callback Firmware Download
 *
 * The user defined function @ref idigi_firmware_download_callback_t "idigi_firmware_download_callback()" is
 * called when a firmware download has been requested; when this routine is called the entire image
 * has been downloaded into RAM.  The name of the file, a pointer to the image along with the size is passed in as arguments,
 * this routine should write the image to the filesystem and return, the  @ref idigi_reset_callback_t "idigi_reset_request_callback()" will be
 * called after this routine.
 *
 * In each of the samples provided a file called firmare.c is linked in which implements the 
 * firware download and reset callbacks, the user can modify these callbacks to match the functionality
 * they need.  The download callback is passed in the name of the file to download, the application
 * could then accept or reject the download request based on the file name.  
 *
 * The routine device_download_callback() in firmware.c does the following
 *  -# Create a string containing the path and name of the file to write to flash, in the sample 
 *     provided the application is written to /opt/digi/bin
 *  -# Create the file 
 *  -# Write the image to the file.
 *
 * After the device device_download_callback()  is called the @ref device_reset_callback "idigi_reset_request_callback()"
 * is called, the reset callback can then take the appropriate action.
 *
 * @note To use the download feature provided with the samples you must download a valid executable, invalid 
 * executables are rejected.
 *
 * Example:
 * 
 * @code
 * idigi_app_error_t device_download_callback(char const * const file_name, char const * const data, unsigned int const length)
 * {
 *     FILE *fp;
 *     size_t bytes_written;
 *     idigi_app_error_t status=idigi_app_firmware_download_failed;
 *     
 *     printf("%s: fw download file=%s length=%d", __FUNCTION__, application_name, length);
 *     
 *     snprintf(application_name, MAX_FILE_NAME_LEN, "/opt/idigi/bin/%s", file_name);
 * 
 *     
 *     fp = fopen(application_name, "w");
 *     if (fp == NULL) 
 *     {
 *         perror("Couldn't open file\n");
 *         goto done;   
 *     }
 * 
 *     bytes_written = fwrite(data, 1, length, fp);
 *     if (bytes_written != length) 
 *     {
 *         perror("fwrite failed\n");
 *         goto done;
 *     }
 * 
 *     status = idigi_app_success;
 * done:
 *     if (fp != NULL) 
 *     {
 *         fclose(fp);
 *     }
 *     return status;
 * }
 * @endcode
 * 
 *  @subsection device_reset_callback Device Reset
 *
 * This callback is called when server requests a target reset. The callback should not return if it's resetting itself. 
 * It may return and continue, however the server may disconnect the device. 
 *
 * In the sample code provided in firmware.c, we change to the mode of the file which was downloaded
 * to be executable (chmod +x), then we <a href="http://linux.die.net/man/3/execl">execl</a> the new process,
 * this replaces the current running process with the process which was just downloaded.  You can modify this
 * callback to rebooot the device or exit the process depending on your requirements, the code provided is
 * one possible implementation.
 *
 * Example:
 *@code
 * idigi_app_error_t device_reset_callback(void)
 * {
 *     char buffer[MAX_FILE_NAME_LEN];
 *     printf("%s: exec'ing applucation %s", __FUNCTION__, application_name);
 * 
 *     snprintf(buffer, MAX_FILE_NAME_LEN, "chmod +x %s", application_name);
 * 
 *     system(buffer);
 *     execl(application_name, NULL); /* switch to the process we just downloaded. */
 * 
 *     printf("%s: Could not execute process", __FUNCTION__);
 * }
 * @endcode
 * @htmlinclude terminate.html
 *
 */
