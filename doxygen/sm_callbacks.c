/*! @page sm_callbacks Short Message Callbacks
 *
 * @htmlinclude nav.html
 *
 * @section sm_callbacks_overview SM callbacks Overview
 *
 * The SM callbacks APIs are used to send and receive short messages to and from the iDigi Device Cloud.
 * The SM transfers are either communicated over SMS or over UDP transport methods. The following requests
 * under @ref connector_class_sm are listed below:
 * 
 * @li @ref cli_request : Carries the CLI command from the iDigi device cloud. The CLI can be initiated either
 * from iDigi connect manager or from a web service client connected to the iDigi Device Cloud.
 * @li @ref server_to_device_config : This is iDigi Device Cloud originated message. The Cloud will use this
 * message to inform the device about the changes in its phone number and/or service ID. This feature is only
 * supported in SMS transport method.
 * @li @ref device_to_server_config : This is device originated message. The device will use this message to
 * communicate its configuration to the iDigi Device Cloud. This feature is only supported in SMS transport
 * method.
 * @li @ref more_data : This callback indicates that there are more data pending on the iDigi Device Cloud
 * for this device.
 * @li @ref session_complete : This callback is called for a device originated request where the response needed
 * flag is not set or when user cancels the session or when the session encounters any internal error.
 * @li @ref opaque_response : This callback is called when a valid response is received from the
 * iDigi Device Cloud,but no matching request is available in iDigi Connector.
 *
 * @section cli_request CLI Request
 *
 * The CLI request is initiated by iDigi device cloud. The user will receive the application
 * callback (@ref connector_sm_cli_request) when a CLI request is received by iDigi
 * Connector. The application callback is called with response_data point to @ref connector_sm_cli_request_t "CLI request"
 * data structure. Apart from processing the CLI command, user need to update the user_context, so that subsequent
 * total length and response callbacks will carry that context.
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
 *   <td>@endhtmlonly @ref connector_class_sm @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_sm_cli_request @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> [IN] NULL pointer </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Zero </td>
 * </tr>
 * <tr>
 * <td>response_data</td>
 *   <td>[IN/OUT] Pointer to @endhtmlonly connector_sm_cli_request_t @htmlonly structure, where member:
 *      <ul>
 *        <li><b><i>user_context</i></b> [OUT] is the application context user can update and will be returned when callback to get the response is called </li>
 *        <li><b><i>buffer</i></b> [IN] pointer to received CLI command </li>
 *        <li><b><i>bytes</i></b> [IN] number of bytes in the buffer (CLI command length) </li>
 *        <li><b><i>flags</i></b> [IN] Will be set to CONNECTOR_MSG_RESPONSE_NOT_NEEDED if no response is needed </li>
 *        <li><b><i>status</i></b> of the CLI command execution </li>
 *      </ul></td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] size of @endhtmlonly connector_sm_cli_request_t @htmlonly structure </td>
 * </tr>
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
 *   <td>Aborts the iDigi connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * The iDigi Connector will call the callback to get the CLI response if the response is requested.
 * The first callback for the response will be with type connector_data_service_type_total_length,
 * to get the size of the total CLI response.
 *
 * This will follow the callback with request ID @ref connector_sm_cli_request with type connector_data_service_type_need_data
 * to get the entire CLI response. The response_data will point to @ref connector_sm_cli_request_t "CLI response"
 * data structure.
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
 *   <td>@endhtmlonly @ref connector_class_sm @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_sm_cli_request @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> [IN] NULL pointer </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Zero </td>
 * </tr>
 * <tr>
 * <td>response_data</td>
 *   <td>[IN/OUT] Pointer to @endhtmlonly connector_sm_cli_request_t @htmlonly structure, where member:
 *      <ul>
 *        <li><b><i>user_context</i></b> is the application context returned during CLI request callback</li>
 *        <li><b><i>buffer</i></b> [OUT] pointer to write the response to. The iDigi Connector owns this buffer</li>
 *        <li><b><i>bytes</i></b> [IN] length of the response buffer in byte, [OUT] actual bytes written to response buffer</li>
 *        <li><b><i>flags</i></b> [IN] not used </li>
 *        <li><b><i>status</i></b> of the CLI command execution </li>
 *      </ul></td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] size of @endhtmlonly connector_sm_cli_request_t @htmlonly structure </td>
 * </tr>
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
 *   <td>Aborts the iDigi connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * A user uses the SCI web service to send a CLI command to the iDigi Device Cloud, which
 * in turn sends it to the device. An example of an application callback for a CLI command
 * is shown below:
 *
 * @code
 * // The code goes here.
 * @endcode
 *
 * @section device_to_server_config Device to server config request
 *
 * @subsection initiate_device_config Initiate the device to server config request
 *
 * The application initiates the device to server config request to the iDigi Device Cloud by
 * calling connector_initiate_action() with @ref connector_initiate_config_message request and
 * @ref connector_device_to_server_config_t "configuration" as request_data. This message is
 * supported only under SMS transport method.
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
 *   <td>@endhtmlonly @ref connector_initiate_config_message @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> Pointer to @endhtmlonly connector_device_to_server_config_t @htmlonly structure, where member:
 *        <ul>
 *        <li><b><i>transport</i></b> is the communication method used to send the device to server config request </li>
 *        <li><b><i>flags</i></b> is set to @endhtmlonly @ref CONNECTOR_DATA_RESPONSE_NOT_NEEDED @htmlonly if no response is needed</li>
 *        <li><b><i>user_context</i></b> is the application context which will be returned in the response</li>
 *        <li><b><i>sim_slot</i></b> is the sim slot number, 0: unknown, 1: first slot, 2: second slot, rest: reserved </li>
 *        <li><b><i>identifier</i></b> A null-terminated string containing an identifier for the current SIM.
 *                                     This will normally be set as the ICCID of the SIM. It is an optional field, unused case
 *                                     need to use empty string.</li>
 *        </ul></td>
 * </tr>
 * <tr>
 *   <td>response_data </td>
 *   <td> NULL </td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * An example of initiating a device to server config request is shown below:
 * @code
 * //TBD: The device to server request code goes here.
 * @endcode
 *
 * This example will invoke the iDigi Connector to send device to server config request to the
 * iDigi Device Cloud. If the response needed is set, the application callback is called with
 * the server response or error in case the iDigi Connector fails to send the request.
 *
 * @subsection device_config_response Callback with device config response
 *
 * After calling connector_initiate_action(), the iDigi connector will prepare and send the device config request
 * to iDigi Device Cloud asynchronously.  Upon receiving the response, it makes @ref connector_sm_device_to_server_config
 * "Device config response" @ref connector_callback_t "callback" to pass the response to the application. Application is free to
 * release the allocated resources at this point.
 *
 * The response from the server is received as @ref connector_message_status_response_t "status" in the request_data. Typically
 * the response consists of success or error status code and an optional text message in case of error.
 *
 * The @ref connector_sm_device_to_server_config "Device config response" @ref connector_callback_t "callback" is called
 * with the following information:
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
 *   <td>@endhtmlonly @ref connector_class_sm @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_sm_device_to_server_config @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_message_status_response_t @htmlonly structure: <br></br>
 *     <ul>
 *       <li><b><i>user_context</i></b> the user context passed to the @endhtmlonly connector_initiate_action() @htmlonly with
 *          @endhtmlonly @ref connector_initiate_config_message @htmlonly call</li>
 *       <li><b><i>status</i></b> @endhtmlonly @ref connector_session_status_t @htmlonly returned from the iDigi device cloud
 *          or any error while preparing/sending device to server config request</li>
 *       <li><b><i>error_text</i></b> an optional error reason string</li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Size of @endhtmlonly @ref connector_message_status_response_t "status" @htmlonly</td>
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
 * <td>Aborts iDigi connector</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 * <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * An example of an application callback for a device to server config request is shown below:
 *
 * @code
 * // TBD: The sample device to server config callback code goes here.
 * @endcode
 *
 *
 * @section server_to_device_config Server to device config request
 *
 * The server to device config request is initiated by the iDigi Device Cloud. The user will receive
 * an application callback (@ref connector_sm_server_to_device_config) when a server to device
 * config request is received by the iDigi Connector.
 *
 * The request_data of this callback will point to @ref connector_server_to_device_config_t "server config", the response_data
 * field will carry the pointer to @ref connector_status_t "status". Set the status to connector_success on success
 * and in case of error set it accordingly.
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
 *   <td>@endhtmlonly @ref connector_class_sm @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_sm_server_to_device_config @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>[IN/OUT] Pointer to @endhtmlonly connector_server_to_device_config_t @htmlonly structure, where member:
 *        <ul>
 *        <li><b><i>phone_number</i></b> A null-terminated string, holds the iDigi Device Cloud phone number</li>
 *        <li><b><i>service_id</i></b> A null-terminated string, holds the iDigi Device Cloud service ID</li>
 *        </ul></td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] size of @endhtmlonly connector_server_to_device_config_t @htmlonly </td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td>[OUT] pointer to @endhtmlonly @ref connector_status_t @htmlonly </td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] Size of connector_status_t </td>
 * </tr>
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
 *   <td>Aborts the iDigi connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * A user uses the SCI web service to send a server to device config request to the iDigi Device Cloud,
 * which in turn sends it to the device. An example of an application callback for a server to device
 * config request is shown below:
 *
 * @code
 *  // TBD: The sample server to device config callback code goes here.
 * @endcode
 *
 * @section more_data More pending data
 *
 * The iDigi Connector calls this callback to indicate the application that there are pending messages
 * on the iDigi Device Cloud. Application can send any messages (ping if no data to send) to retreive
 * the queued messages. Both request_data and response_data are empty in this callback.
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
 *   <td>@endhtmlonly @ref connector_class_sm @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_sm_more_data @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> [IN] NULL </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] 0 </td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td> [OUT] NULL </td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] 0 </td>
 * </tr>
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
 *   <td>Aborts the iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @code
 *  // TBD: The sample code for more_data callback goes here.
 * @endcode
 *
 * @section session_complete Session complete callback
 *
 * The iDigi Connector will use this callback to indicate the application when:
 *
 *   @li The requested send operation is complete for a session where response is not requested.
 *   @li Internal error occurs in a session.
 *   @li User requested session cancel completes.
 *
 * The application is free to release all the allocated resources at this time.
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
 *   <td>@endhtmlonly @ref connector_class_sm @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_sm_session_complete @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_message_status_response_t @htmlonly structure: <br></br>
 *     <ul>
 *       <li><b><i>user_context</i></b> the user context passed to the @endhtmlonly connector_initiate_action() @htmlonly call </li>
 *       <li><b><i>status</i></b> @endhtmlonly @ref connector_session_status_t @htmlonly indicates the session status when this
 *                 call is made. </li>
 *       <li><b><i>error_text</i></b> an optional error reason string</li>
 *     </ul>
 *   </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Size of @endhtmlonly @ref connector_message_status_response_t "status" @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td> [OUT] NULL </td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] 0 </td>
 * </tr>
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
 *   <td>Aborts the iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @code
 *  // TBD: The sample code for session_complete callback goes here.
 * @endcode
 *
 * @section opaque_response Cloud response for deleted request.
 *
 * When the iDigi Device Cloud sends a response for a request, which is deletd by the user either due to timeout
 * or by issuing the cancel, the iDigi Connector will call the application with callback (@ref connector_sm_opaque_response).
 *
 * The response_data of this callback will point to @ref connector_sm_opaque_response_t "opaque response".
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
 *   <td>@endhtmlonly @ref connector_class_sm @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_sm_opaque_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> [IN] NULL </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] 0 </td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td>[IN/OUT] Pointer to @endhtmlonly connector_sm_opaque_response_t @htmlonly structure, where member:
 *      <ul>
 *        <li><b><i>id</i></b> an identifier to keep track of multi-part message</li>
 *        <li><b><i>data</i></b> pointer to a buffer which contains opaque data </li>
 *        <li><b><i>bytes</i></b> number of bytes in data buffer </li>
 *        <li><b><i>flags</i></b> can be used to identify the last part (if CONNECTOR_MSG_LAST_DATA is set) </li>
 *        <li><b><i>status</i></b> response status, application can use this to discard the subsequent parts by returning an error </li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] Size of connector_sm_opaque_response_t </td>
 * </tr>
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
 *   <td>Aborts the iDigi connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @htmlinclude terminate.html
 */
