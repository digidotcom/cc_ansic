/*
 * Copyright (c) 2011, 2012 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

#ifndef _CONNECTOR_API_H
#define _CONNECTOR_API_H

/**
 * Current version of the iDigi connector that application is using or built for.
 *
 * Note current version number denotes:  1.1.0.0
 *                                       | | | |
 *                                       v v v v
 *                                    0x01010000UL
 */
#define CONNECTOR_VERSION   0x01000000UL

#include "connector_config.h"
#include "connector_types.h"


#define asizeof(array)  (sizeof array/sizeof array[0])
/**
 * @defgroup connector_bool_t Connector Boolean Type
 * @{
 */
/**
 * Connector Boolean Type
 *
 */
typedef enum {
    connector_false,    /**< False */
    connector_true      /**< True */
} connector_bool_t;

#include "connector_api_config.h"
#include "connector_api_network.h"

#if (defined CONNECTOR_TRANSPORT_TCP) && (defined CONNECTOR_RCI_SERVICE)
#include "connector_api_remote.h"
#endif

 /**
 * @defgroup connector_port_numbers iDigi Port Numbers
 * @{
 */
/**
 * iDigi server connect port number
 *
 * This port number is for connector_network_open callback to make
 * a connection to iDigi server.
 *
 * @see CONNECTOR_SSL_PORT
 * @see CONNECTOR_UDP_PORT
 * @see connector_network_open
 */
#define CONNECTOR_PORT       3197   /**< Port number used to connect to iDigi server. */

/**
 * iDigi server secured connect port number
 *
 * This port number is for connector_network_open callback to make
 * a secured connection to iDigi server.
 *
 * @see CONNECTOR_PORT
 * @see CONNECTOR_UDP_PORT
 * @see connector_network_open
 */
#define CONNECTOR_SSL_PORT   3199   /**< Secured port number used to connect to iDigi server. */

/**
 * iDigi server UDP port number
 *
 * This port number is for connector_network_open callback to communicate with
 * the iDigi server over UDP.
 *
 * @see CONNECTOR_PORT
 * @see CONNECTOR_SSL_PORT
 * @see connector_network_open
 */
#define CONNECTOR_UDP_PORT       3297   /**< UDP port number used to communicate with iDigi server. */
/**
* @}
*/

 /**
 * @defgroup 412 idigi Status values
 * @{
 */
 /**
 * Status returned by iDigi connector API calls.
 */
typedef enum {
   connector_success,               /**< No error. */
   connector_init_error,            /**< iDigi connector was not initialized. */
   connector_invalid_data_size,     /**< Callback returned configuration with invalid size. */
   connector_invalid_data_range,    /**< Callback returned configuration that is out of range. */
   connector_invalid_data,          /**< Callback returned invalid data. Callback returned a NULL data. */
   connector_keepalive_error,       /**< iDigi connector did not receive keepalive messages. Server may be offline. */
   connector_bad_version,           /**< Invalid firmware version number. Incorrect firmware version number used in Remote Configuration (RCI) */
   connector_device_terminated,     /**< iDigi connector was terminated by user via connector_initiate_action call.
                                     All memory is freed and connector_init must be called to restart iDigi connector. */
   connector_service_busy,          /**< Someone else is using the same service or the device is busy. */
   connector_invalid_response,      /**< Received invalid response from the server. */
   connector_no_resource,           /**< Lack of resource */
   connector_unavailable,           /**< Not available to perform the specified action. */
   connector_idle,                  /**< iDigi connector is idling. iDigi connector has no message to process.
                                     Other task should be processed. */
   connector_working,               /**< iDigi connector has processed a message and should be called at the earliest possible time. */
   connector_pending,               /**< iDigi connector is waiting to process a message or a callback to complete. This allows
                                     iDigi connector to relinquish for other task to run. */
   connector_active,                /**< iDigi connector is processing a message and it should be called at the earliest possible time. */
   connector_abort,                 /**< iDigi connector was aborted and terminated either iDigi connector encountered fatal error or callback aborted iDigi connector.
                                     All memory is freed and connector_init must be called to restart iDigi connector. */
   connector_device_error,          /**< Close Callback returns error. If connector_step or connector_run is called again, it will re-establish the connection. */

   connector_exceed_timeout,        /**< Callback exceeded timeout value before it returned. */
   connector_invalid_payload_packet, /**< iDigi connector received invalid payload message. */
   connector_open_error             /**< Open callback was not successfully opened and returned @ref connector_callback_error.
                                     iDigi connector must be called again to continue running. It will reconnect if
                                     it's initially setup to connect automatically. */
} connector_status_t;
/**
* @}
*/

/**
* @defgroup connector_class_id_t Class IDs
* @{
*/
/**
* Class Id for which class is used in the application's callback.
* It tells class id for the request id passed to the application's callback.
*/
typedef enum {
    connector_class_id_config,             /**< Configuration Class Id */
    connector_class_id_operating_system,   /**< Operating System Class Id */
    connector_class_id_firmware,           /**< Firmware Facility Class Id */
    connector_class_id_data_service,       /**< Data Service Class Id */
    connector_class_id_remote_config,   /**< Remote Configuration Class ID */
    connector_class_id_file_system,        /**< File System Class Id */
    connector_class_id_network_tcp,     /**< TCP Network Class ID */
    connector_class_id_network_udp,        /**< UDP Network Class ID */
    connector_class_id_network_sms,        /**< SMS Network Class ID */
    connector_class_id_status,             /**< Class ID for all status */
    connector_class_id_short_message                  /**< Short message specific class ID */
} connector_class_id_t;
/**
* @}
*/

/**
* @defgroup connector_session_error_t Connector session error codes
* @{
*/
/**
* Error values returned from lower communication layer. It can be either from Etherios device cloud or from
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



/**
* @defgroup connector_file_system_request_t File System Request IDs
* @{
*/
/**
* File System Request Id passed to the application's callback to use file system.
* The class id for this connector_file_system_request_t is connector_class_id_file_system.
*/

