#include "flash.h"
#include "stm32f4xx_hal_flash.h"
#include "string.h"

#define ADCSENSORTHRES_BASE ((uint32_t)0x080E0000)

#define ADCSENSORTHRES_REG  ((const volatile uint16_t *)ADCSENSORTHRES_BASE)

#define ADCSENSORTHRES ((const volatile BL_AdcThres_Type *)ADCSENSORTHRES_BASE)

void ReadADCThreshold(	BL_AdcThres_Type *adcreadthres){
	/*need to be defined*/
	memcpy((void*)adcreadthres->blackupperthres, (void*)ADCSENSORTHRES->blackupperthres,NumofSensor*2);
	memcpy((void*)adcreadthres->whitelowwerthres, (void*)ADCSENSORTHRES->whitelowwerthres,NumofSensor*2);
}


HAL_StatusTypeDef SaveADCThreshold2NVM(const BL_AdcThres_Type AdcThres){
	/*need to be defined*/
	HAL_StatusTypeDef RETVAL;
	volatile uint32_t addressflash = ADCSENSORTHRES_BASE;
	HAL_FLASH_Unlock();
	for(uint8_t i=0; i<NumofSensor;i++)
		{
			RETVAL = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addressflash,AdcThres.blackupperthres[i]);			
			addressflash +=2; //2 bytes
		}
	for(uint8_t i=0; i<NumofSensor;i++)
		{
			RETVAL = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addressflash,AdcThres.whitelowwerthres[i]);			
			addressflash +=2; //2 bytes
		}
	HAL_FLASH_Lock(); 
	return RETVAL;
}

void ReadADCThresholdfromNVM(uint16_t *val2write){
	*val2write = (*ADCSENSORTHRES_REG);
	
}
