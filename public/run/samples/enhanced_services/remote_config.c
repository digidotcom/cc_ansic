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
#include  "remote_config.h"

#include "connector_api.h"



#define CONST const 
#define FIRMWARE_TARGET_ZERO_VERSION  0x1000000

#define CONNECTOR_RCI_ERROR_BAD_COMMAND (connector_remote_all_strings+0)
#define CONNECTOR_RCI_ERROR_BAD_DESCRIPTOR (connector_remote_all_strings+12)
#define SETTING_SYSTEM_ERROR_INVALID_LENGTH (connector_remote_all_strings+30)
#define CONNECTOR_GLOBAL_ERROR_LOAD_FAIL (connector_remote_all_strings+45)
#define CONNECTOR_GLOBAL_ERROR_SAVE_FAIL (connector_remote_all_strings+55)
#define CONNECTOR_GLOBAL_ERROR_MEMORY_FAIL (connector_remote_all_strings+65)

char CONST connector_remote_all_strings[] = {
 11,'B','a','d',' ','c','o','m','m','a','n','d',
 17,'B','a','d',' ','c','o','n','f','i','g','u','r','a','t','i','o','n',
 14,'I','n','v','a','l','i','d',' ','L','e','n','g','t','h',
 9,'L','o','a','d',' ','f','a','i','l',
 9,'S','a','v','e',' ','f','a','i','l',
 19,'I','n','s','u','f','f','i','c','i','e','n','t',' ','m','e','m','o','r','y'
};

static char const * const connector_rci_errors[] = {
 CONNECTOR_RCI_ERROR_BAD_COMMAND, /*bad_command*/
 CONNECTOR_RCI_ERROR_BAD_DESCRIPTOR, /*bad_descriptor*/
 CONNECTOR_GLOBAL_ERROR_LOAD_FAIL, /*load_fail*/
 CONNECTOR_GLOBAL_ERROR_SAVE_FAIL, /*save_fail*/
 CONNECTOR_GLOBAL_ERROR_MEMORY_FAIL /*memory_fail*/
};

static connector_group_element_t CONST setting_system_elements[] = {
 {  /*description*/
   connector_element_access_read_write,
   connector_element_type_string
 },
 {  /*contact*/
   connector_element_access_read_write,
   connector_element_type_string
 },
 {  /*location*/
   connector_element_access_read_write,
   connector_element_type_string
 }
};

static char CONST * CONST setting_system_errors[] = {
 SETTING_SYSTEM_ERROR_INVALID_LENGTH /*invalid_length*/
};

static connector_group_element_t CONST setting_enhanced_services_elements[] = {
 {  /*path*/
   connector_element_access_read_write,
   connector_element_type_string
 },
 {  /*sampling_interval*/
   connector_element_access_read_write,
   connector_element_type_uint32
 },
 {  /*reporting_interval*/
   connector_element_access_read_write,
   connector_element_type_uint32
 }
};

static connector_group_t CONST connector_setting_groups[] = {
 {  /*system*/
   1 , /* instances */
   { asizeof(setting_system_elements),
     setting_system_elements
   },
   { asizeof(setting_system_errors),
     setting_system_errors
   }  /* errors*/
}
,
 {  /*enhanced_services*/
   8 , /* instances */
   { asizeof(setting_enhanced_services_elements),
     setting_enhanced_services_elements
   },
   { 0,
     NULL
   }  /* errors*/
}

};

static connector_group_element_t CONST state_gps_stats_elements[] = {
 {  /*latitude*/
   connector_element_access_read_only,
   connector_element_type_string
 },
 {  /*longitude*/
   connector_element_access_read_only,
   connector_element_type_string
 }
};

static connector_group_t CONST connector_state_groups[] = {
 {  /*gps_stats*/
   1 , /* instances */
   { asizeof(state_gps_stats_elements),
     state_gps_stats_elements
   },
   { 0,
     NULL
   }  /* errors*/
}

};

static connector_remote_group_table_t CONST connector_group_table[] = {
 { connector_setting_groups,
   asizeof(connector_setting_groups)
 },
 { connector_state_groups,
   asizeof(connector_state_groups)
 }
};


connector_remote_config_data_t rci_desc_data = {
    connector_group_table,
    connector_rci_errors,
    connector_global_error_COUNT,
    FIRMWARE_TARGET_ZERO_VERSION,
    0x30000DB,
    "Linux Application"
};
