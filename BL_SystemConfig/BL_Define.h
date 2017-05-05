#ifndef BL_DEFINE_H
#define BL_DEFINE_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

typedef enum{
	
	FALSE =0,
	TRUE = 1

}BOOL;

#define DEBUG_ESP
#define DIAGRESPONSE 1


#define  BL_BUFFSIZE   50
#define  ESPREADBUFF   50
#define  ALARMBUFF		 ESPREADBUFF	


#ifndef RETSTATE
#define RETSTATE

typedef enum{
	E_NOT_OK,	
	E_OK
}retstate;


typedef enum{
		UINT8,
		SINT8,
		UINT16,
		SINT16,
		STRING,
		CHAR,	
}Datatype_en;

#endif 

#endif

