#include "App_Menu.h"
#include "OLED.h"
#include "Key.h"
// #include "App_Clock.h"
// #include "App_Setting.h"
// #include "App_Menu.h"
// #include "App_Stopwatch.h"
// #include "App_Flashlight.h"
// #include "App_MPU6050.h"
// #include "App_Emoji.h"
// #include "App_Gradienter.h"
// #include "App_Pedometer.h"

#define MENU_ITEM_COUNT     7       /* Menu_Graph 一共有 7 个图标，索引 0..6 */
#define MENU_ICON_W         32
#define MENU_ICON_H         32
#define MENU_ICON_Y         16

static uint8_t pre_selection;               //上次选择的选项
static uint8_t target_selection;            //目标选项（指向Menu_Graph数组中的图标的索引）
static uint8_t x_pre = 48;                  //上次选择的x坐标
static uint8_t Speed = 4;                   //速度
static uint8_t move_flag;                   //是否开始移动,1表示开始移动，0表示停止移动
static uint8_t menu_flag = 1;


/**
  * 函    数：把索引钳位到 0..MENU_ITEM_COUNT-1
  */
static uint8_t Menu_ClampIndex(int idx)
{
    if (idx < 0) return 0;
    if (idx > MENU_ITEM_COUNT - 1) return MENU_ITEM_COUNT - 1;
    return (uint8_t)idx;
}

/**
  * 函    数：安全画图标（自动钳位，防止越界）
  * 参    数：x, y    图标左上角坐标
  * 参    数：index   图标索引，越界会被钳位到 0 或 6
  */
static void Menu_DrawIconSafe(int16_t x, int16_t y, int index)
{
    uint8_t safe = Menu_ClampIndex(index);
    OLED_ShowImage(x, y, MENU_ICON_W, MENU_ICON_H, Menu_Graph[safe]);
}

/* ==================== 菜单动画 ==================== */

static void Menu_Animation(void)
{
    /* ---------- 第一步：把 pre_selection 钳位到 0..6 ---------- */
    pre_selection    = Menu_ClampIndex(pre_selection);
    target_selection = Menu_ClampIndex(target_selection);

    /* ---------- 第二步：清屏，画框 ---------- */
    OLED_Clear();
    OLED_ShowImage(42, 10, 44, 44, Frame);

    /* ---------- 第三步：滑动逻辑 ---------- */
    if (pre_selection < target_selection)
    {
        x_pre -= Speed;
        if (x_pre == 0)
        {
            pre_selection++;
            pre_selection = Menu_ClampIndex(pre_selection);
            move_flag = 0;
            x_pre = 48;
        }
    }

    if (pre_selection > target_selection)
    {
        x_pre += Speed;
        if (x_pre == 96)
        {
            pre_selection--;
            pre_selection = Menu_ClampIndex(pre_selection);
            move_flag = 0;
            x_pre = 48;
        }
    }

    /* ---------- 第四步：再钳位一次（滑动过程中可能越界） ---------- */
    pre_selection = Menu_ClampIndex(pre_selection);

    /* ---------- 第五步：画 5 个图标，全部走安全函数 ---------- */

    /* 左边第二个（索引 pre_selection - 2），只在 pre_selection >= 2 时才画 */
    if (pre_selection >= 2)
    {
        Menu_DrawIconSafe(x_pre - 96, MENU_ICON_Y, pre_selection - 2);
    }

    /* 左边第一个（索引 pre_selection - 1），只在 pre_selection >= 1 时才画 */
    if (pre_selection >= 1)
    {
        Menu_DrawIconSafe(x_pre - 48, MENU_ICON_Y, pre_selection - 1);
    }

    /* 中间（索引 pre_selection） */
    Menu_DrawIconSafe(x_pre, MENU_ICON_Y, pre_selection);

    /* 右边第一个（索引 pre_selection + 1），只在 pre_selection <= 5 时才画 */
    if (pre_selection <= MENU_ITEM_COUNT - 2)
    {
        Menu_DrawIconSafe(x_pre + 48, MENU_ICON_Y, pre_selection + 1);
    }

    /* 右边第二个（索引 pre_selection + 2），只在 pre_selection <= 4 时才画 */
    if (pre_selection <= MENU_ITEM_COUNT - 3)
    {
        Menu_DrawIconSafe(x_pre + 96, MENU_ICON_Y, pre_selection + 2);
    }

    /* ---------- 第六步：刷屏 ---------- */
    OLED_Update();
}

/* ==================== 选择设置 ==================== */

static void Set_Selection(uint8_t mf, uint8_t Pre_Selection, uint8_t Target_Selection)
{
    if (mf == 1)
    {
        pre_selection    = Menu_ClampIndex(Pre_Selection);
        target_selection = Menu_ClampIndex(Target_Selection);
    }
    Menu_Animation();
}

// //图标滑动动画
// static void Menu_Animation(void)
// {
//     OLED_Clear();
//     OLED_ShowImage(42, 10, 44, 44, Frame);

//     if (pre_selection < target_selection)
//     {
//         x_pre -= Speed;
//         if (x_pre == 0) { pre_selection++; move_flag = 0; x_pre = 48; }
//     }
//     if (pre_selection > target_selection)
//     {
//         x_pre += Speed;
//         if (x_pre == 96) { pre_selection--; move_flag = 0; x_pre = 48; }
//     }

//     if (pre_selection >= 1)
//         OLED_ShowImage(x_pre - 48, 16, 32, 32, Menu_Graph[pre_selection - 1]);
//     if (pre_selection >= 2)
//         OLED_ShowImage(x_pre - 96, 16, 32, 32, Menu_Graph[pre_selection - 2]);

