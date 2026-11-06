#ifndef __MYRTC_H
#define __MYRTC_H

#include "main.h"
#include "rtc.h"

/* 全局时间数组：年、月、日、时、分、秒 */
extern int MyRTC_Time[6];

void MyRTC_Init(void);
void MyRTC_SetTime(void);
void MyRTC_ReadTime(void);

#endif
