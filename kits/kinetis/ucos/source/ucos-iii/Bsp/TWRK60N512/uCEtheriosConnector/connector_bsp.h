#ifndef BSP_LED_ALL
#define BSP_LED_ALL     0
#endif


void Connector_BSP_software_reset(void);
void Connector_BSP_watchdog_reset(void);
void Connector_BSP_Serial_Read_Str(CPU_CHAR *p_str, CPU_INT16U len);
