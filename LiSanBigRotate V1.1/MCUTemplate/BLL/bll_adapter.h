#ifndef __BLL_ADAPTER_H__ 
#define __BLL_ADAPTER_H__

#include "sys.h"

typedef enum CommonStateFlag_Type
{
	CSF_Idel,
	CSF_Working,
	CSF_Finished,
} CommonStateFlag_Type;

#define ADAPTER_PLACEHOLD_VAL			0

#define FUN_MOTOR_CLEAR_RX_FLAG()		__nop()
#define FUN_MOTOR_ENABLE()				__nop()
#define FUN_MOTOR_SET_PSPEED(...)		__nop()
#define FUN_MOTOR_SET_ZSPEED(...)		__nop()
#define FUN_MOTOR_SET_MODE(m)			__nop()
#define FUN_MOTOR_TRIGGER_RPOS()		__nop()
#define FUN_MOTOR_TRIGGER_SPEED(s)		__nop()
#define FUN_EEPROM_WRITE(...)			__nop()
#define FUN_EEPROM_READ(...)			__nop()

//#define ELECTRIAL_LEVEL             Bit_SET

int FUN_MOTOR_STOP(void);
int FUN_MOTOR_ABORT(void);

u8 BLL_Moter_AD_BackZero(u16 acce, s32 speed);
u8 BLL_Motor_AD_AbsoluteMove(s32 Position, u16 acce, u16 dece, s32 speed);
u8 BLL_Motor_AD_RelativeMove(s32 distance, u16 acce, u16 dece, s32 speed);
u8 BLL_Motor_AD_SpeedMode(u16 acce, u16 dece, s32 speed);
u8 BLL_Motor_AD_SpeedMode_REV(u16 acce, u16 dece, s32 speed);

#endif 
 
