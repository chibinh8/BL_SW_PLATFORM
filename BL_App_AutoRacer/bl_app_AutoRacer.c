#include "bl_app_AutoRacer.h"

#include "pid.h"
#include "timer.h"

typedef enum{
	
	RC_IDLE,
	FOLLOWLINE,
	RIGHTSTEERING,
	LEFTSTEERING,
	BLINDNESSRUN,
	OUTOFROAD,
  SYSTEMFAILURE_STOP
	
}RacerPatternSta_en;

RacerPatternSta_en bl_app_SituationAnalysis(void){
	
	
	return RC_IDLE;
}

RacerPatternSta_en bl_app_RacerPatternWorkStat_en  = RC_IDLE;
void bl_app_AutoRacerInit(void){
	bl_app_RacerPatternWorkStat_en = RC_IDLE;
	
}
uint32_t bl_rc_currtime_u32;
void bl_app_AutoRacerCyclic(void){
	
	bl_app_RacerPatternWorkStat_en = bl_app_SituationAnalysis();
	
	switch(bl_app_RacerPatternWorkStat_en){
		case IDLE:
			GetCurrentTimestamp(&bl_rc_currtime_u32);
			if(TRUE==CheckTimestampElapsed(bl_rc_currtime_u32, (uint32_t)1000)){
					bl_app_RacerPatternWorkStat_en = FOLLOWLINE;
					GetCurrentTimestamp(&bl_rc_currtime_u32);
			}
			break;
		
		case FOLLOWLINE:
			if(TRUE==CheckTimestampElapsed(bl_rc_currtime_u32, (uint32_t)10)){
				bl_pid_ActionAfterPIDCtrl(bl_pid_GetRCContrVal());
				GetCurrentTimestamp(&bl_rc_currtime_u32);
			}
		
			break;
		
		default:
			
			break;		
	}	
	
	
}