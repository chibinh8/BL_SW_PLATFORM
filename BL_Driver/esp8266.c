#ifndef BL_ESP_H
#define BL_ESP_H 

#include "esp8266.h"
#include "uart.h"
#include "string.h"

#define ESPREADBUFF (BL_BUFFSIZE>>1)
extern UART_HandleTypeDef BL_UART;
static char Rx_Buffer_cp[ESPREADBUFF];

uint8_t InitESp8266(void){
	memset(Rx_Buffer_cp,0,ESPREADBUFF);
	printf("AT+RST\r\n");
	HAL_Delay(2000);
	ClearRxBuffer();
	printf("AT\r\n");
	HAL_Delay(2000);
	if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_cp,1,ESPREADBUFF)){
			//if((Rx_Buffer_cp[0]=='O')&&(Rx_Buffer_cp[1]=='K')){
		if(Rx_Buffer_cp[0]==0x41){
				printf("AT+CWSAP=\"Binh Le Chi\",\"password\",3,0\r\n");
				HAL_Delay(1000);
				printf("AT+CWMODE=3\r\n");
				HAL_Delay(500);
				ClearRxBuffer();//clear buffer
				return E_OK;
		}				
	}	
  return E_NOT_OK;
	
}

#endif

