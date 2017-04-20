#include "bl_app_sdalarm.h"

#include "BL_Define.h"

#include "esp8266.h"
#include "string.h"

#define APP_REQUESTBYTE  0x8B
#define APP_REMOTEBYTE   0xA1

#define STARTDATAINDEX 4u


AlarmTasks_st  AlarmTasks;


const uint8_t ConfirmReqbyte[3] = {0xCB, 0xC4, 0xC5};

const uint8_t ConfirmRemotebyte[3] = {0xE1, 0xC4, 0xC5};

void bl_al_AlarmInit(void){
	
	AlarmTasks.Mainstate = ALARM_IDLE;
	AlarmTasks.TaskMode = TASKMODE_IDLE;	
	AlarmTasks.IsESPDatReceived_bo = FALSE;
}

uint8_t CopyRXDataESPClbkSDAlarm(char* RXbuffer){

	  switch((uint8_t)(RXbuffer[STARTDATAINDEX])){
			case APP_REQUESTBYTE:	
					#ifdef DEBUG_ESP
					HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
					#endif				
					memcpy(AlarmTasks.AlarmESPData,RXbuffer,strlen(RXbuffer));
					AlarmTasks.Mainstate = ALARM_CYCLIC;
					AlarmTasks.TaskMode = REQUESTDEVICESTA;	
					AlarmTasks.IsESPDatReceived_bo = TRUE;						
				
			  break;
			
			case APP_REMOTEBYTE:

					#ifdef DEBUG_ESP
					HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
					#endif		
					memcpy(AlarmTasks.AlarmESPData,RXbuffer,strlen(RXbuffer));
					AlarmTasks.Mainstate = ALARM_CYCLIC;
					AlarmTasks.TaskMode = REMOTEDEVICE;	
					AlarmTasks.IsESPDatReceived_bo = TRUE;	
			
			 break;
			
			default:
				
				break;			

		}
		return E_OK;

}

void bl_al_AlarmCyclic(void){
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
							memcpy(AlarmTasks.ConfirmAlarm2App,ConfirmReqbyte,3);
							SendMessagetoESP((char*)AlarmTasks.ConfirmAlarm2App);	
							AlarmTasks.TaskMode = TASKMODE_IDLE;
							break;
						case REMOTEDEVICE:			
							/*process incoming data then do action*/
						  memcpy(AlarmTasks.ConfirmAlarm2App,ConfirmRemotebyte,3);
							SendMessagetoESP((char*)AlarmTasks.ConfirmAlarm2App);				
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

