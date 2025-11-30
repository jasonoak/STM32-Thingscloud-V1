#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "rs485.h"
#include "pump.h"
#include "adc.h"
#include "oled.h"
#include "stdio.h"
#include <stdbool.h>
#include "sgp30.h"
#include "dht11.h"
#include <string.h>


float wendu;
float shidu;
float PH_NUM;

float light_per;
float wendu_hs=35.0;
float shidu_hs= 80.0;
float PH_NUM_hs= 9.0;
float light_per_hs = 100;
float water_per_hs = 80 ;
float wendu_ls =10.0;
float shidu_ls = 10.0;
float PH_NUM_ls = 5.0;
float light_per_ls = 20;
unsigned int CO2Data_ls = 0;
unsigned int CO2Data_hs = 800;
unsigned int MQ2_X_per_ls = 0;
unsigned int MQ2_X_per_hs = 90;


int pump_speed;
u8 temp,humi;
	 
u32 CO2Data,TVOCData;//定义CO2浓度变量与TVOC浓度变量
u32 sgp30_dat;

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

void Oled_show(void);
void PWM_owm_Init(void);
void pump_control_speed(void);
void PWM_Start(void);
void PWM_StopAndPullLow(void);



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
float MQ2_X = 0.0;
float MQ2_X_per = 0.0;

float lIGHT_X = 0.0;
float lIGHT_X_per = 0.0;
 
void MQ_Value_Conversion()      
{
	  
	  MQ2_X=((float)ADC_GetValue(0) * 5.0) / 4095;//数字量转换为电压值显示，12为ADC，所以除以4096;0~3.3V 线性转换为0~5V，所以除以0.66
		if(MQ2_X >= 5.0){MQ2_X_per = 100.0;}
	  if(MQ2_X <= 0.0){MQ2_X_per = 0.0;}
	  else{MQ2_X_per = 100.0-(((5.0-MQ2_X)*100.0)/(5.0));}
	
	  
	  lIGHT_X=((float)ADC_GetValue(4) * 3.3) / 4095;//数字量转换为电压值显示，12为ADC，所以除以4096;0~3.3V 线性转换为0~5V，所以除以0.66
		if(lIGHT_X >= 3.3){lIGHT_X_per = 100.0;}
	  if(lIGHT_X <= 0.0){lIGHT_X_per = 0.0;}
	  else{lIGHT_X_per = (((3.3-lIGHT_X)*100.0)/(3.3));}

}
unsigned int PH_state = 0;
unsigned int water_state = 0;
unsigned int led_state = 0;
unsigned int fans_state = 0;

uint16_t count_20ms = 0;
uint16_t count_1s = 0;
uint16_t count_3s = 0;
uint16_t count_6s = 0;
uint16_t count_10s = 0;

int values[20];
int value_count = 0;
uint8_t rx_buffer[128];
uint16_t rx_index = 0;
uint8_t frame_ready = 0;   // 1 表示收到完整一帧
void parse_frame(void);




void TIM2_IRQHandler(void)
{   
     if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)//判断中断标志是否发生
     {
			 	 // 1s 计数器
        count_20ms++;
        if (count_20ms >= 200)  // 假设定时器中断周期为 0.5ms，2000 次中断即为 1s
        {
						KEY_Scan(1); 
            count_20ms = 0;
        }
				
				// 1s 计数器
        count_1s++;
        if (count_1s >= 2000)  // 假设定时器中断周期为 0.5ms，2000 次中断即为 1s
        { 
				MQ_Value_Conversion()  ;
				DHT11_Read_Data(&temp,&humi); 
					//串口解析
				if (frame_ready)
				{
						frame_ready = 0;
						parse_frame();
				}					
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
						//PH_NUM，wendu，shidu，temp，humi，MQ2_X_per，lIGHT_X_per，CO2Data
             printf("Tem=%d,Hum==%0.2f,PH=%0.2f,co2=%d,light=%0.2f,smoke=%0.2f,\
										 Tem_h=%0.2f,Hum_h=%0.2f,PH_h=%0.2f,co2_h=%d,light_h=%0.2f,smoke_h=%d\
										 Tem_l=%0.2f,Hum_l=%0.2f,PH_l=%0.2f,co2_l=%d,light_l=%0.2f,smoke_l=%d\n",temp,shidu,PH_NUM,CO2Data,lIGHT_X_per,MQ2_X_per,\
																																													wendu_hs,shidu_hs,PH_NUM_hs,CO2Data_hs,light_per_hs,MQ2_X_per_hs,\
																																												  wendu_ls,shidu_ls,PH_NUM_ls,CO2Data_ls,light_per_ls,MQ2_X_per_ls);	
            count_10s = 0;
        }
        // 清除中断标志
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
     }
}
//PH_NUM,wendu,shidu,temp,humi,MQ2_X_per,lIGHT_X_per,CO2Data



