#include "esp8266.h"
#include "uart.h"
#include "string.h"
#include "timer.h"
#include "cmsis_os.h" 
#include "dem.h"
#include "ECUModeManager.h"
#include "flash.h"

#define ESPErrorMaxcnt 10
#define GPIOPINESPRESET GPIO_PIN_12

#define ESPBASEADDR   ((uint32_t)0x080F0000)
#define ESPCONFINFOR_AU8 ((const volatile uint8_t) *ESPBASEADDR)

extern UART_HandleTypeDef BL_UART;
static char Rx_Buffer_ESP[ESPREADBUFF];
ESPInfor_st ESPInforConfg; 


static uint8_t RecheckESPServer(void);
static uint8_t IsReceivedDatafromESP(char *Rx_Buffer_c);
uint8_t GetRawDatafromESP(void);
uint8_t ReadESPInforfromNVM(void);
uint8_t SaveESPInfoToNVM(void);


extern uint8_t (*CopyRXDataESPClbk[]) (char* RXbuffer);

BOOL bl_esp_DataRecieved_bo = FALSE;

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
uint8_t bl_esp_Initsta_u8 = 0;
uint8_t bl_esp_LastInitsta_u8 = 0;
static uint32_t InittimecurrCyc_u32 = 0;
static uint32_t InitTimeDelay_u32 = 0;

uint8_t bl_esp_InitESPSys(void){
		  if(ReadESPInforfromNVM()==E_NOT_OK){
				strcpy(&ESPInforConfg.SsId[0], SSID);
				strcpy(&ESPInforConfg.Pass[0], PASS);
				SaveESPInfoToNVM();
		  }	
			return E_OK;
}

uint8_t InitESp8266(void){	
	switch(bl_esp_Initsta_u8){

		case 0: 
		
			EnableESP();
			memset(Rx_Buffer_ESP,0,ESPREADBUFF);
			ClearRxBuffer();	
			bl_esp_Initsta_u8 = 1;
		  break;
		case 1:
				printf("AT\r\n");
				bl_esp_Initsta_u8 = 2;
				GetCurrentTimestamp(&InittimecurrCyc_u32);
				InitTimeDelay_u32 = 100u;
				bl_esp_LastInitsta_u8 = bl_esp_Initsta_u8;
				break;
		case 2: //wait state
			if((CheckTimestampElapsed(InittimecurrCyc_u32, (uint32_t)InitTimeDelay_u32))==TRUE){
					bl_esp_Initsta_u8 = bl_esp_LastInitsta_u8+1;
			}
			break;
		case 3:
				printf("AT+CIPMUX=1\r\n");				
				GetCurrentTimestamp(&InittimecurrCyc_u32);
				InitTimeDelay_u32 = 300u;
				bl_esp_LastInitsta_u8 = bl_esp_Initsta_u8;	
				bl_esp_Initsta_u8 = 2;
				break;
		case 4: 
				printf("AT+CWMODE=2\r\n");				
				GetCurrentTimestamp(&InittimecurrCyc_u32);
				InitTimeDelay_u32 = 300u;
				bl_esp_LastInitsta_u8 = bl_esp_Initsta_u8;	
				bl_esp_Initsta_u8 = 2;
				break;				
		case 5: 
				printf("AT+CWSAP=\"%s\",\"%s\",3,0\r\n",SSID, PASS);			
				GetCurrentTimestamp(&InittimecurrCyc_u32);
				InitTimeDelay_u32 = 100u;
				bl_esp_LastInitsta_u8 = bl_esp_Initsta_u8;	
				bl_esp_Initsta_u8 = 2;
				break;	
		case 6: 
				printf("AT+CIPSERVER=1,150\r\n");			
				GetCurrentTimestamp(&InittimecurrCyc_u32);
				InitTimeDelay_u32 = 1000u;
				bl_esp_LastInitsta_u8 = bl_esp_Initsta_u8;	
				bl_esp_Initsta_u8 = 2;
				break;	
		case 7: 
			ClearRxBuffer();//clear buffer
			return E_OK;
			break;
		
		default:
			
		    break;		
		
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
		  }
			else if((CheckTimestampElapsed(timecurrCyc_u32, (uint32_t)10000))==TRUE){
					HardResetESP();
					ESPGeneralState_u8 = 0;
			}

			break;
		case 2:
		//Normal communication
		(void)GetRawDatafromESP();
			/*Recheck sever every 5s*/
			if((CheckTimestampElapsed(timecurrCyc_u32, (uint32_t)5000))==TRUE){ 
					if(bl_esp_DataRecieved_bo==FALSE){//over 5s but not receive data
					   ESPGeneralState_u8 = 3; //change state to check sever status
						 GetCurrentTimestamp(&timecurrCyc_u32);
					}else					
					{		
						 bl_esp_DataRecieved_bo = FALSE;
						 GetCurrentTimestamp(&timecurrCyc_u32);
          }
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
					 bl_esp_DataRecieved_bo = TRUE;
					(void)CopyRXDataESPClbk[LoopIndex](Rx_Buffer_ESP);	
					 
			}
			
			RetVal = E_OK;
			
	}else{
		
			RetVal = E_NOT_OK;
	}
			
	return RetVal; 
}


