#ifndef BSP_LED_ALL
#define BSP_LED_ALL         0
#endif

#ifndef BSP_LED_ORANGE
#define BSP_LED_ORANGE      1
#endif

#ifndef BSP_LED_YELLOW
#define BSP_LED_YELLOW      2
#endif

#ifndef BSP_PB_START
#define BSP_PB_START        0
#endif

void connector_BSP_debug_printf(char const * const format, ...);
void Connector_BSP_software_reset(void);
void Connector_BSP_watchdog_reset(void);
void Connector_BSP_Serial_Read_Str(char *p_str, unsigned short len);
void Connector_BSP_LED_On(unsigned char led);
void Connector_BSP_LED_Off(unsigned char led);
void Connector_BSP_LED_Toggle(unsigned char led);
unsigned char Connector_BSP_Status_Read(unsigned char id);

