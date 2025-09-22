#include "stm32f4xx.h"
#include "delay.h"
#include "usart.h"
#include "bll_main.h"
#include "modbus_slave.h"
#include "timer.h"


u8 JODELL_MOTOR_ADDRESS = 0x09;
//u8 MOTOR_ADDRESS = 0x02;

u8 IsJumpIAP = 0;
extern void BLL_Update(void);

int main(void)
{
	SystemInit();
	delay_init(168);
	BLL_Init_All();
	
	while (1) 
	{
		BLL_Execute_Mission();
		if(IsJumpIAP)
			BLL_Update();
	}
}
