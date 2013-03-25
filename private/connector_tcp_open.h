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

static connector_status_t connect_server(connector_data_t * const connector_ptr, char const * server_url, size_t const server_url_length)
{
    connector_status_t result = connector_idle;

    connector_callback_status_t status;
    size_t length = sizeof *connector_ptr->edp_data.network_handle;
    connector_request_t request_id;

    request_id.network_request = connector_network_open;
    status = connector_callback(connector_ptr->callback, connector_class_network_tcp, request_id, server_url, server_url_length, &connector_ptr->edp_data.network_handle, &length);
    switch (status)
    {
    case connector_callback_continue:
        if (length == sizeof *connector_ptr->edp_data.network_handle)
        {
            result = connector_working;
        }
        else
        {
            result = connector_abort;
            notify_error_status(connector_ptr->callback, connector_class_network_tcp, request_id, connector_invalid_data_size);
        }
        break;
    case  connector_callback_abort:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        result = connector_abort;
#else
        result = connector_connect_error;
#endif
        break;

    case connector_callback_unrecognized:
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        result = connector_unavailable;
#else
        result = connector_connect_error;
#endif
        break;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
    case connector_callback_error:
        result = connector_open_error;
        break;
#endif
    case connector_callback_busy:
        result = connector_pending;
        break;
    }

    return result;
}

static connector_status_t send_version(connector_data_t * connector_ptr, uint16_t const type, uint32_t const version)
{
enum edp_version {
    field_define(edp_version, version, uint32_t),
    record_end(edp_version)
};
    size_t const version_message_size = record_bytes(edp_version);

    connector_status_t result = connector_pending;
    uint8_t * edp_version;
    uint8_t * packet;

    /*
     *  version packet format:
     *  -------------------------
     * | 0 - 1 | 2 - 3 |  4 - 7  |
     *  -------------------------
     * |  EDP header   | version |
     *  -------------------------
    */

    packet = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &edp_version, NULL);
    if (packet != NULL)
    {
        message_store_be32(edp_version, version, version);

        result = tcp_initiate_send_packet(connector_ptr, packet, version_message_size,
                                    type,
                                    tcp_release_packet_buffer,
                                    NULL);
    }

    return result;
}

static connector_status_t receive_edp_version(connector_data_t * const connector_ptr)
{
#define SERVER_OVERLOAD_RESPONSE    0x02

    connector_status_t result;
    uint8_t * ptr;
    connector_buffer_t * packet_buffer;

    result = tcp_receive_packet(connector_ptr, &packet_buffer);

    if (result == connector_working && packet_buffer != NULL)
    {
        uint16_t type;
        uint8_t  response_code;
        uint8_t * const packet = packet_buffer->buffer;

        connector_debug_printf("Receive Mt version\n");
        /*
         * MT version response packet format:
         *  ---------------
         * |0 - 1 |  2 - 3 |
         *  ---------------
         * | Type | length |
         *  ---------------
         *
         */

        ptr = GET_PACKET_DATA_POINTER(packet, PACKET_EDP_HEADER_SIZE);
        {
            uint8_t * edp_header = packet;
            type = message_load_be16(edp_header, type);
        }
        response_code = *ptr;

        tcp_release_receive_packet(connector_ptr, packet_buffer);
        if (type != E_MSG_MT2_TYPE_VERSION_OK)
        {
            /*
             * The received message is not acceptable.
             */
            switch (type)
            {
                /* Expected MTv2 message types... */
                case E_MSG_MT2_TYPE_LEGACY_EDP_VER_RESP:
                    if (response_code == SERVER_OVERLOAD_RESPONSE) {
                        connector_debug_printf("receive_edp_version: MTv2 legacy Server responded with overload msg\n");
                    }
                    else {
                        connector_debug_printf("receive_edp_version: MTv2 legacy bad version\n");
                    }
                    break;
                case E_MSG_MT2_TYPE_VERSION_BAD:
                    connector_debug_printf("receive_edp_version: bad version\n");
                    break;
                case E_MSG_MT2_TYPE_SERVER_OVERLOAD:
                    connector_debug_printf("receive_edp_version: Server responded with overload msg\n");
                    break;
                /* Unexpected/unknown MTv2 message types... */
                default:
                    connector_debug_printf("receive_edp_version: unknown MTv2 message\n");
            }
            /* mt version error. let's notify user.
             *
             * ignore error status callback return value since server
             * will close the connection.
             */
            result = connector_abort;
        }
    }
    else if (result == connector_idle) result = connector_pending;

    return result;
}

