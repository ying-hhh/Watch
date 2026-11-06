#include "App_Stopwatch.h"
#include "OLED.h"
#include "Key.h"

static uint8_t hour, min, sec;
static uint8_t start_timing_flag;

static void Show_StopWatch_UI(void)
{
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_Printf(32, 20, OLED_8X16, "%02d:%02d:%02d", hour, min, sec);
    OLED_ShowString(0, 48, "Start", OLED_8X16);
    OLED_ShowString(56, 48, "Stop", OLED_8X16);
    OLED_ShowString(103, 48, "Clr", OLED_8X16);
}

void StopWatch_Tick(void)
{
    if (start_timing_flag == 1)
    {
        sec++;
        if (sec >= 60)
        {
            sec = 0;
            min++;
            if (min >= 60)
            {
                min = 0;
                hour++;
                if (hour > 99) hour = 0;
            }
        }
    }
}

void StopWatch_Page(void)
{
    uint8_t stopwatch_flag = 1;
    TickType_t last_tick = xTaskGetTickCount();
    KeyEvent_t evt;

    while(1)
    {
        if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (evt == KEY_EVENT_1)
            {
                stopwatch_flag--; 
                if (stopwatch_flag <= 0) stopwatch_flag = 4;
            }
            else if (evt == KEY_EVENT_2)
            {
                stopwatch_flag++;
                if (stopwatch_flag >= 5) stopwatch_flag = 1;
            }
            else if (evt == KEY_EVENT_3)
			{
				switch (stopwatch_flag)
				{
                    case 1:  /* 返回 */
                        OLED_Clear();
                        OLED_Update();
                        xTaskNotifyGive(menu_task_handle);
                        return;

                    case 2:  /* 开始 */
                        start_timing_flag = 1;
                        break;

                    case 3:  /* 停止 */
                        start_timing_flag = 0;
                        break;

                    case 4:  /* 清零 */
                        start_timing_flag = 0;
                        hour = min = sec = 0;
                        break;
                }
            }
        }

		if (xTaskGetTickCount() - last_tick >= pdMS_TO_TICKS(1000))
        {
            last_tick = xTaskGetTickCount();
            StopWatch_Tick();
        }
		
        switch (stopwatch_flag)
        {
            case 1: Show_StopWatch_UI(); 
					OLED_ReverseArea(0, 0, 16, 16);
                    OLED_Update(); break;
            case 2: Show_StopWatch_UI();
                    OLED_ReverseArea(0, 48, 40, 16);
                    OLED_Update(); break;
            case 3: Show_StopWatch_UI();
                    OLED_ReverseArea(56, 48, 32, 16);
                    OLED_Update(); break;
            case 4: Show_StopWatch_UI();
                    OLED_ReverseArea(103, 48, 24, 16);
                    OLED_Update(); break;
        }

        vTaskDelay(1);
    }
}

