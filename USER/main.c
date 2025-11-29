#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "rs485.h"
#include "pump.h"
#include "level.h"
#include "adc.h"
#include "oled.h"
#include "stdio.h"
#include <stdbool.h>

float wendu;
float shidu;
float PH_NUM;
float Water_level_int;
float Light_level_int;
float light_per;
float water_per;

float wendu_hs=35.0;
float shidu_hs= 80.0;
float PH_NUM_hs= 9.0;
float light_per_hs = 100;
float water_per_hs = 80 ;

float wendu_ls =10.0;
float shidu_ls = 10.0;
float PH_NUM_ls = 5.0;
float light_per_ls = 0;
float water_per_ls  = 0;
int pump_speed;

bool pump_on_off = 0;
unsigned char first_show[30];
unsigned char second_show[30];
unsigned char third_show[30];
uint16_t num_s = 0;
uint16_t oled_menu = 0;

uint32_t concatenateAndConvertToDecimal(uint8_t highByte, uint8_t lowByte)
{
	unsigned int result;
    result = (highByte << 8) | lowByte;
    return result;
}

 
unsigned int MQ_X_NUM=0;
unsigned int light_low = 4190;
unsigned int light_high = 3800;

unsigned int water_low = 4000;
unsigned int water_high = 10;
void Oled_show(void);
void PWM_owm_Init(void);
void pump_control_speed(void);
void PWM_Start(void);
void PWM_StopAndPullLow(void);
int close_b=0;
	
int ci_shidu = 0;
int ci_shidu1 = 0;
void check_light()
{
	if(Light_level_int >= light_low){light_per = 0.0;}
	if(Light_level_int <= light_high){light_per = 100.0;}
	else{
	
	light_per = ((light_low-Light_level_int)*100)/(light_low-light_high);
	
	}
	
	if(Water_level_int >= water_low){water_per = 0.0;}
	if(Water_level_int <= water_high){water_per = 100.0;}
	else{
	
	water_per = ((water_low-Water_level_int)*100)/(water_low-water_high);
	
	}
	
	
	
	
	if(oled_menu == 6)
	{
	if(pump_on_off == 1){close_b  =0;ci_shidu = 1;TIM_Cmd(TIM3, ENABLE);PWM_Start();}
	else if(pump_on_off == 0){close_b  =1;ci_shidu = 0;TIM_Cmd(TIM3, DISABLE);pump_speed = 0;PWM_StopAndPullLow();}
   }

	if(oled_menu != 6)
	{
	if((shidu < shidu_ls)){close_b = 0;TIM_Cmd(TIM3, ENABLE);PWM_Start();}
	if((shidu >= shidu_ls)){close_b  =1;TIM_Cmd(TIM3, DISABLE);pump_speed = 0;PWM_StopAndPullLow();}
	
  }

}

 void time2_config()
 {
	 NVIC_InitTypeDef nvic;
     TIM_TimeBaseInitTypeDef tim;//结构体
          NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);//优先级分组
     
     
     nvic.NVIC_IRQChannel=TIM2_IRQn;
     nvic.NVIC_IRQChannelCmd=ENABLE;
     nvic.NVIC_IRQChannelPreemptionPriority=0;
     nvic.NVIC_IRQChannelSubPriority=0;
     NVIC_Init(&nvic);
	 
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//开启时钟
     TIM_DeInit(TIM2);//
     tim.TIM_ClockDivision=TIM_CKD_DIV1;//采样分频
     tim.TIM_CounterMode=TIM_CounterMode_Up;//向上计数
     tim.TIM_Period=1000;//自动重装载寄存器的值
     tim.TIM_Prescaler=35;//时钟预分频
     TIM_TimeBaseInit(TIM2,&tim);//初始化结构体
     TIM_ClearFlag(TIM2,TIM_FLAG_Update);//清除溢出中断标志
     TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
     TIM_Cmd(TIM2,ENABLE);//开启时钟
     
 }
 

