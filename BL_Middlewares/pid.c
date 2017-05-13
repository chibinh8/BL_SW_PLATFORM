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



#define SensorWeight   10u
#define CONSTRAINT(Val, Max, Min)   (Val>=Max?Max:((Val<Min)?Min:Val))
#define RCAngleMax									  115u
#define RCAngleMin										40u
#define KD_SAMPLERATE									500u //TASK is run in 5 ms 
#define SENSORPOS2ANGLEDUTYFAC				100.0f

int16_t bl_pid_SensorFactor[NumofSensor] = {4,3,2,1,-1,-2,-3,-4};

PIDInfor_st bl_PIDInfor_st = {1.0f, 2.0f, 0.0f};
PIDInfor_st PIDValCal_st;
int16_t bl_pid_LastErrorPID_i16 = 0;
int16_t bl_pid_ErrorPID_i16;
uint16_t bl_pid_RC_AngComValOut_u16;
float bl_pid_ControlPIDVal_fl;
PIDWorkSta_en bl_pid_PIDJobSta_en = IDLE;

static uint8_t bl_pid_PIDTuningSubSta_u8 =0;

/* ARM PID Instance, float_32 format */
#ifdef USE_ARMMATH

arm_pid_instance_f32 PID;

#endif

void bl_pid_PIDControllerInit(void){
	
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

void bl_pid_PIDTuning(void){
	
	;
}

static int16_t bl_pid_SetpointCal(void){
		/*for futher require of setpoint, just modify this api, curenty use zero*/
		return 0;
}

static inline uint16_t bl_pid_convertpid2servocontrolval(float PIDContrVal_fl){
	uint16_t duyrcout_u16;
	if(PIDContrVal_fl>=0){
		duyrcout_u16 = (uint16_t)PIDContrVal_fl*SENSORPOS2ANGLEDUTYFAC;
	}
	else{
		duyrcout_u16 = (uint16_t)((-1.0f)*PIDContrVal_fl*SENSORPOS2ANGLEDUTYFAC);
	}
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
			//P-D Controller
			PIDValCal_st.KP_fl = bl_PIDInfor_st.KP_fl*(float)bl_pid_ErrorPID_i16;
			PIDValCal_st.KD_fl = bl_PIDInfor_st.KD_fl*((float)(bl_pid_ErrorPID_i16-bl_pid_LastErrorPID_i16)/KD_SAMPLERATE);
			bl_pid_ControlPIDVal_fl = (PIDValCal_st.KI_fl + PIDValCal_st.KD_fl + PIDValCal_st.KI_fl);
			bl_pid_RC_AngComValOut_u16 =  bl_pid_convertpid2servocontrolval(bl_pid_ControlPIDVal_fl);
			//bl_pid_RC_AngComValOut_u16 = CONSTRAINT(bl_pid_RC_AngComValOut_u16,RCAngleMax, RCAngleMin);	
			
			#endif	

			return bl_pid_RC_AngComValOut_u16;	
} 

void bl_pid_ActionAfterPIDCtrl(uint32_t duty){
	   SetAngleRCServo(duty);
}

int16_t bl_pid_DeviationCal(void){
	 int16_t Setpoint_i16 = bl_pid_SetpointCal();
	 int16_t SensorWeighTotal_i16;
	 LineState *FinalLineSensorSta;
	 /*Get current sensor line state*/
		FinalLineSensorSta = bl_adc_GetFinalSensorSta();
	  /*Calculate weight of current sensor line*/
		for(uint8_t LoopIndex = 0; LoopIndex<NumofSensor;LoopIndex++){
			SensorWeighTotal_i16 += (int16_t)(bl_pid_SensorFactor[LoopIndex]*((uint8_t)(*(FinalLineSensorSta+LoopIndex))));			
		}
		return (int16_t)(SensorWeighTotal_i16-Setpoint_i16);
}

void bl_pid_FollowLineContrWithPIDCyclic(void){
		static uint32_t bl_pid_currtime_u32 = 0;
		switch(bl_pid_PIDJobSta_en){
			case IDLE:
				
				break;
			case TUNINGPID:
				switch(bl_pid_PIDTuningSubSta_u8){
					
					case 0: 
						bl_pid_PIDTuning();
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
				(void)bl_pid_RCAngCal();
			
				break;
			default:
				
				break;
			
		}
}



uint16_t bl_pid_GetRCContrVal(void){
	return bl_pid_RC_AngComValOut_u16;
}