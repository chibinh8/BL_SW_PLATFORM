#include "esp8266.h"
#include "uart.h"
#include "string.h"
#include "timer.h"
#include "cmsis_os.h" 
#include "dem.h"
#include "ECUModeManager.h"

#define ESPErrorMaxcnt 10
#define GPIOPINESPRESET GPIO_PIN_12

extern UART_HandleTypeDef BL_UART;
static char Rx_Buffer_ESP[ESPREADBUFF];

static uint8_t RecheckESPServer(void);
static uint8_t IsReceivedDatafromESP(char *Rx_Buffer_c);
uint8_t GetRawDatafromESP(void);

extern uint8_t (*CopyRXDataESPClbk[]) (char* RXbuffer);
uint8_t SendMessagetoESP(char *data);

void HardResetESP(void){
	
	HAL_GPIO_WritePin(GPIOD, GPIOPINESPRESET,GPIO_PIN_RESET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(GPIOD, GPIOPINESPRESET,GPIO_PIN_SET);
	
}

void EnableESP(void){
	
	HAL_GPIO_WritePin(GPIOD, GPIOPINESPRESET,GPIO_PIN_SET);
}

void DisableESPHardware(void){
	
	HAL_GPIO_WritePin(GPIOD, GPIOPINESPRESET,GPIO_PIN_RESET);
	
}



uint8_t ESPGeneralState_u8 = 1;

uint8_t InitESp8266(void){	
	
	EnableESP();
	memset(Rx_Buffer_ESP,0,ESPREADBUFF);
	ClearRxBuffer();
	printf("AT\r\n");
	HAL_Delay(100);
	
	if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_ESP,0,ESPREADBUFF)){
			if((Rx_Buffer_ESP[0]=='O')&&(Rx_Buffer_ESP[1]=='K')){
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

static uint32_t timecurrCyc_u32 = 0;
static uint16_t InitErrorCnt_u16 =0;

void ESPOperationCyclic(void){
	
	switch(ESPGeneralState_u8){		
		case 0: //idle
			 GetCurrentTimestamp(&timecurrCyc_u32);
			 ESPGeneralState_u8 = 1;
			break;
		case 1: //init
		  if(E_OK==InitESp8266()){
			 ESPGeneralState_u8 = 2;
			 HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
			 GetCurrentTimestamp(&timecurrCyc_u32);
		  }else if((CheckTimestampElapsed(timecurrCyc_u32, (uint32_t)10000))==TRUE){
					HardResetESP();
					ESPGeneralState_u8 = 0;
			}

			break;
		case 2:
		//Normal communication
		(void)GetRawDatafromESP();
			/*Recheck sever every 5s*/
			if((CheckTimestampElapsed(timecurrCyc_u32, (uint32_t)5000))==TRUE){
					ESPGeneralState_u8 = 3;
					GetCurrentTimestamp(&timecurrCyc_u32);
			}
			break;
		case 3: 			
			
			if((CheckTimestampElapsed(timecurrCyc_u32, (uint32_t)800)==TRUE)||(RecheckESPServer()==E_OK)){
					ESPGeneralState_u8 = 2;
					GetCurrentTimestamp(&timecurrCyc_u32);
			}
		
		default:
			break;
	}

}
				

static uint8_t RecheckESPServerWithDelay(void){
	ClearRxBuffer();//clear buffer
	//TCP/UDP Connections
	printf("AT+CIPMUX=1\r\n");	
	//waiting for respond OK from module
	HAL_Delay(300);//300ms	
	if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_ESP,0,ESPREADBUFF))
		{
			if((Rx_Buffer_ESP[0]=='O')&&(Rx_Buffer_ESP[1]=='K')){	
			ClearRxBuffer();//clear buffer
			//Set as server
			printf("AT+CIPSERVER=1,150\r\n");
			//waiting for respond OK from module	
			HAL_Delay(300);//300ms				
			if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_ESP,0,ESPREADBUFF))
				{
					if((Rx_Buffer_ESP[0]=='O')&&(Rx_Buffer_ESP[1]=='K')){	
							return E_OK;
					} 
		    }
		}			
	}
		
	return E_NOT_OK; 
}

static uint8_t t_recheckState_en = STATE1;
static uint8_t ESPErrorcnt_u8 = 0;	

