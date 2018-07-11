/*
 * Copyright (c) 2018 Digi International Inc.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */
#include "connector_config.h"
#include "connector_api.h"
#include "platform.h"
#include "remote_config_cb.h"

#if !defined CONNECTOR_RCI_SERVICE
#error "Must define CONNECTOR_RCI_SERVICE in connector_config.h to run this sample"
#endif

connector_callback_status_t app_remote_config_handler(connector_request_id_remote_config_t const request_id,
                                                      void * const data)
{
	connector_callback_status_t status = connector_callback_continue;
	connector_remote_config_t * config = data;

	switch (request_id)
	{
		case connector_request_id_remote_config_group_instances_lock:
			printf("Callback: Lock group collection\n");
			break;
		case connector_request_id_remote_config_group_instances_unlock:
			printf("Callback: Unlock group collection\n");
			break;
		case connector_request_id_remote_config_group_start:
			printf("Callback: Open group %s[%u]\n", config->group.name, config->group.index - 1);
			break;
		case connector_request_id_remote_config_group_end:
			printf("Callback: Close group %s[%u]\n", config->group.name, config->group.index - 1);
			break;
#ifdef RCI_PARSER_USES_LIST
		case connector_request_id_remote_config_list_start:
			printf("Callback: Open list %s[%u]", config->group.name, config->group.index - 1);
			{
				int i;
				for (i = 0; i < config->list.depth; i++)
				{
					printf(".%s[%u]", config->list.level[i].name, config->list.level[i].index - 1);
				}
			}
			printf("\n");
			break;
		case connector_request_id_remote_config_list_end:
			printf("Callback: Close list %s[%u]", config->group.name, config->group.index - 1);
			{
				int i;
				for (i = 0; i < config->list.depth; i++)
				{
					printf(".%s[%u]", config->list.level[i].name, config->list.level[i].index - 1);
				}
			}
			printf("\n");
			break;
#endif
		case connector_request_id_remote_config_element_process:
			switch (config->element.type)
			{
				case connector_element_type_string:
				case connector_element_type_multiline_string:
				case connector_element_type_password:
				case connector_element_type_ipv4:
				case connector_element_type_fqdnv4:
				case connector_element_type_datetime:
					config->response.element_value->string_value = "0.0.0.0";
					break;
				case connector_element_type_int32:
					config->response.element_value->signed_integer_value = 0;
					break;
				case connector_element_type_uint32:
				case connector_element_type_0x_hex32:
					config->response.element_value->unsigned_integer_value = 0;
					break;
				case connector_element_type_enum:
					config->response.element_value->enum_value = 0;
					break;
				case connector_element_type_on_off:
					config->response.element_value->on_off_value = connector_off;
					break;
				case connector_element_type_boolean:
					config->response.element_value->boolean_value = connector_false;
					break;
				default:
					printf("Query for unknown type: %u\n", config->element.type);
					break;
			}
			printf("Callback: Query for %s[%u].", config->group.name, config->group.index - 1);
#ifdef RCI_PARSER_USES_LIST
			{
				int i;
				for (i = 0; i < config->list.depth; i++)
				{
					printf("%s[%u].", config->list.level[i].name, config->list.level[i].index - 1);
				}
			}
#endif
			printf("%s\n", config->element.name);
			break;
	}

    return status;
}

