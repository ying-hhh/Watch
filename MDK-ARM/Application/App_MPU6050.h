#ifndef __APP_MPU6050_H
#define __APP_MPU6050_H

#include "App_config.h"

#define PI  3.1415927

void MPU6050_Page(void);

/* 供水平仪共用 */
void MPU6050_ReadRaw(int16_t *ax, int16_t *ay, int16_t *az);
void MPU6050_Calc(void);
float MPU6050_GetRoll(void);
float MPU6050_GetPitch(void);

#endif
