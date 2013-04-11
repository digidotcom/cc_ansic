/*
 * sdcard_task.h
 *
 *  Created on: Feb 28, 2013
 *      Author: spastor
 */

#ifndef SDCARD_TASK_H_
#define SDCARD_TASK_H_

#include <mfs.h>

typedef struct {
   MQX_FILE_PTR DEV_FD_PTR;
   MQX_FILE_PTR PM_FD_PTR;
   MQX_FILE_PTR FS_FD_PTR;
   char_ptr    DEV_NAME;
   char_ptr    PM_NAME;
   char_ptr    FS_NAME;
} FILESYSTEM_INFO_STRUCT, * FILESYSTEM_INFO_STRUCT_PTR;

extern FILESYSTEM_INFO_STRUCT_PTR  filesystem_info;
void sdcard_task(uint_32 temp);
#endif /* SDCARD_TASK_H_ */
