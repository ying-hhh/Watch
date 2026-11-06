#include "Dino.h"
#include "OLED.h"
#include "Key.h"
#include <stdlib.h>
#include <math.h>
#include "App_MPU6050.h"

/* ============ 全局状态 ============ */

struct Object_Position {
    uint8_t minX, maxX, minY, maxY;
};

int Score;
uint16_t Ground_Pos;
uint8_t barrier_flag;
uint8_t Barrier_Pos;
struct Object_Position barrier;
uint8_t Cloud_Pos;
uint8_t dino_jump_flag = 0;
uint16_t jump_t;
uint8_t Jump_Pos;
struct Object_Position dino;

/* ============ 显示函数 ============ */

void Show_Score(void)
{
    OLED_ShowNum(98, 0, Score, 5, OLED_6X8);
}

void Show_Ground(void)
{
    if (Ground_Pos < 128)
    {
        for (uint8_t i = 0; i < 128; i++)
        {
            OLED_DisplayBuf[7][i] = Ground[i + Ground_Pos];
        }
    }
    else
    {
        for (uint8_t i = 0; i < 255 - Ground_Pos; i++)
        {
            OLED_DisplayBuf[7][i] = Ground[i + Ground_Pos];
        }
        for (uint8_t i = 255 - Ground_Pos; i < 128; i++)
        {
            OLED_DisplayBuf[7][i] = Ground[i - (255 - Ground_Pos)];
        }
    }
}

void Show_Barrier(void)
{
    if (Barrier_Pos >= 143)
    {
        barrier_flag = rand() % 3;
    }
    OLED_ShowImage(127 - Barrier_Pos, 44, 16, 18, Barrier[barrier_flag]);

    barrier.minX = 127 - Barrier_Pos;
    barrier.maxX = 143 - Barrier_Pos;
    barrier.minY = 44;
    barrier.maxY = 62;
}

void Show_Cloud(void)
{
    OLED_ShowImage(127 - Cloud_Pos, 9, 16, 8, Cloud);
}

void Show_Dino(void)
{
    /* 用非阻塞按键获取，避免卡任务 */
    KeyEvent_t evt;
    if (Key_Poll(&evt, 0) == pdTRUE)
    {
        if (evt == KEY_EVENT_1)
        {
            dino_jump_flag = 1;
        }
    }

    Jump_Pos = 28 * sin((float)(PI * jump_t / 200));

    if (dino_jump_flag == 0)
    {
        if (Cloud_Pos % 2 == 0) OLED_ShowImage(0, 44, 16, 18, Dino[0]);
        else                    OLED_ShowImage(0, 44, 16, 18, Dino[1]);
    }
    else
    {
        OLED_ShowImage(0, 44 - Jump_Pos, 16, 18, Dino[2]);
    }

    dino.minX = 0;
    dino.maxX = 16;
    dino.minY = 44 - Jump_Pos;
    dino.maxY = 62 - Jump_Pos;
}

/* ============ 碰撞检测 ============ */

static int isColliding(struct Object_Position *a, struct Object_Position *b)
{
    if ((a->maxX > b->minX) && (a->minX < b->maxX) &&
        (a->maxY > b->minY) && (a->minY < b->maxY))
    {
        OLED_Clear();
        OLED_ShowString(28, 24, "Game Over", OLED_8X16);
        OLED_Update();
        vTaskDelay(pdMS_TO_TICKS(1000));

        OLED_Clear();
        OLED_Printf(20, 24, OLED_8X16, "Score:%5d", Score);
        OLED_Update();
        vTaskDelay(pdMS_TO_TICKS(1000));

        OLED_Clear();
        OLED_Update();
        return 1;
    }
    return 0;
}

/* ============ 游戏主循环 ============ */

//int DinoGame_Animation(void)
//{
//    while (1)
//    {
//        int return_flag;

//        OLED_Clear();
//        Show_Score();
//        Show_Ground();
//        Show_Barrier();
//        Show_Cloud();
//        Show_Dino();
//        OLED_Update();

//        return_flag = isColliding(&dino, &barrier);
//        if (return_flag == 1) return 0;

