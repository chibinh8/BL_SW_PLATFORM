#include "bl_app_sdalarm.h"

#include "BL_Define.h"

#include "esp8266.h"
#include "string.h"

#define APP_REQUESTBYTE  0x8B
#define APP_REMOTEBYTE   0xA1

#define STARTDATAINDEX 4u


AlarmTasks_st  AlarmTasks;

ESPDataSendSta_en bl_alarm_SendataStatus;
AlarmTaskMode_en bl_alarm_LastTaskMode_en;

ESPDatadef_st Confirmbyte_str;

uint8_t ConfirmReqbyte[3] = {0xCB, 0xC4, 0xC5};

uint8_t ConfirmRemotebyte[3] = {0xE1, 0xC4, 0xC5};

void bl_al_AlarmInit(void){
	
	AlarmTasks.Mainstate = ALARM_IDLE;
	AlarmTasks.TaskMode = TASKMODE_IDLE;	
	AlarmTasks.IsESPDatReceived_bo = FALSE;
	bl_alarm_SendataStatus = WAITINGRES;
	Confirmbyte_str.Datatype = UINT8;
	Confirmbyte_str.Len = 3;

}

uint8_t CopyRXDataESPClbkSDAlarm(char* RXbuffer){

	  switch((uint8_t)(RXbuffer[STARTDATAINDEX])){
			case APP_REQUESTBYTE:	
			
					memcpy(AlarmTasks.AlarmESPData,RXbuffer,strlen(RXbuffer));
					AlarmTasks.Mainstate = ALARM_CYCLIC;
					AlarmTasks.TaskMode = REQUESTDEVICESTA;	
					AlarmTasks.IsESPDatReceived_bo = TRUE;						
					bl_alarm_LastTaskMode_en = REQUESTDEVICESTA;
					bl_alarm_SendataStatus = WAITINGRES;
			  break;
			
			case APP_REMOTEBYTE:
					
					#ifdef DEBUG_ESP
					HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
					#endif		
					memcpy(AlarmTasks.AlarmESPData,RXbuffer,strlen(RXbuffer));
					AlarmTasks.Mainstate = ALARM_CYCLIC;
					AlarmTasks.TaskMode = REMOTEDEVICE;	
					AlarmTasks.IsESPDatReceived_bo = TRUE;	
					bl_alarm_LastTaskMode_en = REMOTEDEVICE;
					bl_alarm_SendataStatus = WAITINGRES;
			
				break;
				
			 break;
			
			default:
					
					if(((bl_alarm_LastTaskMode_en!=TASKMODE_IDLE)&&(uint8_t)(RXbuffer[0])==(uint8_t)'>')){
	
						bl_alarm_SendataStatus = READY;
						AlarmTasks.Mainstate = ALARM_CYCLIC;
						AlarmTasks.TaskMode = bl_alarm_LastTaskMode_en;
						bl_alarm_LastTaskMode_en = TASKMODE_IDLE;
					}
				break;			

		}
		return E_OK;

}

void bl_al_AlarmCyclic(void){
	
		uint8_t tempswap;	
		switch(AlarmTasks.Mainstate){
			
			case ALARM_IDLE:
				/*Do something while waiting for data*/

				break;
			case ALARM_CYCLIC:
					/*Process action on according request type*/
					switch(AlarmTasks.TaskMode){
						case TASKMODE_IDLE:
							/*Do something*/
							break;
						case REQUESTDEVICESTA:
							/*process incoming data then do action*/
							Confirmbyte_str.data = (void*)&ConfirmReqbyte[0];
							SendMessagetoESP(Confirmbyte_str, bl_alarm_SendataStatus);	
							AlarmTasks.TaskMode = TASKMODE_IDLE;
							break;
						case REMOTEDEVICE:			
							/*process incoming data then do action*/
							if(bl_alarm_SendataStatus==READY){
							tempswap = ConfirmRemotebyte[1];
							ConfirmRemotebyte[1] = ConfirmRemotebyte[2];
							ConfirmRemotebyte[2] = tempswap;
							}
							Confirmbyte_str.data = (void*)&ConfirmRemotebyte[0];
							SendMessagetoESP(Confirmbyte_str, bl_alarm_SendataStatus);		
							AlarmTasks.TaskMode = TASKMODE_IDLE;
						
							break;
						
						default:
							
							break;				
						
					}
					
				 break;
			default:
				
				break;
			
		}
	
}

