/** TODO: need a reference to off_t to allow the user to override
 * Needs the number of bits as well -- does this go into
 * connector_types.h?
 *
 * TODO: This instruction needs to be documented in the Getting Started process
 *
 * TODO: Add 64 bit types to connector_types.h
 * */

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



/** TODO: map the local file system flags */
#define	CONNECTOR_FILE_O_RDONLY	0
#define	CONNECTOR_FILE_O_WRONLY	1
#define	CONNECTOR_FILE_O_RDWR	2
#define	CONNECTOR_FILE_O_APPEND	0x0008
#define	CONNECTOR_FILE_O_CREAT	0x0200
#define	CONNECTOR_FILE_O_TRUNC	0x0400

typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const *  path;                     /**< File path */
    int  oflag;                             /**< bitwise-inclusive OR of @ref connector_file_system_open_flag_t flags */
    void * handle;                          /**< TODO: Don't check handle in private code!!!  Application defined file handle */

} connector_file_system_open_data_t;


typedef struct
{
   void * user_context;                    /**< Holds user context */
   void * errnum;                          /**< Application defined error token */

   void * handle;                          /**< Application defined file handle */
   off_t  requested_offset;                /**< Requested file offset */
   off_t  resulting_offset;                /**< Resulting file position */
   enum
   {
        connector_file_system_seek_set,
        connector_file_system_seek_cur,
        connector_file_system_seek_end

   } origin;                                    /**< File seek origin */

} connector_file_system_lseek_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void *  handle;                         /**< Application defined file handle */
    void const * buffer;                    /**< A pointer to data to be written to a file */
    size_t  bytes_available;                /**< Number of bytes to write */
    size_t  bytes_used;                     /**< Number of bytes written to a file */

} connector_file_system_write_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void * handle;                          /**< Application defined file handle */
    off_t length_in_bytes;                  /**< File length in bytes to truncate to */

} connector_file_system_truncate_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void *  handle;                         /**< Application defined file handle */
    void *  buffer;                         /**< A pointer to memory, where callback writes data */
    size_t  bytes_available;                /**< Size of a memory buffer */
    size_t  bytes_used;                     /**< Number of bytes read from a file and copied to memory buffer */

} connector_file_system_read_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void * handle;                          /**< Application defined file or directory handle */

} connector_file_close_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * path;                      /**< Directory path */
    void * handle;                          /**< Application defined directory handle */

} connector_file_system_opendir_data_t;


typedef enum
{
    connector_file_system_hash_none,       /**< Don't return hash value */
    connector_file_system_hash_best,       /**< Use best available algorithm */
    connector_file_system_hash_crc32,      /**< Use crc32 */
    connector_file_system_hash_md5         /**< Use md5 */
} connector_file_system_hash_algorithm_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * path;                      /**< File path */
    cc_file_hash_algorithm_t  hash_algorithm;     /**< hash algorithm */
    void const * hash_value;                /**< A pointer to memory, where callback writes hash value */
    size_t bytes_available;                 /**< Size of a memory buffer */
    size_t bytes_used;                      /**< Hash value size in bytes */

} connector_file_system_hash_data_t;


typedef struct
{
    uint32_t last_modified;                       /**< TODO: ASK to consider, decide on time_t, uint32! .  Last modified time for the entry (seconds since 1970). If not supported, use 0 */
    off_t file_size;                            /**< File size in bytes */
    enum
    {
        connector_file_system_file_type_none,
        connector_file_system_file_type_is_dir,
        connector_file_system_file_type_is_reg
    } flags;                                    /**< directory, regular, or neither */

} connector_file_system_stat_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * path;                      /**< File path */
    connector_file_system_stat_t statbuf;   /**< File status data */

    struct
    {
    	connector_file_system_hash_algorithm_t requested;  /**< requested hash algorithm */
        connector_file_system_hash_algorithm_t actual;     /**< actual hash algorithm to use for the all regular files in the directory */
    } hash_algorithm;


} connector_file_system_stat_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * path;                      /**< Full path */
    connector_file_system_stat_t statbuf;   /**< File status data */

} connector_file_system_stat_dir_entry_data_t;



typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    void *  handle;                         /**< Application defined directory handle */
    char * entry_name;                      /**< A pointer to memory, where callback writes directory entry name */
    size_t bytes_available;                 /**< Size of a memory buffer for directory entry name */

} connector_file_system_readdir_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */
    void * errnum;                          /**< Application defined error token */

    char const * path;                      /**< File path */

} connector_file_system_remove_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context*/
    void * errnum;                          /**< Application defined error token */

    void  * buffer;                    /**< A pointer to memory, where callback writes error description */
    size_t  bytes_abailable;                /**< Size of a error description buffer */
    size_t  bytes_used;                     /**< Number of error descriptio bytes */
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

} connector_file_system_get_error_data_t;


typedef struct
{
    void * user_context;                    /**< Holds user context */

    connector_msg_error_t   session_error;      /**	TODO: Needs a describe 	*/

} connector_file_system_session_error_data_t;


