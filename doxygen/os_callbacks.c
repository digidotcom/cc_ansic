/*! @page os_callbacks Operating System Callbacks
 *
 * @htmlinclude nav.html
 *
 * @section os_callbacks OS Callbacks
 * The Etherios Cloud Connector interfaces to the platform's OS through the callbacks listed below.  These
 * are called through the application callback described in the @ref api1_overview.
 * The class id is @ref connector_class_id_operating_system.
 *
 *  -# @ref malloc
 *  -# @ref free
 *  -# @ref uptime
 *  -# @ref yield
 *  -# @ref reboot
 * <br /><br /> 
 *
 * @section malloc malloc
 * Callback is used to dynamically allocate memory.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_os_malloc() in os.c.
 *
 * @see app_os_malloc()
 * @see app_os_free()
 * @see @ref connector_request_id_os_free
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
 *     <th>class_id</th>
 *     <td>@endhtmlonly @ref connector_class_id_operating_system @htmlonly</td>
 *   </tr>
 *   <tr>
 *     <th>request_id</th>
 *     <td>@endhtmlonly @ref connector_request_id_os_malloc @htmlonly</td>
 *   </tr>
 *   <tr>
 *     <th>data</th>
 *     <td>Pointer to data structure of @endhtmlonly @ref connector_os_malloc_t "connector_os_malloc_t" @htmlonly type. Data fields:
 *          <ul>
 *          <li><b><i>size</i></b> - [IN] Number of bytes to allocate</li>
 *          <li><b><i>ptr</i></b> - [OUT] Returned pointer to the beginning of the allocated memory block</li>
 *          </ul>
 *      </td> 
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
 *     <td>@endhtmlonly @ref connector_callback_busy @htmlonly</td>
 *     <td>Memory is not available at this time and needs to be called back again</td>
 *   </tr>
 *   <tr>
 *     <td>@endhtmlonly @ref connector_callback_abort @htmlonly</td>
 *     <td>Callback aborts Etherios Cloud Connector</td>
 *   </tr>
 * </table>
 * @endhtmlonly
 * <br />
 * 
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_os_handler(connector_request_id_os_t const request,
 *                                            void * const data)
 * {
 *     connector_callback_status_t status = connector_callback_continue;
 * 
 *     if (class_id == connector_class_id_operating_system &&
 *         request_id.os_request == connector_request_id_os_malloc)
 *     {
 *         connector_os_malloc_t * p = data;
 * 
 *         p->ptr = malloc(p->size);
 * 
 *         if (p->ptr == NULL)
 *         {
 *             return connector_callback_abort;
 *         }
 *     }
 *     return status;
 * }
 * @endcode
 * <br /> 
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
 * <td>@endhtmlonly @ref connector_class_id_operating_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_os_free @htmlonly</td>
 * </tr>
 * <tr>
 *     <th>data</th>
 *     <td>Pointer to data structure of @endhtmlonly @ref connector_os_free_t "connector_os_free_t" @htmlonly type. Data fields:
 *        <ul>
 *          <li><b><i>ptr</i></b> - [IN] A pointer to the memory block to free </li>
 *        </ul>
 *      </td> 
 * </tr>
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
 *     <td>Callback aborts Etherios Cloud Connector</td>
 *   </tr>
 * </table>
 * @endhtmlonly
 * <br />
 * 
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_os_handler(connector_request_id_os_t const request,
 *                                            void * const data)
 * {
 *     if (class_id == connector_class_id_operating_system &&
 *         request_id.os_request == connector_request_id_os_free)
 *     {
 *         connector_os_free_t * p = data;
 * 
 *         free(p->ptr);
 *     }
 *     return connector_callback_continue;
 * }
 * @endcode
 * <br />
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
 * <td>@endhtmlonly @ref connector_class_id_operating_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_os_system_up_time @htmlonly</td>
 * </tr>
 *     <th>data</th>
 *     <td>Pointer to data structure of @endhtmlonly @ref connector_os_system_up_time_t "connector_os_system_up_time_t" @htmlonly type. Data fields:
 *        <ul>
 *          <li><b><i>sys_uptime</i></b> - [OUT] Returned system up time in seconds </li>
 *        </ul>
 *      </td> 
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 * <td>Callback successfully returned the system time</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 * <td>Error occurred and callback aborted Etherios Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * <br />
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_os_handler(connector_request_id_os_t const request,
 *                                            void * const data)
 * {
 *
 *     if (class_id == connector_class_id_operating_system &&
 *         request_id.os_request == connector_request_id_os_system_up_time)
 *     {
 *          connector_os_system_up_time_t * p = data;
 *          time_t sys_uptime;
 * 
 *          time(&sys_uptime);
 *          p->sys_uptime = (unsigned long) sys_uptime;
 *     }
 *     return connector_callback_continue;
 * }
 * @endcode
 * <br />
 *
 * @section yield Yield
 * This callback is called to relinquish control in the @ref threading "multi-threaded" connector_run() model.
 *
 * This callback is trapped in application.c, in the @b Sample section of @ref AppStructure "Public Application Framework"
 * and implemented in the @b Platform function app_os_yield() in os.c.
 *
 * @see app_os_yield()
 * @see @ref threading "Multi-threading and Etherios Cloud Connector Threading Model"
 *
 * @htmlonly
 * <table class="apitable">
 * <tr> <th colspan="2" class="title">Arguments</th> </tr>
 * <tr><th class="subtitle">Name</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>class_id</th>
 * <td>@endhtmlonly @ref connector_class_id_operating_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_os_yield @htmlonly</td>
 * </tr>
 * <tr>
 *  <th>data</th>
 *  <td>Pointer to data structure of @endhtmlonly @ref connector_os_yield_t "connector_os_yield_t" @htmlonly type
 *      with one data field <b><i>status</i></b> - status of Etherios Cloud Connector:
 *      <ul> <li> @endhtmlonly @ref connector_idle @htmlonly </li>
 *           <li> @endhtmlonly @ref connector_working @htmlonly </li>
 *           <li> @endhtmlonly @ref connector_pending @htmlonly </li>
 *           <li> @endhtmlonly @ref connector_active @htmlonly </li>
 *      </ul>
 *  </td> 
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_continue @htmlonly</th>
 * <td>Callback successfully returned the system time</td>
 * </tr>
 * <tr>
 * <th>@endhtmlonly @ref connector_callback_abort @htmlonly</th>
 * <td>Error occurred and callback aborted Etherios Cloud Connector</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * <br />
 * 
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_os_handler(connector_request_id_os_t const request,
 *                                            void * const data)
 * {
 *     if (class_id == connector_class_id_operating_system &&
 *         request_id.os_request == connector_request_id_os_yield)
 *     {
 *          connector_os_yield_t * p = data;
 *
 *          if (p->status == connector_idle)
 *          {
 *              unsigned int const timeout_in_microseconds =  1000000;
 *              usleep(timeout_in_microseconds);
 *          }
 *     }
 *     return connector_callback_continue;
 * }
 * @endcode
 * <br /> 
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
 * <td>@endhtmlonly @ref connector_class_id_operating_system @htmlonly</td>
 * </tr>
 * <tr>
 * <th>request_id</th>
 * <td>@endhtmlonly @ref connector_request_id_os_reboot @htmlonly</td>
 * </tr>
 * <tr>
 * <th>data</th>
 * <td> N/A </td>
 * </tr>
 * <tr> <th colspan="2" class="title">Return Values</th> </tr>
 * <tr><th class="subtitle">Values</th> <th class="subtitle">Description</th></tr>
 * <tr>
 * <td>No return</td>
 * <td>none</td>
 * </tr>
 * </table>
 * @endhtmlonly
 * <br />
 *
 * Example:
 *
 * @code
 *
 * connector_callback_status_t app_os_handler(connector_request_id_os_t const request,
 *                                            void * const data)
 * {
 *     if (class_id == connector_class_id_operating_system &&
 *         request_id.os_request == connector_request_id_os_reboot)
 *     {
 *       APP_DEBUG("Reboot from server\n");
 *     }
 *     return connector_callback_continue;
 * } 
 *
 * @endcode
 *
 * @htmlinclude terminate.html
 */
