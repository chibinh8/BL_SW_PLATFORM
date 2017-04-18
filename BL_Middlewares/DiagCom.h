#ifndef BL_DIAGCOM_H
#define BL_DIAGCOM_H 

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

#define NumofDID 3
#define MINSIZECOMMAND 11
#define MINSIZE 2
#define MAXDIDNUM 99


void ProcessDiagserviceCyclicMain(void);

uint8_t CopyRXDataESPClbkDiag(char* RXbuffer);

#endif