//        vTaskDelay(1);   /* ★ 让出 CPU，否则会饿死其它 FreeRTOS 任务 */
//    }
//}

int DinoGame_Animation(void)
{
    TickType_t last_tick = xTaskGetTickCount();

    while (1)
    {
        /* 1ms 节拍：推进游戏状态 */
        if (xTaskGetTickCount() - last_tick >= pdMS_TO_TICKS(1))
        {
            last_tick = xTaskGetTickCount();
            Dino_Tick();
        }

        /* 绘制一帧 */
        OLED_Clear();
        Show_Score();
        Show_Ground();
        Show_Barrier();
        Show_Cloud();
        Show_Dino();
        OLED_Update();

        /* 碰撞检测 */
        if (isColliding(&dino, &barrier)) return 0;

        /* 让出 CPU，避免饿死其它任务 */
        vTaskDelay(1);
    }
}


/* ============ 游戏节拍（每 1ms 调用一次） ============ */

//void Dino_Tick(void)
//{
//    static uint8_t Score_Count, Ground_Count, Cloud_Count;

//    Score_Count++;
//    Ground_Count++;
//    Cloud_Count++;

//    if (Score_Count >= 100)
//    {
//        Score_Count = 0;
//        Score++;
//    }
//    if (Ground_Count >= 20)
//    {
//        Ground_Count = 0;
//        Ground_Pos++;
//        Barrier_Pos++;
//        if (Ground_Pos >= 256) Ground_Pos = 0;
//        if (Barrier_Pos >= 144) Barrier_Pos = 0;
//    }
//    if (Cloud_Count >= 50)
//    {
//        Cloud_Count = 0;
//        Cloud_Pos++;
//        if (Barrier_Pos >= 200) Cloud_Pos = 0;
//    }
//    if (dino_jump_flag == 1)
//    {
//        jump_t++;
//        if (jump_t >= 1000)
//        {
//            jump_t = 0;
//            dino_jump_flag = 0;
//        }
//    }
//}

void Dino_Tick(void)
{
    static uint8_t Score_Count, Ground_Count, Cloud_Count;

    Score_Count++;
    Ground_Count++;
    Cloud_Count++;

    /* 分数增长速度保持原样 */
    if (Score_Count >= 100)
    {
        Score_Count = 0;
        Score++;
    }

    /* 地面、障碍物：每 20ms 走 4 像素，等效 200 像素/秒 */
    if (Ground_Count >= 20)
    {
        Ground_Count = 0;
        Ground_Pos += 4;            /* ★ 原 +1 改成 +4 */
        Barrier_Pos += 4;           /* ★ 原 +1 改成 +4 */
        if (Ground_Pos >= 256) Ground_Pos = 0;
        if (Barrier_Pos >= 144) Barrier_Pos = 0;
    }

    /* 云朵：每 50ms 走 4 像素，等效 80 像素/秒 */
    if (Cloud_Count >= 50)
    {
        Cloud_Count = 0;
        Cloud_Pos += 4;             /* ★ 原 +1 改成 +4 */
        if (Cloud_Pos >= 200) Cloud_Pos = 0;
    }

    if (dino_jump_flag == 1)
    {
        jump_t++;
        if (jump_t >= 200)
        {
            jump_t = 0;
            dino_jump_flag = 0;
        }
    }
}

//void DinoGame_Pos_Init(void)
//{
//    Score = Ground_Pos = Barrier_Pos = Cloud_Pos = Jump_Pos = 0;
//}

void DinoGame_Pos_Init(void)
{
    /* 位置和分数 */
    Score        = 0;
    Ground_Pos   = 0;
    Barrier_Pos  = 0;
    Cloud_Pos    = 0;
    barrier_flag = 0;

    /* 跳跃状态复位 */
    dino_jump_flag = 0;
    jump_t         = 0;
    Jump_Pos       = 0;

    /* 恐龙碰撞盒复位 */
    dino.minX = 0;
    dino.maxX = 16;
    dino.minY = 44;
    dino.maxY = 62;
}
