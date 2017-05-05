#include "bl_rc_AutoRacer.h"
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

RacerPatternSta_en bl_rc_SituationAnalysis(void){
	
	
	return IDLE;
}

RacerPatternSta_en bl_rc_RacerPatternWorkStat_en  = IDLE;
void bl_rc_AutoRacerInit(void){
	bl_rc_RacerPatternWorkStat_en = IDLE;
	
}
void bl_rc_AutoRacerCyclic(void){
	
	bl_rc_RacerPatternWorkStat_en = bl_rc_SituationAnalysis();
	
	switch(bl_rc_RacerPatternWorkStat_en){
		case IDLE:
			
			break;
		
		case FOLLOWLINE:
			bl_pid_ActionAfterPIDCtrl(bl_pid_GetRCContrVal());
		
			break;
		
		default:
			
			break;		
	}	
	
	
}