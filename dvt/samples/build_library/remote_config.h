/*
 * This is an auto-generated file - DO NOT EDIT!
 * This is a public header file generated by ConfigGenerator tool.
 * This file was generated on: 2012/12/14 15:26:37
 * The command line arguments were: "-type=global_header "
 * The version of ConfigGenerator tool was: 1.1.0.0 */

#ifndef remote_config_h
#define remote_config_h

#include  "connector_api.h"

#define connector_remote_config_configurations    (connector_remote_config_request_t)-1

#define RCI_PARSER_USES_ERROR_DESCRIPTIONS
#define RCI_PARSER_USES_STRING
#define RCI_PARSER_USES_MULTILINE_STRING
#define RCI_PARSER_USES_PASSWORD
#define RCI_PARSER_USES_INT32
#define RCI_PARSER_USES_UINT32
#define RCI_PARSER_USES_HEX32
#define RCI_PARSER_USES_0X_HEX32
#define RCI_PARSER_USES_FLOAT
#define RCI_PARSER_USES_ENUM
#define RCI_PARSER_USES_ON_OFF
#define RCI_PARSER_USES_BOOLEAN
#define RCI_PARSER_USES_IPV4
#define RCI_PARSER_USES_FQDNV4
#define RCI_PARSER_USES_FQDNV6
#define RCI_PARSER_USES_DATETIME
#define RCI_PARSER_USES_UNSIGNED_INTEGER
#define RCI_PARSER_USES_STRINGS


#include "float.h"

typedef enum {
    connector_off,
    connector_on
} connector_on_off_t;

typedef enum {
    connector_element_type_string = 1,
    connector_element_type_multiline_string,
    connector_element_type_password,
    connector_element_type_int32,
    connector_element_type_uint32,
    connector_element_type_hex32,
    connector_element_type_0x_hex32,
    connector_element_type_float,
    connector_element_type_enum,
    connector_element_type_on_off = 11,
    connector_element_type_boolean,
    connector_element_type_ipv4,
    connector_element_type_fqdnv4,
    connector_element_type_fqdnv6,
    connector_element_type_datetime = 22
} connector_element_value_type_t;

typedef struct {
    size_t min_length_in_bytes;
    size_t max_length_in_bytes;
} connector_element_value_string_t;

typedef struct {
   int32_t min_value;
   int32_t max_value;
} connector_element_value_signed_integer_t;

typedef struct {
   uint32_t min_value;
   uint32_t max_value;
} connector_element_value_unsigned_integer_t;

typedef struct {
    float min_value;
    float max_value;
} connector_element_value_float_t;

typedef struct {
    size_t count;
} connector_element_value_enum_t;


typedef union {
    char const * string_value;
    int32_t signed_integer_value;
    uint32_t unsigned_integer_value;
    float float_value;
    unsigned int enum_value;
    connector_on_off_t  on_off_value;
    connector_bool_t  boolean_value;
} connector_element_value_t;


typedef void connector_element_value_limit_t;


typedef struct {
    connector_element_access_t access;
    connector_element_value_type_t type;
    connector_element_value_limit_t const * const value_limit;
} connector_group_element_t;

typedef struct {
    size_t instances;
    struct {
        size_t count;
        connector_group_element_t const * const data;
    } elements;

    struct {
        size_t count;       /**< Number of errors in a remote configuration */
        char const * const * description;
    } errors;
} connector_group_t;


/**
* @defgroup connector_remote_group_t iDigi connector remote configuration group
* @{
*/
/**
* Remote configuration group
*/
typedef struct {
    connector_remote_group_type_t type; /**< Type of the remote configuration group:
                                         @htmlonly
                                         <ul>
                                         <li>@endhtmlonly @ref connector_remote_group_setting @htmlonly for setting configuration</li>
                                         <li>@endhtmlonly @ref connector_remote_group_state @htmlonly for state configuration</li>
                                         </ul>
                                         @endhtmlonly
                                       */
    unsigned int id;                /**< An enumeration value of the remote configuration group */
    unsigned int index;             /**< Index of the remote configuration group */
} connector_remote_group_t;
/**
* @}
*/

/**
* @defgroup connector_remote_element_t iDigi connector remote configuration element
* @{
*/
/**
* Remote configuration element
*/
typedef struct {
    unsigned int id;                /**< An enumeration value of the element */
    connector_element_value_type_t type;/**< Value type of the element */
    connector_element_value_t * value;  /**< Pointer to memory which contains the element value to be
                                         set for @ref connector_remote_action_set action */
} connector_remote_element_t;
/**
* @}
*/

/**
* @defgroup connector_remote_config_data_t Remote Configuration Data Structure
* @{
*/
/**
* Remote configuration data structure which is used for the following remote configuration callbacks:
*   -# connector_request_id_remote_config_session_start
*   -# connector_request_id_remote_config_action_start
*   -# connector_request_id_remote_config_group_start
*   -# connector_request_id_remote_config_group_process
*   -# connector_request_id_remote_config_group_end
*   -# connector_request_id_remote_config_action_end
*   -# connector_request_id_remote_config_session_end
*
*/
typedef struct {
    void * user_context;                    /**< Pointer to callback's context returned from previous callback call.
                                                 Callback may write its own context which will be passed back to subsequential callback.*/

    connector_remote_action_t action;       /**< @htmlonly <ul><li> @endhtmlonly @ref connector_remote_action_set @htmlonly for setting remote configuration or</li>
                                                 <li> @endhtmlonly @ref connector_remote_action_query @htmlonly for querying remote configuration </li></ul> @endhtmlonly */
    struct {
        connector_remote_group_t group;     /**< Group configuration to be accessed */
        connector_remote_element_t element; /**< Element of the group configuration */
    } request;                              /**< Callback request data */

    struct {
        unsigned int error_id;                  /**< Callback writes error enumeration value if error is encountered.
                                                     Error description is sent if it's provided for the given error_id. */
        union {
            char const * error_hint;            /**< Callback returns a pointer to a constant null-terminated string which will be sent
                                                     to the iDigi connector Device Cloud if error is encountered.
                                                     Note: this string cannot be altered until next callback call. */
            connector_element_value_t * value;  /**< Pointer to memory where callback write the element value */
        } element;                              /**< Callback's response element data or error hint */
    } response;                                 /**< Callback response data */

} connector_remote_config_data_t;
/**
* @}
*/

typedef struct {
    connector_remote_group_t const * groups;
    size_t count;
} connector_remote_group_table_t;


typedef enum {
 connector_rci_error_OFFSET = 1,
 connector_rci_error_bad_command =  connector_rci_error_OFFSET,
 connector_rci_error_bad_descriptor,
 connector_rci_error_COUNT
} connector_rci_error_id_t;


typedef struct {
 connector_group_table_t const * group_table;
 char const * const * error_table;
 unsigned int global_error_count;
 uint32_t firmware_target_zero_version;
} connector_remote_config_t;

#endif /* remote_config_h */
