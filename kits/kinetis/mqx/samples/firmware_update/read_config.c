/*
 * read_config.c
 *
 *  Created on: Apr 19, 2013
 *      Author: spastor
 */

#include "main.h"
#include "platform.h"
#include <ctype.h>

#define SERVERSTR_MAXLEN	30
static char custom_server_url[SERVERSTR_MAXLEN];
static _enet_address custom_mac_addr = {0};
static uint32_t custom_vendor_id;


/*
 *	brief		This function reads a line from the serial console. Can be
 *				invoked in blocking and non blocking mode.
 *	param[in]	lineptr	pointer to the char array where the string read will
 *              be stored.
 *	param[in]	limit	max number of characters for the string that will 
 *              be read.
 *	retval     	The length of the line read
 *	retval 		MQX_EINVAL on error
 */
static int get_line(char **lineptr, size_t limit)
{
	static uint8_t lineidx = 0;
	size_t retval;
	uint8_t c;
	
	if (lineptr == NULL || limit == 0)
		return MQX_EINVAL;

	for(;;)
	{
		c = getchar();
		switch(c) {
		case '\r':
		case '\n':
			if (lineptr[0])
				goto gotline; /* Only if the line is not null */
			else
				break;

		case '\b':
			if (lineidx) {
				printf("\b \b");
				lineidx--;
			}
			break;
			
		/* Cursors are not supported... */

		default:
			(*lineptr)[lineidx++] = (char)c;
			if (lineidx == (limit - 1))
				goto gotline;
			break;
		}
	}	
	return 0;

gotline:
	(*lineptr)[lineidx++] = '\0';
	retval = lineidx;
	lineidx = 0;
	return retval;
}

static int hexstrtobyte (const char *p)
{
	uint8_t b = 0;
	char ch;
	int8_t i;

	for (i = 2; i; --i)
	{
		b <<= 4;
		ch = *p++;
		if ('0' <= ch && ch <= '9')
		{
			b += ch - '0';
		}
		else if ('a' <= ch && ch <= 'f')
		{
			b += ch - ('a' - 10);
		}
		else if ('A' <= ch && ch <= 'F')
		{
			b += ch - ('A' - 10);
		}
		else
		{
			return -1;
		}
	}

	return b;
}

/*
	brief	Parse a text string into a 64-bit IEEE address.

	Converts a text string with eight 2-character hex values, with an optional
	separator between any two values.  For example, the following formats are
	all valid:
		- 01-23-45-67-89-ab
		- 012345-6789AB
		- 01:23:45:67:89:aB
		- 0123 4567 89AB
	param[out]	enet_address	converted address (stored big-endian)
	param[in]	str		string to convert, starting with first hex character

	retval	0	invalid parameters passed to function; if \a address is
							not NULL, it will be set to all zeros
	retval	1			string converted
*/
static int mac_parse(_enet_address enet_address, const char *str)
{
	uint8_t i;
	uint8_t *b;
	int ret;

	i = 6;			// bytes to convert
	if (str != NULL && enet_address != NULL)
	{
		for (b = enet_address; i; ++b, --i)
		{
			// skip over any separator, if present
			while (*str && (*str == ' ' || *str == ':' || *str == '-')) 
				str++;
			ret = hexstrtobyte(str);
			*b = (uint8_t)ret;
			str += 2;					// point past the encoded byte
		}
	}

	if (i == 0)			// successful conversion
	{
		return 1;
	}

	return 0;
}

static int vendor_parse(uint32_t *vendor_id, char *str)
{
	uint8_t i;
	int ret;
	
	if (str == NULL) {
		return 0;
	}
	
	while (*str && (*str == ' ' || *str == ':' || *str == '-')) 
		str++;
	
	*vendor_id = strtol(str, NULL, 16);
	if (!*vendor_id) {
		return 0; /* No conversion */
	}
	
	return 1;
}

static int server_url_parse(char *server_url, char * str)
{
	uint8_t i;
	int valid_url = 0;
	char *str_index = str;
	
	if (str == NULL || server_url == NULL) {
		return 0;
	}
	
	/* skip over any separator, if present */
	while (*str_index && *str_index == ' ') 
		str_index++;
	
	/* blabla.etherios.co.uk OR blabla.etherios.com */
	while (*str_index && *str_index != '.') 
		str_index++;
	if (*str_index == '.') {
		str_index++; /* skip the '.' */
	} else {
		return 0; /* End of Line */
	}
	
	if (strncmp(str_index, "etherios.co", sizeof("etherios.co") - 1))
		return 0;
	
	if (strncmp(str_index + strlen("etherios.co"), "m", sizeof("m") - 1) != 0 &&
		strncmp(str_index + strlen("etherios.co"), ".uk", sizeof(".uk") - 1) != 0) {
		return 0;
	}

	strcpy(custom_server_url, str);
	
	return 1;
}


