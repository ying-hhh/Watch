#ifndef __MYI2C_H
#define __MYI2C_H

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

/* 引脚定义（与 CubeMX 中配置一致） */
#define MYI2C_SCL_PORT      GPIOB
#define MYI2C_SCL_PIN       GPIO_PIN_10

#define MYI2C_SDA_PORT      GPIOB
#define MYI2C_SDA_PIN       GPIO_PIN_11

/* I2C 时序延时（微秒级）
 * FreeRTOS 中没有原生微秒延时，这里用空循环近似。
 * 72MHz 主频下，约 1us 对应 18~30 次循环（取决于编译优化）。
 * 若时序不严格，可适当增大延时。
 */
#define MYI2C_DELAY_US      10

/* 函数声明 */
void    MyI2C_Init(void);
void    MyI2C_W_SCL(uint8_t BitValue);
void    MyI2C_W_SDA(uint8_t BitValue);
uint8_t MyI2C_R_SDA(void);
void    MyI2C_Start(void);
void    MyI2C_Stop(void);
void    MyI2C_SendByte(uint8_t Byte);
uint8_t MyI2C_ReceiveByte(void);
void    MyI2C_SendAck(uint8_t AckBit);
uint8_t MyI2C_ReceiveAck(void);

#endif
