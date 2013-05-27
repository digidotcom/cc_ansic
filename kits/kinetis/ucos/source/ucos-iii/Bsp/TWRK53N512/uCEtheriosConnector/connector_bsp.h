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
void Connector_BSP_Serial_Read_Str(CPU_CHAR *p_str, CPU_INT16U len);
void Connector_BSP_LED_On(CPU_INT08U led);
void Connector_BSP_LED_Off(CPU_INT08U led);
void Connector_BSP_LED_Toggle(CPU_INT08U led);
CPU_BOOLEAN Connector_BSP_Status_Read(CPU_INT08U id);

