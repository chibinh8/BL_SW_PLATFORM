#ifndef BL_ADC_H
#define BL_ADC_H
#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

#include "flash.h"


typedef enum {
		BLACK,
		WHITE,
		UNDEFINE,
}LineState;

typedef enum{
		CALIB,
		CYCLIC,
		ADCUNDEFINE,
}ADCMode;


void BL_ADCInit(void);

void ReadSensor(volatile uint16_t* outsensorval, ADC_HandleTypeDef *hadc, uint8_t channel);

uint8_t ReadAllFinalSensorfromLine(uint16_t *AllsensorFinalVal);

void ReadAllRawSensorfromLine(void);

BOOL ReadStatusofAllsensor(uint8_t * OutStatusSS);

void ADCSensorMaincyclic(void);

#endif

