#ifndef BL_ADC_H
#define BL_ADC_H
#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

#include "flash.h"

#define NumofSampling 8
#define NumOfSensor1  8
#define NumOfSensor2 0
#define NumofSensor (uint8_t)(NumOfSensor1+NumOfSensor2)
#define NumOfIgnoreEle				2

typedef struct{
		__IO uint16_t blackupperthres[NumofSensor];
		__IO uint16_t whitelowwerthres[NumofSensor];
		
}BL_AdcThres_Type;

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

BOOL ReadStatusofAllsensor(LineState * OutStatusSS);

void ADCSensorMaincyclic(void);
void ReadADCThreshold(	BL_AdcThres_Type *adcreadthres);

uint8_t SaveADCThreshold2NVM(const BL_AdcThres_Type AdcThres);

void ReadADCThresholdfromNVM(uint16_t *val2write);

void bl_adc_GetFinalSensorSta(LineState *FinalLineSensorSta);

#endif

