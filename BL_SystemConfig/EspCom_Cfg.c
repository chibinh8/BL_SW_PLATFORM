#ifndef BL_ESPCOM_CONFIG_H
#define BL_ESPCOM_CONFIG_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

#include "bl_app_clock.h"
#include "DiagCom.h"

uint8_t CopyRXDataESPClbkSDAlarm(char* RXbuffer);
uint8_t CopyRXDataESPClbkDiag(char* RXbuffer);

uint8_t (*CopyRXDataESPClbk[]) (char* RXbuffer) = {&CopyRXDataESPClbkSDAlarm,&CopyRXDataESPClbkDiag};


#endif 

