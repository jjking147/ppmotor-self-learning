#include "bll_tocase.h"
#include "bll_adapter.h"
#include "bll_motor.h"
#include "modbus_master.h"
#include "config.h"
#include "delay.h"
#include "usart.h"
#include "gpio.h"
#include "timer.h"
#include "bll_backzero.h"
#include "jexception.h"
#include "exti.h"

static CommonStateFlag_Type tocase_flag = CSF_Idel;//声明原来的状态
volatile u8 running_flag = 0;
volatile u8 has_zero_flag = 0;
volatile u8 last_target = 0;
extern volatile u8 wait_zero_flag;
extern volatile u8 zero_trigger_flag;
extern volatile u32 zero_trigger_time;
extern volatile s32 zero_turn_num;
extern volatile s32 zero_encoder_num;
static vu8 swtich_flag;
volatile vu8 swtich4_count = 0;
volatile vu8 counter_dir = 0;
static vu8 target;
vu8 special_case;
static vu8 div10 = 0;
static vu8 div20 = 0;
static vu8 kind;
static vu8 fast_move_flag;
static vu8 slow_move_flag;
static vu8 counter_flag = 0;
static vu32 counter_time = 0;
extern volatile ModBusState Master_State;
static vu32 switch_trigger_time = 0;
static BitAction ELECTRIAL_LEVEL = Bit_SET;

//==================================================================
// TODO: 测试完毕后，留意各种delay，能缩短的要缩短

static const s32 Special_Positions[] = 
{
	0,
	-23650,//0xFF(255)、出证口
	-35550,//0xFE(254)、入证口
	-38900,//0xFD(253)、入卡口
	0   //0xFC(252)、批量口
};

#define SLOW_ACCE			(5000)
#define SLOW_DECE			(5000)
#define SLOW_SPEED		 	(2000)

#define FAST_ACCE			(80000)
#define FAST_DECE			(50000)
#define FAST_SPEED		 	(15000)

#define FINAL_OFFSET		(0)	//证最终偏移(正数向下，负数向上) 20 to 0 to 20 to 0 to -25
#define FINAL_OFFSET_CARD	(0)		//卡最终偏移(正数向下，负数向上) 28 to 35 to 55 to 70 to 20

#define MAX_SLOW_TIME		(500)

#define K_BOOK				(930)
#define G_BOOK				(440)
#define B_BOOK				(780)

#define K_CARD				(465)
#define G_CARD				(500)
#define B_CARD				(1165)

#define BUG_80_OFFSET		(400) //465 to 400

//当参数4发送4或者5时，触发以下的盘点专用偏移
#define CARD_BUG_OFFSET		(-525)	//正数向上，负数向下
#define PP_BUG_OFFSET		(-190)	//正数向上，负数向下

#define MOVE_DELAY			(50)	//目前只做了证格口运动时的延时

//当参数4发送10或者11时，触发以下的插入后移动
#define PP_DOWN				(85)	//证插入后下移距离(正数向下，负数向上) 10 to 25 to 45 to 25 to 60 to 85
#define CARD_DOWN			(70)	//卡插入后下移距离(正数向下，负数向上) 22 to 5 to 20 to 40 to 25 to 70

//==================================================================

//清除flag状态
void BLL_ToCase_ClearFlag(void)
{
	tocase_flag = CSF_Idel;
}

static void Clear_Manual_Flags(void)
{
	special_case = 0;
	swtich_flag = 0;
	fast_move_flag = 0;
	slow_move_flag = 0;
}

void MyDelay(u16 ms)
{
	u32 now = ReadTick();
	while(TickSpan(now) < ms);
}

//vu16 _motor_sate = 0;
//vu16 states[100],xxx = 0;
#define WAIT_MOTOR_STOP(span,n,label)	{ \
	vu16 _motor_sate = 0,_retry = 0; \
	do \
	{ \
		if(_retry++ >= n) \
		{ \
			*err = Failure_Timeout; \
			has_zero_flag = 0; \
			goto label; \
		} \
		if(Check_LimitTriggered()) \
		{ \
			Brake(); \
			*err = (Failure_Limit); \
			goto label; \
		} \
		MyDelay(300); \
		_motor_sate = Check_Status();\
	}while((_motor_sate & 0x01) != 0x01); \
}

