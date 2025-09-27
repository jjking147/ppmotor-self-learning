#ifndef __BLL_MOTOR_H
#define __BLL_MOTOR_H
#include "sys.h"

#define FAULT_REGISTER                     0x00a3
#define RUN_REGISTER                       0x00c8
#define ENABLE_REGISTER                    0x00d4
#define STATUS_REGISTER                    0x0006
#define ACCE_TIME_REGISTER                 0x0098
#define DECE_TIME_REGISTER                 0x0099
#define REAL_POSITION_REGISTER             0x0004
#define SPEED_REGISTER                     0x00d8
#define RELATIVE_POSITION_REGISTER         0x00ce
#define ABSOLUTE_POSITION_REGISTER         0x00d0
#define SET_POSITION_REGISTER              0x00d2

#define ENABLE_CMD                         0x0000
#define DISABLE_CMD                        0x0001
#define FORWARD_RUN_CMD                    0x0001
#define REVERSE_RUN_CMD                    0x0101
#define STOP_CMD                           0x0000
#define BRAKE_CMD                          0x0100

//以下为夹爪定义
#define JODELL_CONTROL_REGISTER              0x03E8  //低字节 控制寄存器
#define JODELL_POSITION_CONTROL_REGISTER     0x03E9  //高字节 位置设置寄存器 0x00 表示完全打开，0xFF 表示完全闭合，两指之间线性关系
                                                     //EPG50-060 电动夹爪的最大行程为 50mm
#define JODELL_SPEED_POWER_CONTROL_REGISTER  0x03EA  //低字节 速度设置寄存器 ,高字节 力设置寄存器  
                                                     //注意：0x00 代表可以稳定控制的最低速度，0xFF 表示可控的最高速度，他们之间呈线性变化。
#define JODELL_STATUS_REGISTER_0             0x07D0  //低字节 夹爪状态
#define JODELL_STATUS_REGISTER_1             0x07D1  //低字节 故障状态；高字节 位置状态
#define JODELL_STATUS_REGISTER_2             0x07D2  //低字节 速度状态；高字节 力状态(即时电流)
#define JODELL_STATUS_REGISTER_3             0x07D3  //低字节 母线电压；高字节 环境温度
#define JODELL_STATUS_REGISTER_4             0x07E3  //低字节 编码器状态
#define JODELL_STATUS_REGISTER_5             0x07E4  //低字节 制动器状态
#define JODELL_STATUS_REGISTER_6             0x07E5  //编码器值

#define D1           50  //编码器值
#define D2           65535-50  //编码器值
#define D3           20  //编码器值
#define D4           30 //编码器值
#define D5           40  //编码器值
#define D6           50  //编码器值
#define D7           60  //编码器值

void Motor_Enable(void);
void Motor_Disable(void);
void Check_Fault(void);
void Forward_Run(void);
void Reverse_Run(void);
void Stop(void);
void Brake(void);
void Set_Acce_Time(u16 Time);
void Set_Dece_Time(u16 Time);
void Set_Speed(s32 Speed);
void Set_Relative_Position(s32 Pluse);
void Set_Absolute_Position(s32 Position);
void Clear_Position(void);
u16 Check_Status(void);
s32 Get_Position(void);

//夹爪函数
void JODELL_STATUS(u16 rACT);
void READ_JODELL_STATUS_1(void);
void READ_JODELL_STATUS_2(void);
void READ_JODELL_STATUS_3(void);
uint8_t READ_JODELL_STATUS_NUMBER_1(void);
uint8_t READ_JODELL_STATUS_NUMBER_2(void);
void Check_JODELL_STATUS_OPEN_CLOSE(void);
void JODELL_STATUS_ACTIVETION(void);
void JODELL_OPEN_CLOSE_ACTIVETION(u8 position,u8 POWER,u8 SPEED);
#endif
