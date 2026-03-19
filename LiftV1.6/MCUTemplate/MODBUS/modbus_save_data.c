#include "modbus_save_data.h"
#include "config.h"

#warning packed GNU style

REGION_TYPE(0) Reg_4x_0 = 
{ 
	.BaudRate = 1152,
	.Version = VERSION,
	.DrvType = DRIVERTYPE,
	.IsSkipCRC = 0,
};

REGION_TYPE(1) Reg_4x_1 = 
{
		
};

REGION_TYPE(2) Reg_4x_2 = 
{
	.WorkingProcess = 0,
	.LastMotorState = 0
};

REGION_TYPE(3) Reg_4x_3 = 
{
			
};

REGION_TYPE(4) Reg_4x_4 = 
{
		
};

REGION_TYPE(5) Reg_4x_5 = 
{
	
};

u16 Reg_4x_6[100];

void* Reg_Regions[] =
{
	&Reg_4x_0,
	&Reg_4x_1,
	&Reg_4x_2,
	&Reg_4x_3,
	&Reg_4x_4,
	&Reg_4x_5,
	&Reg_4x_6,
};

static u8 _RegionSize[] = 
{ 
	sizeof(REGION_TYPE(0)),
	sizeof(REGION_TYPE(1)),
	sizeof(REGION_TYPE(2)),
	sizeof(REGION_TYPE(3)),
	sizeof(REGION_TYPE(4)),
	sizeof(REGION_TYPE(5)),
	sizeof(Reg_4x_6),
};

u8 ValidateRegRange(u8 region, u8 subaddr)
{
	if (region >= sizeof(Reg_Regions))
		return 0;
	if (subaddr >= _RegionSize[region])
		return 0;
	
	return 1;
}