// void check_all()
// {

//	 if((temp < wendu_ls)||(temp > wendu_hs)){ BEEP = 1;}
//	 else if((shidu < shidu_ls)||(shidu > shidu_hs)){BEEP = 1;}
//	 else if((PH_NUM < PH_NUM_ls)||(PH_NUM > PH_NUM_hs)){BEEP = 1;}
//	 else if((light_per < light_per_ls)||(light_per > light_per_hs)){BEEP = 1;}
//	 else if((water_per < water_per_ls)||(water_per > water_per_hs)){BEEP = 1;}
//	 else {BEEP = 0;}
//			
// }


 void PWM_StopAndPullLow(void);
 
 void check_move()
 {
	if(PH_NUM >= 6){PH_state = 1;}else{PH_state = 0;}
	if(shidu <= shidu_ls){water_state = 1;}else{water_state = 0;}
	if(lIGHT_X_per <= light_per_ls){led_state = 1;}else{led_state = 0;}
	if((CO2Data >= CO2Data_hs)||(temp>= wendu_hs)||(MQ2_X_per>= MQ2_X_per_hs)){fans_state = 1;}else{fans_state = 0;}
 
	if(water_state == 1){TIM_Cmd(TIM4, ENABLE);pump_speed =350;TIM_SetCompare4(TIM4, pump_speed);PWM_Start();}else{PWM_StopAndPullLow();pump_speed =0;TIM_Cmd(TIM4, DISABLE);}
	if(led_state == 1){LED0 = 1;}else{LED0 = 0;}
	if(fans_state == 1){FANS = 1;}else{FANS = 0;}
}


void PWM_StopAndPullLow(void)//关水泵
	{
    
    GPIO_InitTypeDef GPIO_InitStructure;
   	TIM_Cmd(TIM4, DISABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOB, GPIO_Pin_9);
		DOWM_PUMP = 0;
}

void PWM_Start(void) //开水泵
	{

GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    TIM_Cmd(TIM4, ENABLE);
}




void SGP30_ALL_INIT()//co2初始化
{
	SGP30_Init();   //初始化SGP30
	delay_ms(100);
	SGP30_Write(0x20,0x08);
	sgp30_dat = SGP30_Read();//读取SGP30的值
	CO2Data = (sgp30_dat & 0xffff0000) >> 16;
	TVOCData = sgp30_dat & 0x0000ffff;	
	//SGP30模块开机需要一定时间初始化，在初始化阶段读取的CO2浓度为400ppm，TVOC为0ppd且恒定不变，因此上电后每隔一段时间读取一次
	//SGP30模块的值，如果CO2浓度为400ppm，TVOC为0ppd，发送“正在检测中...”，直到SGP30模块初始化完成。
	//初始化完成后刚开始读出数据会波动比较大，属于正常现象，一段时间后会逐渐趋于稳定。
	//气体类传感器比较容易受环境影响，测量数据出现波动是正常的，可自行添加滤波函数。
	while(CO2Data == 400 && TVOCData == 0)
	{
		SGP30_Write(0x20,0x08);
		sgp30_dat = SGP30_Read();//读取SGP30的值
		CO2Data = (sgp30_dat & 0xffff0000) >> 16;//取出CO2浓度值
		TVOCData = sgp30_dat & 0x0000ffff;			 //取出TVOC值
		//printf("正在检测中...\r\n");
		delay_ms(500);
	}
}