typedef enum {
    connector_file_system_open,             /**< inform callback to open a file */
    connector_file_system_read,             /**< inform callback to read a file */
    connector_file_system_write,            /**< inform callback to write a file */
    connector_file_system_lseek,            /**< inform callback to seek file position */
    connector_file_system_ftruncate,        /**< inform callback to truncate a file */
    connector_file_system_close,            /**< inform callback to close a file */
    connector_file_system_rm,               /**< inform callback to remove a file */
    connector_file_system_stat,             /**< inform callback to geten file status */
    connector_file_system_opendir,          /**< inform callback to start processing a directory */
    connector_file_system_readdir,          /**< inform callback to read next directory entry */
    connector_file_system_closedir,         /**< inform callback to end processing a directory */
    connector_file_system_strerror,         /**< inform callback to get an error code description  */
    connector_file_system_msg_error,        /**< inform callback of an error in messaging layer */
    connector_file_system_hash              /**< inform callback to return file hash value */
} connector_file_system_request_t;
/**
* @}
*/

/**
* @defgroup connector_tcp_status_t Status Reason Types
* @{
*/
/**
* TCP Status values passed to the application's callback for @ref connector_tcp_status.
*
* These status values are used for an event notification.
*/
typedef enum {
    connector_tcp_communication_started,      /**< iDigi connector has established connection with the iDigi Device Cloud and starts communicating. */
    connector_tcp_keepalive_missed,           /**< iDigi connector didn't receive a keep alive message within the specified keep alive interval @ref tx_keepalive. */
    connector_tcp_keepalive_restored          /**< iDigi connector received a keep alive message after a keep alive message was not received.
                                         This is called after the callback has been previously notified with connector_miss_keepalive status. */
} connector_tcp_status_t;
/**
* @}
*/

/**
* @defgroup connector_request_id_status_t  iDigi Connector/Cloud 
* status request IDs @{ 
*/
/**
* These request IDs are used whenever the iDigi Connector wants to communicate either its
* status or the iDigi Device Cloud's status to the user.The class ID associated with these
* request ID is connector_class_id_status.
*/
typedef enum {
    connector_request_id_status_ping_response,    /**< Used in a callback when the iDigi Connector receives a status/ping response from the server */
    connector_request_id_status_ping_request,     /**< Used in a callback when the iDigi Connector receives a status/ping request from the server */
    connector_request_id_status_tcp,              /**< Used in a callback for iDigi connector TCP status. The callback is called to notify the application that
                                        TCP connection has been established, a keep-alive message was not received, or keep-alive message was received and restored.
                                          @see connector_tcp_status_t */
    connector_request_id_status_stop_completed    /**< Used in a callback when the iDigi connector has stopped a transport running via @ref connector_initiate_action call with @ref connector_initiate_transport_stop. */

} connector_request_id_status_t;
/**
* @}
*/

/**
* @defgroup connector_sm_request_t Short message specific request IDs
* @{
*/
/**
* Short message related callbacks will use these request IDs. The class ID associated with these request ID is
* connector_class_id_short_message and supported transport methods under this class are SMS and UDP.
*/
typedef enum {
    connector_sm_cli_request,     /**< Indicates CLI request is received from the iDigi device cloud. Also used to get the CLI response from the user */
    connector_sm_server_to_device_config,  /**< Used when device receives a server to device config request from iDigi Device Cloud. Used only if the transport method is SMS */
    connector_sm_device_to_server_config,  /**< Used when device receives a device to server config response from iDigi Device Cloud. Used only if the transport method is SMS */
    connector_sm_more_data,       /**< More data is available on the server. Applicable only if UDP transport method is used.
                                   User must use new request (can be ping) to pull the pending messages from iDigi Device Cloud. */
    connector_sm_opaque_response    /**< iDigi Connector uses this to provide the cloud response for which there is no associated request. */
} connector_sm_request_t;
/**
* @}
*/

/**
* @defgroup connector_initiate_request_t Initiate action enumeration
* @{
*/
/**
* Request IDs used in connector_initiate_action()
*/
typedef enum {
    connector_initiate_terminate,       /**< Terminates and stops iDigi connector from running. */
    #if (defined CONNECTOR_DATA_SERVICE)
    connector_initiate_send_data,       /**< Initiates the action to send data to the Etherios device cloud, the data will be stored in a file on Etherios device cloud. */
    #endif
    connector_initiate_transport_start, /**< Starts the specified (TCP, UDP or SMS) transport method */
    connector_initiate_transport_stop,  /**< Stops the specified (TCP, UDP or SMS) transport method */
    connector_initiate_status_message,  /**< Sends status message to the iDigi Device Cloud. Supported only under UDP and SMS transport methods */
    #if (defined CONNECTOR_DATA_POINT)
    connector_initiate_data_point_binary,  /**< Initiates the action to send a binary data point to Etherios Device Cloud */
    connector_initiate_data_point_single,  /**< Initiates the action to send data points of a stream to Etherios Device Cloud */
    #endif
    connector_initiate_config_message,  /**< Sends device configuration to the iDigi Device Cloud. Supported only under SMS transport method */
    connector_initiate_session_cancel   /**< Initiates the action to cancel the session, can be used in case of timeout. Supported only under UDP and SMS transport methods */
} connector_initiate_request_t;
/**
* @}
*/

/**
* @defgroup.connector_callback_status_t Callback return status
* @{
*/
/**
* Return status from iDigi connector callback
*/
typedef enum  {
    connector_callback_continue,        /**< Continues with no error */
    connector_callback_busy,            /**< Callback is busy */
    connector_callback_abort,           /**< Aborts iDigi connector. iDigi connector frees all memory. @ref connector_init must be called to restart iDigi connector. */
    connector_callback_unrecognized,    /**< Unsupported callback request */
    connector_callback_error            /**< Exits iDigi connector when error is encountered. iDigi connector will try reconnecting to the iDigi Device Cloud
                                         if @ref connector_step or @ref connector_run is called again. */
} connector_callback_status_t;
/**
* @}
*/

