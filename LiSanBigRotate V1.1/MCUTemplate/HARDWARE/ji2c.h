#ifndef __JI2C_H__ 
#define __JI2C_H__ 
 
 
#include "sys.h" 
#include "stdint.h"
#include "stdbool.h"
 
#define JI2C_OWNADDR				(0x33)


typedef void (*JI2C_TRANSFINISH_CB)(bool, uint8_t*);

typedef enum JI2C_State_Type
{
	JI2C_State_Idle = 0,
	JI2C_State_Start,
	JI2C_State_Address,
	JI2C_State_Start_R,
	JI2C_State_Start_W,
	JI2C_State_Reading,
	JI2C_State_Writing,
	JI2C_State_Finished,
	JI2C_State_Error,

}JI2C_State_Type;

typedef enum JI2C_TType_Type
{
	JI2C_TType_Read,
	JI2C_TType_Write,

}JI2C_TType_Type;

typedef union JI2C_Addr_Union
{
	uint8_t Addr8;
	struct
	{
		uint8_t Addr7 : 7;	
		uint8_t RWFlag : 1;		
	};

}JI2C_Addr_Union;

typedef union UnionInt16
{
	u16 Data16;
	struct
	{
		u8 Lo8;
		u8 Hi8;
	};
}UnionInt16;

typedef struct JI2C_Message_Type
{
	uint16_t Len;
	uint16_t Index;
	JI2C_Addr_Union SlaveAddr;
	JI2C_TType_Type TransType;
	JI2C_State_Type Status;
	uint8_t Is16Bit;
	UnionInt16 RegAddr;
	uint8_t* Buffer;
	JI2C_TRANSFINISH_CB Callback;

}JI2C_Message_Type;

void JI2C_Init(uint32_t speed);
void JI2C_ReadData_Async(uint8_t slav_addr, uint16_t bytes2read, uint8_t* buffer, JI2C_TRANSFINISH_CB callback);
JI2C_State_Type JI2C_GetState(void);
void JI2C_WriteReg8_Async(uint8_t slav_addr,uint8_t regaddr,uint16_t bytes2write, uint8_t* buffer, JI2C_TRANSFINISH_CB callback);
void JI2C_ReadReg8_Async(uint8_t slav_addr,uint8_t regaddr,uint16_t bytes2read, uint8_t* buffer, JI2C_TRANSFINISH_CB callback);

void Print_Debug(u8 len);
 
#endif 
 