static uint8_t IsReceivedDatafromESP(char *Rx_Buffer_c){
		uint8_t loopindexcp_u8, loopindex_u8;
		loopindexcp_u8 = 0;
		if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_c,0,ESPREADBUFF)){
				if(Rx_Buffer_c[0]=='+'&&Rx_Buffer_c[1]=='I'&&Rx_Buffer_c[2]=='P'&&Rx_Buffer_c[3]=='D'&&Rx_Buffer_c[4]==','){
					 ClearRxBuffer();//clear buffer
					 for(loopindex_u8=5; loopindex_u8<ESPREADBUFF;loopindex_u8++){
							Rx_Buffer_c[loopindexcp_u8++] = Rx_Buffer_c[loopindex_u8];
					 }
					 for(loopindex_u8=ESPREADBUFF-5; loopindex_u8<ESPREADBUFF;loopindex_u8++){
							Rx_Buffer_c[loopindex_u8] = 0;
						 
					 }
					 
					 return TRUE;
					 
				}else if(Rx_Buffer_c[0]=='>'){
						ClearRxBuffer();//clear buffer
						return TRUE;
				}
		}		
	  return FALSE;
}


uint8_t SendMessagetoESP(ESPDatadef_st Data, ESPDataSendSta_en EspSendSta){
	
	uint8_t datalenth_u8, portID;
	if(EspSendSta==WAITINGRES){
		Data.Datatype==CHAR? printf("AT+CIPSEND=%d,%d\r\n",0, strlen(Data.data)):printf("AT+CIPSEND=%d,%d\r\n",0, Data.Len);
	}
	else if(EspSendSta==READY){
	  printf("%s\r\n",Data.data);
	}
	return E_OK;
}

uint8_t SendMessagetoESPWithLen(uint8_t *data, uint8_t len){
	uint8_t datalenth_u8, portID, index;
	printf("AT+CIPSEND=%d,%d\r\n",0, len); 
	HAL_Delay(5);//5ms
	for (index=0; index<len; len++)
	printf("%d",data[index]);
	printf("\r\n"); 
	return E_OK;
}

uint8_t SaveESPInfoToNVM(void){
	//Infor: SSID, PASS, Faulty, ESP status...
	bl_fl_WriteByte2NVM((uint8_t*)&ESPInforConfg.SsId[0], ESPBASEADDR, sizeof(ESPInfor_st));
	
	return E_OK;
	
}

uint8_t ReadESPInforfromNVM(void){
		
	bl_fl_ReadbytefromNVM((uint8_t*)&ESPInforConfg.SsId[0],ESPBASEADDR,sizeof(ESPInfor_st));
	if((uint8_t)ESPInforConfg.SsId[0]==0xFF) return E_NOT_OK;
	return E_OK;
}






