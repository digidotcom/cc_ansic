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
#ifndef _CONNECTOR_API_FILE_SYSTEM_H
#define _CONNECTOR_API_FILE_SYSTEM_H

/**
* @defgroup connector_request_id_file_system_t File System 
* Request IDs @{ 
*/
/**
* File System Request Id passed to the application's callback to use file system.
* The class id for this connector_request_id_file_system_t is 
* connector_class_id_file_system. 
*/
typedef enum {
    connector_request_id_file_system_open,             /**< inform callback to open a file */
    connector_request_id_file_system_read,             /**< inform callback to read a file */
    connector_request_id_file_system_write,            /**< inform callback to write a file */
    connector_request_id_file_system_lseek,            /**< inform callback to seek file position */
    connector_request_id_file_system_ftruncate,        /**< inform callback to truncate a file */
    connector_request_id_file_system_close,            /**< inform callback to close a file */
    connector_request_id_file_system_remove,           /**< inform callback to remove a file */
    connector_request_id_file_system_stat,             /**< inform callback to get file status */
    connector_request_id_file_system_stat_dir_entry,   /**< inform callback to get file status for directory entry */
    connector_request_id_file_system_opendir,          /**< inform callback to start processing a directory */
    connector_request_id_file_system_readdir,          /**< inform callback to read next directory entry */
    connector_request_id_file_system_closedir,         /**< inform callback to end processing a directory */
    connector_request_id_file_system_get_error,         /**< inform callback to get the error data informatiopn */
    connector_request_id_file_system_session_error,     /**< inform callback of an error condition */
    connector_request_id_file_system_hash              /**< inform callback to return file hash value */
} connector_request_id_file_system_t;
/**
* @}
*/


#if (defined CONNECTOR_FILE_SYSTEM_HAS_LARGE_FILES)
typedef int64_t connector_file_offset_t;
#else
typedef int32_t connector_file_offset_t;
#endif

/**
* @defgroup connector_file_system_open_flag_t File open flags
* @{
*/
/**
 * Open file for reading only.
 *
 * @see connector_file_system_open_t
 * @see connector_request_id_file_system_open callback
 */
#define	CONNECTOR_FILE_O_RDONLY	0

/**
 * Open for writing only.
 *
 * @see connector_file_system_open_t
 * @see connector_request_id_file_system_open callback
 */
#define	CONNECTOR_FILE_O_WRONLY	1

/**
 * Open for reading and writing.
 *
 * @see connector_file_system_open_t
 * @see connector_request_id_file_system_open callback
 */
#define	CONNECTOR_FILE_O_RDWR	2

/**
 * File offset shall be set to the end of the file prior to each write.
 *
 * @see connector_file_system_open_t
 * @see connector_request_id_file_system_open callback
 */
#define	CONNECTOR_FILE_O_APPEND	0x0008

/**
 * Create file, if does not exist.
 *
 * @see connector_file_system_open_t
 * @see connector_request_id_file_system_open callback
 */
#define	CONNECTOR_FILE_O_CREAT	0x0200

/**
 *
 * Truncate file, successfully opened for writing to 0 length, don't change
 * the owner and ile access modes.
 *
 * @see connector_file_system_open_t
 * @see connector_request_id_file_system_open callback
 */
#define	CONNECTOR_FILE_O_TRUNC	0x0400
/**
* @}
*/
 
 
/**
* @defgroup connector_file_system_open_t Data type used 
* for file system open callback 
* @{ 
*/
/**
* Data structure used in connector_request_id_file_system_open 
* callback. 
*/
typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * CONST path;                /**< File path */
    int CONST oflag;                             /**< bitwise-inclusive OR of @ref connector_file_system_open_flag_t flags */
    void * handle;                          /**< Don't check handle in private code!!!  Application defined file handle */

} connector_file_system_open_t;
/**
* @}
*/

