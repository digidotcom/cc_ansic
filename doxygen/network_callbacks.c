
/*! @page network_callbacks Network Callbacks
 *
 * @htmlinclude nav.html
 *
 * @section net Network Callbacks
 * The Etherios Cloud Connector interfaces to the platform's network interface through the callbacks listed below.  These
 * are called through the application callback described in the @ref api1_overview.
 *
 *  -# @ref open
 *  -# @ref send
 *  -# @ref receive
 *  -# @ref close
 * <br /><br /> 
 *
 * @section open Open Callback
 *
 * This callback is called to start a network transport to communicate with the iDigi Device Cloud. For
 * @ref connector_class_network_tcp transport it establishes a connection between Etherios Cloud Connector and the iDigi Device Cloud.
 * The callback is responsible to setup any socket options.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function:
 * <ul>
 *   <li> @ref app_network_tcp_open() in network_tcp.c</li>
 *   <li> @ref app_network_udp_open() in network_udp.c</li>
 *   <li> @ref app_network_sms_open() in network_sms.c</li>
 * </ul>
 * <br />
 *
 * @see @ref network_tcp_start "Start TCP transport automatically or manually"
 * @see @ref network_udp_start "Start UDP transport automatically or manually"
 * @see @ref network_sms_start "Start SMS transport automatically or manually"
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td> 
 *    <ul>
 *       <li>@endhtmlonly @ref connector_class_network_tcp @htmlonly - Establishes a connection and guarantees reliable data delivery on 
 *         <ul>
 *           <li>@endhtmlonly @ref CONNECTOR_PORT @htmlonly for non-secure port number.</li>
 *           <li>@endhtmlonly @ref CONNECTOR_SSL_PORT @htmlonly for secure port number.</li>
 *         </ul>
 *       </li> 
 *       <li>@endhtmlonly @ref connector_class_id_network_udp @htmlonly - Does not guarantee reliable data delivery on
 *         <ul>
 *           <li>@endhtmlonly @ref CONNECTOR_UDP_PORT @htmlonly for non-secure port number.</li>
 *         </ul>
 *       <li>@endhtmlonly @ref connector_class_id_network_sms @htmlonly - Does not guarantee reliable data delivery.</li>
 *    </ul>
 * </td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_network_open @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td>Pointer to the iDigi Device Cloud server URL.</td>
 * </tr>
 * <tr>
 * <th>request_length</th>
 * <td> Length of the iDigi Device Cloud server URL.</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> Callback returns the pointer to  @endhtmlonly @ref connector_network_handle_t @htmlonly which is used throughout network callbacks. </td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td>Pointer to memory where callback writes the size of connector_network_handle_t</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully established a connection</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Callback is busy connecting and needs to be called back again</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_error @htmlonly</td>
 * <td>Callback was unable to connect to the server; Etherios Cloud Connector will exit @endhtmlonly @ref connector_run "connector_run()"@htmlonly. 
 *     The callback will be retried when @endhtmlonly @ref connector_run "connector_run()" @htmlonly is called again,
 *     if the transport is configured to start automatically</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Etherios Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * <br /><br />
 *
 * @section send Send Callback
 *
 * Callback is called to send data to the iDigi Device Cloud. This function must not block.
 * Number of bytes actually sent could be less than the requested number. If the callback could not 
 * send any data because it encountered EAGAIN or EWOULDBLOCK error, it must return @ref connector_callback_busy
 * and Etherios Cloud Connector would continue calling this function.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function:
 * <ul>
 *   <li> @ref app_network_tcp_send() in network_tcp.c</li>
 *   <li> @ref app_network_udp_send() in network_udp.c</li>
 *   <li> @ref app_network_sms_send() in network_sms.c</li>
 * </ul>
 * <br /> 
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td> 
 *    <ul>
 *       <li>@endhtmlonly @ref connector_class_network_tcp @htmlonly</li> 
 *       <li>@endhtmlonly @ref connector_class_id_network_udp @htmlonly</li>
 *       <li>@endhtmlonly @ref connector_class_id_network_sms @htmlonly</li>
 *    </ul> 
 * </td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_network_send @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td>Pointer to @endhtmlonly connector_write_request_t @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_length</th>
 * <td> Size of @endhtmlonly connector_write_request_t. @htmlonly</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> Pointer to size_t type memory where callback writes number of bytes sent to the iDigi Device Cloud. </td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td>N/A</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully sent data to server</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Callback could not send data due to temporary unavailability of resources. It needs to be called again to send data</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_error @htmlonly</td>
 * <td>Callback was unable to send data due to irrecoverable communications error. 
 *     Etherios Cloud Connector will @endhtmlonly @ref close "close" @htmlonly the network handle</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Etherios Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * <br /><br />
 *
 * @section receive Receive Callback
 *
 * Callback is called to receive a specified number of bytes of data from the iDigi
 * Device Cloud.  This function must not block. 
 * Number of bytes actually received could be less than the requested number. If there was no data pending and the callback
 * has encountered EAGAIN or EWOULDBLOCK error, it must return @ref connector_callback_busy
 * and Etherios Cloud Connector would continue calling this function.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function:
 * <ul>
 *   <li> @ref app_network_tcp_receive() in network_tcp.c</li>
 *   <li> @ref app_network_udp_receive() in network_udp.c</li>
 *   <li> @ref app_network_sms_receive() in network_sms.c</li>
 * </ul>
 *
 * @note In the multithreaded model (connector_run()) this is the point where Etherios Cloud Connector
 * will relinquish control of the CPU.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td> 
 *    <ul>
 *       <li>@endhtmlonly @ref connector_class_network_tcp @htmlonly</li> 
 *       <li>@endhtmlonly @ref connector_class_id_network_udp @htmlonly</li>
 *       <li>@endhtmlonly @ref connector_class_id_network_sms @htmlonly</li>
 *    </ul> 
 * </td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_network_receive @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td>Pointer to @endhtmlonly connector_read_request_t @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_length</th>
 * <td> Size of @endhtmlonly connector_read_request_t. @htmlonly</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> Pointer to size_t type memory where callback writes number of bytes received from the iDigi Device Cloud. </td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td>N/A</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully received some data</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>There is no data pending to receive. Callback will be called again to receive data</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_error @htmlonly</td>
 * <td> Callback was unable to receive data due to irrecoverable communications error. 
 *     Etherios Cloud Connector will @endhtmlonly @ref close "close" @htmlonly the network handle</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Etherios Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * <br /><br />
 *
 * @section close Close Callback
 *
 * Application callback request to close a network handle
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function:
 * <ul>
 *   <li> @ref app_network_tcp_close() in network_tcp.c</li>
 *   <li> @ref app_network_udp_close() in network_udp.c</li>
 *   <li> @ref app_network_sms_close() in network_sms.c</li>
 * </ul>
 * <br /> 
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td> 
 *    <ul>
 *       <li>@endhtmlonly @ref connector_class_network_tcp @htmlonly</li> 
 *       <li>@endhtmlonly @ref connector_class_id_network_udp @htmlonly</li>
 *       <li>@endhtmlonly @ref connector_class_id_network_sms @htmlonly</li>
 *    </ul> 
 * </td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_network_close @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td>Pointer to @endhtmlonly connector_close_request_t,@htmlonly in which @endhtmlonly @ref connector_close_status_t "status" @htmlonly 
 *     field provides reason for close</td>
 * </tr>
 * <tr>
 * <th>request_length</th>
 * <td> Size of @endhtmlonly connector_close_request_t @htmlonly</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td>Pointer to @endhtmlonly @ref connector_auto_connect_type_t "connector_auto_connect_type_t"@htmlonly memory where callback 
 *     writes action on transport close.</td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td> Size of @endhtmlonly @ref connector_auto_connect_type_t "connector_auto_connect_type_t"@htmlonly</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully closed the connection. Etherios Cloud Connector will restart the transport if @endhtmlonly @ref connector_auto_connect @htmlonly 
 *     is returned in response_data </td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 * <td>Callback was unable to complete the network close (most likely due to a blocking network handle)
 *     and will be called again</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Etherios Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @htmlinclude terminate.html
 */
