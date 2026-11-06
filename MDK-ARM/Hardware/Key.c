#include "Key.h"
#include "main.h"

static uint8_t  Key_Num = 0;
static int      press_time = 0;

QueueHandle_t g_KeyQueue = NULL;

/* ==================== 初始化 ==================== */

void Key_Init(void)
{
    /* GPIO 已由 CubeMX 配置好 */
    g_KeyQueue = xQueueCreate(8, sizeof(KeyEvent_t));   /* 最多缓存 8 个事件 */
    configASSERT(g_KeyQueue != NULL);
}

/* ==================== 读取物理按键 ==================== */

static uint8_t Key_GetState(void)
{
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
        return KEY_EVENT_1;
    else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET)
        return KEY_EVENT_2;
    else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
    {
        if (press_time > KEY_LONG_PRESS_THRESHOLD)
            return KEY_EVENT_3_LONG;
        else
            return KEY_EVENT_3;
    }
    return KEY_EVENT_NONE;
}

static void Key3_Tick(void)
{
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
        press_time++;
    else
        press_time = 0;
}

/* ==================== 每 10ms 调用一次 ==================== */

void Key_Tick(void)
{
    static uint8_t CurrentState = 0;
    static uint8_t PreState     = 0;

    Key3_Tick();

    PreState     = CurrentState;
    CurrentState = Key_GetState();

    /* 松手瞬间产生事件，写入队列 */
    if (PreState != KEY_EVENT_NONE && CurrentState == KEY_EVENT_NONE)
    {
        KeyEvent_t evt = (KeyEvent_t)PreState;
        xQueueSend(g_KeyQueue, &evt, 0);   /* 队列满就丢弃，不阻塞 */
    }
}

/* ==================== 任务消费接口 ==================== */

BaseType_t Key_Poll(KeyEvent_t *evt, TickType_t timeout)
{
    if (evt == NULL) return pdFALSE;
    return xQueueReceive(g_KeyQueue, evt, timeout);
}

void Key_Flush(void)
{
    KeyEvent_t tmp;
    while (xQueueReceive(g_KeyQueue, &tmp, 0) == pdTRUE) { /* 清空 */ }
}












//#include "Key.h"

///* ===================== 全局变量 ===================== */
//uint8_t  Key_Num = 0;       // 按键事件缓存（供 Key_GetNum 读取）
//int      press_time = 0;    // PA4 按下持续计数（用于判断长按）

///* ===================== 初始化 =====================
// * 说明：GPIO 已由 CubeMX 在 MX_GPIO_Init() 中配置为
// *       上拉输入模式，此处无需重复初始化。
// *       若你希望保持与标准库一致的调用习惯，可保留空函数。
// */
//void Key_Init(void)
//{
//    /* 无需操作，GPIO 由 CubeMX 自动生成初始化代码 */
//    /* 若使用非 CubeMX 工程，可在此添加：
//     *   __HAL_RCC_GPIOA_CLK_ENABLE();
//     *   __HAL_RCC_GPIOB_CLK_ENABLE();
//     *   GPIO_InitTypeDef GPIO_InitStruct = {0};
//     *   GPIO_InitStruct.Pin  = GPIO_PIN_1;
//     *   GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//     *   GPIO_InitStruct.Pull = GPIO_PULLUP;
//     *   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//     *   GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_6;
//     *   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//     */
//}

///* ===================== 读取按键编号 =====================
// * 有事件时返回对应编号，同时清零；无事件返回 0。
// * 应在主循环中轮询调用。
// */
//uint8_t Key_GetNum(void)
//{
//    uint8_t Temp;
//    if (Key_Num)
//    {
//        Temp = Key_Num;
//        Key_Num = 0;
//        return Temp;
//    }
//    return 0;
//}

///* ===================== PA4 长按计时 =====================
// * 每 20ms 调用一次：
// *   - 按下时计数递增
// *   - 松开时清零
// */
//void Key3_Tick(void)
//{
//    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
//    {
//        press_time++;
//    }
//    else
//    {
//        press_time = 0;
//    }
//}

///* ===================== 读取当前按键状态 =====================
// * 返回值：
// *   0 —— 无按键
// *   1 —— PB1 按下
// *   2 —— PA6 按下
// *   3 —— PA4 短按（未达长按阈值）
// *   4 —— PA4 长按（超过阈值）
// */
//uint8_t Key_GetState(void)
//{
//    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
//    {
//        return KEY_1;
//    }
//    else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET)
//    {
//        return KEY_2;
//    }
//    else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
//    {
//        if (press_time > KEY_LONG_PRESS_THRESHOLD)
//            return KEY_3_LONG;
//        else
//            return KEY_3;
//    }
//    return KEY_NONE;
//}

///* ===================== 按键消抖状态机 =====================
// * 每 20ms 调用一次：
// *   - 记录上一次状态（PreState）与当前状态（CurrentState）
// *   - 检测"按下→释放"边沿（PreState != 0 && CurrentState == 0）
// *   - 触发边沿时把 PreState 写入 Key_Num
// */
//void Key_Tick(void)
//{
//    static uint8_t CurrentState = 0;
//    static uint8_t PreState     = 0;

//    /* 先更新长按计时（仅影响 PA4） */
//    Key3_Tick();

//    PreState     = CurrentState;
//    CurrentState = Key_GetState();

//    /* 松手瞬间触发一次事件 */
//    if (PreState != 0 && CurrentState == 0)
//    {
//        Key_Num = PreState;
//    }
//}
