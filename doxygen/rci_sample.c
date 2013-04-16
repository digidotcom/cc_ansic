/*! @page rci_sample Remote Configuration Simple Sample
 *
 * @htmlinclude nav.html
 *
 * @section rci_sample_overview Overview
 *
 * This sample demonstrates how to retreive and set device configurations using @ref rci_service "remote configuration callbacks".
 * This is a simple sample which does not write on the device but demonstrates the remote configuration callback.
 *
 * @section rci_sample_location Source Code Location
 *
 *      - For the connector_run() version:
 *           - @htmlonly<I>/connector/public/run/samples/simple_remote_config</I>@endhtmlonly
 *
 *      - For the connector_step() version:
 *          - @htmlonly<I>/connector/public/step/samples/simple_remote_config</I>@endhtmlonly
 *
 * @note Check @ref threading "here for more information" on using the connector_run()
 * versus connector_step() model.
 *
 * @section connect_description Code Flow
 *
 * The routine main() in the platform directory initializes Etherios Cloud Connector and
 * registers the application callback.  In the file application.c is the
 * application defined callback app_connector_callback(), this routine calls app_remote_config_handler()
 * when a remote command interface (RCI) request is received.
 *
 * A User can use the included python script to send a device request to the device.
 *
 * This sample defines a configuration group with the following confgiruations:
 * - System
 *   - Description
 *   - Contact
 *   - Location
 * - GPS
 *   - Latitude
 *   - Longitude
 * - Device security (optional)
 *   - Connection security
 *   - Password
 *
 * @subsection config_file Etherios Cloud Connector configuration file
 *
 * The following is Etherios Cloud Connector configuration file:
 *
 * @code
 * globalerror load_fail "Load fail"
 * globalerror save_fail "Save fail"
 * globalerror memory_fail "Insufficient memory"
 *
 * # Device info that shows up in Etherios Device Cloud under Device Manger
 * # Device Manager queries this system setting to display the information in root folder
 * group setting system "System"
 *     element description "Description" type string max 63
 *     element contact "Contact" type string max 63
 *     element location "Location" type string max 63
 *     error invalid_length "Invalid Length"
 *
 * # Etherios Device Cloud password
 * # Etherios Device Cloud uses this devicesecurity setting for password authentication
 * # Note, as with all password, password type is a write-only field
 * # To include the devicesecurity setting, change devicesecurity.c.optional
 * # to devicesecurity.c
 * # group setting devicesecurity "Device Cloud device security"
 * #    element identityVerificationForm "Connection security" type enum
 * #        value simple "No connection security"
 * #        value password "Connection is password authenticated"
 * #    element password "Device connection password" type password max 133
 *
 * # Device location
 * # State configuration for GPS
 * # Must setup the following group for Latitude and Longitude to be shown in Etherios Device Cloud.
 * group state gps_stats "GPS"
 *     element latitude "Latitude" type string access read_only
 *     element longitude "Longitude" type string access read_only
 * @endcode
 *
 *
 * Note that the device security is not included. Remove the comment tags in configuration file to include it and
 * change the devicesecurity.c.optional to devicesecurity.c. Add this devicesecurity.c to the Makefile.
 *
 * Run @ref rci_tool to generate remote_config.h:
 * @code
 *
 *   java -jar /connector/tools/ConfigGenerator.jar username:password "Linux Application" 1.0.0.0 config.rci
 *
 * @endcode
 *
 * When Etherios Device Cloud sends a RCI request to the device, app_remote_config_handler() is called with the following request ID:
 * -# @ref connector_request_id_remote_config_session_start
 * -# @ref connector_request_id_remote_config_action_start
 * -# @ref connector_request_id_remote_config_group_start
 * -# @ref connector_request_id_remote_config_group_process
 * -# @ref connector_request_id_remote_config_group_end
 * -# @ref connector_request_id_remote_config_action_end
 * -# @ref connector_request_id_remote_config_session_end
 *
 * The callback with @ref connector_request_id_remote_config_group_process request ID is continually called
 * until all elements in the configuration group is processed.
 *
 * When the callback is called with @ref connector_request_id_remote_config_session_cancel request ID it indicates
 * an error is encountered and needs to cancel the RCI. In this example it cancels
 * the request by freeing any memory used.
 *
 *
 * @section connect_build Building
 *
 * To build this example on a Linux system, go to the public/run/samples/simple_remote_config
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
 * <th>config.rci</th>
 * <td>Remote Configuration file</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <th>application.c</th>
 * <td>Contains application_run() and the application defined callback</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <th>remote_config.h</th>
 * <td>Remote configuration definitions generated by the @endhmtlonly @ref rci_tool @htmlonly </td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <th>remote_config_cb.c</th>
 * <td>Routines used to remote configuration callback</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <th>remote_config_cb.h</th>
 * <td>Header for remote configuration callback</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <th>gps_stats.c</th>
 * <td>GPS remote configuration callback</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <th>system.c</th>
 * <td>System remote configuration callback</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <th>status.c</th>
 * <td>Status calls</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <th>devicesecurity.c.optional</th>
 * <td>Device security remote configuration callback (optional)</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
 * <tr>
 * <tr>
 * <th>connector_config.h</th>
 * <td>Etherios Cloud Connector options</td>
 * <td>samples/simple_remote_config</td>
 * </tr>
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
 * <tr>
 * <th>network_tcp.c</th>
 * <td> Network interface </td>
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
 * @li run/platforms/@a my_platform
 *
 * @section running Running
 *
 * Once successfully built, run the executable, in Linux type ./connector to run the
 * sample.
 *
 * Log on to @htmlonly <a href="http://login.etherios.com/">Etherios Device Cloud</a>@endhtmlonly
 * (described in the @ref connector_login "Getting Started Section").
 * Once you are logged, go to Device Manager tab and click the Refresh button.
 * The device's status should show as 'Connected'.
 *
 * @image html cloud_rci_connected.png
 *
 * Then double click the device. Click the Refresh button to update the configurations.
 *
 * @image html cloud_rci_refresh.png
 *
 * Click the System item under the Configuration link. Click the Save button
 * after updating the configurations.
 *
 * @image html cloud_configuration_save.png
 *
 * Go back to Home page and click the Refresh button, it will show the new configuration data.
 *
 * @image html cloud_home_refresh.png
 *
 * @htmlinclude terminate.html
 *
 */