connector_callback_status_t app_get_server_url(connector_config_pointer_string_t * const config_url)
{
	config_url->string = custom_server_url;
	config_url->length = strlen(custom_server_url);
	
	return connector_callback_continue;
}

connector_callback_status_t app_get_vendor_id(connector_config_vendor_id_t * const config_vendor_id)
{
	config_vendor_id->id = custom_vendor_id;
	
	return connector_callback_continue;
}

connector_callback_status_t app_custom_get_mac_addr(unsigned char * config_mac)
{
	memcpy(config_mac, custom_mac_addr, sizeof (custom_mac_addr));
	
	return connector_callback_continue;
}
void read_mac(void)
{
	char linebuffer[25];
	char *p_linebuffer = linebuffer;
	uint8_t got_mac = 0;

	do {
		printf("Enter the MAC address (aabbcc:ddeeffgg): ");
		get_line(&p_linebuffer, sizeof (linebuffer));
		putchar('\n');
		got_mac = mac_parse(custom_mac_addr, linebuffer);
	} while (!got_mac);
}

void read_vendorid(void)
{
	char linebuffer[25];
	char *p_linebuffer = linebuffer;
	uint8_t got_vendor = 0;

	do {
		printf("Enter Vendor ID (aabbccdd): ");
		get_line(&p_linebuffer, sizeof (linebuffer));
		putchar('\n');
		got_vendor = vendor_parse(&custom_vendor_id, linebuffer);
	} while (!got_vendor);
}

void read_server_url(void)
{
	char linebuffer[25];
	char *p_linebuffer = linebuffer;
	uint8_t got_server_url = 0;

	do {
		printf("Enter server URL: ");
		get_line(&p_linebuffer, sizeof (linebuffer));
		putchar('\n');
		got_server_url = server_url_parse(custom_server_url, linebuffer);
	} while (!got_server_url);
}

static int parse_ip(_ip_address *ip_address, char * str)
{
	#define AUX_BUF_LEN	6
	uint8_t i, j;
	int valid_ip = 1;
	char *str_index = str;
	char aux_buffer[AUX_BUF_LEN];
	int ip_bytes[4];
	
	if (str == NULL || ip_address == NULL) {
		return 0;
	}
	
	/* skip over any separator, if present */
	while (*str_index && isspace(*str_index)) 
		str_index++;
	if (str_index == '\0')
		return 0;
	
	for (i = 0; i < 4; i++) {
		for (j = 0; j < AUX_BUF_LEN && *str_index; j++, str_index++) {
			if (*str_index != '.') {
				*(aux_buffer + j) = *str_index;
			} else {
				str_index++;
				break;
			}
		}
		*(aux_buffer + j) = '\0'; /* End String */

		ip_bytes[3 - i] = strtol(aux_buffer, NULL, 10);
	}
	
	*ip_address = IPADDR(ip_bytes[3], ip_bytes[2], ip_bytes[1], ip_bytes[0]);
	
	return 1;
}

static void read_ipconfig_static(network_start_params_t *network_parameters)
{
	char linebuffer[25];
	char *p_linebuffer = linebuffer;
	
	do {
		printf("Enter IP: ");
		get_line(&p_linebuffer, sizeof (linebuffer));
	} while (!parse_ip(&network_parameters->ip_data.ip, linebuffer));
	
	do {
		printf("Enter Network Mask: ");
		get_line(&p_linebuffer, sizeof (linebuffer));
	} while (!parse_ip(&network_parameters->ip_data.mask, linebuffer));
	
	do {
		printf("Enter Gateway IP: ");
		get_line(&p_linebuffer, sizeof (linebuffer));
	} while (!parse_ip(&network_parameters->ip_data.gateway, linebuffer));
	
	do {
		printf("Enter DNS IP: ");
		get_line(&p_linebuffer, sizeof (linebuffer));
	} while (!parse_ip(&network_parameters->dns_server, linebuffer));
}

void read_ipconfig(network_start_params_t *network_parameters)
{
	char linebuffer[25];
	char *p_linebuffer = linebuffer;
	int got_ip_type;
	
	do {
		printf("Use (D)ynamic or (S)tatic IP? ");
		get_line(&p_linebuffer, sizeof (linebuffer));
		while (*p_linebuffer && isspace(*p_linebuffer)) 
			p_linebuffer++;
		if (*p_linebuffer == 'D') {
			network_parameters->ip_type = ip_dynamic;
			got_ip_type = 1;
		} else if (*p_linebuffer == 'S') {
			network_parameters->ip_type = ip_static;
			read_ipconfig_static(network_parameters);
			got_ip_type = 1;
		} else {
			got_ip_type = 0;
		}
	} while (!got_ip_type);
	
	
}

