#ifndef __COMMON_H__
#define __COMMON_H__

#include "sys.h"
#include "config.h"

#define XINRAW(p,n) P##p##in(n)
#define XINRAW_EXPAND(p,n) XINRAW(p,n) 

#define INIT_PORT_IN_FLOAT(p,n)  {\
	GPIO_InitTypeDef GPIO_InitStructure;    \
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIO##p, ENABLE);\
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_##n;\
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;\
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;\
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;\
	GPIO_Init(GPIO##p, &GPIO_InitStructure);\
}

#define INIT_PORT_IN_FLOAT_EXPAND(p,n) INIT_PORT_IN_FLOAT(p,n)

#define GET_X_PORT(n)   X##n##_PORT
#define GET_X_PIN(n)    X##n##_PIN

#define XIN(n)  XINRAW_EXPAND(GET_X_PORT(n),GET_X_PIN(n))
#define INITX(n)    INIT_PORT_IN_FLOAT_EXPAND(GET_X_PORT(n),GET_X_PIN(n))

#endif
