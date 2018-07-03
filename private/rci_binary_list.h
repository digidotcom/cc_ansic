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

#define CURRENT_LIST_INDEX_VARIABLE(rci)		(CURRENT_LIST_VARIABLE(rci).specifier.value.index)
#define get_current_list_index(rci)				(CURRENT_LIST_INDEX_VARIABLE(rci))
#define have_current_list_index(rci)			(get_current_list_index(rci) != INVALID_INDEX)
#define increment_current_list_index(rci)		(CURRENT_LIST_INDEX_VARIABLE(rci)++)
#define set_current_list_index(rci, value)		(CURRENT_LIST_INDEX_VARIABLE(rci) = (value))
#define invalidate_current_list_index(rci)		(set_current_list_index(rci, INVALID_INDEX))
