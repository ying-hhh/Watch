#include "App_Gradienter.h"
#include "OLED.h"
#include "Key.h"
#include "App_MPU6050.h"
#include <math.h>

/* 外圆半径，与 OLED_DrawCircle 一致 */
#define OUTER_RADIUS   30
/* 小球半径 */
#define BALL_RADIUS     4
/* 小球圆心能到达的最大偏移量 */
#define MAX_OFFSET     (OUTER_RADIUS - BALL_RADIUS)   /* = 26 */

void Gradienter_Page(void)
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

        MPU6050_Calc();
        float dx = MPU6050_GetRoll();        /* X 方向偏移 */
        float dy = MPU6050_GetPitch();       /* Y 方向偏移 */
        /* 计算偏移量长度 */
        float d = sqrtf(dx * dx + dy * dy);
        /* 若超出边界，等比例缩回到边界 */
        if (d > MAX_OFFSET)
        {
            dx = dx * MAX_OFFSET / d;
            dy = dy * MAX_OFFSET / d;
        }
        /* 小球圆心坐标 */
        int16_t bx = 64 + (int16_t)dx;
        int16_t by = 32 + (int16_t)dy;

        OLED_Clear();
        OLED_DrawCircle(64, 32, OUTER_RADIUS, 0);   /* 外圆 */
        OLED_DrawCircle(bx, by, BALL_RADIUS, 1);    /* 小球 */
        OLED_Update();

        vTaskDelay(1);
    }
}
