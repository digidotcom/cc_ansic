/*
 * Copyright (c) 2015 Digi International Inc.,
 * All rights not expressly granted are reserved.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
 * =======================================================================
 */

#ifndef _CONNECTOR_DATA_POINT_CSV_GENERATOR_H_
#define _CONNECTOR_DATA_POINT_CSV_GENERATOR_H_

#if (defined CONNECTOR_SUPPORTS_64_BIT_INTEGERS)
#define largest_uint_t uint64_t
#define largest_int_t int64_t
#else
#define largest_uint_t uint32_t
#define largest_int_t int32_t
#endif

#define QUOTES_NEEDED_FLAG          UINT32_C(0x01)
#define LEADING_QUOTES_PUT_FLAG     UINT32_C(0x02)
#define TRAILING_QUOTES_PUT_FLAG    UINT32_C(0x04)
#define ESCAPE_FLAG                 UINT32_C(0x08)

#define connector_bool(cond)    ((cond) ? connector_true : connector_false)

#define IsBitSet(flag, bit)   (connector_bool(((flag) & (bit)) == (bit)))
#define IsBitClear(flag, bit) (connector_bool(((flag) & (bit)) == 0))
#define BitSet(flag, bit)    ((flag) |= (bit))
#define BitClear(flag, bit)  ((flag) &= ~(bit))

#define ClearAllFlags(flag)         ((flag) = 0)

#define QuotesNeeded(flag)          IsBitSet((flag), QUOTES_NEEDED_FLAG)
#define QuotesNotNeeded(flag)       IsBitClear((flag), QUOTES_NEEDED_FLAG)
#define SetQuotesNeeded(flag)       BitSet((flag), QUOTES_NEEDED_FLAG)
#define ClearQuotesNeeded(flag)     BitClear((flag), QUOTES_NEEDED_FLAG)

#define LeadingPut(flag)            IsBitSet((flag), LEADING_QUOTES_PUT_FLAG)
#define LeadingNotPut(flag)         IsBitClear((flag), LEADING_QUOTES_PUT_FLAG)
#define SetLeadingPut(flag)         BitSet((flag), LEADING_QUOTES_PUT_FLAG)
#define ClearLeadingPut(flag)       BitClear((flag), LEADING_QUOTES_PUT_FLAG)

#define TrailingPut(flag)           IsBitSet((flag), TRAILING_QUOTES_PUT_FLAG)
#define TrailingNotPut(flag)        IsBitClear((flag), TRAILING_QUOTES_PUT_FLAG)
#define SetTrailingPut(flag)        BitSet((flag), TRAILING_QUOTES_PUT_FLAG)
#define ClearTrailingPut(flag)      BitClear((flag), TRAILING_QUOTES_PUT_FLAG)

#define EscapeChar(flag)            IsBitSet((flag), ESCAPE_FLAG)
#define NotEscapeChar(flag)         IsBitClear((flag), ESCAPE_FLAG)
#define SetEscapeChar(flag)         BitSet((flag), ESCAPE_FLAG)
#define ClearEscapeChar(flag)       BitClear((flag), ESCAPE_FLAG)

#define character_needs_escaping(character) ((character) == '\\' || (character) == '\"' ? connector_true : connector_false)

typedef struct {
    largest_uint_t value;
    int figures;
    connector_bool_t negative;
} int_info_t;

typedef struct {
    int_info_t integer;
    int_info_t fractional;
    connector_bool_t point_set;
} double_info_t;

/************************************************************************
** WARNING: Don't change the order of the state unless default         **
**          CSV format described in the Cloud documentation changes.   **
************************************************************************/
typedef enum {
    csv_data,
    csv_time,
    csv_quality,
    csv_description,
    csv_location,
    csv_type,
    csv_unit,
    csv_forward_to,
    csv_stream_id,
    csv_finished
} csv_field_t;

typedef struct {
    char const * next_char;
    int quotes_info;
} string_info_t;

typedef struct {
    char * buffer;
    size_t bytes_available;
    size_t bytes_written;
} buffer_info_t;

