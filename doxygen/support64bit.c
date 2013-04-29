/*! @page support64bit Support for 64-bit targets
 *
 * @htmlinclude nav.html
 *
 * @section sixtyfourbit 64-bit data types?
 *
 * The connector has two 64-bit data types that are implemented using two 32-bit values.  These would
 * be handled more efficiently with a 64-bit data type.
 *  *
 * Please be aware of the @ref i_have_C89_and_64bit "restrictions" on ANSI C89 and 64-bit support.
 *
 * @subsection data_type_format_etc Updates to connector_types.h
 *
 * When 64-bit support is included, you will have to review (and edit) @ref connector_types.h
 * "public/include/connector_types.h" to update the standard ANSI data types, minimums, maximum,
 * and standard format specifiers.
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
 * @note These are automatically defined for ANSI C99.
 *
 * @section data_point_time_since_epoch 64-bit Data Point Epoch time
 *
 * The @ref data_point requires time in milliseconds since the epoch (00:00:00 UTC on 1 January 1970).  This
 * number requires more than 32-bits.  On a 64-bit target, it can make use of the @ref connector_time_epoch_whole_t
 * data structure.  This data structure includes a long long unsigned integer millisecond member to hold this value.
 *
 * 32-bit targets would instead use @ref connector_time_epoch_fractional_t, which includes two 32-bit unsigned
 * values.
 *
 * To enable the 64-bit target support, you need to define CONNECTOR_HAS_64_BIT_INTEGERS.
 *
 * @section sixtyfourfilesystemls File System Support for large Files
 *
 * when the @ref file_system encounters very large files, special handling is required.  The following
 * limitations apply:
 *
 * -# The size of data in one get_file request from Device Cloud is limited to 2MB - 1byte (2097151 bytes).
 *    <br /><br />
 * -# Offset for setting file position and truncating a file is limited to 2 gigabytes.
 *    <br /><br />
 * -# File sizes sent back to Device Cloud in listing requests are limited to 2 gigabytes by default.
 *    To enable support of file sizes larger than 2 gigabytes in file or directory listings:
 *      <br /><br />
 *      -# Define @ref CONNECTOR_FILE_SYSTEM_HAS_LARGE_FILES in connector_config.h
 *      <br /><br />
 *      -# On linux platform add: <b>CFLAGS += -D_FILE_OFFSET_BITS=64</b> to Makefile.
 *
 * @htmlinclude terminate.html
 */
