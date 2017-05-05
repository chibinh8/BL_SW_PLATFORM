#include "flash.h"
#include "stm32f4xx_hal_flash.h"
#include "string.h"
#include "cmsis_os.h" 
#include "dem.h"
#include <stm32f4xx_hal_flash_ex.h>
#include "timer.h"

extern FaultInfor_st FlashFlt;
NVMJobSta_en bl_fl_NVMJobmainSta_en = BL_IDLE;
uint8_t bl_fl_Sector2Write_u8;
uint32_t bl_fl_currtime_u32;

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

uint8_t bl_fl_Erase_Sector(int sec_no)
{
	FLASH_EraseInitTypeDef er = {
		.TypeErase = FLASH_TYPEERASE_SECTORS,
		.Sector = sec_no,
		.NbSectors = 1,
		.VoltageRange = FLASH_VOLTAGE_RANGE_3
	};
	uint32_t fault_sec = 0;
	HAL_FLASH_Unlock();
	HAL_StatusTypeDef res = HAL_FLASHEx_Erase(&er, &fault_sec);
	HAL_FLASH_Lock();
	return res == HAL_OK ? E_OK : E_NOT_OK;
}

void bl_fl_UserTriggerNVMAction(NVMJobSta_en NVMJobmainSta_en, uint8_t NVMSector){
	bl_fl_NVMJobmainSta_en = NVMJobmainSta_en;
	bl_fl_Sector2Write_u8 = NVMSector;
	
}

void bl_fl_NVMOperationCyclic(void){
	
	switch(bl_fl_NVMJobmainSta_en){
		case BL_IDLE:
		 break;
		case BL_STARTWRITING:
			//so first erease the given sector and then write all data configured in that section
			bl_fl_NVMJobmainSta_en = BL_ERASING;
			break;
		case BL_ERASING:
				taskENTER_CRITICAL();
			  while(bl_fl_Erase_Sector(bl_fl_Sector2Write_u8)!=E_OK);
				GetCurrentTimestamp(&bl_fl_currtime_u32);
				taskEXIT_CRITICAL();
				bl_fl_NVMJobmainSta_en = BL_WRITING;
				
			break;
		case BL_WRITING:
				
			break;
		default:
			
			break;
				
		
	}
	
}
