#include "bll_tocase.h"
#include "bll_adapter.h"
#include "bll_motor.h"
#include "modbus_master.h"
#include "config.h"
#include "delay.h"
#include "usart.h"
#include "gpio.h"
#include "timer.h"

static CommonStateFlag_Type tocase_flag = CSF_Idel;//声明原来的状态
volatile u8 has_zero_flag = 0;
volatile u8 last_target = 0;
static vu8 swtich_flag;
volatile vu8 swtich_count = 0;
static vu8 fast_move_flag;
 vu8 slow_move_flag;
static vu8 counter_dir = 0;
static BitAction ELECTRIAL_LEVEL = Bit_SET;

//extern u8 Master_Receive_Buff[MODBUS_MASTER_BUFF_LEN];

//清除flag状态
void BLL_ToCase_ClearFlag(void)
{
	tocase_flag = CSF_Idel;
}

static void Clear_Manual_Flags(void)
{
	swtich_flag = 0;
	fast_move_flag = 0;
	slow_move_flag = 0;
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

static const s32 position_table[] = {0, 205-25, 1610-50, 2850-120};	//这个是未到值



//static const s32 position_table[] = {0, 1600, 16250, 29000};	//这个是精控电机
//static const s32 position_table[] = {0, 220, 1685, 2960};	//这个是正确位置

#define CARD_BUG_OFFSET			(0)	//负数往零位方向转，正数往格口方向转
#define FINAL_OFFSET(i)			(final_offsets[i])	//最终偏移（只对3号位起效）
#define DEFAULT_FINAL_OFFSET	(10)	//

static s32 final_offsets[19] = {DEFAULT_FINAL_OFFSET,-10,10,00,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};



CommonStateFlag_Type BLL_ToCase_Execute(ParamShadow_Type params, u8 *err)
{
	u32 start;
	if(tocase_flag == CSF_Idel)
	{
//		s32 ext_move = *(s32*)&params.Param2;
//		s32 distance = ext_move?ext_move:pos_table[params.Param1];
		
		//Step0：初始化各种标志位等
		tocase_flag = CSF_Working;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);	
		Clear_Manual_Flags();
	
		//Step1：进行回零
		if(has_zero_flag == 0)
		{
			BLL_Moter_AD_BackZero(10,30);
//			BLL_Moter_AD_BackZero(RUN_REG.ZeroAcc,RUN_REG.ZeroSpeed);
			WAIT_MOTOR_STOP(100,200,die);	//100ms查一次，查200次不行就超时
			delay_ms(100);
			Clear_Position();
			has_zero_flag = 1;
			last_target = 0;
			swtich_count = 0;
		}		
		
		delay_ms(1000);
		
		//Step2：快速相对位移模式
		if(params.Param1 - last_target > 0)
		{
//			counter_dir = 0;
//			NVIC_Config(1,0);
//			ELECTRIAL_LEVEL = Bit_SET;
		}
		else if(params.Param1 - last_target < 0)
		{
//			counter_dir = 1;
//			NVIC_Config(1,1);
//			ELECTRIAL_LEVEL = Bit_RESET;
		}
		else
		{
			goto die;
		}
		
		last_target = params.Param1;
		fast_move_flag = 1;
		u8 target = params.Param1;
		u8 card_bug = 0;
		if(target == 4)
		{
			card_bug = 1;
			target = 2;
		}
		
		BLL_Motor_AD_AbsoluteMove(position_table[target],5,5,100);//原5，5，200
//		BLL_Motor_AD_RelativeMove(distance,RUN_REG.MaxAcc,RUN_REG.MaxDec,RUN_REG.MaxSpeed);
		WAIT_MOTOR_STOP(100,200,die);	//100ms查一次，查200次不行就超时
		fast_move_flag = 0;
		
		//goto die;
		delay_ms(700);
		
		//Step3：判断位移模式结果，决定如何修正		
		if(Read_Switch(1) == Bit_SET)
		{
//		if((Read_Switch(1) == Bit_SET) && (swtich_count == params.Param1))
			goto card_bug_offset;
		}
		else
		{
//			if(swtich_count == (params.Param1-1))
//			{
//				slow_move_flag = 1;
//				counter_dir = 0;
//				BLL_Motor_AD_SpeedMode(5,5,20);
//			}
//			else if(swtich_count == params.Param1)
//			{
//				slow_move_flag = 1;
//				counter_dir = 1;
//				swtich_count++;
//				BLL_Motor_AD_SpeedMode(5,5,-20);
//			}
//			else
//			{
//				*err = Failure_Actuator;
//				has_zero_flag = 0;
//				goto die;
//			}
			swtich_flag = 0;
			slow_move_flag = 1;
			BLL_Motor_AD_RelativeMove(200,5,5,20);//原5，5，20
				
			start = ReadTick();
			while(1)
			{
				if(swtich_flag)
				{
					Stop();
					goto card_bug_offset;
				}
				if(TickSpan(start) > 1500)
				{
					break;
				}	
			}
		
			BLL_Motor_AD_RelativeMove(-400,5,5,20);//原5，5，20
				
			start = ReadTick();	//这句话要加上
			while(1)
			{
				if(swtich_flag)
				{
					Stop();
					goto card_bug_offset;
				}
				if(TickSpan(start) > 2000)
				{
					*err = Failure_Aim;
					has_zero_flag = 0;
					goto die;
				}
			}
		}
		
//		//Step4：死等光电触发
//		u32 start = ReadTick();
//		while(swtich_flag == 0)
//		{
//			if(TickSpan(start) > 10000)
//			{
//				*err = Failure_Timeout;
//				has_zero_flag = 0;
//				goto die;
//			}
//		}
//		//Step5：停止电机
//		Stop();
//		delay_ms(100);
//		if((Read_Switch(1) == Bit_SET) && (swtich_count == params.Param1))
//		{
//			goto die;
//		}
//		else
//		{
//			*err = Failure_Actuator;
//			has_zero_flag = 0;
//			goto die;
//		}

card_bug_offset:
		if(card_bug)
		{
			delay_ms(100);
			BLL_Motor_AD_RelativeMove(CARD_BUG_OFFSET*100,5,5,200);
			WAIT_MOTOR_STOP(100,2000,die);
		}
		
		if(1==1)
		{
			u8 M = params.Param4;
			delay_ms(800);
			BLL_Motor_AD_RelativeMove(FINAL_OFFSET(target),5,5,200);
			WAIT_MOTOR_STOP(100,2000,die);
		}

	}

//最终程序出口
die:	
	tocase_flag = CSF_Finished;
	Clear_Manual_Flags();
	return tocase_flag;
}

void BLL_ToCase_EXTIHandler(void)
{
	if(tocase_flag == CSF_Working) 
	{
		delay_us(100);
		if(Read_Switch(1) == ELECTRIAL_LEVEL)
		{
//			if(fast_move_flag || slow_move_flag)
//			{
//				if(counter_dir == 0)
//				{
//					swtich_count++;
//				}
//				else
//				{
//					swtich_count--;
//				}
				if(slow_move_flag)
				{
					swtich_flag = 1;
				}	
//			}	
		}	
	}
}
