#include "dem.h"



void Error_Handler(void){
	
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
}


void Dem_ErrorReportStatus(uint8_t FaultID, Errorsta_en errorsta){
	
	
}


uint8_t DebouncingError(uint16_t threshold_u16, uint16_t *deboundcnt);

