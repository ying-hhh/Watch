#include "App_MPU6050.h"
#include "OLED.h"
#include "Key.h"
#include "MPU6050.h"
#include <math.h>

static int16_t ax, ay, az, gx, gy, gz;
static float   Roll, Pitch, Yaw;
static const float a = 0.9f;
static const float Delta_t = 0.005f;

void MPU6050_ReadRaw(int16_t *p_ax, int16_t *p_ay, int16_t *p_az)
{
    MPU6050_GetData(p_ax, p_ay, p_az, &gx, &gy, &gz);
}

void MPU6050_Calc(void)
{
    MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);

    float roll_g  = Roll  + (float)gx * Delta_t;
    float pitch_g = Pitch + (float)gy * Delta_t;
    float yaw_g   = Yaw   + (float)gz * Delta_t;

    float pitch_a = atan2((-1) * ax, az) * 180 / PI;
    float roll_a  = atan2(ay, az) * 180 / PI;

    Roll  = a * roll_g  + (1 - a) * roll_a;
    Pitch = a * pitch_g + (1 - a) * pitch_a;
    Yaw   = a * yaw_g;
}

float MPU6050_GetRoll(void)  { return Roll;  }
float MPU6050_GetPitch(void) { return Pitch; }

static void Show_MPU6050_UI(void)
{
    OLED_ShowImage(0, 0, 16, 16, Return);
    OLED_Printf(0, 16, OLED_8X16, "Roll: %.2f", Roll);
    OLED_Printf(0, 32, OLED_8X16, "Pitch:%.2f", Pitch);
    OLED_Printf(0, 48, OLED_8X16, "Yaw:  %.2f", Yaw);
}

void MPU6050_Page(void)
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

        OLED_Clear();
        MPU6050_Calc();
        Show_MPU6050_UI();
        OLED_ReverseArea(0, 0, 16, 16);
        OLED_Update();

        vTaskDelay(1);
    }
}