static void WaitMotorStop(u16 span,u16 n)
{
	u16 _motor_sate = 0,_retry = 0; 
	do 
	{ 
		if(_retry++ >= n) 
		{ 
			throw(Failure_Timeout);
		} 
		if(Check_LimitTriggered())
		{
			Brake();
			throw(Failure_Limit);
		}
		delay_ms(span); 
		_motor_sate = Check_Status(); 
	}while((_motor_sate & 0x01) != 0x01); 

}

__forceinline u8 Check_Switch( u8 kind,  u8 special_case)
{
	if(special_case == 0)
	{
//		return (XIN(2) == SET && (kind ? (XIN(3) == SET) : (XIN(1) == SET)));
		if(kind == 0)
		{
			return (XIN(1) == SET && (XIN(2) == SET));
		}
		else
		{
			return (XIN(2) == SET && (XIN(1) == SET));
		}
	}
	else
	{
		return (XIN(4) == SET);
	}
}

static u8 CaseSlowMove(u32 acc,u32 dece,u32 speed,s32 maxdistance,u16 maxtime,u8 kind)
{
	slow_move_flag = 1;
	swtich_flag = 0;
	BLL_Motor_AD_RelativeMove(maxdistance,acc,dece,speed);
	
	u32 start = ReadTick();
	while(1)
	{
		if(swtich_flag)
		{
			if(Check_Switch(kind, special_case))
			{
				if(TickSpan(switch_trigger_time) >= 2)
				{
					Brake();
					slow_move_flag = 0;
					delay_ms(100);
					if(special_case)
					{
						if(Read_Switch(4) == SET)
						{
							if(counter_dir == 0)
							{
								swtich4_count++;
							}
							else
							{
								swtich4_count--;
							}
						}
						else
						{
							has_zero_flag = 0;
						}
					}
					return 1;
				}
			}
			else
			{
				swtich_flag = 0;
				slow_move_flag = 1;
			}
		}
		if(TickSpan(start) > maxtime)
		{
			Brake();
			slow_move_flag = 0;
			return 0;
		}
	}
}