uint16_t count_20ms = 0;
uint16_t count_1s = 0;
uint16_t count_3s = 0;
uint16_t count_6s = 0;
uint16_t count_10s = 0;
  void TIM2_IRQHandler(void)
 {   
     if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)//判断中断标志是否发生
     {
			 	 // 1s 计数器
        count_20ms++;
        if (count_20ms >= 200)  // 假设定时器中断周期为 0.5ms，2000 次中断即为 1s
        {
						KEY_Scan(1);  					
					TIM_SetCompare4(TIM3, pump_speed);
            count_20ms = 0;
        }
				
				     // 1s 计数器
        count_1s++;
        if (count_1s >= 2000)  // 假设定时器中断周期为 0.5ms，2000 次中断即为 1s
        { 
					  		Water_level_int  = temp_Get_Adc(1);
                Light_level_int = temp_Get_Adc(4);

            count_1s = 0;
        }
        // 3s 计数器
        count_3s++;
        if (count_3s >= 6000)  // 6000 次中断即为 3s
        {
					
            count_3s = 0;
        }
        // 6s 计数器
        count_6s++;
        if (count_6s >= 12000)  // 12000 次中断即为 6s
        {

            count_6s = 0;
        }
        // 10s 计数器
        count_10s++;
        if (count_10s >= 20000)  // 20000 次中断即为 10s
        {

             printf("PH=%0.2f,Tem=%0.2f,Hum=%0.2f,Light=%0.2f,Level=%0.2f,Pump=%d\n",PH_NUM,wendu,shidu,light_per,water_per,pump_on_off);	
            count_10s = 0;
        }
        // 清除中断标志
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
     }
}

 void check_all()
 {
	 if((wendu < wendu_ls)||(wendu > wendu_hs)){ BEEP = 1;}
	 else if((shidu < shidu_ls)||(shidu > shidu_hs)){BEEP = 1;}
	 else if((PH_NUM < PH_NUM_ls)||(PH_NUM > PH_NUM_hs)){BEEP = 1;}
	 else if((light_per < light_per_ls)||(light_per > light_per_hs)){BEEP = 1;}
	 else if((water_per < water_per_ls)||(water_per > water_per_hs)){BEEP = 1;}
	 else {BEEP = 0;}
			
 }
void PWM_StopAndPullLow(void) {
    
    GPIO_InitTypeDef GPIO_InitStructure;
   	TIM_Cmd(TIM3, DISABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_1);
}