void USART1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能 GPIOA 和 USART1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // 配置 PA9 为复用推挽输出（USART1 TX）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 PA10 为浮空输入（USART1 RX）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART 结构体配置
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // 使能 USART1 接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 使能 USART1
    USART_Cmd(USART1, ENABLE);

    // 配置 NVIC
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
uint8_t received_data;
char ch;
void USART1_IRQHandler(void)//串口1初始化
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        uint8_t ch = USART_ReceiveData(USART1);

        if (ch == '\n')   // 一帧结束
        {
            rx_buffer[rx_index] = '\0'; // 字符串结束符
            rx_index = 0;
            frame_ready = 1;            // 标记可解析
        }
        else
        {
            rx_buffer[rx_index++] = ch;
            if (rx_index >= sizeof(rx_buffer)) rx_index = 0; // 防止溢出
        }
    }
}


void parse_frame(void)//解析
{
    char *ptr = strtok((char*)rx_buffer, ",");
    value_count = 0;

    while (ptr != NULL)
    {
        values[value_count++] = atoi(ptr);  // 字符串变成 int
        ptr = strtok(NULL, ",");
    }
		//value放进阈值
		CO2Data_hs =values[0];
		CO2Data_ls = values[1];
		shidu_hs = values[2];
		shidu_ls = values[3];
		PH_NUM_hs = values[4];
		PH_NUM_ls = values[5];
		light_per_hs = values[6];
		light_per_ls = values[7];
		wendu_hs = values[8];
		wendu_ls = values[9];
		MQ2_X_per_hs = values[10];
		MQ2_X_per_ls = values[11];


		
		
		
		
		
		
		
		
		
		
}
//void USART1_IRQHandler(void)
//{
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
//	{
//			USART_ClearITPendingBit(USART1, USART_IT_RXNE);   // 清除接收中断标志
//			ch = USART_ReceiveData(USART1);  // 自动清除RXNE中断标志
//			rx_buffer[rx_index++] = ch;
//		
//		
////		if (ch == 0x0A)
////		{
////			TEMP = (int)rx_buffer[1]-48;
////			HUM = (int)rx_buffer[3]-48;
////			GAS = (int)rx_buffer[5]-48;
////			FIRE =(int) rx_buffer[7]-48;
////			door = (int)rx_buffer[9]-48;

////			rx_index = 1;
////		}

