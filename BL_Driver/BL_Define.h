#ifndef BL_DEFINE_H
#define BL_DEFINE_H
#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 
 
#define TRUE (1!=0)
#define FALSE (0!=0)


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

