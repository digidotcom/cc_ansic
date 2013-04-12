/*! @page device_request_sample Device Request Sample
 *
 * @htmlinclude nav.html
 *
 * @section device_request_overview Overview
 *
 * This sample demonstrates how to process data sent from Etherios Device Cloud.
 * Using the @ref data_service_overview "data service api" this application prints
 * out the data sent from Etherios Device Cloud.
 *
 * @section connect_description Code Flow
 *
 * The routine main() in the platform directory initializes Etherios Cloud Connector and
 * registers the application callback.  In the file application.c is the
 * application defined callback app_connector_callback(), this routine calls app_data_service_handler()
 * when a device request is received.
 *
 * A User can use the included python script to send a device request to the device.
 *
 * When Etherios Device Cloud sends a device request to the device, app_data_service_handler() is called
 * with @ref connector_request_id_data_service_receive_target request message type to process the data. This callback
 * is continually called with this message type until the last chunk of data is processed. This
 * sample examines the given target and prints out the device request data. It cancels
 * the device request if the target name is not "myTarget".
 *
 * After the callback is called with @ref connector_data_service_type_have_data message type for the last chunk of data,
 * the callback is called with @ref connector_data_service_type_need_data message type to send response
 * data back to Etherios Device Cloud. This sample sends and prints out the request and response data.
 *
 * When the callback is called with @ref connector_data_service_type_error message type it indicates
 * an error is encountered and needs to cancel the device request. In this example it cancels
 * the request by freeing any memory used.
 *
 * @section connect_build Building
 *
 * To build this example on a Linux system, go to the public/run/samples/device_request
 * directory and type: @htmlonly"<I>make clean all</I>"@endhtmlonly.
 * If you are not using Linux you will need to update your environment with
 * the information below and then build the image for your platform.
 *
 * @subsection source_files Source Files
 *
 * The following is a list of source files required to build this sample:
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 * <th class="title">Name</th>
 * <th class="title">Description</th>
 * <th class="title">Location</th>
 * </tr>
 * <tr>
 * <th>application.c</th>
 * <td>Contains application_run() and the application callback</td>
 * <td>samples/device_request</td>
 * </tr>
 * <tr>
 * <th>device_request.c</th>
 * <td>Rountines used to process the data service device requests</td>
 * <td>samples/device_request</td>
 * </tr>
 * <tr>
 * <th>device_request.py</th>
 * <td>Python script to send device request</td>
 * <td>samples/device_request</td>
 * </tr>
 * <tr>
 *   <th>connector_config.h</th>
 *   <td>Etherios Cloud Connector options</td>
 *   <td>samples/device_request</td>
 * </tr>
 * <tr>
 * <th>connector_api.c</th>
 * <td>Code for Etherios Cloud Connector </td>
 * <td>private</td>
 * </tr>
 * <tr>
 * <th>os.c</th>
 * <td>Operating system calls</td>
 * <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 * <th>network_tcp.c</th>
 * <td> Network interface for TCP</td>
 * <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 * <th>config.c</th>
 * <td>Configuration routines</td>
 * <td>platforms/<i>my_platform</i></td>
 * </tr>
 * <tr>
 * <th>main.c</th>
 * <td>Starting point of program, dependent on build environment</td>
 * <td>platforms/<i>my_platform</i></td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * @subsection include_paths Include Paths
 *
 * The following include paths are required:
 *
 * @li public/include
 * @li run/platforms/linux (substitute with your platform)
 *
 * @section python_script Python script to send device request
 *
 * This sample provides a simple python script to send a data service device request to login.etherios.com
 * using @htmlonly <a href="web_services.html">Etherios Device Cloud Web Services.</a> @endhtmlonly
 * To run the python script for this sample:
 *
 * @code
 * python device_request.py <username> <password> <device_id>
 * @endcode
 *
 * Output from this sample:
 *
 * @code
 * Device request data: received data = "My device request data" for target = "myTarget"
 * Device response data: send response data = My device response data
 * @endcode

 * Output from the device_request.py:
 *
 * @code
 *
 * Response:
 * <sci_reply version="1.0">
 *    <data_service>
 *        <device id="00000000-00000000-00000000-00000000">
 *            <requests>
 *               <device_request target_name="myTarget" status="0">
 *               My device response data
 *               </device_request>
 *            </requests>
 *        </device>
 *    </data_service>
 * </sci_reply>
 * @endcode
 *
 * @htmlinclude terminate.html
 */
