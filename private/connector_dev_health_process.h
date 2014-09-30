/*
 * Copyright (c) 2014 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

#define CONNECTOR_DEVICE_HEALTH_REALLOC_SIZE       256

typedef struct dev_health_info dev_health_info_t;

typedef union {
    int32_t int32;
    uint64_t uint64;
    float flt;
    char const * string;
} dev_health_item_value_t;

STATIC char const csv_header[] = "#DATA,TIMESTAMP,STREAMTYPE,STREAMID\n";

STATIC connector_status_t dev_health_reallocate_csv_data(connector_data_t * const connector_ptr)
{
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    unsigned int const old_size = dev_health_info->csv.total_size;
    unsigned int const additional_size = CONNECTOR_DEVICE_HEALTH_REALLOC_SIZE;
    unsigned int const new_size = old_size + additional_size;
    void * const old_ptr = dev_health_info->csv.data;
    connector_status_t const status = realloc_data(connector_ptr, old_size, new_size, (void * *) &dev_health_info->csv.data);

    if (dev_health_info->csv.data == NULL)
    {
        connector_debug_line("Error when reallocating CSV buffer from %d to %d", dev_health_info->csv.total_size, new_size);
        dev_health_info->csv.data = old_ptr;
        goto done;
    }

    dev_health_info->csv.total_size = new_size;
    dev_health_info->csv.free_bytes += additional_size;

done:
    return status;
}

STATIC connector_status_t add_csv_header(connector_data_t * const connector_ptr)
{
    connector_status_t status = connector_working;
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;

    if (dev_health_info->csv.free_bytes < sizeof csv_header)
    {
        status = dev_health_reallocate_csv_data(connector_ptr);
        if (status != connector_working)
        {
            connector_debug_line("Realloc for CSV failed, header NOT added");
            goto done;
        }
    }

    strcat(dev_health_info->csv.data, csv_header);
    dev_health_info->csv.free_bytes -= sizeof csv_header;
done:
    return status;
}

STATIC void dev_health_reset_csv_data(dev_health_info_t * const dev_health_info)
{
    dev_health_info->csv.data_points_count = 0;
    dev_health_info->csv.data[0] = '\0';
    dev_health_info->csv.free_bytes = dev_health_info->csv.total_size;
}

STATIC connector_status_t dev_health_allocate_csv_data(connector_data_t * const connector_ptr)
{
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    connector_status_t status = connector_working;
    unsigned int const total_bytes = CONNECTOR_DEVICE_HEALTH_REALLOC_SIZE;
    void * allocated_memory;

    status = malloc_data(connector_ptr, total_bytes, &allocated_memory);
    ASSERT_GOTO(status == connector_working, done);

    if (allocated_memory == NULL)
    {
        connector_debug_line("Error while allocating memory for CSV data");
        status = connector_no_resource;
        goto done;
    }
    dev_health_info->csv.data = allocated_memory;
    dev_health_info->csv.total_size = total_bytes;
    dev_health_reset_csv_data(dev_health_info);

done:
    return status;
}

STATIC connector_status_t dev_health_teardown_csv_data(connector_data_t * const connector_ptr)
{
    connector_status_t status;

    status = free_data(connector_ptr, connector_ptr->dev_health.info.csv.data);
    if (status != connector_working)
    {
        ASSERT(status == connector_working);
        goto done;
    }

    connector_ptr->dev_health.info.csv.data = NULL;
done:
    return status;
}

STATIC void process_csv_data(char * const csv, dev_health_item_value_t const * const value, dev_health_value_type_t const type)
{
    switch (type)
    {
        case DEV_HEALTH_TYPE_INT32:
        {
            sprintf(csv, "%" PRId32, value->int32);
            break;
        }
        case DEV_HEALTH_TYPE_UINT64:
        {
            sprintf(csv, "%" PRIu64, value->uint64);
            break;
        }
        case DEV_HEALTH_TYPE_FLOAT:
        {
            sprintf(csv, "%f", value->flt);
            break;
        }
        case DEV_HEALTH_TYPE_STRING:
        case DEV_HEALTH_TYPE_JSON:
        case DEV_HEALTH_TYPE_GEOJSON:
        {
            connector_bool_t const needs_quotes = string_needs_quotes(value->string);
            unsigned int const temp_csv_size = CONNECTOR_DEVICE_HEALTH_REALLOC_SIZE;
            dp_process_string(value->string, csv, temp_csv_size, NULL, needs_quotes, connector_true);
            break;
        }
        case DEV_HEALTH_TYPE_NONE:
        {
            connector_debug_line("Fatal error, type not set");
            break;
        }
    }
}

STATIC void process_csv_timestamp(char * const csv)
{
    uint32_t timestamp = cc_dev_health_get_posix_time();

    sprintf(csv, "%s,%" PRIu32 "000", csv, timestamp); /* Timestamp is in milliseconds */
}