typedef enum {
    LOCATION_STATE_PUT_LEADING_QUOTE,
    LOCATION_STATE_INIT_LATITUDE,
    LOCATION_STATE_PUT_LATITUDE,
    LOCATION_STATE_PUT_1ST_COMMA,
    LOCATION_STATE_INIT_LONGITUDE,
    LOCATION_STATE_PUT_LONGITUDE,
    LOCATION_STATE_PUT_2ND_COMMA,
    LOCATION_STATE_INIT_ELEVATION,
    LOCATION_STATE_PUT_ELEVATION,
    LOCATION_STATE_PUT_TRAILING_QUOTE,
    LOCATION_STATE_FINISH
} location_state_t;

typedef enum {
    TIME_EPOCH_FRAC_STATE_SECONDS,
    TIME_EPOCH_FRAC_STATE_MILLISECONDS,
    TIME_EPOCH_FRAC_STATE_FINISH
} time_epoch_frac_state_t;

typedef struct {
    connector_data_stream_t const * current_data_stream;
    connector_data_point_t const * current_data_point;
    csv_field_t current_csv_field;

    struct {
        connector_bool_t init;
        union {
            int_info_t intg;
            double_info_t dbl;
            string_info_t str;
        } info;
        union {
            location_state_t location;
            time_epoch_frac_state_t time;
        } internal_state;
    } data;
} csv_process_data_t;

STATIC connector_bool_t string_needs_quotes(char const * const string)
{
    connector_bool_t need_quotes = connector_false;
    size_t index;

    if (string == NULL)
    {
        goto done;
    }

    for (index = 0; string[index] != '\0'; index++)
    {
        if (strchr("\"\\, \t\r\n", string[index]) != NULL)
        {
            need_quotes = connector_true;
            break;
        }
    }
done:
    return need_quotes;
}

STATIC void put_character(char const character, buffer_info_t * const buffer_info)
{
    size_t const offset = buffer_info->bytes_written;
    buffer_info->buffer[offset] = character;
    buffer_info->bytes_written += 1;
    buffer_info->bytes_available -= 1;
}

STATIC connector_bool_t process_string(string_info_t * const string_info, buffer_info_t * const buffer_info)
{
    int quotes_flags = string_info->quotes_info;
    connector_bool_t done_processing = connector_false;

    if (string_info->next_char == NULL)
    {
        done_processing = connector_true;
        goto done;
    }

    if (QuotesNeeded(quotes_flags) && LeadingNotPut(quotes_flags))
    {
        if (buffer_info->bytes_available > 0)
        {
            put_character('\"', buffer_info);
            SetLeadingPut(quotes_flags);
        }
    }

    while (*string_info->next_char != '\0' && buffer_info->bytes_available > 0)
    {
        if (NotEscapeChar(quotes_flags) && character_needs_escaping(*string_info->next_char))
        {
            SetEscapeChar(quotes_flags);
            put_character('\\', buffer_info);
        }
        else
        {
            put_character(*string_info->next_char, buffer_info);
            string_info->next_char += 1;
            ClearEscapeChar(quotes_flags);
        }
    };

    if (*string_info->next_char == '\0')
    {
        if (QuotesNeeded(quotes_flags))
        {
            if (TrailingPut(quotes_flags))
            {
                done_processing = connector_true;
            }
            else
            {
                if (buffer_info->bytes_available > 0)
                {
                    put_character('\"', buffer_info);
                    SetTrailingPut(quotes_flags);
                    done_processing = connector_true;
                }
            }
        }
        else
        {
            done_processing = connector_true;
        }
    }

done:
    string_info->quotes_info = quotes_flags;
    return done_processing;
}

STATIC int count_int_ciphers(int const int_value)
{
    int length = 1;
    int value = int_value;

    while ((value /= 10) >= 1)
    {
        length++;
    }

    return length;
}

STATIC unsigned int get_next_cipher(largest_uint_t number, unsigned int const cipher_order)
{
    unsigned int const base = 10;
    unsigned int cipher;
    unsigned int i;

    for (i = 0; i < cipher_order - 1; i++)
    {
        number /= base;
    }

    cipher = number % base;

    return cipher;
}

STATIC connector_bool_t process_integer(int_info_t * const int_info, buffer_info_t * const buffer_info)
{
    connector_bool_t done_processing = connector_false;

    if (int_info->negative)
    {
        if (buffer_info->bytes_available > 0)
        {
            put_character('-', buffer_info);
            int_info->negative = connector_false;
        }
        else
        {
            goto done;
        }
    }

    while (int_info->figures != 0 && buffer_info->bytes_available > 0)
    {
        unsigned int const cipher = get_next_cipher(int_info->value, int_info->figures--);

        put_character('0' + cipher, buffer_info);
    }

    if (int_info->figures == 0)
    {
        done_processing = connector_true;
    }

done:
    return done_processing;
}

