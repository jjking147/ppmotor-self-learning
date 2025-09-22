#include "bll_eeprom.h" 
#include "modbus_save_data.h"
#include "ji2c.h"
#include "timer.h"
#include "delay.h"
#include "config.h"

static CommonStateFlag_Type flag = CSF_Idel;

static u8 wait_flag = 0;
static const u8 page_size = 16;

static void EEPROM_Callback(bool succ,u8* data)
{
	wait_flag = 0;
}

static u8 EEPROM_Write(u8 bank,u8 regaddr,u8* buffer,u16 len)
{
	u8 pkg = len / page_size;
	u8 mod = len % page_size;
	u8 romaddr = 0xA0 + (bank << 1);
	for(u8 i=0; i<pkg; i++)
	{
		wait_flag=1;
		JI2C_WriteReg8_Async(romaddr,regaddr + i*page_size,page_size,buffer+i*page_size,EEPROM_Callback);
		u32 start = ReadTick();
		while(wait_flag)
		{
			if(TickSpan(start) > EEPROM_TIMEOUT)
				return Failure_Timeout;
		}
		delay_ms(5);
	}

	if(mod != 0)
	{
		wait_flag=1;
		JI2C_WriteReg8_Async(romaddr,regaddr + pkg*page_size,mod,buffer+pkg*page_size,EEPROM_Callback);
		u32 start = ReadTick();
		while(wait_flag)
		{
			if(TickSpan(start) > EEPROM_TIMEOUT)
				return Failure_Timeout;
		}
		delay_ms(5);
	}
	
	
	return Failure_None;
}

static u8 EEPROM_Read(u8 bank,u8 regaddr,u8* buffer,u16 len)
{
	u8 romaddr = 0xA0 + (bank << 1);
	wait_flag = 1;
	JI2C_ReadReg8_Async(romaddr,regaddr,len,buffer,EEPROM_Callback);
	u32 start = ReadTick();
	while(wait_flag)
	{
		if(TickSpan(start) > EEPROM_TIMEOUT)
			return Failure_Timeout;
	}
	return Failure_None;
}

static FailureID_Type Save_Data(void)
{
	vu8 len = sizeof(REGION_TYPE(0));
	vu8 len2 = sizeof(REGION_TYPE(1));
	u8 err = EEPROM_Write(0,0,(u8 *)&SYSTEM_REG,len);
	if(err)
		return err;

	err = EEPROM_Write(1,0,(u8 *)&RUN_REG,len2);
	return err;
}

static FailureID_Type Save_Default(void)
{
	u8 err = EEPROM_Write(2,0,(u8 *)&SYSTEM_REG,sizeof(REGION_TYPE(0)));
	if(err)
		return err;
	
	err = EEPROM_Write(3,0,(u8 *)&RUN_REG,sizeof(REGION_TYPE(1)));
	return err;
}

static FailureID_Type Read_Default(void)
{
	u16 magic_num = 0;
	EEPROM_Read(2,0,(u8 *)&magic_num,2);
	if(magic_num != 0xa5a5)
	{
		return Failure_DataFault;
	}
	
	u8 err = EEPROM_Read(2,0,(u8 *)&SYSTEM_REG,sizeof(REGION_TYPE(0)));
	if(err)
		return err;
	
	err = EEPROM_Read(3,0,(u8 *)&RUN_REG,sizeof(REGION_TYPE(1)));
	
	SYSTEM_REG.DrvType = DRIVERTYPE;
	SYSTEM_REG.Version = VERSION;
	
	return err;
}

//0xA0 0xA2 0xA4 0xA6
void EEPROM_Init(void)
{
	JI2C_Init(EEPROM_SPEED);
	u16 magic_num = 0;
	EEPROM_Read(0,0,(u8 *)&magic_num,2);
	if(magic_num == 0xA5A5)
	{
		EEPROM_Read(0,0,(u8 *)&SYSTEM_REG,sizeof(REGION_TYPE(0)));
		EEPROM_Read(1,0,(u8 *)&RUN_REG,sizeof(REGION_TYPE(1)));
		SYSTEM_REG.DrvType = DRIVERTYPE;
		SYSTEM_REG.Version = VERSION;
	}
	
}

CommonStateFlag_Type BLL_EEPROM_Execute(ParamShadow_Type params ,u8 *err)
{
	if (flag == CSF_Idel)
	{
		flag = CSF_Working;
		if (params.Param1 == 1)
			*err = Save_Data();
		else if (params.Param1 == 2)
			*err = Read_Default();
		else if(params.Param1 == 3)
			*err = Save_Default();
		
		flag = CSF_Finished;
		return flag;
	}
	
}

void BLL_EEPROM_ClearFlag(void)
{
	flag = CSF_Idel;
}
