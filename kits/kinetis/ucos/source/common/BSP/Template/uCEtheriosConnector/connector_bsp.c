#include <includes.h>
#include <connector_config.h>

/* Micrium BSPs don't have unified function names or functionalities.
   This file offers the user the possibility of implementing those BSP 
   functionalities required by the Etherios Cloud Connector core or sample applications
*/

#if (defined CONNECTOR_DEBUG)

void connector_BSP_debug_printf(char const * const format, ...)
{
#if defined(APP_TRACE)		// uCOS-III TRACE macro
    APP_TRACE((char *)format);
#elif defined(APP_CFG_TRACE)	// uCOS-II TRACE macro
    APP_CFG_TRACE((char *)format);
#else
    (void) format;
#endif
}
#endif


/* Implement here your BSP function to reset the module */
void Connector_BSP_software_reset(void)
{

}

/* Implement here your BSP function to read a string from the serial port */
void Connector_BSP_Serial_Read_Str(char *p_str, unsigned short len)
{
    // Frequently:
    // BSP_Ser_RdStr(p_str, len);
}

/* Implement here your BSP function to Turn On a led */
void Connector_BSP_LED_On(unsigned char led)
{
    // Frequently:
    // BSP_LED_On(led);
}

/* Implement here your BSP function to Turn Off a led */
void Connector_BSP_LED_Off(unsigned char led)
{
    // Frequently:
    // BSP_LED_Off(led);
}

/* Implement here your BSP function to Toggle a led */
void Connector_BSP_LED_Toggle(unsigned char led)
{
    // Frequently:
    // BSP_LED_Toggle(led);
}

/* Implement here your BSP function to read a push button */
unsigned char Connector_BSP_Status_Read(unsigned char  id)
{
#if 0
    // Frequently:  
    // return BSP_StatusRd(id);
#else
    return DEF_NO;
#endif
}
