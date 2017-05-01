#ifndef BL_FLASH_H
#define BL_FLASH_H 

#include "BL_Define.h"

#define ADCSENSORTHRES_BASE ((uint32_t)0x080E0000)

typedef struct{
		__IO uint16_t blackupperthres[NumofSensor];
		__IO uint16_t whitelowwerthres[NumofSensor];
		
}BL_AdcThres_Type;

void ReadADCThreshold(	BL_AdcThres_Type *adcreadthres);
uint8_t SaveADCThreshold2NVM(const BL_AdcThres_Type AdcThres);

void ReadADCThresholdfromNVM(uint16_t *val2write);

uint8_t bl_fl_WriteByte2NVM(const uint8_t* data2write_u8, const uint32_t BaseAddress_u32, uint8_t Numofbyte);

uint8_t bl_fl_ReadbytefromNVM(uint8_t* data2Read_u8, const uint32_t BaseAddress_u32, uint8_t Numofbyte);

#endif

