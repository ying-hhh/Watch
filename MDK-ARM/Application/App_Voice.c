#include "App_Voice.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "OLED.h"
#include <stdio.h> // 引入 printf 用于调试

extern UART_HandleTypeDef huart2;

static uint8_t rx_byte;
static uint8_t rx_step = 0;
static uint8_t voice_cmd_temp = 0;

// 串口中断
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        if (rx_step == 0 && rx_byte == 0x00) {
            rx_step = 1; 
        } else if (rx_step == 1) {
            voice_cmd_temp = rx_byte;
            rx_step = 0;

            // 打印调试：证明收到了有效指令
            //printf("Voice ISR RX: 0x%02X\r\n", voice_cmd_temp);
            
            // 【核心】把钥匙递给调度中枢
            xTaskNotifyFromISR(voice_task_handle, 
                               (uint32_t)voice_cmd_temp, 
                               eSetValueWithOverwrite, 
                               &xHigherPriorityTaskWoken);
        }
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void Voice_Task(void *args)
{
    uint32_t notify_value = 0;

    // 初始化串口接收
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

    for (;;)
    {
        // 1. 在门口死等钥匙
        xTaskNotifyWait(0, 0xFFFFFFFF, &notify_value, portMAX_DELAY);
        if (notify_value == 0) continue;

        // 打印调试：证明门卫醒了
        //printf("Voice Task Get: 0x%02X\r\n", notify_value);

        // 2. 拿到钥匙，统一处理！(先挂起老的，再唤醒新的，中间清屏)
        TaskHandle_t previous_task = g_current_active_task;
        TaskHandle_t next_task = NULL;

        switch (notify_value)
        {
            case 0x01: next_task = menu_task_handle;            break;
            case 0x02: next_task = menu_task_handle;            break; // 返回默认回菜单
            case 0x03: next_task = stopwatch_task_handle;       break;
            case 0x04: next_task = flashlight_task_handle;      break;
            case 0x05: next_task = emoji_task_handle;           break;
            default: continue; // 无效钥匙扔掉
        }

        // 3. 执行开门逻辑
        if (next_task != previous_task && next_task != NULL)
        {
            //printf("Switching Task...\r\n");

            // ★ 极其关键：在切换任务前，先强制清屏，打断可能正在进行的屏幕刷新
            OLED_Clear();
            OLED_Update();

            // 第一步：挂起当前任务（防止它继续画画）
            if (previous_task != NULL) {
                vTaskSuspend(previous_task);
            }

            // 第二步：更新记录
            g_current_active_task = next_task;

            // 第三步：唤醒新任务
            vTaskResume(next_task);
        }
    }
}

// #include "App_Voice.h"
// #include "usart.h"
// #include "FreeRTOS.h"
// #include "task.h"
// #include "OLED.h"

// extern UART_HandleTypeDef huart2;

// static uint8_t rx_byte;

// /* ============ 串口接收中断 ============ */

// void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//     if (huart->Instance == USART2)
//     {
//         BaseType_t xHigherPriorityTaskWoken = pdFALSE;

//         /* 
//          * 直接将收到的指令字节通过任务通知发送给 VoiceTask 
//          * 参数：xTaskToNotify, ulValue, eAction, pxHigherPriorityTaskWoken
//          */
//         xTaskNotifyFromISR(voice_task_handle,
//                            (uint32_t)rx_byte,
//                            eSetValueWithOverwrite,
//                            &xHigherPriorityTaskWoken);

//         /* 触发任务切换（如果需要） */
//         portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

//         /* 重新开启接收中断 */
//         HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
//     }
// }

// /* ============ 语音调度任务 ============ */

// void Voice_Task(void *args)
// {
//     /* 初始化：开启串口接收中断 */
//     HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

//     uint32_t notify_value = 0;

//     for (;;)
//     {
//         /* 
//          * 使用 FreeRTOS 原生 API 阻塞等待通知 
//          * 参数：ulBitsToClearOnEntry, ulBitsToClearOnExit, pulNotificationValue, xTicksToWait
//          */
//         xTaskNotifyWait(0,                // 进入时不清除
//                         0xFFFFFFFF,       // 退出时清除所有位
//                         &notify_value,
//                         portMAX_DELAY);   // 无限阻塞等待

//         if (notify_value == 0) continue;

//         // 1. 无论切换到哪里，先清屏，防止UI混合！
//         OLED_Clear();
//         OLED_Update();

//         // 2. 提取当前的任务句柄
//         TaskHandle_t previous_task = g_current_active_task;

//         /* 根据通知值执行指令 */
//         switch (notify_value)
//         {
//             case 0x01:  // "打开菜单"
//                 g_current_active_task = menu_task_handle;
//                 if (previous_task != menu_task_handle) vTaskSuspend(previous_task);
//                 vTaskResume(menu_task_handle);
//                 break;

//             case 0x02:  // "返回"
//                 g_current_active_task = menu_task_handle;
//                 if (previous_task == menu_task_handle && previous_task != NULL)
//                 {
//                     vTaskSuspend(menu_task_handle);
//                     vTaskResume(clock_task_handle);
//                 }
//                 else if (previous_task != menu_task_handle && previous_task != NULL)
//                 {
//                     vTaskSuspend(previous_task);
//                     vTaskResume(menu_task_handle);
//                 } 
//                 break;

//             case 0x03:  // "秒表"
//                 g_current_active_task = stopwatch_task_handle;
//                 if (previous_task != stopwatch_task_handle) vTaskSuspend(previous_task);
//                 vTaskResume(stopwatch_task_handle);
//                 break;

//             case 0x04:  // "手电筒"
//                 g_current_active_task = flashlight_task_handle;
//                 if (previous_task != flashlight_task_handle) vTaskSuspend(previous_task);
//                 vTaskResume(flashlight_task_handle);
//                 break;

//             case 0x05:  // "关闭/回到时钟"
//                 g_current_active_task = clock_task_handle;
//                 if (previous_task != clock_task_handle) vTaskSuspend(previous_task);
//                 vTaskResume(clock_task_handle);
//                 break;

//             default:
//                 break;
//         }
//     }
// }