/**
* @}
*/

/**
* @defgroup connector_transport_t  Transport layer type
* @{
*/
/**
* iDigi Cloud Connector's will use the specified transport method when sending a request to the
* iDigi Device Cloud.
*/
typedef enum
{
    connector_transport_tcp, /**< Use TCP. @ref CONNECTOR_TRANSPORT_TCP must be enabled. */
    connector_transport_udp, /**< Use UDP. @ref CONNECTOR_TRANSPORT_UDP must be enabled. */
    connector_transport_sms, /**< Use SMS. @ref CONNECTOR_TRANSPORT_SMS must be enabled. */
    connector_transport_all  /**< All transports. */
} connector_transport_t;
/**
* @}
*/

#if (defined CONNECTOR_TRANSPORT_TCP) && (defined CONNECTOR_FIRMWARE_SERVICE)
#include "connector_api_firmware.h"
#endif

#if (defined CONNECTOR_DATA_SERVICE)
#include "connector_api_data_service.h"
#endif

#if (defined CONNECTOR_DATA_POINT)
#include "connector_api_data_point.h"
#endif

#include "connector_api_os.h"


/**
* @defgroup connector_request_id_t Request IDs
* @{
*/
/**
* Request IDs passed to callback identifying the type of request
* @see connector_class_id_t
*/
typedef union {
    connector_request_id_config_t config_request;                /**< Configuration request ID for configuration class */
    connector_request_id_os_t os_request;                           /**< Operating system request ID for operating system class */
    #if (defined CONNECTOR_TRANSPORT_TCP) && (defined CONNECTOR_FIRMWARE_SERVICE)
    connector_request_id_firmware_t firmware_request;            /**< Firmware request ID for firmware facility class */
    #endif
    #if (defined CONNECTOR_DATA_SERVICE)
    connector_request_id_data_service_t data_service_request;    /**< Data service request ID for data service class */
    #endif
    #if (defined CONNECTOR_TRANSPORT_TCP) && (defined CONNECTOR_RCI_SERVICE)
    connector_request_id_remote_config_t remote_config_request;     /**< Remote configuration request ID for remote configuration service class */
    #endif
    connector_file_system_request_t   file_system_request;       /**< File system request ID for file system class */
    connector_request_id_network_t  network_request;                /**< Network request ID for network TCP class, network UDP class, and network SMS class */
    connector_request_id_status_t status_request;                   /**< Status request ID for status class */
    connector_sm_request_t sm_request;                           /**< Short message request ID for SM class */
} connector_request_id_t;
/**
* @}
*/

/**
* @defgroup connector_handle_t iDigi Handle
* @{
*/
/**
*
* iDigi connector Handle type that is used throughout iDigi connector APIs, this is used by the application
* to store context information about a connections, for example this could
* be used to store a file descriptor or a pointer to a structure.
*/
#define connector_handle_t void *
/**
* @}
*/

/**
* @defgroup connector_error_status_t Error Status
* @{
*/
/**
* Error status structure for @ref connector_request_id_config_error_status callback which
* is called when iDigi connector encounters an error.
* @see connector_request_id_config_t
*/
typedef struct  {
    connector_class_id_t class_id;         /**< Class ID which iDigi connector encounters error with */
    connector_request_id_t request_id;     /**< Request ID which iDigi connector encounters error with */
    connector_status_t status;          /**< Error status */
} connector_error_status_t;
/**
* @}
*/




#ifdef CONNECTOR_FILE_SYSTEM
/**
* @defgroup connector_file_seek_origin_t File seek origin
* @{
*/
/**
 * Seek file position relative to start-of-file.
 *
 * @see connector_file_lseek_request_t
 * @see connector_file_system_lseek
 */
#define CONNECTOR_SEEK_SET	0

/**
 * Seek file position relative to current position.
 *
 * @see connector_file_lseek_request_t
 * @see connector_file_system_lseek
 */
#define CONNECTOR_SEEK_CUR	1

/**
 * Seek file position relative to end-of-file.
 *
 * @see connector_file_lseek_request_t
 * @see connector_file_system_lseek
 */
#define CONNECTOR_SEEK_END	2
/**
* @}
*/

/**
* @defgroup file_open_flag_t File open flags
* @{
*/
/**
 * Open file for reading only.
 *
 * @see connector_file_open_request_t
 * @see connector_file_system_open callback
 */
#define	CONNECTOR_O_RDONLY	0

/**
 * Open for writing only.
 *
 * @see connector_file_open_request_t
 * @see connector_file_system_open callback
 */
#define	CONNECTOR_O_WRONLY	1

/**
 * Open for reading and writing.
 *
 * @see connector_file_open_request_t
 * @see connector_file_system_open callback
 */
#define	CONNECTOR_O_RDWR	2

/**
 * File offset shall be set to the end of the file prior to each write.
 *
 * @see connector_file_open_request_t
 * @see connector_file_system_open callback
 */
#define	CONNECTOR_O_APPEND	0x0008

/**
 * Create file, if does not exist.
 *
 * @see connector_file_open_request_t
 * @see connector_file_system_open callback
 */
#define	CONNECTOR_O_CREAT	0x0200

/**
 *
 * Truncate file, successfully opened for writing to 0 length, don't change
 * the owner and ile access modes.
 *
 * @see connector_file_open_request_t
 * @see connector_file_system_open callback
 */
#define	CONNECTOR_O_TRUNC	0x0400
/**
* @}
*/

/**
* @defgroup file_stat_flag_t File status flags
* @{
*/
/**
 * File is a directory.
 *
 * @see connector_file_stat_t
 * @see connector_file_system_stat callback
 */
