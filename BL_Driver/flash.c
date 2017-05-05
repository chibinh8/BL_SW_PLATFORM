#include "flash.h"
#include "stm32f4xx_hal_flash.h"
#include "string.h"
#include "cmsis_os.h" 
#include "dem.h"
#include <stm32f4xx_hal_flash_ex.h>
#include "timer.h"

#define ONEBYTEWAITTIME  200u
#define DATASECTORWAITTIME  10000u

extern FaultInfor_st FlashFlt;
extern DataInSectorInfor_st bl_fl_DataInSector_pu8[NUMBEROFDATASECTOR];
uint8_t bl_flDataSectorIndex_u8 = 255;
uint8_t bl_flEndDataSectorIndex_u8 = 255;

uint8_t bl_fl_NumOfByte2Flash_u8;

DataWriteJob_st bl_fl_DataWriteJob_st;

uint32_t bl_fl_currtime_u32;
uint32_t bl_fl_Wiringtime_u32;

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
	
	bl_fl_DataWriteJob_st.NVMJobSta = NVMJobmainSta_en;
	bl_fl_DataWriteJob_st.SectorNum_u8 = NVMSector;
	
}


void bl_fl_WriteChunkofData2SectorNVM(void){
	
			uint32_t address_u32;
			uint8_t Multifactor_u8;
	
			Multifactor_u8 = bl_fl_DataInSector_pu8[bl_flDataSectorIndex_u8].DatatypeLen;
			address_u32 = bl_fl_DataInSector_pu8[bl_flDataSectorIndex_u8].NVMAddress_u32 + bl_fl_NumOfByte2Flash_u8*Multifactor_u8;
			taskENTER_CRITICAL();
			while(E_OK!=bl_fl_WriteByte2NVM((const uint8_t*)(bl_fl_DataInSector_pu8[bl_flDataSectorIndex_u8].Data),address_u32,Multifactor_u8)){
						 FlashFlt.FaultStatus = DEM_FAIL;
						 Dem_ErrorReportStatus(&FlashFlt);
					};	
			taskEXIT_CRITICAL();
			bl_fl_NumOfByte2Flash_u8 +=1;
			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);		
			if(bl_fl_NumOfByte2Flash_u8==bl_fl_DataInSector_pu8[bl_flDataSectorIndex_u8].size){
				bl_fl_NumOfByte2Flash_u8 = 0;
				bl_flDataSectorIndex_u8++;
			}			

}

void bl_fl_NVMOperationCyclic(void){
	
	switch(bl_fl_DataWriteJob_st.NVMJobSta){
		case BL_IDLE:
			
		 break;
		case BL_STARTWRITING:
			//so first erease the given sector and then write all data configured in that section
			bl_fl_DataWriteJob_st.NVMJobSta = BL_ERASING;
			break;  
		case BL_ERASING:

				//find the first index of data sector
				for(uint8_t LoopIndex = 0; LoopIndex<NUMBEROFDATASECTOR;LoopIndex++){
					if(bl_fl_DataInSector_pu8[LoopIndex].SectorNum_u8 ==bl_fl_DataWriteJob_st.SectorNum_u8){							
							if(bl_flDataSectorIndex_u8==255){
								bl_flDataSectorIndex_u8 = LoopIndex;
								bl_flEndDataSectorIndex_u8 = 0;
							}
							bl_flEndDataSectorIndex_u8++;
					}	
				}
				if(bl_flEndDataSectorIndex_u8!=255){
					taskENTER_CRITICAL();
					while(bl_fl_Erase_Sector(bl_fl_DataWriteJob_st.SectorNum_u8)!=E_OK);				
					taskEXIT_CRITICAL();
					bl_flEndDataSectorIndex_u8 -=1;
					bl_fl_DataWriteJob_st.NVMJobSta = BL_WRITING;
					GetCurrentTimestamp(&bl_fl_currtime_u32); 
					GetCurrentTimestamp(&bl_fl_Wiringtime_u32);
				}else{
					 bl_fl_DataWriteJob_st.NVMJobSta = BL_IDLE; //not found data sector
				}
			break;
		case BL_WRITING: 
				// check in config table, if the current element belongs to sector then write the corresponding data to NVM
			  if(TRUE==CheckTimestampElapsed(bl_fl_currtime_u32, (uint32_t)ONEBYTEWAITTIME)&&(FALSE==CheckTimestampElapsed(bl_fl_Wiringtime_u32, (uint32_t)DATASECTORWAITTIME))){
						bl_fl_WriteChunkofData2SectorNVM();
						GetCurrentTimestamp(&bl_fl_currtime_u32);
						if(bl_flDataSectorIndex_u8==NUMBEROFDATASECTOR) //write all data sector
							bl_fl_DataWriteJob_st.NVMJobSta = BL_WRITEDONE;
			  }else if(TRUE==CheckTimestampElapsed(bl_fl_Wiringtime_u32, (uint32_t)DATASECTORWAITTIME)){ //TIMEOUT for writing
						/*Notify DEM*/
						FlashFlt.FaultStatus = DEM_FAIL;
						Dem_ErrorReportStatus(&FlashFlt);
						bl_fl_DataWriteJob_st.NVMJobSta = BL_ERROR;							
				}
		break;
		
		case BL_WRITEDONE:
				bl_fl_DataWriteJob_st.NVMJobSta = BL_IDLE;
				break;
	  case BL_ERROR:			
				
		
				break;
		default:
			/*Do something*/
			break;				
		
	}
	
}

NVMJobSta_en bl_fl_GetNVMJobSta(void){
	return bl_fl_DataWriteJob_st.NVMJobSta;
}
