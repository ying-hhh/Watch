#include "App_Clock.h"
#include "AD.h"

static uint16_t ADValue;
//static float    VBAT;
static int      Battery_Capacity;

static void Show_Battery(void)
{
    int sum = 0;
    for (int i = 0; i < 3000; i++)
    {
        ADValue = AD_GetValue();
        sum += ADValue;
    }
    ADValue = sum / 3000;
    //VBAT = (float)ADValue / 4095 * 3.3f;
    Battery_Capacity = (ADValue - 3276) * 100 / 819;
    if (Battery_Capacity < 0) Battery_Capacity = 0;

    OLED_ShowNum(85, 4, Battery_Capacity, 3, OLED_6X8);
    OLED_ShowChar(103, 4, '%', OLED_6X8);

    if (Battery_Capacity == 100)
    {
        OLED_ShowImage(110, 0, 16, 16, Battery);
    }
    else if (Battery_Capacity >= 10 && Battery_Capacity < 100)
    {
        OLED_ShowImage(110, 0, 16, 16, Battery);
        OLED_ClearArea((112 + Battery_Capacity / 10), 5,
                       (10 - Battery_Capacity / 10), 6);
        OLED_ClearArea(85, 4, 6, 8);
    }
    else
    {
        OLED_ShowImage(110, 0, 16, 16, Battery);
        OLED_ClearArea(112, 5, 10, 6);
        OLED_ClearArea(85, 4, 12, 8);
    }
}

void Show_Clock_UI(void)
{
    Show_Battery();
    MyRTC_ReadTime();
    OLED_Printf(0, 0, OLED_6X8, "%d-%d-%d",
                MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
    OLED_Printf(16, 16, OLED_12X24, "%02d:%02d:%02d",
                MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
    OLED_ShowString(0, 48, "Menu", OLED_8X16);
    OLED_ShowString(104, 48, "Set", OLED_8X16);
}

int Clock_Option(void)
{
    int option = 1;
    KeyEvent_t evt;

    while (1)
    {
        /* 从队列取按键，超时 20ms */
        if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (evt == KEY_EVENT_1)
            {
                option--;
                if (option < 1) option = 2;
            }
            else if (evt == KEY_EVENT_2)
            {
                option++;
                if (option > 2) option = 1;
            }
            else if (evt == KEY_EVENT_3)
            {
                OLED_Clear();
                OLED_Update();
                return option;
            }
        }

        /* 显示 */
        switch (option)
        {
            case 1:
                Show_Clock_UI();
                OLED_ReverseArea(0, 48, 32, 16);
                OLED_Update();
                break;
            case 2:
                Show_Clock_UI();
                OLED_ReverseArea(104, 48, 24, 16);
                OLED_Update();
                break;
        }
    }
}
