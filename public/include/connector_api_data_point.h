/*
 * Copyright (c) 2013 Digi International Inc.,
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
  *  @brief Functions and prototypes for Connector Data Point related API
  *         public API
  *
  */
#ifndef _CONNECTOR_API_DATA_POINT_H
#define _CONNECTOR_API_DATA_POINT_H

/**
* @defgroup connector_request_id_data_point_t Data Point Request IDs
* @{
*/
/**
 * Data Point request ID, passed to the application callback to request the data points,
 * to return Etherios  response, and to pass the status/error.
 */
typedef enum
{
    connector_request_id_data_point_binary_response,
    connector_request_id_data_point_binary_status,
    connector_request_id_data_point_single_response,
    connector_request_id_data_point_single_status
} connector_request_id_data_point_t;
/**
* @}
*/

/**
* @defgroup connector_data_point_type_t data types for Data Points
* @{
*/
/**
* This lists the types supported for Data Points for Devices.
*
* @see connector_data_point_request_t
*/
typedef enum
{
    connector_data_point_type_integer,   /**< data can be represented with a network (big endian) 32-bit two's complement integer */
    connector_data_point_type_long,      /**< data can be represented with a network (big endian) 64-bit two's complement integer */
    connector_data_point_type_float,     /**< data can be represented with a network (big endian) 32-bit IEEE754 floating point */
    connector_data_point_type_double,    /**< data can be represented with a network (big endian) 64-bit IEEE754 floating point */
    connector_data_point_type_string,    /**< UTF-8 encoding (ASCII compatible) */
    connector_data_point_type_binary     /**< binary data */
} connector_data_point_type_t;
/**
* @}
*/

/**
* @defgroup connector_timeval_t  Structure to represent time in seconds and milliseconds.
* @{
*/
/**
* The data structure to represent seconds and milliseconds since the epoch (00:00:00 UTC on 1 January 1970).
* Devices on which the milliseconds are not available can use 0 for milliseconds.
*
* @see connector_data_point_t
*/
typedef struct connector_timeval_t
{
    uint32_t seconds;
    unsigned int milliseconds;
} connector_timeval_t;
/**
* @}
*/

/**
* @defgroup connector_data_point_t Data structure used to represent each Data Point.
* @{
*/
/**
* User can use this structure to make linked list of Data Points to send it out in one transaction.
* Note: All data points must belong to same stream.
*
* @see connector_request_data_point_single_t
* @see connector_data_point_type_t
*/
typedef struct connector_data_point_t
{
    struct
    {
        enum
        {
            connector_data_type_native, /**< the data value is in native format */
            connector_data_type_text    /**< the data value is in ascii/text format */
        } type;

        union
        {
            union
            {
                int int_value;       /**< 32-bit two's complement integer */
                long long_value;     /**< 64-bit two's complement integer */
                char * string_value; /**< a null-terminated utf-8 encoding string */
                #if (defined FLOATING_POINT_SUPPORTED)
                float float_value;   /**< 32-bit IEEE754 floating point */
                double double_value; /**< 64-bit IEEE754 floating point */
                #endif
            } native;

            char * text;            /**< carries data in ascii format, a null-terminated string */
        } element;

    } data;
 
    struct
    {
        enum
        {
            connector_time_server,          /**< The timev.alue is ignored and the server time is used instead. */
            connector_time_local_epoch,     /**< A transport-specific time.value is specified in epoch long format. */
            connector_time_local_iso8601    /**< A transport-specific time.value is specified in ISO 8601 string format. */
        } source;

        union
        {
            connector_timeval_t msec_since_epoch; /**< Time since the Epoch time in milliseconds */
            char * iso8601_string;      /**< A null-terminated local time in ISO 8601 format */
        } value;

    } time;   /**< Time at the data point is captured */
 
    struct
    {
        enum
        {
            connector_location_type_ignore, /**< location is ignored */
            #if (defined FLOATING_POINT_SUPPORTED)
            connector_location_type_native,  /**< location value is represented in float */
            #endif
            connector_location_type_text   /**< location value is represented in ascii */
        } type;

        union
        {
            #if (defined FLOATING_POINT_SUPPORTED)
            struct
            {
                float latitude;     /**< latitude in degree */
                float longitude;    /**< longitude in degree */
                float elevation;    /**< elevation in meters */
            } native;
            #endif

            struct
            {
                char * latitude;   /**< latitude in degree (null-terminated string) */
                char * longitude;  /**< longitude in degree (null-terminated string) */
                char * elevation;  /**< elevation in meters (null-terminated string) */
            } text;

        } value;

    } location;

    struct
    {
        enum
        {
            connector_quality_type_ignore, /**< Quality is ignored */
            connector_quality_type_native  /**< user specified data quality, an integer value */
        } type;

        int value;
    } quality;

    char * description; /**< null terminated description string (optional field, set to NULL if not used) */

    struct connector_data_point_t * next; /**< Points to next data point, set to NULL if this is the last one. */
} connector_data_point_t;
/**
* @}
*/

