#include "sys.h"
#include "usart.h"	
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
////////////////////////////////////////////////////////////////////////////////// 	 
#define BUFFER_SIZE 256  // 增大缓冲区以应对可能较长的字符串
//char rx_buffer[BUFFER_SIZE];
//uint8_t rx_index = 0;
extern bool pump_on_off ;
int pump_int;
//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/
 /* 
int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);

	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {}	
   
    return ch;
}
int GetKey (void)  { 

    while (!(USART1->SR & USART_FLAG_RXNE));

    return ((int)(USART1->DR & 0x1FF));
}
*/
 
#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	  
  
void uart_init(u32 bound){
  //GPIO端口设置
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
	//USART1_TX   GPIOA.9
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
  //USART1_RX	  GPIOA.10初始化
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

  //Usart1 NVIC 配置
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART1, &USART_InitStructure); //初始化串口1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_Cmd(USART1, ENABLE);                    //使能串口1 

}
// 解析字符串函数
void parse_string(const char* str) {
    
    // 尝试解析 Pump=xx 格式的字符串
    if (sscanf(str, "Pump=%d", &pump_int) == 1) {
       pump_on_off = (bool)pump_int;
        // 打印解析结果
        //printf("Pump: %s\n", pump_value ? "true" : "false");
    } else {
       // printf("非目标格式字符串: %s\n", str);
    }
}


//// 串口1中断服务函数
//void USART1_IRQHandler(void) {
//    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
//        char ch = USART_ReceiveData(USART1);
//    if (ch == '0') {
//        pump_on_off = 0;
//    } else if (ch >= '1' && ch <= '9') {
//        pump_on_off = 1;
//    }
//        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//    }
//}


//uint8_t received_data;
//char ch;
//void USART1_IRQHandler(void)
//{
//    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
//    {
//		    USART_ClearITPendingBit(USART1, USART_IT_RXNE);   // 清除接收中断标志
//        ch = USART_ReceiveData(USART1);  // 自动清除RXNE中断标志
//		    rx_buffer[rx_index++] = ch;
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

//    }
//}

#endif	