static size_t build_keepalive_param(uint8_t * const edp_header, uint16_t const type, uint16_t const value)
{
enum edp_keepalive{
    field_define(edp_keepalive, interval, uint16_t),
    record_end(edp_keepalive)
};

    size_t const keepalive_size = record_bytes(edp_keepalive);
    uint8_t * edp_keepalive;

    edp_keepalive = GET_PACKET_DATA_POINTER(edp_header, PACKET_EDP_HEADER_SIZE);
    message_store_be16(edp_header, type, type);
	{
		uint16_t const keepalive_size16 = (uint16_t) keepalive_size;

		ASSERT(keepalive_size <= UINT16_MAX);
		message_store_be16(edp_header, length, keepalive_size16);
	}
    message_store_be16(edp_keepalive, interval, value);


    return (PACKET_EDP_HEADER_SIZE + keepalive_size); /* return count of bytes added to buffer */
}

static connector_status_t send_keepalive(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_working;

    uint8_t * ptr;
    uint8_t * packet;
    uint8_t * start_ptr;
    size_t i;

    struct {
        uint16_t type;
        uint16_t value;
    } keepalive_parameters[3];

#define init_param(i, t, v) keepalive_parameters[i].type = (t); keepalive_parameters[i].value = (v)
    init_param(0, E_MSG_MT2_TYPE_KA_RX_INTERVAL, GET_RX_KEEPALIVE_INTERVAL(connector_ptr));
    init_param(1, E_MSG_MT2_TYPE_KA_TX_INTERVAL, GET_TX_KEEPALIVE_INTERVAL(connector_ptr));
    init_param(2, E_MSG_MT2_TYPE_KA_WAIT,        GET_WAIT_COUNT(connector_ptr));
#undef  init_param

    connector_debug_printf("Send keepalive params \n");
    connector_debug_printf("Rx keepalive parameter = %d\n", keepalive_parameters[0].value);
    connector_debug_printf("Tx keepalive parameter = %d\n", keepalive_parameters[1].value);
    connector_debug_printf("Wait Count parameter = %d\n", keepalive_parameters[2].value);

    packet = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &ptr, NULL);
    if (packet == NULL)
    {
        result = connector_pending;
        goto  done;
    }
    ptr = (uint8_t *)packet;
    start_ptr = ptr;

    for (i=0; i < asizeof(keepalive_parameters); i++)
    {
        size_t const len = build_keepalive_param(ptr, keepalive_parameters[i].type, keepalive_parameters[i].value);
        ptr += len;
    }
    /* Setting the total_length will enable tcp_send_packet_process.
     * Clear length to 0 for actual length that has been sent.
     */
    {
        size_t const total_packet_length = (size_t)(ptr - start_ptr);
        ASSERT(ptr > start_ptr);
        connector_ptr->edp_data.send_packet.total_length = total_packet_length;
        connector_ptr->edp_data.send_packet.bytes_sent = 0;
        connector_ptr->edp_data.send_packet.ptr = packet;
        connector_ptr->edp_data.send_packet.complete_cb = tcp_release_packet_buffer;
    }

done:
    return result;
}

