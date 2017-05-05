#ifndef BL_FLASH_H
#define BL_FLASH_H 

#include "BL_Define.h"

#define NUMBEROFDATASECTOR    3

typedef enum{
	
	BL_IDLE,
	BL_BUSY,
	BL_STARTWRITING,
	BL_ERASING,
	BL_WRITING,
	BL_WRITEDONE,
	BL_ERROR,
	
}NVMJobSta_en;

typedef struct{
uint32_t NVMAddress_u32;
void* Data;
uint8_t SectorNum_u8;
uint8_t DatatypeLen;
uint8_t size;	
}DataInSectorInfor_st;

typedef struct{
	
NVMJobSta_en  NVMJobSta;
uint8_t SectorNum_u8;
	
}DataWriteJob_st;


uint8_t bl_fl_WriteByte2NVM(volatile uint8_t* data2write_u8, const uint32_t BaseAddress_u32, uint8_t Numofbyte);

uint8_t bl_fl_ReadbytefromNVM(uint8_t* data2Read_u8, const uint32_t BaseAddress_u32, uint8_t Numofbyte);

uint8_t bl_fl_Erase_Sector(int sec_no);

void bl_fl_UserTriggerNVMAction(NVMJobSta_en NVMJobmainSta_en, uint8_t NVMSector);
NVMJobSta_en bl_fl_GetNVMJobSta(void);

void bl_fl_NVMOperationCyclic(void);

#endif

