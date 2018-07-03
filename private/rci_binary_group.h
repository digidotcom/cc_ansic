/*
 * Copyright (c) 2014 Digi International Inc.
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


#define GROUP_ID_VARIABLE(rci)          ((rci)->shared.group.id)
#define set_group_id(rci, value)        (GROUP_ID_VARIABLE(rci) = (value))
#define get_group_id(rci)               (GROUP_ID_VARIABLE(rci))
#define invalidate_group_id(rci)        set_group_id(rci, INVALID_ID)
#define have_group_id(rci)              (get_group_id(rci) != INVALID_ID)
#define increment_group_id(rci)         (GROUP_ID_VARIABLE(rci)++)

#define GROUP_INDEX_VARIABLE(rci)       ((rci)->shared.group.specifier.value.index)
#define set_group_index(rci, value)     (GROUP_INDEX_VARIABLE(rci) = (value))
#define get_group_index(rci)            (GROUP_INDEX_VARIABLE(rci))
#define increment_group_index(rci)      (GROUP_INDEX_VARIABLE(rci)++)
#define invalidate_group_index(rci)     set_group_index(rci, INVALID_INDEX)
#define have_group_index(rci)           (get_group_index(rci) != INVALID_INDEX)

static connector_group_t const * get_current_group(rci_t const * const rci)
{
    connector_remote_config_data_t const * const rci_data = rci->service_data->connector_ptr->rci_data;
    connector_remote_group_table_t const * const table = rci_data->group_table + rci->shared.callback_data.group.type;
    unsigned int const group_id = get_group_id(rci);

    ASSERT(have_group_id(rci));
    ASSERT(group_id < table->count);

    return (table->groups + group_id);
}