#define CONNECTOR_FILE_IS_DIR   0x01

/**
 * File is a regular file.
 *
 * @see connector_file_stat_t
 * @see connector_file_system_stat callback
 */
#define CONNECTOR_FILE_IS_REG   0x02
/**
* @}
*/

/**
* @defgroup connector_file_error_status_t File system error status
* @{
*/
/**
* Error code, used on return in the error_status field of @ref connector_file_error_data_t
* in all file system callbacks.
*/
typedef enum
{
    connector_file_noerror,                      /**< No error */
    connector_file_user_cancel,                  /**< User application requests to cancel the session */
    connector_file_unspec_error,                 /**< Fatal unspecified error */
    connector_file_path_not_found,               /**< Path not found */
    connector_file_insufficient_storage_space,   /**< Insufficient storage space */
    connector_file_request_format_error,         /**< Request format error */
    connector_file_invalid_parameter,            /**< Invalid parameter */
    connector_file_out_of_memory,                /**< Out of memory */
    connector_file_permision_denied              /**< Permision denied */
} connector_file_error_status_t;
/**
* @}
*/

/**
* @defgroup connector_file_hash_algorithm_t File system hash algorithm
* @{
*/
/**
* Hash algorithm gives different options for hash values returned in the file lisings.
*
* @see @ref connector_file_system_stat
* @see @ref connector_file_system_hash
*/
typedef enum
{
    connector_file_hash_none,       /**< Don't return hash value */
    connector_file_hash_best,       /**< Use best available algorithm */
    connector_file_hash_crc32,      /**< Use crc32 */
    connector_file_hash_md5         /**< Use md5 */
} connector_file_hash_algorithm_t;
/**
* @}
*/

/**
* @defgroup connector_file_stat_t File status data
* @{
*/
/**
* File status data structure is used to return the status of a direcory or a file, specified by the path.
* It is used in @ref connector_file_stat_response_t for @ref connector_file_system_stat callback.
* The returned hash_alg value will be used in the consequent @ref connector_file_system_hash callbacks.
*/
typedef struct
{
  uint32_t     last_modified; /**< Last modified time for the entry (seconds since 1970). If not supported, use 0 */
  size_t       file_size;               /**< File size in bytes */
  connector_file_hash_algorithm_t hash_alg; /**< hash algorithm */
  uint8_t flags;                   /**< 0, or @ref file_stat_flag_t */
} connector_file_stat_t;
/**
* @}
*/

/**
* @defgroup connector_file_request_t File or directory request
* @{
*/
/**
* File or directory request structure is used for @ref connector_file_system_read, @ref connector_file_system_close,
* @ref connector_file_system_readdir, and @ref connector_file_system_closedir,
*/
typedef struct
{
    void * handle;            /**< Application defined file or directory handle */

} connector_file_request_t;
/**
* @}
*/

/**
* @defgroup connector_file_lseek_request_t File lseek request
* @{
*/
/**
* File lseek request structure is used for @ref connector_file_system_lseek callback.
*/
typedef struct
{
    void   * handle;      /**< Application defined file handle */
    long int offset;      /**< File offset */
    int      origin;      /**< File seek origin of @ref connector_file_seek_origin_t type */

} connector_file_lseek_request_t;
/**
* @}
*/

/**
* @defgroup connector_file_ftruncate_request_t File ftruncate request
* @{
*/
/**
* File ftruncate request structure is used for @ref connector_file_system_ftruncate callback.
*/
typedef struct
{
    void   * handle; /**< Application defined file handle */
    long int length;      /**< File length in bytes to truncate to */

} connector_file_ftruncate_request_t;
/**
* @}
*/

/**
* @defgroup connector_file_write_request_t File write request
* @{
*/
/**
* File write request structure is used for @ref connector_file_system_write callback.
*/
typedef struct
{
    void       * handle;        /**< Application defined file handle */
    void const * data_ptr;      /**< A pointer to data to be written to a file */
    size_t       size_in_bytes; /**< Number of bytes to write */

} connector_file_write_request_t;
/**
* @}
*/

/**
* @defgroup connector_file_open_request_t File open request
* @{
*/
/**
* File open request structure is used for @ref connector_file_system_open callback.
*/
typedef struct
{
    char const * path;                      /**< File path */
    int          oflag;                     /**< bitwise-inclusive OR of @ref file_open_flag_t flags */
} connector_file_open_request_t;
/**
* @}
*/

/**
* @defgroup connector_file_stat_request_t File status request
* @{
*/
/**
* File status request structure is used for @ref connector_file_system_stat and @ref connector_file_system_hash
* callbacks.
*/
typedef struct
{
    char const * path;                      /**< File path */
    connector_file_hash_algorithm_t hash_alg;   /**< suggested hash algorithm */
} connector_file_stat_request_t;
/**
* @}
*/

/**
* @defgroup connector_file_path_request_t File path request
* @{
*/
/**
* File path request structure is used for @ref connector_file_system_rm and
* @ref connector_file_system_opendir callbacks.
*/
typedef struct
{
    char const * path;                      /**< File path */
} connector_file_path_request_t;
/**
* @}
*/

/**
* @defgroup connector_file_error_data_t File error data
* @{
*/
/**
* Error data structure is used to return error status and error token for all file system callbacks.
* An error token is used to return an error description in a future @ref connector_file_system_strerror callback.
*
*/
typedef struct
{
    void const * errnum;                    /**< Application defined error token.*/
    connector_file_error_status_t error_status; /**< Error status of @ref connector_file_error_status_t type */
} connector_file_error_data_t;
/**
* @}
*/

/**
* @defgroup connector_file_error_request_t File system messaging error request
* @{
*/
/**
* Messaging error request structure is used for @ref connector_file_system_msg_error callback.
*/
typedef struct
{
    connector_msg_error_t message_status;   /**< Error status in messaging layer */
} connector_file_error_request_t;
/**
* @}
*/