CommonStateFlag_Type BLL_ToCase_Execute(ParamShadow_Type params, u8 *err)
{	
	if(tocase_flag == CSF_Idel)
	{
		//Step0：初始化各种标志位等
		tocase_flag = CSF_Working;
		running_flag = 1;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);	
		Clear_Manual_Flags();
		
		//Step1：进行回零
		if(has_zero_flag == 0)
		{
			BackZero();
			MyDelay(1000);
		}
		
		if(params.Param4 == 10 || params.Param4 == 11)
		{
			s32 move = ((params.Param4 == 10) ? PP_DOWN : CARD_DOWN);
			
			try
			{
				BLL_Motor_AD_RelativeMove(move,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//最终修正
				WaitMotorStop(200,2000);
			}
			catch
			{
				*err = GetLastError();
				goto die;
			}
			
			goto die;
		}
		
		if((params.Param1>>7) == 0 && (params.Param4 == 0 || params.Param4==5))	//证格口
		{
			target = params.Param1&0x00ff;
			div10 = (target - 1) / 10;
			
			s32 pp_bug = 0;
			if(params.Param4 == 5)
			{
				pp_bug = PP_BUG_OFFSET;
			}
			
			kind = 0;
			//Step2：快速相对位移模式
			fast_move_flag = 1;
			s32 sub1 = target * K_BOOK + div10 * G_BOOK + B_BOOK;
			u8 move_rst =  BLL_Motor_AD_AbsoluteMove(-sub1,FAST_ACCE,FAST_DECE,FAST_SPEED);
			//MyDelay(500);
			WAIT_MOTOR_STOP(100,3000,die);	//100ms查一次，查300次不行就超时
			
			fast_move_flag = 0;

			if(move_rst)
			{
				*err = 8;
				goto die;
			}
			
			MyDelay(MOVE_DELAY);
			
			//goto die; //暂时不用修正
			
			//Step3：判断位移模式结果，决定如何修正
			if((Read_Switch(1) == SET) && (Read_Switch(2) == SET))
			{
				delay_ms(2);
				if((Read_Switch(1) == SET) && (Read_Switch(2) == SET))
				{
					MyDelay(MOVE_DELAY);
					BLL_Motor_AD_RelativeMove(FINAL_OFFSET-pp_bug,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);
					WAIT_MOTOR_STOP(200,3000,die);	//100ms查一次，查300次不行就超时
					goto die;
				}
			}
			else
			{			
				try
				{
					if(CaseSlowMove(SLOW_ACCE,SLOW_ACCE*10,SLOW_SPEED,-400,MAX_SLOW_TIME,0)) //如果上修成功
					{
						MyDelay(MOVE_DELAY);
						BLL_Motor_AD_RelativeMove(FINAL_OFFSET-pp_bug,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//最终修正
						WaitMotorStop(200,2000);
						goto die;
					}
					else if(CaseSlowMove(SLOW_ACCE,SLOW_ACCE*10,SLOW_SPEED,800,MAX_SLOW_TIME,0)) //如果下修成功
					{
						MyDelay(MOVE_DELAY);
						BLL_Motor_AD_RelativeMove(FINAL_OFFSET-pp_bug,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//最终修正
						WaitMotorStop(200,2000);
						goto die;
					}
					else
					{
						throw(Failure_Aim);
					}
				}
				catch
				{
					*err = GetLastError();
					goto die;
				}	
			}
		}
		if((params.Param1>>7) == 0 && (params.Param4 == 1 || params.Param4==4))	//卡格口
		{
			target = params.Param1&0x00ff;
			div10 = (target - 1) / 20;
			
			//80号格口bug
			u8 bug80 = 0;
			if(target == 80)
			{
				bug80 = 1;
				target = 79;
			}
			
			kind = 1;
			//Step2：快速相对位移模式
			fast_move_flag = 1;
			s32 sub2 = target * K_CARD + div10 * G_CARD + B_CARD;
			u8 move_rst = BLL_Motor_AD_AbsoluteMove(-sub2,FAST_ACCE,FAST_DECE,FAST_SPEED);
			WAIT_MOTOR_STOP(200,3000,die);	//100ms查一次，查300次不行就超时
			fast_move_flag = 0;

			if(move_rst)
			{
				*err = 8;
				goto die;
			}
			
			MyDelay(500);
			
//			goto die;
			
			//Step3：判断位移模式结果，决定如何修正
//			if((Read_Switch(2) == SET) && (Read_Switch(3) == SET))
//			{
//				delay_ms(2);
//				if((Read_Switch(2) == SET) && (Read_Switch(3) == SET))
//				{
//					goto die;
//				}
//			}
      if((Read_Switch(2) == SET) && (Read_Switch(1) == SET))
			{
				delay_ms(2);
				if((Read_Switch(2) == SET) && (Read_Switch(1) == SET))
				{
					if(bug80)	//80bug
					{
						BLL_Motor_AD_RelativeMove(-BUG_80_OFFSET,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//80号卡格口修正
						WaitMotorStop(200,2000);
					}
					if(params.Param4 == 4)
					{
						BLL_Motor_AD_RelativeMove(-CARD_BUG_OFFSET,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	
						WaitMotorStop(200,2000);
					}
					MyDelay(MOVE_DELAY);
					BLL_Motor_AD_RelativeMove(FINAL_OFFSET_CARD,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//最终修正
					WaitMotorStop(200,2000);
					goto die;
				}
			}
			else
			{
				try
				{
					if(CaseSlowMove(SLOW_ACCE,SLOW_ACCE*10,SLOW_SPEED,-200,MAX_SLOW_TIME,0)) //如果上修成功
					{
						//卡就不进行最终修正了
						if(bug80)	//80bug
						{
							BLL_Motor_AD_RelativeMove(-BUG_80_OFFSET,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//80号卡格口修正
							WaitMotorStop(200,2000);
						}
						if(params.Param4 == 4)
						{
							BLL_Motor_AD_RelativeMove(-CARD_BUG_OFFSET,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	
							WaitMotorStop(200,2000);
						}
						MyDelay(MOVE_DELAY);
						BLL_Motor_AD_RelativeMove(FINAL_OFFSET_CARD,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//最终修正
						WaitMotorStop(200,2000);
						goto die;
					}
					else if(CaseSlowMove(SLOW_ACCE,SLOW_ACCE*10,SLOW_SPEED,400,MAX_SLOW_TIME,0)) //如果下修成功
					{
						//卡就不进行最终修正了
						if(bug80)	//80bug
						{
							BLL_Motor_AD_RelativeMove(-BUG_80_OFFSET,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//80号卡格口修正
							WaitMotorStop(200,2000);
						}
						if(params.Param4 == 4)
						{
							BLL_Motor_AD_RelativeMove(-CARD_BUG_OFFSET,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	
							WaitMotorStop(200,2000);
						}
						MyDelay(MOVE_DELAY);
						BLL_Motor_AD_RelativeMove(FINAL_OFFSET_CARD,SLOW_ACCE,SLOW_DECE,SLOW_SPEED);	//最终修正
						WaitMotorStop(200,2000);
						goto die;
					}
					else
					{
						throw(Failure_Aim);
					}
				}
				catch
				{
					*err = GetLastError();
					goto die;
				}
			}
		}
		if((params.Param1 & 0xf0) == 0xf0)	
		{
			special_case = 0-(u8)params.Param1;
			
			//Step2：快速相对位移模式
//			if(special_case - last_target > 0)
//			{
//				counter_dir = 0;
//				BLL_Motor_AD_RelativeMove(-500,FAST_ACCE,FAST_DECE,FAST_SPEED);
//				WAIT_MOTOR_STOP(200,3000,die);	//100ms查一次，查300次不行就超时
//			}
//			else if(special_case - last_target < 0)
//			{
//				counter_dir = 1;
//				swtich4_count--;
//				BLL_Motor_AD_RelativeMove(500,FAST_ACCE,FAST_DECE,FAST_SPEED);
//				WAIT_MOTOR_STOP(200,3000,die);	//100ms查一次，查300次不行就超时
//			}
//			else
//			{
//				goto die;
//			}
			
			last_target = special_case;
			
			//Step2：快速相对位移模式
			fast_move_flag = 1;
			counter_flag = 0;
			u8 move_rst = BLL_Motor_AD_AbsoluteMove(Special_Positions[special_case],FAST_ACCE,FAST_DECE,FAST_SPEED);
			{
				vu16 _motor_sate = 0,_retry = 0; 
				u32 starttime = 0;
				do 
				{ 
					if(counter_flag == 1 && TickSpan(counter_time)>=15)
					{
						counter_flag = 0;
						if(Read_Switch(4) == ELECTRIAL_LEVEL)
						{
							if(counter_dir == 0)
							{
								swtich4_count++;
							}
							else
							{
								swtich4_count--;
							}
						}
					}
					if(TickSpan(starttime) >= 300)
					{
						starttime = ReadTick();
						_motor_sate = Check_Status();
						if(_retry++ >= 4000) 
						{ 
							*err = Failure_Timeout; 
							has_zero_flag = 0;
							goto die; 
						} 
					}
				}while((_motor_sate & 0x01) != 0x01);
			}
			fast_move_flag = 0;

			if(move_rst)
			{
				*err = 8;
				goto die;
			}
			
			MyDelay(100);
			
//			goto die;
			
			//Step3：判断位移模式结果，决定如何修正
			if((Read_Switch(4) == SET))
//			if((Read_Switch(4) == SET) && (swtich4_count == special_case))
			{
				delay_ms(2);
				if(Read_Switch(4) == SET)
				{
					goto die;
				}
			}
			else
			{
				try
				{
					if(CaseSlowMove(SLOW_ACCE,SLOW_ACCE*10,SLOW_SPEED,-2000,MAX_SLOW_TIME,0)) //如果上修成功
					{
						goto die;
					}
					else if(CaseSlowMove(SLOW_ACCE,SLOW_ACCE*10,SLOW_SPEED,4000,MAX_SLOW_TIME*2,0)) //如果下修成功
					{
						goto die;
					}
					else
					{
						throw(Failure_Aim);
					}
				}
				catch
				{
					*err = GetLastError();
					goto die;
				}
//				if(swtich4_count == (special_case - 1))
//				{
//					slow_move_flag = 1;
//					counter_dir = 0;
//					if(CaseSlowMove(SLOW_ACCE,SLOW_ACCE*10,SLOW_SPEED,-0x0fffffff,MAX_SLOW_TIME,1))	//如果上修成功
//					{
//						goto die;
//					}
//				}
//				else if(swtich4_count == special_case)
//				{
//					slow_move_flag = 1;
//					counter_dir = 1;
//					swtich4_count++;
//					if(CaseSlowMove(SLOW_ACCE,SLOW_ACCE*10,SLOW_SPEED,0x0fffffff,MAX_SLOW_TIME,1)) //如果下修成功
//					{
//						goto die;
//					}
//				}
//				else
//				{
//					*err = Failure_Actuator;
//					has_zero_flag = 0;
//					goto die;
//				}
			}
		}
	}

//最终程序出口
die:	
	tocase_flag = CSF_Finished;
	running_flag = 0;
	Clear_Manual_Flags();
	return tocase_flag;
}

#define EXTI_DEBOUNCE_US	10

void SWTICH1_INT_HANDLER(void)
{
	if(tocase_flag == CSF_Working) 
	{
		if(slow_move_flag)
		{
			if(special_case == 0)
			{
				delay_us(EXTI_DEBOUNCE_US);
				if(kind == 0)
				{
					if((Read_Switch(1) == SET) && (Read_Switch(2) == SET))
					{
						swtich_flag = 1;
						switch_trigger_time = ReadTick();
						slow_move_flag = 0;
					}
				}
			}
		}
	}
}

void SWTICH2_INT_HANDLER(void)
{
	if(tocase_flag == CSF_Working) 
	{
		if(slow_move_flag)
		{
			if(special_case == 0)
			{
				delay_us(EXTI_DEBOUNCE_US);
				if(kind == 0)
				{
					if((Read_Switch(2) == SET) && (Read_Switch(1) == SET))
					{
						swtich_flag = 1;
						switch_trigger_time = ReadTick();
						slow_move_flag = 0;
					}	
				}
				if(kind == 1)
				{
					if((Read_Switch(2) == SET) && (Read_Switch(1) == SET))
					{
						swtich_flag = 1;
						switch_trigger_time = ReadTick();
						slow_move_flag = 0;
					}
				}
			}
		}
	}
}

void SWTICH3_INT_HANDLER(void)
{
	if(tocase_flag == CSF_Working) 
	{
		if(slow_move_flag)
		{
			if(special_case == 0)
			{
				delay_us(EXTI_DEBOUNCE_US);
				if(kind == 1)
				{
					if((Read_Switch(3) == SET) && (Read_Switch(2) == SET))
					{
						swtich_flag = 1;
						switch_trigger_time = ReadTick();
						slow_move_flag = 0;
					}		
				}
			}
		}			
	}
}

void SWTICH4_INT_HANDLER(void)
{
	if(tocase_flag == CSF_Working) 
	{
		if(special_case != 0)
		{
			delay_us(EXTI_DEBOUNCE_US);
			if(Read_Switch(4) == ELECTRIAL_LEVEL)
			{
				if(fast_move_flag)
				{
					if(counter_flag == 0)
					{
						counter_flag = 1;
						counter_time = ReadTick();
					}
				}
				if(slow_move_flag)
				{
					swtich_flag = 1;
					switch_trigger_time = ReadTick();
					slow_move_flag = 0;
				}		
			}
		}		
	}
}
