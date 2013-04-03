/*! @page os_callbacks Operating System Callbacks
 *
 * @htmlinclude nav.html
 *
 * @section os_callbacks OS Callbacks
 * The iDigi connector interfaces to the platform's OS through the callbacks listed below.  These
 * are called through the application callback described in the @ref api1_overview.
 * The class id is @ref connector_class_operating_system.
 *
 *  -# @ref malloc
 *  -# @ref free
 *  -# @ref uptime
 *  -# @ref yield
 *  -# @ref reboot
 *
 * @section malloc malloc
 * Callback is used to dynamically allocate memory.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_os_malloc() in os.c.
 *
 * @see app_os_malloc()
 * @see app_os_free()
 * @see @ref connector_os_free
 *
 * @htmlonly
 * <table class="apitable">
 * <tr>
 *   <th colspan="2" class="title">Arguments</th>
 *   </tr>
 *   <tr>
 *     <th class="subtitle">Name</th>
 *     <th class="subtitle">Description</th>
 *   </tr>
 *   <tr>
 *     <td>class_id</td>
 *     <td>@endhtmlonly @ref connector_class_operating_system @htmlonly</td>
 *   </tr>
 *   <tr>
 *     <td>request_id</td>
 *     <td>@endhtmlonly @ref connector_request_id_os_malloc @htmlonly</td>
 *   </tr>
 *   <tr>
 *     <td>request_data</td>
 *     <td>Pointer to number of bytes to be allocated </td>
 *   </tr>
 *   <tr>
 *     <td>request_length</td>
 *     <td> The size of *request_data which is sizeof size_t.</td>
 *   </tr>
 *   <tr>
 *     <td>response_data</td>
 *     <td> Returns a pointer to memory for allocated address </td>
 *   </tr>
 *   <tr>
 *     <td>response_length</td>
 *     <td>N/A</td>
 *   </tr>
 *   <tr>
 *     <th colspan="2" class="title">Return Values</th>
 *   </tr>
 *   <tr>
 *     <th class="subtitle">Values</th>
 *     <th class="subtitle">Description</th>
 *   </tr>
 *   <tr>
 *     <td>@endhtmlonly @ref connector_callback_continue @htmlonly</td>
 *     <td>Callback successfully allocated memory</td>
 *   </tr>
 *   <tr>
 *     <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 *     <td>Callback was unable to allocate memory and callback aborts iDigi connector</td>
 *   </tr>
 *   <tr>
 *     <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *     <td>Memory is not available at this time and needs to be called back again</td>
 *   </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id
 *                              void const * const request_data, size_t const request_length,
 *                              void * response_data, size_t * const response_length)
 * {
 *
 *     if (class_id == connector_class_operating_system && request_id.os_request == connector_request_id_os_malloc)
 *     {
 *         size_t * const size = request_data;
 *         void ** ptr = (void **)response_data;
 *
 *         *ptr = malloc(*size);
 *         if (*ptr == NULL)
 *         {
 *             return connector_callback_abort;
 *         }
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section free free
 *
 * Callback is called to free previously allocated memory.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_os_free() in os.c.
 *
 * @see app_os_free()
 * @see app_os_malloc()
 * @see @ref connector_request_id_os_malloc
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_operating_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_os_free @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td>Pointer address to be freed </td>
 * </tr>
 * <tr>
 * <th>request_length</th>
 * <td> N/A </td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> N/A </td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td>N/A</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>None</th>
 * <td>None</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id
 *                              void const * const request_data, size_t const request_length,
 *                              void * response_data, size_t * const response_length)
 * {
 *
 *     if (class_id == connector_class_operating_system && request_id.os_request == connector_os_free)
 *     {
 *         free((void *)request_data);
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section uptime System Uptime
 * This callback is called to return system up time in seconds. It is the time
 * that a device has been up and running.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_os_get_system_time() in os.c.
 *
 * @see app_os_get_system_time()
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_operating_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_os_system_up_time @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td>N/A </td>
 * </tr>
 * <tr>
 * <th>request_length</th>
 * <td> N/A</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> Pointer to unsigned long integer memory where callback writes the system up time to (in seconds) </td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td> N/A</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 * <td>Callback successfully returned the system time</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 * <td>Error occurred and callback aborted iDigi connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_connector_callback(connector_class_id_t const class_id, connector_request_id_t const request_id
 *                              void const * const request_data, size_t const request_length,
 *                              void * response_data, size_t * const response_length)
 * {
 *
 *     if (class_id == connector_class_operating_system && request_id.os_request == connector_request_id_os_system_up_time)
 *     {
 *         time((time_t *)response_data);
 *     }
 *     return connector_callback_continue;
 * }
 *
 * @endcode
 *
 * @section yield Yield
 * This callback is called to relinquish control in the @ref threading "multi-threaded" connector_run() model.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_os_yield() in os.c.
 *
 * @see app_os_yield()
 * @see @ref threading "Multi-threading and the iDigi connector Threading Model"
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_operating_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_os_yield @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td>Status of iDigi connector <ul> <li> @endhtmlonly @ref connector_idle @htmlonly </li>
 *                                    <li> @endhtmlonly @ref connector_working @htmlonly </li>
 *                                    <li> @endhtmlonly @ref connector_pending @htmlonly </li>
 *                                    <li> @endhtmlonly @ref connector_active @htmlonly </li> </ul></td>
 * </tr>
 * <tr>
 * <th>request_length</th>
 * <td>Size of @endhtmlonly @ref connector_status_t @htmlonly</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> N/A</td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td> N/A</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>None</th>
 * <td>None</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 *  connector_callback_status_t app_os_yield(connector_status_t const * const status)
 *   {
 *       if (*status == connector_idle)
 *       {
 *           unsigned int const timeout_in_microseconds =  1000000;
 *           usleep(timeout_in_microseconds);
 *       }
 *
 *       return connector_callback_continue;
 *   }
 *
 * @endcode
 *
 * @section reboot Reboot
 *
 * Callback is called when the iDigi Device Cloud requests that the device reboots.
 * The device should reboot and this callback should not return.
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_operating_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_os_reboot @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_data</th>
 * <td> N/A </td>
 * </tr>
 * <tr>
 * <th>request_length</th>
 * <td> N/A.</td>
 * </tr>
 * <tr>
 * <th>response_data</th>
 * <td> N/A </td>
 * </tr>
 * <tr>
 * <th>response_length</th>
 * <td>N/A</td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>No return</td>
 * <td>none</td>
 * </tr>
 * </table>
 * @endhtmlonly
 *
 * Example:
 *
 * @code
 *
 *   static connector_callback_status_t app_os_reboot(void)
 *   {
 *
 *       APP_DEBUG("Reboot from server\n");
 *       return connector_callback_continue;
 *   }
 *
 * @endcode
 *
 * @htmlinclude terminate.html
 */