STATIC void process_csv_stream_type(char * const csv, dev_health_value_type_t const type)
{
    char const * stream_type_string = NULL;

    switch (type)
    {
        case DEV_HEALTH_TYPE_INT32:
            stream_type_string = ",INTEGER";
            break;
        case DEV_HEALTH_TYPE_UINT64:
            stream_type_string = ",LONG";
            break;
        case DEV_HEALTH_TYPE_FLOAT:
            stream_type_string = ",FLOAT";
            break;
        case DEV_HEALTH_TYPE_STRING:
            stream_type_string = ",STRING";
            break;
        case DEV_HEALTH_TYPE_JSON:
            stream_type_string = ",JSON";
            break;
        case DEV_HEALTH_TYPE_GEOJSON:
            stream_type_string = ",GEOJSON";
            break;
        case DEV_HEALTH_TYPE_NONE:
            ASSERT(type != DEV_HEALTH_TYPE_NONE);
            break;
    }

    strcat(csv, stream_type_string);
}

STATIC void process_csv_stream_id(char * const csv, char const * const stream_id)
{
    sprintf(csv, "%s,metrics/%s\n", csv, stream_id);
}

#define MAX_DATA_POINTS_PER_REQUEST 250

STATIC void add_item_to_csv(connector_data_t * const connector_ptr, dev_health_item_value_t const * const value, dev_health_value_type_t const type)
{
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    char temp_csv[CONNECTOR_DEVICE_HEALTH_REALLOC_SIZE];
    unsigned int temp_csv_strlen;
    char * const stream_id = dev_health_info->stream_id.string;
    connector_status_t status;

    if (dev_health_info->csv.data_points_count % MAX_DATA_POINTS_PER_REQUEST == 0)
    {
        status = add_csv_header(connector_ptr);
        if (status != connector_working)
        {
            goto done;
        }
    }

    process_csv_data(temp_csv, value, type);
    process_csv_timestamp(temp_csv);
    process_csv_stream_type(temp_csv, type);
    process_csv_stream_id(temp_csv, stream_id);

    temp_csv_strlen = strlen(temp_csv);

    if (temp_csv_strlen + sizeof "" > dev_health_info->csv.free_bytes)
    {
        status = dev_health_reallocate_csv_data(connector_ptr);
        if (status != connector_working)
        {
            connector_debug_line("Realloc for CSV failed, %s sample NOT added", stream_id);
            goto done;
        }
    }

    dev_health_info->csv.free_bytes -= temp_csv_strlen;
    dev_health_info->csv.data_points_count += 1;
    strcat(dev_health_info->csv.data, temp_csv);
done:
    return;
}