/**
* @defgroup connector_file_data_response_t File system data response
* @{
*/
/**
* File system data response structure is used for @ref connector_file_system_read,
* @ref connector_file_system_readdir, and @ref connector_file_system_strerror callbacks.
*/
typedef struct
{
    void                    * user_context; /**< To hold the user context */
    connector_file_error_data_t * error;        /**< Holds error status of @ref connector_file_error_status_t type and errnum */
    void                    * data_ptr;     /**< Pointer to write response data */
    size_t                  size_in_bytes;  /**< On input size of data buffer, on output size of response data */
} connector_file_data_response_t;
/**
* @}
*/

/**
* @defgroup connector_file_response_t File system basic response
* @{
*/
/**
* File system basic response structure is used for @ref connector_file_system_close,
* @ref connector_file_system_rm, @ref connector_file_system_closedir,
* @ref connector_file_system_ftruncate, and  @ref connector_file_system_msg_error callbacks.
*/
typedef struct
{
    void                    * user_context; /**< To hold the user context */
    connector_file_error_data_t * error;        /**< Pointer to file error data */
} connector_file_response_t;
/**
* @}
*/

/**
* @defgroup connector_file_open_response_t File or directory open response
* @{
*/
/**
* File or directory open response structure is used for the @ref connector_file_system_open
* and @ref connector_file_system_opendir callback.
*/
typedef struct
{
    void                    * user_context; /**< To hold the user context */
    connector_file_error_data_t * error;        /**< Pointer to file error data */
    void                    * handle;       /**< Application defined file handle or directory */
} connector_file_open_response_t;
/**
* @}
*/

/**
* @defgroup connector_file_lseek_response_t File lseek response
* @{
*/
/**
* File lseek response structure is used for @ref connector_file_system_lseek callback.
*/
typedef struct
{
    void                    * user_context; /**< To hold the user context */
    connector_file_error_data_t * error;        /**< Pointer to file error data */
    long int                  offset;       /**< Resulting file position */
} connector_file_lseek_response_t;
/**
* @}
*/

/**
* @defgroup connector_file_write_response_t File write response
* @{
*/
/**
* File write response structure is used for @ref connector_file_system_write callback.
*/
typedef struct
{
    void                    * user_context;  /**< To hold the user context */
    connector_file_error_data_t * error;         /**< Pointer to file error data */
    size_t                    size_in_bytes; /**< On input size of data buffer, on output size of response data */
} connector_file_write_response_t;
/**
* @}
*/


/**
* @defgroup connector_file_stat_response_t File status response
* @{
*/
/**
* File status response structure is used for @ref connector_file_system_stat callback.
*/
typedef struct
{
    void                    * user_context; /**< To hold the user context */
    connector_file_error_data_t * error;        /**< Pointer to file error data */
    connector_file_stat_t         statbuf;      /**< Pointer to place file status data on output */
} connector_file_stat_response_t;
/**
* @}
*/
#endif

#ifdef CONNECTOR_SM
/**
* @defgroup connector_message_status_response_t Data type used in all the status responses
* @{
*/
/**
* This data structure is used when the callback is called with the iDigi Device Cloud response
* if the response is requested and it contains just the status.
*
* @see connector_status_ping_response
* @see connector_sm_device_to_server_config
* @see connector_initiate_binary_point
*/
typedef struct
{
    void const * user_context;  /**< User context passed in the request */
    connector_session_status_t status;  /**< Response status */
    char const * error_text;    /**< An optional, null-terminated error string */
} connector_message_status_response_t;
/**
* @}
*/

/**
* @defgroup connector_message_status_request_t Data type used for status message/response
* @{
*/
/**
* This data structure is used when the connector_initiate_action() API is called with
* connector_initiate_status_message, connector_initiate_config_message where only the status
* is returned from the iDigi Device Cloud.
*
* @see connector_initiate_status_message
* @see connector_initiate_config_message
* @see connector_initiate_action
* @see connector_initiate_session_cancel
*/
typedef struct
{
    connector_transport_t transport;    /**< Transport method to use */
    unsigned int flags;             /**< Set to CONNECTOR_DATA_RESPONSE_NOT_NEEDED if response is not needed */
    void const * user_context;      /**< User provided application context */
} connector_message_status_request_t;
/**
* @}
*/

/**
* @defgroup connector_sm_cli_request_t Data type used for cli request and response
* @{
*/
/**
* This data structure is used when the callback is called to pass the request or to get the response.
*
* @see connector_sm_cli_request
*
*/
typedef struct
{
    connector_data_service_type_t type;    /**< Type to select the content to use. have_data: CLI request, need_data: CLI response,  total_length: Need total response bytes */
    void const * user_context;         /**< User context, will be NULL when request is called, so that user can update this. Will carry user assigned context in the response */

    union
    {
        struct
        {
            char const * buffer;       /**< Buffer pointer to write the CLI response to */
            size_t bytes;              /**< It carries the sizeof CLI command */
            unsigned int flags;        /**< Will be set to CONNECTOR_MSG_RESPONSE_NOT_NEEDED if response is not needed */
        } request;

        struct
        {
            char * buffer;             /**< Buffer pointer to write the CLI response to */
            size_t bytes;              /**< When called it contains the size of the available response buffer size, in return it will carry the filled buffer size */
            connector_session_status_t status; /**< Response status */
        } response;

        connector_session_status_t status; /**< When the type is connector_data_service_type_session_status, then this field will carry the reason for the session close */
        size_t * total_bytes_ptr;      /**< When the type is connector_data_service_type_total_length, then user need to update the content of this pointer
                                            with total response length. Applicable only for UDP and SMS transport methods */
    } content;

} connector_sm_cli_request_t;
/**
* @}
*/

