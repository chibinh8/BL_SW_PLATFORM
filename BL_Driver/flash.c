#include "flash.h"
#include "stm32f4xx_hal_flash.h"
#include "string.h"
#include "cmsis_os.h" 
#include "dem.h"

extern FaultInfor_st FlashFlt;

uint8_t bl_fl_WriteByte2NVM(const uint8_t* data2write_u8, const uint32_t BaseAddress_u32, uint8_t Numofbyte){
	uint32_t address_u32 = BaseAddress_u32;
	HAL_StatusTypeDef RETVAL;
	taskENTER_CRITICAL();
	HAL_FLASH_Unlock();
	for(uint8_t i=0; i<Numofbyte;i++)
		{				
			if(HAL_OK!=HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address_u32,data2write_u8[i])){
				return E_NOT_OK;
			}
			address_u32 +=1; //1 bytes
		}	
	HAL_FLASH_Lock();
	taskEXIT_CRITICAL();
	return E_OK;
}

uint8_t bl_fl_ReadbytefromNVM(uint8_t* data2Read_u8, const uint32_t BaseAddress_u32, uint8_t Numofbyte){
	
const volatile uint8_t *ADD_REG  = ((const volatile uint8_t *)BaseAddress_u32);
memcpy((void*)data2Read_u8, (void*)ADD_REG ,Numofbyte);
return E_OK;

}



void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue){
	FlashFlt.FaultStatus = DEM_FAIL;
	Dem_ErrorReportStatus(&FlashFlt);
	
}