//	}
//}


 int main(void)
	 {	 
			u8 key;
			u8 rs485buf[8]; 
			u8 rs485buf_rec[13]; 

			delay_init();	    	
			NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//		uart_init(115200);	 	
		  USART1_Init();
			LED_Init();		  		
			KEY_Init();				

			while(DHT11_Init());//配置DHT11
			Pump_Pin_Init();
			UP_PUMP = 0;
			DOWM_PUMP = 0;
		
			OLED_Init();
			
			OLED_ColorTurn(0);
			OLED_DisplayTurn(0);


			OLED_ShowString(0,0,"Loading...",16);
			OLED_Refresh();
			delay_ms(1000);
			
			ADC_Init_All();
			time2_config();
			RS485_Init(4800);	
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

			LED0 = 1;
			BEEP = 0;
			SGP30_ALL_INIT();//co2初始
			OLED_Clear();	OLED_Refresh();
			
			PWM_owm_Init();
			pump_speed = 400;
		
	while(1)
	{
		
		RS485_Send_Data(rs485buf,8);							   
		delay_ms(1000);
		RS485_Receive_Data(rs485buf_rec,&key);
	  shidu= (concatenateAndConvertToDecimal(rs485buf_rec[3],rs485buf_rec[4]))/10.0;
		wendu = (concatenateAndConvertToDecimal(rs485buf_rec[5],rs485buf_rec[6]))/10.0;
		PH_NUM = (rs485buf_rec[10])/10.0;
		Oled_show();
		
		SGP30_Write(0x20,0x08);//写入SGP30的值
		sgp30_dat = SGP30_Read();//读取SGP30的值
		CO2Data = (sgp30_dat & 0xffff0000) >> 16;//取出CO2浓度值

		//check_all();
		check_move();
		 
	} 
}
void Oled_show()
{
			if(oled_menu == 0)
		{
			  OLED_ColorTurn(0);
			  sprintf((char *)first_show, "PH:%0.1f  ",PH_NUM);
			  sprintf((char *)second_show,"Tem:%0.2f C  ",wendu);
		    sprintf((char *)third_show, "Hum:%0.2f %%  ",shidu);
				
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);

				OLED_Refresh();
		
		}
		//PH_NUM，wendu，shidu，temp，humi，MQ2_X_per，lIGHT_X_per，CO2Data
		if(oled_menu == 1)
		{
			OLED_ColorTurn(0);
			  sprintf((char *)first_show, "Tem:%d C",temp);
		    sprintf((char *)second_show, "Hum:%0d %%",humi);
				
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				

				OLED_Refresh();
		
		}
		
		if(oled_menu == 2)
		{
			OLED_ColorTurn(0);
			  sprintf((char *)first_show, "Smoke:%0.2f %%  ",MQ2_X_per);
		    sprintf((char *)second_show, "Light:%0.2f %%  ",lIGHT_X_per);
				sprintf((char *)third_show, "Co2:%0d ",CO2Data);
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);

				OLED_Refresh();
		
		}
		
		if(oled_menu == 3)
		{
			  OLED_ColorTurn(1);
				sprintf((char *)first_show,  "PH_REL:%0.1f ",PH_NUM);
			  sprintf((char *)second_show, "PH_SET_H:%0.1f ",PH_NUM_hs);
			  sprintf((char *)third_show,  "PH_SET_L:%0.1f ",PH_NUM_ls);

	
				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
			  OLED_ShowString(0,43,third_show,16);

				OLED_Refresh();
		
		}
		
		if(oled_menu == 4)
		{
				OLED_ColorTurn(1);
				sprintf((char *)first_show,  "T-Hum_REL:%0.1f",shidu);
				sprintf((char *)second_show, "SET_H:%0.1f ",shidu_hs);
		   	sprintf((char *)third_show, "SET_L:%0.1f ",shidu_ls);


				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);
				OLED_Refresh();
		
		}
		
		
		if(oled_menu == 5)
		{
				OLED_ColorTurn(1);
				sprintf((char *)first_show,  "Tem_REL:%d ",temp);
				sprintf((char *)second_show, "Tem_SET_H:%0.1f ",wendu_hs);
		   	sprintf((char *)third_show, "Tem_SET_L:%0.1f ",wendu_ls);


				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);
				OLED_Refresh();
		
		}
		
			if(oled_menu == 6)
		{
				OLED_ColorTurn(1);
				sprintf((char *)first_show,  "Lit_REL:%0.1f ",lIGHT_X_per);
				sprintf((char *)second_show, "Lit_SET_H:%0.1f ",light_per_hs);
		   	sprintf((char *)third_show, "Lit_SET_L:%0.1f ",light_per_ls);


				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);
				OLED_Refresh();
		
		}
		
			if(oled_menu == 7)
		{
				OLED_ColorTurn(1);
				sprintf((char *)first_show,  "CO2Data_REL:%d",CO2Data);
				sprintf((char *)second_show, "SET_H:%d ",CO2Data_hs);
		   	sprintf((char *)third_show, "SET_L:%d ",CO2Data_ls);


				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);
				OLED_Refresh();
		
		}
		
		if(oled_menu == 8)
		{
				OLED_ColorTurn(1);
				sprintf((char *)first_show,  "MQ2_X_REL:%0.1f",MQ2_X_per);
				sprintf((char *)second_show, "SET_H:%d ",MQ2_X_per_hs);
		   	sprintf((char *)third_show, "SET_L:%d ",MQ2_X_per_ls);


				OLED_ShowString(0,0,first_show,16);
				OLED_ShowString(0,23,second_show,16);
				OLED_ShowString(0,43,third_show,16);
				OLED_Refresh();
		
		}

}



void PWM_owm_Init(void) //pwm初始化 tim4
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;


    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);


    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    TIM_TimeBaseStructure.TIM_Period = 999;         
    TIM_TimeBaseStructure.TIM_Prescaler = 71;      
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);


    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;


    TIM_OCInitStructure.TIM_Pulse = 0;
//    TIM_OC3Init(TIM4, &TIM_OCInitStructure);
    TIM_OC4Init(TIM4, &TIM_OCInitStructure);


//    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM4, ENABLE);

    TIM_Cmd(TIM4, DISABLE);
		
}
//void pump_control_speed()
//{
//	if(close_b == 0)
//	{
//	pump_speed = 700-((shidu*900)/100);
//	}
//		if(close_b == 1)
//	{
//	pump_speed = 0;
//		
//	}
//}
