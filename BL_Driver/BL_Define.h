#ifndef BL_DEFINE_H
#define BL_DEFINE_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

typedef enum{
	
	FALSE =0,
	TRUE = 1

}BOOL;

#define DEBUG_ESP
#define DIAGRESPONSE 1


#define  BL_BUFFSIZE   50
#define  ESPREADBUFF   50
#define  ALARMBUFF		 ESPREADBUFF	

#define NumofSampling 8
#define NumOfSensor1 8
#define NumOfSensor2 0
#define NumofSensor (uint8_t)(NumOfSensor1+NumOfSensor2)

#ifndef RETSTATE
#define RETSTATE

typedef enum{
	E_OK,
	E_NOT_OK,	
}retstate;
#endif 

#endif

