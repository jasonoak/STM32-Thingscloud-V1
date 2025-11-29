#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"




#define KEY0  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10)//读取按键0
#define KEY1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)//读取按键1
#define KEY2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)//读取按键1
#define KEY3  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)//读取按键1
#define KEY4  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)//读取按键1
 



void KEY_Init(void);//IO初始化
void KEY_Scan(u8);  	//按键扫描函数					    
#endif
