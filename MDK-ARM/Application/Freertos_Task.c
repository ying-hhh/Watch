#include "Freertos_Task.h" 

void Key_Task(void *args);
#define KEY_TASK_STACKSIZE     128            				//占空间的大小(最小推荐写128 =>128*4=512字节)
#define KEY_TASK_PRIORITY      2           				    //优先级
TaskHandle_t key_task_handle;               				//句柄
#define KEY_TASK_PERIOD        pdMS_TO_TICKS(10)		    //任务周期

void Clock_Task(void *args);
#define CLOCK_TASK_STACKSIZE        512
#define CLOCK_TASK_PRIORITY         1
TaskHandle_t clock_task_handle;
#define CLOCK_TASK_PERIOD           100

void SetTime_Task(void *args);
#define SETTING_TASK_STACKSIZE      512
#define SETTING_TASK_PRIORITY       1
TaskHandle_t setting_task_handle;
#define SETTING_TASK_PERIOD         100

void Menu_Task(void *args);
#define MENU_TASK_STACKSIZE         512
#define MENU_TASK_PRIORITY          1
TaskHandle_t menu_task_handle;
//#define MENU_TASK_PERIOD            100

void StopWatch_Task(void *args);
#define STOPWATCH_TASK_STACKSIZE         512
#define STOPWATCH_TASK_PRIORITY          1
TaskHandle_t stopwatch_task_handle;
//#define STOPWATCH_TASK_PERIOD            100

void Flashlight_Task(void *args);
#define FLASHLIGHT_TASK_STACKSIZE         128
#define FLASHLIGHT_TASK_PRIORITY          1
TaskHandle_t flashlight_task_handle;

void MPU6050_Task(void *args);
#define MPU6050_TASK_STACKSIZE         256
#define MPU6050_TASK_PRIORITY          1
TaskHandle_t mpu6050_task_handle;

void Game_Task(void *args);
#define GAME_TASK_STACKSIZE         128
#define GAME_TASK_PRIORITY          1
TaskHandle_t game_task_handle;

void Emoji_Task(void *args);
#define EMOJI_TASK_STACKSIZE         128
#define EMOJI_TASK_PRIORITY          1
TaskHandle_t emoji_task_handle;

void Gradienter_Task(void *args);
#define GRADIENTER_TASK_STACKSIZE         128
#define GRADIENTER_TASK_PRIORITY          1
TaskHandle_t gradienter_task_handle;

#define VOICE_TASK_STACKSIZE         128
#define VOICE_TASK_PRIORITY          3
TaskHandle_t voice_task_handle;

TaskHandle_t g_current_active_task = NULL;

void FreeRTOS_Start(void)
{
	//初始化
	OLED_Init();
	MyRTC_Init();
	Key_Init();
	MPU6050_Init();
	
    //创建任务
	xTaskCreate(Key_Task,"Key_Task",KEY_TASK_STACKSIZE,NULL,KEY_TASK_PRIORITY,&key_task_handle);

    xTaskCreate(Clock_Task,"Clock_Task",CLOCK_TASK_STACKSIZE,NULL,CLOCK_TASK_PRIORITY,&clock_task_handle);

    xTaskCreate(SetTime_Task,"SetTime_Task",SETTING_TASK_STACKSIZE,NULL,SETTING_TASK_PRIORITY,&setting_task_handle);

    xTaskCreate(Menu_Task,"Menu_Task",MENU_TASK_STACKSIZE,NULL,MENU_TASK_PRIORITY,&menu_task_handle);
	
	xTaskCreate(StopWatch_Task,"StopWatch_Task",STOPWATCH_TASK_STACKSIZE,NULL,STOPWATCH_TASK_PRIORITY,&stopwatch_task_handle);
	
    xTaskCreate(Flashlight_Task,"Flashlight_Task",FLASHLIGHT_TASK_STACKSIZE,NULL,FLASHLIGHT_TASK_PRIORITY,&flashlight_task_handle);

    xTaskCreate(MPU6050_Task,"MPU6050_Task",MPU6050_TASK_STACKSIZE,NULL,MPU6050_TASK_PRIORITY,&mpu6050_task_handle);

	xTaskCreate(Game_Task,"Game_Task",GAME_TASK_STACKSIZE,NULL,GAME_TASK_PRIORITY,&game_task_handle);

    xTaskCreate(Emoji_Task,"Emoji_Task",EMOJI_TASK_STACKSIZE,NULL,EMOJI_TASK_PRIORITY,&emoji_task_handle);
    
    xTaskCreate(Gradienter_Task,"Gradienter_Task",GRADIENTER_TASK_STACKSIZE,NULL,GRADIENTER_TASK_PRIORITY,&gradienter_task_handle);

    xTaskCreate(Voice_Task,"Voice_Task",VOICE_TASK_STACKSIZE,NULL,VOICE_TASK_PRIORITY,&voice_task_handle);

    //启动调度器
    vTaskStartScheduler();
}

void Clock_Task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
	
    while(1)
    {	
        //时钟界面显示+光标显示
        int Option=Clock_Option();

        if(Option == 1)
        {
            Key_Flush();
            OLED_Clear();
            OLED_Update();
            g_current_active_task = menu_task_handle;
            xTaskNotifyGive(menu_task_handle);              /* 通知 Menu */
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        /* ★ 等 Menu 通知回来 */
        }
        else if(Option == 2)
        {
            Key_Flush();
            OLED_Clear();
            OLED_Update();
            g_current_active_task = setting_task_handle;
            xTaskNotifyGive(setting_task_handle);           /* 通知 SetTime */
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);        /* ★ 等 SetTime 通知回来 */
        }

        vTaskDelayUntil(&xLastWakeTime,CLOCK_TASK_PERIOD);
    }
}

void SetTime_Task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
        SetTime_Page();
		
        vTaskDelayUntil(&xLastWakeTime, SETTING_TASK_PERIOD);
    }
}

void Menu_Task(void *args)
{

    while(1)
    {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        OLED_Clear();
        OLED_Update();
        Menu_Page();
    }
}

void StopWatch_Task(void *args)
{
    while(1)
    {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        OLED_Clear();
        OLED_Update();
        StopWatch_Page();
    }
}

void Flashlight_Task(void *args)
{
    while(1)
    {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        OLED_Clear();
        OLED_Update();
        LED_Page();
    }
}

void MPU6050_Task(void *args)
{
    while(1)
    {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        MPU6050_Page();
    }
}

void Game_Task(void *args)
{
	while(1)
    {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Game_Page();
    }
}

void Emoji_Task(void *args)
{
    while(1)
    {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
        Emoji_Page();
    }
}

void Gradienter_Task(void *args)
{
    while(1)
    {
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        Gradienter_Page();
    }
}

void Key_Task(void *args)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while(1)
    {
        Key_Tick();
		
        vTaskDelayUntil(&xLastWakeTime, KEY_TASK_PERIOD);
    }
}


