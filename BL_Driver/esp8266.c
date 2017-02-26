#ifndef BL_ESP_H
#define BL_ESP_H 

#include "esp8266.h"
#include "uart.h"
#include "string.h"
#include "timer.h"

#define ESPREADBUFF (BL_BUFFSIZE>>1)
extern UART_HandleTypeDef BL_UART;
static char Rx_Buffer_cp[ESPREADBUFF];
uint8_t checkespcall = FALSE;
uint8_t InitESp8266(void){
	uint32_t timecurr_u32 =0;
	memset(Rx_Buffer_cp,0,ESPREADBUFF);
	ClearRxBuffer();
  char ch = 'A';
	HAL_UART_Transmit(&BL_UART, (uint8_t *)&ch, 1, 10);
	ch = 'T';
	HAL_UART_Transmit(&BL_UART, (uint8_t *)&ch, 1, 10);
	ch = '\r';
	HAL_UART_Transmit(&BL_UART, (uint8_t *)&ch, 1, 10);
	ch = '\n';
	HAL_UART_Transmit(&BL_UART, (uint8_t *)&ch, 1, 10);
	//printf("AT\r\n");
  checkespcall = TRUE;
	//GetCurrentTimestamp(&timecurr_u32);
	//while(!CheckTimestampElapsed(timecurr_u32, 500));
	HAL_Delay(500);

	if(TRUE==GetDataRXcomplete(&BL_UART,Rx_Buffer_cp,0,ESPREADBUFF)){
			if((Rx_Buffer_cp[0]=='O')&&(Rx_Buffer_cp[1]=='K')){
				printf("AT+CWSAP=\"Binh Le Chi\",\"password\",3,0\r\n");
				//HAL_Delay(1000);
				printf("AT+CWMODE=3\r\n");
				//HAL_Delay(500);
				ClearRxBuffer();//clear buffer
				return E_OK;
		}				
	}	
	checkespcall = FALSE;
  return E_NOT_OK;
	
}

#endif

