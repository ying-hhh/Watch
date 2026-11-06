#include "App_Emoji.h"
#include "OLED.h"
#include "Key.h"

void Show_Emoji_UI(void)
{
    for (uint8_t i = 0; i < 3; i++)
    {
        OLED_Clear();
        OLED_ShowImage(30, 10 + i, 16, 16, Eyebrow[0]);
        OLED_ShowImage(82, 10 + i, 16, 16, Eyebrow[1]);
        OLED_DrawEllipse(40, 32, 6, 6 - i, 1);
        OLED_DrawEllipse(88, 32, 6, 6 - i, 1);
        OLED_ShowImage(54, 40, 20, 20, Mouth);
        OLED_Update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    for (uint8_t i = 0; i < 3; i++)
    {
        OLED_Clear();
        OLED_ShowImage(30, 12 - i, 16, 16, Eyebrow[0]);
        OLED_ShowImage(82, 12 - i, 16, 16, Eyebrow[1]);
        OLED_DrawEllipse(40, 32, 6, 4 + i, 1);
        OLED_DrawEllipse(88, 32, 6, 4 + i, 1);
        OLED_ShowImage(54, 40, 20, 20, Mouth);
        OLED_Update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelay(pdMS_TO_TICKS(500));
}

void Emoji_Page(void)
{
    KeyEvent_t evt;

    while(1)
    {
        if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (evt == KEY_EVENT_3)
			{
				OLED_Clear();
                OLED_Update();
                xTaskNotifyGive(menu_task_handle);
                return;
			}
        }

        Show_Emoji_UI();
    }
}
