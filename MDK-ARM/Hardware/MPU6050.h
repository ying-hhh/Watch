#ifndef __MPU6050_H
#define __MPU6050_H

#include "main.h"

/* MPU6050 从机地址（7位地址 0x68 左移 1 位） */
#define MPU6050_ADDRESS     0xD0

/* 函数声明 */
void    MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);
void    MPU6050_Init(void);
uint8_t MPU6050_GetID(void);
void    MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                        int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);

#endif
