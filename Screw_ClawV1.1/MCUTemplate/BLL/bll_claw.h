#ifndef __BLL_CLAW_H__ 
#define __BLL_CLAW_H__ 

#include "sys.h"
#include "bll_adapter.h"
#include "config.h"
#include "bll_main.h"

void BLL_Claw_ClearFlag(void);
CommonStateFlag_Type BLL_Claw_Execute(ParamShadow_Type params, u8 *err);
 
 void Claw_Action(u8 onoff);

#endif 

 
