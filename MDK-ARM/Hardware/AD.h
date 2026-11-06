#ifndef __AD_H
#define __AD_H

#include "main.h"   // 引入 HAL 库定义（含 hadc1 声明）

/* 函数声明 */
void     AD_Init(void);       // ADC 初始化（校准 + 使能）
uint16_t AD_GetValue(void);   // 获取 ADC 转换值（0~4095）

#endif