static connector_status_t receive_protocol_version(connector_data_t * const connector_ptr)
{
    enum {
        initialization_version_response_acceptable,
        initialization_version_response_unacceptable,
        initialization_version_response_unavailable
    };

    connector_status_t result;
    connector_buffer_t * packet_buffer;

    result = tcp_receive_packet(connector_ptr, &packet_buffer);

    if (result == connector_working)
    {
        uint8_t * edp_header;

        ASSERT(packet_buffer != NULL);

        edp_header = packet_buffer->buffer;

        connector_debug_printf("Receive protocol version\n");
        /*
         *  version response packet format:
         *  ---------------------------------------
         * |    0 - 1     |  2 - 3 |     4         |
         *  ---------------------------------------
         * | Payload Type | length | response code |
         *  ----------------------------------------
        */
        /*
         * Empty data packet
         */
        if (message_load_be16(edp_header, length) > 0)
        {
            uint8_t * const response_code = GET_PACKET_DATA_POINTER(edp_header, PACKET_EDP_HEADER_SIZE);
            /* Parse the version response (0 = version response ok).
             * If the protocol version number was not acceptable to the server,
             * tell the application.
             */
            if (*response_code != initialization_version_response_acceptable)
            {
                edp_set_close_status(connector_ptr, connector_close_status_abort);
                connector_debug_printf("receive_procotol_version:version is not accepted by the server\n");
                edp_set_active_state(connector_ptr, connector_transport_close);
            }
        }

        tcp_release_receive_packet(connector_ptr, packet_buffer);
    }
    else if (result == connector_idle) result = connector_pending;

    return result;
}

static connector_status_t send_identity_verification(connector_data_t * const connector_ptr)
{
    /* The security operations other than encryption... */
    #define SECURITY_OPER_IDENT_FORM     0x80U

    /* Identity verification form codes... */
    #define SECURITY_IDENT_FORM_SIMPLE   0x00U /* simple verification */
    #define SECURITY_IDENT_FORM_PASSWORD 0x02U /* password verification */

    enum edp_security {
        field_define(edp_security, opcode, uint8_t),
        field_define(edp_security, identity, uint8_t),
        record_end(edp_security)
    };

    connector_status_t result;
    size_t const edp_security_size = record_bytes(edp_security);
    uint8_t * edp_security;
    uint8_t * edp_header;
    uint8_t identity = SECURITY_IDENT_FORM_SIMPLE;

    edp_header = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &edp_security, NULL);
    if (edp_header == NULL)
    {
        result = connector_pending;
        goto done;
    }


    connector_debug_printf("Send identity verification\n");

    /*
     * packet format:
     *  -----------------------------------------------------
     * | 0 - 1 | 2 - 3 |     4       |     5                 |
     *  -----------------------------------------------------
     * |   EDP header  | Form opcode | identity verification |
     *  -----------------------------------------------------
    */
    message_store_u8(edp_security, opcode, SECURITY_OPER_IDENT_FORM);

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
    {

#if (defined CONNECTOR_IDENTITY_VERIFICATION)
        connector_identity_verification_t const identity_verification = CONNECTOR_IDENTITY_VERIFICATION;
#else
        connector_identity_verification_t const identity_verification = connector_ptr->edp_data.config.identity_verification;
#endif

        switch (identity_verification)
        {
        case connector_simple_identity_verification:
            identity = SECURITY_IDENT_FORM_SIMPLE;
            break;
        case connector_password_identity_verification:
            identity = SECURITY_IDENT_FORM_PASSWORD;
            break;
        }
    }
#endif

    message_store_u8(edp_security, identity, identity);

    result = tcp_initiate_send_packet(connector_ptr, edp_header, edp_security_size,
                                E_MSG_MT2_TYPE_PAYLOAD,
                                tcp_release_packet_buffer,
                                NULL);

done:
    return result;
}

