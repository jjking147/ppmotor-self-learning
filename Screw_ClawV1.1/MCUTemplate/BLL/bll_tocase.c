#include "bll_tocase.h"
#include "bll_adapter.h"
#include "bll_motor.h"
#include "modbus_master.h"
#include "config.h"
#include "delay.h"
#include "usart.h"
#include "gpio.h"
#include "timer.h"

#include "jexception.h"

INIT_EXCEPTION_SYSTEM //仅需要初始化一次！

static CommonStateFlag_Type tocase_flag = CSF_Idel;//声明原来的状态
volatile u8 has_zero_flag = 0;
//volatile s32 zero_position = 0;

static const s32 position_table[] = 
{
	0,
	8200,//入证 8750 to 8500 to 8200
	13250,//入卡 13750 to 13250
	18750,//批量入证
	23400,//存证 23125 to 23400
	21750,//存卡 22000 to 21750
	15000,	//出证 7650 to 8500 to 8800 to 9200 to 9800 to 10500 to 13500 to 15000
	21500, //出卡 11250 to 19500 to 19750 to 20000 to 21500
	23500,	//取证 23125 to 23800 to 23500 to 23400 to 23600 to 23500
	22500		//取卡 22500
};

//清除flag状态
void BLL_ToCase_ClearFlag(void)
{
	tocase_flag = CSF_Idel;
}

static void MyDelay(u16 ms)
{
	u32 now = ReadTick();
	while(TickSpan(now) < ms);
}

#define WAIT_MOTOR_STOP(span,n,label)	{ \
	vu16 _motor_sate = 0,_retry = 0; \
	do \
	{ \
		if(_retry++ >= n) \
		{ \
			*err = Failure_Timeout; \
			goto label; \
		} \
		MyDelay(span); \
		_motor_sate = Check_Status();\
	}while((_motor_sate & 0x03) == 0x03); \
}

static void WaitMotorStop(u16 span, u16 n)
{
	vu16 _motor_sate = 0,_retry = 0; 
	do 
	{ 
		if(_retry++ >= n) 
		{ 
			throw(Failure_Timeout);
		} 
		MyDelay(span); 
		_motor_sate = Check_Status();
	}while((_motor_sate & 0x03) == 0x03); 
}

CommonStateFlag_Type BLL_ToCase_Execute(ParamShadow_Type params, u8 *err)
{
	if(tocase_flag == CSF_Idel)
	{
		//Step0：初始化各种标志位等
		tocase_flag = CSF_Working;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);	
		
		//Step1：进行回零
		if(has_zero_flag == 0)
		{
			BLL_Moter_AD_BackZero(5,600);
			//WAIT_MOTOR_STOP(200,300,die);	//200ms查一次，查300次不行就超时
			try
			{
				WaitMotorStop(200,300);
			}
			catch
			{
				*err = GetLastError();
				goto die;
			}
			Clear_Position();
			delay_ms(500);
//			zero_position = Get_Position();
			has_zero_flag = 1;
		}
		
		//Step2：快速绝对位移模式
		BLL_Motor_AD_AbsoluteMove(position_table[params.Param1],10,10,1200);
//		BLL_Motor_AD_AbsoluteMove(position_table[params.Param1]+zero_position,5,5,100);
		//WAIT_MOTOR_STOP(200,300,die);	//200ms查一次，查300次不行就超时
		try
		{
			WaitMotorStop(200,300);
		}
		catch
		{
			*err = GetLastError();
			goto die;
		}
	}

//最终程序出口
die:	
	tocase_flag = CSF_Finished;
	return tocase_flag;
}

