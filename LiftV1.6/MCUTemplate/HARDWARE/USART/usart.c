#include "sys.h"
#include "string.h"
#include "usart.h"
#include "modbus_master.h"
#include "modbus_slave.h"
//////////////////////////////////////////////////////////////////
// 加入以下代码,支持printf函数,而不需要选择use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
// 标准库需要的支持函数
struct __FILE
{
	int handle;
};

FILE __stdout;
// 定义_sys_exit()以避免使用半主机模式
void _sys_exit(int x)
{
	x = x;
}
// 重定义fputc函数
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; // 循环发送,直到发送完毕
	USART1->DR = (u8)ch;
	return ch;
}
#endif

// 接收状态
// bit15，	接收完成标志
// bit14，	接收到0x0d
// bit13~0，	接收到的有效字节数目

// bound:波特率
void Master_USART_Init(u32 bound)
{
	// GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	MOTOR_USART_CLK_EN;
	MOTOR_USART_GPIO_CLK_EN;

	MOTOR_USART_RX_AF;
	MOTOR_USART_TX_AF;

	// USART端口配置
	GPIO_InitStructure.GPIO_Pin = MOTOR_USART_RX | MOTOR_USART_TX; // GPIOA2与GPIOA3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;				   // 复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			   // 速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;				   // 推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;				   // 上拉
	GPIO_Init(MOTOR_USART_GPIO, &GPIO_InitStructure);			   // 初始化PA2，PA3

	// USART初始化设置
	USART_InitStructure.USART_BaudRate = bound;										// 波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(MOTOR_USART, &USART_InitStructure);									// 初始化串口

	USART_Cmd(MOTOR_USART, ENABLE); // 使能串口

	// USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);

	USART_ITConfig(MOTOR_USART, USART_IT_RXNE, ENABLE); // 开启相关中断

	// MOTOR_USART NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = MOTOR_USART_IRQn;	  // 串口2中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器
}

void USART2_IRQHandler(void) // 串口2中断服务程序
{
	if (USART_GetITStatus(MOTOR_USART, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(MOTOR_USART, USART_IT_RXNE);
		Modbus_Master_Receive(USART_ReceiveData(MOTOR_USART));
	}
}

void Slave_USART_Init(u32 bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_Cmd(USART1, ENABLE);
	USART1_Tx_InitDMA();
}

static u8 DMA_Buffer[256];
static u8 DMA_Status = 0;

void USART1_Tx_InitDMA(void)
{
	DMA_InitTypeDef DMA_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	DMA_DeInit(DMA2_Stream7);
	while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE)
	{
	} // 等待DMA可配置

	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&USART1->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)DMA_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_Init(DMA2_Stream7, &DMA_InitStructure); // 初始化DMA Stream

	DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7); // 清除传输完成标志

	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE); // 使能串口1的DMA发送

	// Init DMA send finish interrupt
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE); // 传输完成启用中断
}

// 设置DMA缓存大小并且使能DMA数据流
void USART1_SendWithDMA(u8 *data, u32 len)
{
	while (DMA_Status == 1)
		;
	memcpy(DMA_Buffer, data, len); // 将输入数据复制到DMA缓冲区中，确保所有需要发送的数据都已加载完毕。
	while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE)
	{
	}
	DMA_Cmd(DMA2_Stream7, DISABLE);			   // 关闭DMA通道
	DMA_SetCurrDataCounter(DMA2_Stream7, len); // 数据的传输量
	DMA_Cmd(DMA2_Stream7, ENABLE);			   // 开启DMA传输
	DMA_Status = 1;
}

void USART1_SendByte(u8 byte)
{
	USART_SendData(USART1, byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		; // 标志位表示发送数据缓冲区（数据寄存器）是否为空。
}

void USART1_SendBytes(u8 *bytes, u32 len)
{
	while (len--)
	{
		USART1_SendByte(*bytes++);
	}
}

void USART1_SendString(u8 *str)
{
	while (*str)
	{
		USART1_SendByte(*str++);
	}
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		ModBusRecByte(USART_ReceiveData(USART1)); // 读取串口1的数据到modbus
	}
}

void DMA2_Stream7_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7) != RESET)
	{
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
		DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7);
		DMA_Status = 0;
		Modbus_ResetToIdle();
	}
}

void WriteToFlash(u32 *dataArray, u16 len, u8 sector, u8 *flashAddr)
{
	FLASH_Unlock();
	FLASH_EraseSector(sector, VoltageRange_3);
	// Loop to program all data
	for (u16 i = 0; i < len; i += 4)
	{
		FLASH_ProgramWord((u32)(flashAddr + i), *(u32 *)(dataArray + i));
	}
	FLASH_Lock();
}
