
#ifndef BL_AUTORACER_H
#define BL_AUTORACER_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

typedef enum{
	
	RC_IDLE,
	FOLLOWLINE,
	RIGHTSTEERING,
	LEFTSTEERING,
	BLINDNESSRUN,
	OUTOFROAD,
  SYSTEMFAILURE_STOP,
	WAITTIME,
	
}RacerPatternSta_en;

void bl_app_AutoRacerInit(void);
void bl_app_AutoRacerCyclic(void);

#endif