/**
* @defgroup connector_file_system_lseek_t Data type used 
* for file system lseek callback 
* @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_lseek callback. 
*/
typedef struct
{
   void * user_context;                    		    /**< Holds user context */
   void * errnum;                          		    /**< Application defined error token */

   void * CONST handle;                             /**< Application defined file handle */
   connector_file_offset_t CONST requested_offset;  /**< Requested file offset */
   connector_file_offset_t resulting_offset;	    /**< Resulting file position */
   enum
   {
        connector_file_system_seek_set,    		    /**<  Seek file position relative to start-of-file */
        connector_file_system_seek_cur,    		    /**<  Seek file position relative to current position */
        connector_file_system_seek_end     		    /**<  Seek file position relative to end-of-file */

   } CONST origin;                                  /**< File seek origin */

} connector_file_system_lseek_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_write_t Data type used 
* for file system write callback 
* @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_write callback. 
*/
typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void *  CONST handle;                   /**< Application defined file handle */
    void const * CONST buffer;              /**< A pointer to data to be written to a file */
    size_t  CONST bytes_available;          /**< Number of bytes to write */
    size_t  bytes_used;                     /**< Number of bytes written to a file */

} connector_file_system_write_t;
/**
* @}
*/

/**
* @defgroup connector_file_system_truncate_t Data type used 
* for file system truncate callback 
* @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_ftruncate callback. 
*/
typedef struct
{
    void * user_context;                    	    /**< Holds user context */
    void * errnum;                          	    /**< Application defined error token */

    void * CONST handle;                          	/**< Application defined file handle */
    connector_file_offset_t CONST length_in_bytes;  /**< File length in bytes to truncate to */

} connector_file_system_truncate_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_read_t Data type used 
* for file system read callback 
* @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_read callback. 
*/
typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void * CONST handle;                    /**< Application defined file handle */
    void * CONST buffer;                    /**< A pointer to memory, where callback writes data */
    size_t CONST bytes_available;           /**< Size of a memory buffer */
    size_t bytes_used;                      /**< Number of bytes read from a file and copied to memory buffer */

} connector_file_system_read_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_close_t Data type used 
* for file system close callback 
* @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_close callback. 
*/
typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void * CONST handle;                    /**< Application defined file or directory handle */

} connector_file_system_close_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_opendir_t Data type used 
* for file system opendir callback 
* @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_opendir callback. 
*/
typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * CONST path;                /**< Directory path */
    void * handle;                          /**< Application defined directory handle */

} connector_file_system_opendir_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_hash_algorithm_t File system 
* hash algorithm 
* @{ 
*/
/**
* Hash algorithm gives different options for hash values returned in the file lisings.
*
* @see @ref connector_request_id_file_system_stat
* @see @ref connector_request_id_file_system_hash
*/
typedef enum
{
    connector_file_system_hash_none,       /**< Don't return hash value */
    connector_file_system_hash_best,       /**< Use best available algorithm */
    connector_file_system_hash_crc32,      /**< Use crc32 */
    connector_file_system_hash_md5         /**< Use md5 */
} connector_file_system_hash_algorithm_t;
/**
* @}
*/

/**
* @defgroup connector_file_system_hash_t Data type used 
* for file system hash callback 
* @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_hash callback. 
*/
typedef struct
{
    void * user_context;                                /**< Holds user context */
    void * errnum;                                      /**< Application defined error token */

    char const * CONST path;                             /**< File path */
    connector_file_system_hash_algorithm_t CONST hash_algorithm;/**< hash algorithm */
    void * CONST hash_value;                             /**< A pointer to memory, where callback writes hash value */
    size_t CONST bytes_requested;                        /**< Size of a memory buffer */

} connector_file_system_hash_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_statbuf_t File status data
* @{
*/
/**
* File status data structure is used to return the status of a direcory or a file, specified by the path.
* It is used in  
* @ref connector_request_id_file_system_stat and 
* @ref connector_request_id_file_system_stat_dir_entr callbacks.
*/
typedef struct
{
    uint32_t last_modified;                     /**< TODO: ASK to consider, decide on time_t, uint32! .  Last modified time for the entry (seconds since 1970). If not supported, use 0 */
    connector_file_offset_t file_size;          /**< File size in bytes */
    enum
    {
        connector_file_system_file_type_none,   /**< Is not a regular file or directory. */
        connector_file_system_file_type_is_dir, /**< Is a directory. */
        connector_file_system_file_type_is_reg  /**< Is a regular file. */
    } flags;                                    /**< Directory, regular, or neither */

} connector_file_system_statbuf_t;
/**
* @}
*/

/**
* @defgroup connector_file_system_stat_t Data type used 
* for file system status callback @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_stat callback, used 
* to get status for a path received from Etherios Device Cloud 
* for ls request. 
*/
typedef struct
{
    void * user_context;                        /**< Holds user context */
    void * errnum;                              /**< Application defined error token */

    char const * CONST path;                    /**< File path */
    connector_file_system_statbuf_t statbuf;    /**< File status data */

    struct
    {
    	connector_file_system_hash_algorithm_t CONST requested; /**< requested hash algorithm */
        connector_file_system_hash_algorithm_t actual;          /**< actual hash algorithm to use for the all regular files in the directory */
    } hash_algorithm;


} connector_file_system_stat_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_stat_dir_entry_t Data 
* type used for file system directory entry status callback 
* @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_stat_dir_entry callback, 
* used to get status for for each directory entry in 
* the directory path, received from Etherios Device Cloud for ls
* request. 
*/
typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * CONST path;                      /**< Full path */
    connector_file_system_statbuf_t statbuf;   /**< File status data */

} connector_file_system_stat_dir_entry_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_readdir_t Data type used 
* for file system readdir callback @{ 
*/
/**
* Data structure used in  
* connector_request_id_file_system_readdir callback
*/
typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void * CONST handle;                    /**< Application defined directory handle */
    char * CONST entry_name;                /**< A pointer to memory, where callback writes directory entry name */
    size_t CONST bytes_available;           /**< Size of a memory buffer for directory entry name */

} connector_file_system_readdir_t;
/**
* @}
*/


/**
* @defgroup connector_file_system_remove_t Data type used 
* for file system remove callback @{ 
*/
/**
* Data structure used in connector_request_id_file_system_remove 
* callback 
*/
typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * CONST path;                /**< File path */

} connector_file_system_remove_t;
/**
* @}
*/

