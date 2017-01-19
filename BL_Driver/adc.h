
#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 
#define NumOfSensor1 7
#define NumOfSensor2 1

void BL_ADCInit(void);
void ReadSensor(volatile uint16_t* outsensorval, ADC_HandleTypeDef *hadc, uint8_t channel);
