#include "bll_tocase.h"
#include "delay.h"
#include "usart.h"
#include "modbus_master.h"
#include "motor.h"
#include "gpio.h"

static CommonStateFlag_Type flag = CSF_Idel;//声明原来的状态
extern u8 swtich_flag;
extern u8 swtich_count;
extern u8 zero_flag;
extern u8 photoelectric_sign;
extern u8 Master_Receive_Buff[MODBUS_BUFF_LEN];

//清除flag状态
void BLL_ToCase_ClearFlag(void)
{
	flag = CSF_Idel;
}

CommonStateFlag_Type BLL_ToCase_Execute(ParamShadow_Type params, u8 *err)
{

	if(flag == CSF_Idel)
	{
		flag = CSF_Working;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);
		
		if(zero_flag == 0)
		{
        Master_Set_Idle();
        Init_Parameter();
        Master_Set_Idle();
        Motor_Enable();
        Master_Set_Idle();
        Set_Back_Zero_Mode();
        Master_Set_Idle();
        Set_Neg_Limit_Mode();
        Master_Set_Idle();
        Set_Back_Zero_Acce(params.Param4);
        Master_Set_Idle();
        Set_Back_Zero_Speed(params.Param5);
        Master_Set_Idle();
        Set_Abso_Position_Mode();
        Master_Set_Idle();
        Abso_Position_Start_Sample();
        Master_Set_Idle();
		}
		
		Master_Set_Idle();
		Driver_Power_On();
		Master_Set_Idle();
		Init_Parameter();
		Master_Set_Idle();
		Motor_Enable();//电机使能
		Master_Set_Idle();
		Set_Position_Mode();//设置位置模式
		Master_Set_Idle();
		Position_Mode_Set_Speed(params.Param1);//设置目标速度（正值）
		Master_Set_Idle();
		Set_Acce(1);//设置加速度
		Master_Set_Idle();
		Set_Dece(1);//设置减速度
		Master_Set_Idle();
		Set_Distence(1000);//设置运行位置
		Master_Set_Idle();
		Set_Abso_Position();//设置绝对位置模式
		Master_Set_Idle();
		Abso_Position_Start_Sample();//触发运动
		Master_Set_Idle();
		Check_Status();//检查电机状态
		Master_Set_Idle();
		if((Master_Receive_Buff[0] & 0X0001) == 0X0001)
		{
		    Fast_Dece_Stop_Motor_Enable();
		    Master_Set_Idle();
			*err = 1; //电机产生故障
		    return CSF_Finished; 
		}
		
		do
		{
		Master_Set_Idle();
		Check_Status();//检查电机状态
		Master_Set_Idle();
		}
		while((Master_Receive_Buff[0] & 0X0400) != 0X0400);
		
		
		   if(photoelectric_sign != 1)
		   {
		      if(swtich_count == 3)
		      {
		       Master_Set_Idle();
		       Driver_Power_On();
	           Master_Set_Idle();
		       Init_Parameter();
		       Master_Set_Idle();
		       Motor_Enable();//电机使能 
			   Master_Set_Idle();
               Set_Speed_Mode();
               Master_Set_Idle();
	       	   Set_Acce(1);//设置加速度
		       Master_Set_Idle();
		       Set_Dece(1);//设置减速度
		       Master_Set_Idle();
               Position_Mode_Set_Speed(params.Param2);//设置目标速度（该速度（负值）比较小）	
               Master_Set_Idle();				  
		      }
		      else if(swtich_count == 2)			  
			  {
		       Master_Set_Idle();
		       Driver_Power_On();
	           Master_Set_Idle();
		       Init_Parameter();
		       Master_Set_Idle();
		       Motor_Enable();//电机使能 
			   Master_Set_Idle();
               Set_Speed_Mode();
               Master_Set_Idle();
	       	   Set_Acce(1);//设置加速度
		       Master_Set_Idle();
		       Set_Dece(1);//设置减速度
		       Master_Set_Idle();
               Position_Mode_Set_Speed(params.Param3);//设置目标速度（该速度（正值）比较小） 
               Master_Set_Idle();				  
			  }
		      else 
			  {
			   Master_Set_Idle();
			   Fast_Dece_Stop_Motor_Enable();
		       Master_Set_Idle();
			   *err = 2; //计数器故障
		       return CSF_Finished; 
			  }
			  
			   if( photoelectric_sign == 1)
		      {
		       Master_Set_Idle();
               Fast_Dece_Stop_Motor_Enable();
		       Master_Set_Idle();
		       return CSF_Finished; 
		      }
		   }
		      if((swtich_flag & photoelectric_sign) == 1)
	          {
		       Master_Set_Idle();  
		       Stop();
		       swtich_flag = 0;
		       Master_Set_Idle();
		       flag = CSF_Finished;
	          }
      
	}
	return flag;
}