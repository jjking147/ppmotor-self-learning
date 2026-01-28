#include "bll_tocase.h"
#include "bll_adapter.h"
#include "bll_motor.h"
#include "modbus_master.h"
#include "config.h"
#include "delay.h"
#include "usart.h"
#include "gpio.h"
#include "timer.h"
#include "exti.h"
#include "stdlib.h"


static CommonStateFlag_Type tocase_flag = CSF_Idel;//声明原来的状态
volatile u8 has_zero_flag = 0;
volatile u8 last_target = 0;
static vu8 swtich_flag;
volatile vu8 swtich_count = 0;
static vu8 fast_move_flag;
static vu8 slow_move_flag;
static vu8 counter_dir = 0;
static int speed_select;
static const BitAction ELECTRIAL_LEVEL = Bit_SET;

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

#define WAIT_MOTOR_STOP(span,n,label)	{delay_ms(400); \
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
	}while((_motor_sate & 0x03) != 0);; \
}

void mydelay(u16 ms)
{
	u16 s = ms / 1000;
	ms = ms % 1000;
	while(s--)
		delay_ms(1000);
	delay_ms(ms);
}

//快速运动模式速度表，0<Abs(终点-起点)<18，所以第0个元素不起作用
const int fastmove_speed[19]={40,40,40,40,40,40,40,40,80,80,80,80,80,80,80,80,80,80,80}; //8 130 to 100，9 130 to 110
//const int fastmove_speed[19]={0,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10}; 
	
#define FIX_DELAY			(2500-1000)	//快速修正完毕后延迟多久开始修正，单位：ms. 1000 to 2500
	
#define FIX_SPEED			(2)		//慢速修正速度 1 to 2

#define FINAL_DELAY			(800)	//慢速修正结束后延迟多久开始最终偏移//1800——800
#define FINAL_OFFSET_FIXED		(0)
#define FINAL_SPEED			(10)		//最终固定偏移的运动速度

#define FIX_NORM_MAX_TIME		(5000)	//普通修正最大时间
#define FIX_SELF_MAX_TIEM		(3000)	//自修正最大时间
#define FIX_SELF_FIX_DIR		(1)	//-1代表优先反向，1代表优先正向，不要输入1和-1以外的数值

#if FINAL_OFFSET_FIXED	
#define FINAL_OFFSET(i)		(0)
#else
#define FINAL_OFFSET(i)		(final_offsets[i])	//慢速修正完毕后的最终固定偏移，正数为远离0点方向. 20 to 17
#endif
	
#define ROTATE_K		(820)
#define ROTATE_B		(850)
	
static s32 final_offsets[19] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //{0,10,12,15,20,20,20,25,25,20,15,15,18,18,10,10,10,10,0}

	

