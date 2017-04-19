#ifndef BL_ESP_H
#define BL_ESP_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 


#define SSID    "BLPRO"
#define PASS		"123456"

typedef enum{
		WAITING,
		STATE1,
		STATE2,
		STATE3,	
}Recheckstate_en;

typedef struct{
	uint8_t Header_u8;
	char Status_c[10];
	uint8_t Data[25];
	
}ESPMessage_st;


uint8_t InitESp8266(void);

void ESPOperationCyclic(void);

uint8_t SendMessagetoESP(uint8_t *data, uint8_t len);

#endif
