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

#ifndef CONNECTOR_API_SHORT_MESSAGE_H
#define CONNECTOR_API_SHORT_MESSAGE_H

/**
* @defgroup connector_sm_request_t Short message specific request IDs
* @{
*/
/**
* Short message related callbacks will use these request IDs. The class ID associated with these request ID is
* connector_class_id_short_message and supported transport methods under this class is UDP.
*/
typedef enum
{
    connector_request_id_sm_ping_request,   /**< used when Cloud Connector receives ping request from Etherios Device Cloud */
    connector_request_id_sm_ping_response,  /**< used when Cloud Connector receives ping response from Etherios Device Cloud */
    connector_request_id_sm_cli_request,    /**< used when Cloud Connector receives CLI request from Etherios Device Cloud */
    connector_request_id_sm_cli_response,   /**< used to get the CLI response */
    connector_request_id_sm_cli_status,     /**< called when error occurs in CLI session */
    connector_request_id_sm_more_data,      /**< indicates pending messages are available on Etherios Device Cloud,
                                                 User need to use new request (can be ping) to pull the pending messages from iDigi Device Cloud. */
    connector_request_id_sm_opaque_response /**< Cloud Connector will use this to provide Etherios Device Cloud response for which
                                                 there is no associated request */
} connector_request_id_sm_t;
/**
* @}
*/

/**
* @defgroup connector_sm_ping_request_t Data type used in ping request.
* @{
*/
/**
* This data structure is used when the device initiates the ping request or when the callback is called with the
* connector_request_id_sm_ping_request. In the latter case, a ping request is received from Etherios Device Cloud, and
* user context will not be used. Returning error from the callback will result in error response to Etherios Device Cloud.
*
* @see connector_request_id_sm_ping_request
*/
typedef struct
{
    connector_transport_t transport;
    void * user_context;
    connector_bool_t response_required;
} connector_sm_ping_request_t;
/**
* @}
*/

/**
* @defgroup connector_sm_ping_response_t Data type used in incoming ping response.
* @{
*/
/**
* This data structure is used when the callback is called with the connector_request_id_sm_ping_response.
* A ping response is received from Etherios Device Cloud.
*
* @see connector_request_id_sm_ping_response
*/
typedef struct
{
    connector_transport_t transport;
    void * user_context;

    enum
    {
        connector_sm_ping_status_success,
        connector_sm_ping_status_complete,
        connector_sm_ping_status_cancel,
        connector_sm_ping_status_timeout,
        connector_sm_ping_status_error
    } status;

} connector_sm_ping_response_t;
/**
* @}
*/

/**
* @defgroup connector_sm_cli_request_t Data type used for cli request
* @{
*/
/**
* This data structure is used when the callback is called to pass the CLI request received from
* Etherios Device Cloud.
*
* @see connector_request_id_sm_cli_request
*
*/
typedef struct
{
    connector_transport_t transport;
    void * user_context;        /**< User context, will be NULL when request is called, so that user can update this. */

    char const * buffer;        /**< Buffer pointer to write the CLI response to */
    size_t bytes_used;          /**< It carries the sizeof CLI command */
    connector_bool_t response_required;
} connector_sm_cli_request_t;
/**
* @}
*/

/**
* @defgroup connector_sm_cli_response_t Data type used for CLI response
* @{
*/
/**
* This data structure is used when the callback is called to get the response.
*
* @see connector_request_id_sm_cli_response
*
*/
typedef struct
{
    connector_transport_t transport;
    void * user_context;        /**< the user context passed during CLI request callback */

    char * buffer;              /**< buffer pointer to write the CLI response to */
    size_t bytes_available;     /**< total bytes available in buffer */
    size_t bytes_used;          /**< bytes filled */
    connector_bool_t more_data; /**< */    
} connector_sm_cli_response_t;
/**
* @}
*/

/**
* @defgroup connector_sm_cli_status_t Data type used to pass CLI status.
* @{
*/
/**
* This data structure is used when the callback is called to indicate the
* termination of CLI session.
*
* @see connector_request_id_sm_cli_response
*
*/
typedef struct
{
    connector_transport_t transport;
    void * user_context;        /**< the user context passed during CLI request callback */

    enum
    {
        connector_sm_cli_status_cancel,
        connector_sm_cli_status_error
    } status;

} connector_sm_cli_status_t;
/**
* @}
*/

/**
* @defgroup connector_sm_opaque_response_t Data type used to deliver the opaque response from the Cloud
* @{
*/
/**
* This data structure is used when the callback is called to pass Etherios Device Cloud response for which the
* associated request is not present. It may be cancelled by the user or by Cloud Connector after after
* sending the request.
*
* @see connector_request_id_sm_opaque_response
*
*/
typedef struct
{
    connector_transport_t transport;
    uint32_t id;        /**< Can be used to keep track of a multipart response */
    void * data;        /**< Pointer to opaque response */
    size_t bytes_used;  /**< Number of bytes available in the data */
} connector_sm_opaque_response_t;
/**
* @}
*/

/**
* @defgroup connector_sm_more_data_t Data type used to indicate pending data.
* @{
*/
/**
* This data structure is used when the callback is called with the connector_request_id_more_data.
* This indicates that more messages are pending in Etherios Device Cloud for the device. User need
* to send request (can be ping) to retreive pending messages.
*
* @see connector_request_id_more_data
*/
typedef struct
{
    connector_transport_t transport;
} connector_sm_more_data_t;
/**
* @}
*/

#endif /* CONNECTOR_API_SHORT_MESSAGE_H */
