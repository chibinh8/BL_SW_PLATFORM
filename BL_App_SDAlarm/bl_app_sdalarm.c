#include "bl_app_sdalarm.h"

#include "BL_Define.h"

#include "esp8266.h"

#define APP_REQUESTBYTE  0x8B
#define APP_REMOTEBYTE   0xA1

#define STARTDATAINDEX 4u


BOOL bl_al_IsESPDatReceived_bo = FALSE;

AlarmSta_en bl_al_AlarmMainstat_en;

static char AlarmESPData[ALARMBUFF]; //25 bytes currently

uint8_t ConfirmReqbyte[3] = {0xCB, 0xC4, 0xC5};

uint8_t ConfirmRemotebyte[3] = {0xE1, 0xC4, 0xC5};

uint8_t CopyRXDataESPClbkSDAlarm(char* RXbuffer){
		uint8_t tempreqbyte;

	  switch((uint8_t)(RXbuffer[STARTDATAINDEX])){
			case APP_REQUESTBYTE:	
				bl_al_IsESPDatReceived_bo = TRUE;
				#ifdef DEBUG_ESP
				HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
				#endif				
				tempreqbyte = ConfirmReqbyte[1];
				ConfirmReqbyte[1] = ConfirmReqbyte[2];
				ConfirmReqbyte[2] = tempreqbyte;
				SendMessagetoESP((char*)ConfirmReqbyte);				
				
			  break;
			
			case APP_REMOTEBYTE:
					bl_al_IsESPDatReceived_bo = TRUE;
					HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
					ConfirmRemotebyte[1] = RXbuffer[STARTDATAINDEX+1] + 0x40;
					ConfirmRemotebyte[2] = RXbuffer[STARTDATAINDEX+2] + 0x40;
					ConfirmReqbyte[1] = ConfirmRemotebyte[1];
					ConfirmReqbyte[2] = ConfirmRemotebyte[2];
				  SendMessagetoESP((char*)ConfirmRemotebyte);			
			
			 break;
			
			default:
				
				break;			

		}
		return E_OK;

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

