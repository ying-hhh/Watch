#include "App_Flashlight.h"
#include "OLED.h"
#include "Key.h"
#include "LED.h"

static void Show_LED_UI(void)
{
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_ShowString(20, 20, "OFF", OLED_12X24);
    OLED_ShowString(72, 20, "ON", OLED_12X24);
}

void LED_Page(void)
{
    uint8_t led_flag = 1;
    KeyEvent_t evt;

    while(1)
    {
        if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (evt == KEY_EVENT_1)
            {
                led_flag--; 
                if (led_flag <= 0) led_flag = 3;
            }
            else if (evt == KEY_EVENT_2)
            {
                led_flag++; 
                if (led_flag >= 4) led_flag = 1;
            }
            else if (evt == KEY_EVENT_3)
			{
				switch (led_flag)
				{
                    case 1:  /* 返回 */
                        OLED_Clear();
                        OLED_Update();
                        xTaskNotifyGive(menu_task_handle);
                        return;

                    case 2:  /* 关灯 */
                        LED_OFF();
                        break;

                    case 3:  /* 开灯 */
                        LED_ON();
                        break;
                }
            }
        }

        switch (led_flag)
        {
            case 1: Show_LED_UI(); OLED_ReverseArea(0, 0, 16, 16);
                    OLED_Update(); break;
            case 2: Show_LED_UI();OLED_ReverseArea(20, 20, 36, 24);
                    OLED_Update(); break;
            case 3: Show_LED_UI();OLED_ReverseArea(72, 20, 24, 24);
                    OLED_Update(); break;
        }

        vTaskDelay(1);
    }
}