STATIC connector_bool_t process_double(double_info_t * const double_info, buffer_info_t * const buffer_info)
{
    connector_bool_t done_processing = connector_false;

    while (!done_processing && buffer_info->bytes_available > 0)
    {
        connector_bool_t const done_processing_integer_part = process_integer(&double_info->integer, buffer_info);

        if (done_processing_integer_part)
        {
            if (!double_info->point_set)
            {
                if (buffer_info->bytes_available > 0)
                {
                    put_character('.', buffer_info);
                    double_info->point_set = connector_true;
                }
                else
                {
                    goto done;
                }
            }

            {
                connector_bool_t const done_processing_fractional_part = process_integer(&double_info->fractional, buffer_info);

                if (done_processing_fractional_part)
                {
                    done_processing = connector_true;
                }
            }
        }
    }

done:
    return done_processing;
}

STATIC void init_int_info(int_info_t * const int_info, largest_int_t const value)
{
    largest_uint_t const absolute_value = value >= 0 ? value : -value;

    int_info->value = absolute_value;
    int_info->figures = count_int_ciphers(absolute_value);
    int_info->negative = value < 0 ? connector_true : connector_false;
}

STATIC long double_to_long_rounded(double const double_val)
{
    long long_value;

    if (double_val >= 0)
    {
        long_value = (long)(double_val + 0.5);
    }
    else
    {
        long_value = (long)(double_val - 0.5);
    }

    return long_value;
}

STATIC void init_double_info(double_info_t * const double_info, double const value)
{
    double const absolute_value = value >= 0 ? value : -value;
    long const integer_part = (long)absolute_value;
    double const double_fractional_part = (absolute_value - integer_part) * 1000000;
    long const fractional_part = double_to_long_rounded(double_fractional_part);

    init_int_info(&double_info->integer, integer_part);
    init_int_info(&double_info->fractional, fractional_part);
    if (value < 0)
    {
        double_info->integer.negative = connector_true;
    }
    double_info->point_set = connector_false;
}

STATIC void terminate_csv_field(csv_process_data_t * const csv_process_data, buffer_info_t * const buffer_info, csv_field_t const next_field)
{
    if (buffer_info->bytes_available > 0)
    {
        csv_process_data->data.init = connector_false;
        put_character(',', buffer_info);
        csv_process_data->current_csv_field = next_field;
    }
}

STATIC void init_string_info(string_info_t * const string_info, char const * const string)
{
    ClearAllFlags(string_info->quotes_info);
    if (string_needs_quotes(string))
    {
        SetQuotesNeeded(string_info->quotes_info);
    }
    string_info->next_char = string;
}

