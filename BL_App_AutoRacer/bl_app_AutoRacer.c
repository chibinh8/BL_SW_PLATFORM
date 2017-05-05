#include "bl_app_AutoRacer.h"

#include "pid.h"

typedef enum{
	
	IDLE,
	FOLLOWLINE,
	RIGHTSTEERING,
	LEFTSTEERING,
	BLINDNESSRUN,
	OUTOFROAD,
  SYSTEMFAILURE_STOP
	
}RacerPatternSta_en;

RacerPatternSta_en bl_app_SituationAnalysis(void){
	
	
	return IDLE;
}

RacerPatternSta_en bl_app_RacerPatternWorkStat_en  = IDLE;
void bl_app_AutoRacerInit(void){
	bl_app_RacerPatternWorkStat_en = IDLE;
	
}
void bl_app_AutoRacerCyclic(void){
	
	bl_app_RacerPatternWorkStat_en = bl_app_SituationAnalysis();
	
	switch(bl_app_RacerPatternWorkStat_en){
		case IDLE:
			
			break;
		
		case FOLLOWLINE:
			bl_pid_ActionAfterPIDCtrl(bl_pid_GetRCContrVal());
		
			break;
		
		default:
			
			break;		
	}	
	
	
}