#include "bll_backzero.h"
#include "bll_adapter.h"
#include "bll_motor.h"
#include "modbus_master.h"
#include "config.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"

#include "jexception.h"

extern volatile u8 has_zero_flag;
//extern volatile s32 zero_position;

static CommonStateFlag_Type backzero_flag = CSF_Idel;//声明原来的状态

void BLL_BackZero_ClearFlag(void)
{
	backzero_flag = CSF_Idel;
}

static void MyDelay(u16 ms)
{
	u32 now = ReadTick();
	while(TickSpan(now) < ms);
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
		MyDelay(span); \
		_motor_sate = Check_Status(); \
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

CommonStateFlag_Type BLL_BackZero_Execute(ParamShadow_Type params, u8 *err)
{
	if(backzero_flag == CSF_Idel)
	{
		backzero_flag = CSF_Working;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);	
		
		BLL_Moter_AD_BackZero(5,500);
		//WAIT_MOTOR_STOP(100,200,die);
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
//	  zero_position = Get_Position();
		has_zero_flag = 1;
	}
	
//最终程序出口
die:	
	backzero_flag = CSF_Finished;
	return backzero_flag;
}
