#include "exti.h"
#include "delay.h"
#include "gpio.h"
#include "bll_motor.h"
#include "bll_tocase.h"

void NVIC_Config(u8 num, u8 edge)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // 使能SYSCFG时钟

	EXTITrigger_TypeDef triger_edge = EXTI_Trigger_Rising;

	if (edge == 0)
	{
		triger_edge = EXTI_Trigger_Rising;
	}

	else
	{
		triger_edge = EXTI_Trigger_Falling;
	}

	switch (num)
	{
	case 1:
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource2); // PC2 连接到中断线2
		/* 配置EXTI_Line2 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line2;			//
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断事件
		EXTI_InitStructure.EXTI_Trigger = triger_edge;		// 上升沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			// 使能LINE2
		EXTI_Init(&EXTI_InitStructure);						// 配置
		// 配置EXTI_Line2优先级
		NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			 // 外部中断0
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		 // 子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能外部中断通道
		NVIC_Init(&NVIC_InitStructure);								 // 配置
		break;

	case 2:
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource3); // PC3 连接到中断线3
		/* 配置EXTI_Line3 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line3;			//
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断事件
		EXTI_InitStructure.EXTI_Trigger = triger_edge;		// 上升沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			// 使能LINE3
		EXTI_Init(&EXTI_InitStructure);						// 配置
		// 配置EXTI_Line3优先级
		NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			 // 外部中断0
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		 // 子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能外部中断通道
		NVIC_Init(&NVIC_InitStructure);								 // 配置
		break;

	case 3:
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource4); // PC4 连接到中断线4
		/* 配置EXTI_Line4 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line4;			//
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断事件
		EXTI_InitStructure.EXTI_Trigger = triger_edge;		// 上升沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			// 使能LINE4
		EXTI_Init(&EXTI_InitStructure);						// 配置
		// 配置EXTI_Line4优先级
		NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			 // 外部中断0
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		 // 子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能外部中断通道
		NVIC_Init(&NVIC_InitStructure);								 // 配置 优化一下这个程序
		break;

	case 4:
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource5); // PC5 连接到中断线5
		/* 配置EXTI_Line5 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line5;			//
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断事件
		EXTI_InitStructure.EXTI_Trigger = triger_edge;		// 上升沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			// 使能LINE5
		EXTI_Init(&EXTI_InitStructure);						// 配置
		// 配置EXTI_Line5优先级
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			 // 外部中断0
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		 // 子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能外部中断通道
		NVIC_Init(&NVIC_InitStructure);								 // 配置
		break;

	case 5:
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource7); // PC7 连接到中断线7
		/* 配置EXTI_Line7 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line7;			//
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断事件
		EXTI_InitStructure.EXTI_Trigger = triger_edge;		// 上升沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			// 使能LINE7
		EXTI_Init(&EXTI_InitStructure);						// 配置
		// 配置EXTI_Line7优先级
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			 // 外部中断0
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		 // 子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能外部中断通道
		NVIC_Init(&NVIC_InitStructure);								 // 配置
		break;

	case 6:
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8); // PC8 连接到中断线8
		/* 配置EXTI_Line8 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line8;			//
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断事件
		EXTI_InitStructure.EXTI_Trigger = triger_edge;		// 上升沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			// 使能LINE8
		EXTI_Init(&EXTI_InitStructure);						// 配置
		// 配置EXTI_Line8优先级
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			 // 外部中断0
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		 // 子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能外部中断通道
		NVIC_Init(&NVIC_InitStructure);								 // 配置
		break;

	case 7:
		SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8); // PC9 连接到中断线9
		/* 配置EXTI_Line9 */
		EXTI_InitStructure.EXTI_Line = EXTI_Line9;			//
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; // 中断事件
		EXTI_InitStructure.EXTI_Trigger = triger_edge;		// 上升沿触发
		EXTI_InitStructure.EXTI_LineCmd = ENABLE;			// 使能LINE9
		EXTI_Init(&EXTI_InitStructure);						// 配置
		// 配置EXTI_Line9优先级
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			 // 外部中断0
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级0
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		 // 子优先级2
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能外部中断通道
		NVIC_Init(&NVIC_InitStructure);								 // 配置
		break;

	default:
		break;
	}
}

static const u32 exti_lines[] =
	{
		EXTI_Line2,
		EXTI_Line3,
		EXTI_Line4,
		EXTI_Line5,
		EXTI_Line7,
		EXTI_Line8,
		EXTI_Line9};

void My_EXTI_Cmd(u8 line, u8 cmd)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = exti_lines[line];
	EXTI_InitStructure.EXTI_LineCmd = cmd;
	EXTI_Init(&EXTI_InitStructure);
}

void My_EXTI_DisableAll(void)
{
	u8 len = sizeof(exti_lines) / sizeof(u32);
	for (u8 i = 0; i < len; i++)
	{
		My_EXTI_Cmd(i, DISABLE);
	}
}

void EXTI0_IRQHandler(void)
{
	if (EXTI_GetFlagStatus(EXTI_Line0) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line0); // 清除LINE0上的中断标志位
	}
}

void EXTI1_IRQHandler(void)
{
	if (EXTI_GetFlagStatus(EXTI_Line1) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line1); // 清除LINE1上的中断标志位
	}
}
void EXTI2_IRQHandler(void)
{
	if (EXTI_GetFlagStatus(EXTI_Line2) != RESET)
	{
		SWTICH1_INT_HANDLER();
		EXTI_ClearITPendingBit(EXTI_Line2); // 清除LINE2上的中断标志位
	}
}

void EXTI3_IRQHandler(void)
{
	if (EXTI_GetFlagStatus(EXTI_Line3) != RESET)
	{
		SWTICH2_INT_HANDLER();
		EXTI_ClearITPendingBit(EXTI_Line3); // 清除LINE3上的中断标志位
	}
}

void EXTI4_IRQHandler(void)
{
	if (EXTI_GetFlagStatus(EXTI_Line4) != RESET)
	{
		SWTICH3_INT_HANDLER();
		EXTI_ClearITPendingBit(EXTI_Line4); // 清除LINE4上的中断标志位
	}
}

void EXTI9_5_IRQHandler(void)
{
	// 检查并处理EXTI_Line5的中断
	if (EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		SWTICH4_INT_HANDLER();
		EXTI_ClearITPendingBit(EXTI_Line5);
	}
	// 检查并处理EXTI_Line6的中断
	if (EXTI_GetITStatus(EXTI_Line6) != RESET)
	{
		GPIO_INT_HANDLER(7);
		EXTI_ClearITPendingBit(EXTI_Line6);
	}
	// 检查并处理EXTI_Line7的中断
	if (EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		SWTICH5_INT_HANDLER();
		EXTI_ClearITPendingBit(EXTI_Line7);
	}
	// 检查并处理EXTI_Line8的中断
	if (EXTI_GetITStatus(EXTI_Line8) != RESET)
	{
		EStop_INT_HANDLER(1);
		EXTI_ClearITPendingBit(EXTI_Line8);
	}
	// 检查并处理EXTI_Line9的中断
	if (EXTI_GetITStatus(EXTI_Line9) != RESET)
	{
		EStop_INT_HANDLER(0);
		EXTI_ClearITPendingBit(EXTI_Line9);
	}
}