/**
* @defgroup connector_request_data_point_binary_t Data type used to send binary data point of a stream to the Cloud.
* @{
*/
/**
* This data structure is used when the connector_initiate_action() API is called with connector_initiate_binary_point
* request id. This can be used on a device where foot print and or the bandwidth is limited.
*
* @see connector_message_status_response_t
* @see connector_initiate_action
* @see connector_initiate_data_point_binary
*/
typedef struct
{
    connector_transport_t transport;/**< transport method to use */
    void const * user_context;      /**< user context to be passed back in response */

    char const * path;              /**< data point path in the stream */
    void * point;                   /**< binary data of size specified in bytes */
    size_t bytes;                   /**< size of data in bytes */
    connector_bool_t response_required;  /**< set to connector_true if response is needed */
} connector_request_data_point_binary_t;
/**
* @}
*/

/**
* @defgroup connector_request_data_point_single_t Data type used to send Data Points of single stream on Etherios Device Cloud.
* @{
*/
/**
* This data structure is used when the connector_initiate_action() API is called with
* connector_initiate_data_point_single request id.
*
* @see connector_data_point_t
* @see connector_initiate_action
* @see connector_initiate_data_point_single
* @see connector_data_point_type_t
*/
typedef struct
{
    connector_transport_t transport;    /**< transport method to use */
    void const * user_context;          /**< user context to be passed back in response */

    connector_data_point_type_t type;   /**< data point content type */
    char * unit;                        /**< null-terminated unit, optional field, set to NULL if not used */
    char * forward_to;                  /**< comma separated list of streams to replicate data points to (a null-terminated optional field, set to NULL if not used) */
    connector_data_point_t * point;     /**< pointer to list of data points */
} connector_request_data_point_single_t;
/**
* @}
*/

/**
* @defgroup connector_data_point_response_t Carries Etherios Device Cloud response.
* @{
*/
/**
* The data in the callback with request id connector_request_id_data_point_binary_response and
* connector_request_id_data_point_single_response will point to this data structure. The callback
* is called when the Connector receives a response from Etherios Cloud Connector.
*
* @see connector_request_id_data_point_t
* @see connector_initiate_data_point_binary
* @see connector_initiate_data_point_single
*/
typedef struct
{
    connector_transport_t transport;    /**< transport method to use */
    void const * user_context;          /**< user context to be passed back in response */

    enum
    {
        connector_data_point_response_success,
        connector_data_point_response_bad_request,
        connector_data_point_response_unavailable,
        connector_data_point_response_cloud_error
    } response;

} connector_data_point_response_t;

/**
* @defgroup connector_data_point_response_t Carries data point session status.
* @{
*/
/**
* The data in the callback with request id connector_request_id_data_point_binary_status and
* connector_request_id_data_point_single_status will point to this data structure. The callback
* is called when session completes either due to error or the response is not requested.
*
* @see connector_request_id_data_point_t
* @see connector_initiate_data_point_binary
* @see connector_initiate_data_point_single
*/
typedef struct
{
    connector_transport_t transport;    /**< transport method to use */
    void const * user_context;          /**< user context to be passed back in response */

    enum
    {
        connector_data_point_status_complete,      /**< response is not requested and session is complete successfully */
        connector_data_point_status_cancel,        /**< session is cancelled by the user */
        connector_data_point_status_timeout,       /**< session timed out */
        connector_data_point_status_session_error  /**< error from lower communication layer  */
    } status;       /**< IN: reason for end of session */

    connector_session_error_t session_error;      /**< IN: reason for lower communication layer error */

} connector_data_point_status_t;

#endif /* _CONNECTOR_API_DATA_POINT_H */

