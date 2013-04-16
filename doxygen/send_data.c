/*! @page send_data Send Data Sample
 *
 * @htmlinclude nav.html
 *
 * @section send_data_overview Overview
 *
 * This sample demonstrates how to send data from a device to Device Cloud, using the
 * @ref data_service_overview "data service api" this application writes data to a file
 * on Device Cloud.
 * 
 * @section connect_description Code Flow
 *
 * The routine main() in the platform directory initializes Cloud Connector and registers the
 * application-defined callback. In the file application.c is the application-defined callback
 * app_connector_callback(), this routine calls app_data_service_handler() when a data service
 * request is received.
 *
 * The routine application_run() (or application_step()) which is called after initialization calls the routine
 * app_send_put_request() to initiate the data transfer to Device Cloud. This routine initiates the data transfer
 * by calling connector_initiate_action(); once the server is ready to receive data the app_data_service_handler()
 * routine is called. The callback then returns the string "Welcome to iDigi data service sample!\n" back to
 * Device Cloud. This data is @b copied @b into @b the @b callback's @b buffer, by setting more_data to
 * connector_false indicates the only message in this data transfer. If you set more_data to connector_true then
 * the callback is called again to get the next chunk of data, this can be repeated to transfer larger data.
 *
 * If app_data_service_handler() is called with @ref connector_request_id_data_service_send_response then this
 * indicates a response from the server, @ref connector_request_id_data_service_send_status would indicate
 * the session is completed without Device Cloud response.
 *
 * @section connect_build Building
 *
 * To build this example on a Linux system, go to the public/run/samples/send_data
 * directory and type: @htmlonly"<I>make clean all</I>"@endhtmlonly.
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
 *   <td>samples/send_data</td>
 * </tr>
 * <tr>
 *   <th>put_request.c</th>
 *   <td>data service callback and send_put_request() which starts the data transfer</td>
 *   <td>samples/send_data</td>
 * </tr>
 * <tr>
 *   <th>get_file.py</th>
 *   <td>Python script to retrieve a file</td>
 *   <td>samples/send_data</td>
 * </tr>
 * <tr>
 *   <th>connector_config.h</th>
 *   <td>Cloud Connector options</td>
 *   <td>samples/send_data</td>
 * </tr>
 * <tr>
 *   <th>connector_api.c</th>
 *   <td>Cloud Connector source</td>
 *   <td>private</td>
 * </tr>
 * <tr>
 *   <th>os.c</th>
 *   <td>Operating system calls</td>
 *   <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 *   <th>network_tcp.c</th>
 *   <td> Network interface </td>
 *   <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 *   <th>config.c</th>
 *   <td>Configuration routines</td>
 *   <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 *   <th>main.o</th>
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
 * @subsection Defines
 *
 * @section web_gui Viewing results from Etherios Device Cloud
 *
 * To view the results for this sample log on to Device Cloud as described
 * in the @ref add_your_device_to_the_cloud "Getting Started" section. 
 *
 * Once logged in click on the Data Services tab as shown below. Double click to the device folder.
 *
 * @image html cloud_data_services_file.png
 *
 * This sample creates a file test.txt in the test directory.  Double click on the test folder to
 * open the test directory.
 *
 * @image html cloud_data_services_folder.png
 *
 * The file is then listed as shown (below). You can click on the file to view its contents, you will see the file test.txt in the test directory
 * after you run this sample.
 *
 * @image html cloud_data_services_refresh.png
 *
 * @section python_script Python script to retrieve a file from Device Cloud storage
 *
 * This sample provides a basic python script to retrieve the test.txt file from login.etherios.com
 * after a device has sent it to Device Cloud.
 *
 * Run the python script in the sample:
 *
 * @code
 * python ./get_file.py <Username> <Password> <Device ID> [<Device Cloud URL>]
 * @endcode
 *
 * The script prints the content of test.txt.
 *
 * @htmlinclude terminate.html
 *
 */
