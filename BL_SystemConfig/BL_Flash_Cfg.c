#ifndef BL_ESPCOM_CONFIG_H
#define BL_ESPCOM_CONFIG_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

#include "flash.h"
#include "adc.h"
#include "pid.h"

extern BL_AdcThres_Type adcreadthres;

extern PIDInfor_st bl_PIDInfor_st;

/*SectorNumber  -  Data*/

DataInSectorInfor_st bl_fl_DataInSector_pu8[NUMBEROFDATASECTOR] = {
	{ADCSENSORTHRES_BASE , (uint8_t*)&adcreadthres.blackupperthres[0],11u, 2u, sizeof(BL_AdcThres_Type)>>1},
		
	{PIDINFORADRESS_BASE , (uint8_t*)&bl_PIDInfor_st, 11u, 4u, sizeof(PIDInfor_st)>>2},
	
};

#endif 

