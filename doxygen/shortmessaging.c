/*! @page shortmessaging Short Messaging
 *
 * @htmlinclude nav.html
 *
 * @section secsmintroduction Introduction
 *
 * Short Messaging (SM) provides an alternative transport for communicating with the cloud and is 
 * typically used only when data usage is costly and guaranteed data delivery is unnecessary. SM 
 * uses UDP as a transport rather than TCP transport normally used in device-to-cloud communications.
 *
 * TCP communication includes overhead to establish, maintain, and close connections as well as 
 * overhead to ensure delivery and integrity. The communication is reliable and can be 
 * @ref network_ssl_callbacks "secured". Most cloud communications discussed in this guide leverage
 * standard TCP communications.
 * 
 * SM communicates over UDP and therefore does not incur the TCP connection overhead. 
 * Unfortunately, UDP includes limitations such as unreliable delivery and the potential for 
 * duplicate packets. Applications that include SM will require consideration and
 * handling of these UDP limitations - a price you may choose to pay to reduce data usage on costly
 * networks such as cellular or satellite.
 *
 * @section smsectionwhoneedsit Should I use Short Messaging?
 * 
 * Most devices will be deployed on networks with few restrictions on data usage. For these 
 * applications, TCP should be used and is the method demonstrated in all of the samples included in 
 * this kit. These applications will establish a TCP connection and maintain that connection so that 
 * the device is always securely accessible from the cloud.
 * 
 * Conversely, some devices could be deployed on networks that include data restrictions. Others may 
 * be battery-powered and must minimize active network time. For this latter group, Short Messaging 
 * should be considered.
 *
 * @subsection smsolutions How does Short Messaging solve my problem?
 *
 * A device need not always have a TCP connection.  Further, some devices can eliminate
 * this connection altogether.  For example, a cellular based device might send GPS data to
 * Device Cloud every few hours and then check for remote instructions.  Based on the instruction,
 * the application could then start a TCP connection and perform a standard Cloud Connector
 * operation.
 *
 * Alternatively, a battery based device might power up, read a sensor,
 * send the data to Device Cloud, check for pending @ref pending_data "Web Service" instructions,
 * and then shut down if no instructions are queued.
 *
 * In these two examples, minimizing network traffic is essential.  In the latter example,
 * shutting power as soon as possible is critical.
 *
 * The system requirements of your application will have radical effects on your SM design.
 * You can always optimize the power consumption, or network traffic, but this
 * comes with the additional application overhead.
 *
 * @subsection smcomplications How does Short Messaging complicate my application?
 *
 * <b>UDP Reliability </b>
 *
 * Depending on what you optimize, a device will need to consider
 * the unreliability of UDP.
 *
 * Suppose your application collects data periodically and sends it to Device Cloud once
 * per hour.  What if the data is lost?  Can your system tolerate this?  If not, you'll
 * need your application to check for acknowledgments.  After some period of time
 * you'll have to timeout and re-send the data.  This adds complexity to your Cloud
 * Connector application.
 *
 * Now consider your @ref web_services Application that pulls this data from Device Cloud.
 * What happens if the data sent from the last sample was received by Device Cloud, but the
 * acknowledgment was lost?  Now your data is duplicated on Device Cloud and your
 * @ref web_services application will need to handle this case.
 *
 * Suppose data integrity is a critical requirement.  In this case, a Cloud Connector
 * application might choose to start a TCP connection and send the data reliably
 * instead of using the UDP transport.
 *
 * Cloud Connector has the flexibility to support either case.
 *
 * <b>Punching through Firewalls</b>
 *
 * Considering most devices are deployed behind a Firewall, Device Cloud is incapable of
 * initiating an SM message exchange.  All @ref web_services "web service" SM requests are queued
 * on Device Cloud, waiting for a Cloud Connector message.
 *
 * Cloud Connector @b must @b always @b initiate SM exchanges.  A key component of
 * your implementation must include a strategy to open the firewall and
 * drain the SM requests.
 *
 * If Cloud Connector has no pending message to send, the application can @ref initiate_ping_to_cloud.
 *
 * @section smsectionexamples Short Messaging Features
 *
 * The SM protocol includes four major functions:
 *      -# @ref initiate_ping_to_cloud "Initiate Ping" to Device Cloud to open the Firewall between Cloud Connector and Device Cloud
 *      -# @ref data_service "Data transfer" between Cloud Connector and Device Cloud
 *      -# @ref cli_support "Command Line Interface" support for Device Manager
 *      -# @ref sm_connect "Request Connection" from Device Cloud to start the Cloud Connector TCP transport
 *
 * An additional major function is available for @ref CONNECTOR_SM_BATTERY "Battery-backed" SM applications:
 *      -# @ref pending_data "Message pending" to notify applications more messages are queued.
 *
 * Several @ref additional_apis "SM convenience APIs" are also described below.
 *
 * @section ping_request Ping Device Cloud
 *
 * A device can send a @ref initiate_ping_to_cloud "Ping request to Device Cloud" at any
 * time.  The purpose is to open a hole in a firewall and let Device Cloud know that the Application is
 * ready to receive pending or queued messages.
 *
 * @subsection initiate_ping_to_cloud  Initiate Ping Device Cloud
 *
 * When an application has only @ref network_udp_start "UDP started" (TCP has @ref network_tcp_start
 * "not started"), it's necessary to periodically contact Device Cloud so to open a hole in a firewall
 * and receive data or commands from Device Cloud or @ref web_services applications.  This can be achieved
 * by initiating a Ping Device Cloud.
 *
 * The application initiates a Ping Device Cloud by calling @ref connector_initiate_action()
 * using a @ref connector_initiate_ping_request type and a connector_sm_send_ping_request_t
 * request_data structure:
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
 *   <td>@endhtmlonly @ref connector_handle_t "Handle" returned from the connector_init() @htmlonly function.</td>
 * </tr>
 * <tr>
 *   <td>request</td>
 *   <td>@endhtmlonly @ref connector_initiate_ping_request @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td>Pointer to @endhtmlonly connector_sm_send_ping_request_t @htmlonly structure, where member:
 *      <ul>
 *        <li><b><i>transport</i></b>: @endhtmlonly Should be set to @ref connector_transport_udp. @htmlonly </li>
 *        <li><b><i>user_context</i></b>: An opaque application-defined context.  This pointer is returned
 *                                        during a @endhtmlonly @ref ping_response_callback "response callback". @htmlonly </li>
 *        <li><b><i>response_required</i></b>: Set to @endhtmlonly @ref connector_true if notification of ping receipt is needed,
 *                                             otherwise set to @ref connector_false.  See @ref ping_response_callback . @htmlonly </li>
 *      </ul>
 *    </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_success @htmlonly</th>
 *   <td>Device Cloud Ping initiated</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_unavailable @htmlonly</th>
 *   <td>Cloud Connector is not running (not started or stopped) </td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_service_busy @htmlonly</th>
 *   <td>Previous SM request is not yet processed</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 *
 * An example of initiating a ping is shown below:
 * @code
 *   static connector_sm_send_ping_request_t ping_request;
 *
 *   ping_request.transport = connector_transport_udp;
 *   ping_request.response_required = connector_true;
 *
 *   // Send ping to Device Cloud
 *   status = connector_initiate_action(handle, connector_initiate_ping_request, &ping_request);
 * @endcode
 *
 * This example will invoke Cloud Connector to initiate a ping request to Device Cloud. The completion
 * of this request is indicated by a @ref ping_response_callback "response callback".
 *
 * @note The ping_request variable must remain in scope or allocated until the @ref ping_response_callback "response" callback. 
 *
 * @see @ref connector_initiate_action()
 * @see @ref ping_response_callback
 *
 * @subsection ping_response_callback  Ping Device Cloud Response callback
 *
 * The ping_response_callback is made for one of the following conditions: 
 * the ping was successfully sent or a ping response has been received
 * from the cloud, the ping timed out, or an error occurred. 
 *
 * @note When @ref initiate_ping_to_cloud operation has the response_required set @ref connector_false, this callback
 * is made with the @ref connector_sm_ping_response_t status set to @b connector_sm_ping_status_complete when the
 * message is sent.
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
 *       <li><b><i>transport</i></b>: @endhtmlonly Will be set to @ref connector_transport_udp. @htmlonly </li>
 *       <li><b><i>user_context</i></b>: The user_context pointer from the  @endhtmlonly @ref initiate_ping_to_cloud. @htmlonly </li>
 *       <li><b><i>status</i></b>: The response code, where @endhtmlonly @b connector_sm_ping_status_success indicates reply
 *                 received, @b connector_sm_ping_status_complete indicates @ref initiate_ping_to_cloud sent. @htmlonly </li>
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
 *   <td>@endhtmlonly @ref connector_callback_unrecognized @htmlonly</td>
 *   <td>Application doesn't implement this callback</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 * <td>Aborts Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @note The response code connector_sm_ping_status_success will only be received if the @ref initiate_ping_to_cloud request has the 
 * @b response_required set to @ref connector_true and a ping response has been received from the cloud. 
 *
 * @see @ref initiate_ping_to_cloud
 *
 *
 * @section pending_data  Pending Data Available
 *
 * Cloud Connector will make a @ref connector_request_id_sm_more_data @ref connector_callback_t "callback"
 * to notify the application that additional SM messages are pending.   Applications should
 * call @ref initiate_ping_to_cloud to retrieve pending messages from Device Cloud.
 *
 * This callback is geared towards @ref CONNECTOR_SM_BATTERY "Battery-backed" Applications.   This callback
 * can be used to re-trigger a timer which initiates a power down sequence.  When this callback occurs, the
 * timer should be reset, extending the time before shutdown.
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
 *       <li><b><i>transport</i></b>: @endhtmlonly Will be set to @ref connector_transport_udp. @htmlonly </li>
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
 * @note This callback is made only when @ref CONNECTOR_SM_BATTERY is defined.
 *
 * @see @ref initiate_ping_to_cloud
 * @see @ref CONNECTOR_SM_BATTERY "Handling Battery-backed Applications"
 *
 * @section cli_support Command Line Interface Support
 *
 * Cloud Connector includes support for a Command Line Interface (CLI) support to be displayed on the Device
 * Cloud Device Manager CLI.
 *
 * @subsection cli_cb_sequence CLI Callback Sequence
 * The following SM callbacks are made (in sequence) to support an individual CLI request:
 *
 *      -# @ref cli_request_callback
 *      -# @ref cli_response_length_callback
 *      -# @ref cli_response_callback
 *
 *      If an error is encountered during the CLI Callback Sequence, this additional CLI callback is made:
 *      -# @ref cli_status_callback
 *
 * @subsection cli_request_callback  CLI request callback
 *
 * The @ref connector_request_id_sm_cli_request callback is the initial call
 * in a @ref cli_cb_sequence.  This callback is triggered by a Device Manager CLI request and it
 * passes the Device Cloud command and arguments to
 * the Application in a connector_sm_cli_request_t data structure.
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
 *       <li><b><i>transport</i></b>: @endhtmlonly Will be set to @ref connector_transport_udp. @htmlonly </li>
 *       <li><b><i>user_context</i></b>: An opaque application-defined context.  This pointer is passed to all subsequent
 *                                        callbacks during the CLI sequence for this command.
 *       <li><b><i>buffer</i></b>: Buffer containing the Device Cloud CLI command. </li>
 *       <li><b><i>bytes_used</i></b>: Number of bytes used for the CLI command in the buffer. </li>
 *       <li><b><i>response_required</i></b>: Set to @endhtmlonly @ref connector_true if a response
 *                                            is requested for this CLI command.  @ref connector_false if no
 *                                            response required.  @htmlonly </li>
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
 *   <td>@endhtmlonly @ref connector_callback_unrecognized @htmlonly</td>
 *   <td>Application doesn't implement this callback</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_error @htmlonly</th>
 *   <td>Application encountered error</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @see @ref cli_response_length_callback
 * @see @ref cli_response_callback
 * @see @ref cli_status_callback
 *
 * @subsection cli_response_length_callback  Response length callback
 *
 * The @ref connector_request_id_sm_cli_response_length callback is the second call
 * in a @ref cli_cb_sequence and used to get the maximum size of CLI response length in bytes.
 * This callback is made only if the @b response_required was set to @ref connector_true in
 * the initial @ref cli_request_callback call.
 *
 * Cloud Connector will allocate this memory block to process the ensuing @ref cli_request_callback
 * sequence.
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
 *       <li><b><i>transport</i></b>: @endhtmlonly Will be set to @ref connector_transport_udp. @htmlonly </li>
 *       <li><b><i>user_context</i></b>: The opaque application-defined context passed in from the @endhtmlonly @ref cli_request_callback @htmlonly </li>
 *       <li><b><i>total_bytes</i></b>: Maximum size in bytes of the CLI response to the initial @endhtmlonly @ref cli_request_callback @htmlonly </li>
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
 *   <td>@endhtmlonly @ref connector_callback_unrecognized @htmlonly</td>
 *   <td>Application doesn't implement this callback</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *   <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @note The actual bytes used during @ref cli_response_callback sequence must be less than or equal
 * to the @b total_bytes set by this callback.
 *
 * @see @ref cli_request_callback
 * @see @ref cli_response_callback
*
 * @subsection cli_response_callback  CLI response callback
 *
 * The @ref connector_request_id_sm_cli_response callback is the third call
 * in a @ref cli_cb_sequence and is used to assemble a CLI response to send to Device Cloud.
 * This callback is made only if the @b response_required was set to @ref connector_true in
 * the initial @ref cli_request_callback call and after the @b cli_response_length_callback was
 * called to define the maximum CLI response size.
 *
 * Cloud Connector will continue to make @ref connector_request_id_sm_cli_response callbacks
 * until the @b more_data field is set to @ref connector_false.
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
 *       <li><b><i>transport</i></b>: @endhtmlonly Will be set to @ref connector_transport_udp. @htmlonly </li>
 *       <li><b><i>user_context</i></b>: The opaque application-defined context passed in from the @endhtmlonly @ref cli_request_callback @htmlonly </li>
 *       <li><b><i>buffer</i></b>: the memory to copy your CLI response. </li>
 *       <li><b><i>bytes_available</i></b>: buffer size available in bytes. </li>
 *       <li><b><i>bytes_used</i></b>: number of bytes copied by the callback.</li>
 *       <li><b><i>more_data</i></b>: Set to @endhtmlonly @ref connector_true if more data expected.
 *                                    @ref connector_false if your response is complete.  @htmlonly
 *       </li>
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
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_error @htmlonly</th>
 *   <td>Application encountered error</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @note This callback is used to fill a private buffer allocated after the @ref cli_response_length_callback.
 * The initial size of the @b bytes_available will reflect that size and get decremented for subsequent callbacks.
 * The total number of @b bytes_used for all these callbacks must not exceed the maximum length set by cli_response_length_callback.
 *
 * @see @ref cli_request_callback
 * @see @ref cli_response_length_callback
 * @see @ref cli_status_callback
 *
 * @subsection cli_status_callback  CLI session error callback
 *
 * The @ref connector_request_id_sm_cli_status callback is made when an unexpected CLI error occurs during
 * a @ref cli_cb_sequence.  This error can occur if the @ref connector_initiate_stop_request_t "stop transport"
 * was initiated while processing a CLI request or if Cloud Connector fails to allocate the required resources.
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
 *       <li><b><i>transport</i></b>: @endhtmlonly Will be set to @ref connector_transport_udp. @htmlonly </li>
 *       <li><b><i>user_context</i></b>: The opaque application-defined context passed in from the @endhtmlonly @ref cli_request_callback @htmlonly </li>
 *       <li><b><i>status</i></b>: @endhtmlonly
 *                                 @b connector_sm_cli_status_cancel if @ref connector_initiate_stop_request_t "transport stopped"
 *                                 @b connector_sm_cli_status_error if resource allocation failure. @htmlonly
 *       </li>
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
 *   <td>@endhtmlonly @ref connector_callback_unrecognized @htmlonly</td>
 *   <td>Application doesn't implement this callback</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @see @ref cli_request_callback
 * @see @ref cli_response_length_callback
 * @see @ref cli_response_callback
 *
 * @section additional_apis Additional SM APIs
 *
 * The following SM APIs are also available.
 *
 * @subsection sm_connect Request TCP start
 *
 * Requests the Cloud Connector to start it's TCP transport.  This request will
 * be handled in the Cloud Connector private layer will start the TCP.
 *
 * Once the TCP transport @ref network_tcp_start "is started", devices can make use of
 * the TCP features like @ref firmware_download, @ref rci_service, or reliable
 * @ref data_point or @ref data_service over a @ref connector_transport_tcp "TCP transport".
 *
 * The following @ref web_services example shows how to request a device to start it's TCP service:
 *
 * @code
 *    <sci_request version="1.0">
 *      <send_message synchronous="false">
 *        <targets>
 *          <device id="00000000-00000000-00409DFF-FF432311"/>
 *        </targets>
 *        <sm_udp>
 *          <request_connect/>
 *        </sm_udp>
 *      </send_message>
 *    </sci_request>
 * @endcode
 *
 * @note If @ref CONNECTOR_TRANSPORT_TCP "CONNECTOR_TRANSPORT_TCP is disabled" Cloud Connector
 * returns an error response to Device Cloud.
 *
 * @subsection sm_reboot Reboot device
 *
 * Requests a Cloud Connector reboot.  After receiving this request, Cloud Connector
 * will invoke a @ref reboot callback.
 *
 * The following @ref web_services example shows how to reboot a device:
 *
 * @code
 *    <sci_request version="1.0">
 *      <send_message synchronous="false">
 *        <targets>
 *          <device id="00000000-00000000-00409DFF-FF432311"/>
 *        </targets>
 *        <sm_udp>
 *          <reboot/>
 *        </sm_udp>
 *      </send_message>
 *    </sci_request>
 * @endcode
 *
 *
* @subsection opaque_response  Unsequenced Message callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_opaque_response @ref connector_callback_t "callback"
 * to notify an application that a response was received with no known associated request.  The reason
 * for this is either the session  @ref CONNECTOR_SM_TIMEOUT "timed-out", or the transport was
 * @ref connector_initiate_stop_request_t "terminated" and the Message context lost.
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
 *       <li><b><i>transport</i></b>: @endhtmlonly Will be set to @ref connector_transport_udp. @htmlonly </li>
 *       <li><b><i>id</i></b>: request ID associated with the response </li>
 *       <li><b><i>data</i></b>: received payload </li>
 *       <li><b><i>bytes_used</i></b>: number of bytes in the payload </li>
 *       <li><b><i>error</i></b>: is connector_true for error response for Device Cloud </li>
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
 * @see @ref CONNECTOR_SM_TIMEOUT
 *
 * @subsection ping_request_from_cloud  Device Cloud Ping Notification
 *
 * Cloud Connector will make a Ping Request @ref connector_request_id_sm_ping_request "callback" to
 * notify an Application that a ping was received.  This callback is informational only and no action
 * is necessary.
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
 *       <li><b><i>transport</i></b>: @endhtmlonly Will be set to @ref connector_transport_udp. @htmlonly </li>
 *       <li><b><i>response_required</i></b>: Set to @endhtmlonly @ref connector_true if Device Cloud requests a response,
 *                                            @ref connector_false if no response was requested.  @htmlonly </li>
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
 *   <td>@endhtmlonly @ref connector_callback_unrecognized @htmlonly</td>
 *   <td>Application doesn't implement this callback</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @note The @b response_required member is passing information on to the application.  There is no required
 * action necessary from the Cloud Connector Application, regardless of the value for @b response_required.
 *
 *
 * @htmlinclude terminate.html
 */
