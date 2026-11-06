#include "App_Setting.h"
#include "OLED.h"
#include "MyRTC.h"
#include "Key.h"

/* 本地缓冲 */
static uint16_t s_Year, s_Month, s_Day, s_Hour, s_Min, s_Sec;

typedef enum
{
    PHASE_DATE = 0,     /* 年月日编辑 */
    PHASE_TIME,         /* 时分秒编辑 */
    PHASE_EXIT,
} SetPhase_t;

static uint8_t s_Field = 0;    /* 当前编辑的字段 0=年/时 1=月/分 2=日/秒 */
static SetPhase_t s_Phase = PHASE_DATE;

/* ============ 显示 ============ */

static void Show_Date_UI(void)
{
    OLED_Clear();
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_Printf(0, 16, OLED_8X16, "Year :%4d", s_Year);
    OLED_Printf(0, 32, OLED_8X16, "Month:%2d",  s_Month);
    OLED_Printf(0, 48, OLED_8X16, "Day  :%2d",  s_Day);
}

static void Show_Time_UI(void)
{
    OLED_Clear();
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_Printf(0, 16, OLED_8X16, "Hour :%2d", s_Hour);
    OLED_Printf(0, 32, OLED_8X16, "Min  :%2d", s_Min);
    OLED_Printf(0, 48, OLED_8X16, "Sec  :%2d", s_Sec);
}

static void Highlight_Field(void)
{
    if (s_Phase == PHASE_DATE)
    {
        switch (s_Field)
        {
            case 0: OLED_ReverseArea(0, 16, 80, 16); break;
            case 1: OLED_ReverseArea(0, 32, 80, 16); break;
            case 2: OLED_ReverseArea(0, 48, 80, 16); break;
        }
    }
    else
    {
        switch (s_Field)
        {
            case 0: OLED_ReverseArea(0, 16, 80, 16); break;
            case 1: OLED_ReverseArea(0, 32, 80, 16); break;
            case 2: OLED_ReverseArea(0, 48, 80, 16); break;
        }
    }
    OLED_Update();
}

/* ============ 边界钳位 ============ */

static void Clamp(void)
{
    if (s_Month < 1)  s_Month = 12;
    if (s_Month > 12) s_Month = 1;
    if (s_Day   < 1)  s_Day   = 31;
    if (s_Day   > 31) s_Day   = 1;
    if (s_Hour  > 23) s_Hour  = 0;
    if (s_Min   > 59) s_Min   = 0;
    if (s_Sec   > 59) s_Sec   = 0;
}

/* ============ 写回 RTC ============ */

static void Save_To_RTC(void)
{
    MyRTC_Time[0] = s_Year;
    MyRTC_Time[1] = s_Month;
    MyRTC_Time[2] = s_Day;
    MyRTC_Time[3] = s_Hour;
    MyRTC_Time[4] = s_Min;
    MyRTC_Time[5] = s_Sec;
    MyRTC_SetTime();
    MyRTC_ReadTime();
}

/* ============ SetTime 任务 ============ */

void SetTime_Page(void)
{
    KeyEvent_t evt;

    while(1)
    {
        /* ★ 1. 阻塞等待 Clock 通知（一开始会在这里等） */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        /* ★ 2. 被通知后，加载当前时间，进入编辑 */

        /* ---------- 进入时先加载当前时间 ---------- */
        MyRTC_ReadTime();
        s_Year  = MyRTC_Time[0];
        s_Month = MyRTC_Time[1];
        s_Day   = MyRTC_Time[2];
        s_Hour  = MyRTC_Time[3];
        s_Min   = MyRTC_Time[4];
        s_Sec   = MyRTC_Time[5];

        s_Phase = PHASE_DATE;
        s_Field = 0;

        Key_Flush();

        /* ---------- 编辑循环 ---------- */
        for (;;)
        {
            if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
            {
                if (evt == KEY_EVENT_1)      /* 加 */
                {
                    if (s_Phase == PHASE_DATE)
                    {
                        if (s_Field == 0) s_Year++;
                        else if (s_Field == 1) s_Month++;
                        else s_Day++;
                    }
                    else
                    {
                        if (s_Field == 0) s_Hour++;
                        else if (s_Field == 1) s_Min++;
                        else s_Sec++;
                    }
                    Clamp();
                }
                else if (evt == KEY_EVENT_2) /* 减 */
                {
                    if (s_Phase == PHASE_DATE)
                    {
                        if (s_Field == 0) s_Year--;
                        else if (s_Field == 1) s_Month--;
                        else s_Day--;
                    }
                    else
                    {
                        if (s_Field == 0) s_Hour--;
                        else if (s_Field == 1) s_Min--;
                        else s_Sec--;
                    }
                    Clamp();
                }
                else if (evt == KEY_EVENT_3) /* 确认 → 下一字段 */
                {
                    if (s_Field < 2)
                    {
                        s_Field++;
                    }
                    else
                    {
                        /* 当前页最后一个字段，切页 */
                        if (s_Phase == PHASE_DATE)
                        {
                            s_Phase = PHASE_TIME;
                            s_Field = 0;
                        }
                        else
                        {
                            /* 时、分、秒也处理完了 → 保存退出 */
                            Save_To_RTC();

                            Key_Flush();
                            OLED_Clear();
                            OLED_Update();

                            xTaskNotifyGive(clock_task_handle);/* ★ 通知 Clock */
                            break;
                        }
                    }
                }
                else if (evt == KEY_EVENT_3_LONG) /* 长按 → 取消 */
                {
                    Key_Flush();
                    OLED_Clear();
                    OLED_Update();

                    xTaskNotifyGive(clock_task_handle);/* ★ 通知 Clock */
                    break;
                }
            }

            /* ---------- 刷新显示 ---------- */
            if (s_Phase == PHASE_DATE) Show_Date_UI();
            else                       Show_Time_UI();

            Highlight_Field();
            vTaskDelay(1);
        }
    }
}
