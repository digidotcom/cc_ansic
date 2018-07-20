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

#define LIST_DEPTH_VARIABLE(rci)		((rci)->shared.list.depth)
#define get_list_depth(rci)				(LIST_DEPTH_VARIABLE(rci))
#define set_list_depth(rci, value)		(LIST_DEPTH_VARIABLE(rci) = (value))
#define increment_list_depth(rci)		(LIST_DEPTH_VARIABLE(rci)++)
#define decrement_list_depth(rci)		(LIST_DEPTH_VARIABLE(rci)--)

#define LIST_QUERY_DEPTH_VARIABLE(rci)		((rci)->shared.list.query_depth)
#define get_query_depth(rci)				(LIST_QUERY_DEPTH_VARIABLE(rci))
#define set_query_depth(rci, value)			(LIST_QUERY_DEPTH_VARIABLE(rci) = (value))

#define CURRENT_LIST_VARIABLE(rci)	((rci)->shared.list.level[get_list_depth(rci) - 1])

#define CURRENT_LIST_ID_VARIABLE(rci)		(CURRENT_LIST_VARIABLE(rci).id)
#define get_current_list_id(rci)			(CURRENT_LIST_ID_VARIABLE(rci))
#define have_current_list_id(rci)			(get_current_list_id(rci) != INVALID_ID)
#define set_current_list_id(rci, value)		(CURRENT_LIST_ID_VARIABLE(rci) = (value))
#define invalidate_current_list_id(rci)		(set_current_list_id(rci, INVALID_ID))

#define CURRENT_LIST_LOCK_VARIABLE(rci)		(CURRENT_LIST_VARIABLE(rci).lock)
#define get_current_list_lock(rci)			(CURRENT_LIST_LOCK_VARIABLE(rci))
#define have_current_list_lock(rci)			(get_current_list_lock(rci) != INVALID_ID)
#define set_current_list_lock(rci, value)	(CURRENT_LIST_LOCK_VARIABLE(rci) = (value))
#define invalidate_current_list_lock(rci)	(set_current_list_lock(rci, INVALID_ID))

#define invalidate_list_lock(rci, index)	((rci)->shared.list.level[(index)].lock = INVALID_ID)

#define CURRENT_LIST_COUNT_VARIABLE(rci)	(CURRENT_LIST_VARIABLE(rci).info.keys.count)
#define get_current_list_count(rci)			(CURRENT_LIST_COUNT_VARIABLE(rci))
#define set_current_list_count(rci, value)	(CURRENT_LIST_COUNT_VARIABLE(rci) = (value))

#define CURRENT_LIST_INSTANCE_VARIABLE(rci)		(CURRENT_LIST_VARIABLE(rci).info.instance)
#define get_current_list_instance(rci)			(CURRENT_LIST_INSTANCE_VARIABLE(rci))
#define have_current_list_instance(rci)			(get_current_list_instance(rci) != INVALID_INDEX)
#define increment_current_list_instance(rci)	(CURRENT_LIST_INSTANCE_VARIABLE(rci)++)
#define set_current_list_instance(rci, value)	(CURRENT_LIST_INSTANCE_VARIABLE(rci) = (value))
#define invalidate_current_list_instance(rci)	(set_current_list_instance(rci, INVALID_INDEX))

#define set_current_list_key_list(rci, value)	(CURRENT_LIST_VARIABLE(rci).info.keys.list = (value))

#define get_current_list_name(rci)			(get_current_list_instance(rci) == 0 ? \
											 CURRENT_LIST_VARIABLE(rci).info.keys.key_store : \
											 CURRENT_LIST_VARIABLE(rci).info.keys.list[get_current_list_instance(rci) - 1])

#define get_current_list_collection_type(rci) (get_current_collection_info((rci))->collection_type)

static connector_bool_t have_collection_instances(rci_t const * const rci)
{
	if (!have_group_id(rci)) 
	{
		return connector_false;
	}
	else
	{
		int i;
		for (i = 0; i < get_list_depth(rci); i++)
		{
			if (rci->shared.list.level[i].info.instance == INVALID_INDEX) return connector_false;
		}
	}

	return connector_true;
}

static connector_collection_t const * get_current_collection_info(rci_t const * const rci)
{
	ASSERT(have_group_id(rci));
	{
		connector_group_t const * const group = get_current_group(rci);
		connector_collection_t const * info =  &group->collection;

#if (defined RCI_PARSER_USES_LIST)
		{
			int i;
			for (i = 0; i < get_list_depth(rci); i++)
			{
				connector_item_t const * list = info->item.data + rci->shared.list.level[i].id;
				ASSERT(list->type == connector_element_type_list);
				info = list->data.collection;
			}
		}
#endif

		return info;
	}
}

static connector_bool_t current_list_is_dynamic(rci_t const * const rci)
{
	connector_collection_type_t collection_type = get_current_list_collection_type(rci);

	return (collection_type == connector_collection_type_variable_array || collection_type == connector_collection_type_variable_dictionary) ? connector_true : connector_false;
}
