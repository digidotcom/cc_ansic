/*! @page support64bit Support for 64-bit targets
 *
 * @htmlinclude nav.html
 *
 * @section sixtyfourbit Why do I need support for 64-bit data types?
 *
 * From the top level these callbacks are analogous to the @ref network_callbacks.
 * Internally they provide interface to securely connect to Device Cloud
 * on SSL. Any application can make use of these APIs to obtain secure connection
 * just by including network_tcp_ssl.c instead of network_tcp.c. The sample application
 *
 * @subsection data_type_format_etc Updates to connector_types.h
 *
 * When 64-bit support is included, you will have to review (and edit) @ref connector_types.h
 * "public/include/connector_types.h" to update the standard ANSI data types, minimums, maximum, and standard format specifiers.
 *
 * -# Data types:
 *  @li uint64_t
 *  @li int64_t
 *
 * -# Minimum and maximum values:
 *  @li INT64_MIN
 *  @li INT64_MAX
 *
 * -# Format specifiers:
 *  @li SCNd64
 *  @li SCNu64
 *  @li SCNx64
 *  @li PRId64
 *  @li PRIu64
 *  @li PRIx64
 *
 * @note These are defined for ANSI C99 or typedefed/defined for ANSI C89.
 *
 *
 * @htmlinclude terminate.html
 */