STATIC connector_bool_t process_csv_data(csv_process_data_t * const csv_process_data, buffer_info_t * const buffer_info)
{
    connector_data_point_t const * const current_data_point = csv_process_data->current_data_point;
    connector_data_stream_t const * const current_data_stream = csv_process_data->current_data_stream;
    connector_bool_t done_processing = connector_false;

    if (!csv_process_data->data.init)
    {
        csv_process_data->data.init = connector_true;

        switch (current_data_point->data.type)
        {
            case connector_data_type_text:
            {
                init_string_info(&csv_process_data->data.info.str, current_data_point->data.element.text);
                break;
            }
            case connector_data_type_native:
            {
                switch (current_data_stream->type)
                {
                    case connector_data_point_type_string:
                    case connector_data_point_type_geojson:
                    case connector_data_point_type_json:
                    case connector_data_point_type_binary:
                    {


                        init_string_info(&csv_process_data->data.info.str, current_data_point->data.element.native.string_value);
                        break;
                    }
                    case connector_data_point_type_double:
                    {
#if (defined CONNECTOR_SUPPORTS_FLOATING_POINT)
                        init_double_info(&csv_process_data->data.info.dbl, current_data_point->data.element.native.double_value);
#else
                        connector_debug_line("CONNECTOR_SUPPORTS_FLOATING_POINT not defined");
                        ASSERT(current_data_stream->type != connector_data_point_type_double);
#endif
                        break;
                    }
                    case connector_data_point_type_float:
                    {
#if (defined CONNECTOR_SUPPORTS_FLOATING_POINT)
                        init_double_info(&csv_process_data->data.info.dbl, current_data_point->data.element.native.float_value);
#else
                        connector_debug_line("CONNECTOR_SUPPORTS_FLOATING_POINT not defined");
                        ASSERT(current_data_stream->type != connector_data_point_type_float);
#endif
                        break;
                    }
                    case connector_data_point_type_integer:
                    {
                        init_int_info(&csv_process_data->data.info.intg, current_data_point->data.element.native.int_value);
                        break;
                    }
                    case connector_data_point_type_long:
                    {
#if (defined CONNECTOR_SUPPORTS_64_BIT_INTEGERS)
                        init_int_info(&csv_process_data->data.info.intg, current_data_point->data.element.native.long_value);
                        break;
#else
                        connector_debug_line("CONNECTOR_SUPPORTS_64_BIT_INTEGERS not defined");
                        ASSERT(current_data_stream->type != connector_data_point_type_long);
#endif
                    }

                }
                break;
            }
        }
    }

    switch (current_data_point->data.type)
    {
        case connector_data_type_text:
            done_processing = process_string(&csv_process_data->data.info.str, buffer_info);
            break;
        case connector_data_type_native:
            switch (current_data_stream->type)
            {
                case connector_data_point_type_string:
                case connector_data_point_type_geojson:
                case connector_data_point_type_json:
                case connector_data_point_type_binary:
                    done_processing = process_string(&csv_process_data->data.info.str, buffer_info);
                    break;
                case connector_data_point_type_double:
                case connector_data_point_type_float:
                    done_processing = process_double(&csv_process_data->data.info.dbl, buffer_info);
                    break;
                case connector_data_point_type_integer:
                case connector_data_point_type_long:
                    done_processing = process_integer(&csv_process_data->data.info.intg, buffer_info);
                    break;
            }
            break;
    }

    return done_processing;
}

STATIC connector_bool_t process_csv_location(csv_process_data_t * const csv_process_data, buffer_info_t * const buffer_info)
{
    connector_data_point_t const * const current_data_point = csv_process_data->current_data_point;
    connector_bool_t done_processing = connector_false;

    switch (current_data_point->location.type)
    {
        case connector_location_type_ignore:
        {
            done_processing = connector_true;
            break;
        }
        case connector_location_type_text:
#if (defined CONNECTOR_SUPPORTS_FLOATING_POINT)
        case connector_location_type_native:
#endif
        {
            if (!csv_process_data->data.init)
            {
                csv_process_data->data.init = connector_true;
                csv_process_data->data.internal_state.location = LOCATION_STATE_PUT_LEADING_QUOTE;
            }
            switch (csv_process_data->data.internal_state.location)
            {
                case LOCATION_STATE_PUT_LEADING_QUOTE:
                case LOCATION_STATE_PUT_TRAILING_QUOTE:
                {
                    if (buffer_info->bytes_available > 0)
                    {
                        put_character('\"', buffer_info);
                        csv_process_data->data.internal_state.location++;
                    }
                    break;
                }
                case LOCATION_STATE_INIT_LATITUDE:
                {
#if (defined CONNECTOR_SUPPORTS_FLOATING_POINT)
                    if (current_data_point->location.type == connector_location_type_native)
                    {
                        init_double_info(&csv_process_data->data.info.dbl, current_data_point->location.value.native.latitude);
                    }
                    else
                    {
                        init_string_info(&csv_process_data->data.info.str, current_data_point->location.value.text.latitude);
                        ClearQuotesNeeded(csv_process_data->data.info.str.quotes_info);
                    }
#else
                    init_string_info(&csv_process_data->data.info.str, current_data_point->location.value.text.latitude);
                    ClearQuotesNeeded(csv_process_data->data.info.str.quotes_info);
#endif
                    csv_process_data->data.internal_state.location++;
                    break;
                }
                case LOCATION_STATE_INIT_LONGITUDE:
                {
#if (defined CONNECTOR_SUPPORTS_FLOATING_POINT)
                    if (current_data_point->location.type == connector_location_type_native)
                    {
                        init_double_info(&csv_process_data->data.info.dbl, current_data_point->location.value.native.longitude);
                    }
                    else
                    {
                        init_string_info(&csv_process_data->data.info.str, current_data_point->location.value.text.longitude);
                        ClearQuotesNeeded(csv_process_data->data.info.str.quotes_info);
                    }
#else
                    init_string_info(&csv_process_data->data.info.str, current_data_point->location.value.text.longitude);
                    ClearQuotesNeeded(csv_process_data->data.info.str.quotes_info);
#endif
                    csv_process_data->data.internal_state.location++;
                    break;
                }
                case LOCATION_STATE_INIT_ELEVATION:
                {
#if (defined CONNECTOR_SUPPORTS_FLOATING_POINT)
                    if (current_data_point->location.type == connector_location_type_native)
                    {
                        init_double_info(&csv_process_data->data.info.dbl, current_data_point->location.value.native.elevation);
                    }
                    else
                    {
                        init_string_info(&csv_process_data->data.info.str, current_data_point->location.value.text.elevation);
                        ClearQuotesNeeded(csv_process_data->data.info.str.quotes_info);
                    }
#else
                    init_string_info(&csv_process_data->data.info.str, current_data_point->location.value.text.elevation);
                    ClearQuotesNeeded(csv_process_data->data.info.str.quotes_info);
#endif
                    csv_process_data->data.internal_state.location++;
                    break;
                }
                case LOCATION_STATE_PUT_LATITUDE:
                case LOCATION_STATE_PUT_LONGITUDE:
                case LOCATION_STATE_PUT_ELEVATION:
                {
#if (defined CONNECTOR_SUPPORTS_FLOATING_POINT)
                    connector_bool_t field_done;

                    if (current_data_point->location.type == connector_location_type_native)
                    {
                        field_done = process_double(&csv_process_data->data.info.dbl, buffer_info);
                    }
                    else
                    {
                        field_done = process_string(&csv_process_data->data.info.str, buffer_info);
                    }
#else
                    connector_bool_t const field_done = process_string(&csv_process_data->data.info.str, buffer_info);
#endif

                    if (field_done)
                    {
                        csv_process_data->data.internal_state.location++;
                    }
                    break;
                }
                case LOCATION_STATE_PUT_1ST_COMMA:
                case LOCATION_STATE_PUT_2ND_COMMA:
                {
                    if (buffer_info->bytes_available > 0)
                    {
                        put_character(',', buffer_info);
                        csv_process_data->data.internal_state.location++;
                    }
                    break;
                }
                case LOCATION_STATE_FINISH:
                    done_processing = connector_true;
                    break;
            }
            break;
        }
    }

    return done_processing;
}

