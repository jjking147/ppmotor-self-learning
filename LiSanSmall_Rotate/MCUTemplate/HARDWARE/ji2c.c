#include "ji2c.h" 
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

static JI2C_Message_Type JI2C_Message = { .Status= JI2C_State_Idle };

JI2C_State_Type JI2C_GetState(void)
{
	return JI2C_Message.Status;
}

static void JI2C_IO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_I2C1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_I2C1);

	GPIO_SetBits(GPIOB, GPIO_Pin_6);
	GPIO_SetBits(GPIOB, GPIO_Pin_7);
}

static void JI2C_IIC_Init(u8 ownaddr,u32 speed)
{
	I2C_InitTypeDef I2C_InitStructure;
	I2C_DeInit(I2C1);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = ownaddr;  //从设备地址
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = speed;  //SCL最大400KHz

	I2C_Init(I2C1, &I2C_InitStructure);


	//禁止时钟延长
	I2C1->CR1 |= 0x80;
}

static void JI2C_Interrupt_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
}

void JI2C_Init(uint32_t speed)
{
	JI2C_IO_Init();
	JI2C_IIC_Init(JI2C_OWNADDR, speed);
	JI2C_Interrupt_Init();
}

void JI2C_ReadData_Async(uint8_t slav_addr, uint16_t bytes2read, uint8_t* buffer, JI2C_TRANSFINISH_CB callback)
{
	JI2C_Message.Status = JI2C_State_Start_R;
	JI2C_Message.SlaveAddr.Addr8 = slav_addr;
	JI2C_Message.Index = 0;
	JI2C_Message.Buffer = buffer;
	JI2C_Message.Len = bytes2read;
	JI2C_Message.TransType = JI2C_TType_Read;
	JI2C_Message.Callback = callback;

	I2C_ITConfig(I2C1, I2C_IT_BUF, DISABLE);
	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
	I2C1->CR1 = (I2C_CR1_START | I2C_CR1_PE);	//1 触发起始中断

}

void JI2C_WriteReg8_Async(uint8_t slav_addr,uint8_t regaddr,uint16_t bytes2write, uint8_t* buffer, JI2C_TRANSFINISH_CB callback)
{
	JI2C_Message.Status = JI2C_State_Start;
	JI2C_Message.SlaveAddr.Addr8 = slav_addr;
	JI2C_Message.Index = 0;
	JI2C_Message.Buffer = buffer;
	JI2C_Message.Len = bytes2write;
	JI2C_Message.RegAddr.Hi8 = regaddr;
	JI2C_Message.TransType = JI2C_TType_Write;
	JI2C_Message.Callback = callback;

	I2C_ITConfig(I2C1, I2C_IT_BUF, DISABLE);
	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
	I2C1->CR1 = (I2C_CR1_START | I2C_CR1_PE);	//1 触发起始中断
}

void JI2C_ReadReg8_Async(uint8_t slav_addr,uint8_t regaddr,uint16_t bytes2read, uint8_t* buffer, JI2C_TRANSFINISH_CB callback)
{
	JI2C_Message.Status = JI2C_State_Start;
	JI2C_Message.SlaveAddr.Addr8 = slav_addr;
	JI2C_Message.Index = 0;
	JI2C_Message.Buffer = buffer;
	JI2C_Message.Len = bytes2read;
	JI2C_Message.RegAddr.Hi8 = regaddr;
	JI2C_Message.TransType = JI2C_TType_Read;
	JI2C_Message.Callback = callback;

	I2C_ITConfig(I2C1, I2C_IT_BUF, DISABLE);
	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
	I2C1->CR1 = (I2C_CR1_START | I2C_CR1_PE);	//1 触发起始中断
}




void  I2C1_ER_IRQHandler(void)	//I2C Error Interrupt
{
	//OSAL_IRQ_PROLOGUE();

	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF))
	{
		I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, DISABLE);

		I2C_ClearFlag(I2C1, I2C_FLAG_AF);
	
	}
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BERR))
	{
		I2C_ClearFlag(I2C1, I2C_FLAG_BERR);
	}
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_OVR))
	{
		I2C_ClearFlag(I2C1, I2C_FLAG_OVR);
	}
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ARLO))
	{
		I2C_ClearFlag(I2C1, I2C_FLAG_ARLO);
	}
	
	JI2C_Message.Status = JI2C_State_Error;
		if (JI2C_Message.Callback != 0)
			(*JI2C_Message.Callback)(false, JI2C_Message.Buffer);
		JI2C_Message.Status = JI2C_State_Idle;

	//OSAL_IRQ_EPILOGUE();
}




