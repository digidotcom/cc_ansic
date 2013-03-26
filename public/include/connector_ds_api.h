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
  *  @brief Functions and prototypes for Connector Data Service related API
  *         public API
  *
  */
#ifndef _CONNECTOR_DS_API_H
#define _CONNECTOR_DS_API_H

/**
* @defgroup connector_request_id_data_service_t Data Service Request IDs
* @{
*/
/**
 * Data service request ID, passed to the application callback
 * to request the data, to pass the response, and to pass the
 * error.
 */
typedef enum {
    connector_request_id_data_service_send_length,      /**< to get the total length of the send data. Not applicable in TCP transport method */
    connector_request_id_data_service_send_data,        /**< to get the data to send to the Cloud */
    connector_request_id_data_service_send_status,      /**< to inform the session status */
    connector_request_id_data_service_send_response,    /**< to inform the Cloud response */

    connector_request_id_data_service_recieve_target,   /**< to inform the start of the Cloud request for a given target */
    connector_request_id_data_service_recieve_data,     /**< to pass the received request data from the Cloud */
    connector_request_id_data_service_recieve_status,   /**< to inform the session status */
    connector_request_id_data_service_recieve_reply_length, /**< to get the total length of the response data. Not applicable in TCP transport method */
    connector_request_id_data_service_recieve_reply_data    /**< to get the response data */
    /* connector_data_service_dp_response     /**< Used in a callback when the iDigi Connector receives a response to data point request */
} connector_request_id_data_service_t;
/**
* @}
*/

/**
* @defgroup connector_session_error_t Connector session error codes
* @{
*/
/**
 * Error values returned from lower communication layer. It can be either from Cloud or from
 * Connector. These are errors originated from a layer where compression/decompression, resource
 * allocation and state handling takes place.
 */
typedef enum
{
    connector_session_error_none,                   /**< Success */
    connector_session_error_fatal,                  /**< Generally represents internal, unexpected error */
    connector_session_error_invalid_opcode,         /**< Opcode used in the message is invalid/unsupported */
    connector_session_error_format,                 /**< Packet is framed incorrectly */
    connector_session_error_session_in_use,         /**< Session with same ID is already in use */
    connector_session_error_unknown_session,        /**< Session is not opened or already closed */
    connector_session_error_compression_failure,    /**< Failed during compression of the data to send */
    connector_session_error_decompression_failure,  /**< Failed during decompression of the received data */
    connector_session_error_memory,                 /**< Malloc failed, try to restrict the number of active sessions */
    connector_session_error_send,                   /**< Send socket error */
    connector_session_error_cancel,                 /**< Used to force termination of a session */
    connector_session_error_busy,                   /**< Either the cloud or the connector is busy processing */
    connector_session_error_ack,                    /**< Invalid ack count */
    connector_session_error_timeout,                /**< Session timed out */
    connector_session_error_no_service,             /**< Requested service is not supported */
    connector_session_error_count                   /**< Maximum error count value, new value goes before this element */
} connector_session_error_t;
/**
* @}
*/

typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< IN/OUT: context passed in connector_initiate_action */

    size_t total_bytes;                 /**< OUT: total bytes in to send */
} connector_data_service_send_length_t;

typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< IN/OUT: user context passed */

    uint8_t * buffer;                   /**< OUT: to be filled with data to send */
    size_t bytes_available;             /**< IN: available bytes in buffer */
    size_t bytes_used;                  /**< OUT: bytes filled */
    connector_bool_t more_data;         /**< OUT: more data to send */
} connector_data_service_send_data_t;

typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< IN/OUT: user context passed */

    enum
    {
        connector_data_service_send_status_complete,      /**< response is not requested and session is complete successfully */
        connector_data_service_send_status_cancel,        /**< session is cancelled by the user */
        connector_data_service_send_status_timeout,       /**< session timed out */
        connector_data_service_send_status_session_error  /**< error from lower communication layer  */
    } status;       /**< IN: reason for end of session */

    connector_session_error_t session_error;              /**< IN: reason for lower communication layer error */

} connector_data_service_send_status_t;

typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< IN/OUT: user context passed */

    enum
    {
        connector_data_service_send_response_success,       /**< send data success */
        connector_data_service_send_response_bad_request,   /**< at least some portion of the request is not valid */
        connector_data_service_send_response_unavailable,   /**< service not available, may retry later */
        connector_data_service_send_response_cloud_error    /**< cloud encountered error while handling the request */
    } response;     /**< IN: the Cloud response code for send data */

    char const * hint; /** IN: error hint returned from the cloud, NULL if success or hint not available */
} connector_data_service_send_response_t;

#define CONNECTOR_DATA_SERVICE_SEND_FLAG_OVERWRITE  0x00
#define CONNECTOR_DATA_SERVICE_SEND_FLAG_ARCHIVE    0x01
#define CONNECTOR_DATA_SERVICE_SEND_FLAG_APPEND     0x02
#define CONNECTOR_DATA_SERVICE_SEND_FLAG_TRANSIENT  0x04

typedef struct
{
    connector_transport_t transport;    /**< transport method to use to send the data */
    void * user_context;                /**< user context, this will be returned in subsequent callbacks */

    char const * path;                  /**< destination path */
    char const * content_type;          /**< null-terminated content type (text/plain, text/xml, application/json, etc. */
    unsigned int flags;                 /**< archive, append or transient */
    connector_bool_t response_required; /**< set to connector_true if response is needed */
} connector_data_service_send_request_t;

/*** Device request callbacks, API should return error on any error ***/
typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< OUT: user context */

    char const * target;                /**< IN: target of the data received */
    connector_bool_t response_required; /**< IN: connector_true means the cloud needs the response and will get the callbacks to collect the response */
} connector_data_service_recieve_target_t;

typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< IN/OUT: user context passed */

    uint8_t const * buffer;             /**< IN: received data */
    size_t bytes_used;                  /**< IN: bytes filled in the buffer */
    connector_bool_t more_data;         /**< IN: connector_true means more request to follow */
} connector_data_service_recieve_data_t;

typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< IN/OUT: user context passed */

    enum
    {
        connector_data_service_recieve_status_cancel,
        connector_data_service_recieve_status_timeout,
        connector_data_service_recieve_status_no_resource,
        connector_data_service_recieve_status_session_error
    } status;                           /**< IN: reason for the end of session */

    connector_session_error_t   session_error;   /**< IN: applicable only if the status is set to connector_data_service_recieve_status_session_error */

} connector_data_service_recieve_status_t;

typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< IN/OUT: user context passed */

    size_t total_bytes;                 /**< OUT: total response data length */
} connector_data_service_recieve_reply_length_t;

typedef struct
{
    connector_transport_t transport;    /**< IN: transport method from where the callback is originated */
    void * user_context;                /**< IN/OUT: user context passed */

    uint8_t * buffer;                   /**< OUT: to be filled with the response data */
    size_t bytes_available;             /**< IN: number of bytes available */
    size_t bytes_used;                  /**< OUT: number of bytes filled */
    connector_bool_t more_data;         /**< OUT: connector_true means more response to fill */
} connector_data_service_recieve_reply_data_t;

#endif /* _CONNECTOR_DS_API_H */

