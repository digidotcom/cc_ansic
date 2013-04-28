/*! @page shortmessaging Short Messaging
 *
 * @htmlinclude nav.html
 *
 * @section secsmintroduction Introduction
 *
 * TCP/IP communication includes overhead to establish, maintain, and close connections,
 * as well as overhead to ensure delivery and integrity.  The communication is reliable and
 * potentially @ref network_ssl_callbacks "secure".  Most Cloud Connector communication
 * covered in this guide occurs over TCP/IP.
 *
 * The Short Message (SM) protocol was designed to be a lightweight, message based protocol
 * ideal for costly transports or for minimizing traffic.  SM transports over UDP and therefore
 * does not incur the cost TCP/IP connections.  Unfortunately, UDP includes classic limitations
 * such as unreliable delivery, potential duplicate packets, and lack of data integrity.
 * Cloud Connector applications that include SM will require consideration and handling of
 * these UDP limitations.
 *
 * @section smsectionwhoneedsit Should I use Short Messaging?
 * Most Cloud Connector applications will be deployed on networks with few restrictions
 * on data usage.  For these applications, we highly recommend using the samples included in
 * this kit, that were intended for a @ref network_tcp_start "TCP/IP transport".  These applications
 * will establish a @ref app_start_network_tcp "TCP/IP connection" and maintain it throughout the
 * entire application's up time.
 *
 * Conversely, some Cloud Connector applications could be deployed on networks that include
 * data restrictions; or some Cloud Connector applications could be deployed on systems
 * with severe power limitations.
 *
 * For this latter group, Short Messaging should be considered.
 *
 * @subsection smsolutions How does Short Messaging solve my problem?
 *
 * A Cloud Connector application need not always have a TCP/IP connection.  Further, some
 * Cloud Connector applications can eliminate this connection altogether.
 *
 * For example, a cellular based Cloud Connector application might send GPS data to Device Cloud
 * every few hours and then check for remote instructions.  Based on the instruction, the
 * application could then start a TCP/IP connection and perform a standard Cloud Connector
 * operation.
 *
 * Alternatively, a battery based Cloud Connector application might power up, read a sensor,
 * send the data to Device Cloud, check for pending @ref web_services "Web Service" instructions,
 * and then shut down if no instructions are queued.
 *
 * In these two examples, minimizing network traffic is essential.  In the latter example,
 * shutting power as soon as possible is critical.
 *
 * The system requirements of your application will have radical effects on your SM design.
 * You can always optimize one thing, like power consumption, or network traffic, but this
 * comes with the additional application overhead.
 *
 * @subsection smcomplications How does Short Messaging complicate my application?
 *
 * <b>UDP Reliability </b>
 *
 * Depending on what you optimize, a Cloud Connector application will need to consider
 * the unreliability of UDP.
 *
 * Suppose your application samples data periodically and sends to Device Cloud once
 * per hour.  What if the data is lost?  Can your system tolerate this?  If no, you'll
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
 * application might choose to start a TCP/IP connection and send the data reliably
 * instead of using the UDP transport.
 *
 * Cloud Connector has the flexibility to support either case.
 *
 * <b>Punching through Firewalls</b>
 *
 * Considering most devices are deployed behind a Firewall, Device Cloud is incapable of
 * initiating an SM message exchange.  All @ref web_services "web service" requests are queued
 * on Device Cloud and waiting for Device Cloud to receive Cloud Connector message.
 *
 * Cloud Connector @b must @b always @b initiate SM exchanges.  A critical
 * component of your Cloud Connector Application must include a strategy to
 * open the firewall.
 *
 * If Cloud Connector has no pending message to send, the application can
 * @ref initiate_ping_to_cloud.
 *
 * @section smsectionexamples Short Messaging Features
 *
 * The SM protocol includes two categories of features: those requiring Cloud Connector
 * Application extensions and those handled directly by the private Cloud Connector library.
 *
 * Requires Cloud Connector Application extensions:
 *      -# A @ref initiate_ping_to_cloud capability to open a Firewall between Cloud Connector and Device Cloud
 *      -# @ref data_service "Data transfer" between Cloud Connector and Device Cloud
 *      -# @ref cli_support "Command Line Interface" support for Device Manager
 *      -# @ref pending_data "Message pending" to indicate more messages are queued on the Device Cloud for this device.
 *
 * Handled by the Cloud Connector private:
 *      -# @ref sm_connect "Request Connection" from Device Cloud to start the Cloud Connector TCP transport
 *      -# @ref sm_reboot "Reboot" from Device Cloud to Cloud Connector
 *
 * @section ping_request Ping Operations
 *
 * A Cloud Connector Application can send a @ref initiate_ping_to_cloud "Ping request to Device Cloud" at any time.
 * The purpose is to open a hole in a firewall and let Device Cloud know that the Application Cloud Connector is
 * ready to receive pending or queued messages.
 *
 * Additionally, a @ref web_services application can @ref ping_request_from_cloud "search"
 * for a Cloud Connector device by Pinging through Device Cloud.
 *
 * @subsection initiate_ping_to_cloud  Ping Device Cloud
 *
 * When an application has only @ref network_udp_start "UDP started"
 * (TCP has @ref network_tcp_start "not started"), it's necessary to
 * periodically contact Device Cloud so to open a hole in a firewall and receive data or commands
 * from Device Cloud or @ref web_services applications.
 * This can be achieved by initiating a Ping Device Cloud.
 *
 * The application initiates a Ping Device Cloud by calling @ref connector_initiate_action()
 * using a @ref connector_initiate_ping_request type and a connector_sm_send_ping_request_t
 * request_data structure:
 *
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
 *        <li><b><i>transport</i></b>: Ping Request transport.  See @endhtmlonly @ref connector_transport_udp. @htmlonly </li>
 *        <li><b><i>user_context</i></b>: An opaque application defined context.  This pointer is returned
 *                                        during a @endhtmlonly @ref ping_response_callback "response callback" @htmlonly and used
 *                                        to identify the origin of the response. </li>
 *        <li><b><i>response_required</i></b>: Set to @endhtmlonly @ref connector_true if a @ref ping_response_callback " response callback" is required.
 *                                             Set to @ref connector_false when no response callback is required.  @htmlonly </li>
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
 *   ping_request.user_context = &ping_request;
 *
 *   // Send ping to Device Cloud
 *   status = connector_initiate_action(handle, connector_initiate_ping_request, &ping_request);
 * @endcode
 *
 * This example will invoke Cloud Connector to initiate a ping request to Device Cloud. The completion
 * of this request is indicated by a @ref ping_response_callback "response callback".
 *
 * @note The variable ping_request above is not a stack variables. Either you can use a heap or
 * a memory (global or static) variable to hold this value. You can release them when you get a
 * @ref  ping_response_callback "response" callback. The value passed as the user_context will
 * be returned in the response.
 *
 * @subsection ping_response_callback  Ping Device Cloud Response callback
 *
 * When a @ref initiate_ping_to_cloud operation has the @b response_required set to @ref connector_true,
 * a @b ping_response_callback is made when a reply to the original Ping is received.  This callback
 * has a @ref connector_request_id_sm_ping_response Request ID and with the data cast as
 * a @ref connector_sm_ping_response_t with status set to @b connector_sm_ping_status_success .
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
 *       <li><b><i>transport</i></b>: Ping Request Transport.  See @endhtmlonly @ref connector_transport_udp. @htmlonly</li>
 *       <li><b><i>user_context</i></b>: The user_context pointer from the @ref initiate_ping_to_cloud.
 *       <li><b><i>status</i></b>: The response code, where @endhtmlonly @b connector_sm_ping_status_success indicates reply
 *                 received, @b connector_sm_ping_status_complete indicates @ref initiate_ping_to_cloud sent,
 *                 and @b connector_sm_ping_status_cancel indicates the session was @ref cancel_session "canceled". @htmlonly </li>
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
 * @note When the @ref initiate_ping_to_cloud operation has the response_required set to @ref connector_false, this callback
 * is made data with the @ref connector_sm_ping_response_t status set to @b connector_sm_ping_status_complete.  If
 * the @b Ping @b Device @b Cloud session is @ref cancel_session "canceled", then the callback status is set to
 * @b connector_sm_ping_status_cancel.
 *
 *
 * @subsection ping_request_from_cloud  Device Cloud Ping Request
 *
 * Cloud Connector will make a Ping Request @ref connector_request_id_sm_ping_request "callback" to
 * inform the Cloud Connector Application that a ping was received.  This callback
 * is just for the information purpose only.  No action is necessary.
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
 *       <li><b><i>transport</i></b>: Ping Request Transport.  See @endhtmlonly @ref connector_transport_udp. @htmlonly</li>
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
 *   <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *   <td>Aborts Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @note The @b response_required member is passing information on to the application.  There is no required
 * action necessary from the Cloud Connector Application, regardless of the value for @b response_required.
 *
 * @section cli_support Command Line Interface Support
 *
 * Cloud Connector includes support for a Command Line Interface support to be displayed on the Device
 * Cloud Device Manager CLI.  The following SM four callbacks are made (in sequence) to support an individual
 * CLI request:
 *
 *      -# @ref cli_request_callback
 *      -# @ref cli_response_length_callback
 *      -# @ref cli_response_callback
 *      -# @ref cli_status_callback
 *
 * @subsection cli_request_callback  CLI request callback
 *
 * The @ref connector_request_id_sm_cli_request @ref connector_callback_t "callback" is the first call
 * in a CLI command sequence.  This callback passes the Device Cloud CLI command and arguments to Cloud Connector
 * using a @ref connector_sm_cli_request_t data cast.
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
 *       <li><b><i>transport</i></b>: Ping Request Transport.  See @endhtmlonly @ref connector_transport_udp. @htmlonly</li>
 *       <li><b><i>user_context</i></b>: An opaque application defined context.  This pointer is passed to all subsequent
 *                                        callbacks during the CLI sequence for this command.
 *       <li><b><i>buffer</i></b>: Buffer containing the Device Cloud CLI command. </li>
 *       <li><b><i>bytes_used</i></b>: Number of bytes in the CLI command. </li>
  *       <li><b><i>response_required</i></b>: Set to @endhtmlonly @ref connector_true if a CLI response
 *                                            callback" is required.  Set to @ref connector_false when no response callback
 *                                            is required.  @htmlonly </li>
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
 *   <td>Application encountered error</td>
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
 * @ref cli_response_callback "response callback" can be anything, but not to exceed this bytes.
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
 * "total length" provided.
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
 *       <li><b><i>buffer</i></b>, to copy the CLI response </li>
 *       <li><b><i>bytes_available</i></b>, size of buffer in bytes </li>
 *       <li><b><i>bytes_used</i></b>, number of response bytes copied </li>
 *       <li><b><i>more_data</i></b>, set this to connector_false if there is no more response data </li>
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
 * @subsection cli_status_callback  CLI session error callback
 *
 * This callback is called with @ref connector_request_id_sm_cli_status "CLI status" @ref connector_callback_t "callback"
 * to indicate the reason for unusual CLI session terminate. User may get this call when @ref connector_initiate_stop_request_t
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
 *       <li><b><i>user_context</i></b>, user provided context </li>
 *       <li><b><i>status</i></b>, reason for CLI session termination </li>
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
 * @section pending_data  Pending Data Available
 *
 * Cloud Connector will make a @ref connector_request_id_sm_more_data @ref connector_callback_t "callback"
 * to notify the application that additional SM messages are pending.
 *
 * Battery-backed Applications can use this mechanism to remain awake for additional messages.  The callback
 * can re-trigger a timer which signals a power down sequence, once the timer has expired.
 *
 *
 * To retrieve pending messages from Device Cloud, Applications should call @ref initiate_ping_to_cloud.
 *
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
 *       <li><b><i>transport</i></b>, on which the pending data can be retrieved </li>
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
 * @note This callback may be called only if @ref CONNECTOR_SM_BATTERY is defined.
 *
 * @subsection cancel_session  Cancel request
 *
 * The application initiates the cancel session request to Cloud Connector by calling @ref connector_initiate_action
 * "initiate cancel" with @ref connector_initiate_session_cancel request and @ref connector_sm_cancel_request_t request_data.
 *
 * User can make use of this API to cancel a session for which no response is received in a specified time. A
 * response from Device Cloud after canceling a session will result in @ref opaque_response.
 *
 * The @ref connector_initiate_action "initiate cancel" is called with the following arguments:
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
 *         <li><b><i>transport</i></b>, on which the original request is sent </li>
 *         <li><b><i>user_context</i></b>, the last user context used on this session </li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_success @htmlonly</th>
 *   <td>Cancel session initiated</td>
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
 * @subsection opaque_response  Opaque response callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_opaque_response "opaque response"
 * @ref connector_callback_t "callback" to indicate the application that it received a response from
 * Device Cloud for which no associated request is available. The reason for this is the session is
 * terminated either because of the timeout specified in @ref CONNECTOR_SM_TIMEOUT or it is
 * @ref cancel_session "canceled" by the user.
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
 *       <li><b><i>bytes_used</i></b>, number of bytes in the payload </li>
 *       <li><b><i>error</i></b>, is connector_true for error response for Device Cloud </li>
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
 *
 * @subsection sm_connect Request TCP start
 *
 * Requests the Cloud Connector to start it's TCP/IP transport.  This request will
 * be handled in the Cloud Connector private layer will start the TCP.
 *
 * Once the TCP transport @ref network_tcp_start "is started", Cloud Connector Applications can make use of
 * the TCP features like @ref firmware_download, @ref rci_service, or reliable
 * @ref data_point or @ref data_service over a @ref connector_transport_tcp "TCP/IP transport".
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
 * @htmlinclude terminate.html
 */
