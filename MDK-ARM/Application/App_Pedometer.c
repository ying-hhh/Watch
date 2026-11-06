#include "App_Pedometer.h"
#include "OLED.h"
#include "Key.h"
#include "MPU6050.h"
#include "App_MPU6050.h"
#include <math.h>

/* ==================== 算法参数宏定义 ==================== */
#define PEDO_SAMPLE_MS        20      /* 采样周期 20ms (50Hz) */
#define PEDO_TIME_WINDOW      300     /* 步间隔最短时间 250ms */
#define PEDO_LPF_ALPHA        0.2f    /* 低通滤波系数 */
#define PEDO_DYNAMIC_ALPHA    0.1f    /* 动态阈值衰减系数 */
#define PEDO_BASE_THRESHOLD   0.15f   /* 基础阈值 (单位: g) */

/* ==================== 计步状态机变量 ==================== */
static uint32_t Step_Count = 0;
static float    LPF_Value = 1.0f;     
static float    Dynamic_Threshold = PEDO_BASE_THRESHOLD;
static uint8_t  Peak_Flag = 0;        
static TickType_t Last_Step_Tick = 0; 

/**
  * 函    数：重置计步器
  */
static void Pedometer_Reset(void)
{
    Step_Count = 0;
    LPF_Value = 1.0f;
    Dynamic_Threshold = PEDO_BASE_THRESHOLD;
    Peak_Flag = 0;
    Last_Step_Tick = 0;
}

/**
  * 函    数：核心计步算法处理（只认正向波峰）
  */
static void Pedometer_Process(float acc_g)
{
    /* 1. 一阶低通滤波 */
    LPF_Value = PEDO_LPF_ALPHA * acc_g + (1.0f - PEDO_LPF_ALPHA) * LPF_Value;

    /* 2. 只取正向波动 (合加速度大于 1.0g 的部分) */
    float val = LPF_Value - 1.0f;

    /* 3. 峰值检测状态机 */
    if (val > Dynamic_Threshold)
    {
        if (Peak_Flag == 0) // 刚越过阈值，上升沿
        {
            Peak_Flag = 1;
            TickType_t current_tick = xTaskGetTickCount();
            
            /* 时间窗防抖：距离上次计步超过 250ms */
            if ((current_tick - Last_Step_Tick) > pdMS_TO_TICKS(PEDO_TIME_WINDOW))
            {
                Step_Count++;
                Last_Step_Tick = current_tick;
            }
        }
    }
    else
    {
        /* 回落到阈值以下 */
        if (Peak_Flag == 1)
        {
            Peak_Flag = 0;
        }
    }

    /* 4. 动态阈值调整 */
    Dynamic_Threshold = PEDO_DYNAMIC_ALPHA * val + (1.0f - PEDO_DYNAMIC_ALPHA) * Dynamic_Threshold;
    
    /* 限制阈值范围，防止过小导致静止抖动计步，或过大导致走快了测不到 */
    if (Dynamic_Threshold < PEDO_BASE_THRESHOLD) Dynamic_Threshold = PEDO_BASE_THRESHOLD;
    if (Dynamic_Threshold > 0.5f) Dynamic_Threshold = 0.5f;
}

/**
  * 函    数：绘制UI
  */
static void Show_Pedometer_UI(void)
{
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_ShowString(0, 16, "Step:", OLED_8X16);
    OLED_ShowNum(48, 16, Step_Count, 6, OLED_8X16);
}

/* ==================== 测试模式：自动模拟波形 ==================== */
/* 打开此宏，将不读取真实的 MPU6050，而是生成完美的正弦波形 
#define PEDO_TEST_MODE  1 开启测试
#define PEDO_TEST_MODE  0 关闭测试 开启实际环境测试
*/
#define PEDO_TEST_MODE  0 

