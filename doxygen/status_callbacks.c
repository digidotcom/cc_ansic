/*! @page status_callbacks Status Callbacks
 *
 * @htmlinclude nav.html
 *
 * @section status_callbacks_overview Status callbacks Overview
 *
 * The status callbacks APIs are used to send and receive status messages to and from the iDigi Device Cloud and to notify
 * application when TCP communication is established, keep alive message is not received from the iDigi Device Cloud, keep alive
 * message is restored and received from the iDigi Device Cloud, and Etherios Cloud Connector stops running.
 *
 * The following requests under @ref connector_class_id_status are listed below:
 *
 * @li @ref cloud_status
 * - A device originated request to get the iDigi Device Cloud status. If the transport layer is UDP,
 * then this ping message can be used to trigger the delivery of pending/queued messages on the iDigi Device Cloud. This
 * feature is only supported in UDP and SMS transport methods.
 *
 * @li @ref device_status
 * - This request is originated by the iDigi Device Cloud to get the device status. This
 * feature is only supported in UDP and SMS transport methods.
 *
 * @li @ref status_tcp
 * - This callback is called to notify the application that TCP communication has been established,
 * a keep-alive message was not received, or keep-alive message was received and recovered.
 *
 * @li @ref status_stop_completed
 * - This callback is called to notify the application that Etherios Cloud Connector has stopped a transport running.
 *
 * @section cloud_status Cloud status request
 *
 * @subsection initiate_ping Initiate the ping/cloud status request
 *
 * The application initiates the Ping/status request to the iDigi Device Cloud by calling connector_initiate_action()
 * with @ref connector_initiate_ping_request request and @ref connector_message_status_request_t "status request"
 * as request_data.
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
 *   <td> Pointer to @endhtmlonly connector_message_status_request_t @htmlonly structure, where member:
 *        <ul>
 *        <li><b><i>transport</i></b> is the communication method used to send the ping to the iDigi Device Cloud</li>
 *        <li><b><i>flags</i></b> is set to @endhtmlonly @ref CONNECTOR_DATA_RESPONSE_NOT_NEEDED @htmlonly if no response is needed</li>
 *        <li><b><i>user_context</i></b> is the application context which will be returned in the response</li>
 *        </ul></td>
 * </tr>
 * <tr>
 *   <td>response_data </td>
 *   <td> NULL </td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * An example of initiating a ping request is shown below:
 * @code
 * //TBD: The ping request code goes here.
 * @endcode
 *
 * This example will invoke the iDigi Connector to initiate a ping request to the iDigi Device
 * Cloud. If the response needed is set, the application callback is called with the server
 * response or error in case the iDigi Connector fails to send the request.
 *
 * @subsection ping_response Callback with ping_status response
 *
 * After calling connector_initiate_action(), the iDigi Connector will prepare and send the ping request
 * to iDigi Device Cloud asynchronously.  Upon receiving the response, it makes @ref connector_status_ping_response "Status response"
 * @ref connector_callback_t "callback" to pass the response to the application. Application is free to release the allocated
 * resources at this point.
 *
 * The response from the server is received as @ref connector_message_status_response_t "status response" in request_data.
 * Typically the response consists of the success or error status code and an optional text message in case of error.
 *
 * The @ref connector_status_ping_response "status response"  @ref connector_callback_t "callback" is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_class_id_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_status_ping_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>[IN] pointer to @endhtmlonly connector_message_status_response_t @htmlonly structure:
 *     <ul>
 *       <li><b><i>user_context</i></b> set to the user_context passed to the @endhtmlonly connector_initiate_action() @htmlonly with
 *             @endhtmlonly @ref connector_initiate_ping_request @htmlonly call</li>
 *       <li><b><i>status</i></b> response @endhtmlonly @ref connector_session_status_t @htmlonly returned from the iDigi device cloud
 *             or any error while preparing/sending ping request</li>
 *       <li><b><i>error_text</i></b> an optional error reason string</li>
 *   </td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Size of @endhtmlonly connector_message_status_response_t @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td>[OUT] NULL </td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] 0 </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * An example of an application callback for a ping request is shown below:
 *
 * @code
 * // TBD: The sample ping callback code goes here.
 * @endcode
 *
 * @section device_status Device status request
 *
 * The device status request is initiated by the iDigi Device Cloud. The user will receive an application
 * callback (@ref connector_status_ping_request) when a device status request is received by the iDigi Connector.
 *
 * The response_data field will carry the pointer to @ref connector_status_t "status". Set the status to connector_success
 * on success or in case of error set it accordingly.
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
 *   <td>@endhtmlonly @ref connector_class_id_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_status_ping_request @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>NULL</td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> 0 </td>
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
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * A user uses the SCI web service to send a device status request to the iDigi Device Cloud,
 * which in turn sends it to the device. An example of an application callback for a device
 * status request is shown below:
 *
 * @code
 *  // TBD: The sample device status request callback code goes here.
 * @endcode




 * @section status_tcp TCP Status
 *
 * This callback is called to notify application the status of TCP communication.
 *
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
 *   <td>@endhtmlonly @ref connector_class_id_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_status_tcp @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> Pointer to one of the following @endhtmlonly @ref connector_tcp_status_t @htmlonly status:
 *        <dl>
 *        <dt><b>connector_tcp_communication_started</b> </dt>
 *        <dd> - TCP communication has been established. User and application is able to start message and data transfer. </dd>
 *        <dt><b>connector_tcp_keepalive_missed</b>
 *        <dd> - Etherios Cloud Connector has not received a keep alive message from the iDigi Device Cloud within the time specifed in @endhtmlonly @ref connector_config_tx_keepalive or @ref CONNECTOR_TX_KEEPALIVE_IN_SECONDS @htmlonly </dd>
 *        <dt><b>connector_tcp_keepalive_restored</b>
 *        <dd> - Digi connector has received and recovered missed keep alive message from the iDigi Device Cloud. </dd>
 *        </dl></td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Size of @endhtmlonly @ref connector_tcp_status_t @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td>[OUT] NULL </td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] N/A </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @section status_stop_completed Stop Completion
 *
 * This callback is called when Etherios Cloud Connector has stopped running. It's invoked from connector_initiate_action() call on @ref connector_initiate_transport_stop.
 * When connector_initiate_action() is called, this calblack will be called after the specified transport has stopped running.
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
 *   <td>@endhtmlonly @ref connector_class_id_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_status_stop_completed @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> Pointer to @endhtmlonly connector_initiate_stop_request_t @htmlonly structure, where member:
 *        <ul>
 *        <li><b><i>transport</i></b>
 *        <ul><li> @endhtmlonly @ref connector_transport_tcp @htmlonly </li>
 *            <li> @endhtmlonly @ref connector_transport_udp @htmlonly </li>
 *            <li> @endhtmlonly @ref connector_transport_sms @htmlonly </li>
 *            <li> @endhtmlonly @ref connector_transport_all @htmlonly </li> </ul>
 *        </li>
 *        <li><b><i>condition</i></b> Not used
 *        </li>
 *        <li><b><i>user_context</i></b> User's context from @endhtmlonly connector_initiate_action() @htmlonly call.</li>
 *        </ul></td>
 * </tr>
 * <tr>
 *   <td>request_length</td>
 *   <td> [IN] Size of @endhtmlonly connector_initiate_stop_request_t @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>response_data</td>
 *   <td>[OUT] NULL </td>
 * </tr>
 * <tr>
 *   <td>response_length</td>
 *   <td>[OUT] N/A </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *   <td>Continue</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Etherios Cloud Connector</td>
 * </tr>
 * <tr>
 *   <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 *
 * @htmlinclude terminate.html
 */
