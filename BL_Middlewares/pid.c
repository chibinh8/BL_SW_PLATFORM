#include "pid.h"
#include "adc.h"
#include "pwm.h"
#include "timer.h"

#ifdef USE_ARMMATH
/* Include ARM math */
#include "arm_math.h"
#endif

LineState * bl_adc_GetFinalSensorSta(void);
int16_t bl_pid_DeviationCal(void);

static int16_t bl_pid_SetpointCal(void);
const extern LineState FinalLineSensorState[NumofSensor];

typedef struct{
	int16_t SensorValMin_i16;
	int16_t SensorValMax_i16;
	uint8_t RCDuty_u8;	
}MapDutySSVal_st;

const MapDutySSVal_st bl_pid_MappingRCdutySSValTbl[1]={	
	{12,8,40},
};

typedef enum{
	PATTNOTVALID,
	PATTVALID,
	
}LinePatt_en;

#define SensorWeight   10u
#define CONSTRAINT(Val, Max, Min)   (Val>=Max?Max:((Val<Min)?Min:Val))
#define RCAngleMax									  90u
#define RCAngleMin										70u
#define KD_SAMPLERATE									1u //TASK is run in 5 ms 
#define SENSORPOS2ANGLEDUTYFAC				(float)(RCAngleMax-RCAngleMin)/24

int16_t bl_pid_SensorFactor[NumofSensor] = {8,4,2,1,-1,0,-4,-8};

uint8_t bl_pid_CenterRCDuty_u8;
/*Get current sensor line state*/

uint8_t bl_pid_CenterValFilterCnt_u8 = CENTERRCDUTY;

PIDInfor_st bl_PIDInfor_st = {1.0f, 2.0f, 0.001f};
PIDInfor_st PIDValCal_st;
int16_t bl_pid_LastErrorPID_i16 = 0;
int16_t bl_pid_ErrorPID_i16 = 0;
uint16_t bl_pid_RC_AngComValOut_u16;
float bl_pid_ControlPIDVal_fl;
PIDWorkSta_en bl_pid_PIDJobSta_en = IDLE;

static uint8_t bl_pid_PIDTuningSubSta_u8;

/* ARM PID Instance, float_32 format */
#ifdef USE_ARMMATH

arm_pid_instance_f32 PID;

#endif

void bl_pid_PIDControllerInit(void){
	
		bl_pid_CenterValFilterCnt_u8 = CENTERRCDUTY;
		bl_pid_CenterRCDuty_u8 = CENTERRCDUTY;
		bl_pid_RC_AngComValOut_u16 = CENTERRCDUTY;
		#ifdef USE_ARMMATH
				/* Set PID parameters */
			/* Set this for your needs */
			PID.Kp = PIDInfor_st.KP_fl;		/* Proporcional */
			PID.Ki = PIDInfor_st.KI_fl;		/* Integral */
			PID.Kd = PIDInfor_st.KD_fl;	/* Derivative */
			arm_pid_init_f32(&PID, 1);
	
		#else
			
		#endif
}
LinePatt_en bl_pid_SituationAnalysis(void){
	for(uint8_t i = 0; i<NumofSensor;i++)
		if(FinalLineSensorState[i]==BLACK)
				return PATTVALID;
		
	return PATTNOTVALID;	
}


void bl_pid_PIDTuning(void){
	
	;
}

static int16_t bl_pid_SetpointCal(void){
		/*for futher require of setpoint, just modify this api, curenty use zero*/
		return 0;
}

static inline uint16_t bl_pid_convertpid2servocontrolval(float PIDContrVal_fl){
	uint16_t duyrcout_u16;

	duyrcout_u16 = -((int16_t)PIDContrVal_fl*SENSORPOS2ANGLEDUTYFAC) + bl_pid_CenterRCDuty_u8;
	

	return duyrcout_u16;
}