static connector_status_t send_device_id(connector_data_t * const connector_ptr)
{
    #define SECURITY_OPER_DEVICE_ID      0x81

    enum edp_device_id {
        field_define(edp_device_id, opcode, uint8_t),
        field_define_array(edp_device_id, id, DEVICE_ID_LENGTH),
        record_end(edp_device_id)
    };

    connector_status_t result;
    uint8_t * edp_header;
    uint8_t * edp_device_id;
    size_t const device_id_message_size = record_bytes(edp_device_id);

    edp_header = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &edp_device_id, NULL);
    if (edp_header == NULL)
    {
        result = connector_pending;
        goto done;
    }

    /*
     * packet format:
     *  ----------------------------------------------
     * | 0 - 1 | 2 - 3 |        4         |  5 - 20   |
     *  ----------------------------------------------
     * |   EDP header  | Device ID opcode | Device ID |
     *  ----------------------------------------------
    */
    message_store_u8(edp_device_id, opcode, SECURITY_OPER_DEVICE_ID);

    ASSERT(connector_ptr->device_id != NULL);
    memcpy((edp_device_id + field_named_data(edp_device_id, id, offset)), connector_ptr->device_id, DEVICE_ID_LENGTH);

    result = tcp_initiate_send_packet(connector_ptr, edp_header, device_id_message_size,
                                E_MSG_MT2_TYPE_PAYLOAD, tcp_release_packet_buffer,
                                NULL);
done:
    return result;
}

static connector_status_t send_server_url(connector_data_t * const connector_ptr)
{
    #define SECURITY_OPER_URL            0x86U

    /*
     * packet format:
     *  ------------------------------------------------
     * | 0 - 1 | 2 - 3 |    4       |   5 - 6    | 7... |
     *  ------------------------------------------------
     * |   EDP Header  | URL opcode | URL length | URL  |
     *  ------------------------------------------------
    */
    enum edp_server_url {
        field_define(edp_server_url, opcode, uint8_t),
        field_define(edp_server_url, url_length, uint16_t),
        record_end(edp_server_url)
    };
    connector_status_t result;
    uint8_t * edp_header;

    char * server_url = connector_ptr->edp_data.config.server_url;
    uint8_t * edp_server_url;
    uint8_t * start_ptr;

    static char const url_prefix[] = URL_PREFIX;
    static size_t const prefix_length = sizeof url_prefix -1;

    edp_header = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &start_ptr, NULL);
    if (edp_header == NULL)
    {
        result = connector_pending;
        goto done;
    }

    edp_server_url = start_ptr;

    connector_debug_printf("Send server url = %.*s\n", (int)connector_ptr->edp_data.config.server_url_length, connector_ptr->edp_data.config.server_url);

    message_store_u8(edp_server_url, opcode, SECURITY_OPER_URL);

    {
        size_t const len = connector_ptr->edp_data.config.server_url_length + prefix_length;
		uint16_t const len16 = (uint16_t) len;

		ASSERT(len <= UINT16_MAX);
        message_store_be16(edp_server_url, url_length, len16);
    }

    edp_server_url += record_bytes(edp_server_url);
    memcpy(edp_server_url, url_prefix, prefix_length);
    edp_server_url += prefix_length;

    memcpy(edp_server_url, server_url, connector_ptr->edp_data.config.server_url_length);
    edp_server_url += connector_ptr->edp_data.config.server_url_length;

    {
        size_t const length = (size_t)(edp_server_url-start_ptr);
        ASSERT(edp_server_url > start_ptr);

        result = tcp_initiate_send_packet(connector_ptr, edp_header, length,
                                E_MSG_MT2_TYPE_PAYLOAD,
                                tcp_release_packet_buffer,
                                NULL);
    }
