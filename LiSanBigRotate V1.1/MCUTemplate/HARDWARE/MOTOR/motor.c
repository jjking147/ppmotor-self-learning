#include "sys.h"
#include "motor.h"
#include "usart.h" 
#include "modbus_master.h"

extern u8 MOTOR_ADDRESS;

void Motor_Init(void)
{
	Driver_Power_On();
	Init_Parameter();
	Motor_Enable();
}

void Driver_Power_On(void)//电机启动
{
	Send_Func06_Data(CONTROL_REGISTER, DRIVER_POWER_ON_CMD);
}

void Init_Parameter(void)
{
	Send_Func06_Data(CONTROL_REGISTER, INIT_PARA_CMD);
}

void Motor_Enable(void)
{
	Send_Func06_Data(CONTROL_REGISTER, MOROR_ENABLE_CMD);
}

void Stop(void)
{
	Send_Func06_Data(CONTROL_REGISTER, STOP_CMD);
}


//快速停止代码
void Free_Stop(void)
{
	Send_Func06_Data(STOP_REGISTER, FREE_STOP_CMD);//0000
}

void Dece_Stop_Motor_Disable(void)
{
	Send_Func06_Data(STOP_REGISTER, DECE_STOP_MOTOR_DISABLE_CMD);//0001
}

void Fast_Dece_Stop_Motor_Disable(void)//0002
{
	Send_Func06_Data(STOP_REGISTER, FAST_DECE_STOP_MOTOR_DISABLE_CMD);
}

void Dece_Stop_Motor_Enable(void)//0005
{
	Send_Func06_Data(STOP_REGISTER, DECE_STOP_MOTOR_ENABLE_CMD);
}

void Fast_Dece_Stop_Motor_Enable(void)//0006
{
	Send_Func06_Data(STOP_REGISTER, FAST_DECE_STOP_MOTOR_ENABLE_CMD);
}

//运动模式控制寄存器
void Set_Position_Mode(void)
{
	u8 err = Send_Func06_Data(MODOL_REGISTER, POSITION_MODOL_CMD);
}

void Set_Speed_Mode(void)
{
	Send_Func06_Data(MODOL_REGISTER, SPEED_MODOL_CMD);
}

void Position_Mode_Set_Speed(u16 Speed)
{
	Send_Func10_Data(POSITION_MODE_SPEED_REGISTER, Speed);//位置模式设置运行速度
}

void Speed_Mode_Set_Speed(s16 Speed1)
{
	Send_Func10_Data(SPEED_MODE_SPEED_REGISTER, Speed1);//速度模式设置运行速度
}

void Set_Back_Zero_Mode(void)//回零模式
{
	Send_Func06_Data(MODOL_REGISTER, BACK_ZERO_MODOL_CMD);
}

void Set_Acce(u16 Acce)
{
	Acce = Acce * 10;
	Send_Func10_Data(ACCE_REGISTER, Acce);//加速度
}

void Set_Dece(u16 Dece)
{
	Dece = Dece * 10;
	Send_Func10_Data(DECE_REGISTER, Dece);//减速度
}

void Set_Distence(u16 Distence)
{
	Send_Func10_Data(DISTANCE_REGISTER, Distence);
}

void Set_Abso_Position_Mode(void)//配置绝对位置设置
{
	Send_Func06_Data(CONTROL_REGISTER, SET_ABSO_POSITION_CMD);
}

void Set_Rela_Position_Mode(void)//重新配置相对位置
{
	Send_Func06_Data(CONTROL_REGISTER, SET_RELA_POSITION_CMD);
}

void Abso_Position_Start_Sample(void)//开始绝对位置采样
{
	Send_Func06_Data(CONTROL_REGISTER, ABSO_POSITION_START_SAMPLE_CMD);
}

void Rela_Position_Start_Sample(void)//开始相对位置采样。
{
	Send_Func06_Data(CONTROL_REGISTER, RELA_POSITION_START_SAMPLE_CMD);
}

void Set_Neg_Limit_Mode(void)
{
	Send_Func06_Data(BACK_ZERO_MODE_REGISTER, SET_NEG_LIMIT_CMD);
}

void Set_Back_Zero_Acce(u16 Acce)
{
	Acce = Acce * 10;
	Send_Func10_Data(BACK_ZERO_ACCE_REGISTER, Acce);
}

void Set_Back_Zero_Speed(u16 Speed)
{
	Send_Func10_Data(BACK_ZERO_SPEED_REGISTER, Speed);
}

static u8 wait_flag = 0, code = 0;
static void Check_Status_Cb(ModBusFailCode_Type err,u8* data,u16 len)
{
	wait_flag = 0;
	code = data[3];
}

u8 Check_Status(void)
{
	Send_Func03_Data(STATUS_REGISTER, 1, Check_Status_Cb);
	wait_flag = 1;
	while(wait_flag);
	return code;
}

void Check_Fault(void)
{
	Send_Func03_Data(FAULT_REGISTER, 1, 0);
}

void Set_DI1_Func(u16 Func)
{
	Send_Func06_Data(DI1_FUNC_REGISTER, Func);
}
