#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

#define MODBUS_SLAVE_TIM TIM6
#define MODBUS_SLAVE_TIM_IRQn TIM6_DAC_IRQn

#define MODBUS_MASTER_TIM TIM7
#define MODBUS_MASTER_TIM_IRQn TIM7_IRQn

// TIM5用于延时计数
#define DELAY_TIM TIM5
// #define DELAY_TIM_IRQn	           TIM8_UP_TIM13_IRQn

#define DISPATCHER_TIM TIM9
#define DISPATCHER_TIM_IRQn TIM1_BRK_TIM9_IRQn

#define MODBUS_SLAVE_TIM_CLK_EN RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE)
#define MODBUS_MASTER_TIM_CLK_EN RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE)
#define DELAY_TIM_CLK_EN RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE)
#define DISPATCHER_TIM_CLK_EN() RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE)

// tim5就是DELAY_TIM,用于延时计数
#define ReadTick() (DELAY_TIM->CNT / 2)
#define TickSpan(start) ((DELAY_TIM->CNT / 2) - start)

void Modbus_Slave_Timer_Init(int ms);
void Modbus_Master_Timer_Init(int ms);
void Delay_Timer_Init(void);
void Dispatcher_Tim_Init(u16 ms);

#endif
