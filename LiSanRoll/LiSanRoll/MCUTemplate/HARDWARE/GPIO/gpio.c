#include "gpio.h"

//GPIO初始化以及配置函数
extern u8 switch_state;

void GPIO_Config(void)
{ 
  INITX(1);
	INITX(2);
	INITX(3);
	INITX(4);
	INITX(5);
	INITX(6);
	INITX(7);
	INITX(8);
	INITX(9);
	INITX(10);
}

u8 Read_Switch(u8 switch_number)
{
	u8 switch_state = 0;
	switch(switch_number)
	{
    case 1:
		switch_state=XIN(1);
		break;
		
		case 2:
		switch_state=XIN(2);
		break;
		
		case 3:
		switch_state=XIN(3);
		break;
		
		case 4:
		switch_state=XIN(4);
		break;
		
		case 5:
		switch_state=XIN(5);
		break;
		
		case 6:
		switch_state=XIN(6);
		break;
		
		case 7:
		switch_state=XIN(7);
		break;
		
		case 8:
		switch_state=XIN(9);
		break;
		
		case 9:
		switch_state=XIN(9);
		break;
		
		case 10:
		switch_state=XIN(10);
		break;
		
		default:
		break;
  }
  return switch_state;
}
