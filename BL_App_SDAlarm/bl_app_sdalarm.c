#include "bl_app_sdalarm.h"

#include "BL_Define.h"



BOOL bl_al_IsESPDatReceived_bo = FALSE;

AlarmSta_en bl_al_AlarmMainstat_en;

static char AlarmESPData[ALARMBUFF]; //25 bytes currently

uint8_t CopyRXDataESPClbkSDAlarm(char* RXbuffer){
		
		bl_al_IsESPDatReceived_bo = TRUE;
		#ifdef DEBUG_ESP
		if((uint8_t)(RXbuffer[4])==0xA1)
			HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
		return E_OK;
		#endif
}

void bl_al_AlarmCyclic(void){
		switch(bl_al_AlarmMainstat_en){
			case INIT:
				if(bl_al_IsESPDatReceived_bo==TRUE){
					
					bl_al_AlarmMainstat_en = CYCLIC;
				}
				break;
			case CYCLIC:
				
				break;
			default:
				
				break;
			
		}
	
}

