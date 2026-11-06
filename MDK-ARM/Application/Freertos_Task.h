#ifndef __FREERTOS_TASK_H
#define __FREERTOS_TASK_H

#include "FreeRTOS.h"
#include "task.h"
#include "Debug.h"
#include "OLED.h"
#include "LED.h"
#include "MyRTC.h"
#include "MPU6050.h"
#include "App_config.h"
#include "App_Clock.h"
#include "App_Setting.h"
#include "App_Menu.h"
#include "App_Stopwatch.h"
#include "App_Flashlight.h"
#include "App_MPU6050.h"
#include "App_Game.h"
#include "App_Emoji.h"
#include "App_Gradienter.h"
#include "App_Voice.h"

//启动FReeRTOS
void FreeRTOS_Start(void);

#endif