//     OLED_ShowImage(x_pre, 16, 32, 32, Menu_Graph[pre_selection]);
//     OLED_ShowImage(x_pre + 48, 16, 32, 32, Menu_Graph[pre_selection + 1]);
//     OLED_ShowImage(x_pre + 96, 16, 32, 32, Menu_Graph[pre_selection + 2]);

//     OLED_Update();
// }

// static void Set_Selection(uint8_t mf, uint8_t Pre_Selection, uint8_t Target_Selection)
// {
//     if (mf == 1)
//     {
//         pre_selection = Pre_Selection;
//         target_selection = Target_Selection;
//     }
//     Menu_Animation();
// }


// static void Show_End_UI(void)
// {
//     OLED_Clear();
//     OLED_ShowImage(0, 0, 16, 16, Return);
//     OLED_ShowString(0, 16, "MPU6050 Data", OLED_8X16);
//     OLED_ShowString(0, 32, "Gradienter", OLED_8X16);
//     OLED_ShowString(0, 48, "Pedometer", OLED_8X16);
// }

// void End_Page(void)
// {
//     int option = 1;
//     KeyEvent_t evt;

//     for (;;)
//     {
//         if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
//         {
//             if (evt == KEY_EVENT_1)      /* 减 */
//             {
//                 option--;
//                 if (option < 1) option = 4;
//             }
//             else if (evt == KEY_EVENT_2) /* 加 */
//             {
//                 option++;
//                 if (option > 4) option = 1;
//             }
//             else if (evt == KEY_EVENT_3) /* 确认 → 下一字段 */
//             {
//                 // if(option==1)
//                 // {
//                 //     OLED_Clear();
//                 //     OLED_Update();
//                 //     xTaskNotifyGive(menu_task_handle);
//                 //     return;
//                 // }
//                 // else if(option==2){MPU6050_Page();}
//                 switch (option)
//                 {
//                 case 1:
//                     OLED_Clear();
//                     OLED_Update();
//                     xTaskNotifyGive(menu_task_handle);
//                     return;
//                 case 2:
//                     MPU6050_Page();                              
//                     break;
//                 case 3:
//                     Gradienter_Page();                           
//                     break;
//                 case 4:
//                     Pedometer_Page();                            
//                     break;
//                 }
//             }
//         }
//         /* 显示 */
//         switch (option)
//         {
//             case 1:
//                 Show_End_UI();
//                 OLED_ReverseArea(0, 0, 16, 16);
//                 OLED_Update();
//                 break;
//             case 2:
//                 Show_End_UI();
//                 OLED_ReverseArea(0, 16, 96, 16);
//                 OLED_Update();
//                 break;
//             case 3:
//                 Show_End_UI();
//                 OLED_ReverseArea(0, 32, 80, 16);
//                 OLED_Update();
//                 break;
//             case 4:
//                 Show_End_UI();
//                 OLED_ReverseArea(0, 48, 72, 16);
//                 OLED_Update();
//                 break;
//         }
//     }          
// }

void Menu_Page(void)
{
    move_flag = 1;
    uint8_t DirectFlag = 2;
    KeyEvent_t evt;

    Set_Selection(move_flag, 0, 0);

    while(1)
    {
        if (Key_Poll(&evt, pdMS_TO_TICKS(20)) == pdTRUE)
        {
            if (evt == KEY_EVENT_1)
            {
                DirectFlag = 1; move_flag = 1;
                menu_flag--; if (menu_flag <= 0) menu_flag = 7;
            }
            else if (evt == KEY_EVENT_2)
            {
                DirectFlag = 2; move_flag = 1;
                menu_flag++; if (menu_flag >= 8) menu_flag = 1;
            }
            else if (evt == KEY_EVENT_3)
            {
                OLED_Clear();
                OLED_Update();

                switch (menu_flag)
                {
                    case 1: 
                    g_current_active_task = clock_task_handle;
                    xTaskNotifyGive(clock_task_handle);
                    break;
                    case 2: 
                    g_current_active_task = stopwatch_task_handle;
                    xTaskNotifyGive(stopwatch_task_handle);
                    //StopWatch_Page();         
                    break;
                    case 3: 
                    g_current_active_task = flashlight_task_handle;
                    xTaskNotifyGive(flashlight_task_handle); 
                    //LED_Page();       
                    break;
                    case 4: 
                    g_current_active_task = mpu6050_task_handle;
                    xTaskNotifyGive(mpu6050_task_handle);
                    //MPU6050_Page();           
                    break;
                    case 5: 
                    g_current_active_task = game_task_handle;
                    xTaskNotifyGive(game_task_handle);
                    //Game_Page();				
                    break;
                    case 6: 
                    g_current_active_task = emoji_task_handle;
                    xTaskNotifyGive(emoji_task_handle);             
                    //Emoji_Page();
                    break;
                    case 7: 
                    g_current_active_task = gradienter_task_handle;
                    xTaskNotifyGive(gradienter_task_handle);        
                    //Gradienter_Page();
                    //Pedometer_Page();
                    //End_Page();
                    break;
                }
                
				return;
            }
        }

        if (menu_flag == 1)
        {
            if (DirectFlag == 1)      Set_Selection(move_flag, 1, 0);
            else if (DirectFlag == 2) Set_Selection(move_flag, 0, 0);
        }
        else
        {
            if (DirectFlag == 1)      Set_Selection(move_flag, menu_flag, menu_flag - 1);
            else if (DirectFlag == 2) Set_Selection(move_flag, menu_flag - 2, menu_flag - 1);
        }

        vTaskDelay(1);
    }
}
