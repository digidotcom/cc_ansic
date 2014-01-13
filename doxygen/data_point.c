/*! @page data_point Data Points
 *
 * @htmlinclude nav.html
 *
 * @section data_point_overview Overview
 *
 * The Data Point API is used to send data points to Device Cloud. It can be used to send simple
 * data in binary form, to send several points of a single stream in a message or to send several points of multiple
 * streams in a message.
 *
 * @li @ref binary_point : To send a data point with just value. With this one can send just one point per message.
 * @li @ref single_stream_points : To send several data points of a single stream in one message. User can use this
 * API to send timestamp, location, quality and/or description of each point.
 * @li @ref multiple_stream_points : To send several data points of multiple streams in one message. User can use this
 * API to send timestamp, location, quality and/or description of each point.
 *
 * @note See @ref CONNECTOR_DATA_POINTS to enable and disable data points.
 *
 * @section binary_point Binary data point
 *
 * This section contains following APIs:
 *      -# @ref initiate_binary_point
 *      -# @ref binary_point_response
 *      -# @ref binary_point_status
 *
 * @subsection initiate_binary_point Initiate binary data point
 *
 * The application can send binary data point by calling connector_initiate_action() with
 * @ref connector_initiate_data_point_binary "action id" and @ref connector_request_data_point_binary_t "data point"
 * as request_data. The @ref connector_request_data_point_binary_t "data point" cannot be a stack
 * variable and can be released/reused only after receiving the response.
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
 *   <td>@endhtmlonly @ref connector_initiate_data_point_binary @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> Pointer to @endhtmlonly connector_request_data_point_binary_t @htmlonly structure, where member:
 *      <ul>
 *        <li><b><i>transport</i></b> is the communication method used to send the data point to Device Cloud</li>
 *        <li><b><i>user_context</i></b> is the application context which will be returned in the response</li>
 *        <li><b><i>path</i></b>, pointer to data stream name </li>
 *        <li><b><i>point</i></b>, pointer to binary data point value </li>
 *        <li><b><i>bytes_used</i></b>, number of bytes filled in point </li>
 *        <li><b><i>response_required</i></b>, set to connector_true if response is needed </li>
 *      </ul>
 *    </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_success @htmlonly</th>
 *   <td>Transfer initiated</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_invalid_data @htmlonly</th>
 *   <td>One or more input is not valid</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_service_busy @htmlonly</th>
 *   <td>Previous binary data point is not yet processed</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 *
 * An example of initiating a binary data point transfer is shown below:
 * @code
 *   static connector_request_data_point_binary_t dp_binary_data;
 *   static char path[] = "run_dp_binary";
 *   static int dp_value = 123;
 *
 *   dp_binary_data.transport = connector_transport_tcp;
 *   dp_binary_data.path  = path;
 *   dp_binary_data.point = &dp_value;
 *   dp_binary_data.bytes_used = sizeof dp_value;
 *   dp_binary_data.response_required = connector_true;
 *   dp_binary_data.user_context = path;
 *
 *   // Begin a binary data point transfer to Device Cloud
 *   status = connector_initiate_action(handle, connector_initiate_data_point_binary, &dp_binary_data);
 * @endcode
 *
 * This example will invoke Cloud Connector to send binary data point to Device
 * Cloud.  The result of this operation adds an entry in run_dp_binary data stream on Device Cloud.
 *
 * @note The dp_binary_data, path and dp_value above are not stack variables. Either you can use a heap or a memory (global or static)
 * variable to hold these values. You can release them when you get a @ref connector_request_id_data_point_binary_response "response"
 * or @ref connector_request_id_data_point_binary_status "status" callback. The value passed as the user_context will be returned
 * in the callback.
 *
 * @see connector_initiate_action()
 * @see @ref binary_point_response
 * @see connector_request_data_point_binary_t
 * @see connector_initiate_data_point_binary
 *
 *
 * @subsection binary_point_response  Binary data point response
 *
 * After calling connector_initiate_action(), Cloud Connector will prepare and send data point request
 * to Device Cloud asynchronously. Upon receiving the response, it makes
 * @ref connector_request_id_data_point_binary_response "binary data point response" callback to pass
 * Device Cloud response to the application. Application is free to release the allocated resources at this point.
 *
 * The @ref connector_request_id_data_point_binary_response "binary data point response"
 * @ref connector_callback_t "callback" is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_class_id_data_point @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_point_binary_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_data_point_response_t @htmlonly structure:
 *      <ul>
 *        <li><b><i>transport</i></b>, method on which data point is sent</li>
 *        <li><b><i>user_context</i></b> is the user context passed in the @endhtmlonly connector_initiate_action() @htmlonly </li>
 *        <li><b><i>response</i></b>, response code returned by Device Cloud </li>
 *        <li><b><i>hint</i></b>, returned from Device Cloud in case of error</li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *    <td>Continue</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *    <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *    <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @see @ref initiate_binary_point
 * @see connector_data_point_response_t
 * @see connector_request_id_data_point_binary_response
 *
 * @subsection binary_point_status  Binary data point status
 *
 * Cloud Connector makes @ref connector_request_id_data_point_binary_status "binary data point status"
 * @ref connector_callback_t "callback"
 *
 * The @ref connector_request_id_data_point_binary_status "binary data point status"
 * @ref connector_callback_t "callback" is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_class_id_data_point @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_point_binary_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_data_point_status_t @htmlonly structure:
 *      <ul>
 *        <li><b><i>transport</i></b>, method used to send data point </li>
 *        <li><b><i>user_context</i></b> is the user context passed in the @endhtmlonly connector_initiate_action() @htmlonly </li>
 *        <li><b><i>status</i></b>, the reason for the end of session </li>
 *       <li><b><i>session_error</i></b>, this field will carry the session error code
 *                 when the status is set to connector_data_service_status_session_error </li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *    <td>Continue</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *    <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *    <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @section single_stream_points  Single stream data points
 *
 * This section contains following APIs:
 *      -# @ref initiate_single_stream_points
 *      -# @ref single_stream_point_response
 *      -# @ref single_stream_point_status
 *
 * @subsection initiate_single_stream_points Initiate single stream data points
 *
 * The application can send several data points which belong to a single data stream by calling
 * connector_initiate_action() with @ref connector_initiate_data_point_single "action id" and
 * @ref connector_request_data_point_single_t "action data" as request_data.
 * The @ref connector_request_data_point_single_t "action data" cannot be a stack variable and can be
 * released/reused only after receiving the response.
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
 *   <td>@endhtmlonly @ref connector_initiate_data_point_single @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> Pointer to @endhtmlonly connector_request_data_point_single_t @htmlonly structure, where member:
 *      <ul>
 *        <li><b><i>transport</i></b> is the communication method used to send the data points to Device Cloud</li>
 *        <li><b><i>user_context</i></b> is the application context which will be returned in the response/status</li>
 *        <li><b><i>path</i></b>, pointer to data stream name </li>
 *        <li><b><i>unit</i></b>, a null-terminated unit, optional field, set to NULL if not used </li>
 *        <li><b><i>forward_to</i></b>, comma separated list of streams to replicate data points to (a null-terminated optional field, set to NULL if not used) </li>
 *        <li><b><i>point</i></b>, pointer to a @endhtmlonly @ref connector_data_point_t "connector_data_point_t" @htmlonly</li>
 *        <li><b><i>response_required</i></b>, set to connector_true if response is needed </li>
 *        <li><b><i>type</i></b>, data point @endhtmlonly @ref connector_data_point_type_t "connector_data_point_type_t" @htmlonly </li>
 *      </ul>
 *    </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_success @htmlonly</th>
 *   <td>Transfer initiated</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_invalid_data @htmlonly</th>
 *   <td>One or more input is not valid</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_service_busy @htmlonly</th>
 *   <td>Previous single data request is not yet processed</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @note If using a @ref shortmessaging transport (SMS or UDP), the number of Data Points that can be sent at once is limited by @ref CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS.
 *@see connector_data_point_type_t
 *@see connector_data_point_t
 *@see connector_request_data_point_single_t
 *@see connector_initiate_action()
 *@see @ref single_stream_point_response
 *
 * An example of initiating a single stream data points transfer is shown @ref data_point_sample.
 *
 * @subsection single_stream_point_response  Single stream data point response
 *
 * After calling connector_initiate_action(), Cloud Connector will prepare and send data point request
 * to Device Cloud asynchronously. Upon receiving the response, it makes
 * @ref connector_request_id_data_point_single_response "single stream data point response" callback to pass
 * Device Cloud response to the application. Application is free to release the allocated resources at this point.
 *
 * The @ref connector_request_id_data_point_single_response "single stream data point response"
 * @ref connector_callback_t "callback" is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_class_id_data_point @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_point_single_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_data_point_response_t @htmlonly structure:
 *      <ul>
 *        <li><b><i>transport</i></b>, method on which data point is sent</li>
 *        <li><b><i>user_context</i></b> is the user context passed in the @endhtmlonly connector_initiate_action() @htmlonly </li>
 *        <li><b><i>response</i></b>, response code returned by Device Cloud </li>
 *        <li><b><i>hint</i></b>, returned from Device Cloud in case of error</li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *    <td>Continue</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *    <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *    <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @see connector_request_id_data_point_single_response
 * @see connector_data_point_response_t
 * @see connector_request_data_point_single_t
 * @see @ref initiate_single_stream_points

 * @subsection single_stream_point_status  Single stream data point status
 *
 * Cloud Connector makes @ref connector_request_id_data_point_single_status @ref connector_callback_t "callback"
 *
 * The @ref connector_request_id_data_point_single_status "single stream data point status"
 * @ref connector_callback_t "callback" is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_class_id_data_point @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_point_single_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_data_point_status_t @htmlonly structure:
 *      <ul>
 *        <li><b><i>transport</i></b>, method used to send data point </li>
 *        <li><b><i>user_context</i></b> is the user context passed in the @endhtmlonly connector_initiate_action() @htmlonly </li>
 *        <li><b><i>status</i></b>, the reason for the end of session </li>
 *       <li><b><i>session_error</i></b>, this field will carry the session error code
 *                 when the status is set to connector_data_service_status_session_error </li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *    <td>Continue</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *    <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *    <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @section multiple_stream_points  Multiple stream data points
 *
 * This section contains following APIs:
 *      -# @ref initiate_multiple_stream_points
 *      -# @ref multiple_stream_point_response
 *      -# @ref multiple_stream_point_status
 *
 * @subsection initiate_multiple_stream_points Initiate multiple stream data points
 *
 * The application can send several data points which belong to multiple data streams by calling
 * connector_initiate_action() with @ref connector_initiate_data_point_multiple "action id" and
 * @ref connector_request_data_point_multiple_t "action data" as request_data.
 * The @ref connector_request_data_point_multiple_t "action data" cannot be a stack variable and can be
 * released/reused only after receiving the response.
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
 *   <td>@endhtmlonly @ref connector_initiate_data_point_multiple @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_data</td>
 *   <td> Pointer to @endhtmlonly connector_request_data_point_multiple_t @htmlonly structure, where member:
 *      <ul>
 *        <li><b><i>transport</i></b> is the communication method used to send the data points to Device Cloud</li>
 *        <li><b><i>user_context</i></b> is the application context which will be returned in the response/status</li>
 *        <li><b><i>stream</i></b>, pointer to a @endhtmlonly @ref connector_data_stream_t "connector_data_stream_t" @htmlonly</li>
 *        <li><b><i>response_required</i></b>, set to connector_true if response is needed </li>
 *      </ul>
 *    </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_success @htmlonly</th>
 *   <td>Transfer initiated</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_invalid_data @htmlonly</th>
 *   <td>One or more input is not valid</td>
 * </tr>
 * <tr>
 *   <th>@endhtmlonly @ref connector_service_busy @htmlonly</th>
 *   <td>Previous multiple data request is not yet processed</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @note If using a @ref shortmessaging transport (SMS or UDP), the number of Data Points that can be sent at once is limited by @ref CONNECTOR_SM_MAX_DATA_POINTS_SEGMENTS.
 *@see connector_data_stream_t
 *@see connector_request_data_point_multiple_t
 *@see connector_initiate_action()
 *@see @ref multiple_stream_point_response
 *
 * An example of initiating a multiple stream data points transfer is shown @ref data_point_multiple_sample.
 *
 * @subsection multiple_stream_point_response  Multiple stream data point response
 *
 * After calling connector_initiate_action(), Cloud Connector will prepare and send data point request
 * to Device Cloud asynchronously. Upon receiving the response, it makes
 * @ref connector_request_id_data_point_multiple_response "multiple stream data point response" callback to pass
 * Device Cloud response to the application. Application is free to release the allocated resources at this point.
 *
 * The @ref connector_request_id_data_point_multiple_response "multiple stream data point response"
 * @ref connector_callback_t "callback" is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_class_id_data_point @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_point_multiple_response @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_data_point_response_t @htmlonly structure:
 *      <ul>
 *        <li><b><i>transport</i></b>, method on which data point is sent</li>
 *        <li><b><i>user_context</i></b> is the user context passed in the @endhtmlonly connector_initiate_action() @htmlonly </li>
 *        <li><b><i>response</i></b>, response code returned by Device Cloud </li>
 *        <li><b><i>hint</i></b>, returned from Device Cloud in case of error</li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *    <td>Continue</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *    <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *    <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @see connector_request_id_data_point_multiple_response
 * @see connector_data_point_response_t
 * @see connector_request_data_point_multiple_t
 * @see @ref initiate_multiple_stream_points

 * @subsection multiple_stream_point_status  Multiple stream data point status
 *
 * Cloud Connector makes @ref connector_request_id_data_point_multiple_status @ref connector_callback_t "callback"
 *
 * The @ref connector_request_id_data_point_multiple_status "multiple stream data point status"
 * @ref connector_callback_t "callback" is called with the following information:
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
 *   <td>@endhtmlonly @ref connector_class_id_data_point @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>request_id</td>
 *   <td>@endhtmlonly @ref connector_request_id_data_point_multiple_status @htmlonly</td>
 * </tr>
 * <tr>
 *   <td>data</td>
 *   <td>Pointer to @endhtmlonly connector_data_point_status_t @htmlonly structure:
 *      <ul>
 *        <li><b><i>transport</i></b>, method used to send data point </li>
 *        <li><b><i>user_context</i></b> is the user context passed in the @endhtmlonly connector_initiate_action() @htmlonly </li>
 *        <li><b><i>status</i></b>, the reason for the end of session </li>
 *       <li><b><i>session_error</i></b>, this field will carry the session error code
 *                 when the status is set to connector_data_service_status_session_error </li>
 *      </ul>
 *   </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 *    <td>Continue</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 *    <td>Aborts Cloud Connector</td>
 * </tr>
 * <tr>
 *    <th>@endhtmlonly @ref connector_callback_busy @htmlonly</th>
 *    <td>Busy and needs to be called back again</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @htmlinclude terminate.html
 *
 */
