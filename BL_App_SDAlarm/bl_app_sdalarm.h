#ifndef BL_APP_ALARM_H
#define BL_APP_ALARM_H

#include "BL_Define.h"


uint8_t CopyRXDataESPClbkSDAlarm(char* RXbuffer);

typedef enum{	
	
	ALARM_IDLE,
	
	ALARM_CYCLIC,
	
}AlarmSta_en;

typedef enum{
	TASKMODE_IDLE,
	REQUESTDEVICESTA,
	REMOTEDEVICE,
	HARDRESET,
	SCHEDULE,
	WIFICONFIG,
	SYSTEMDATACONFIG
	
}AlarmTaskMode_en;


typedef struct{
	BOOL IsESPDatReceived_bo;
	AlarmSta_en Mainstate;
	AlarmTaskMode_en TaskMode;
	uint8_t ConfirmAlarm2App[3];
	char AlarmESPData[ALARMBUFF]; //25 bytes currently	
}AlarmTasks_st;

void bl_al_AlarmInit(void);

#endif