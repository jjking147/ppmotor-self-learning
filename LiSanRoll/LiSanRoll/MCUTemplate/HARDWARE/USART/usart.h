#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32f4xx_conf.h"
#include "sys.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途

#define USART_REC_LEN  			20  	//定义最大接收字节数 20

#define MOTOR_USART         USART2
#define MOTOR_USART_IRQn	  USART2_IRQn

#define MOTOR_USART_RX	    GPIO_Pin_3
#define MOTOR_USART_TX	    GPIO_Pin_2
#define MOTOR_USART_GPIO    GPIOA

#define MOTOR_USART_CLK_EN	      RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)
#define MOTOR_USART_GPIO_CLK_EN	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE)
 
//串口对应引脚复用映射
#define MOTOR_USART_RX_AF	 GPIO_PinAFConfig(GPIOA,GPIO_PinSource3, GPIO_AF_USART2)
#define MOTOR_USART_TX_AF  GPIO_PinAFConfig(GPIOA,GPIO_PinSource2, GPIO_AF_USART2)
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void Master_USART_Init(u32 bound);
void Slave_USART_Init(u32 bound);
void USART1_Tx_InitDMA(void);
void USART1_SendByte(u8 byte);
void USART1_SendBytes(u8 *bytes, u32 len);
void USART1_SendString(u8 *str);
void USART1_SendWithDMA(u8* data, u32 len);

#endif
