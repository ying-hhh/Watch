#include "MPU6050.h"
#include "MyI2C.h"
#include "MPU6050_Reg.h"

/**
  * 函    数：MPU6050写寄存器
  * 参    数：RegAddress 寄存器地址，参考 MPU6050 手册
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    MyI2C_Start();                          // I2C 起始
    MyI2C_SendByte(MPU6050_ADDRESS);        // 从机地址 + 写
    MyI2C_ReceiveAck();                     // 接收应答
    MyI2C_SendByte(RegAddress);             // 寄存器地址
    MyI2C_ReceiveAck();                     // 接收应答
    MyI2C_SendByte(Data);                   // 写入数据
    MyI2C_ReceiveAck();                     // 接收应答
    MyI2C_Stop();                           // I2C 终止
}

/**
  * 函    数：MPU6050读寄存器
  * 参    数：RegAddress 寄存器地址，参考 MPU6050 手册
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;

    MyI2C_Start();                          // I2C 起始
    MyI2C_SendByte(MPU6050_ADDRESS);        // 从机地址 + 写
    MyI2C_ReceiveAck();                     // 接收应答
    MyI2C_SendByte(RegAddress);             // 寄存器地址
    MyI2C_ReceiveAck();                     // 接收应答

    MyI2C_Start();                          // I2C 重复起始
    MyI2C_SendByte(MPU6050_ADDRESS | 0x01); // 从机地址 + 读
    MyI2C_ReceiveAck();                     // 接收应答
    Data = MyI2C_ReceiveByte();             // 接收数据
    MyI2C_SendAck(1);                       // 非应答，终止从机输出
    MyI2C_Stop();                           // I2C 终止

    return Data;
}

/**
  * 函    数：MPU6050初始化
  * 参    数：无
  * 返 回 值：无
  */
void MPU6050_Init(void)
{
    MyI2C_Init();   // 初始化底层软件 I2C（含 DWT 延时）

    /* MPU6050 寄存器初始化，对照手册配置 */
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);     // 取消睡眠，时钟源选 X 轴陀螺仪
    MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);     // 所有轴不待机
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x04);     // 采样率分频
    MPU6050_WriteReg(MPU6050_CONFIG, 0x06);         // DLPF 配置
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);    // 陀螺仪满量程 ±2000°/s
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);   // 加速度计满量程 ±16g
}

/**
  * 函    数：MPU6050获取ID号
  * 参    数：无
  * 返 回 值：MPU6050 的 ID 号（正常应为 0x68）
  */
uint8_t MPU6050_GetID(void)
{
    return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

/**
  * 函    数：MPU6050获取数据
  * 参    数：AccX AccY AccZ 加速度计 X、Y、Z 轴数据（-32768~32767）
  * 参    数：GyroX GyroY GyroZ 陀螺仪 X、Y、Z 轴数据（-32768~32767）
  * 返 回 值：无
  */
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t DataH, DataL;

    /* 加速度计 X */
    DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
    *AccX = (int16_t)((DataH << 8) | DataL);

    /* 加速度计 Y */
    DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
    *AccY = (int16_t)((DataH << 8) | DataL);

    /* 加速度计 Z */
    DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
    *AccZ = (int16_t)((DataH << 8) | DataL);

    /* 陀螺仪 X */
    DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
    *GyroX = (int16_t)((DataH << 8) | DataL);

    /* 陀螺仪 Y */
    DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
    *GyroY = (int16_t)((DataH << 8) | DataL);

    /* 陀螺仪 Z */
    DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
    DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
    *GyroZ = (int16_t)((DataH << 8) | DataL);
}
