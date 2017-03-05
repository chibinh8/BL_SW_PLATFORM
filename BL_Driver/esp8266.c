#ifndef BL_ESP_H
#define BL_ESP_H 

#include "esp8266.h"
#include "uart.h"
#include "string.h"
#include "timer.h"
#include "cmsis_os.h" 

#define ESPREADBUFF (BL_BUFFSIZE>>1)
#define SSID    "BLPRO"
#define PASS		"123456"



extern UART_HandleTypeDef BL_UART;
static char Rx_Buffer_cp[ESPREADBUFF];

typedef enum{
		WAITING,
		STATE1,
		STATE2,
		STATE3,	
}Recheckstate_en;

uint8_t InitESp8266(void){
	uint32_t timecurr_u32 =0;
	memset(Rx_Buffer_cp,0,ESPREADBUFF);
	ClearRxBuffer();
	printf("AT\r\n");
	HAL_Delay(100);

	if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_cp,0,ESPREADBUFF)){
			if((Rx_Buffer_cp[0]=='O')&&(Rx_Buffer_cp[1]=='K')){
				printf("AT+CIPMUX=1\r\n");
				HAL_Delay(300);
				//WiFi Mode is AP
				printf("AT+CWMODE=2\r\n");
				HAL_Delay(300);//300ms
				printf("AT+CWSAP=\"%s\",\"%s\",3,0\r\n",SSID, PASS);
				HAL_Delay(100);
				printf("AT+CIPSERVER=1,150\r\n");	
				HAL_Delay(1000);//1s
				ClearRxBuffer();//clear buffer
				return E_OK;
		}				
	}	
	
  return E_NOT_OK;
	
}


void ESPOperationCyclic(void){
	
	
	
	
}

static uint8_t RecheckESPServerWithDelay(void){
	ClearRxBuffer();//clear buffer
	//TCP/UDP Connections
	printf("AT+CIPMUX=1\r\n");	
	//waiting for respond OK from module
	HAL_Delay(300);//300ms	
	if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_cp,0,ESPREADBUFF))
		{
			if((Rx_Buffer_cp[0]=='O')&&(Rx_Buffer_cp[1]=='K')){	
			ClearRxBuffer();//clear buffer
			//Set as server
			printf("AT+CIPSERVER=1,150\r\n");
			//waiting for respond OK from module	
			HAL_Delay(300);//300ms				
			if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_cp,0,ESPREADBUFF))
				{
					if((Rx_Buffer_cp[0]=='O')&&(Rx_Buffer_cp[1]=='K')){	
							return E_OK;
					} 
		    }
		}			
	}
		
	return E_NOT_OK; 
}

static uint8_t RecheckESPServer(void){
	
	static uint16_t t_delay_u16 = 0;
	static Recheckstate_en t_recheckState_en = STATE1;
	static uint32_t timecurr_u32 = 0;
	static Recheckstate_en t_LastRecheckState_en = STATE1;
	
	switch(t_recheckState_en){	
		
		case STATE1: 
			t_LastRecheckState_en  = t_recheckState_en;
			ClearRxBuffer();//clear buffer
			//TCP/UDP Connections
			printf("AT+CIPMUX=1\r\n");	
	    //waiting for respond OK from module		
			t_recheckState_en = WAITING;
			t_delay_u16 = 300u;
			GetCurrentTimestamp(&timecurr_u32);
		  break;
				
		case WAITING:		
			
			if((CheckTimestampElapsed(timecurr_u32, (uint32_t)t_delay_u16))&&(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_cp,0,ESPREADBUFF)) \
				&&((Rx_Buffer_cp[0]=='O')&&(Rx_Buffer_cp[1]=='K')))
				{			
					    t_recheckState_en = t_LastRecheckState_en+1;
							ClearRxBuffer();//clear buffer
				}
			else{
							return E_NOT_OK;
				}				
		  break;			
				
		case STATE2:
			
			t_LastRecheckState_en  = t_recheckState_en;		
			ClearRxBuffer();//clear buffer
			//Set as server
			printf("AT+CIPSERVER=1,150\r\n");
	    //waiting for respond OK from module		
			t_recheckState_en = WAITING;
			t_delay_u16 = 300u;
			GetCurrentTimestamp(&timecurr_u32);				
			break;
		case STATE3: //currently state 3, init process is already sucessful.
					
				return E_OK;
				break;		
		default:
			
		  break;
  }

}


#endif

