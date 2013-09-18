/*! @page config_callbacks Configuration Callbacks
 *
 * @htmlinclude nav.html
 *
 * @section config Configuration Callbacks
 * Cloud Connector interfaces to the platform's configuration through the callbacks listed below.  These
 * are called through the application callback described in the @ref api1_overview.
 *
 *  -# @ref get_device_id
 *  -# @ref set_device_id
 *  -# @ref vendor_id
 *  -# @ref device_type
 *  -# @ref device_cloud_url
 *  -# @ref connection_type
 *  -# @ref mac_address
 *  -# @ref link_speed
 *  -# @ref phone_number
 *  -# @ref tx_keepalive
 *  -# @ref rx_keepalive
 *  -# @ref wait_count
 *  -# @ref ip_address
 *  -# @ref error_status
 *  -# @ref firmware_support
 *  -# @ref data_service_support
 *  -# @ref file_system_support
 *  -# @ref rci_support
 *  -# @ref max_msg_transactions
 *  -# @ref device_id_method
 *  -# @ref network_tcp_start
 *  -# @ref network_udp_start
 *  -# @ref network_sms_start
 *  -# @ref wan_type
 *  -# @ref imei_number
 *  -# @ref esn_number
 *  -# @ref meid_number
 *  -# @ref identity_verification
 *  -# @ref password
 *  -# @ref get_device_cloud_phone
 *  -# @ref set_device_cloud_phone
 *  -# @ref device_cloud_service_id
 *
 * @section get_device_id Get the Device ID
 * Returns a unique Device ID used to identify the device.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_device_id() in config.c. It is called to get a unique device ID 
 * which is used to identify the device, typically, it will be read from a non-volatile media.
 *
 * Device IDs are a globally unique 16-octet value identifier for Device Cloud clients.
 * If the @ref device_id_method is set to @ref connector_device_id_method_manual, this function
 * is called to provide the Cloud Connector with a previously saved (e.g.: to a file or EEPROM)
 * Device ID. Else, if @ref device_id_method is set to @ref connector_device_id_method_auto, they
 * Device ID will be generated either from the MAC address (if @ref connector_connection_type_lan
 * specified) or from MEID/IMEI/ESN (if @ref connector_connection_type_wan).
 * If this function sets config_device_id->data to a zeroed buffer, the Cloud Connector will ask Device Cloud for a 
 * Device ID and then call function @ref app_set_device_id so the user saves to a non-volatile
 * storage that provisioned Device ID. In future starts, this Device Cloud-assigned Device ID must be returned
 * in this function.
 * @note If Device Cloud provides a Device ID, it automatically adds that Device ID to the Device Cloud
 * account which @ref vendor_id is provided.
 * @note Provision a Device ID can only be done by TCP transport.
 *
 * @see @ref add_your_device_to_the_cloud "Adding your Device to Device Cloud"
 * @see app_get_device_id()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_device_id @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_data_t @htmlonly:
 *          <dl>
 *              <dt><i>data</i></dt>
 *              <dd> - Callback sets the pointer which contains the Device ID</dd>
 *              <dt><i>bytes_required</i></dt><dd> - Length of the Device ID in bytes.</dd>
 *            </dl>
 * </td>
 *
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned device ID</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get Device ID and Cloud Connector will be aborted</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 * #define DEVICE_ID_LENGTH    16
 * #define DEVICE_ID_FILENAME  "device_id.cfg"
 * 
 * static uint8_t provisioned_device_id[DEVICE_ID_LENGTH];
 * 
 * static connector_callback_status_t app_get_device_id(connector_config_pointer_data_t * const config_device_id)
 * {
 *     if (access(DEVICE_ID_FILENAME, F_OK) != -1)
 *     {
 *         FILE *file;
 *         int bytes_read;
 * 
 *         file = fopen(DEVICE_ID_FILENAME, "r");
 *         bytes_read = fread(provisioned_device_id, sizeof provisioned_device_id[0], sizeof provisioned_device_id / sizeof provisioned_device_id[0], file);
 *         ASSERT(bytes_read == sizeof provisioned_device_id);
 *         fclose(file);
 *     }
 * 
 *     config_device_id->data = provisioned_device_id;
 * 
 *     return connector_callback_continue;
 * }
 * @endcode
 *
 * @section set_device_id Set the Device ID
 * Saves the Device ID into a non-volatile storage (EEPROM, file, NVRAM, etc.).
 *
 * This routine is called when a zero'ed Device ID is provided in @ref app_get_device_id and 
 * @ref device_id_method is set to @ref connector_device_id_method_manual.
 * In this function the provided Device ID must be saved to a non-volatile storage to be read in future
 * access by @ref app_get_device_id function.
 * @note Provision a Device ID can only be done by TCP transport.
 * @param [in] config_device_id  pointer to memory containing the device ID to be stored in non-volatile storage.
 *
 * @retval connector_callback_continue  Device ID was successfully written.
 * @retval connector_callback_abort     Could not set the device ID, abort Cloud Connector.
 *
 * @see @ref device_id API Configuration Callback
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_device_id @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_data_t @htmlonly:
 *          <dl>
 *              <dt><i>data</i></dt>
 *              <dd> - Pointer which contains application's Device ID</dd>
 *              <dt><i>bytes_required</i></dt><dd> - Length of the Device ID (16 bytes).</dd>
 *            </dl>
 * </td>
 *
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned device ID</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to save Device ID and Cloud Connector will be aborted</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 * #define DEVICE_ID_LENGTH    16
 * #define DEVICE_ID_FILENAME  "device_id.cfg"
 * 
 * static uint8_t provisioned_device_id[DEVICE_ID_LENGTH];
 * 
 * static connector_callback_status_t app_get_device_id(connector_config_pointer_data_t * const config_device_id)
 * {
 *     if (access(DEVICE_ID_FILENAME, F_OK) != -1)
 *     {
 *         FILE *file;
 *         int bytes_writen;
 * 
 *         file = fopen(DEVICE_ID_FILENAME, "w+");
 *         bytes_writen = fwrite(config_device_id->data, sizeof config_device_id->data[0], sizeof provisioned_device_id / sizeof provisioned_device_id[0], file);
 *
 *         ASSERT(bytes_writen == sizeof provisioned_device_id);
 *         fclose(file);
 *     }
 * 
 *     config_device_id->data = provisioned_device_id;
 * 
 *     return connector_callback_continue;
 * }
 * @endcode
 * 
 * @section vendor_id Vendor ID
 *
 * Return vendor ID which is a unique code identifying the manufacturer of a device.
 * Vendor IDs are assigned to manufacturers by Device Cloud.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_vendor_id() in config.c.
 *
 * @note If @ref CONNECTOR_VENDOR_ID configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @see @ref connector_vendor_id "Obtaining an Device Cloud Vendor ID"
 * @see app_get_vendor_id()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_vendor_id @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_vendor_id_t @htmlonly:
 *          <dl>
 *              <dt><i>id</i></dt>
 *              <dd> - Callback writes 4-byte vendor ID </dd>
 *            </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned vendor ID</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get vendor ID and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_get_vendor_id(connector_config_vendor_id_t * const config_vendor)
 * {
 *
 *   static uint32_t const device_vendor_id = 0xFE000000;
 *
 *   config_vendor->id  =  device_vendor_id;
 *
 *   return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section device_type Device Type
 *
 * Returns a pointer to the device type which is an iso-8859-1 encoded string.
 * This string should be chosen by the device manufacturer as a name that uniquely
 * identifies this model of device  to Device Cloud. When Device Cloud finds two devices
 * with the same device type, it can infer that they are the same product and
 * product-scoped data may be shared among all devices with this device type.
 * A device's type cannot be an empty string, nor contain only whitespace.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_device_type() in config.c.
 *
 * @note If @ref CONNECTOR_DEVICE_TYPE configuration is defined in @ref connector_config.h, this callback
 * will not be called.
 *
 * @see @ref connector_config_data_options
 * @see app_get_device_type()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_device_type @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_string_t @htmlonly:
 *          <dl>
 *              <dt><i>string</i></dt>
 *              <dd> - Callback returns the pointer which contains device type</dd>
 *              <dt><i>length</i></dt><dd> - Callback returns number of bytes in the device type</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned device type</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get device type and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void  * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_device_type)
 *     {
 *         static char const device_type[] = "Linux Application";
 *
 *         connector_config_pointer_string_t * const config_type = data;
 *         // Return pointer to device type.
 *         config_device_type->string = (char *)device_type;
 *         config_device_type->length = sizeof device_type -1;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section device_cloud_url Device Cloud URL
 *
 * Return Device Cloud FQDN.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_device_cloud_url() in config.c.
 *
 * @note If @ref CONNECTOR_CLOUD_URL configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @see app_get_device_cloud_url()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_device_cloud_url @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_string_t @htmlonly:
 *          <dl>
 *              <dt><i>string</i></dt>
 *              <dd> - Callback returns the pointer to FQDN of Device Cloud to be connected.</dd>
 *              <dt><i>length</i></dt><dd> - Callback returns number of bytes of Device Cloud FQDN. Maximum is 64 bytes.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned Device Cloud URL</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get Device Cloud URL and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_device_cloud_url)
 *     {
 *         // Return pointer to Device Cloud url.
 *         static  char const device_cloud_url[] = "login.etherios.com";
 *         connector_config_pointer_string_t * const config_url = data;
 *
 *         config_url->string = (char *)device_cloud_url;
 *         config_url->length = sizeof device_cloud_url -1;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section connection_type Connection Type
 *
 * Return LAN or WAN connection type.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_connection_type() in config.c.
 *
 * @note If @ref CONNECTOR_CONNECTION_TYPE configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @see app_get_connection_type()
 *
 * @htmlonly
 * <table class="apitable">
 *   <tr>
 *     <th colspan="2" class="title">Arguments</th>
 *   </tr>
 *   <tr>
 *     <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 *    </tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_connection_type @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_connection_type_t @htmlonly:
 *          <dl>
 *              <dt><i>type</i></dt>
 *              <dd> <ul><li> @endhtmlonly @ref connector_connection_type_lan @htmlonly - Callback returns this for LAN connection type</li>
 *                       <li> @endhtmlonly @ref connector_connection_type_wan @htmlonly - Callback returns this for WAN connection type</li></ul>
 *              </dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned connection type</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get connection type and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void  * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_config_connection_type)
 *     {
 *         connector_config_connection_type_t * const config_connection = data;
 *
 *         config_connection->type = connector_connection_type_lan;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section mac_address MAC Address
 *
 * Return the device's MAC address.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_mac_addr() in config.c.
 *
 * @see app_get_mac_addr()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_mac_addr @htmlonly</td>
 * </tr>
 * <tr>
* <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_data_t @htmlonly:
 *          <dl>
 *              <dt><i>data</i></dt>
 *              <dd> - Callback returns the pointer which contains device's MAC address</dd>
 *              <dt><i>bytes_required</i></dt><dd> - Contains 6 bytes which is required for the device's MAC address in the data pointer</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned MAC address</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get MAC address and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * #define MAC_ADDR_LENGTH     6
 * static uint8_t const device_mac_addr[MAC_ADDR_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
 *
 * static connector_callback_status_t app_get_mac_addr(connector_config_pointer_data_t * const config_mac)
 * {
 *
 *   ASSERT(config_mac->bytes_required == MAC_ADDR_LENGTH);
 *
 *   config_mac->data = (uint8_t *)device_mac_addr;
 *
 *   return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section link_speed Link Speed
 *
 * Return link speed for a WAN connection type. If connection
 * type is LAN, Cloud Connector will not request link speed configuration..
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_link_speed() in config.c.
 *
 * @note If @ref CONNECTOR_WAN_LINK_SPEED_IN_BITS_PER_SECOND configuration is defined in @ref connector_config.h,
 * this callback will not be called. See @ref connector_config_data_options
 *
 * @see app_get_link_speed()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_link_speed @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_link_speed_t @htmlonly:
 *          <dl>
 *              <dt><i>speed</i></dt>
 *              <dd> - Callback writes 4-byte integer speed of the link in bits per second. All zeros represent unknown speed.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned link speed</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get link speed and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_config_link_speed)
 *     {
 *         connector_config_link_speed_t * const config_link = data;
 *         config_link->speed = 0;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section phone_number Phone Number
 *
 * Return the phone number dialed for a WAN connection type.
 * The phone number is a variable length, non null-terminated string.
 * If connection type is LAN, Cloud Connector will not request phone number
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_phone_number() in config.c.
 *
 * @note If @ref CONNECTOR_WAN_PHONE_NUMBER_DIALED configuration is defined in @ref connector_config.h,
 * this callback will not be called.  See @ref connector_config_data_options
 *
 * @see app_get_phone_number()
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
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_phone_number @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_string_t @htmlonly:
 *          <dl>
 *              <dt><i>string</i></dt>
 *              <dd> - Callback returns pointer to memory which contains the phone number dialed, including any dialing prefixes. </dd>
 *              <dt><i>length</i></dt>
 *              <dd> - Callback returns number of bytes of the phone number in the string pointer.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned phone number</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get phone number and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_config_phone_number)
 *     {
 *         static char const phone_number[] ="000-000-0000";
 *         connector_config_pointer_string_t * const config_phone_number = data;
 *
 *         config_phone_number->string = (char *)phone_number;
 *         config_phone_number->length = sizeof phone_number -1;
 *
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section tx_keepalive TX Keepalive Interval
 *
 * Return TX keepalive interval in seconds. This tells how
 * often Device Cloud sends keepalive messages to the device.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_tx_keepalive_interval() in config.c.
 *
 * @note If @ref CONNECTOR_TX_KEEPALIVE_IN_SECONDS is defined in @ref connector_config.h,
 * this callback will not be called.  See @ref connector_config_data_options
 *
 * @see app_get_tx_keepalive_interval()
 *
 * @htmlonly
 * <table class="apitable">
 *   <tr>
 *     <th colspan="2" class="title">Arguments</th>
 *   </tr>
 *   <tr>
 *     <th class="subtitle">Name</th>
 *     <th class="subtitle">Description</th>
 *   </tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_tx_keepalive @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_keepalive_t @htmlonly:
 *          <dl>
 *              <dt><i>interval_in_seconds</i></dt>
 *              <dd> - Callback writes 2-byte integer TX keep alive interval in seconds. It must be between 5 and 7200 seconds.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned TX keepalive interval</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get TX keepalive interval and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_tx_keepalive)
 *     {
 *         #define DEVICE_TX_KEEPALIVE_INTERVAL_IN_SECONDS     90
 *         connector_config_keepalive_t * const config_keepalive = data;
 *
 *         // Return Tx keepalive interval in seconds
 *         config_keepalive->interval_in_seconds = DEVICE_TX_KEEPALIVE_INTERVAL_IN_SECONDS;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section rx_keepalive RX Keepalive Interval
 *
 * Return RX keepalive interval in seconds. This tells how
 * often Cloud Connector sends keepalive messages to Device Cloud (device to Device Cloud).
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_rx_keepalive_interval() in config.c.
 *
 * @note If @ref CONNECTOR_RX_KEEPALIVE_IN_SECONDS configuration is defined in @ref connector_config.h,
 * this callback will not be called.  See @ref connector_config_data_options
 *
 * @see app_get_rx_keepalive_interval()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_rx_keepalive @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_keepalive_t @htmlonly:
 *          <dl>
 *              <dt><i>interval_in_seconds</i></dt>
 *              <dd> - Callback writes 2-byte integer RX keep alive interval in seconds. It must be between 5 and 7200 seconds.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned RX keepalive interval</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get RX keepalive interval and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_rx_keepalive)
 *     {
 *         #define DEVICE_RX_KEEPALIVE_INTERVAL_IN_SECONDS     60
 *         connector_config_keepalive_t * const config_keepalive = data;
 *
 *         // Return Tx keepalive interval in seconds
 *         config_keepalive->interval_in_seconds = DEVICE_RX_KEEPALIVE_INTERVAL_IN_SECONDS;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section wait_count Wait Count
 *
 * Return the number of times of not receiving a keepalive message which a connection
 * should be considered lost.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_wait_count() in config.c.
 *
 * @note If @ref CONNECTOR_WAIT_COUNT configuration is defined in connector_config.h,
 * this callback will not be called.  See @ref connector_config_data_options
 *
 * @see app_get_wait_count()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_wait_count @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_wait_count_t @htmlonly:
 *          <dl>
 *              <dt><i>count</i></dt>
 *              <dd> - Callback writes 2-byte wait count. It must be between 2 and 63 times.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned wait count</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get wait count and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_wait_count)
 *     {
 *         #define DEVICE_WAIT_COUNT     5
 *         connector_config_wait_count_t * const config_wait = data;
 *
 *          // Return wait count (number of times not receiving Tx keepalive
 *          // from Device Cloud is allowed).
 *         config_wait->count = DEVICE_WAIT_COUNT;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section ip_address Device IP Address
 *
 * Return a unique device IP address. The IP address must be either
 * an IPv4 or IPv6 address.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_ip_address() in config.c.
 *
 * @see app_get_ip_address()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_ip_addr @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_ip_address_t @htmlonly:
 *          <dl>
 *              <dt><i>address</i></dt>
 *              <dd> - Callback returns pointer to device's IP address.</dd>
 *              <dt><i>ip_address_type</i></dt>
 *              <dd> - <ul><li>@endhtmlonly @ref connector_ip_address_ipv4 @htmlonly - Callback returns this for IPv4 address. The size of the address returned must be 4 bytes.</li>
 *                         <li>@endhtmlonly @ref connector_ip_address_ipv6 @htmlonly - Callback returns this for IPv6 address. The size of the address returned must be 16 bytes.</li></ul></dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned IP address</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get IP address and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_ip_addr)
 *     {
 *         extern uint32_t device_ip_address;
 *
 *         connector_config_ip_address_t * const config_ip = data;

 *         config_ip->ip_address_type = connector_ip_address_ipv4;
 *         config_ip->address = (uint8_t *)&device_ip_address;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section error_status  Error Status Notification
 *
 * This callback is called to notify the user that Cloud Connector encountered an error. When Cloud Connector finds an
 * error, Cloud Connector will call this callback indicating the error status
 *
 * @note If @ref CONNECTOR_DEBUG  is not defined in connector_config.h, this callback
 * will not be called.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_error_status @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_error_status_t @htmlonly containing
 *     the class id, request id, and error status which Cloud Connector encountered error with:
 *          <dl>
 *              <dt><i>class_id</i></dt><dd> - Class ID </dd>
 *              <dt><i>request_id</i></dt><dd> - Request ID</dd>
 *              <dt><i>status</i></dt><dd> - Error status</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>None</td>
 * <td>None</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * void app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_error_status)
 *     {
 *         connector_config_error_status_t * error_data = data;
 *         printf("connector_error_status: unsupport class_id = %d request_id = %d status = %d\n",
 *                   error_data->class_id, error_data->request_id.config_request, error_data->status);
 *     }
 *     return;
 * }
 *
 * @endcode
 *
 * @section firmware_support  Firmware Access Support
 *
 * Return @ref connector_config_supported_t status to enable or disable Firmware
 * download capability. If it's supported, callback for @ref connector_request_id_firmware_t
 * must be implemented for firmware download. This callback allows application to enable
 * or disable firmware download capability during runtime.
 *
 * @note If @ref CONNECTOR_FIRMWARE_SERVICE configuration is not defined in @ref connector_config.h, this callback
 * will not be called and Firmware Download is not supported. Cloud Connector does not include firmware support.
 *
 * @note If @ref CONNECTOR_FIRMWARE_SUPPORT is defined in @ref connector_config.h, this callback is not needed.
 * It enables firmware download capability. See @ref connector_config_data_options. CONNECTOR_FIRMWARE_SERVICE must be defined.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_firmware_facility @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_supported_t @htmlonly:
 *          <dl>
 *              <dt><i>supported</i></dt><dd> -
 *                     <ul><li>@endhtmlonly @ref connector_false @htmlonly - callback writes this if it doesn't support firmware download.</li>
 *                         <li>@endhtmlonly @ref connector_true @htmlonly - callback writes this if it supports firmware download.</li></ul></dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned the status</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_firmware_facility)
 *     {
 *         connector_config_supported_t * const config_firmware = data;
 *         config_firmware->supported = connector_true;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section data_service_support  Data Service Support
 *
 * Enable or disable data service capability. If it's supported, callback
 * for @ref connector_request_id_config_data_service must be
 * implemented for data service. This callback allows application to enable
 * or disable data service capability during runtime.
 *
 * @note If @ref CONNECTOR_DATA_SERVICE configuration is not defined in @ref connector_config.h, this callback
 * will not be called and Data Service is not supported. Cloud Connector does not include data service.
 *
 * @note If @ref CONNECTOR_DATA_SERVICE_SUPPORT is defined in @ref connector_config.h, this callback is not needed.
 * It enables data service capability. See @ref connector_config_data_options. CONNECTOR_DATA_SERVICE must be defined.
 *
 * @note Define @ref CONNECTOR_COMPRESSION in @ref connector_config.h for compression transfer.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_data_service @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_supported_t @htmlonly:
 *          <dl>
 *              <dt><i>supported</i></dt><dd> -
 *                     <ul><li>@endhtmlonly @ref connector_false @htmlonly - callback writes this if it doesn't support data service.</li>
 *                         <li>@endhtmlonly @ref connector_true @htmlonly - callback writes this if it supports data service.</li></ul></dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned the status</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_data_service)
 *     {
 *         connector_config_supported_t * const config_data_service = data;
 *         config_data_service->supported = connector_true;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section file_system_support  File System Support
 *
 * Enable or disable file system capability. If it's supported, callback for
 * @ref connector_request_id_config_file_system must be
 * implemented for file system. This callback allows application to enable
 * or disable file system capability during runtime.
 *
 * @note If @ref CONNECTOR_FILE_SYSTEM configuration is not defined in @ref connector_config.h, this callback
 * will not be called and File System is not supported. Cloud Connector does not include file system.
 *
 * @note If @ref CONNECTOR_FILE_SYSTEM_SUPPORT is defined in @ref connector_config.h, this callback is not needed.
 * It enables file system capability. See @ref connector_config_data_options. CONNECTOR_FILE_SYSTEM must be defined.
 *
 * @note Define @ref CONNECTOR_COMPRESSION in @ref connector_config.h for compression transfer.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_file_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_supported_t @htmlonly:
 *          <dl>
 *              <dt><i>supported</i></dt><dd> -
 *                     <ul><li>@endhtmlonly @ref connector_false @htmlonly - callback writes this if it doesn't support file system.</li>
 *                         <li>@endhtmlonly @ref connector_true @htmlonly - callback writes this if it supports file system. </li></ul></dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned the status</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_file_system)
 *     {
 *         connector_config_supported_t * const config_file_system = data;
 *         config_file_system->supported = connector_true;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section rci_support  Remote Configuration Support
 *
 * Enable or disable remote configuration capability. If it's supported, callback for
 * @ref connector_request_id_config_remote_configuration must be
 * implemented for remote configuration. This callback allows application to enable
 * or disable remote configuration capability during runtime.
 *
 * @note If @ref CONNECTOR_RCI_SERVICE is not defined in @ref connector_config.h, this callback
 * will not be called and remote configuration is not supported. Cloud Connector does not include remote configuration.
 *
 * @note If @ref CONNECTOR_REMOTE_CONFIGURATION_SUPPORT is defined in @ref connector_config.h, this callback is not needed.
 * It enables remote configuration capability. See @ref connector_config_data_options.
 *
 * @note The @ref CONNECTOR_RCI_MAXIMUM_CONTENT_LENGTH must be set to the maximum content length in bytes of an element's value in @ref connector_config.h.
 *
 * @note Define @ref CONNECTOR_COMPRESSION in @ref connector_config.h for compression transfer.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_remote_configuration @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_supported_t @htmlonly:
 *          <dl>
 *              <dt><i>supported</i></dt><dd> -
 *                     <ul><li>@endhtmlonly @ref connector_false @htmlonly - callback writes this if it doesn't support remote configuration.</li>
 *                         <li>@endhtmlonly @ref connector_true @htmlonly - callback writes this if it supports remote configuration.</li></ul></dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned the status</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_remote_configuration)
 *     {
 *         connector_config_supported_t * const config_rci = data;
 *         config_rci->supported = connector_true;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section max_msg_transactions Maximum Message Transactions
 *
 * Return maximum simultaneous transactions for data service and file system to receive message from
 * Device Cloud. This configuration is required if @ref data_service_support or @ref file_system_support
 * is enabled.
 *
 * @note If @ref CONNECTOR_MSG_MAX_TRANSACTION configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_max_transaction @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_max_transaction_t @htmlonly:
 *          <dl>
 *              <dt><i>count</i></dt><dd>Callback writes maximum simultaneous transactions. Use 0 for unlimited transactions.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned maximum transactions</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                   connector_request_id_t const request_id
 *                                                   void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_max_transaction)
 *     {
 *         #define CONNECTOR_MAX_MSG_TRANSACTIONS   1
 *         connector_config_max_transaction_t * const config_max_transaction = data;
 *
 *         config_max_transaction->count = CONNECTOR_MAX_MSG_TRANSACTIONS;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section device_id_method Device ID Method
 * Returns device ID method on how to obtain device ID.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_device_id_method() in config.c.
 *
 * @see @ref add_your_device_to_the_cloud "Adding your Device to Device Cloud"
 * @see app_get_device_id()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_device_id_method @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_device_id_method_t @htmlonly:
 *          <dl>
 *              <dt><i>method</i></dt><dd>
 *                  <ul><li>@endhtmlonly @ref connector_device_id_method_auto @htmlonly - Callback returns this telling Cloud Connector to generate
 *                                                            the device ID from @endhtmlonly @ref mac_address callback for LAN connection type or
 *                                                            generate the device ID according to the @ref wan_type @htmlonly for WAN connection type.</li>
 *                      <li>@endhtmlonly @ref connector_device_id_method_manual @htmlonly - Callback returns this telling Cloud Connector to retrieve
 *                                                            the device ID from @endhtmlonly @ref connector_request_id_config_device_id @htmlonly callback.</li></ul>
 *                  Note: the @endhtmlonly @ref device_id @htmlonly callback will not be called if connector_device_id_method_auto is returned.
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned device ID method</td></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get device ID method and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_device_id_method)
 *     {
 *         connector_config_device_id_method_t * const config_device = data;
 *
 *         config_device->method = connector_device_id_method_auto;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section network_tcp_start  Start network TCP
 *
 * Return @ref connector_config_connect_type_t to automatic or manual start TCP transport.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_start_network_tcp() in config.c.
 *
 * @note If @ref CONNECTOR_TRANSPORT_TCP configuration is not defined in @ref connector_config.h, this callback
 * will not be called and TCP transport is not supported. Cloud Connector does not include TCP transport.
 *
 * @note If @ref CONNECTOR_NETWORK_TCP_START is defined in @ref connector_config.h, this callback is not needed.
 * See @ref connector_config_data_options. CONNECTOR_TRANSPORT_TCP must be defined.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_network_tcp @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_connect_type_t @htmlonly:
 *          <dl>
 *              <dt><i>type</i></dt>
 *              <dd>
 *                  <ul><li>@endhtmlonly @ref connector_connect_auto @htmlonly - Callback returns this to automatic connect to Device Cloud.</li>
 *                      <li>@endhtmlonly @ref connector_connect_manual @htmlonly - Callback returns this to manual connect to Device Cloud.
 *                          @endhtmlonly Note: Call @ref connector_initiate_action with @ref connector_initiate_transport_start @htmlonly to start TCP transport.</li></ul></dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr><th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned the status</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_start_network_tcp(connector_config_connect_type_t * const device_connect)
 * {
 *    device_connect->type = connector_connect_auto;
 *
 *    return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section network_udp_start  Start network UDP
 *
 * Return @ref connector_config_connect_type_t to automatically or manually start UDP transport.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_start_network_udp() in config.c.
 *
 * @note If @ref CONNECTOR_TRANSPORT_UDP configuration is not defined in @ref connector_config.h, this callback
 * will not be called and UDP transport is not supported. Cloud Connector does not include UDP transport.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_network_udp @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_connect_type_t @htmlonly:
 *          <dl>
 *              <dt><i>type</i></dt><dd>
 *                  <ul><li>@endhtmlonly @ref connector_connect_auto @htmlonly - Callback returns this to automatic establish UDP connection to Device Cloud.</li>
 *                      <li>@endhtmlonly @ref connector_connect_manual @htmlonly - Callback returns this to manual establish UDP connection to Device Cloud.
 *                          @endhtmlonly Note: Call @ref connector_initiate_action with
 *                                                  @ref connector_initiate_transport_start @htmlonly to start UPD transport.</li></ul>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned the status</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_start_network_udp(connector_config_connect_type_t * const device_connect)
 * {
 *    device_connect->type = connector_connect_auto;
 *
 *    return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section network_sms_start  Start network SMS
 *
 * Return @ref connector_config_connect_type_t to automatically or manually start SMS transport.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_start_network_sms() in config.c.
 *
 * @note If @ref CONNECTOR_TRANSPORT_SMS configuration is not defined in @ref connector_config.h, this callback
 * will not be called and SMS transport is not supported. Cloud Connector does not include SMS transport.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_network_sms @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_connect_type_t @htmlonly:
 *          <dl>
 *              <dt><i>type</i></dt><dd>
 *                  <ul><li>@endhtmlonly @ref connector_connect_auto @htmlonly - Callback returns this to automatic establish SMS connection to Device Cloud.</li>
 *                      <li>@endhtmlonly @ref connector_connect_manual @htmlonly - Callback returns this to manual establish SMS connection to Device Cloud.
 *                          @endhtmlonly Note: Call @ref connector_initiate_action with
 *                                                  @ref connector_initiate_transport_start @htmlonly to start UPD transport.</li></ul>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned the status</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_start_network_sms(connector_config_connect_type_t * const device_connect)
 * {
 *    device_connect->type = connector_connect_auto;
 *
 *    return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section wan_type WAN Type
 *
 * Return IMEI, ESN or MEID WAN type for @ref connector_connection_type_wan returned from @ref connection_type callback.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_wan_type() in config.c.
 *
 * @note If @ref CONNECTOR_WAN_TYPE configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @note if @ref device_id_method callback returns @ref connector_device_id_method_manual or
 * @ref connection_type callback returns @ref connector_connection_type_lan,
 * this callback will not be called.
 *
 * @note If it returns @ref connector_wan_type_imei, @ref imei_number callback will be called.
 * @note If it returns @ref connector_wan_type_esn, @ref esn_number callback will be called.
 * @note If it returns @ref connector_wan_type_meid, @ref meid_number callback will be called.
 *
 * @see app_get_wan_type()
 *
 * @htmlonly
 * <table class="apitable">
 *   <tr>
 *     <th colspan="2" class="title">Arguments</th>
 *   </tr>
 *   <tr>
 *     <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 *    </tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_wan_type @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_wan_type_t @htmlonly:
 *          <dl>
 *              <dt><i>type</i></dt><dd>
 *                  <ul><li>@endhtmlonly @ref connector_wan_type_imei @htmlonly - Callback returns this for IMEI number for GSM network.</li>
 *                      <li>@endhtmlonly @ref connector_wan_type_esn @htmlonly - Callback returns this for ESN for CDMA network.</li>
 *                      <li>@endhtmlonly @ref connector_wan_type_meid @htmlonly - Callback returns this for MEID for CDMA network.</li>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned WAN type</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get WAN type and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_get_wan_type(connector_config_wan_type_t * const config_wan)
 * {
 *
 *     config_wan->type = connector_wan_type_imei;
 *
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section imei_number IMEI number
 * Returns IMEI number. This is called if @ref wan_type callback
 * returns @ref connector_wan_type_imei WAN type.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_imei_number() in config.c.
 *
 * @see @ref add_your_device_to_the_cloud "Adding your Device to Device Cloud"
 * @see app_get_device_id()
 * @see @ref device_id_method for device ID Method
 * @see @ref connection_type for connection type.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_imei_number @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_data_t @htmlonly:
 *          <dl>
 *              <dt><i>data</i></dt>
 *              <dd> - Callback returns the pointer which contains IMEI number including a check digit. Each nibble corresponds with a
 *                     decimal digit and most upper nibble must be 0.</dd>
 *              <dt><i>bytes_required</i></dt><dd> - Contains 8 bytes which is required for the IMEI number in the data pointer</dd>
 *            </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned IMEI number</td></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_get_imei_number(connector_config_pointer_data_t * const config_imei)
 * {
 *     #define APP_IMEI_LENGTH 8
 *
 *     // Each nibble corresponds a decimal digit.
 *     // Most upper nibble must be 0.
 *     static char const app_imei_number_string[] = "000000-00-000000-0";
 *     static uint8_t app_imei_number[APP_IMEI_LENGTH] = {0};
 *     int i = sizeof app_imei_number_string -1;
 *     int index = sizeof app_imei_number -1;
 *
 *     while (i > 0)
 *     {
 *         int n = 0;
 *
 *         app_imei_number[index] = 0;
 *
 *         // loop 2 times here for 2 digits (1 bytes)
 *         while (n < 2 && i > 0)
 *         {
 *             i--;
 *             if (app_imei_number_string[i] != '-')
 *             {
 *                 uint8_t value;
 *                 get_hex_digit(app_imei_number_string[i], &value);
 *                 app_imei_number[index] += (value << ((uint8_t)n * 4));
 *                 n++;
 *             }
 *         }
 *         index--;
 *     }
 *
 *     config_imei->data = app_imei_number;
 *     ASSERT(config_imei->bytes_required == sizeof app_imei_number);
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section esn_number ESN number
 *
 * Returns ESN number. This is called if @ref wan_type callback
 * returns @ref connector_wan_type_esn WAN type.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_esn_number() in config.c.
 *
 * @see @ref add_your_device_to_the_cloud "Adding your Device to Device Cloud"
 * @see app_get_wan_type()
 * @see @ref wan_type for WAN type
 * @see @ref connection_type for connection type.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_esn @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_data_t @htmlonly:
 *          <dl>
 *              <dt><i>data</i></dt>
 *              <dd> - Callback returns the pointer which contains ESN number. Each nibble corresponds a
 *                     hexadecimal digit.</dd>
 *              <dt><i>bytes_required</i></dt><dd> - Contains 4 bytes which is required for the ESN number in the data pointer</dd>
 *            </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned ESN number</td></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_get_esn(connector_config_pointer_data_t * const config_esn)
 * {
 * #define APP_ESN_HEX_LENGTH 4
 *
 *     // Each nibble corresponds a decimal digit.
 *     // Most upper nibble must be 0.
 *
 *     static char const app_esn_hex_string[] = "00000000";
 *     static uint8_t app_esn_hex[APP_ESN_HEX_LENGTH] = {0};
 *     int i = sizeof app_esn_hex_string -1;
 *     int index = sizeof app_esn_hex -1;
 *
 *     while (i > 0)
 *     {
 *         int n = 0;
 *
 *         app_esn_hex[index] = 0;
 *
 *         // loop 2 times here for 2 digits (1 bytes)
 *         while (n < 2 && i > 0)
 *         {
 *             i--;
 *             if (app_esn_hex_string[i] != '-')
 *             {
 *                 uint8_t value;
 *                 get_hex_digit(app_esn_hex_string[i], &value);
 *                 app_esn_hex[index] += (value << ((uint8_t)n * 4));
 *                 n++;
 *             }
 *         }
 *         index--;
 *     }
 *
 *     config_esn->data = app_esn_hex;
 *     ASSERT(config_esn->bytes_required == sizeof app_esn_hex);
 *
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section meid_number MEID number
 * Returns MEID number. This is called if @ref wan_type callback
 * returns @ref connector_wan_type_meid WAN type.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_meid_number() in config.c.
 *
 * @see @ref add_your_device_to_the_cloud "Adding your Device to Device Cloud"
 * @see app_get_wan_type()
 * @see @ref wan_type for WAN type
 * @see @ref connection_type for connection type.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_meid @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_data_t @htmlonly:
 *          <dl>
 *              <dt><i>data</i></dt>
 *              <dd> - Callback returns the pointer which contains MEID number. Each nibble corresponds a
 *                     hexadecimal digit. Check digit is not included.</dd>
 *              <dt><i>bytes_required</i></dt><dd> - Contains 7 bytes which is required for the MEID number in the data pointer</dd>
 *            </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned MEID number</td></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_get_meid(connector_config_pointer_data_t * const config_meid)
 * {
 * #define APP_MEID_HEX_LENGTH 7
 *
 *     // Each nibble corresponds a decimal digit.
 *     // Most upper nibble must be 0.
 *
 *     static char const app_meid_hex_string[] = "00000000000000";
 *     static uint8_t app_meid_hex[APP_MEID_HEX_LENGTH] = {0};
 *     int i = sizeof app_meid_hex_string -1;
 *     int index = sizeof app_meid_hex -1;
 *
 *     while (i > 0)
 *     {
 *         int n = 0;
 *
 *         app_meid_hex[index] = 0;
 *
 *         // loop 2 times here for 2 digits (1 bytes)
 *         while (n < 2 && i > 0)
 *         {
 *             i--;
 *             if (app_meid_hex_string[i] != '-')
 *             {
 *                uint8_t value;
 *                 get_hex_digit(app_meid_hex_string[i], &value);
 *                 app_meid_hex[index] += (value << ((uint8_t)n * 4));
 *                 n++;
 *             }
 *         }
 *         index--;
 *     }
 *
 *     config_meid->data = app_meid_hex;
 *     ASSERT(config_meid->bytes_required == sizeof app_meid_hex);
 *
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section identity_verification Identity verification form
 *
 * Return simple or password identity verification form.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_identity_verification() in config.c.
 *
 * @note If @ref CONNECTOR_IDENTITY_VERIFICATION configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @see app_get_identity_verification()
 *
 * @htmlonly
 * <table class="apitable">
 *   <tr>
 *     <th colspan="2" class="title">Arguments</th>
 *   </tr>
 *   <tr>
 *     <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 *    </tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_identity_verification @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_identity_verification_t @htmlonly:
 *          <dl>
 *              <dt><i>type</i></dt>
 *              <dd> <ul><li>@endhtmlonly @ref connector_identity_verification_simple @htmlonly -  Callback returns this type for simple identity verification.</li>
 *                       <li>@endhtmlonly @ref connector_identity_verification_password @htmlonly - Callback returns this type for password identity verification.</li></ul></dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned identity verification form</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get identity verification form and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 *    static connector_callback_status_t app_get_identity_verification(connector_config_identity_verification_t * const identity)
 *    {
 *        identity->type = connector_identity_verification_simple;
 *
 *        return connector_callback_continue;
 *    }
 *
 * @endcode
 *
 * @section password Password
 *
 * Return the password for password identity verification form.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_password() in config.c.
 *
 * @note This callback will not be called for simple identity verification form. See @ref identity_verification
 *
 * @see app_get_password()
 *
 * @htmlonly
 * <table class="apitable">
 *   <tr>
 *     <th colspan="2" class="title">Arguments</th>
 *   </tr>
 *   <tr>
 *     <th class="subtitle">Name</th> <th class="subtitle">Description</th>
 *    </tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_password @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_data_t @htmlonly:
 *          <dl>
 *              <dt><i>data</i></dt>
 *              <dd> - Callback returns the pointer to the password.</dd>
 *              <dt><i>length</i></dt><dd> - Callback writes the length of the password in bytes.</dd>
 *            </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned the password</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get the password and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static connector_callback_status_t app_get_password(connector_config_pointer_string_t * const config_password)
 * {
 *     static  char const connector_password[] = "password";
 *
 *      // Return pointer to password.
 *     config_password->string = (char *)connector_password;
 *     config_password->length = sizeof connector_password -1;
 *
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section get_device_cloud_phone Device Cloud Phone Number (Get)
 *
 * Returns Device Cloud Phone Number where to send SMSs (Only used for SMS transport).
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_device_cloud_phone() in config.c.
 *
 * @note If @ref CONNECTOR_CLOUD_PHONE configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @see app_get_device_cloud_phone()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_get_device_cloud_phone @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_string_t @htmlonly:
 *          <dl>
 *              <dt><i>string</i></dt>
 *              <dd> - Callback returns the pointer to the Device Cloud Phone Number where to send SMSs (Only used for SMS transport).</dd>
 *              <dt><i>length</i></dt><dd> - Callback returns number of bytes of Device Cloud Phone Number. Maximum is 64 bytes.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned Device Cloud Phone Number</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get Device Cloud Phone Number and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static char connector_cloud_phone[] = "447786201216";
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_get_device_cloud_phone)
 *     {
 *         // Return pointer to Device Cloud Phone Number.
 *         connector_config_pointer_string_t * const config_phone = data;
 *
 *         config_phone->string = (char *)connector_cloud_phone;
 *         config_phone->length = sizeof connector_cloud_phone -1;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section set_device_cloud_phone Device Cloud Phone Number (Set)
 *
 * This config callback is used to set (not to get) the Device Cloud Phone Number where to send SMSs (Only used for SMS transport).
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_set_device_cloud_phone() in config.c.
 *
 * @note If @ref CONNECTOR_CLOUD_PHONE configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @see app_set_device_cloud_phone()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_set_device_cloud_phone @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_string_t @htmlonly:
 *          <dl>
 *              <dt><i>string</i></dt>
 *              <dd> - Pointer passed to the Callback containing the Device Cloud Phone Number where to send SMSs in order to 
 *              </dd>  be stored in persistent storage. (Only used for SMS transport)
 *              <dt><i>length</i></dt><dd> - Number of bytes of Device Cloud Phone Number passed. Maximum is 64 bytes.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully set Device Cloud Phone Number</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to set Device Cloud Phone Number and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * static char connector_cloud_phone[16] = "";	// empty: will require a provisioning message from the server for initialization
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_set_device_cloud_phone)
 *     {
 *         if (config_phone->length > (sizeof connector_cloud_phone -1))
 *         {
 *             return connector_callback_error;
 *         }
 *
 *         strcpy(connector_cloud_phone, config_phone->string);
 *
 *         // Maybe user want to save connector_cloud_phone to persistent storage
 *
 *         return connector_callback_continue;
 *     }
 * }
 *
 * @endcode
 *
 *
 * @section device_cloud_service_id Device Cloud Phone service-id
 *
 * Return Device Cloud service-id (if required) used during SMS transport.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_get_device_cloud_service_id() in config.c.
 *
 * @note If @ref CONNECTOR_CLOUD_SERVICE_ID configuration is defined in @ref connector_config.h, this callback
 * will not be called. See @ref connector_config_data_options
 *
 * @see app_get_device_cloud_service_id()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_config @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_config_device_cloud_service_id @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> Pointer to @endhtmlonly connector_config_pointer_string_t @htmlonly:
 *          <dl>
 *              <dt><i>string</i></dt>
 *              <dd> - Callback returns the pointer to Device Cloud Phone Number service-id (if required) used during SMS transport.</dd>
 *              <dt><i>length</i></dt><dd> - Callback returns number of bytes of Device Cloud service-id. Maximum is 64 bytes.</dd>
 *          </dl>
 * </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 * <td>Callback successfully returned Device Cloud service-id</td>
 * </tr>
 * <tr>
 * <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 * <td>Callback was unable to get Device Cloud service-id and callback aborted Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id,
 *                                                    connector_request_id_t const request_id
 *                                                    void * const data)
 * {
 *
 *     if (class_id == connector_class_id_config && request_id.config_request == connector_request_id_config_device_cloud_service_id)
 *     {
 *         // Return pointer to Device Cloud service_id.
 *         static char connector_cloud_service_id[] = "";	// empty: No shared code
 *         connector_config_pointer_string_t * const config_service_id = data;
 *
 *         config_service_id->string = (char *)connector_cloud_service_id;
 *         config_service_id->length = sizeof connector_cloud_service_id -1;
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @htmlinclude terminate.html
 */