uint16_t bl_pid_RCAngCal(void){
			
		 bl_pid_LastErrorPID_i16 = bl_pid_ErrorPID_i16;
		 bl_pid_ErrorPID_i16 = bl_pid_DeviationCal();

		 #ifdef USE_ARMMATH

			bl_pid_ControlPIDVal_fl = arm_pid_f32(&PID, pid_error);
			bl_pid_RC_AngComValOut_u16 =  bl_pid_convertpid2servocontrolval(bl_pid_ControlPIDVal_fl);
			bl_pid_RC_AngComValOut_u16 = CONSTRAINT(bl_pid_RC_AngComValOut_u16,RCAngleMax, RCAngleMin);
			
			#else
			if((FinalLineSensorState[3]==BLACK)&&(FinalLineSensorState[4]==BLACK)){
				bl_pid_CenterValFilterCnt_u8++;
				PIDValCal_st.KI_fl = 0;
				PIDValCal_st.KD_fl = 0;
				bl_pid_LastErrorPID_i16 = bl_pid_ErrorPID_i16;
			}else{
				bl_pid_CenterValFilterCnt_u8 = 0;
			}
			if(bl_pid_CenterValFilterCnt_u8>=1){ //filter threshold
				bl_pid_CenterRCDuty_u8 =bl_pid_RC_AngComValOut_u16;
				bl_pid_CenterValFilterCnt_u8 = 1;
		  }
			//P-D Controller
			PIDValCal_st.KP_fl = bl_PIDInfor_st.KP_fl*(float)bl_pid_ErrorPID_i16;
			PIDValCal_st.KD_fl = bl_PIDInfor_st.KD_fl*((float)(bl_pid_ErrorPID_i16-bl_pid_LastErrorPID_i16)/KD_SAMPLERATE);
			PIDValCal_st.KI_fl += bl_PIDInfor_st.KI_fl*bl_pid_ErrorPID_i16;
			bl_pid_ControlPIDVal_fl = (PIDValCal_st.KP_fl  + PIDValCal_st.KD_fl + PIDValCal_st.KI_fl);
			bl_pid_RC_AngComValOut_u16 =  bl_pid_convertpid2servocontrolval(bl_pid_ControlPIDVal_fl);
			bl_pid_RC_AngComValOut_u16 = CONSTRAINT(bl_pid_RC_AngComValOut_u16,MAXRCDUTY, MINRCDUTY);	
			
			#endif	

			return bl_pid_RC_AngComValOut_u16;	
} 

void bl_pid_ActionAfterPIDCtrl(uint32_t duty){
	   SetAngleRCServo(duty);
}

int16_t bl_pid_DeviationCal(void){
	 int16_t Setpoint_i16 = bl_pid_SetpointCal();
	 int16_t SensorWeighTotal_i16;
		SensorWeighTotal_i16 = 0;
	  /*Calculate weight of current sensor line*/
		for(uint8_t LoopIndex = 0; LoopIndex<NumofSensor;LoopIndex++){
			if((FinalLineSensorState[LoopIndex]==BLACK)&&(LoopIndex!=5)){//sensor 5 is not stable	
				SensorWeighTotal_i16 += (int16_t)(bl_pid_SensorFactor[LoopIndex]);			
			}		
	
		}
	return (int16_t)(SensorWeighTotal_i16-Setpoint_i16);
}

static uint32_t bl_pid_currtime_u32 = 0;

void bl_pid_FollowLineContrWithPIDCyclic(void){

		switch(bl_pid_PIDJobSta_en){
			case IDLE:
				GetCurrentTimestamp(&bl_pid_currtime_u32);	
				bl_pid_ActionAfterPIDCtrl(bl_pid_CenterRCDuty_u8);
				bl_pid_PIDJobSta_en = WAITFROMIDLE;
				break;
			case WAITFROMIDLE:
				if(TRUE==CheckTimestampElapsed(bl_pid_currtime_u32, (uint32_t)5000u)){
								bl_pid_PIDJobSta_en = NORMALCONTROL;
				}
				break;				
			case TUNINGPID:
				switch(bl_pid_PIDTuningSubSta_u8){
					
					case 0: 
						bl_pid_PIDTuning();
						if(bl_pid_SituationAnalysis()!=PATTNOTVALID)
							(void)bl_pid_RCAngCal();
						bl_pid_ActionAfterPIDCtrl(bl_pid_RC_AngComValOut_u16);
						GetCurrentTimestamp(&bl_pid_currtime_u32);
						break;
					case 1: //Erase NVM 
						if(TRUE==CheckTimestampElapsed(bl_pid_currtime_u32, (uint32_t)200u)){
								
						}
						break;
				}

			
				break;
			case NORMALCONTROL:
				if(bl_pid_SituationAnalysis()!=PATTNOTVALID)
				(void)bl_pid_RCAngCal();
			
				break;
			default:
				
				break;
			
		}
}



uint16_t bl_pid_GetRCContrVal(void){
	return bl_pid_RC_AngComValOut_u16;
}