STATIC void dev_health_process_item(connector_data_t * const connector_ptr, dev_health_item_t const * const element, unsigned int const upper_index, unsigned int const lower_index)
{
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    dev_health_query_fn_t const function = element->getter;
    dev_health_value_type_t const type = element->type;
    dev_health_item_value_t value;
    connector_bool_t item_present = connector_false;
    connector_indexes_t indexes;

    indexes.upper = upper_index;
    indexes.lower = lower_index;

    ASSERT(function != NULL);
    switch (type)
    {
        case DEV_HEALTH_TYPE_INT32:
        {
            item_present = function(&indexes, &value.int32);
            break;
        }
        case DEV_HEALTH_TYPE_UINT64:
        {
            item_present = function(&indexes, &value.uint64);
            break;
        }
        case DEV_HEALTH_TYPE_FLOAT:
        {
            item_present = function(&indexes, &value.flt);
            break;
        }
        case DEV_HEALTH_TYPE_STRING:
        case DEV_HEALTH_TYPE_JSON:
        case DEV_HEALTH_TYPE_GEOJSON:
        {
            value.string = NULL;
            item_present = function(&indexes, &value.string);
            break;
        }
        case DEV_HEALTH_TYPE_NONE:
        {
            connector_debug_line("Fatal error, type not set");
            ASSERT(type != DEV_HEALTH_TYPE_NONE);
            break;
        }
    }

    if (item_present)
    {
        char * const stream_id = dev_health_info->stream_id.string;

        dev_health_info->stream_id.len = sprintf(stream_id, "%s/%s", stream_id, element->name);

        add_item_to_csv(connector_ptr, &value, type);
    }

    switch (type)
    {
        case DEV_HEALTH_TYPE_INT32:
        case DEV_HEALTH_TYPE_UINT64:
        case DEV_HEALTH_TYPE_FLOAT:
            break;
        case DEV_HEALTH_TYPE_STRING:
        case DEV_HEALTH_TYPE_JSON:
        case DEV_HEALTH_TYPE_GEOJSON:
            if (value.string != NULL)
            {
                cc_dev_health_free_string(value.string);
            }
            break;
        case DEV_HEALTH_TYPE_NONE:
            connector_debug_line("Fatal error, type not set");
            ASSERT(type != DEV_HEALTH_TYPE_NONE);
            break;
    }
}

static char const * get_remaining_path(char const * const path)
{
    char const * remaining_path = NULL;
    static char const * const empty_path = "";

    if (strchr(path, '/') != NULL)
    {
        remaining_path = strchr(path, '/') + 1;
    }
    else
    {
        remaining_path = empty_path;
    }

    return remaining_path;
}

STATIC void dev_health_process_group_items(connector_data_t * const connector_ptr, unsigned int const upper_index, unsigned int const lower_index, dev_health_item_t const * const * const items_array, unsigned int const array_size, char const * const path)
{
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    unsigned int const stream_id_len = dev_health_info->stream_id.len;
    char * const p_original_stream_id_end = &dev_health_info->stream_id.string[stream_id_len];
    connector_bool_t const handle_all = path[0] == '\0' ? connector_true : connector_false;
    unsigned int i;

    for (i = 0; i < array_size; i++)
    {
        dev_health_item_t const * const item = items_array[i];
        size_t const name_len = item->name_len;

        if (handle_all || strncmp(path, item->name, name_len) == 0)
        {
            dev_health_process_item(connector_ptr, item, upper_index, lower_index);
        }
        *p_original_stream_id_end = '\0';
    }
}

STATIC void dev_health_process_next_group(connector_data_t * connector_ptr, unsigned int upper_index, unsigned int const lower_index, dev_health_path_group_t const * const group, char const * const path);

STATIC void dev_health_process_subgroups(connector_data_t * connector_ptr, unsigned int upper_index, unsigned int lower_index, dev_health_path_group_t const * const * const subgroups_array, unsigned int const array_size, char const * const path)
{
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    unsigned int stream_id_len = dev_health_info->stream_id.len;
    char * const p_stream_id_end = &dev_health_info->stream_id.string[stream_id_len];
    unsigned int i;
    connector_bool_t const handle_all = path[0] == '\0' ? connector_true : connector_false;

    for (i = 0; i < array_size; i++)
    {
        dev_health_path_group_t const * const subgroup = subgroups_array[i];
        size_t const name_len = subgroup->name_len;
        char const * const remaining_path = get_remaining_path(path);
        connector_bool_t const multi_instance = subgroup->multi_instance != NULL ? connector_true : connector_false;

        if (multi_instance)
        {
            connector_bool_t const handle_all_instances = remaining_path[0] == '\0' ? connector_true : connector_false;

            if (handle_all_instances)
            {
                unsigned int current_instance;
                unsigned int const instances = subgroup->multi_instance(upper_index);

                for (current_instance = 0; current_instance < instances; current_instance++)
                {
                    if (subgroup->name[0] != '\0')
                    {
                        dev_health_info->stream_id.len = sprintf(dev_health_info->stream_id.string, "%s/%s/%u", dev_health_info->stream_id.string, subgroup->name, current_instance);
                    }
                    dev_health_process_next_group(connector_ptr, upper_index, current_instance, subgroup, remaining_path); /* recursion */
                    *p_stream_id_end = '\0';
                }
            }
            else if (name_len != 0 && strncmp(path, subgroup->name, name_len) == 0)
            {
                unsigned long int const single_instance = strtoul(remaining_path, NULL, 10);
                char const * const updated_remaining_path = get_remaining_path(remaining_path);

                if (subgroup->name[0] != '\0')
                {
                    dev_health_info->stream_id.len = sprintf(dev_health_info->stream_id.string, "%s/%s/%lu", dev_health_info->stream_id.string, subgroup->name, single_instance);
                }
                dev_health_process_next_group(connector_ptr, upper_index, lower_index, subgroup, updated_remaining_path);
                *p_stream_id_end = '\0';
            }
            break;
        }
        else
        {
            if (subgroup->name[0] != '\0')
            {
                dev_health_info->stream_id.len = sprintf(dev_health_info->stream_id.string, "%s/%s", dev_health_info->stream_id.string, subgroup->name);
            }

            if (handle_all || (name_len != 0 && strncmp(path, subgroup->name, name_len) == 0))
            {
                dev_health_process_next_group(connector_ptr, upper_index, lower_index, subgroup, remaining_path); /* recursion */
                *p_stream_id_end = '\0';
            }
        }
    }
}

