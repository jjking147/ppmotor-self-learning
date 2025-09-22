#include "bll_motor.h" 
#include "sys.h"
#include "delay.h" 
#include "modbus_master.h"

extern u8 Master_Receive_Buff[MODBUS_MASTER_BUFF_LEN];

void Motor_Enable(void)
{
	Send_Func06_Data(ENABLE_REGISTER, ENABLE_CMD);
}

void Motor_Disable(void)
{
	Send_Func06_Data(ENABLE_REGISTER, DISABLE_CMD);
}

void Check_Fault(void)
{
	Send_Func03_Data(FAULT_REGISTER, 1, 0);
}

void Forward_Run(void)
{
	Send_Func06_Data(RUN_REGISTER, FORWARD_RUN_CMD);
}

void Reverse_Run(void)
{
	Send_Func06_Data(RUN_REGISTER, REVERSE_RUN_CMD);
}

void Stop(void)
{
	Send_Func06_Data(RUN_REGISTER, 0);
}

void Brake(void)
{
	Send_Func06_Data(RUN_REGISTER, BRAKE_CMD);
}

void Set_Acce_Time(u16 Time)
{
	Send_Func06_Data(ACCE_TIME_REGISTER, Time);
}

void Set_Dece_Time(u16 Time)
{
	Send_Func06_Data(DECE_TIME_REGISTER, Time);
}

void Set_Speed(s32 Speed)
{
	Send_Func10_Data(SPEED_REGISTER, Speed);
}

void Set_Relative_Position(s32 Pluse)
{
	Send_Func10_Data(RELATIVE_POSITION_REGISTER, Pluse);
}

void Set_Absolute_Position(s32 Position)
{
	Send_Func10_Data(ABSOLUTE_POSITION_REGISTER, Position);
}

void Clear_Position(void)
{
	Send_Func10_Data(SET_POSITION_REGISTER, 0);
}

static u8 wait_flag = 0, code = 0;
static void Check_Status_Cb(ModBusFailCode_Type err,u8* data,u16 len)
{
	wait_flag = 0;
	code = data[3];
}

u8 Check_Status(void)
{
	wait_flag = 1;
	Send_Func03_Data(STATUS_REGISTER, 1, Check_Status_Cb);
	while(wait_flag);
	return code;
}

//static int position = 0;
//static void Get_Position_Cb(ModBusFailCode_Type err,u8* data,u16 len)
//{
//	wait_flag = 0;
//	position = data[5]<<24 | data[6]<<16 | data[3]<<8 | data[4];
//}

//s32 Get_Position(void)
//{
//	wait_flag = 1;
//	Send_Func03_Data(REAL_POSITION_REGISTER, 2, Get_Position_Cb);
//	while(wait_flag);
//	return position;
//}

//夹爪控制函数
void JODELL_STATUS(u16 rACT)
{
	Send_Func10_Data_JODELL(JODELL_CONTROL_REGISTER, rACT);//0x0:停止电动夹爪，0x1:使能电动夹爪
}

void READ_JODELL_STATUS_1(void)//读取夹爪激活状态
{
	Send_Func04_Data_JODELL(JODELL_STATUS_REGISTER_0, 0x0001);
}

void READ_JODELL_STATUS_2(void)//读取夹爪打开状态
{
	Send_Func04_Data_JODELL(JODELL_STATUS_REGISTER_0, 0x0003);
}

uint8_t READ_JODELL_STATUS_NUMBER_1(void)
{
	READ_JODELL_STATUS_1();
	
	return Master_Receive_Buff[5];
}
uint8_t READ_JODELL_STATUS_NUMBER_2(void)
{
	READ_JODELL_STATUS_2();

	return Master_Receive_Buff[5];
}

void READ_JODELL_STATUS_3(void)//读取夹爪状态,寄存器 07D0 的内容,寄存器 07D0 的内容,寄存器 07D2 的内容
{
	Send_Func04_Data_JODELL(JODELL_STATUS_REGISTER_0, 0x0003);
}

void Check_JODELL_STATUS_OPEN_CLOSE(void) 
{
	int retries = 0;
	READ_JODELL_STATUS_2(); // 初始读取

	if((Master_Receive_Buff[5] & 0xF1) != 0xF1) 
	{
		// 持续轮询，直到状态符合条件或达到最大重试次数
		while (READ_JODELL_STATUS_NUMBER_2() != 0xF1 && retries < 50)
		{
			delay_ms(10); // 避免忙等待
			retries++;
		}

		if (retries >= 50) 
		{
			// 处理超时情况
			// 例如，可以记录日志或者设置错误标志
		}
	}
}

void JODELL_OPEN_CLOSE_ACTIVETION(u8 position,u8 POWER,u8 SPEED)//u8 position,u8 POWER,u8 SPEED 范围00~FF
{
	uint16_t POWER_SPEED = ((uint16_t)POWER << 8) | SPEED;
	uint16_t POSITION = (uint16_t)position << 8;
	Send_Func10_Data_JODELL_3(JODELL_CONTROL_REGISTER, 0x0009,POSITION,POWER_SPEED);
}
