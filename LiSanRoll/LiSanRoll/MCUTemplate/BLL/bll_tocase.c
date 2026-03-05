#include "bll_tocase.h"
#include "bll_adapter.h"
#include "bll_motor.h"
#include "modbus_master.h"
#include "config.h"
#include "delay.h"
#include "usart.h"
#include "gpio.h"
#include "timer.h"

static CommonStateFlag_Type roll_flag = CSF_Idel;//声明原来的状态
static vu8 kind;
static vu8 swtich1_flag;
static vu8 swtich2_flag;
static vu8 swtich3_flag;
static vu8 finish_flag;
extern u8 MOTOR_ADDRESS;

//extern u8 Master_Receive_Buff[MODBUS_MASTER_BUFF_LEN];


#define PP_ROLL_IN_DELAY		(4100-2500)	//证卷入延时
#define CARD_ROLL_IN_DELAY		(2900-1500)	//卡卷入延时
#define ROLL_OUT_DELAY			(4900)	//卷出延时
#define BATCH_IN_DELAY			(3900)	//批量口延时


//清除flag状态
void BLL_Roll_ClearFlag(void)
{
	roll_flag = CSF_Idel;
}

static void Clear_Manual_Flags(void)
{
	swtich1_flag = 0;
	swtich2_flag = 0;
	swtich3_flag = 0;
	finish_flag = 0;
}

#define WAIT_MOTOR_STOP(span,n,label)	{ \
	u16 _motor_sate = 0,_retry = 0; \
	do \
	{ \
		if(_retry++ >= n) \
		{ \
			*err = Failure_Timeout; \
			goto label; \
		} \
		delay_ms(span); \
		_motor_sate = Check_Status(); \
	}while((_motor_sate & 0x02) == 0x02); \
}

void delay_s(u16 s)
{
	while(s--)
		delay_ms(1000);
}

__forceinline static void delay(u16 ms)
{
	delay_s(ms/1000);
	delay_ms(ms%1000);
}

u8 WaitRolling(u8 sw,u16 ms)
{
	u32 start = ReadTick();
	while(TickSpan(start) < 20000)
	{
		u8 state = Read_Switch(sw);
		if(state)
		{
			delay_ms(10);
			if(Read_Switch(sw))
			{
				delay(ms);
				Stop();
				return 0;
			}
		}
	}
	Stop();
	return Failure_Timeout;
}

CommonStateFlag_Type BLL_Roll_Execute(ParamShadow_Type params, u8 *err)
{
	if(roll_flag == CSF_Idel)
	{
		//Step0：初始化各种标志位等
		roll_flag = CSF_Working;
		
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);	
		Clear_Manual_Flags();
		
		if(params.Param2 == 1)
		{
			BLL_Motor_AD_SpeedMode(50,80,200);//150-300
		}
		else if(params.Param2 == 2)
		{
			BLL_Motor_AD_SpeedMode(50,80,-200);
		}
		
		switch(params.Param1){
		case 1:	//卡卷入
			kind = 1;
			*err = WaitRolling(1,PP_ROLL_IN_DELAY);
			goto die;
			break;
		case 2:	//证卷入
			kind = 2;
			*err = WaitRolling(2,CARD_ROLL_IN_DELAY);
			goto die;
			break;
		case 3:	//出口
			kind = 3;
			delay(ROLL_OUT_DELAY);
			Stop();
			break;
		case 4:	//批量
			kind = 4;
			*err = WaitRolling(3,BATCH_IN_DELAY);
			goto die;
			break;
		default:
			break;
		}
		
		//Step4：死等光电触发
//		u32 start = ReadTick();
//		while(finish_flag == 0)
//		{
//			if(TickSpan(start) > 10000)
//			{
//				*err = Failure_Timeout;
//				goto die;
//			}
//		}
		//Step5：停止电机
		Stop();
	}

//最终程序出口
die:	
	roll_flag = CSF_Finished;
	Clear_Manual_Flags();
	return roll_flag;
}

void SWTICH1_INT_HANDLER(void)
{
	if(roll_flag == CSF_Working) 
	{
		if(kind == 1)
		{
			delay_us(100);
			if(Read_Switch(1) == SET)
			{
				finish_flag  = 1;		
			}
		}
	}
}

void SWTICH2_INT_HANDLER(void)
{
	if(roll_flag == CSF_Working) 
	{
		if(kind == 2)
		{
			delay_us(100);
			if(Read_Switch(2) == SET && swtich1_flag == 0)
			{
				swtich1_flag  = 1;			
			}
			if(Read_Switch(1) == SET && swtich1_flag == 1)
			{
				finish_flag  = 1;		
			}
		}
	}
}

void SWTICH3_INT_HANDLER(void)
{
	if(roll_flag == CSF_Working) 
	{
		if(kind == 4)
		{
			delay_us(100);
			if(Read_Switch(3) == SET)
			{
				finish_flag  = 1;	
			}
		}
	}
}
