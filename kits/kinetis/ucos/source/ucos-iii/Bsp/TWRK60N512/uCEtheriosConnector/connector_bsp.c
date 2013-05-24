#include <bsp.h>

void Connector_BSP_software_reset(void)
{
    #define VECTKEY  0x05FA0000

    // Issue a System Reset Request
    SCB_AIRCR = VECTKEY | SCB_AIRCR_SYSRESETREQ_MASK;
}

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
