#include "MyI2C.h"

/* ==================== 微秒级延时 ====================
 * FreeRTOS 中不能用 HAL_Delay 做微秒延时（最小 1ms）。
 * 这里用 DWT 循环计数实现高精度微秒延时（Cortex-M3/M4 通用）。
 */
static void MyI2C_Delay_us(uint32_t us)
{
    /* 使用 DWT_CYCCNT 计数器实现精确微秒延时（72MHz 主频） */
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < ticks);
}

/* ==================== 引脚配置层 ==================== */

void MyI2C_W_SCL(uint8_t BitValue)
{
    HAL_GPIO_WritePin(MYI2C_SCL_PORT, MYI2C_SCL_PIN,
                      BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);
    MyI2C_Delay_us(MYI2C_DELAY_US);
}

void MyI2C_W_SDA(uint8_t BitValue)
{
    HAL_GPIO_WritePin(MYI2C_SDA_PORT, MYI2C_SDA_PIN,
                      BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);
    MyI2C_Delay_us(MYI2C_DELAY_US);
}

uint8_t MyI2C_R_SDA(void)
{
    uint8_t BitValue;
    BitValue = HAL_GPIO_ReadPin(MYI2C_SDA_PORT, MYI2C_SDA_PIN);
    MyI2C_Delay_us(MYI2C_DELAY_US);
    return BitValue;
}

void MyI2C_Init(void)
{
    /* 使能 DWT 计数器（仅需调用一次）-->DWT初始化 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* GPIO 已由 CubeMX 的 MX_GPIO_Init() 完成配置，
     * 此处无需额外操作。
     * 若需手动初始化（非 CubeMX 工程）：
     *   __HAL_RCC_GPIOB_CLK_ENABLE();
     *   GPIO_InitTypeDef GPIO_InitStruct = {0};
     *   GPIO_InitStruct.Pin   = GPIO_PIN_10 | GPIO_PIN_11;
     *   GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
     *   GPIO_InitStruct.Pull  = GPIO_PULLUP;
     *   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
     *   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
     *   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 | GPIO_PIN_11, GPIO_PIN_SET);
     */
}

/* ==================== 协议层 ==================== */

void MyI2C_Start(void)
{
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(0);
    MyI2C_W_SCL(0);
}

void MyI2C_Stop(void)
{
    MyI2C_W_SDA(0);
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(1);
}

void MyI2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SDA(!!(Byte & (0x80 >> i)));
        MyI2C_W_SCL(1);
        MyI2C_W_SCL(0);
    }
}

uint8_t MyI2C_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    MyI2C_W_SDA(1);
    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SCL(1);
        if (MyI2C_R_SDA()) { Byte |= (0x80 >> i); }
        MyI2C_W_SCL(0);
    }
    return Byte;
}

void MyI2C_SendAck(uint8_t AckBit)
{
    MyI2C_W_SDA(AckBit);
    MyI2C_W_SCL(1);
    MyI2C_W_SCL(0);
}

uint8_t MyI2C_ReceiveAck(void)
{
    uint8_t AckBit;
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    AckBit = MyI2C_R_SDA();
    MyI2C_W_SCL(0);
    return AckBit;
}
