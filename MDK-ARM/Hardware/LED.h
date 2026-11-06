#ifndef __LED_H
#define __LED_H

#include "main.h"

/* 引脚宏定义，方便后续修改 */
#define LED1_PIN        GPIO_PIN_15
#define LED1_PORT       GPIOB

void LED_Init(void);
void LED_ON(void);
void LED_OFF(void);
void LED_TOGGLE(void);

#endif
