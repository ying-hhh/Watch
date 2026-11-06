#include "LED.h"

/* GPIO 已由 CubeMX 在 MX_GPIO_Init() 中配置为推挽输出，
 * 此函数留作扩展或用于非 CubeMX 工程。
 */
void LED_Init(void)
{
    /* CubeMX 工程中无需操作 */
    /* 若使用非 CubeMX 工程，可在此添加：
     *   __HAL_RCC_GPIOB_CLK_ENABLE();
     *   GPIO_InitTypeDef GPIO_InitStruct = {0};
     *   GPIO_InitStruct.Pin   = GPIO_PIN_15 | GPIO_PIN_12 | GPIO_PIN_13;
     *   GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
     *   GPIO_InitStruct.Pull  = GPIO_NOPULL;
     *   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
     *   HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
     *
     *   // 初始电平
     *   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
     *   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
     *   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
     */
}

/* LED_ON：对应原代码，拉低 PB15 点亮 LED */
void LED_ON(void)
{
    HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET);
}

/* LED_OFF：对应原代码，拉高 PB15 熄灭 LED */
void LED_OFF(void)
{
    HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET);
}

void LED_TOGGLE(void)
{
	HAL_GPIO_TogglePin(LED1_PORT, LED1_PIN);
}
