#include "bl_app_sdalarm.h"

#include "BL_Define.h"

BOOL bl_al_IsESPDatReceived_bo = FALSE;

static char AlarmESPData[ALARMBUFF]; //25 bytes currently

uint8_t CopyRXDataESPClbkSDAlarm(char* RXbuffer){
		
		bl_al_IsESPDatReceived_bo = TRUE;
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		return E_OK;
}

void bl_al_AlarmCyclic(void){
		
	;
	
}

