#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "modbus_master.h"
#include "modbus_slave.h"

//extern u8 MOTOR_ADDRESS;
#define   MOTOR_ADDRESS	MOTOR_ADDR
extern u8 JODELL_MOTOR_ADDRESS;
volatile ModBusState Master_State = MODBUS_IDLE;
static volatile u16 Master_Rec_Count = 0;
u8 Master_Receive_Buff[MODBUS_MASTER_BUFF_LEN];
u16 USART_RX_STA = 0;

void Master_Set_Idle(void)
{
	Master_State = MODBUS_IDLE;
}

void Master_Set_Timeout(void)
{
	Master_State = MODBUS_TIMEOUT;
}

void Modbus_Master_Receive(u8 data)
{
	if(Master_State == MODBUS_WAIT_RESPONSE)
	{
		Master_Receive_Buff[0] = data;
		Master_Rec_Count = 1;
		TIM_Cmd(MODBUS_MASTER_TIM, ENABLE);;
		TIM_SetCounter(MODBUS_MASTER_TIM, 0);
		Master_State = MODBUS_RECING;
	}
	else if(Master_State == MODBUS_RECING)
	{
		Master_Receive_Buff[Master_Rec_Count++] = data;
		if(Master_Rec_Count >= MODBUS_BUFF_LEN)
		{
			Master_Rec_Count = 0;
		  TIM_SetCounter(MODBUS_MASTER_TIM, 0);
		}
	}
}

void Modbus_Master_SendUART(u8* data,u16 crc_result,u16 len)
{
	for(u8 i=0;i<len;i++)
	{
		USART_SendData(MOTOR_USART, data[i]);
		while(USART_GetFlagStatus(MOTOR_USART,USART_FLAG_TC)!=SET);
	}
	
	USART_SendData(MOTOR_USART, crc_result);
	while(USART_GetFlagStatus(MOTOR_USART,USART_FLAG_TC)!=SET);
	crc_result = crc_result >> 8;
	USART_SendData(MOTOR_USART, crc_result);
	while(USART_GetFlagStatus(MOTOR_USART, USART_FLAG_TC)!=SET);
}

void Send_Func03_Data(u16 Reg, u16 Num, MODBUS_RESPONSE_CALLBACK fun)
{
	u8 USART_TX_BUF[6];
	u16 crc_result;
	
	if(Master_State == MODBUS_IDLE)
	{
		Master_State = MODBUS_SENDING;
		USART_TX_BUF[0] = MOTOR_ADDRESS;
		USART_TX_BUF[1] = 0x03;
		USART_TX_BUF[2] = (Reg >> 8) & 0xFF;
		USART_TX_BUF[3] = Reg & 0xFF;
		USART_TX_BUF[4] = (Num >> 8) & 0xFF;
		USART_TX_BUF[5] = Num & 0xFF;

		crc_result = Calculate_CRC(USART_TX_BUF, 6);

		Modbus_Master_SendUART(USART_TX_BUF,crc_result,6);
		
		Master_State = MODBUS_WAIT_RESPONSE;
		
		u32 start = ReadTick();
		while(Master_State != MODBUS_TIMEOUT)
		{
			if(TickSpan(start) > MODBUS_MASTER_LOSS_TIME)
			{
				if(fun != 0)
				{
					Master_State = MODBUS_HANDLING;
					(*fun)(MODBUS_ERR_NoResponse,0,0);
				}
				Master_State = MODBUS_IDLE;
				return;
			}
		}
		
		if(Master_Receive_Buff[1] & 0x80)
		{
			if(fun != 0)
			{
				Master_State = MODBUS_HANDLING;
				(*fun)(MODBUS_ERR_Failure,0,0);
			}
			Master_State = MODBUS_IDLE;
			return;
		}
		
		if(fun != 0)
		{
			Master_State = MODBUS_HANDLING;
			(*fun)(MODBUS_ERR_NoResponse,Master_Receive_Buff,Master_Rec_Count);
			Master_State = MODBUS_IDLE;
		}
		Master_State = MODBUS_IDLE;
	}
}

