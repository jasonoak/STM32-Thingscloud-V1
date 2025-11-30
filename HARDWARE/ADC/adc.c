#include "delay.h"
#include "sys.h"
#include "adc.h"
#include "stm32f10x_adc.h"





void ADC_Init_All(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 开启 GPIOA 和 ADC1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);

    // ADC 时钟分频：72MHz / 6 = 12MHz（不得超过14MHz）
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // PA0~PA4 设为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 复位 ADC
    ADC_DeInit(ADC1);

    // ADC 配置：单通道、单次转换、软件触发
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;      // 单通道模式 = 1
    ADC_Init(ADC1, &ADC_InitStructure);

    // 启动 ADC
    ADC_Cmd(ADC1, ENABLE);

    // 校准 ADC
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/**
  * @brief  获取 ADC 单次转换值
  * @param  ch = ADC_Channel_0~ADC_Channel_4
  */
u16 ADC_GetValue(u8 ch)
{
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5);

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));

    return ADC_GetConversionValue(ADC1);
}
 
u16 temp_Get_Adc_Average(u8 ch,u8 times)
{
    u32 temp_val=0;
    u8 t;
    for(t=0;t<times;t++)
    {
        temp_val+=ADC_GetValue(ch);
        delay_ms(5);
    }
    return temp_val/times;
}