STATIC connector_bool_t process_csv_time(csv_process_data_t * const csv_process_data, buffer_info_t * const buffer_info)
{
    connector_data_point_t const * const current_data_point = csv_process_data->current_data_point;
    connector_bool_t done_processing = connector_false;

    switch (current_data_point->time.source)
    {
        case connector_time_cloud:
            done_processing = connector_true;
            break;
        case connector_time_local_epoch_fractional:
        {
            if (!csv_process_data->data.init)
            {
                csv_process_data->data.init = connector_true;
                init_int_info(&csv_process_data->data.info.intg, current_data_point->time.value.since_epoch_fractional.seconds);
                csv_process_data->data.internal_state.time = TIME_EPOCH_FRAC_STATE_SECONDS;
            }

            switch (csv_process_data->data.internal_state.time)
            {
                case TIME_EPOCH_FRAC_STATE_SECONDS:
                case TIME_EPOCH_FRAC_STATE_MILLISECONDS:
                {
                    connector_bool_t const field_done = process_integer(&csv_process_data->data.info.intg, buffer_info);

                    if (field_done)
                    {
                        init_int_info(&csv_process_data->data.info.intg, current_data_point->time.value.since_epoch_fractional.milliseconds);
                        csv_process_data->data.internal_state.time++;
                    }
                    break;
                }
                case TIME_EPOCH_FRAC_STATE_FINISH:
                    done_processing = connector_true;
                    break;
            }
            break;
        }
#if (defined CONNECTOR_SUPPORTS_64_BIT_INTEGERS)
        case connector_time_local_epoch_whole:
        {
            if (!csv_process_data->data.init)
            {
                csv_process_data->data.init = connector_true;
                init_int_info(&csv_process_data->data.info.intg, current_data_point->time.value.since_epoch_whole.milliseconds);
            }

            done_processing = process_integer(&csv_process_data->data.info.intg, buffer_info);
            break;
        }
#endif
        case connector_time_local_iso8601:
        {
            if (!csv_process_data->data.init)
            {
                csv_process_data->data.init = connector_true;
                init_string_info(&csv_process_data->data.info.str, current_data_point->time.value.iso8601_string);
            }
            done_processing = process_string(&csv_process_data->data.info.str, buffer_info);
            break;
        }
    }

    return done_processing;
}

