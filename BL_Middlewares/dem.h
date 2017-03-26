#ifndef BL_DEM_H
#define BL_DEM_H
#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 



void Error_Handler(void);

typedef enum{
	
	DEM_PASS,
	DEM_FAIL
	
}Errorsta_en;

#endif
