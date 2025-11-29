#ifndef __ADC_H
#define __ADC_H 

#include "sys.h"
u16 temp_Get_Adc_Average(u8 ch,u8 times);
u16 temp_Get_Adc(u8 ch)   ;
void  temp_Adc_Init(void);
float temp_Get_R(u16 adct);
float Get_Kelvin_Temperature(u16 t);
#endif
