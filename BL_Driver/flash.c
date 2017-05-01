#include "flash.h"
#include "stm32f4xx_hal_flash.h"
#include "string.h"
#include "cmsis_os.h" 

#define ADCSENSORTHRES_BASE ((uint32_t)0x080E0000)

#define ADCSENSORTHRES_REG  ((const volatile uint16_t *)ADCSENSORTHRES_BASE)

#define ADCSENSORTHRES ((const volatile BL_AdcThres_Type *)ADCSENSORTHRES_BASE)
	
#define GETBASEADDRESS(BaseAddress_u32)  ((const volatile uint8_t *)BaseAddress_u32);

void ReadADCThreshold(	BL_AdcThres_Type *adcreadthres){
	/*need to be defined*/
	memcpy((void*)adcreadthres->blackupperthres, (void*)ADCSENSORTHRES->blackupperthres,NumofSensor*2);
	memcpy((void*)adcreadthres->whitelowwerthres, (void*)ADCSENSORTHRES->whitelowwerthres,NumofSensor*2);
}


uint8_t SaveADCThreshold2NVM(const BL_AdcThres_Type AdcThres){
	/*need to be defined*/
	uint8_t i=0;
	uint8_t RETVAL = E_NOT_OK;
	static volatile uint32_t addressflash = ADCSENSORTHRES_BASE;
	taskENTER_CRITICAL();
	HAL_FLASH_Unlock();
	for(i=0; i<NumofSensor;i++)
		{
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addressflash, AdcThres.blackupperthres[i])!=HAL_OK){
				return E_NOT_OK;
			}
			addressflash +=2; //2 bytes
		}
	for(i=0; i<NumofSensor;i++)
		{
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addressflash,AdcThres.whitelowwerthres[i])!=HAL_OK){
				return E_NOT_OK;
			}
			addressflash +=2; //2 bytes
		}
	HAL_FLASH_Lock(); 
	taskEXIT_CRITICAL();
	return E_OK;
}

void ReadADCThresholdfromNVM(uint16_t *val2write){
	*val2write = (*ADCSENSORTHRES_REG);
	
}

uint8_t bl_fl_WriteByte2NVM(const uint8_t* data2write_u8, const uint32_t BaseAddress_u32, uint8_t Numofbyte){
	uint32_t address_u32 = BaseAddress_u32;
	HAL_StatusTypeDef RETVAL;
	HAL_FLASH_Unlock();
	for(uint8_t i=0; i<Numofbyte;i++)
		{				
			if(HAL_OK!=HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address_u32,data2write_u8[i])){
				return E_NOT_OK;
			}
			address_u32 +=1; //1 bytes
		}	
	HAL_FLASH_Lock();
	return E_OK;
}

uint8_t bl_fl_ReadbytefromNVM(uint8_t* data2Read_u8, const uint32_t BaseAddress_u32, uint8_t Numofbyte){
	
const volatile uint8_t *ADD_REG  = ((const volatile uint8_t *)BaseAddress_u32);
memcpy((void*)data2Read_u8, (void*)ADD_REG ,Numofbyte);
return E_OK;

}
