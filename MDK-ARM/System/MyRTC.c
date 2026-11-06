#include "MyRTC.h"
#include <time.h>

/* 全局时间数组：年、月、日、时、分、秒 */
int MyRTC_Time[] = {2026, 4, 11, 18, 12, 55};

/* 备份寄存器标志位，用于判断 RTC 是否第一次配置 */
#define RTC_BKP_FLAG        RTC_BKP_DR1
#define RTC_BKP_FLAG_VALUE  0xA5A5

/* 函数声明 */
void MyRTC_SetTime(void);

/**
  * 函    数：RTC 初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：CubeMX 生成的 MX_RTC_Init() 已经完成了 RTC 时钟源、
  *           预分频器等硬件初始化，这里主要判断是否是第一次配置，
  *           如果是第一次，就把全局数组里的时间写入 RTC。
  */
void MyRTC_Init(void)
{
    /* 判断备份寄存器标志位，检查 RTC 是否是第一次配置 */
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_FLAG) != RTC_BKP_FLAG_VALUE)
    {
        /* ---------- 第一次配置：写入初始时间 ---------- */
        RTC_TimeTypeDef sTime = {0};
        RTC_DateTypeDef sDate = {0};

        /* 写入时间 */
        sTime.Hours          = MyRTC_Time[3];
        sTime.Minutes        = MyRTC_Time[4];
        sTime.Seconds        = MyRTC_Time[5];
        HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

        /* 写入日期 */
        sDate.Year     = MyRTC_Time[0] - 2000;   /* HAL 里年份从 2000 起算 */
        sDate.Month    = MyRTC_Time[1];
        sDate.Date     = MyRTC_Time[2];
        sDate.WeekDay  = RTC_WEEKDAY_SATURDAY;   /* 2026-4-11 是周六，可按需修改 */
        HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

        /* 写入标志位，防止下次上电重复配置 */
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_FLAG, RTC_BKP_FLAG_VALUE);
    }
    else
    {
        /* ---------- 不是第一次配置：RTC 硬件已在走时，无需再做 ---------- */
        /* 只需确认一下同步（HAL 内部已处理，这里留空即可） */
    }
}

/**
  * 函    数：RTC 设置时间
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，全局数组里的时间值将刷新到 RTC 硬件
  */
void MyRTC_SetTime(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    /* 时间 */
    sTime.Hours          = MyRTC_Time[3];
    sTime.Minutes        = MyRTC_Time[4];
    sTime.Seconds        = MyRTC_Time[5];
    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

    /* 日期 */
    sDate.Year     = MyRTC_Time[0] - 2000;
    sDate.Month    = MyRTC_Time[1];
    sDate.Date     = MyRTC_Time[2];
    sDate.WeekDay  = RTC_WEEKDAY_SATURDAY;   /* 可根据实际日期修改，或另写函数计算 */
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

/**
  * 函    数：RTC 读取时间
  * 参    数：无
  * 返 回 值：无
  * 说    明：调用此函数后，RTC 硬件电路里的时间值将刷新到全局数组
  */
void MyRTC_ReadTime(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    /* 先读时间，再读日期 —— 这是 HAL 库的推荐顺序 */
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    MyRTC_Time[0] = sDate.Year + 2000;   /* HAL 里年份从 2000 起算 */
    MyRTC_Time[1] = sDate.Month;
    MyRTC_Time[2] = sDate.Date;
    MyRTC_Time[3] = sTime.Hours;
    MyRTC_Time[4] = sTime.Minutes;
    MyRTC_Time[5] = sTime.Seconds;
}
