#include <mqx.h>
#include <rtcs.h>
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
int get_line(char **lineptr, size_t limit)
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

int hexstrtobyte (const char *p)
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

int mac_parse(_enet_address enet_address, const char *str)
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

int vendor_parse(uint32_t *vendor_id, char *str)
{
	uint8_t i;
	int ret;
	
	if (str == NULL) {
		return 0;
	}
	
	while (*str && (*str == ' ' || *str == ':' || *str == '-')) 
		str++;
	
	if (*str == '0' && (*(str + 1) == 'x' || *(str + 1) == 'x'))
		str += 2;

	*vendor_id = strtol(str, NULL, 16);
	if (!*vendor_id) {
		return 0; /* No conversion */
	}
	
	return 1;
}

int server_url_parse(char *server_url)
{
	uint8_t i;
	int valid_url = 0;
	char *str_index = server_url;
	
	if (server_url == NULL) {
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

	return 1;
}
