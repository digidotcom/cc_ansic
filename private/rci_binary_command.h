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

#if (defined RCI_LEGACY_COMMANDS)

#define RCI_DO_COMMAND_ATTRIBUTE_COUNT 1
#define RCI_DO_COMMAND_TARGET_BIN_ID 0     /* 'target' attribute is bin_id=0 in the uploaded descriptor */

connector_callback_status_t app_process_do_command(connector_remote_config_t * const remote_config, char const * const target, char const * const request_payload, char const * * response_payload);
connector_callback_status_t app_process_set_factory_default(void);
#endif


