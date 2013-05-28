#include <includes.h>
#include <connector_config.h>

/* Micrium BSPs don't have unified function names or functionalities.
   This file offers the user the possibility of implementing those BSP 
   functionalities required by the Etherios Cloud Connector core or sample applications
*/

#if (defined CONNECTOR_DEBUG)
void connector_BSP_debug_printf(char const * const format, ...)
{
#if defined(APP_TRACE)		    // uCOS-III TRACE macro
    APP_TRACE((char *)format);
#else
    (void) format;
#endif
}
#endif

/* Implement here your BSP function to reset the module */
void Connector_BSP_software_reset(void)
{
    #define VECTKEY  0x05FA0000

    // Issue a System Reset Request
    SCB_AIRCR = VECTKEY | SCB_AIRCR_SYSRESETREQ_MASK;
}

//TODO: Remove, not used.
void Connector_BSP_watchdog_reset(void)
{
    /* Issue a watchdog */

    /* disable all interrupts */
    asm(" CPSID i");

    /* Write 0xC520 to the unlock register */ WDOG_UNLOCK = 0xC520;
 
    /* Followed by 0xD928 to complete the unlock */ WDOG_UNLOCK = 0xD928;
 
    /* enable all interrupts */
    asm(" CPSIE i");
 
    /* Clear the WDOGEN bit to disable the watchdog */ //WDOG_STCTRLH &= ~WDOG_STCTRLH_WDOGEN_MASK;

    WDOG_STCTRLH != WDOG_STCTRLH_WDOGEN_MASK;
}

/* Implement here your BSP function to read a string from the serial port */
void Connector_BSP_Serial_Read_Str(char *p_str, unsigned short len)
{
    BSP_Ser_RdStr(p_str, len);
}

/* Implement here your BSP function to Turn On a led */
void Connector_BSP_LED_On(unsigned char led)
{
    BSP_LED_On(led);
}

/* Implement here your BSP function to Turn Off a led */
void Connector_BSP_LED_Off(unsigned char led)
{
    BSP_LED_Off(led);
}

/* Implement here your BSP function to Toggle a led */
void Connector_BSP_LED_Toggle(unsigned char led)
{
    BSP_LED_Toggle(led);
}

/* Implement here your BSP function to read a push button */
unsigned char Connector_BSP_Status_Read(unsigned char id)
{
    return BSP_StatusRd(id);
}