/**
* @defgroup connector_server_to_device_config_t Data type used for the Cloud to device config request
* @{
*/
/**
* This data structure is used when the callback is called with connector_data_service_server_to_device_config request id.
*
* @see connector_sm_server_to_device_config
*
*/
typedef struct
{
    char * phone_number;    /**< A null-terminated phone number */
    char * service_id;      /**< A null-terminated service identifier. It can be empty */
} connector_server_to_device_config_t;
/**
* @}
*/

/**
* @defgroup connector_device_to_server_config_t Data type used to send transport specific configuration message
* @{
*/
/**
* This data structure is used when the connector_initiate_action() API is called with
* connector_initiate_config_message request id.
*
* @see connector_initiate_action
* @see connector_initiate_config_message
* @see connector_sm_device_to_server_config
* @see connector_message_status_response_t
*/
typedef struct
{
    connector_transport_t transport; /**< Transport layer to use */
    unsigned int flags;          /**< Set to CONNECTOR_DATA_RESPONSE_NOT_NEEDED if response is not needed */
    void const * user_context;   /**< User context to be passed back in the response */
    unsigned int sim_slot;       /**< The SIM slot number, use 0 for unknown */
    char * identifier;           /**< A null-terminated, SIM or phone number identifier */
} connector_device_to_server_config_t;
/**
* @}
*/

/**
* @defgroup connector_sm_opaque_response_t Data type used to deliver the opaque response from the Cloud
* @{
*/
/**
* This data structure is used when the callback is called to pass the Cloud response for which the
* associated request is not present. It may be cancelled by the user or iDigi Connector maybe restarted
* after sending the request.
*
* @see connector_sm_opaque_response
*
*/
typedef struct
{
    uint32_t id;                    /**< Can be used to keep track of a multipart response */
    void * data;                    /**< Pointer to opaque response */
    size_t bytes;                   /**< Number of bytes available in the data */
    unsigned int flags;             /**< Can be used to track last data block */
    connector_session_status_t status;  /**< Response status, Application can use this to discard the subsequent
                                        parts by returning connector_session_status_cancel */
} connector_sm_opaque_response_t;
/**
* @}
*/
#endif


/**
* @defgroup connector_stop_condition_t Condition used on @ref connector_initiate_transport_stop in @ref connector_initiate_action
* @{
*/
/**
* This is used to tell how iDigi connector handles all active sessions before stopping.
*
*/
typedef enum {
    connector_stop_immediately,             /**< Cancel all active sessions and stop the transport of iDigi connector immediately */
    connector_wait_sessions_complete         /**< Stop the transport of iDigi connector after all active sessions complete */
} connector_stop_condition_t;
/**
* @}
*/

/**
* @defgroup connector_initiate_stop_request_t Data type used to stop a transport of iDigi connector
* @{
*/
/**
* This data structure is used on @ref connector_initiate_transport_stop in @ref connector_initiate_action API
* to stop a transport.
*
*/
typedef struct
{
    connector_transport_t transport;        /**< Transport will be stopped:
                                            - @ref connector_transport_tcp
                                            - @ref connector_transport_udp
                                            - @ref connector_transport_sms
                                            - @ref connector_transport_all */

    connector_stop_condition_t condition;   /**< Condition to stop the transport:
                                             - @ref connector_stop_immediately
                                             - @ref connector_wait_sessions_complete */

    void * user_context;                /**< User's defined context that will be passed to @ref connector_status_stop_completed after the transport of  iDigi connector has stopped running. */

} connector_initiate_stop_request_t;
/**
* @}
*/


 /**
 * @defgroup connector_callback_t Application-defined callback
 * @{
 * @b Include: connector_api.h
 *
 * @brief   The is the iDigi Connector Application defined callback structure.
 *
 * An application must define this function and pass this to the iDigi Connector during the
 * connector_init() call.  This callback is used to pass configuration data and exchange system
 * services with the iDigi COnnector state machine.  The callback return value is then used to
 * guide the iDigi Connector how to proceed after completion of the application callback.
 *
 */
 /**
 * @param class_id 				This is a grouping or category of callback functions.  Each class_id contains a number of related request_id's.
 * @param request_id 			The request ID defines the specific callback being requested.
 * @param data 			        Points to specific structure for a given class ID and request ID
 *
 * @retval	connector_callback_continue	The callback completed successfully and the iDigi Connector should continue
 * 										it's process.
 * @retval  connector_callback_busy		The callback could not complete the operation within the allowable time frame.
 * 										Do not advance the state of the iDigi Connector and recall this callback at some
 * 										later time.
 * @retval  connector_callback_error    An application level error occured while processing the callback.
 * @retval 	connector_callback_abort	The application is requesting the iDigi Connector to abort execution.  This will
 * 										cause connector_run() or connector_step() to terminate with status @ref connector_abort.
 * @retval	connector_callback_unrecognized	An unsupported and unrecognized callback was received.  The application does not
 * 										support this request.  This should be implemented in the application to allow for
 * 										graceful recovery and upgrade-ability to future iDigi Connector releases.
 *
 * @see connector_callback_status_t
 * @see connector_init()
 */
typedef connector_callback_status_t (* connector_callback_t) (connector_class_id_t const class_id, connector_request_id_t const request_id, void * const data);
/**
* @}
*/

 /**
 * @defgroup connector_init Initialize the iDigi connector.
 *@{
 * @b Include: connector_api.h
 */
/**
 * @brief This API is called initially at startup to allocate and initialize the iDigi connector.
 *
 * This function takes the @ref iik_callback "application-defined callback" function pointer as
 * it's only argument.  This callback is used by the iDigi connector to communicate with the
 * application environment.  This function must be called prior to other iDigi connector APIs,
 * like connector_step(), connector_run(), and connector_initiate_action().
 *
 * @param [in] callback  Callback function that is used to
 *        interface between the application and the iDigi connector.
 *
 * @retval NULL   		An error occurred and the iDigi connector was unable to initialize.
 * @retval "Not NULL"	Success.  A Handle was returned for subsequent iDigi Connector calls.
 *
 * Example Usage:
 * @code
 *    connector_handle = connector_init(application_callback);
 * @endcode
 *
 * @see connector_handle_t
 * @see connector_callback_t
 */
