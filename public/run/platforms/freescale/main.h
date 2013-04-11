#ifndef __main_h_
#define __main_h_
#include <mqx.h>
#include <bsp.h>

#include <rtcs.h>
#ifdef APPLICATION_HAS_SHELL
#include <sh_rtcs.h>
#endif
#include <ipcfg.h>

#define MAIN_TASK 1
#define CONNECTOR_TASK 2
#define CONNECTOR_FLASH_TASK 3

#define ENET_DEVICE 0

extern void Main_task(uint_32);
extern void connector_thread(unsigned long initial_data);
extern void connector_flash_task(unsigned long initial_data);
extern int application_start(void);
#endif /* __main_h_ */

