#include "AD.h"

/* hadc1 由 CubeMX 在 adc.c 中定义，并通过 main.h 或其他头文件声明 */
extern ADC_HandleTypeDef hadc1;

/**
  * 函    数：AD初始化
  * 参    数：无
  * 返 回 值：无
  * 说    明：GPIO 与 ADC 参数已由 CubeMX 的 MX_ADC1_Init() 完成，
  *           此函数仅负责启动校准与使能 ADC。
  */
void AD_Init(void)
{
    /* ADC 校准（HAL 库推荐在初始化后调用一次） */
    if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /* 使能 ADC（对应标准库的 ADC_Cmd(ADC1, ENABLE)） */
    HAL_ADC_Start(&hadc1);
}

/**
  * 函    数：获取AD转换的值
  * 参    数：无
  * 返 回 值：AD转换的值，范围：0~4095
  */
uint16_t AD_GetValue(void)
{
    /* 软件触发一次转换 */
    HAL_ADC_Start(&hadc1);

    /* 等待转换完成，超时时间 10ms */
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        /* 读取数据寄存器，读取后 EOC 标志会自动清除 */
        return (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    return 0;   // 转换超时或失败
}