#if PEDO_TEST_MODE
void Pedometer_Page(void)
{
    float simulated_acc_g;
    TickType_t last_tick = xTaskGetTickCount();
    
    Pedometer_Reset();
    OLED_Clear();
    OLED_ShowString(0, 0, "Algorithm Test", OLED_8X16);
    OLED_ShowString(0, 16, "Target: 100", OLED_8X16);
    OLED_Update();
    vTaskDelay(pdMS_TO_TICKS(1000));

    /* 生成 100 步波形 */
    for (int step = 0; step < 100; step++)
    {
        /* 模拟正常步行，频率约 2Hz (500ms一步) */
        for (int t = 0; t < 500; t += 20) 
        {
            /* 
              正弦波模拟：
              t=0    -> 1.0g (平稳)
              t=125  -> 1.3g (加速度最大)
              t=250  -> 1.0g (平稳)
              t=375  -> 0.7g (失重最小)
              t=500  -> 1.0g (平稳)
            */
            simulated_acc_g = 1.0f + 0.3f * sinf(t * 3.14159f / 250.0f);
            
            /* 将模拟数据送入核心算法 */
            Pedometer_Process(simulated_acc_g);
            
            /* 实时刷新屏幕 */
            OLED_Clear();
            OLED_ShowImage(0, 0, 16, 16, Return);
            OLED_Printf(0, 0, OLED_8X16, "Time: %d", step);
            OLED_Printf(0, 16, OLED_8X16, "Step: %d", Step_Count);
            
            /* 屏幕上画一个点显示当前的加速度波动 */
            int16_t y = 50 - (int16_t)((simulated_acc_g - 1.0f) * 40);
            OLED_DrawPoint((t / 4) + 10, y);
            
            OLED_Update();

            /* 模拟真实的 20ms 采样间隔 (FreeRTOS 的 1 tick = 1ms) */
            vTaskDelay(pdMS_TO_TICKS(PEDO_SAMPLE_MS));
            
            /* 为了方便你按返回键结束测试 */
            KeyEvent_t evt;
            if (Key_Poll(&evt, 0) == pdTRUE && evt == KEY_EVENT_3) {
                return; // 退出测试
            }
        }
    }

    /* 测试结束，将结果固定显示在屏幕上 */
    OLED_Clear();
    OLED_ShowString(0, 0, "Test Over!", OLED_8X16);
    OLED_Printf(0, 16, OLED_8X16, "Target: 100", OLED_8X16);
    OLED_Printf(0, 32, OLED_8X16, "Actual: %d", Step_Count);
    OLED_Update();

    while(1)
    {
        KeyEvent_t evt;
        if (Key_Poll(&evt, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (evt == KEY_EVENT_3) {
                xTaskNotifyGive(menu_task_handle);
                return;
            }
        }
    }
}
#else
/* ==================== 真实环境正常运行 ==================== */
void Pedometer_Page(void)
{
    int16_t ax, ay, az;
    TickType_t last_tick = xTaskGetTickCount();
    uint8_t is_running = 1; 
    KeyEvent_t evt;

    OLED_Clear();
    OLED_Update();

    while (1)
    {
        if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (evt == KEY_EVENT_2) 
            {
                is_running = !is_running; 
            }
            else if (evt == KEY_EVENT_3) 
            {
                OLED_Clear();
                OLED_Update();
                xTaskNotifyGive(menu_task_handle);
                return;
            }
            else if (evt == KEY_EVENT_3_LONG) 
            {
                Pedometer_Reset();
            }
        }

        if ((xTaskGetTickCount() - last_tick) >= pdMS_TO_TICKS(PEDO_SAMPLE_MS))
        {
            last_tick = xTaskGetTickCount();

            if (is_running)
            {
                MPU6050_ReadRaw(&ax, &ay, &az);
                float acc_g = sqrtf((float)ax * ax + (float)ay * ay + (float)az * az) / 16384.0f;
                Pedometer_Process(acc_g);
            }
        }

        Show_Pedometer_UI();
        OLED_Update();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
#endif