ModBusFailCode_Type Send_Func06_Data(u16 Reg, u16 Cmd)
{
	u8 USART_TX_BUF[6];
	u16 crc_result;
	
	if(Master_State == MODBUS_IDLE)
	{
		Master_State = MODBUS_SENDING;
		USART_TX_BUF[0] = MOTOR_ADDRESS;
		USART_TX_BUF[1] = 0x06;
		USART_TX_BUF[2] = (Reg >> 8) & 0xFF;
		USART_TX_BUF[3] = Reg & 0xFF;
		USART_TX_BUF[4] = (Cmd >> 8) & 0xFF;
		USART_TX_BUF[5] = Cmd & 0xFF;
		
		crc_result = Calculate_CRC(USART_TX_BUF, 6);
		
		Modbus_Master_SendUART(USART_TX_BUF,crc_result,6);
		
		Master_State = MODBUS_WAIT_RESPONSE;
		
		u32 start = ReadTick();
		while(Master_State != MODBUS_TIMEOUT)
		{
			if(TickSpan(start) > MODBUS_MASTER_LOSS_TIME)
			{
				Master_State = MODBUS_IDLE;
				return MODBUS_ERR_NoResponse;
			}
		}
		
		if(Master_Receive_Buff[1] & 0x80)
		{
			Master_State = MODBUS_IDLE;
			return MODBUS_ERR_Failure;
		}
		Master_State = MODBUS_IDLE;
		return MODBUS_OK;
	}
	return MODBUS_ERR_Busy;
}

ModBusFailCode_Type Send_Func10_Data(u16 Reg, u32 Cmd)
{
	u8 USART_TX_BUF[11];
	u16 crc_result;
	
	if(Master_State == MODBUS_IDLE)
	{
		Master_State = MODBUS_SENDING;
		USART_TX_BUF[0] = MOTOR_ADDRESS;
		USART_TX_BUF[1] = 0x10;
		USART_TX_BUF[2] = (Reg >> 8) & 0xFF;
		USART_TX_BUF[3] = Reg & 0xFF;
		USART_TX_BUF[4] = 0x00;
		USART_TX_BUF[5] = 0x02;
		USART_TX_BUF[6] = 0x04;
		USART_TX_BUF[7] = (Cmd >> 8) & 0xFF;
		USART_TX_BUF[8] = Cmd & 0xFF;
		USART_TX_BUF[9] = (Cmd >> 24) & 0xFF;
		USART_TX_BUF[10] = (Cmd >> 16) & 0xFF;
		
		crc_result = Calculate_CRC(USART_TX_BUF, 11);
		
		Modbus_Master_SendUART(USART_TX_BUF,crc_result,11);
		
		Master_State = MODBUS_WAIT_RESPONSE;
		
		u32 start = ReadTick();
		while(Master_State != MODBUS_TIMEOUT)
		{
			if(TickSpan(start) > MODBUS_MASTER_LOSS_TIME)
			{
				Master_State = MODBUS_IDLE;
				return MODBUS_ERR_NoResponse;
			}
		}
		
		if(Master_Receive_Buff[1] & 0x80)
		{
			Master_State = MODBUS_IDLE;
			return MODBUS_ERR_Failure;
		}
		Master_State = MODBUS_IDLE;
		return MODBUS_OK;
	}
	return MODBUS_ERR_Busy;
}
	
u16 Calculate_CRC(const u8 *buffer, u16 length)
{
  u16 crc = 0xffff;
	uint16_t pos;
	u8 i;
 
	for(pos = 0; pos < length; pos++)
	{
		crc ^= (uint16_t)buffer[pos];

		for(i= 8; i != 0; i--)
		{
			if((crc & 0x0001) != 0)
			{
				crc >>= 1;
				crc ^= 0xa001;
			}
			else
			{
				crc >>= 1;
			}
		}
	}
  return crc;
}

//以下为夹爪电机函数	
ModBusFailCode_Type Send_Func10_Data_JODELL(u16 Reg, u16 Cmd)
{
	u8 USART_TX_BUF[9];
	u16 crc_result;
	
	if(Master_State == MODBUS_IDLE)
	{
		Master_State = MODBUS_SENDING;
		USART_TX_BUF[0] = JODELL_MOTOR_ADDRESS;
		USART_TX_BUF[1] = 0x10;
		USART_TX_BUF[2] = (Reg >> 8) & 0xFF;
		USART_TX_BUF[3] = Reg & 0xFF;
		USART_TX_BUF[4] = 0x00;
		USART_TX_BUF[5] = 0x01;
		USART_TX_BUF[6] = 0x02;
		USART_TX_BUF[7] = (Cmd >> 8) & 0xFF;
		USART_TX_BUF[8] = Cmd & 0xFF;
		
		crc_result = Calculate_CRC(USART_TX_BUF, 9);
		
		Modbus_Master_SendUART(USART_TX_BUF,crc_result,9);
		
		Master_State = MODBUS_WAIT_RESPONSE;
		
		u32 start = ReadTick();
		while(Master_State != MODBUS_TIMEOUT)
		{
			if(TickSpan(start) > MODBUS_MASTER_LOSS_TIME)
			{
				Master_State = MODBUS_IDLE;
				return MODBUS_ERR_NoResponse;
			}
		}
		
		if(Master_Receive_Buff[1] & 0x80)
		{
			Master_State = MODBUS_IDLE;
			return MODBUS_ERR_Failure;
		}
		Master_State = MODBUS_IDLE;
		return MODBUS_OK;
	}
	return MODBUS_ERR_Busy;
}

