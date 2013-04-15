/*! @page data_service Data Service
 *
 * @htmlinclude nav.html
 *
 * @section data_service_overview Data Service Overview
 *
 * The Data Service API is used to send data to and from Etherios Device Cloud.  Data service transfers
 * are either initiated from Etherios Device Cloud (@ref device_request) or the device itself (@ref send_data).
 *
 * @li @ref send_data : Data transfers initiated by the device and used to write files on to Etherios Device Cloud.
 * Device Cloud may send a response back to the device indicating the transfer status.
 * @li @ref device_request : Transfers initiated from a web services client connected to Etherios Device Cloud which
 * hosts the device. This transfer is used to send data to the device and the device may send a response back.
 *
 * @note See @ref data_service_support under Configuration to disable the data service.
 *
 * @section send_data Send data to Device Cloud
 *
 * This is one way the device can send data to Etherios Device Cloud. The other method is using @ref data_points.
 * It is the device originated transaction, starts when the connector_initiate_action() is called with request ID
 * @ref connector_initiate_send_data.
 *
 * Etherios Cloud Connector invokes the application-defined callback to get the actual data to send and to pass
 * Device Cloud response.
 *      -# @ref initiate_send_data
 *      -# @ref send_data_length_callback
 *      -# @ref send_data_callback
 *      -# @ref send_data_response_callback
 *      -# @ref send_data_status_callback
 *
 * Once the @ref initiate_send_data is called the callbacks will be called in following sequence:
 *      -# Etherios Cloud Connector calls application-defined callback @ref send_data_callback to get the user data (chunk of data).
 *      -# Etherios Cloud Connector calls application-defined callback @ref send_data_response_callback to pass Device Cloud response.
 *      -# Etherios Cloud Connector calls application-defined callback @ref send_data_status_callback to inform session complete if
 *         the response is not requested or error occurs while sending the data.
 *
 * For a non-tcp transport session, before making the very first call to get the user data, Etherios Cloud
 * Connector will call the @ref send_data_length_callback to get the total length of the response data.
 *
 * Etherios Cloud Connector calls @ref send_data_callback repeatedly to get all data.
 *
 * @note See @ref data_service_support under Configuration to enable the data service.
 * @note See @ref CONNECTOR_TRANSPORT_TCP and @ref network_tcp_start to enable and start TCP.
 * @note See @ref CONNECTOR_TRANSPORT_UDP and @ref network_udp_start to enable and start UDP.
 *
 * @subsection initiate_send_data Initiate Sending Data
 *
 * The application initiates the send data to Etherios Device Cloud by calling connector_initiate_action()
 * with @ref connector_initiate_send_data request and @ref connector_request_data_service_send_t request_data.
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
 *   <td> Pointer to @endhtmlonly connector_request_data_service_send_t @htmlonly structure, where member:
 *        <ul>
 *        <li><b><i>transport</i></b>, a method to use to send data </li>
 *        <li><b><i>user_context</i></b>, is the user owned context pointer </li>
 *        <li><b><i>path</i></b> is Etherios Device Cloud file path containing the data (shouldn't be stack variable) </li>
 *        <li><b><i>content_type</i></b> is "text/plain", "text/xml", "application/json", etc (shouldn't be stack variable) </li>
 *        <li><b><i>option</i></b>, is to inform Device Connector on what to do with the data </li>
 *        <li><b><i>response_required</i></b>, set to connector_true is the response is needed </li>
 *        </ul></td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 *
 * An example of initiating a device transfer is shown below:
 * @code
 *   static connector_request_data_service_send_t header;
 *   static char file_path[] = "testdir/testfile.txt";
 *   static char file_type[] = "text/plain";
 *
 *   header.transport = connector_transport_tcp;
 *   header.path  = file_path;
 *   header.content_type = file_type;
 *   header.response_required = connector_true;
 *   header.option = connector_data_service_send_option_append;
 *   header.context = file_path;
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
 * Note: The header, file_path and file_type above are not stack variables. Either you can use a heap or a memory (global or static)
 * variable to hold these values. You can release themwhen you get a @ref  connector_request_id_data_service_send_response "response"
 * or @ref connector_request_id_data_service_send_status "status" callback. The value passed as the user_context will be returned
 * in every callback for this session. User is free to update this at any point during the session.
 *
 * @subsection send_data_length_callback Get total length callback
 *
 * For a non-tcp transport session, before making the very first @ref connector_request_id_data_service_send_data "Send Data"
 * callback, Cloud Connector will call the @ref connector_request_id_data_service_send_length "total length" callback to get the total
 * length of the send buffer.
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
 *   <td>@endhtmlonly @ref connector_request_id_data_service_send_length @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_data_service_length_t @htmlonly structure:<br></br>
 *     <ul>
 *       <li><b><i>transport</i></b>, a method chosen to send data </li>
 *       <li><b><i>user_context</i></b>, is the user owned context pointer </li>
 *       <li><b><i>total_bytes</i></b>, the total size of the user data to send in bytes </li>
 *     </ul>
 *   </td>
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
 * @subsection send_data_callback Send data callback
 *
 * After calling connector_initiate_action(), Etherios Cloud Connector will make @ref connector_request_id_data_service_send_data "Send Data"
 * @ref connector_callback_t "callbacks" to retrieve the application data. These callbacks will continue
 * until the user sets more_data flag to connector_false or an error is encountered.
 *
 * The @ref connector_request_id_data_service_send_data "Send Data" @ref connector_callback_t "callback" is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_request_id_data_service_send_data @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_data_service_send_data_t @htmlonly structure:<br></br>
 *     <ul>
 *       <li><b><i>transport</i></b>, a method chosen to send data </li>
 *       <li><b><i>user_context</i></b>, is the user owned context pointer </li>
 *       <li><b><i>buffer</i></b> pointer to store user data </li>
 *       <li><b><i>bytes_available</i></b>, the maximum number of bytes the user can copy to the buffer </li>
 *       <li><b><i>bytes_used</i></b>, the number of bytes filled, cannot be more than the bytes_available </li>
 *       <li><b><i>more_data</i></b>, set to connector_true if more data to send, the callback will be called again in that case </li>
 *     </ul>
 *   </td>
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
 * @subsection send_data_response_callback Send data response callback
 *
 * Etherios Cloud Connector will make @ref connector_request_id_data_service_send_response "response"
 * @ref connector_callback_t "callback" to pass Etherios Device Cloud response to the send data request.
 * User can free their user_context and any other reserved data as soon as they receive this callback.
 *
 * The @ref connector_request_id_data_service_send_response "response" @ref connector_callback_t "callback"
 * is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_request_id_data_service_send_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_data_service_send_response_t @htmlonly structure:<br></br>
 *     <ul>
 *       <li><b><i>transport</i></b>, a method chosen to send data </li>
 *       <li><b><i>user_context</i></b>, holds the user owned context pointer </li>
 *       <li><b><i>response</i></b> response code returned from Device Cloud </li>
 *       <li><b><i>hint</i></b>, incase of error, Device Connector will return a cause for failure </li>
 *     </ul>
 *   </td>
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
 * @subsection send_data_status_callback Send data status callback
 *
 * Etherios Cloud Connector will make @ref connector_request_id_data_service_send_status "status"
 * @ref connector_callback_t "callback" to pass the reason for session complete. User will receive this
 * callback when Device Connector response is not requested or in case of any errors. The error from
 * the application level is returned via @ref connector_request_id_data_service_send_response "response".
 * User can free their user_context and any other reserved data as soon as they receive this callback.
 *
 * The @ref connector_request_id_data_service_send_status "status" @ref connector_callback_t "callback"
 * is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_request_id_data_service_send_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_data_service_status_t @htmlonly structure:<br></br>
 *     <ul>
 *       <li><b><i>transport</i></b>, a method chosen to send data </li>
 *       <li><b><i>user_context</i></b>, holds the user owned context pointer </li>
 *       <li><b><i>status</i></b> reason to end the session, returned from Device Connector </li>
 *       <li><b><i>session_error</i></b>, this field will carry the session error code
 *                 when the status is set to connector_data_service_status_session_error </li>
 *     </ul>
 *   </td>
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
