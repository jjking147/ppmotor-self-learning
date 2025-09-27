#include "stm32f4xx.h"
#include "delay.h"
#include "usart.h"
#include "bll_main.h"
#include "modbus_slave.h"
#include "timer.h"
#include "jexception.h"

#include "gpio.h"

INIT_EXCEPTION_SYSTEM;

u8 MOTOR_ADDRESS = 0x03;

u8 IsJumpIAP = 0;
extern void BLL_Update(void);

int main(void)
{
	SystemInit();
	BLL_Init_All();
	delay_init(168);
//	
//	while(1)
//	{
//		vu8 sw = Read_Switch(4);
//		vu8 sm = Read_Switch(5);
//		delay_ms(100);
//	}
//	
	while (1) 
	{
		BLL_Execute_Mission();
		if(IsJumpIAP)
			BLL_Update();
	}
}
