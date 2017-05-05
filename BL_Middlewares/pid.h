#ifndef BL_PID_H
#define BL_PID_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

void bl_pid_ActionAfterPIDCtrl(uint32_t duty);
void bl_pid_FollowLineContrWithPIDCyclic(void);
uint16_t bl_pid_RCAngCal(void);
uint16_t bl_pid_GetRCContrVal(void);
void bl_pid_PIDControllerInit(void);
#endif