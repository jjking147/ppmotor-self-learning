#include "bll_adapter.h"
#include "bll_motor.h"
#include "math.h"

int __Unimplemented()
{
	return -1;
}

int FUN_MOTOR_STOP(void)
{
	return -1;
}

int FUN_MOTOR_ABORT(void)
{
	return -1;
}

u8 BLL_Moter_AD_BackZero(u16 acce, s32 speed)
{
  Set_Acce_Time(speed/(acce * 10));
	
	Set_Speed(speed * 100);
	
	Reverse_Run();
	
	return 0;
}

u8 BLL_Motor_AD_RelativeMove(s32 position, u16 acce, u16 dece, s32 speed)
{
  Set_Acce_Time(speed/(acce * 10));
	
	Set_Dece_Time(speed/(dece * 10));
	
	Set_Speed(speed * 100);
	
	Set_Relative_Position(position);

	return 0;
}

u8 BLL_Motor_AD_AbsoluteMove(s32 position, u16 acce, u16 dece, s32 speed)
{
  Set_Acce_Time(speed/(acce * 10));
	
	Set_Dece_Time(speed/(dece * 10));
	
	Set_Speed(speed * 100);
	
	Set_Absolute_Position(position);

	return 0;
}

u8 BLL_Motor_AD_SpeedMode(u16 acce, u16 dece, s32 speed)
{
  Set_Acce_Time(speed/(acce * 10));
	
	Set_Dece_Time(speed/(dece * 10));
	
	Set_Speed(abs(speed) * 100);
	
	if(speed > 0)
		Forward_Run();
	else
		Reverse_Run();

	return 0;
}