connector_handle_t connector_init(connector_callback_t const callback);
/**
* @}
*/

/**
 * @defgroup connector_step Step Routine
 * @{
 * @b Include: connector_api.h
 */
/**
 * @brief   Executes a small portion of the iDigi Connector in a non-blocking call.
 *
 * This non-blocking function is used to execute a portion the iDigi Connector after it's been
 * successfully initialized with an connector_init() call.  This function performs a sequence of
 * operations or events and then returns.  This allows applications to perform other tasks,
 * especially in non-threading environment.  For more information on the differences between
 * connector_run() and connector_step() see the @ref threading "iDigi Connector threading model".
 *
 * This function is similar to connector_run() except this is the non-blocking variant.  This function would
 * normally be made from system control loop or state machine.  This function must be repeatedly made to
 * maintain the iDigi Connector state.
 *
 * See the iDigi Connector @ref threading "threading model" section for more information on the differences
 * between connector_run() and connector_step().
 *
 * @param [in] handle  Handle returned from connector_init
 *
 * @retval connector_init_error             The iDigi connector was not properly initialized.  The application requires a
 * 										call to connector_init() prior to calling this function.
 * @retval connector_abort                  iDigi connector was aborted by callback function.
 * @retval connector_invalid_data_size      An @ref iik_callback "application callback" returned an invalid response_length.
 *   									This is an application callback defect and should be corrected.  To locate the
 *  									faulty callback, enable @ref CONNECTOR_DEBUG.  For more information on
 * 										debugging, see the section on @ref debug_routine "how to implement debug support".
 * @retval connector_invalid_data_range     An @ref iik_callback "application callback" returned invalid response_data.
 *  									This is an application callback defect and should be corrected.  To locate the
 *  									faulty callback, enable @ref CONNECTOR_DEBUG.  For more information on
 * 										debugging, see the section on @ref debug_routine "how to implement debug support".
 * @retval connector_invalid_payload_packet iDigi connector received invalid payload message.
 * @retval connector_keepalive_error        iDigi connector did not receive keep alive messages. Server may be offline.
 * @retval connector_server_overload        Server overload.
 * @retval connector_bad_version            Server rejected version number.
 * @retval connector_exceed_timeout         Callback exceeded timeout value before it returned.
 * @retval connector_unsupported_security   iDigi connector received a packet with unsupported security.
 * @retval connector_invalid_data           An @ref iik_callback "application callback" returned unexpected NULL response_data.
 *  									This is an application callback defect and should be corrected.  To locate the
 *  									faulty callback, enable @ref CONNECTOR_DEBUG.  For more information on
 * 										debugging, see the section on @ref debug_routine "how to implement debug support".
 * @retval connector_device_terminated      iDigi connector was terminated by user via connector_initiate_action() call.
 * @retval connector_server_redirect        iDigi connector was stopped when redirecting to a new server.
 * @retval connector_idle                   iDigi connector is idling. iDigi connector has no message to process and relinquishes other task execution.
 * @retval connector_working                iDigi connector is processing a message and should be called at the earliest possible time.
 * @retval connector_pending                iDigi connector is busy or waiting to process a message and relinquishes other task execution
 * @retval connector_active                 iDigi connector is busy or waiting to process a message and should be called
 *                                      at the earliest possible time
 *
 *
 * Example Usage:
 * @code
 *     connector_status_t status;
 *     connector_handle_t connector_handle = connector_init(application_callback);
 *
 *     for (;;)
 *     {
 *     	    status = connector_step(connector_handle);
 *     	    other_system_tasks();
 *     }
 * @endcode
 *
 * @see connector_init()
 * @see connector_handle_t
 * @see connector_status_t
 * @see connector_run()
 */
connector_status_t connector_step(connector_handle_t const handle);
/**
* @}
*/


 /**
 * @defgroup connector_run Run routine
 * @{
 * @b Include: connector_api.h
 */
/**
 * @brief   Runs the iDigi Connector as a blocking call.
 *
 * This blocking function is typically used in a separate thread (in a multi-threaded environment) to run
 * the iDigi Connector after it's been successfully initialized with an connector_init() call.  This function
 * only returns when the iDigi Connector encounters an unexpected error.  This function is similar to
 * connector_step() except this is the blocking variant.
 *
 * See the iDigi Connector @ref threading "Threading Model section" for more information on the differences
 * between connector_run() and connector_step().
 *
 * @param [in] handle  					Handle returned from connector_init() call.
 *
 * @retval connector_init_error             The iDigi connector was not properly initialized.  The application requires a
 * 										call to connector_init() prior to calling this function.
 * @retval connector_abort                  iDigi connector was aborted by callback function.
 * @retval connector_invalid_data_size      An @ref iik_callback "application callback" returned an invalid response_length.
 *   									This is an application callback defect and should be corrected.  To locate the
 *  									faulty callback, enable @ref CONNECTOR_DEBUG.  For more information on
 * 										debugging, see the section on @ref debug_routine "how to implement debug support".
 * @retval connector_invalid_data_range     An @ref iik_callback "application callback" returned invalid response_data.
 *  									This is an application callback defect and should be corrected.  To locate the
 *  									faulty callback, enable @ref CONNECTOR_DEBUG.  For more information on
 * 										debugging, see the section on @ref debug_routine "how to implement debug support".
 * @retval connector_keepalive_error        iDigi connector did not receive keep alive messages. Server may be offline.
 * @retval connector_server_overload        Server overload.
 * @retval connector_bad_version            Server rejected version number.
 * @retval connector_exceed_timeout         Callback exceeded timeout value before it returned.
 * @retval connector_unsupported_security   iDigi connector received a packet with unsupported security.
 * @retval connector_invalid_data           An @ref iik_callback "application callback" returned unexpected NULL response_data.
 *  									This is an application callback defect and should be corrected.  To locate the
 *  									faulty callback, enable @ref CONNECTOR_DEBUG.  For more information on
 * 										debugging, see the section on @ref debug_routine "how to implement debug support".
 * @retval connector_server_disconnected    Server disconnected iDigi connector.
 * @retval connector_device_error           Close Callback returns
 *                                      error. If connector_step or connector_run is called again, it
 *                                      will re-establish the connection.
 * @retval connector_device_terminated      iDigi connector was terminated by user via connector_initiate_action call.
 * @retval connector_server_redirect        iDigi connector was stopped when redirecting to a new server.
*
 * Example Usage:
 * @code
 *     connector_status_t status;
 *     connector_handle_t connector_handle = connector_init(application_callback);
 *
 *     status = connector_run(connector_handle);
 * @endcode
 *
 * @see connector_init()
 * @see connector_handle_t
 * @see connector_status_t
 * @see connector_step()
 */
