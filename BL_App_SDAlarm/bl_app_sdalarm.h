#ifndef BL_APP_ALARM_H
#define BL_APP_ALARM_H

#include "BL_Define.h"


uint8_t CopyRXDataESPClbkSDAlarm(char* RXbuffer);

typedef enum{
	
	INIT,
	CYCLIC,
	
}AlarmSta_en;

#endif