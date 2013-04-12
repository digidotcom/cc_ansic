/*! @page data_service Data Service
 *
 * @htmlinclude nav.html
 *
 * @section data_service_overview Data Service Overview
 *
 * The Data Service API is used to send data to and from Etherios Device Cloud.  Data service transfers
 * are either initiated from Etherios Device Cloud or the device itself. The following requests under
 * @ref connector_class_id_data_service are listed below:
 *
 * @li @ref put_request : Data transfers initiated by the device and used to
 * write files on to Etherios Device Cloud.  Etherios Device Cloud may send a status response back
 * indicating the transfer was successful.
 * @li @ref device_request : Transfers initiated from a web services client connected to Etherios Device Cloud
 * to the device.  This transfer is used to send data to the device and the device may send a response back.
 * @li @ref data_point_request : These are device originated messages to send data points to Etherios Device Cloud.
 * The provided path name will be used as the stream ID and the stream can be accessed by Etherios Device Manager.
 *
 * @note See @ref data_service_support under Configuration to disable the data service.
 *
 * @section put_request Put Request
 *
 * @subsection initiate_send Initiate Sending Data
 *
 * The application initiates the Put Request to Etherios Device Cloud by calling connector_initiate_action()
 * with @ref connector_initiate_send_data request and @ref connector_data_service_put_request_t request_data.
 *
 * The connector_initiate_action() is called with the following arguments:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th>
 *   <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>handle</td>
 *   <td>@endhtmlonly @ref connector_handle_t @htmlonly returned from the @endhtmlonly connector_init() @htmlonly function.</td>
 * </tr>
 * <tr>
 *   <td>request</td>
 *   <td>@endhtmlonly @ref connector_initiate_send_data @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> Pointer to @endhtmlonly connector_data_service_put_request_t @htmlonly structure, where member:
 *        <ul>
 *        <li><b><i>path</i></b> is Etherios Device Cloud file path containing the data</li>
 *        <li><b><i>content_type</i></b> is "text/plain", "text/xml", "application/json", etc</li>
 *        <li><b><i>flags</i></b> are the @endhtmlonly @ref put_flags @htmlonly</li>
 *        <li><b><i>context</i></b> is the application session context</li>
 *        </ul></td>
 * </tr>
 * <tr>
 *   <td>response_data </td>
 *   <td> NULL </td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 *
 * An example of initiating a device transfer is shown below:
 * @code
 *   static connector_data_service_put_request_t header;
 *   static char file_path[] = "testdir/testfile.txt";
 *   static char file_type[] = "text/plain";
 *
 *   header.flags = CONNECTOR_DATA_PUT_APPEND;
 *   header.path  = file_path;
 *   header.content_type = file_type;
 *   header.context = &header;
 *
 *   // Begin a file transfer to Etherios Device Cloud
 *   status = connector_initiate_action(handle, connector_initiate_send_data, &header);
 * @endcode
 *
 * This example will invoke Etherios Cloud Connector to initiate a data transfer to Etherios Device
 * Cloud.  The result of this operation creates a file testfile.txt in the testdir directory
 * on Etherios Device Cloud.  Once Etherios Device Cloud is ready to receive data
 * from the device the application callback is called requesting data.
 *
 * @subsection get_data Put Request Callback
 *
 * After calling connector_initiate_action(), Etherios Cloud Connector will make @ref connector_data_service_put_request "Put Request"
 * @ref connector_callback_t "callbacks" to retrieve the application data. These callbacks will continue
 * until the transfer is complete or an error is encountered. @ref CONNECTOR_MSG_LAST_DATA flag in the client_data
 * field indicates the last chunk of data.
 *
 * For a non-tcp transport session, before making the very first @ref connector_data_service_put_request "Put Request"
 * callback, Etherios Connector will call the @ref total_length "total length" callback to get the total
 * length of the send buffer.
 *
 * The data to the server (@ref connector_data_service_type_need_data message type) is sent in the
 * connector_data_service_msg_response_t field.
 *
 * The response from the server (@ref connector_data_service_type_have_data message type) is received in the
 * connector_data_service_msg_request_t field. Typically the response consists of
 * the success or error status code and text of the data service session.
 *
 * A callback with @ref connector_data_service_type_error message type is received when message processing error occurs.
 * This aborts the data service session.
 *
 * The @ref connector_data_service_put_request "Put Request"  @ref connector_callback_t "callback" is called with the following information:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>class_id</td>
 *   <td>@endhtmlonly @ref connector_class_id_data_service @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_data_service_put_request @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_data_service_msg_request_t @htmlonly structure:<br></br>
 *     <b><i>service_context</i></b> set to the request_data argument from the @endhtmlonly connector_initiate_action() @htmlonly
 *     @endhtmlonly @ref connector_initiate_send_data @htmlonly call<br></br>
 *     <b><i>message_type</i></b> can be set to one of the following:
 *       <ul>
 *       <li> @endhtmlonly @ref connector_data_service_type_need_data @htmlonly: request to send more data to the server.</li>
 *       <li> @endhtmlonly @ref connector_data_service_type_have_data @htmlonly: response from the server.</li>
 *       <li> @endhtmlonly @ref connector_data_service_type_error @htmlonly: message processing error.</li>
 *       </ul>
 *
 *     <b><i>server_data</i></b> is based on <i>message_type</i> setting:
 *       <ul>
 *       <li> For <i>message_type</i> set to @endhtmlonly @ref connector_data_service_type_need_data @htmlonly: server_data not used.</li>
 *       <li> For <i>message_type</i> set to @endhtmlonly @ref connector_data_service_type_have_data @htmlonly: server_data points to a @endhtmlonly connector_data_service_block_t @htmlonly structure where:
 *           <ul>
 *           <li> <b><i>data</i></b> might contain response from the server.</li>
 *           <li> <b><i>length_in_bytes</i></b> is the size of response from the server.</li>
 *           <li> <b><i>flags</i></b> is @endhtmlonly @ref data_service_flags @htmlonly: success or error code from the server.</li>
 *           </ul></li>
 *       <li> For <i>message_type</i> set to @endhtmlonly @ref connector_data_service_type_error @htmlonly: server_data points to a @endhtmlonly connector_data_service_block_t @htmlonly structure:
 *           <ul>
 *           <li>where <b><i>data</i></b> contains the message processing error code of @endhtmlonly @ref connector_msg_error_t @htmlonly type.</li>
 *           </ul></li>
 *       </ul>
 *   </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Size of @endhtmlonly connector_data_service_msg_request_t @htmlonly</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td>[OUT] pointer to @endhtmlonly connector_data_service_msg_response_t @htmlonly structure where:
 *   <ul>
 *   <li><b><i>user_context</i></b> is not used.</li>
 *   <li><b><i>message_status</i></b> is the status of @endhtmlonly @ref connector_msg_error_t @htmlonly type to send to the server on return.</li>
 *   <li><b><i>client_data</i></b> points to a @endhtmlonly connector_data_service_block_t @htmlonly structure where:
 *      <ul>
 *      <li><b><i>data</i></b> points to a copy of the data sent to the server.</li>
 *      <li><b><i>length_in_bytes</i></b> is the size of data buffer on input, actual data size on output.</li>
 *      <li><b><i>@endhtmlonly @ref data_service_flags "flags" @htmlonly</i></b> used to mark the @endhtmlonly @ref CONNECTOR_MSG_FIRST_DATA "first" @htmlonly @endhtmlonly or @ref CONNECTOR_MSG_LAST_DATA "last" @htmlonly chunk of data.</li>
 *      </ul></li>
 *    </ul>
 * </td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td>[OUT] Size of @endhtmlonly connector_data_service_msg_response_t @htmlonly</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 * <td>Continue</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 * <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 * <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * An example of an application callback for a put request is shown below:
 *
 * @code
 * connector_callback_status_t app_data_service_callback(connector_class_id_t const class_id, connector_request_id_t const request_d,
 *                                                  void const * request_data, size_t const request_length,
 *                                                  void * response_data, size_t * const response_length)
 * {
 *    connector_callback_status_t status = connector_callback_continue;
 *    connector_data_service_msg_request_t const * const put_request = request_data;
 *    connector_data_service_msg_response_t * const put_response = response_data;
 *
 *
 *     if (class_id == connector_class_id_data_service && request_id.data_service_request == connector_data_service_device_request)
 *    {
 *        switch (put_request->message_type)
 *        {
 *        case connector_data_service_type_need_data: // iDigi Device Cloud requesting data
 *            {
 *                connector_data_service_block_t * const message = put_response->client_data;
 *                char const buffer[] = "iDigi data service sample\n";
 *                size_t const bytes = strlen(buffer);
 *
 *                if (message->length_in_bytes > bytes)
 *                    message->length_in_bytes = bytes;
 *
 *                memcpy(message->data, buffer, message->length_in_bytes);
 *                message->flags = CONNECTOR_MSG_LAST_DATA | CONNECTOR_MSG_FIRST_DATA;
 *                put_response->message_status = connector_msg_error_none;
 *            }
 *            break;
 *
 *        case connector_data_service_type_have_data: // Response from Etherios Device Cloud
 *            {
 *                connector_data_service_block_t * const message = put_request->server_data;
 *
 *                APP_DEBUG("Received %s response from server\n",
 *                          ((message->flags & CONNECTOR_MSG_RESP_SUCCESS) != 0) ? "success" : "error");
 *                if (message->length_in_bytes > 0)
 *                {
 *                    char * const data = message->data;
 *
 *                    data[message->length_in_bytes] = '\0';
 *                    APP_DEBUG("Server response %s\n", data);
 *                }
 *            }
 *            break;
 *
 *        case connector_data_service_type_error: // Etherios Device Cloud sent back an error
 *            {
 *                connector_data_service_block_t * const message = put_request->server_data;
 *                connector_msg_error_t const * const error_value = message->data;
 *
 *                APP_DEBUG("Data service error: %d\n", *error_value);
 *            }
 *            break;
 *
 *        default:
 *            APP_DEBUG("Unexpected command: %d\n", request);
 *            break;
 *        }
 *    }
 *    else
 *    {
 *        APP_DEBUG("Request not supported in this sample: %d\n", request);
 *    }
 *
 * done:
 *    return status;
 * }
 * @endcode
 *
 * @section device_request Device Request
 *
 * Device requests are data transfers initiated by Etherios Device Cloud. They are used to
 * send data from Device Cloud to the device, and the device may send back a reply.
 * The user will receive the application callback when a device request is received from Device Cloud.
 * The application callback is continually called with the data from Device Cloud
 * until the transfer is complete or an error is encountered.
 *
 * Etherios Cloud Connector invokes the application-defined callback to process and respond a device request.
 *      -# @ref ds_receive_target
 *      -# @ref ds_receive_data
 *      -# @ref ds_receive_reply_data
 *      -# @ref ds_receive_reply_length
 *      -# @ref ds_receive_status
 *
 * The sequence calling an application-defined callback for device request is:
 *      -# Etherios Cloud Connector calls application-defined callback @ref ds_receive_target to process the target of the device request
 *      -# Etherios Cloud Connector calls application-defined callback @ref ds_receive_data to process the data of the device request
 *      -# Etherios Cloud Connector calls application-defined callback @ref ds_receive_reply_length to get the total length in bytes for the response data.
 *         This is only callback when device request is from UDP transport. See @ref CONNECTOR_TRANSPORT_UDP to enable UDP transport.
 *      -# Etherios Cloud Connector calls application-defined callback @ref ds_receive_reply_data for response data sent back to Etherios Device Cloud.
 *
 * Etherios Cloud Connector calls step 2 repeatedly for all device request data and step 4 for all response data.
 *
 * For a non-tcp transport session, before making the very first call to get the user response (only if
 * Etherios Device Cloud requested for the response), Etherios Cloud Connector will call the
 * @ref ds_receive_reply_length to get the total length of the response data.
 *
 * @note See @ref data_service_support under Configuration to enable the data service.
 * @note See @ref CONNECTOR_TRANSPORT_TCP and @ref network_tcp_start to enable and start TCP.
 * @note See @ref CONNECTOR_TRANSPORT_UDP and @ref network_udp_start to enable and start UDP.
 *
 * @subsection ds_receive_target  Device Request Target Callback
 *
 * This callback is called first with the device request target.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>class_id</td>
 *   <td>@endhtmlonly @ref connector_class_id_data_service @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_service_receive_target @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td> Pointer to @endhtmlonly connector_data_service_receive_target_t @htmlonly structure where:
 *       <dl><dt>transport</dt><dd> - Transport method from where the device request is originated.</dd>
 *           <dt>user_context</dt><dd> - Callback writes its own context which will be passed back to
 *                                       subsequential callback.</dd>
 *           <dt>target</dt><dd> - Contains pointer to requested target string.</dd>
 *           <dt>response_required</dt><dd>
 *               <ul><li>@endhtmlonly @ref connector_true @htmlonly if Etherios Device Cloud requests a response and
 *                       @endhtmlonly @ref ds_receive_reply_data @htmlonly will be called for response data.</li>
 *                   <li>@endhtmlonly @ref connector_false @htmlonly if Device Cloud does not require a response and
 *                       @endhtmlonly @ref ds_receive_reply_data @htmlonly will not be called.
 *                       <br><b>Note:</b> Application must free any memory associated with the device request.</li></ul>
 *       </dd></dl>
 * </td>
 * <tr>
 *   <th colspan="2" class="title">Return Values</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Values</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_error @htmlonly</td>
 *   <td>Application encountered error.
 *       <br> @endhtmlonly @ref ds_receive_reply_data @htmlonly will be called
 *       to send error data back to Etherios Device Cloud.
 *       <br><b>Note:</b> The status in the response will be set to zero (not handled) if the request is originated from TCP transport.
 *   </td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection ds_receive_data  Device Request Data Callback
 *
 * This callback is called to process device request data. This callback will be called repeatedly until the last chuck of data.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>class_id</td>
 *   <td>@endhtmlonly @ref connector_class_id_data_service @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_service_receive_data @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td> Pointer to @endhtmlonly connector_data_service_receive_data_t @htmlonly structure where:
 *       <dl><dt>transport</dt><dd> - Transport method from where the device request is originated.</dd>
 *           <dt>user_context</dt><dd> - Pointer to callback's context returned from previous device request callback.
 *                                       Callback may write its own context which will be passed back to
 *                                       subsequential callback.</dd>
 *           <dt>buffer</dt><dd> - Pointer to device request data.</dd>
 *           <dt>bytes_used</dt><dd> - Contains the length of device request data in bytes.</dd>
 *           <dt>more_data</dt><dd> - Contains @endhtmlonly @ref connector_true @htmlonly for more data and this callback
 *                                    will be called again or @endhtmlonly @ref connector_false @htmlonly if this is last chuck
 *                                    of data.
 *       </dd></dl>
 * </td>
 * <tr>
 *   <th colspan="2" class="title">Return Values</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Values</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 *   <td>@endhtmlonly @ref connector_callback_error @htmlonly</td>
 *   <td>Application encountered error.
 *       <br>@endhtmlonly @ref ds_receive_reply_data @htmlonly will be called
 *       to send error data back to Etherios Device Cloud.
 *       <br><b>Note:</b> The status in the response will be set to zero (not handled) if the request is originated from TCP transport.
 *   </td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection ds_receive_reply_data  Device Request Response Data Callback
 *
 * This callback is called for response or error data to be sent back to Etherios Device Cloud.
 * This callback will be called repeatedly until there is no more data.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>class_id</td>
 *   <td>@endhtmlonly @ref connector_class_id_data_service @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_service_receive_reply_data @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td> Pointer to @endhtmlonly connector_data_service_receive_reply_data_t @htmlonly structure where:
 *       <dl><dt>transport</dt><dd> - Transport method from where the device request is originated.</dd>
 *           <dt>user_context</dt><dd> - Pointer to callback's context returned from previous device request callback.
 *                                       Callback may write its own context which will be passed back to
 *                                       subsequential callback.</dd>
 *           <dt>buffer</dt><dd> - Pointer to memory where callback writes response data to.</dd>
 *           <dt>bytes_available</dt><dd> - Contains number of bytes available in the buffer.</dd>
 *           <dt>bytes_used</dt><dd> - Callback writes number of bytes written onto the buffer.</dd>
 *           <dt>more_data</dt><dd> - Callback writes @endhtmlonly @ref connector_true @htmlonly for more response data and this callback
 *                                    will be called again or @endhtmlonly @ref connector_false @htmlonly if this is last chuck
 *                                    of data.
 *       </dd></dl>
 * </td>
 * <tr>
 *   <th colspan="2" class="title">Return Values</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Values</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>ds_receive_status
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 *   <td>@endhtmlonly @ref connector_callback_error @htmlonly</td>
 *   <td>Application encountered error. Etherios Cloud Connector will cancel the device request.</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection ds_receive_reply_length  Device Request Response Length Callback
 *
 * This callback is called to get the total number of bytes for the response data before calling @ref ds_receive_reply_data.
 * This is only called when the device request is originated from UDP transport and response is required.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>class_id</td>
 *   <td>@endhtmlonly @ref connector_class_id_data_service @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_service_receive_reply_length @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td> Pointer to @endhtmlonly connector_data_service_length_t @htmlonly structure where:
 *       <dl><dt>transport</dt><dd> - Transport method from where the device request is originated.</dd>
 *           <dt>user_context</dt><dd> - Pointer to callback's context returned from previous device request callback.
 *                                       Callback may write its own context which will be passed back to
 *                                       subsequential callback.</dd>
 *           <dt>total_bytes</dt><dd> - Callback writes total number of bytes for the response data.
 *       </dd></dl>
 * </td>
 * <tr>
 *   <th colspan="2" class="title">Return Values</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Values</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection ds_receive_status  Device Request Status Callback
 *
 * This callback is called when Etherios Cloud Connector is done with the device request,
 * encounters error or receives error from Etherios Device Cloud. Application should
 * free any memory associated with the request in ths callback.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>class_id</td>
 *   <td>@endhtmlonly @ref connector_class_id_data_service @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_service_receive_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td> Pointer to @endhtmlonly connector_data_service_status_t @htmlonly structure where:
 *       <dl><dt>transport</dt><dd> - Transport method from where the device request is originated.</dd>
 *           <dt>user_context</dt><dd> - Pointer to callback's context returned from previous device request callback.</dd>
 *           <dt>status</dt><dd> - Contains one of the following status:
 *                              <ul><li><b>connector_data_service_status_complete</b> - Etherios Cloud Connector is done with the device
 *                                       request. This is called only when Etherios Device Cloud doesn't request a response and
 *                                        @endhtmlonly @ref ds_receive_data @htmlonly will not be called. </li>
 *                                  <li><b>connector_data_service_status_cancel</b> - The device request was canceled by the user.</li>
 *                                  <li><b>connector_data_service_status_timeout</b> - The device request was timed out.</li>
 *                                  <li><b>connector_data_service_status_session_error</b> - Etherios Cloud Connector encountered error.
 *                                         See session_error. </li></dd>
 *           <dt>session_error</dt><dd> - Contains @endhtmlonly @ref connector_session_error_t @htmlonly when status is
 *                                        <b>connector_data_service_status_session_error</b>.</li>
 *       </dd></dl>
 * </td>
 * <tr>
 *   <th colspan="2" class="title">Return Values</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Values</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * A user uses the SCI web service to send a device request to Etherios Device Cloud, which
 * in turn sends it to the device. An example of an application callback for a device
 * request is show below:
 *
 *
 * @section data_point_request Data point request
 *
 * @subsection initiate_data_point Initiate the data point request
 *
 * The application initiates the Data point request to Etherios Device Cloud by calling connector_initiate_action()
 * with @ref connector_initiate_data_point request and @ref connector_message_dp_request_t "data point request" as request_data.
 *
 * The connector_initiate_action() is called with the following arguments:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th>
 *   <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>handle</td>
 *   <td>@endhtmlonly @ref connector_handle_t @htmlonly returned from the @endhtmlonly connector_init() @htmlonly function.</td>
 * </tr>
 * <tr>
 *   <td>request</td>
 *   <td>@endhtmlonly @ref connector_initiate_data_point @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> Pointer to @endhtmlonly connector_message_dp_request_t @htmlonly structure, where member:
 *      <ul>
 *        <li><b><i>transport</i></b> is the communication method used to send the data point to Etherios Device Cloud</li>
 *        <li><b><i>flags</i></b> is set to @endhtmlonly @ref CONNECTOR_DATA_RESPONSE_NOT_NEEDED @htmlonly if no response is needed</li>
 *        <li><b><i>user_context</i></b> is the application context which will be returned in the response</li>
 *        <li><b><i>channels</i></b> pointer to first @endhtmlonly connector_message_dia_channel_t @htmlonly structure where:
 *        <ul>
 *          <li><b><i>name</i></b> points to dia channel name, a null-terminated string.</li>
 *          <li><b><i>data</i></b> points to dia data buffer.</li>
 *          <li><b><i>bytes</i></b> size of data buffer in bytes.</li>
 *          <li><b><i>unit</i></b> a null-terminated string, carries the unit (e.g. "mph\0").</li>
 *          <li><b><i>type</i></b> @endhtmlonly @ref connector_dia_data_type_t "data type" @htmlonly</li>
 *          <li><b><i>content_type</i></b> @endhtmlonly @ref connector_dia_content_type_t "content type" @htmlonly</li>
 *          <li><b><i>time</i></b> is a time structure where:
 *          <ul>
 *            <li><b><i>source</i></b> local time or server time.</li>
 *            <li><b><i>value</i></b> ISO 8601 string for TCP transport and epoch time for non-TCP transport.</li>
 *          </ul></li>
 *        </ul></li>
 *      </ul></td>
 * </tr>
 * <tr>
 *   <td>response_data </td>
 *   <td> NULL </td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 *
 * An example of initiating a data point request is shown below:
 * @code
 * //TBD: The data point request code goes here.
 * @endcode
 *
 * This example will invoke Etherios Connector to initiate a data point request to Etherios Device
 * Cloud. If the response needed is set (on non-TCP method), the application callback is called
 * with the cloud response or error in case Etherios Connector fails to send the request.
 *
 * @subsection dp_response Callback with data point response
 *
 * After calling connector_initiate_action(), Etherios Connector will prepare and send data point request
 * to Etherios Device Cloud asynchronously. Upon receiving the response, it makes
 * @ref connector_data_service_dp_response "data point response" callback to pass the response to the application.
 * Application is free to release the allocated resources at this point.
 *
 * The response from the server is received in the @ref connector_message_status_response_t "status" as request_data.
 * Typically the response consists of the success or error status code and an optional text message in case of error.
 *
 * The @ref connector_data_service_dp_response "data point response" @ref connector_callback_t "callback" is called with the
 * following information:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <td>class_id</td>
 *   <td>@endhtmlonly @ref connector_class_id_data_service @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_data_service_dp_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_message_status_response_t @htmlonly structure:
 *      <ul>
 *        <li><b><i>user_context</i></b> is the user context passed in the @endhtmlonly connector_initiate_action() @htmlonly
 *              with @endhtmlonly @ref connector_initiate_data_point @htmlonly call</li>
 *        <li><b><i>status</i></b> @endhtmlonly @ref connector_session_status_t @htmlonly returned from Etherios device cloud
 *               or any error while preparing/sending data point request</li>
 *        <li><b><i>error_text</i></b> an optional error reason string</li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Size of @endhtmlonly connector_message_status_response_t @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td>[OUT] NULL </td></tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] 0 </td> </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 * <td>Continue</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 * <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 * <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * An example of an application callback for a data point is shown below:
 *
 * @code
 * // TBD: The sample data point callback code goes here.
 * @endcode
 *
 * @section zlib Optional Data Compression Support
 * Etherios Cloud Connector has an optional Data Compression switch that reduces the amount of network traffic.  This option requires applications
 * to link with the zlib library and add the zlib header file (zlib.h) to Etherios Cloud Connector include path.
 *
 * @note Enabling this option greatly increases the application code size and memory required to execute.
 *
 * If your application requires Data compression, but your development environment does not include the zlib library,
 * you will need to download and build the library.  The zlib home page is located at: http://zlib.net/.   Instructions
 * on how to build zlib are provided with the package.
 *
 * @note The zlib library is required only if your application enables the @ref CONNECTOR_COMPRESSION "Data Compression switch".
 *
 * @htmlinclude terminate.html
 */
