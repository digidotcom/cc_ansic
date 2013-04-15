/*! @page send_data Send Data Sample
 *
 * @htmlinclude nav.html
 *
 * @section send_data_overview Overview
 *
 * This sample demonstrates how to send data from a device to the iDigi Server.
 * Using the @ref data_service_overview "data service api" this application writes data to a file on the
 * idigi server.
 * 
 * @section connect_description Code Flow
 *
 * The routine main() in the platform directory initializes Etherios Cloud Connector and
 * registers the application-defined callback.  In the file application.c is the
 * application-defined callback app_connector_callback(), this routine calls app_data_service_handler()
 * when a data service request is received.
 *
 * The routine application_run() (or application_step()) which is called after initialization calls the routine
 * app_send_put_request() to initiate the data transfer to Etherios Device Cloud.  This routine
 * initiates the data transfer by calling connector_initiate_action(); once the server is 
 * ready to receive data the app_data_service_handler() routine is called. The callback then returns
 * the string "Welcome to iDigi data service sample!\n" back to Etherios Device Cloud.  This data is @b copied
 * @b into @b the @b callback's @b buffer, the flags @ref CONNECTOR_MSG_LAST_DATA and @ref CONNECTOR_MSG_FIRST_DATA
 * indicate this is the only message in this data transfer.  By modifying these flags
 * you could modify this sample to send large amounts of data back to the server.
 *
 * If app_data_service_handler() is called with @ref connector_data_service_type_have_data then this
 * indicates a response from the server, @ref connector_data_service_type_error would indicate
 * an error is sent back from the server.  In this example app_send_put_request() is called
 * from application_run() (or application_step()) until iDigi Connector initialization completes and
 * it returns success.
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
 *   <td> iDigi Connector options</td>
 *   <td>samples/send_data</td>
 * </tr>
 * <tr>
 *   <th>connector_api.c</th>
 *   <td>Code for Etherios Cloud Connector </td>
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
 * The following defines are used in this sample:
 *
 * @li -DCONNECTOR_VERSION=0x1020000UL
 *
 * @section web_gui Viewing results from Etherios Device Cloud
 *
 * To view the results for this sample log on to Etherios Device Cloud as described
 * in the @ref add_your_device_to_the_cloud "Getting Started" section. 
 *
 * Once logged in click on the Data Services tab as shown
 * below. Double click to the device folder.
 *
 * @image html cloud_data_services_file.png
 *
 * This sample creates a file test.txt in the test directory.  Double click on the test folder to
 * open the test directory.
 *
 * @image html cloud_data_services_folder.png
 *
 * The file is then listed as shown (below). You can click on a 
 * file to view its contents, you will see the file test.txt in the test directory
 * after you run this sample.
 *
 * @image html cloud_data_services_refresh.png
 *
 * @section python_script Python script to retrieve a file from Etherios Device Cloud storage
 *
 * This sample provides a basic python script to retrieve the test.txt file from login.etherios.com
 * after a device has sent it to the Device Cloud.
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
