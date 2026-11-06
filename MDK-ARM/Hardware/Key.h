#ifndef __KEY_H
#define __KEY_H

#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"

/* 按键事件类型 */
typedef enum
{
    KEY_EVENT_NONE = 0,
    KEY_EVENT_1,        /* PB1 */
    KEY_EVENT_2,        /* PA6 */
    KEY_EVENT_3,        /* PA4 短按 */
    KEY_EVENT_3_LONG,   /* PA4 长按 */
} KeyEvent_t;

#define KEY_LONG_PRESS_THRESHOLD    50

/* 队列句柄，外部可用 */
extern QueueHandle_t g_KeyQueue;

/* 初始化 */
void Key_Init(void);

/* 底层扫描，只被 Key_Task 调用 */
void Key_Tick(void);

/* 供任务消费队列的接口，返回 pdTRUE 表示有事件 */
BaseType_t Key_Poll(KeyEvent_t *evt, TickType_t timeout);

/* 清空队列里所有积压事件（可选） */
void Key_Flush(void);

#endif












//#ifndef __KEY_H
//#define __KEY_H

//#include "main.h"   // 引入 HAL 库定义（由 CubeMX 生成）

///* 按键编号定义（与 Key_GetState 返回值对应） */
//#define KEY_NONE        0
//#define KEY_1           1   // PB1
//#define KEY_2           2   // PA6
//#define KEY_3           3   // PA4 短按
//#define KEY_3_LONG      4   // PA4 长按

///* 长按阈值（单位：定时器 Tick 次数）
// * 假设定时器中断周期为 20ms，则 50 次 ≈ 1 秒
// * 原代码用的是 1000 次 ≈ 20 秒，可按需调整
// */
//#define KEY_LONG_PRESS_THRESHOLD    50

///* 函数声明 */
//void    Key_Init(void);         // 可选：GPIO 已由 CubeMX 初始化，此函数留作扩展
//uint8_t Key_GetNum(void);       // 获取按键编号（读取后自动清零）
//uint8_t Key_GetState(void);     // 读取当前按键状态（瞬时）
//void    Key_Tick(void);         // 20ms 周期调用：消抖 + 状态机
//void    Key3_Tick(void);        // 20ms 周期调用：长按计时

//#endif





