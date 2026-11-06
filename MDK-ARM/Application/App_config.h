#ifndef __APP_CONFIG_H
#define __APP_CONFIG_H

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t key_task_handle;
extern TaskHandle_t clock_task_handle;
extern TaskHandle_t setting_task_handle;
extern TaskHandle_t menu_task_handle;
extern TaskHandle_t stopwatch_task_handle;
extern TaskHandle_t flashlight_task_handle;
extern TaskHandle_t mpu6050_task_handle;
extern TaskHandle_t game_task_handle;
extern TaskHandle_t emoji_task_handle;
extern TaskHandle_t gradienter_task_handle;

extern TaskHandle_t voice_task_handle;

/* 全局记录当前活跃的前台任务，供“返回”指令使用 */
extern TaskHandle_t g_current_active_task;

// /* 语音调度中枢任务句柄 */
// extern TaskHandle_t VoiceDispatchTaskHandle; 


#endif