STATIC void dev_health_process_next_group(connector_data_t * connector_ptr, unsigned int upper_index, unsigned int const lower_index, dev_health_path_group_t const * const group, char const * const path)
{
    dev_health_path_type_t const type = group->child.type;

    switch (type)
    {
        case ITEMS:
        {
            dev_health_item_t const * const * const items_array = group->child.data.items.array;
            unsigned int const array_size = group->child.data.items.size;

            dev_health_process_group_items(connector_ptr, upper_index, lower_index, items_array, array_size, path);
            break;
        }
        case SUBGROUPS:
        {
            dev_health_path_group_t const * const * const subgroups_array = group->child.data.subgroups.array;
            unsigned int const array_size = group->child.data.subgroups.size;

            /* Calls us recursively until "type" is "ITEM"*/
            dev_health_process_subgroups(connector_ptr, upper_index, lower_index, subgroups_array, array_size, path);
            break;
        }
    }
}

STATIC void dev_health_process_root_group(connector_data_t * const connector_ptr, dev_health_path_group_t const * const root_group, char const * const path)
{
    dev_health_info_t * const dev_health_info = &connector_ptr->dev_health.info;
    connector_bool_t const handle_all = path[0] == '\0' ? connector_true : connector_false;
    connector_bool_t const multi_instance = root_group->multi_instance != NULL ? connector_true : connector_false;

    if (handle_all)
    {
        unsigned long int current_instance;
        unsigned long int const total_instances = multi_instance ? root_group->multi_instance(0) : 1;

        for (current_instance = 0; current_instance < total_instances; current_instance++)
        {
            unsigned int const lower_index_0 = 0;
            if (multi_instance)
            {
                dev_health_info->stream_id.len = sprintf(dev_health_info->stream_id.string, "%s/%lu", root_group->name, current_instance);
            }
            else
            {
                dev_health_info->stream_id.len = sprintf(dev_health_info->stream_id.string, "%s", root_group->name);
            }

            dev_health_process_next_group(connector_ptr, current_instance, lower_index_0, root_group, path);
        }
    }
    else
    {
        char const * const remaining_path = get_remaining_path(path);
        unsigned long int const single_instance = strtoul(path, NULL, 10);
        unsigned int const lower_index_0 = 0;

        if (multi_instance)
        {
            dev_health_info->stream_id.len = sprintf(dev_health_info->stream_id.string, "%s/%lu", root_group->name, single_instance);
        }
        else
        {
            dev_health_info->stream_id.len = sprintf(dev_health_info->stream_id.string, "%s", root_group->name);
        }

        dev_health_process_next_group(connector_ptr, single_instance, lower_index_0, root_group, remaining_path);
    }
}

STATIC void dev_health_process_path(connector_data_t * const connector_ptr, char const * const path)
{
    unsigned int i;

    for(i = 0; i < asizeof(dev_health_root_groups); i++)
    {
        dev_health_path_group_t const * const root_group = dev_health_root_groups[i];
        size_t const name_len = root_group->name_len;

        if (strncmp(path, root_group->name, name_len) == 0)
        {
            char const * const remaining_path = get_remaining_path(path);

            dev_health_process_root_group(connector_ptr, root_group, remaining_path);
            break;
        }
    }
}
