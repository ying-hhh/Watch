#include "App_Game.h"
#include "OLED.h"
#include "Key.h"
#include "LED.h"
#include "Dino.h"

static void Show_Game_UI(void)
{
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_ShowString(0, 16, "Dino Game", OLED_8X16);
}

void Game_Page(void)
{
    uint8_t game_flag = 1;
    KeyEvent_t evt;

    while(1)
    {
        if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (evt == KEY_EVENT_1)
            {
                game_flag--; 
                if (game_flag <= 0) game_flag = 2;
            }
            else if (evt == KEY_EVENT_2)
            {
                game_flag++; 
                if (game_flag >= 3) game_flag = 1;
            }
            else if (evt == KEY_EVENT_3)
            {
				switch (game_flag)
				{
                    case 1:  /* ∑µªÿ */
                        OLED_Clear();
                        OLED_Update();
                        xTaskNotifyGive(menu_task_handle);
                        return;

                    case 2:  /* –°ø÷¡˙”Œœ∑ */
						DinoGame_Pos_Init();
						DinoGame_Animation();
                        break;
				}
            }
        }

        switch (game_flag)
        {
            case 1:
                Show_Game_UI();
                OLED_ReverseArea(0, 0, 16, 16);
                OLED_Update();
                break;
            case 2:
                Show_Game_UI();
                OLED_ReverseArea(0, 16, 72, 16);
                OLED_Update();
                break;
        }

        vTaskDelay(1);
    }
}