CommonStateFlag_Type BLL_ToCase_Execute(ParamShadow_Type params, u8 *err)
{
	u32 start;
	if(tocase_flag == CSF_Idel)
	{
		//不同格口的速度以数组形式赋予

		
		//Step0：初始化各种标志位等
		tocase_flag = CSF_Working;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);	
		Clear_Manual_Flags();
		
		//Step1：进行回零
		if(has_zero_flag == 0)
		{
			BLL_Moter_AD_BackZero(10,80);
			WAIT_MOTOR_STOP(200,3000,die);	//100ms查一次，查200次不行就超时
			mydelay(3000);
			Clear_Position();
			has_zero_flag = 1;
			last_target = 0;
//			swtich_count = -1;
		}
		
//		
//		BLL_Motor_AD_AbsoluteMove(100,1,1,10);
//		goto die;
		
		u8 M = params.Param1;
		s8 fix_dir = 1;
		u32 fix_max_time = FIX_NORM_MAX_TIME;
		
		//Step2：快速相对位移模式
		if(params.Param1 - last_target > 0)
		{
//			counter_dir = 0;
//			NVIC_Config(1,1);
//			ELECTRIAL_LEVEL = Bit_RESET;
		}
		else if(params.Param1 - last_target < 0)
		{
//			counter_dir = 1;
//			NVIC_Config(1,0);
//			ELECTRIAL_LEVEL = Bit_SET;
		}
		else
		{
			//fix_dir = (FINAL_OFFSET(M)>0?1:-1);
			//fix_max_time = FIX_SELF_MAX_TIEM;
			//goto FIX_GO;
		}

		speed_select = params.Param1 - last_target;
		if(has_zero_flag == 0){
			speed_select = params.Param1;
		}else{
			speed_select = abs(speed_select);
		}
		last_target = params.Param1;
		vu32 distance = ROTATE_K * (params.Param1 - 1) + ROTATE_B;
		fast_move_flag = 1;
		
//		My_EXTI_Cmd(1,DISABLE);
		BLL_Motor_AD_AbsoluteMove(distance,1,1,fastmove_speed[speed_select]);
//		delay_ms(400);
//		My_EXTI_Cmd(1,ENABLE);
		WAIT_MOTOR_STOP(200,30000,die);	//100ms查一次，查200次不行就超时
		
		
		
		fast_move_flag = 0;
		
	  //goto die;
		
		
		mydelay(FIX_DELAY);
FIX_GO:
		//Step3：判断位移模式结果，决定如何修正
		if(Read_Switch(1) == Bit_RESET)
		{
//		if((Read_Switch(1) == Bit_RESET) && (swtich_count == params.Param1 * 2))
			mydelay(FINAL_DELAY);
			BLL_Motor_AD_RelativeMove(FINAL_OFFSET(M),10,10,FINAL_SPEED);
			WAIT_MOTOR_STOP(100,30000,die);	
			goto die;
		}
		else
		{
//			if(swtich_count == (params.Param1 * 2 - 2))
//			{
//				slow_move_flag = 1;
//				counter_dir = 0;
//				BLL_Motor_AD_SpeedMode(1,1,3);
//			}
//			else if(swtich_count == (params.Param1 * 2 - 1))
//			{
//				slow_move_flag = 1;
//				counter_dir = 1;
//				swtich_count++;
//				BLL_Motor_AD_SpeedMode(1,1,-3);
//			}
//			else
//			{
//				*err = Failure_Actuator;
//				has_zero_flag = 0;
//				goto die;
//			}
			swtich_flag = 0;
			slow_move_flag = 1;
			BLL_Motor_AD_SpeedMode(1,1,FIX_SPEED * fix_dir);
				
			start = ReadTick();
			while(1)
			{
				if(swtich_flag)
				{
					Stop();
					mydelay(FINAL_DELAY);
					BLL_Motor_AD_RelativeMove(FINAL_OFFSET(M),10,10,FINAL_SPEED);
					WAIT_MOTOR_STOP(100,30000,die);	
					goto die;
				}
				if(TickSpan(start) > fix_max_time)
				{
					Stop();
					break;
				}	
			}
		
			BLL_Motor_AD_SpeedMode(1,1,-(FIX_SPEED * fix_dir));
				
			start = ReadTick();	//这句话要加上
			while(1)
			{
				if(swtich_flag)
				{
					Stop();
					mydelay(FINAL_DELAY);
					BLL_Motor_AD_RelativeMove(FINAL_OFFSET(M),10,10,FINAL_SPEED);
					WAIT_MOTOR_STOP(100,30000,die);	
					goto die;
				}
				if(TickSpan(start) > (fix_max_time*2))
				{
					Stop();
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
//		delay_ms(500);
//		if((Read_Switch(1) == Bit_RESET) && (swtich_count == params.Param1  * 2))
//		{
//			goto die;
//		}
//		else
//		{
////			*err = Failure_Actuator;
//			has_zero_flag = 0;
//			goto die;
//		}
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
		
		if(Read_Switch(1) == Bit_RESET)
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
