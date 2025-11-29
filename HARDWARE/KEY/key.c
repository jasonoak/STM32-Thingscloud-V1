#include "stm32f10x.h"
#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "oled.h"
#include "led.h"



extern float wendu_hs;
extern float shidu_hs;
extern float PH_NUM_hs;
extern float light_per_hs;
extern float water_per_hs;

extern float wendu_ls;
extern float shidu_ls;
extern float PH_NUM_ls;
extern float light_per_ls;
extern float water_per_ls;

extern int pump_speed;
extern uint16_t oled_menu;
								    
//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA,PORTE时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//KEY0-KEY1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOE4,3



}
//按键处理函数
void  KEY_Scan(u8 mode)
{	 
		if(KEY0 == 0)
		{
			delay_ms(5);
			if(KEY0 == 0)
			{
				oled_menu++;
				 BEEP = 1; delay_ms(50);BEEP = 0;
				 BEEP = 1; delay_ms(50);BEEP = 0;
				 OLED_Clear();	OLED_Refresh();
				
				if(oled_menu == 6){TIM_Cmd(TIM3, DISABLE);pump_speed = 0;PWM_StopAndPullLow();}
				if(oled_menu == 7){oled_menu = 0;}
				while((KEY0 == 0));
			}
		}
		
		
		if(KEY1 == 0)
		{
			delay_ms(5);
			if(KEY1 == 0)
			{
				if(oled_menu == 1)//ph
				{
					PH_NUM_hs+= 0.5;
					if(PH_NUM_hs == 10.5)PH_NUM_hs = 0;
				}
				if(oled_menu == 2)//ph
				{
					wendu_hs+= 1.0;
					if(wendu_hs == 40.0)wendu_hs = 0;
				}
			  if(oled_menu == 3)//ph
				{
					shidu_hs+= 1.0;
					if(shidu_hs == 100.0)shidu_hs = 0;
				}
				if(oled_menu == 4)//ph
				{
					light_per_hs+= 1.0;
					if(light_per_hs == 100.0)light_per_hs = 0;
				}
				if(oled_menu == 5)//ph
				{
					water_per_hs+= 1.0;
					if(water_per_hs == 100.0)water_per_hs = 0;
				}
				BEEP = 1; delay_ms(50);BEEP = 0;
				while((KEY1 == 0));
				
			}
		}
		if(KEY2 == 0)
		{
			delay_ms(5);
			if(KEY2 == 0)
			{
				if(oled_menu == 1)//ph
				{
					PH_NUM_hs-= 0.5;
					if(PH_NUM_hs == 0.0)PH_NUM_hs = 0;
				}
				if(oled_menu == 2)//ph
				{
					wendu_hs-= 1.0;
					if(wendu_hs == 0.0)wendu_hs = 0;
				}
			  if(oled_menu == 3)//ph
				{
					shidu_hs-= 1.0;
					if(shidu_hs == 0.0)shidu_hs = 0;
				}
				if(oled_menu == 4)//ph
				{
					light_per_hs-= 1.0;
					if(light_per_hs == 0.0)light_per_hs = 0;
				}
				if(oled_menu == 5)//ph
				{
					water_per_hs-= 1.0;
					if(water_per_hs == 0.0)water_per_hs = 0;
				}
				BEEP = 1; delay_ms(50);BEEP = 0;
				while((KEY2 == 0));
				
			}
		}
		
		if(KEY3 == 0)
		{
			delay_ms(5);
			if(KEY3 == 0)
			{
				if(oled_menu == 1)//ph
				{
					PH_NUM_ls+= 0.5;
					if(PH_NUM_ls == 10.5)PH_NUM_ls = 0;
				}
				if(oled_menu == 2)//ph
				{
					wendu_ls+= 1.0;
					if(wendu_ls == 40.0)wendu_ls = 0;
				}
				if(oled_menu == 3)//ph
				{
					shidu_ls+= 1.0;
					if(shidu_ls == 100.0)shidu_ls = 0;
				}
				
				if(oled_menu == 4)//ph
				{
					light_per_ls+= 1.0;
					if(light_per_ls == 100.0)light_per_ls = 0;
				}
				if(oled_menu == 5)//ph
				{
					water_per_ls+= 1.0;
					if(water_per_ls == 100.0)water_per_ls = 0;
				}
				BEEP = 1; delay_ms(50);BEEP = 0;
				
				while((KEY3 == 0));
				
			}
		}
		
		if(KEY4 == 0)
		{
			delay_ms(5);
			if(KEY4 == 0)
			{
				if(oled_menu == 1)//ph
				{
					PH_NUM_ls-= 0.5;
					if(PH_NUM_ls == 0.0)PH_NUM_ls = 0;
				}
				if(oled_menu == 2)//ph
				{
					wendu_ls-= 1.0;
					if(wendu_ls == 0.0)wendu_ls = 0;
				}
				if(oled_menu == 3)//ph
				{
					shidu_ls-= 1.0;
					if(shidu_ls == 0.0)shidu_ls = 0;
				}
				
				if(oled_menu == 4)//ph
				{
					light_per_ls-= 1.0;
					if(light_per_ls == 0.0)light_per_ls = 0;
				}
				if(oled_menu == 5)//ph
				{
					water_per_ls-= 1.0;
					if(water_per_ls == 0.0)water_per_ls = 0;
				}
				BEEP = 1; delay_ms(50);BEEP = 0;
				
				while((KEY4 == 0));
				
			}
		}
}