done:
    return result;
}

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
static connector_status_t send_password(connector_data_t * const connector_ptr)
{
    #define SECURITY_OPER_PASSWORD            0x88

    /*
     * packet format:
     *  -----------------------------------------------------
     * | 0 - 1 | 2 - 3 |    4            |   5    | 7...     |
     *  -----------------------------------------------------
     * |   EDP Header  | password opcode | length | password |
     *  -----------------------------------------------------
    */
    enum edp_password {
        field_define(edp_password, opcode, uint8_t),
        field_define(edp_password, password_length, uint16_t),
        record_end(edp_password)
    };
    connector_status_t result;
    uint8_t * edp_header;

    uint8_t * edp_password;
    uint8_t * start_ptr;

    edp_header = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &start_ptr, NULL);
    if (edp_header == NULL)
    {
        result = connector_pending;
        goto done;
    }

    edp_password = start_ptr;

    message_store_u8(edp_password, opcode, SECURITY_OPER_PASSWORD);

	{
		uint16_t const length16 = (uint16_t) connector_ptr->edp_data.config.password_length;

		ASSERT(connector_ptr->edp_data.config.password_length <= UINT16_MAX);
		message_store_be16(edp_password, password_length, length16);
	}
    edp_password += record_bytes(edp_password);

    memcpy(edp_password, connector_ptr->edp_data.config.password, connector_ptr->edp_data.config.password_length);
    edp_password += connector_ptr->edp_data.config.password_length;

    {
        size_t const length = (size_t)(edp_password-start_ptr);

        ASSERT(edp_password > start_ptr);
        result = tcp_initiate_send_packet(connector_ptr, edp_header, length,
                                E_MSG_MT2_TYPE_PAYLOAD,
                                tcp_release_packet_buffer,
                                NULL);
    }
done:
    return result;
}
#endif

static connector_status_t send_vendor_id(connector_data_t * const connector_ptr)
{
    /*
     * packet format:
     *  ----------------------------------------------------------------
     * | 0 - 1 | 2 - 3 |       4         |       5          |  6 - 9    |
     *  ----------------------------------------------------------------
     * |   EDP Header  | security coding | vendor ID opcode | vendor ID |
     *  ----------------------------------------------------------------
    */
    enum edp_vendor_msg {
        field_define(edp_vendor_msg, security_coding, uint8_t),
        field_define(edp_vendor_msg, opcode, uint8_t),
        field_define_array(edp_vendor_msg, vendor_id, VENDOR_ID_LENGTH),
        record_end(edp_vendor_msg)
    };

    connector_status_t result;
    uint8_t * edp_header = NULL;

    size_t const discovery_vendor_header_size = record_bytes(edp_vendor_msg);
    uint8_t * edp_vendor_msg;

    edp_header = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &edp_vendor_msg, NULL);
    if (edp_header == NULL)
    {
        result = connector_pending;
        goto done;
    }

    message_store_u8(edp_vendor_msg, security_coding, SECURITY_PROTO_NONE);
    message_store_u8(edp_vendor_msg, opcode, DISC_OP_VENDOR_ID);
#if !(defined CONNECTOR_VENDOR_ID)
    message_store_array(edp_vendor_msg, vendor_id, connector_ptr->edp_data.config.vendor_id, VENDOR_ID_LENGTH);
#if (defined CONNECTOR_DEBUG)
    {
        uint8_t * const vendor_id = (uint8_t *)connector_ptr->edp_data.config.vendor_id;

        connector_debug_hexvalue("Send vendor id", vendor_id, VENDOR_ID_LENGTH);
    }
#endif

#else
    {
        message_store_be32(edp_vendor_msg, vendor_id, CONNECTOR_VENDOR_ID);
        connector_debug_printf("Send vendor id = 0x%08X\n", CONNECTOR_VENDOR_ID);
    }
#endif

    result = tcp_initiate_send_packet(connector_ptr, edp_header,
                                discovery_vendor_header_size,
                                E_MSG_MT2_TYPE_PAYLOAD,
                                tcp_release_packet_buffer,
                                NULL);
done:
    return result;
}

