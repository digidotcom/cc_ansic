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

#ifndef _CONNECTOR_API_REMOTE_H
#define _CONNECTOR_API_REMOTE_H

#if (defined CONNECTOR_RCI_SERVICE)

/**
 * @defgroup rci_query_command_attribute_source_t rci query setting command source attribute value
 * @{
 */
/**
 * rci query setting command source attribute value.
 */
typedef enum {
  rci_query_command_attribute_source_current, /**< Current settings */
  rci_query_command_attribute_source_stored,  /**< Settings stored in flash */
  rci_query_command_attribute_source_defaults /**< Device defaults */
} rci_query_command_attribute_source_t;
/**
* @}
*/

/**
 * @defgroup rci_query_command_attribute_compare_to_t rci query setting command compare_to attribute value
 * @{
 */
/**
 * rci query setting command compare_to attribute value.
 */
typedef enum {
  rci_query_command_attribute_compare_to_current,  /**< Current settings */
  rci_query_command_attribute_compare_to_stored,   /**< Settings stored in flash */
  rci_query_command_attribute_compare_to_defaults, /**< Device defaults */
  rci_query_command_attribute_compare_to_none      /**< Return all settings */
} rci_query_command_attribute_compare_to_t;
/**
* @}
*/

/**
 * @defgroup connector_remote_group_type_t Cloud Connector remote configuration types
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
* @defgroup connector_element_access_t Cloud Connector remote configuration access types
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

typedef struct connector_remote_config_data {
    struct connector_remote_group_table const * group_table;
    char const * const * error_table;
    unsigned int global_error_count;
    uint32_t firmware_target_zero_version;
    uint32_t vendor_id;
    char const * device_type;
} connector_remote_config_data_t;

#include "remote_config.h"

#endif

#if !defined _CONNECTOR_API_H
#error  "Illegal inclusion of connector_api_remote.h. You should only include connector_api.h in user code."
#endif

#else
#error  "Illegal inclusion of connector_api_remote.h. You should only include connector_api.h in user code."
#endif
