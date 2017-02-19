#ifndef BL_FLASH_H
#define BL_FLASH_H 

#include "BL_Define.h"


typedef struct{
		__IO uint16_t blackupperthres[NumofSensor];
		__IO uint16_t whitelowwerthres[NumofSensor];
		
}BL_AdcThres_Type;

void ReadADCThreshold(	BL_AdcThres_Type *adcreadthres);
HAL_StatusTypeDef SaveADCThreshold2NVM(const BL_AdcThres_Type AdcThres);
void ReadADCThresholdfromNVM(uint16_t *val2write);

#endif