size_t dp_generate_csv(csv_process_data_t * const csv_process_data, buffer_info_t * const buffer_info)
{
    while (buffer_info->bytes_available && csv_process_data->current_data_point != NULL)
    {
        connector_data_point_t const * const current_data_point = csv_process_data->current_data_point;
        connector_data_stream_t const * const current_data_stream = csv_process_data->current_data_stream;

        switch (csv_process_data->current_csv_field)
        {
            case csv_data:
            {
                connector_bool_t const done_processing = process_csv_data(csv_process_data, buffer_info);

                if (done_processing)
                {
                    terminate_csv_field(csv_process_data, buffer_info, csv_time);
                }
                break;
            }
            case csv_time:
            {
                connector_bool_t const done_processing = process_csv_time(csv_process_data, buffer_info);

                if (done_processing)
                {
                    terminate_csv_field(csv_process_data, buffer_info, csv_quality);
                }
                break;
            }
            case csv_quality:
            {
                connector_bool_t done_processing = connector_false;

                switch (current_data_point->quality.type)
                {
                    case connector_quality_type_ignore:
                    {
                        done_processing = connector_true;
                        break;
                    }
                    case connector_quality_type_native:
                    {
                        if (!csv_process_data->data.init)
                        {
                            csv_process_data->data.init = connector_true;
                            init_int_info(&csv_process_data->data.info.intg, current_data_point->quality.value);
                        }

                        done_processing = process_integer(&csv_process_data->data.info.intg, buffer_info);
                        break;
                    }
                }

                if (done_processing)
                {
                    terminate_csv_field(csv_process_data, buffer_info, csv_description);
                }
                break;
            }

            case csv_location:
            {
                connector_bool_t const done_processing = process_csv_location(csv_process_data, buffer_info);

                if (done_processing)
                {
                    terminate_csv_field(csv_process_data, buffer_info, csv_type);
                }
                break;
            }

            case csv_type:
            case csv_description:
            case csv_unit:
            case csv_forward_to:
            case csv_stream_id:
            {
                connector_bool_t done_processing;

                if (!csv_process_data->data.init)
                {
                    char const * string = NULL;

                    switch (csv_process_data->current_csv_field)
                    {
                        case csv_description:
                            string = current_data_point->description;
                            break;
                        case csv_type:
                        {
                            static char const * const type_list[] = {"INTEGER", "LONG", "FLOAT", "DOUBLE", "STRING", "BINARY", "JSON", "GEOJSON"};
                            string = type_list[csv_process_data->current_data_stream->type];
                            break;
                        }
                        case csv_unit:
                            string = current_data_stream->unit;
                            break;
                        case csv_forward_to:
                            string = current_data_stream->forward_to;
                            break;
                        case csv_stream_id:
                            string = current_data_stream->stream_id;
                            break;
                        default:
                            ASSERT(0);
                            break;
                    }

                    csv_process_data->data.init = connector_true;
                    init_string_info(&csv_process_data->data.info.str, string);
                }

                done_processing = process_string(&csv_process_data->data.info.str, buffer_info);

                if (done_processing && buffer_info->bytes_available > 0)
                {
                    csv_process_data->data.init = connector_false;
                    csv_process_data->current_csv_field++;

                    ASSERT(csv_process_data->current_csv_field <= csv_finished);

                    if (csv_process_data->current_csv_field != csv_finished)
                    {
                        put_character(',', buffer_info);
                    }
                }
                break;
            }

            case csv_finished:
            {
                if (buffer_info->bytes_available > 0)
                {
                    put_character('\n', buffer_info);
                    csv_process_data->current_data_point = current_data_point->next;

                    if (csv_process_data->current_data_point == NULL)
                    {
                        csv_process_data->current_data_stream = csv_process_data->current_data_stream->next;

                        if (csv_process_data->current_data_stream != NULL)
                        {
                            csv_process_data->current_data_point = csv_process_data->current_data_stream->point;
                        }
                    }

                    csv_process_data->current_csv_field = csv_data;
                    csv_process_data->data.init = connector_false;
                }
                break;
            }
        }
    }

    return buffer_info->bytes_written;
}
#endif
