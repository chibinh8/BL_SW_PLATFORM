#ifndef BL_PID_H
#define BL_PID_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

#define PIDINFORADRESS_BASE 							((uint32_t)0x080E0020)

typedef struct{
	
float KP_fl;
float KI_fl;
float KD_fl;

}PIDInfor_st;


typedef enum{
IDLE,
TUNINGPID,
NORMALCONTROL,
WAITFROMIDLE,
	
}PIDWorkSta_en;

void bl_pid_ActionAfterPIDCtrl(uint32_t duty);
void bl_pid_FollowLineContrWithPIDCyclic(void);
uint16_t bl_pid_RCAngCal(void);
uint16_t bl_pid_GetRCContrVal(void);
void bl_pid_PIDControllerInit(void);
#endif