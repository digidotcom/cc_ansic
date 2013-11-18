/*! @page data_point_sample Data Point Sample
 *
 * @htmlinclude nav.html
 *
 * @section data_point_sample_overview Overview
 *
 * This sample demonstrates how to send streaming data points from a device to Device Cloud.
 * This application sends cpu usage as data points periodically. Device Cloud (with Data Stream enabled)
 * on the other hand collects these data and the data shows up under Data Streams page inside the Data
 * Service tab.
 *
 * To learn more about Device Cloud Data Streams, see chapter 12 Device Cloud Data Streams in the
 * @htmlonly <a href="http://www.digi.com/support/productdetail?pid=5575">Device Cloud User's Guide</a>@endhtmlonly.
 *
 * @section data_point_location Source Code Location
 *      - For the connector_run() version:
 *           - @a /connector/public/run/samples/data_point
 *
 *      - For the connector_step() version:
 *           - @a /connector/public/step/samples/data_point
 *
 * @note Check @ref threading "here for more information" on using the connector_run()
 * versus connector_step() model.
 *
 * @section connect_description Code Flow
 *
 * The routine main() in the platform directory initializes Cloud Connector and
 * registers the application-defined callback. In the file application.c is the
 * application-defined callback app_connector_callback(), this routine calls app_data_service_handler()
 * when a data service request is received.
 *
 * The routine application_run() (or application_step()) periodically (once every 2 seconds) calls
 * app_send_data_point() updates one data point (in this sample it is CPU usage). This routine fills
 * the data points list (holds 5 points) one by one until it is full. When it is full (takes 10 seconds),
 * it calls connector_initiate_action() to initiates the actual transfer of the data points. The sample
 * waits for the response before sending the next set of points.
 *
 * If the app_data_point_handler() is called with @ref connector_request_id_data_point_single_response
 * then this indicates a response from Device Cloud is received, The @ref connector_request_id_data_point_single_status
 * would indicate an error occurred while sending data points. This sample uses linux, /proc/stat file
 * to calculate the CPU usage.
 *
 * @section connect_build Building
 *
 * To build this example on a Linux system, go to the public/run/samples/data_point
 * directory and type: @htmlonly"<I>make</I>"@endhtmlonly.
 * If you are not using Linux you will need to update your environment with
 * the information below and then build the image for your platform.
 *
 * @subsection Source Files
 *
 * The following is a list of source files required to build this sample:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th class="title">Name</th>
 *   <th class="title">Description</th>
 *   <th class="title">Location</th>
 * </tr>
 * <tr>
 *   <th>application.c</th>
 *   <td>Contains application_run() and the application callback</td>
 *   <td>samples/data_point</td>
 * </tr>
 * <tr>
 *   <th>data_point.c</th>
 *   <td>data point callback and send_data_point() which starts the data transfer</td>
 *   <td>samples/data_point</td>
 * </tr>
 * <tr>
 *   <th>connector_config.h</th>
 *   <td>Cloud Connector options</td>
 *   <td>samples/data_point</td>
 * </tr>
 * <tr>
 *   <th>connector_api.c</th>
 *   <td>Code for Cloud Connector </td>
 *   <td>private</td>
 * </tr>
 * <tr>
 *   <th>os.c</th>
 *   <td>Operating system calls</td>
 *   <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 *   <th>network_tcp.c</th>
 *   <td> Network interface for TCP </td>
 *   <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 *   <th>config.c</th>
 *   <td>Configuration routines</td>
 *   <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 *   <th>main.c</th>
 *   <td>Starting point of program, dependent on build environment</td>
 *   <td>platforms/<i>my_platform</i></td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection Include Paths
 *
 * The following include paths are required:
 *
 * @li public/include
 * @li run/platforms/linux (substitute with your platform)
 *
 * @section web_gui Viewing results from Device Cloud
 *
 * To view the results for this sample, log on to Device Cloud as described
 * in the @ref add_your_device_to_the_cloud "Getting Started" section.
 *
 * Once logged in click on the Data Services tab as shown below. Then select the Data
 * Streams tab. This should show your stream run_cpu_usage or step_cpu_usage.
 * Select the stream to see the Data Stream charts and raw data.
 *
 * @image html cloud_data_stream_selection.png
 *
 * @htmlinclude terminate.html
 *
 */