void PWM_Start(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    TIM_Cmd(TIM3, ENABLE);
}
 int main(void)
	 {	 
			u8 key;

			u8 rs485buf[8]; 
			u8 rs485buf_rec[13]; 

			delay_init();	    	
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
			uart_init(115200);	 	
			LED_Init();		  		
			KEY_Init();				

			Pump_Pin_Init();
			Water_level_Init();
			temp_Adc_Init();

			Water_level_int  = temp_Get_Adc(1);
			Light_level_int = temp_Get_Adc(4);

			RS485_Init(4800);	  

			OLED_Init();
			RS485_Init(4800);	
			OLED_ColorTurn(0);
			OLED_DisplayTurn(0);

			OLED_ShowString(0,0,"Loading...3s",16);
			OLED_Refresh();
			delay_ms(1000);
			OLED_ShowString(0,0,"Loading...2s",16);
			OLED_Refresh();
			delay_ms(1000);
			OLED_ShowString(0,0,"Loading...1s",16);
			OLED_Refresh();
			delay_ms(1000);
			time2_config();

			BEEP = 1; delay_ms(50);BEEP = 0;
			FANS = 1;LED0 = 1;


			rs485buf[0]	=	0x01;	
			rs485buf[1]	=	0x03;		  
			rs485buf[2]	=	0x00;		  
			rs485buf[3]	=	0x00;		  
			rs485buf[4]	=	0x00;		  
			rs485buf[5]	=	0x04;		  
			rs485buf[6]	=	0x44;		  
			rs485buf[7]	=	0x09;		
			UP_PUMP = 0;
			DOWM_PUMP = 0;
			LED0 = 1;
			BEEP = 0;
			OLED_Clear();	OLED_Refresh();
			PWM_owm_Init();
			pump_speed = 300;



	while(1)
	{
	 
		check_light();
		pump_control_speed();
		
		RS485_Send_Data(rs485buf,8);							   
		delay_ms(1000);
		RS485_Receive_Data(rs485buf_rec,&key);
	  shidu= (concatenateAndConvertToDecimal(rs485buf_rec[3],rs485buf_rec[4]))/10.0;
		wendu = (concatenateAndConvertToDecimal(rs485buf_rec[5],rs485buf_rec[6]))/10.0;
		
		//PH_NUM = (concatenateAndConvertToDecimal(rs485buf_rec[9],rs485buf_rec[10]))/100.0;
		PH_NUM = (rs485buf_rec[10])/10.0;

		
		Oled_show();


		//check_all();
		 
	} 
}
void Oled_show()
{
			if(oled_menu == 0)
		{
			OLED_ColorTurn(0);
			  sprintf((char *)first_show, "PH:%0.1f",PH_NUM);
				sprintf((char *)second_show,"Tem:%0.2f C",wendu);
				sprintf((char *)third_show, "Hum:%0.2f %%",shidu);
				
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);

				OLED_Refresh();
		
		}
		if(oled_menu == 1)
		{
			  OLED_ColorTurn(1);
				sprintf((char *)first_show,  "PH_REL:%0.1f",PH_NUM);
			  sprintf((char *)second_show, "PH_SET_H:%0.1f",PH_NUM_hs);
			  sprintf((char *)third_show,  "PH_SET_L:%0.1f",PH_NUM_ls);

	
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
			  OLED_ShowString(0,43,third_show,16);

				OLED_Refresh();
		
		}
		if(oled_menu == 2)
		{
				OLED_ColorTurn(1);
				sprintf((char *)first_show,  "Tem_REL:%0.1f",wendu);
				sprintf((char *)second_show, "Tem_SET_H:%0.1f",wendu_hs);
		   	sprintf((char *)third_show, "Tem_SET_L:%0.1f",wendu_ls);


				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);
				OLED_Refresh();
		
		}
		if(oled_menu == 3)
		{
				OLED_ColorTurn(1);
				sprintf((char *)first_show,  "Hum_REL:%0.1f",shidu);
				sprintf((char *)second_show, "Hum_SET_H:%0.1f",shidu_hs);
				sprintf((char *)third_show, "Hum_SET_L:%0.1f",shidu_ls);
			
			
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);
			
				OLED_Refresh();
		
		}
		if(oled_menu == 4)
		{
			OLED_ColorTurn(1);
				sprintf((char *)first_show,  "LIG_REL:%0.1f%%",light_per);
			  sprintf((char *)second_show, "LIG_SET_H:%0.1f%%",light_per_hs);
			  sprintf((char *)third_show,  "LIG_SET_L:%0.1f%%",light_per_ls);
			
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
			OLED_ShowString(0,43,third_show,16);
				OLED_Refresh();
		
		}
		
		
		if(oled_menu == 5)
		{
			OLED_ColorTurn(1);
				sprintf((char *)first_show,  "Wat_REL:%0.1f%%",water_per);
			  sprintf((char *)second_show, "Wat_SET_H:%0.1f%%",water_per_hs);
			 sprintf((char *)third_show, "Wat_SET_L:%0.1f%%",water_per_ls);

			
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
			OLED_ShowString(0,43,third_show,16);
				OLED_Refresh();
		
		}			
		
		if(oled_menu == 6)
		{
			OLED_ColorTurn(1);
				sprintf((char *)first_show,  "MAN_MODE");
			  sprintf((char *)second_show,  "Hum_REL:%0.1f",shidu);

			
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_Refresh();
		
		}
}
void PWM_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    TIM_TimeBaseStructure.TIM_Period = 999;         // PWMƵ�� = 1MHz / (999 + 1) = 1kHz
    TIM_TimeBaseStructure.TIM_Prescaler = 71;       // 72MHz / (71 + 1) = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);


    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;


    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);


    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);


    TIM_Cmd(TIM3, ENABLE);
}


void PWM_owm_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    TIM_TimeBaseStructure.TIM_Period = 999;         // PWMƵ�� = 1MHz / (999 + 1) = 1kHz
    TIM_TimeBaseStructure.TIM_Prescaler = 71;       // 72MHz / (71 + 1) = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);


    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;


    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OC3Init(TIM3, &TIM_OCInitStructure);
    TIM_OC4Init(TIM3, &TIM_OCInitStructure);


    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);


    TIM_Cmd(TIM3, DISABLE);
}
void pump_control_speed()
{
	if(close_b == 0)
	{
	pump_speed = 700-((shidu*900)/100);
	}
		if(close_b == 1)
	{
	pump_speed = 0;
		
	}
	//pump_speed = (shidu*900)/100;
	
}
