#include "bll_backzero.h"
#include "bll_adapter.h"
#include "bll_motor.h"
#include "modbus_master.h"
#include "config.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"

static CommonStateFlag_Type backzero_flag = CSF_Idel;//声明原来的状态

extern volatile u8 has_zero_flag;
extern volatile u8 last_target;
extern volatile vu8 swtich_count;

void BLL_BackZero_ClearFlag(void)
{
	backzero_flag = CSF_Idel;
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
	}while((_motor_sate & 0x03) == 0x03); \
}

CommonStateFlag_Type BLL_BackZero_Execute(ParamShadow_Type params, u8 *err)
{
	if(backzero_flag == CSF_Idel)
	{
		//Step0：初始化各种标志位等
		backzero_flag = CSF_Working;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);	
		
		//Step1：进行回零
		BLL_Moter_AD_BackZero(1,30);
		WAIT_MOTOR_STOP(100,200,die);	//100ms查一次，查200次不行就超时
		Clear_Position();
		has_zero_flag = 1;
		last_target = 0;
		swtich_count = 0;
	}
	
//最终程序出口
die:	
	backzero_flag = CSF_Finished;
	return backzero_flag;
}
