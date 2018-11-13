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

#ifndef CONNECTOR_API_STREAMING_CLI_H
#define CONNECTOR_API_STREAMING_CLI_H

#if (defined CONNECTOR_STREAMING_CLI_SERVICE)

#ifndef CONNECTOR_STREAMING_CLI_MAX_CLOSE_REASON_LENGTH
#define CONNECTOR_STREAMING_CLI_MAX_CLOSE_REASON_LENGTH	1024
#else
#if CONNECTOR_STREAMING_CLI_MAX_CLOSE_REASON_LENGTH < 30
#error "CONNECTOR_STREAMING_CLI_MAX_CLOSE_REASON_LENGTH must be greater than 30"
#endif
#endif

typedef enum {
	connector_request_id_streaming_cli_session_start,
	connector_request_id_streaming_cli_poll,
	connector_request_id_streaming_cli_send,
	connector_request_id_streaming_cli_receive,
	connector_request_id_streaming_cli_session_end
} connector_request_id_streaming_cli_service_t;

typedef struct {
	void * handle;
	char * CONST error_hint;
	enum {
		connector_cli_session_start_ok,
		connector_cli_session_start_error
	} session_start_status;
} connector_streaming_cli_session_start_request_t;

typedef struct {
	void * CONST handle;
	char * CONST error_hint;
} connector_streaming_cli_session_end_request_t;

typedef struct {
	void * CONST handle;
	char * CONST error_hint;
	enum {
		connector_cli_session_state_idle,
		connector_cli_session_state_readable,
		connector_cli_session_state_done
	} session_state;
} connector_streaming_cli_poll_request_t;

typedef struct {
	void * CONST handle;
	size_t CONST bytes_available;
	size_t bytes_used;
	uint8_t * CONST buffer;
	connector_bool_t more_data;
} connector_streaming_cli_session_send_data_t;

typedef struct {
	void * CONST handle;
	size_t CONST bytes_available;
	size_t bytes_used;
	uint8_t CONST * CONST buffer;
	connector_bool_t CONST more_data;
} connector_streaming_cli_session_receive_data_t;

#endif

#if !defined _CONNECTOR_API_H
#error  "Illegal inclusion of connector_api_streaming_cli.h. You should only include connector_api.h in user code."
#endif

#else
#error  "Illegal inclusion of connector_api_streaming_cli.h. You should only include connector_api.h in user code."
#endif
