#ifndef __MODBUS_MASTER_H
#define __MODBUS_MASTER_H
#include "sys.h"
#include "modbus_slave.h"

#define MODBUS_MASTER_BUFF_LEN		200
#define MODBUS_MASTER_LOSS_TIME		3000 //ms

typedef void (*MODBUS_RESPONSE_CALLBACK)(ModBusFailCode_Type,u8*,u16);

void Master_Set_Idle(void);
void Master_Set_Timeout(void);
void Modbus_Master_Receive(u8 data);
void ModBus_Master_Pool(void);
void Send_Error(u8 Error);
void Send_Func03_Data(u16 Reg, u16 Num, MODBUS_RESPONSE_CALLBACK fun);
ModBusFailCode_Type Send_Func06_Data(u16 Reg, u16 Cmd);
ModBusFailCode_Type Send_Func10_Data(u16 Reg, u32 Cmd);
u16 Calculate_CRC(const u8 *buffer, u16 length);

//以下为夹爪电机函数
ModBusFailCode_Type Send_Func10_Data_JODELL(u16 Reg, u16 Cmd);
ModBusFailCode_Type Send_Func04_Data_JODELL(u16 Reg, u16 Cmd);
ModBusFailCode_Type Send_Func10_Data_JODELL_3(u16 Reg, u16 Cmd1,u16 Cmd2,u16 Cmd3);
#endif
