/*
 * Copyright (c) 2012 Digi International Inc.,
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
 *  @brief iDigi connector remote configuration definitions and structures
 *
 */
#ifndef _CONNECTOR_REMOTE_H
#define _CONNECTOR_REMOTE_H

#include "connector_types.h"

/**
 * @defgroup connector_remote_action_t iDigi connector remote configuration action types
 * @{
 */
/**
 * Remote request action types
 */
typedef enum {
    connector_remote_action_set,    /**< Set remote configuration */
    connector_remote_action_query   /**< Query remote configuration */
} connector_remote_action_t;
/**
 * @}
 */

/**
 * @defgroup connector_remote_group_type_t iDigi connector remote configuration types
 * @{
 */
/**
 * Remote request group types.
 */
typedef enum {
    connector_remote_group_setting, /**< Setting configuration */
    connector_remote_group_state    /**< State configuration */
} connector_remote_group_type_t;

/**
* @}
*/


/**
* @defgroup connector_element_access_t iDigi connector remote configuration access types
* @{
*/
/**
* Remote Configuration Element Access types
*/
typedef enum {
    connector_element_access_read_only,     /**< Read only */
    connector_element_access_write_only,    /**< Write only */
    connector_element_access_read_write     /**< Read and write */
} connector_element_access_t;
/**
* @}
*/

typedef struct {
    char const * name;
    connector_element_access_t access;
    connector_element_value_type_t type;
    connector_element_value_limit_t const * const value_limit;
} connector_group_element_t;

typedef struct {
    char const * name;
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
* @defgroup connector_remote_group_request_t iDigi connector remote configuration request structure
* @{
*/
/**
* Remote configuration request
*/
typedef struct {
    connector_remote_action_t action;   /**< @htmlonly <ul><li> @endhtmlonly @ref connector_remote_action_set @htmlonly for setting remote configuration or</li>
                                         <li> @endhtmlonly @ref connector_remote_action_query @htmlonly for querying remote configuration </li></ul> @endhtmlonly */
    connector_remote_group_t group;     /**< Group configuration to be accessed */
    connector_remote_element_t element; /**< Element of the group configuration */
} connector_remote_group_request_t;
/**
* @}
*/

/**
* @defgroup connector_remote_element_response_t iDigi connector remote configuration element response union
* @{
*/
/**
* Remote configuration element response
*/
typedef union {
    char const * error_hint;    /**< Callback returns a pointer to a constant null-terminated string which will be sent
                                     to the iDigi connector Device Cloud if error is encountered.
                                     Note: this string cannot be altered until next callback call.
                                  */
    connector_element_value_t * element_value; /**< Pointer to memory where callback write the element value */
} connector_remote_element_response_t;
/**
* @}
*/

/**
* @defgroup connector_remote_group_response_t iDigi connector remote configuration response structure
* @{
*/
/**
* Remote configuration response
*/
typedef struct {
    void * user_context;        /**< Pointer to callback's context returned from previous callback call.
                                     Callback may write its own context which will be passed back to subsequential callback.*/
    unsigned int error_id;      /**< Callback writes error enumeration value if error is encountered.
                                     Error description is sent if it's provided for the given error_id. */
    connector_remote_element_response_t element_data;  /**< Element response data */
} connector_remote_group_response_t;
/**
* @}
*/

typedef struct {
    connector_group_t const * groups;
    size_t count;
} connector_group_table_t;

#endif /* connector_element_H_ */