static connector_status_t send_device_type(connector_data_t * const connector_ptr)
{
    /*
     * packet format:
     *  --------------------------------------------------------------------
     * | 0 - 1 | 2 - 3 |    4     |     5       |    6 - 7    |    8...     |
     *  --------------------------------------------------------------------
     * |   EDP Header  | security | device type | Device type | Device type |
     * |               |  coding  |   opcode    |   length    |             |
     *  --------------------------------------------------------------------
    */
    enum edp_device_type_msg {
        field_define(edp_device_type, security_coding, uint8_t),
        field_define(edp_device_type, opcode, uint8_t),
        field_define(edp_device_type, length, uint16_t),
        record_end(edp_device_type)
    };
    size_t const device_type_header_size = record_bytes(edp_device_type);

    connector_status_t result;
    uint8_t * edp_header = NULL;
    uint8_t * edp_device_type;

#if (defined CONNECTOR_DEVICE_TYPE)
    static const char connector_device_type[] = CONNECTOR_DEVICE_TYPE;
    size_t device_type_length  = sizeof connector_device_type-1;
#else
    char * connector_device_type = connector_ptr->edp_data.config.device_type;
    size_t const device_type_length = connector_ptr->edp_data.config.device_type_length;
#endif

    edp_header = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &edp_device_type, NULL);
    if (edp_header == NULL)
    {
        result = connector_pending;
        goto done;
    }

    message_store_u8(edp_device_type, security_coding, SECURITY_PROTO_NONE);
    message_store_u8(edp_device_type, opcode, DISC_OP_DEVICETYPE);

	{
		uint16_t const length16 = (uint16_t) device_type_length;

		ASSERT(device_type_length <= UINT16_MAX);
		message_store_be16(edp_device_type, length, length16);
	}

    edp_device_type += device_type_header_size;
    memcpy(edp_device_type, connector_device_type, device_type_length);

    connector_debug_printf("Send device type = %.*s\n", (int)device_type_length, connector_device_type);

    result = tcp_initiate_send_packet(connector_ptr, edp_header,
                                (device_type_header_size + device_type_length),
                                E_MSG_MT2_TYPE_PAYLOAD,
                                tcp_release_packet_buffer,
                                NULL);
done:
    return result;
}

static connector_status_t send_complete(connector_data_t * const connector_ptr)
{
    /*
     * packet format:
     *  -----------------------------------------
     * | 0 - 1 | 2 - 3 |   4    |      5         |
     *  -----------------------------------------
     * |   EDP Header  | coding | initialization |
     * |               | scheme |   done opcode  |
     *  -----------------------------------------
    */
    enum edp_discovery_complete {
        field_define(edp_discovery_complete, security_coding, uint8_t),
        field_define(edp_discovery_complete, opcode, uint8_t),
        record_end(edp_discovery_complete)
    };
    size_t const discovery_complete_message_size = record_bytes(edp_discovery_complete);
    uint8_t * edp_discovery_complete;
    connector_status_t result;
    uint8_t * edp_header = NULL;

    edp_header = tcp_get_packet_buffer(connector_ptr, E_MSG_MT2_MSG_NUM, &edp_discovery_complete, NULL);
    if (edp_header == NULL)
    {
        result = connector_pending;
        goto done;
    }

    connector_debug_printf("Send complete\n");
    message_store_u8(edp_discovery_complete, security_coding, SECURITY_PROTO_NONE);
    message_store_u8(edp_discovery_complete, opcode, DISC_OP_INITCOMPLETE);

    result = tcp_initiate_send_packet(connector_ptr, edp_header,
                                discovery_complete_message_size,
                                E_MSG_MT2_TYPE_PAYLOAD,
                                tcp_release_packet_buffer,
                                NULL);

done:
    return result;
}

static connector_status_t layer_discovery_facility(connector_data_t * const connector_ptr);

