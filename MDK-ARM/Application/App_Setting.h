#ifndef __SETTIME_H
#define __SETTIME_H

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Freertos_Task.h"
#include "App_config.h"

/* 进入设置界面（阻塞式，返回后回到调用者） */
void SetTime_Page(void);

#endif