void  I2C1_EV_IRQHandler(void)	//I2C Event Interrupt
{
	volatile uint32_t SR1 = I2C1->SR1;
	volatile uint32_t SR2;
	//OSAL_IRQ_PROLOGUE();

	if (SR1 & I2C_SR1_SB)
	{
		if (JI2C_Message.Status == JI2C_State_Start)
		{
			I2C_Send7bitAddress(I2C1, JI2C_Message.SlaveAddr.Addr8, I2C_Direction_Transmitter);//2
			JI2C_Message.Status = JI2C_State_Address;
		}
		else if (JI2C_Message.Status == JI2C_State_Start_R)
		{
			I2C_AcknowledgeConfig(I2C1, ENABLE);//5 转为读数据
			I2C_Send7bitAddress(I2C1, JI2C_Message.SlaveAddr.Addr8, I2C_Direction_Receiver);//2
			JI2C_Message.Status = JI2C_State_Reading;
		}
		else if (JI2C_Message.Status == JI2C_State_Start_W)
		{
			I2C_Send7bitAddress(I2C1, JI2C_Message.SlaveAddr.Addr8, I2C_Direction_Transmitter);//2
			JI2C_Message.Status = JI2C_State_Writing;
		}
	}
	else if (SR1 & I2C_SR1_ADDR)	//一次地址传输完毕
	{
		SR2 = I2C1->SR2;
		if (JI2C_Message.Status == JI2C_State_Address)
		{
			I2C_SendData(I2C1, JI2C_Message.RegAddr.Hi8);	//4
		}
		else if (JI2C_Message.Status == JI2C_State_Writing)
		{
			I2C_SendData(I2C1, JI2C_Message.Buffer[JI2C_Message.Index++]);
			if (JI2C_Message.Index == JI2C_Message.Len)
			{
				I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, DISABLE);//关闭中断
				I2C1->CR1 = (I2C_CR1_STOP | I2C_CR1_PE);//后续测试是否添加
				JI2C_Message.Status = JI2C_State_Finished;
			}
		}

		if (JI2C_Message.Len == 1 && JI2C_Message.Status == JI2C_State_Reading)
		{
			I2C_AcknowledgeConfig(I2C1, DISABLE);
		}
		I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
	}
	else if (SR1 & I2C_SR1_BTF)	//一次传输完毕中断
	{
		if (SR1 & I2C_SR1_TXE)//发送完成
		{
			if(JI2C_Message.Is16Bit)
			{
				JI2C_Message.Is16Bit = 0;
				I2C_SendData(I2C1, JI2C_Message.RegAddr.Lo8);
			}
			else
			{
				if (JI2C_Message.TransType == JI2C_TType_Read)//读数据
			{
				if (JI2C_Message.Status == JI2C_State_Address)//寄存器发送完
				{
					I2C1->CR1 = (I2C_CR1_START | I2C_CR1_PE);//
					JI2C_Message.Status = JI2C_State_Start_R;//启动读数据
				}
				}
				else
				{
					//I2C_SendData(I2C1, JI2C_Message.RegAddr.Hi8 + JI2C_Message.Index);
					I2C_SendData(I2C1, JI2C_Message.Buffer[JI2C_Message.Index++]);
					if (JI2C_Message.Index == JI2C_Message.Len)
					{
						I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, DISABLE);//关闭中断
						I2C1->CR1 = (I2C_CR1_STOP | I2C_CR1_PE);//后续测试是否添加
						JI2C_Message.Status = JI2C_State_Finished;
					}
				}
			}	
		}
	}
	if (SR1 & I2C_SR1_RXNE)
	{
		JI2C_Message.Buffer[JI2C_Message.Index++] = I2C_ReceiveData(I2C1);
		if (JI2C_Message.Index == JI2C_Message.Len - 1)
		{
			I2C_AcknowledgeConfig(I2C1, DISABLE);//末尾数据禁止应答
		}
		if (JI2C_Message.Index == JI2C_Message.Len)
		{
			I2C1->CR2 = (I2C_CR1_STOP | I2C_CR1_PE);//后续测试是否添加
			I2C_ITConfig(I2C1, I2C_IT_EVT | I2C_IT_BUF, DISABLE);//接受结束
			JI2C_Message.Status = JI2C_State_Finished;
		}
	}

	if (JI2C_Message.Status == JI2C_State_Finished)
	{
		if(JI2C_Message.Callback != 0)
			(*JI2C_Message.Callback)(true, JI2C_Message.Buffer);
		JI2C_Message.Status = JI2C_State_Idle;
	}

	//OSAL_IRQ_EPILOGUE();
}
