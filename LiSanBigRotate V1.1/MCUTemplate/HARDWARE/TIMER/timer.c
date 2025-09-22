#include "timer.h"
#include "iwdg.h"
#include "modbus_slave.h"
#include "modbus_master.h"

void Modbus_Slave_Timer_Init(int ms)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	MODBUS_SLAVE_TIM_CLK_EN; 

	TIM_DeInit(MODBUS_SLAVE_TIM);  

	TIM_TimeBaseStructure.TIM_Period = (ms<<1) -1;
	TIM_TimeBaseStructure.TIM_Prescaler = (84000/2-1);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(MODBUS_SLAVE_TIM, &TIM_TimeBaseStructure);  //初始化定时器
	
	TIM_ClearFlag(MODBUS_SLAVE_TIM, TIM_FLAG_Update); 
	TIM_ITConfig(MODBUS_SLAVE_TIM, TIM_IT_Update,ENABLE); 

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;      
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  
	NVIC_Init(&NVIC_InitStructure); 
	
	TIM_Cmd(MODBUS_SLAVE_TIM, DISABLE);      
}

void TIM6_DAC_IRQHandler()
{
	if(TIM_GetITStatus(MODBUS_SLAVE_TIM, TIM_IT_Update) != RESET)
	{
		ModBus_SetTimeout();
		TIM_Cmd(MODBUS_SLAVE_TIM,DISABLE); 
		TIM_ClearITPendingBit(MODBUS_SLAVE_TIM, TIM_FLAG_Update);
	}
}

void Modbus_Master_Timer_Init(int ms)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	MODBUS_MASTER_TIM_CLK_EN;  ///使能时钟
	
	TIM_TimeBaseInitStructure.TIM_Period = (ms<<1) -1;; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = (84000/2-1);;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	TIM_TimeBaseInit(MODBUS_MASTER_TIM, &TIM_TimeBaseInitStructure);//初始化
	
	TIM_ClearFlag(MODBUS_MASTER_TIM, TIM_FLAG_Update); 
	TIM_ITConfig(MODBUS_MASTER_TIM, TIM_IT_Update,ENABLE); //允许定时器更新中断
	
	NVIC_InitStructure.NVIC_IRQChannel = MODBUS_MASTER_TIM_IRQn; //定时器中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(MODBUS_MASTER_TIM, DISABLE); //使能定时器
}

//定时器7中断服务函数
void TIM7_IRQHandler(void)
{
	if(TIM_GetITStatus(MODBUS_MASTER_TIM, TIM_IT_Update)==SET) //溢出中断
	{
		Master_Set_Timeout();
		TIM_Cmd(MODBUS_MASTER_TIM,DISABLE);
		TIM_ClearITPendingBit(MODBUS_MASTER_TIM, TIM_IT_Update);  //清除中断标志位
	}
}

void Delay_Timer_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	DELAY_TIM_CLK_EN;  ///使能时钟
	
	TIM_TimeBaseInitStructure.TIM_Period = 0xFFFFFFFF; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = (84000/2-1);  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(DELAY_TIM, &TIM_TimeBaseInitStructure);//初始化
	
	TIM_ITConfig(DELAY_TIM, TIM_IT_Update,ENABLE); //允许定时器更新中断
	TIM_Cmd(DELAY_TIM, ENABLE); //使能定时器
	
//	NVIC_InitStructure.NVIC_IRQChannel = DELAY_TIM_IRQn; //定时器中断
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; //抢占优先级1
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03; //子优先级3
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
}

void Dispatcher_Tim_Init(u16 ms)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DISPATCHER_TIM_CLK_EN();
	
	TIM_TimeBaseInitStructure.TIM_Period = ((ms*2)<<1) -1; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = (84000/2-1);  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(DISPATCHER_TIM, &TIM_TimeBaseInitStructure);//初始化
	
	NVIC_InitStructure.NVIC_IRQChannel = DISPATCHER_TIM_IRQn; //定时器中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01; //抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearITPendingBit(DISPATCHER_TIM, TIM_IT_Update);
	TIM_ITConfig(DISPATCHER_TIM, TIM_IT_Update,ENABLE); //允许定时器更新中断
	TIM_Cmd(DISPATCHER_TIM, ENABLE); //使能定时器
}

//定时器9中断服务函数
void TIM1_BRK_TIM9_IRQHandler(void)
{
	if(TIM_GetITStatus(DISPATCHER_TIM, TIM_IT_Update)==SET) //溢出中断
	{
		ModBus_Pool();
		IWDG_Feed();
		TIM_ClearITPendingBit(DISPATCHER_TIM, TIM_IT_Update);  //清除中断标志位
	}
}