static connector_status_t edp_tcp_open_process(connector_data_t * const connector_ptr)
{
    connector_status_t result = connector_idle;

    switch (edp_get_edp_state(connector_ptr))
    {
    case edp_communication_connect_server:
        if (connector_ptr->edp_data.keepalive.last_tx_received_time != 0)
        {
            if (connector_ptr->edp_data.keepalive.last_rx_sent_time == 0)
            {
                connector_debug_printf("Wait %d sec before connecting\n", connector_ptr->edp_data.keepalive.last_tx_received_time);
                if (get_system_time(connector_ptr, &connector_ptr->edp_data.keepalive.last_rx_sent_time) != connector_working)
                {
                    result = connector_abort;
                    goto done;
                }
            }
            if (is_valid_timing_limit(connector_ptr, connector_ptr->edp_data.keepalive.last_rx_sent_time, connector_ptr->edp_data.keepalive.last_tx_received_time))
            {
                /* within timing */
                goto done;
            }

            connector_ptr->edp_data.keepalive.last_rx_sent_time = 0;
            connector_ptr->edp_data.keepalive.last_tx_received_time = 0;
        }
        result = connect_server(connector_ptr, connector_ptr->edp_data.config.server_url, connector_ptr->edp_data.config.server_url_length);

        if (result == connector_working)
        {
            edp_set_edp_state(connector_ptr, edp_communication_send_version);
            connector_ptr->edp_data.send_packet.packet_buffer.in_use = connector_false;
            connector_ptr->edp_data.receive_packet.packet_buffer.in_use = connector_false;
            connector_ptr->edp_data.receive_packet.packet_buffer.next = NULL;
            connector_ptr->edp_data.receive_packet.free_packet_buffer = &connector_ptr->edp_data.receive_packet.packet_buffer;
        }
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        else if (result == connector_unavailable)
        {
            edp_set_active_state(connector_ptr, connector_transport_idle);
            connector_ptr->edp_data.stop.connect_action = connector_manual_connect;
        }
#endif
        break;

    case edp_communication_send_version:
    case edp_communication_send_keepalive:
    case edp_initialization_send_protocol_version:
    case edp_security_send_identity_verification:
    case edp_security_send_device_id:
    case edp_security_send_server_url:
    case edp_security_send_password:
    case edp_discovery_send_vendor_id:
    case edp_discovery_send_device_type:
    case edp_discovery_facility:
    case edp_discovery_send_complete:
    {
        connector_edp_state_t next_state = edp_get_edp_state(connector_ptr);

        switch (edp_get_edp_state(connector_ptr))
        {
        case edp_communication_send_version:
            connector_debug_printf("Send MT Version\n");
            result = send_version(connector_ptr, E_MSG_MT2_TYPE_VERSION, EDP_MT_VERSION);
            if (result == connector_working)
            {
                next_state = edp_communication_receive_version_response;
            }
            break;

        case edp_communication_send_keepalive:
            result = send_keepalive(connector_ptr);
            if (result == connector_working)
            {
                next_state =  edp_initialization_send_protocol_version;
            }
            break;

        case edp_initialization_send_protocol_version:
        {
            #define EDP_PROTOCOL_VERSION    0x120

            connector_debug_printf("Send protocol version\n");
            result = send_version(connector_ptr, E_MSG_MT2_TYPE_PAYLOAD, EDP_PROTOCOL_VERSION);
            if (result == connector_working)
            {
                next_state =  edp_initialization_receive_protocol_version;
            }
            break;
        }
        case edp_security_send_identity_verification:
            result = send_identity_verification(connector_ptr);
            if (result == connector_working)
            {
                next_state =  edp_security_send_device_id;
            }
            break;
        case edp_security_send_device_id:
            result = send_device_id(connector_ptr);
            if (result == connector_working)
            {
                next_state = edp_security_send_server_url;
            }
            break;
        case edp_security_send_server_url:
            result = send_server_url(connector_ptr);
            if (result == connector_working)
            {
#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)

#if (defined CONNECTOR_IDENTITY_VERIFICATION)
                next_state = (CONNECTOR_IDENTITY_VERIFICATION == connector_password_identity_verification) ? edp_security_send_password : edp_discovery_send_vendor_id;
#else
                next_state = (connector_ptr->edp_data.config.identity_verification == connector_password_identity_verification) ? edp_security_send_password : edp_discovery_send_vendor_id;
#endif

#else
                next_state =  edp_discovery_send_vendor_id;
#endif
            }
            break;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
        case edp_security_send_password:
            result = send_password(connector_ptr);
            if (result == connector_working)
            {
                next_state =  edp_discovery_send_vendor_id;
            }
            break;
#endif
        case edp_discovery_send_vendor_id:
            result = send_vendor_id(connector_ptr);
            if (result == connector_working)
            {
                next_state =  edp_discovery_send_device_type;
            }
            break;

        case edp_discovery_send_device_type:
            result = send_device_type(connector_ptr);
            if (result == connector_working)
            {
                next_state =  edp_discovery_facility;
            }
            break;
        case edp_discovery_facility:
            result = layer_discovery_facility(connector_ptr);

            if (result == connector_working)
            {
                next_state = edp_discovery_send_complete;
            }
            break;

        case edp_discovery_send_complete:
            result = send_complete(connector_ptr);
            break;

        default:
            break;
        }

        if (result == connector_working || result == connector_idle || result == connector_pending)
        {
            result = edp_tcp_send_process(connector_ptr);
            if (result == connector_working)
            {
                if (edp_get_edp_state(connector_ptr) == edp_discovery_send_complete)
                {
                    /* we are connected and EDP communication is fully established. */
                    edp_set_edp_state(connector_ptr, edp_facility_process);
                    edp_set_active_state(connector_ptr, connector_transport_receive);

    #if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)
                    if (notify_status(connector_ptr->callback, connector_tcp_communication_started) != connector_working)
                        result = connector_abort;
    #endif
               }

                edp_set_edp_state(connector_ptr, next_state);
            }
        }

        goto done;
    }
    case edp_communication_receive_version_response:
        result = receive_edp_version(connector_ptr);
        if (result == connector_working)
        {
            edp_set_edp_state(connector_ptr, edp_communication_send_keepalive);
        }

        break;

    case edp_initialization_receive_protocol_version:
        result = receive_protocol_version(connector_ptr);
        if (result == connector_working)
        {
            edp_set_edp_state(connector_ptr, edp_security_send_identity_verification);
        }
        break;

    case edp_facility_process:
        /* Should not be here since active state should not be open state. */
        ASSERT(connector_false);
        break;
    }

done:
    if (result != connector_idle && result != connector_pending && result != connector_working)
    {
        if (edp_get_edp_state(connector_ptr) != edp_communication_connect_server)
        {
           /* set the close state and make it goes to close connection state */
            edp_set_close_status(connector_ptr, connector_close_status_abort);
            edp_set_active_state(connector_ptr, connector_transport_close);
            result = connector_working;
        }
        else if (result == connector_open_error)
        {
            /* setup the 1 second delay for reconnection */
            connector_ptr->edp_data.keepalive.last_tx_received_time = 1;
            connector_ptr->edp_data.keepalive.last_rx_sent_time = 0;

#if (CONNECTOR_VERSION >= CONNECTOR_VERSION_1300)

#if (defined CONNECTOR_NETWORK_TCP_START)
            if (CONNECTOR_NETWORK_TCP_START == connector_manual_connect)
#else
            if (connector_ptr->edp_data.connect_type == connector_manual_connect)
#endif
            {
                /* Application must call initiate_action to start iDigi connector */
                edp_set_active_state(connector_ptr, connector_transport_idle);
            }
#endif

        }
    }
    return result;
}