connector_status_t connector_run(connector_handle_t const handle);
/**
* @}
*/


 /**
 * @defgroup connector_initiate_action Initiate Action
 * @{
 * @b Include: connector_api.h
 */
/**
 * @brief   Requests the iDigi Connector to perform an asynchronous action.
 *
 * This function is called to initiate an iDigi Connector action.  It can be
 * used to initiate a send data sequence from the device to the iDigi Device Cloud,
 * or to terminate the iDigi Connector library.
 *
 * @param [in] handle  Handle returned from the connector_init() call.
 *
 * @param [in] request  Request action (one of the following):
 *                      @li @b connector_initiate_terminate:
 *                          Terminates and stops the iDigi Connector from running.  This call closes all open network
 *                          Handles and frees all allocated memory.
 *                          If the application is using connector_step(), the next call to connector_step() terminates
 *                          the iDigi connector.  If a blocking call to connector_run() is still pending, this call
 *                          will eventually terminate that call.  Once the iDigi Connector is terminated, the
 *                          iDigi Connector must restart by calling connector_init().
 *
 *                      @li @b connector_initiate_send_data:
 *                           This is used to trigger the send
 *                           data to the iDigi Device Cloud. Only the
 *                           header information is passed by
 *                           this method. The actual data is
 *                           transferred through callbacks. The
 *                           data is stored in a specified file
 *                           on the server.
 *
 *                      @li @b connector_initiate_transport_start:
 *                          Starts the specified (@ref connector_transport_tcp, @ref connector_transport_udp or
 *                          @ref connector_transport_sms) transport method.
 *
 *                      @li @b connector_initiate_transport_stop:
 *                          Stops the specified transport:
 *                              - @ref connector_transport_tcp - TCP transport,
 *                              - @ref connector_transport_udp - UDP transport
 *                              - @ref connector_transport_sms - SMS transport
 *                              - @ref connector_transport_all - all transports.
 *                              - @ref @b Note: This triggers @ref connector_status_stop_completed callback. @b See @ref status_stop_completed callback.
 *
 *                      @li @b connector_initiate_status_message:
 *                          Sends status message to the iDigi Device Cloud.  Supported only under
 *                          @ref connector_transport_udp and @ref connector_transport_sms transport methods.
 *
 *                      @li @b connector_initiate_data_point_binary:
 *                          Initiates the action to send a binary data point to Etherios Device Cloud.
 *
 *                      @li @b connector_initiate_data_point_single:
 *                          Initiates the action to send data points which are belongs to a single stream on Etherios Device Cloud.
 *
 *                      @li @b connector_initiate_config_message:
 *                          Sends configuration info to the iDigi Device Cloud. Supported only under SMS transport method.
 *
 *                      @li @b connector_initiate_session_cancel:
 *                          Initiates the action to cancel the timedout session.
 *
 * @param [in] request_data  Pointer to Request data
 *                      @li @b connector_initiate_terminate:
 *                          Should be NULL.
 *                      @li @b connector_initiate_send_data:
 *                          Pointer to connector_data_service_put_request_t.
 *                      @li @b connector_initiate_transport_start:
 *                          Pointer to @ref connector_transport_t "connector_transport_t"
 *                      @li @b connector_initiate_transport_stop:
 *                          Pointer to @ref connector_initiate_stop_request_t "connector_initiate_stop_request_t"
 *                      @li @b connector_initiate_status_message:
 *                          Pointer to connector_message_status_request_t
 *                      @li @b connector_initiate_data_point_binary:
 *                          Pointer to connector_request_data_point_binary_t
 *                      @li @b connector_initiate_data_point_single:
 *                          Pointer to connector_request_data_point_single_t
 *                      @li @b connector_initiate_config_message:
 *                          Pointer to connector_device_to_server_config_t
 *                      @li @b connector_initiate_session_cancel:
 *                          Pointer to connector_message_status_request_t
 *
* @retval connector_success              No error
 * @retval connector_init_error           iDigi connector was not initialized or not connected to the iDigi Device Cloud.
 * @retval connector_abort                Callback aborted iDigi connector.
 * @retval connector_invalid_data         Invalid parameter
 * @retval connector_no_resource          Insufficient memory
 * @retval connector_service_busy         iDigi connector is busy
 *
 * Example Usage:
 * @code
 *     connector_data_service_put_request_t  file_info;
 *     :
 *     status = connector_initiate_action(handle, connector_initiate_send_data, &file_info);
 *     :
 *     :
 *     status = connector_initiate_action(connector_handle, connector_initiate_terminate, NULL);
 * @endcode
 *
 * @see connector_handle_t
 * @see connector_callback_t
 */
connector_status_t connector_initiate_action(connector_handle_t const handle, connector_initiate_request_t const request, void const * const request_data);
/**
* @}.
*/

#endif /* _CONNECTOR_API_H */

