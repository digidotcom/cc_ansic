/*! @page sm_callbacks Short Message Callbacks
 *
 * @htmlinclude nav.html
 *
 * @section sm_callbacks_overview Short Message (SM) callbacks Overview
 *
 * The SM callback APIs are used to send and receive short messages to and from Device Cloud.
 * The SM transfers are communicated over UDP transport method. The following requests under
 * @ref connector_class_id_short_message are listed below:
 *
 * @li @ref ping_request : Explains how to send ping request to Device Cloud and how to receive a ping request
 * or ping response from Device Cloud.
 * @li @ref cli_request : Explains how the callback are called to handle the CLI command from Device Cloud.
 * The CLI can be initiated either from Device Cloud Manager or from a web service client connected to Device Cloud.
 * @li @ref pending_data : This callback indicates that there are more data pending on Device Cloud for this device.
 * @li @ref opaque_response : This callback is called when a valid response is received from Device Cloud, but no
 * matching request is available in Cloud Connector.
 * @li @ref cancel_session : This section explains how to cancel a pending short message session.
 *
 * @note See @ref CONNECTOR_TRANSPORT_UDP and @ref network_udp_start to enable and start UDP.
 *
 * @section ping_request  Ping request
 *
 * The device can send ping request to Device Cloud at any time to let Device Cloud know that Cloud Connector
 * is ready to receive any pending messages. Similarly Device Cloud user can ping their devices using a ping
 * request to Cloud Connector. Thease ping requests can be sent without requesting for a response.
 *
 * The following sub-sections will explain each scenario separately.
 *      -# @ref initiate_ping
 *      -# @ref ping_response_callback
 *      -# @ref ping_request_callback
 *
 * @subsection initiate_ping  Ping Device Cloud
 *
 * The application initiates the ping request to Device Cloud by calling connector_initiate_action()
 * with @ref connector_initiate_ping_request request and @ref connector_sm_send_ping_request_t request_data.
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
 *   <td>@endhtmlonly @ref connector_initiate_ping_request @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_send_ping_request_t @htmlonly structure, where member:
 *        <ul>
 *        <li><b><i>transport</i></b>, a method to use to send data (CONNECTOR_TRANSPORT_UDP only at this point) </li>
 *        <li><b><i>user_context</i></b>, is the user owned context pointer, can be used to identify the response </li>
*        <li><b><i>response_required</i></b>, set to connector_true if the response is needed </li>
 *        </ul></td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 *
 * An example of initiating a ping is shown below:
 * @code
 *   static connector_sm_send_ping_request_t request;
 *
 *   request.transport = connector_transport_udp;
 *   header.response_required = connector_true;
 *   header.user_context = &request;
 *
 *   // Send ping to Device Cloud
 *   status = connector_initiate_action(handle, connector_initiate_ping_request, &request);
 * @endcode
 *
 * This example will invoke Cloud Connector to initiate a ping request to Device Cloud. The completion of this request
 * is indicated by a @ref ping_response_callback "response callback".
 *
 * @note: The variable request above is not a stack variables. Either you can use a heap or a memory (global or static)
 * variable to hold this value. You can release them when you get a @ref  ping_response_callback "response" callback.
 * The value passed as the user_context will be returned in the response.
 *
 * @subsection ping_response_callback  Ping response callback
 *
 * This callback is called with @ref connector_request_id_sm_ping_response "ping response" @ref connector_callback_t "callback".
 *
 * If the response is requested then the callback with connector_sm_ping_status_success indicates a success response
 * from Device Cloud. When the response is not requested, a callback with connector_sm_ping_status_complete
 * indicates the ping is sent successfully. If the session is @ref cancel_session "cancelled" by the user then the callback
 * will be called with connector_sm_ping_status_cancel.
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
 *   <td>@endhtmlonly @ref connector_class_id_short_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_sm_ping_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_ping_response_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>transport</i></b>, a method chosen to send ping </li>
 *       <li><b><i>user_context</i></b>, is the user owned context pointer </li>
 *       <li><b><i>status</i></b>, a response code indicating success, complete or error </li>
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
 * <td>Aborts Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection ping_request_callback  Ping request callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_ping_request "Ping Request"
 * @ref connector_callback_t "callback" to inform the application that ping is received. This callback
 * is just for the information purpose only. No action is needed from the application, unless it wants to
 * abort the connection.
 *
 * The @ref connector_request_id_sm_ping_request "Ping Request" @ref connector_callback_t "callback" is called with
 * the following information:
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
 *   <td>@endhtmlonly @ref connector_class_id_short_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_sm_ping_request @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_receive_ping_request_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>transport</i></b>, a method on which ping is received </li>
 *       <li><b><i>response_required</i></b>, it will be set to connector_true if Device Cloud wants response from Cloud Connector </li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_error @htmlonly</th>
 *   <td>Application encountered error.</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @section cli_request  CLI request
 *
 * The CLI request sent to the device from Device Cloud will use this callbacks to carry out the requests.
 * The CLI callbacks will come in following order, the status callback can come in any order after
 * cli_request_callback to inform any error condition.
 *
 *      -# @ref cli_request_callback
 *      -# @ref cli_response_length_callback
 *      -# @ref cli_response_callback
 *      -# @ref cli_status_callback
 *
 * @subsection cli_request_callback  CLI request callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_cli_request "CLI request"
 * @ref connector_callback_t "callback" to pass received CLI command from Device Cloud.
 *
 * The @ref connector_request_id_sm_cli_request "CLI request" @ref connector_callback_t "callback"
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
 *   <td>@endhtmlonly @ref connector_class_id_short_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_sm_cli_request @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_cli_request_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>transport</i></b>, a method on which the CLI request is received </li>
 *       <li><b><i>user_context</i></b>, user provided context </li>
 *       <li><b><i>buffer</i></b>, CLI command received from Device Cloud </li>
 *       <li><b><i>bytes_used</i></b>, number of bytes in CLI command </li>
 *       <li><b><i>response_required</i></b>, will be set to connector_true if Device Cloud needs response </li>
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
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_error @htmlonly</th>
 *   <td>Application encountered error.</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection cli_response_length_callback  Response length callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_cli_response_length "response length"
 * @ref connector_callback_t "callback" to get the total/maximum predicted CLI response length in bytes. This
 * callback will be made only if the response is requested by Device Cloud. Cloud Connector will allocate
 * the required resources based on the length specified here. Actual bytes used during
 * @ref cli_response_callback "response callback" can be anything, but must be less than this number.
 *
 * The @ref connector_request_id_sm_cli_response_length "response length" @ref connector_callback_t "callback"
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
 *   <td>@endhtmlonly @ref connector_class_id_short_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_sm_cli_response_length @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_cli_response_length_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>transport</i></b>, a method on which the CLI request is received </li>
 *       <li><b><i>user_context</i></b>, user provided context </li>
 *       <li><b><i>total_bytes</i></b>, total number of bytes in CLI response </li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection cli_response_callback  CLI response callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_cli_response "CLI response"
 * @ref connector_callback_t "callback" to get the CLI response to send to Device Cloud. This
 * callback will be made only if the response is requested by Device Cloud. The filled bytes
 * must not exceed the available bytes. The available bytes is based on the @ref cli_response_length_callback
 * "total length".
 *
 * The @ref connector_request_id_sm_cli_response "CLI response" @ref connector_callback_t "callback"
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
 *   <td>@endhtmlonly @ref connector_class_id_short_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_sm_cli_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_cli_response_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>transport</i></b>, a method on which the CLI request is received </li>
 *       <li><b><i>user_context</i></b>, user provided context </li>
 *       <li><b><i>buffer</i></b>, for user to copy CLI response to </li>
 *       <li><b><i>bytes_available</i></b>, size of buffer in bytes </li>
 *       <li><b><i>bytes_used</i></b>, number of response bytes copied </li>
 *       <li><b><i>more_data</i></b>, set this to connector_false if there is no more response buffer </li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_error @htmlonly</th>
 *   <td>Application encountered error.</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection cli_status_callback  CLI session error callback
 *
 * This callback is called with @ref connector_request_id_sm_cli_status "CLI status" @ref connector_callback_t "callback"
 * to indicate the reason for unusual CLI session end. User may get this call when @ref connector_initiate_stop_request_t
 * "stop transport" is called while preparing the response or if Cloud Connector fails to allocate the required resources.
 * 
 * The @ref connector_request_id_sm_cli_status "CLI status" @ref connector_callback_t "callback"
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
 *   <td>@endhtmlonly @ref connector_class_id_short_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_sm_cli_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_cli_status_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>transport</i></b>, a method on which CLI request is received </li>
 *       <li><b><i>user_context</i></b>, is the user owned context pointer </li>
 *       <li><b><i>status</i></b>, reason code for the end of CLI session </li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @section pending_data  Pending data callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_more_data "pending data"
 * @ref connector_callback_t "callback" to indicate the application that there are pending messages
 * on Device Cloud. Application can send any messages (ping if no data to send) to retreive
 * the queued messages.
 *
 * @note This callback may be called only if @ref CONNECTOR_SM_BATTERY is defined.
 *
 * The @ref connector_request_id_sm_more_data "pending data" @ref connector_callback_t "callback"
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
 *   <td>@endhtmlonly @ref connector_class_id_short_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_sm_more_data @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_more_data_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>transport</i></b>, a method on which the pending data can be retreived </li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr>
 *   <th colspan="2" class="title">Return Values</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Values</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @section opaque_response  Opaque response callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_opaque_response "opaque response"
 * @ref connector_callback_t "callback" to indicate the application that it received a response from
 * Device Cloud for which no associated request is available. The reason for this is the session is
 * terminated either because of the timeout specified in @ref CONNECTOR_SM_TIMEOUT or it is
 * @ref cancel_session "cancelled" by the user.
 *
 * The @ref connector_request_id_sm_opaque_response "pending data" @ref connector_callback_t "callback"
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
 *   <td>@endhtmlonly @ref connector_class_id_short_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_sm_opaque_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_opaque_response_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>transport</i></b>, a method on which the response is received </li>
 *       <li><b><i>id</i></b>, request ID associated with the response </li>
 *       <li><b><i>data</i></b>, received payload </li>
 *       <li><b><i>bytes_used</i></b>, number of bytes filled in the data </li>
 *       <li><b><i>error</i></b>, will be set to connector_true if this is an error response </li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr>
 *   <th colspan="2" class="title">Return Values</th>
 * </tr>
 * <tr>
 *   <th class="subtitle">Values</th> <th class="subtitle">Description</th>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @section cancel_session  Cancel request
 *
 * The application initiates the cancel session request to Cloud Connector by calling connector_initiate_action()
 * with @ref connector_initiate_session_cancel request and @ref connector_sm_cancel_request_t request_data.
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
 *   <td>@endhtmlonly @ref connector_initiate_session_cancel @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_cancel_request_t @htmlonly structure, where member:
 *      <ul>
 *         <li><b><i>transport</i></b>, a method to use to send data </li>
 *         <li><b><i>user_context</i></b>, the last user context used on this session </li>
 *      </ul>
 *   </td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @htmlinclude terminate.html
 */