static uint8_t RecheckESPServer(void){
	
	static uint16_t t_delay_u16 = 0;

	static uint32_t timecurr_u32 = 0;
	static uint8_t t_LastRecheckState_en = STATE1;
	
	switch(t_recheckState_en){	
		
		case STATE1: 
			t_LastRecheckState_en  = t_recheckState_en;
			ClearRxBuffer();//clear buffer
			memset(Rx_Buffer_ESP,0,ESPREADBUFF);
			//TCP/UDP Connections
			//printf("AT+CIPMUX=1\r\n");	
			printf("AT\r\n");
	    //waiting for respond OK from module		
			t_recheckState_en = WAITING;
			t_delay_u16 = 100u;
			GetCurrentTimestamp(&timecurr_u32);
		  break;
				
		case WAITING:		
			if(IsReceivedDatafromESP(Rx_Buffer_ESP)==FALSE){
				if((CheckTimestampElapsed(timecurr_u32, (uint32_t)t_delay_u16)==TRUE)&&(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_ESP,0,ESPREADBUFF)) \
					&&((Rx_Buffer_ESP[0]=='O')&&(Rx_Buffer_ESP[1]=='K')))
					{			GetCurrentTimestamp(&timecurr_u32);
								t_recheckState_en = ((uint8_t)t_LastRecheckState_en+1);
								t_delay_u16 = 100u;
								ClearRxBuffer();//clear buffer
								memset(Rx_Buffer_ESP,0,ESPREADBUFF);
					}
				else if(CheckTimestampElapsed(timecurr_u32, (uint32_t)t_delay_u16)==TRUE){		/*retry send request*/
								t_recheckState_en = t_LastRecheckState_en;
								ESPErrorcnt_u8++;								
								if(ESPErrorcnt_u8>ESPErrorMaxcnt){
									ESPErrorcnt_u8=0;		
									ESPGeneralState_u8 = 1;
									HardResetESP();
									ResetUARTESP();
									SofResetSysem();
								}
					
								return E_NOT_OK;
					}	
				else{
					
						return E_NOT_OK;
				}
			}else{
						return E_OK;
			}
		  break;			
				
		case STATE2:
			
			t_LastRecheckState_en  = t_recheckState_en;		
			ClearRxBuffer();//clear buffer
			memset(Rx_Buffer_ESP,0,ESPREADBUFF);
			//Set as server
			//printf("AT+CIPSERVER=1,150\r\n");
			printf("AT\r\n");
	    //waiting for respond OK from module		
			t_recheckState_en = WAITING;
			t_delay_u16 = 100u;
			GetCurrentTimestamp(&timecurr_u32);				
			break;
		case STATE3: //currently state 3, init process is already sucessful.
	
				t_recheckState_en = STATE1;
				return E_OK;
		
				break;		
				
		default:
			
		  break;
  }
	
	return E_NOT_OK;

}

uint8_t GetRawDatafromESP(void){
	uint8_t LoopIndex = 0;
	uint8_t RetVal;
	if(TRUE==IsReceivedDatafromESP(Rx_Buffer_ESP)){ //data is exactly recieved from ESP
			//parse data from esp then call RX copy data function of caller module
			for(LoopIndex=0;LoopIndex<2;LoopIndex++){
						
					(void)CopyRXDataESPClbk[LoopIndex](Rx_Buffer_ESP);	
					SendMessagetoESP(Rx_Buffer_ESP);
			}
			
			RetVal = E_OK;
			
	}else{
		
			RetVal = E_NOT_OK;
	}
			
	return RetVal; 
}


static uint8_t IsReceivedDatafromESP(char *Rx_Buffer_c){
		
		if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_c,0,ESPREADBUFF)){
				if(Rx_Buffer_ESP[0]=='+'&&Rx_Buffer_ESP[1]=='I'&&Rx_Buffer_ESP[2]=='P'&&Rx_Buffer_ESP[3]=='D'&&Rx_Buffer_ESP[4]==','){
					 ClearRxBuffer();//clear buffer
					 return TRUE;
				}				
		}		
	  return FALSE;
}


uint8_t SendMessagetoESP(char *data){
	uint8_t datalenth_u8, portID;
	printf("AT+CIPSEND=%d,%d\r\n",0, strlen(data)); 
	HAL_Delay(5);//5ms
	printf("%s\r\n",data); 
	return E_OK;
}

uint8_t SaveESPInfoToNVM(void){
	//Infor: SSID, PASS, Faulty, ESP status...
	
	return E_OK;
	
}




