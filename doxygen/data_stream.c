/*! @page data_stream Data Stream Sample
 *
 * @htmlinclude nav.html
 *
 * @section data_stream_overview Overview
 *
 * This sample demonstrates how to send streaming data from a device to Etherios Device Cloud.
 * Using the @ref data_service_overview "data service api" this application sends periodic
 * data points in xml format. The Etherios Device Cloud (with Data Stream enabled) on the
 * otherhand collects these data and the data shows up under Data Streams page inside the Data
 * Service tab.
 *
 * The formatted sample streaming data looks like this:
 *
 * @code
 * <connector_data> <sample>
 *     <name>sensor0.noise_level</name>
 *     <value>78</value>
 *     <unit>dB</unit>
 *     <timestamp>2012-07-31 18:56:46</timestamp>
 * </sample></connector_data>
 * @endcode
 *
 * The name (Stream ID) is represented as instance.channel, where the instance is device name
 * (senser0 in this sample) and the channel is actual name (noise_level in this sample).
 *
 * The timestamp in the above sample is in UTC. This is an optional field, the Cloud will use browser
 * timing by default (if timestamp is not specified). If you prefer to use the localtime then make
 * sure to include the timezone info in the timestamp.
 *
 * To learn more about iDigi Data Streams, see Chapter 12 iDigi Data Streams in the
 * @htmlonly <a href="http://ftp1.digi.com/support/documentation/90001150_H.pdf">iDigi User's Guide</a>@endhtmlonly.
 *
 * @section data_stream_location Source Code Location
 *
 *      - For the connector_run() version:
 *           - @htmlonly<I>/connector/public/run/samples/data_stream</I>@endhtmlonly
 *
 *      - For the connector_step() version:
 *          - @htmlonly<I>/connector/public/step/samples/data_stream</I>@endhtmlonly
 *
 * @note Check @ref threading "here for more information" on using the connector_run()
 * versus connector_step() model.
 *
 * @section connect_description Code Flow
 *
 * The routine main() in the platform directory initializes Etherios Cloud Connector and
 * registers the application-defined callback. In the file application.c is the
 * application-defined callback app_connector_callback(), this routine calls app_data_service_handler()
 * when a data service request is received.
 *
 * The routine application_run() (or application_step()) periodically calls app_send_put_request()
 * to initiate the data transfer to Etherios Device Cloud. This routine formats a data point and
 * initiates the data transfer by calling connector_initiate_action(); once the server is
 * ready to receive data the app_data_service_handler() callback is called. This callback copies
 * the formatted data into its buffer and sets the flags @ref CONNECTOR_MSG_LAST_DATA and @ref CONNECTOR_MSG_FIRST_DATA
 * indicate this is the only message in this data transfer. By modifying these flags you could
 * send multiple blocks of data back to the server.
 *
 * If the app_data_service_handler() is called with @ref connector_data_service_type_have_data then this
 * indicates a response from the server, @ref connector_data_service_type_error would indicate
 * an error is sent back from the server.  In this example app_send_put_request() is called
 * periodically (intially at faster rate just to demonstrate the Data Stream Chart on the Cloud and
 * after few minutes it sends the data once a minute) from application_run() (or application_step()).
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
 *   <td>samples/data_stream</td>
 * </tr>
 * <tr>
 *   <th>put_request.c</th>
 *   <td>data service callback and send_put_request() which starts the data transfer</td>
 *   <td>samples/data_stream</td>
 * </tr>
 * <tr>
 *   <th>connector_config.h</th>
 *   <td> iDigi Connector options</td>
 *   <td>samples/data_stream</td>
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
 *   <td> Network interface for TCP </td>
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
 * @section web_gui Viewing results from Etherios Device Cloud
 *
 * To view the results for this sample, log on to Etherios Device Cloud as described
 * in the @ref add_your_device_to_the_cloud "Getting Started" section.
 *
 * Once logged in click on the Data Services tab as shown below. Then select the Data
 * Streams tab. This should show your channel (senser0/noise_level). Select the channel to
 * see the Data Stream charts and raw data.
 *
 * @image html cloud_data_stream_selection.png
 *
 * @htmlinclude terminate.html
 *
 */
