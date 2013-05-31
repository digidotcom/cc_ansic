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

#ifndef __PARSING_UTILS_H__
#define __PARSING_UTILS_H__

#include <mqx.h>
#include <rtcs.h>

int get_line(char **lineptr, size_t limit);
int hexstrtobyte (const char *p);
int mac_parse(_enet_address enet_address, const char *str);
int vendor_parse(uint32_t *vendor_id, char *str);

int server_url_parse(char *server_url);
#endif /* __PARSING_UTILS_H__ */