/**
* @defgroup connector_file_system_get_error_t Data type 
* used for file system ret error data callback @{ 
*/
/**
* Data structure used in 
* connector_request_id_file_system_get_error callback, to get 
* error status and description for a previously recorder error 
* token. 
*/
typedef struct
{
    void * user_context;               /**< Holds user context*/
    void * CONST errnum;               /**< Application defined error token */

    void  * CONST buffer;              /**< A pointer to memory, where callback writes error description */
    size_t  CONST bytes_available;          /**< Size of a error description buffer */
    size_t  bytes_used;                /**< Number of error descriptio bytes */

    enum
    {
        connector_file_system_unspec_error,                 /**< Fatal unspecified error */
        connector_file_system_path_not_found,               /**< Path not found */
        connector_file_system_insufficient_storage_space,   /**< Insufficient storage space */
        connector_file_system_request_format_error,         /**< Request format error */
        connector_file_system_invalid_parameter,            /**< Invalid parameter */
        connector_file_system_out_of_memory,                /**< Out of memory */
        connector_file_system_permision_denied              /**< Permision denied */
    } error_status;                                         /**< Error status */

} connector_file_system_get_error_t;
/**
* @}
*/

/**
* @defgroup connector_file_system_session_error_t Data type 
* used for file system session error callback 
* @{ 
*/
/**
* Data structure used in 
* connector_request_id_file_system_session_error callback 
*/
typedef struct
{
    void * user_context;                            /**< Holds user context */

    connector_session_error_t CONST session_error;  /**	TODO: Needs a describe 	*/

} connector_file_system_session_error_t;
/**
* @}
*/

#endif
