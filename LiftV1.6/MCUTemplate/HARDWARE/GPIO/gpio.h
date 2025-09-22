#ifndef __GPIO_H
#define __GPIO_H	 
#include "sys.h" 

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

#define X1_PORT C
#define X1_PIN 2
#define X2_PORT C
#define X2_PIN 3
#define X3_PORT C
#define X3_PIN 4
#define X4_PORT C
#define X4_PIN 5
#define X5_PORT C
#define X5_PIN 7
#define X6_PORT C
#define X6_PIN 8
#define X7_PORT C
#define X7_PIN 9
#define X8_PORT D
#define X8_PIN 6
#define X9_PORT D
#define X9_PIN 0
#define X10_PORT D
#define X10_PIN 1

void GPIO_Config(void);	//IO≥ı ºªØ
u8 Read_Switch(u8);

#endif
