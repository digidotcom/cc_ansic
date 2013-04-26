/*! @page shortmessaging Short Messaging
 *
 * @htmlinclude nav.html
 *
 * @section secsmintroduction Short Messaging Introduction
 *
 * Short Message (SM) is a lightweight, message oriented protocol that
 * allows applications to optimize network data usage.
 *
 * This chapter will explain:
 *
 *   -# @ref smwhatisit
 *        -# @ref smtransports
 *   -# @ref smsectionwhoneedsit
 *        -# @ref smsectionwhodoesnotneedsit
 *   -# @ref smsolutions
 *        -# @ref smcomplications
 *   -# @ref smsectionexamples
 *
 * @section smwhatisit What is Short Messaging?
 *
 * The Short Message (SM) protocol was designed to be a lightweight, message oriented
 * protocol suitable for use on transports that are:
 *    @li Expensive or where data usage needs to be kept to a minimum.
 *    @li Lossy
 *       -# Delivery not guaranteed
 *       -# Duplicates possible
 *       -# Data integrity not guaranteed
 *
 * The SM protocol supports the following features:
 *      -# @ref ping_request "PING mechanism" between Cloud Connector and Device Cloud or vice versa
 *      -# @ref data_service "Data transfer" between Cloud Connector and Device Cloud or vice versa
 *      -# @ref cli_request "CLI" from Device Cloud to Cloud Connector
 *      -# @ref sm_reboot "Reboot" from Device Cloud to Cloud Connector
 *      -# @ref sm_connect "Connect" from Device Cloud to Cloud Connector to start communication over TCP
 *      -# @ref pending_data "Message pending" to indicate more messages are queued on the Device Cloud for this device.
 *
 * @note Cloud Connector supports only the UDP transport (see @ref CONNECTOR_TRANSPORT_UDP) for Short Messaging.
 *       In the future, SMS will be supported.
 *
 * @subsection smtransports Why is the transport important?
 *
 * At this point, all the communication with Device Cloud is either on the TCP or on the UDP.
 * The TCP based communication requires initial handshaking and connection maintenance
 * messages, but it is secure and reliable. On the other hand the UDP based communication
 * has no overhead that the TCP has, but it is unreliable.
 * The message which are queued up in Device Cloud, are sent to the device when Device Cloud
 * receives a request from the device.
 *
 * @section smsectionwhoneedsit Who needs Short Messaging?
 *
 * A small device with very limited memory and/or running on an expensive network where data
 * usage needs to be kept to a minimum. For example a battery operated device which uses cellular
 * connection to send a very small data to Device Cloud once a day can make use of this feature.
 * @note This method doesn't support firmware update and file system functionality.
 *
 * @subsection smsectionwhodoesnotneedsit Who doesn't need Short Messaging?
 *
 * The device which has ample/free/less expensive network resource and which has enough
 * memory doesn't need Short Messaging. Also, if the device requires firmware update or
 * file system access then it should enable TCP transport method.
 *
 * @section smsolutions How does Short Messaging solve my problem?
 *
 *  -# It uses smaller footprints compare to the TCP method
 *  -# Very less overhead per message
 *  -# No need to maintain the connection
 *  -# Has ability to opt out the response
 *
 * @subsection smcomplications How does Short Messaging complicate my application?
 *
 * If you decide to send a large amount of data (more than 1KiB), then the message will
 * be transferred in multiple segments. This can lead to a packet loss and an extended
 * delay in detecting that loss. So the large data transfer is not recommended in SM.
 *
 * @section smsectionexamples Short Messaging Examples
 *
 *  -# Battery operated device with small footprint which sends small amount of
 *     data once a day.
 *  -# A remote cellular device which uses expensive cellular link to send
 *     a reading once an hour.
 *
 * @section ping_request  Ping Operations
 *
 * A Cloud Connector Application can send a @ref initiate_ping_to_cloud "Ping request to Device Cloud" at any time.  The purpose is to
 * open a hole in a firewall and let Device Cloud know that the Application Cloud Connector is ready to
 * receive pending operations.  Additionally, a Web Services application can @ref ping_request_from_cloud "search"
 * for a Cloud Connector device by Pinging through the cloud.
 *
 * @subsection initiate_ping_to_cloud  Ping Device Cloud
 *
 * When an application has only @ref network_udp_start "UDP started"
 * (TCP has @ref network_tcp_start "not started"), it's necessary to
 * periodically contact Device Cloud so to open a hole in a firewall and receive data or commands
 * from Device Cloud or Web Services applications.
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
 *        <li><b><i>transport</i></b>: Transport mechanism for the Ping Request.  See @endhtmlonly @ref connector_transport_udp. @htmlonly </li>
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
 *   ping_request.user_context = &request;
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
 * has a @ref connector_request_id_sm_ping_response Request ID and with the data cast as a @ref connector_sm_ping_response_t
 * with status set to @b connector_sm_ping_status_success .
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
 *       <li><b><i>transport</i></b>: Transport mechanism of the original Ping Request.  See @endhtmlonly @ref connector_transport_udp. @htmlonly</li>
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
 *       <li><b><i>transport</i></b>: Ping transport mechanism.  See @endhtmlonly @ref connector_transport_udp. @htmlonly</li>
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
 * action necessary from the Cloud COnnector Application, regardless of the value for @b response_required.
 *
 * @section cli_request  CLI request
 *
 * Cloud Connector will call these callbacks while processing a CLI request from Device Cloud.
 * The CLI callbacks will come in the following order, In case of any error while processing,
 * the @ref cli_status_callback will be called.
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
 * is called with following information:
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
 *       <li><b><i>user_context</i></b>, pointer to hold user context </li>
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
 * @section pending_data  Pending data callback
 *
 * Cloud Connector will make @ref connector_request_id_sm_more_data "pending data"
 * @ref connector_callback_t "callback" to indicate the application that there are pending messages
 * on Device Cloud. Application can send any messages (ping if no data to send) to retrieve
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
 * @subsection sm_reboot Reboot device on SM
 *
 * User can use Short Message to reboot the device. After receiving this request
 * from Device Cloud, Cloud Connector will call @ref reboot to execute it.
 *
 * @subsection sm_connect Request to start TCP
 *
 * This acts like a shoulder tap to start the TCP transport method on a device. After
 * receiving this request, Cloud Connector will start the TCP. If TCP is disabled
 * (@ref CONNECTOR_TRANSPORT_TCP is not defined), then Cloud Connector will return an
 * error response to Device Cloud. Once the TCP is started, user can make use of the
 * TCP only features like firmware download, file transfer or secure and reliable
 * data transfer.
 *
 * @htmlinclude terminate.html
 */