ModBusFailCode_Type Send_Func10_Data_JODELL_3(u16 Reg, u16 Cmd1,u16 Cmd2,u16 Cmd3)
{
	u8 USART_TX_BUF[13];
	u16 crc_result;
	
	if(Master_State == MODBUS_IDLE)
	{
		Master_State = MODBUS_SENDING;
		USART_TX_BUF[0] = JODELL_MOTOR_ADDRESS;
		USART_TX_BUF[1] = 0x10;
		USART_TX_BUF[2] = (Reg >> 8) & 0xFF;
		USART_TX_BUF[3] = Reg & 0xFF;
		USART_TX_BUF[4] = 0x00;
		USART_TX_BUF[5] = 0x03;
		USART_TX_BUF[6] = 0x06;
		USART_TX_BUF[7] = (Cmd1 >> 8) & 0xFF;
		USART_TX_BUF[8] = Cmd1 & 0xFF;
		USART_TX_BUF[9] = (Cmd2 >> 8) & 0xFF;
		USART_TX_BUF[10] = Cmd2 & 0xFF;
		USART_TX_BUF[11] = (Cmd3 >> 8) & 0xFF;
		USART_TX_BUF[12] = Cmd3 & 0xFF;
		
		crc_result = Calculate_CRC(USART_TX_BUF, 13);
		
		Modbus_Master_SendUART(USART_TX_BUF,crc_result,13);
		
		Master_State = MODBUS_WAIT_RESPONSE;
		
		u32 start = ReadTick();
		while(Master_State != MODBUS_TIMEOUT)
		{
			if(TickSpan(start) > MODBUS_MASTER_LOSS_TIME)
			{
				Master_State = MODBUS_IDLE;
				return MODBUS_ERR_NoResponse;
			}
		}
		
		if(Master_Receive_Buff[1] & 0x80)
		{
			Master_State = MODBUS_IDLE;
			return MODBUS_ERR_Failure;
		}
		Master_State = MODBUS_IDLE;
		return MODBUS_OK;
	}
	return MODBUS_ERR_Busy;
}

ModBusFailCode_Type Send_Func04_Data_JODELL(u16 Reg, u16 Cmd)
{
	u8 USART_TX_BUF[6];
	u16 crc_result;
	
	if(Master_State == MODBUS_IDLE)
	{
		Master_State = MODBUS_SENDING;
		USART_TX_BUF[0] = JODELL_MOTOR_ADDRESS;
		USART_TX_BUF[1] = 0x04;
		USART_TX_BUF[2] = (Reg >> 8) & 0xFF;
		USART_TX_BUF[3] = Reg & 0xFF;
		USART_TX_BUF[4] = (Cmd >> 8) & 0xFF;
		USART_TX_BUF[5] = Cmd & 0xFF;
		
		crc_result = Calculate_CRC(USART_TX_BUF, 6);
		
		Modbus_Master_SendUART(USART_TX_BUF,crc_result,6);
		
		Master_State = MODBUS_WAIT_RESPONSE;
		
		u32 start = ReadTick();
		while(Master_State != MODBUS_TIMEOUT)
		{
			if(TickSpan(start) > MODBUS_MASTER_LOSS_TIME)
			{
				Master_State = MODBUS_IDLE;
				return MODBUS_ERR_NoResponse;
			}
		}
		
		if(Master_Receive_Buff[1] & 0x80)
		{
			Master_State = MODBUS_IDLE;
			return MODBUS_ERR_Failure;
		}
		Master_State = MODBUS_IDLE;
		return MODBUS_OK;
	}
	return MODBUS_ERR_Busy;
}
