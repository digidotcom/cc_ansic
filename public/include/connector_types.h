/*
 * Copyright (c) 2011, 2012, 2013 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

/**
 * @file
 *  @brief Types required by Etherios Cloud Connector
 *
 */

#ifndef CONNECTOR_TYPES_H_
#define CONNECTOR_TYPES_H_

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stddef.h>

/**
 * The Etherios Cloud Connector uses this function to produce output string according to the format.
 * This function behaves exactly as snprintf except that the variable argument list is passed
 * as a va_list instead of a succession of arguments.
 *
 * In the C library the prototype for vsnprintf is defined as vsnprintf(char * str, size_t size, const char *format, va_list ap);
 *
 */
int connector_snprintf(char * const str, size_t const size, char const * const format, ...);

#if __STDC_VERSION__ >= 199901L
  /* If the compiler is C99 complaint, then we have stdint.h. */
  #define CONNECTOR_HAVE_STDINT_HEADER
#endif

#if (defined __MWERKS__) && (defined __MQX__)
  #if (!defined CONNECTOR_HAVE_STDINT_HEADER)
    #define CONNECTOR_HAVE_STDINT_HEADER
  #endif
#else
  #include <stdio.h>
#endif

#if defined CONNECTOR_HAVE_STDINT_HEADER
  #include <stdint.h>
  #include <inttypes.h>

#else
/**
* @defgroup user_types User Defined C types
* If your compiler is C89 complaint these defines are used: you will need
* to define them appropriately for your system.  If your compiler is C99 complaint
* then the types from stdint.h and inttypes.h are used.
* @{
*/
/**
 *  Unsigned 8 bit value.
 */
  typedef unsigned char uint8_t;

/**
 *  Unsigned 16 bit value.
 */
  typedef unsigned short uint16_t;

#ifndef UINT16_MAX
/**
*  Unsigned 16 bit maximum value.
*/
#define UINT16_MAX  0xFFFF
#endif

/**
 *  Unsigned 32 bit value.
 */
  typedef unsigned long int uint32_t;

#ifndef UINT32_MAX
/**
*  Unsigned 32 bit maximum value.
*/
#define UINT32_MAX 4294967295UL
#endif

#ifndef SCNu32
/**
*  Scan format specifier for unsigned 32 bit value.
*/
#define SCNu32 "lu"
#endif

#ifndef PRIu32
/**
*  Print format specifier for unsigned 32 bit value.
*/
#define PRIu32 "lu"
#endif

/**
*  Signed 32 bit value.
*/
/*    typedef long int int32_t; */

#ifndef INT32_MIN
/**
*  Signed 32 bit minimum value.
*/
#define INT32_MIN -2147483648L
#endif

#ifndef INT32_MAX
/**
*  Signed 32 bit maximum value.
*/
#define INT32_MAX 2147483647L
#endif

#ifndef SCNd32
/**
*  Scan format specifier for signed 32 bit value.
*/
#define SCNd32 "ld"
#endif

#ifndef PRId32
/**
*  Print format specifier for signed 32 bit value.
*/
#define PRId32 "ld"
#endif

#ifndef SCNx32
/**
*  Scan format specifier for 32 bit hex value.
*/
#define SCNx32 "lx"
#endif

#ifndef PRIx32
/**
*  Print format specifier for 32 bit hex value.
*/
#define PRIx32 "lx"
#endif

#if (defined CONNECTOR_HAS_64_BIT_INTEGERS)

/**
 *  Unsigned 64 bit value.
 */
  typedef unsigned long long int uint64_t;

#ifndef UINT64_MAX
/**
*  Unsigned 64 bit maximum value.
*/
#define UINT64_MAX 18446744073709551615ULL
#endif

#ifndef SCNu64
/**
*  Scan format specifier for unsigned 64 bit value.
*/
#define SCNu64 "llu"
#endif

#ifndef PRIu64
/**
*  Print format specifier for unsigned 64 bit value.
*/
#define PRIu64 "llu"
#endif

/**
*  Signed 32 bit value.
*/
   typedef long long int int64_t;

#ifndef INT64_MIN
/**
*  Signed 64 bit minimum value.
*/
#define INT64_MIN -9223372036854775808LL
#endif

#ifndef INT64_MAX
/**
*  Signed 64 bit maximum value.
*/
#define INT64_MAX 9223372036854775807LL
#endif

#ifndef SCNd64
/**
*  Scan format specifier for signed 64 bit value.
*/
#define SCNd64 "lld"
#endif

#ifndef PRId64
/**
*  Print format specifier for signed 64 bit value.
*/
#define PRId64 "lld"
#endif

#ifndef SCNx64
/**
*  Scan format specifier for 64 bit hex value.
*/
#define SCNx64 "llx"
#endif

#ifndef PRIx64
/**
*  Print format specifier for 64 bit hex value.
*/
#define PRIx64 "llx"
#endif

#endif

#ifndef SIZE_MAX
/**
*  size_t maximum value.
*/
#define SIZE_MAX  UINT32_MAX
#endif

/**
* @}
*/


#endif

#if __STDC_VERSION__ >= 199901L
#define PRIsize "zu"
#else
#define PRIsize "u"
#endif

/**
* @defgroup connector_network_handle_t Network Handle
* @{
*/
/**
 * Used to reference a network session and is passed into Etherios Cloud Connector API calls. This
 * type is defined as void * and is used by the application to 
 * keep track of and identify sessions.
 */
typedef void * connector_network_handle_t;
/**
* @}
*/


#endif /* CONNECTOR_TYPES_H_ */
