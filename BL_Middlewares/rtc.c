
#include <stdio.h>
#include <string.h>
#include "esp8266.h"
#include "library.h"
#include "BL_Ds1307.h"
#include "task.h"

extern TASK Task_st;
extern ESP8266 ModuleWifi_st;
extern bl_rtc_time RTCTime_st;
extern int count,flag;
extern unsigned char LasPos,l_deviceState_ub[5];
extern void Linked_SendAfterPressingSwitch_V(void);

unsigned char l_rtcgettingTimeDone_ub=E_NOT_OK, l_FlagStt_ub=0, taskCnt=0;
rtc_scheduler rtc_st;
rtc_scheduler begin_time_st;
rtc_scheduler end_time_st;

/////////////////////////////////using internal delay with frequency of MCU
#define DELAY_2N	12
void intdelay (uint32_t cnt)
{
  cnt <<= DELAY_2N;

  while (cnt--);
}

static
void delay_us(uint32_t cnt)
{
  cnt <<= 6;

  while (cnt--);
}

/*
** Function name: RTC_DataChecking_V
** Descriptions: using to checking data of RTC
** Company: SDVietNam
*/
static
unsigned char RTC_DataChecking_V(unsigned char sel,unsigned char *buf,int numOfTask){
	
	uint8_t t_check_ub=0,t_stt_ub=0;
	
	t_check_ub = APP_VALID_BYTE;
	t_stt_ub = E_OK;
	
	if(sel==START_TIMSET_BYTE){
		
		//setting time checking: day/mon/yea/hou:min:sec
		if(buf[10]>31 || buf[11]>12 || buf[12]<15 || buf[13]>=24 || buf[14]>=60 || buf[15]>=60){
		
			t_check_ub = APP_INVALID_BYTE;
			t_stt_ub = E_NOT_OK;
		}
	}else if(sel==START_TIMSCHE_BYTE){
		
		//schedule time checking for begin date: day/mon/yea/hou:min:sec
		if(buf[numOfTask+0]>31 || buf[numOfTask+1]>12 || buf[numOfTask+2]<15 || buf[numOfTask+3]>=24 || buf[numOfTask+4]>=60){
		
			t_check_ub = APP_INVALID_BYTE;
			t_stt_ub = E_NOT_OK;
		}
		
		//schedule time checking for begin date: day/mon/yea/hou:min:sec
		if(buf[numOfTask+5]>31 || buf[numOfTask+6]>12 || buf[numOfTask+7]<15 || buf[numOfTask+8]>=24 || buf[numOfTask+9]>=60){
		
			t_check_ub = APP_INVALID_BYTE;
			t_stt_ub = E_NOT_OK;
		}
		
	}else if (sel==EVEDAY_TIMSCHE_BYTE){
	
		//schedule time checking for begin date: day/mon/yea/hou:min:sec
		if(buf[numOfTask+0]>31 || buf[numOfTask+1]>12 || buf[numOfTask+2]<15 || buf[numOfTask+3]>=24 || buf[numOfTask+4]>=60){
		
			t_check_ub = APP_INVALID_BYTE;
			t_stt_ub = E_NOT_OK;
		}
		
		//schedule time checking for begin date: day/mon/yea/hou:min:sec
		if(buf[numOfTask+5]!=0x00 || buf[numOfTask+6]!=0x00 || buf[numOfTask+7]!=0x00 || buf[numOfTask+8]>=24 || buf[numOfTask+9]>=60){
		
			t_check_ub = APP_INVALID_BYTE;
			t_stt_ub = E_NOT_OK;
		}
	}else{
		
		//do nothing else
	}
	
	ModuleWifi_st.l_datasend_ub[1]=t_check_ub;
	ModuleWifi_st.l_datasend_ub[2]=t_check_ub;
	return t_stt_ub;
}

/*
** Function name: ESP8266_RTCTimeSetting_V
** Descriptions: using to setting RTC timer
** Company: SDVietNam
*/
static
void ESP8266_RTCTimeSetting_V(void){
	
	//processing: RTC scheduler setting
	if(flag == E_OK && ModuleWifi_st.l_buffer_ub[9]==START_TIMSET_BYTE){
		int i;
				
		printf("AT+CIPSEND=%c,3\r",ModuleWifi_st.l_IDport_ub[count]);				
		ModuleWifi_st.l_datasend_ub[0]=APP_TIMSET_BYTE;
		
		delay_us(1);//do not erase
		
		ModuleWifi_st.l_checkSaveData_ub = E_OK;		
		ModuleWifi_st.l_timset_flag_ub = E_NOT_OK;
		
		for(i=10;i<16;i++){
			
			ModuleWifi_st.l_buffer_ub[i] -= APP_MINUS_BYTE;
		}
		
		//checking invalid of data or not
		ModuleWifi_st.l_timset_flag_ub = RTC_DataChecking_V(START_TIMSET_BYTE,ModuleWifi_st.l_buffer_ub,7);
		
		ModuleWifi_st.l_Diagnostic_ub[E_CONF] = E_OK;		
		flag = E_NOT_OK;
	}
}

/*
** Function name: RTC_TimeSetting_V
** Descriptions: using to setting scheduler timer
** Company: SDVietNam
*/
static
void RTC_TimeSetting_V(void){

	unsigned char date, mon, yea, day=0, hou, min, sec;
		
	if(ModuleWifi_st.l_timset_flag_ub == E_OK){
	
		date = ModuleWifi_st.l_buffer_ub[10];
		mon = ModuleWifi_st.l_buffer_ub[11];
		yea = ModuleWifi_st.l_buffer_ub[12];
		hou = ModuleWifi_st.l_buffer_ub[13];
		min = ModuleWifi_st.l_buffer_ub[14];
		sec = ModuleWifi_st.l_buffer_ub[15];
		
		DS1307_TimeSetting_UB(date, mon, yea, day, hou, min, sec);
		
		ModuleWifi_st.l_timset_flag_ub = E_NOT_OK;
	}
}

/*
** Function name: RTC_TaskProcessing_V
** Descriptions: using to processing task
** Company: SDVietNam
*/
unsigned char l_deleteTask=0;
static
void RTC_TaskProcessing_V(void){
		
	//processing: getting RTC scheduler
	if(flag == E_OK && ModuleWifi_st.l_buffer_ub[LasPos+1]==START_TIMSCHE_BYTE){
		int i,j,k,numOfTask=0;
		unsigned char evday=0;
		
		/////////////////data from scheduler buffer will have following type:
		/////////////////start byte | scheduler timer | device state 1 | device state 2 = length
		/////////////////length of scheduler timer = length - 3			
		ModuleWifi_st.l_timsche_flag_ub = E_NOT_OK;
		ModuleWifi_st.l_buffer_ub[LasPos+1] = 0x00;		
		ModuleWifi_st.l_checkSaveData_ub = E_OK;
				
		numOfTask = (uint8_t)(ModuleWifi_st.l_length_ul/12);//number of tasks
						
		intdelay(1);//do not erase
		
		if(numOfTask>MAX_NUM_TASK){
			
			ModuleWifi_st.l_datasend_ub[1] = APP_INVALID_BYTE;
			ModuleWifi_st.l_datasend_ub[2] = APP_INVALID_BYTE;
		}else{
						
			int t_count=0;		
			intdelay(1);//do not erase
			
			//this function is delete at 1 last task
			if(numOfTask==1){
				
				for(i=0;i<12;i++){
					
					if(ModuleWifi_st.l_buffer_ub[(LasPos+2+i)]==0x00){
						rtc_st.Buffer[i] = 0x00;
						t_count++;
					}
				}
			}
			//when number of data > 10 --> delete function
			if(t_count>10){				
				for(i=0;i<12;i++){
					rtc_st.Buffer[i] = 0x00;
				}	
				rtc_st.Device1[0] = 0x00;
				rtc_st.Device2[0] = 0x00;
				l_deleteTask = 1;
				ModuleWifi_st.l_datasend_ub[1] = APP_VALID_BYTE;
				ModuleWifi_st.l_datasend_ub[2] = APP_VALID_BYTE;
			}else{
			
				k = 0;
				l_deleteTask = 0;
				for(i=0;i<numOfTask;i++){
				
					k = i*12;
					
					evday = 0;			
					evday = (uint8_t)(ModuleWifi_st.l_buffer_ub[k+0+(LasPos+2)]>>5);//getting everyday bit at position 5: xx1xxxxx
					
					//everyday mode --> begin date: dd/mm/ye:ho:mi --> end date: 00/00/00: ho:mi
					if(evday==5){
					
						//save all data with maximum = 1KB
						rtc_st.Buffer[(i*10)+0] = ModuleWifi_st.l_buffer_ub[k+0+(LasPos+2)] - APP_MINUS_EVEDAY_BYTE;
						
						for(j=1;j<5;j++){
							
							//save all data with maximum = 1KB
							rtc_st.Buffer[(i*10)+j] = ModuleWifi_st.l_buffer_ub[k+j+(LasPos+2)] - APP_MINUS_BYTE;//dd/mm/ye:ho:mi
						}
						
						for(j=5;j<8;j++){
							
							//save all data with maximum = 1KB
							rtc_st.Buffer[(i*10)+j] = ModuleWifi_st.l_buffer_ub[k+j+(LasPos+2)];//0x00/0x00/0x00
						}
						
						for(j=8;j<10;j++){
							
							//save all data with maximum = 1KB
							rtc_st.Buffer[(i*10)+j] = ModuleWifi_st.l_buffer_ub[k+j+(LasPos+2)] - APP_MINUS_BYTE;//ho:mi
						}
						
						intdelay(1);//do not erase
						
						//checking invalid of data or not
						ModuleWifi_st.l_timsche_flag_ub = RTC_DataChecking_V(EVEDAY_TIMSCHE_BYTE,rtc_st.Buffer,10*i);
						rtc_st.EveryDay_ub[i] = E_OK;
						
					}else	{//normal mode
						
						for(j=0;j<10;j++){
							
							//save all data with maximum = 1KB
							rtc_st.Buffer[(i*10)+j] = ModuleWifi_st.l_buffer_ub[k+j+(LasPos+2)] - APP_MINUS_BYTE;
						}
						
						intdelay(1);//do not erase
						
						//checking invalid of data or not
						ModuleWifi_st.l_timsche_flag_ub = RTC_DataChecking_V(START_TIMSCHE_BYTE,rtc_st.Buffer,10*i);
						rtc_st.EveryDay_ub[i] = E_NOT_OK;
					
					}
					
					//saving data of devices
					rtc_st.Device1[i] = ModuleWifi_st.l_buffer_ub[k+(LasPos+2)+10];
					rtc_st.Device2[i] = ModuleWifi_st.l_buffer_ub[k+(LasPos+2)+11];
				}
			}
		}
		
		intdelay(1);//do not erase
						
		printf("AT+CIPSEND=%c,3\r",ModuleWifi_st.l_IDport_ub[count]);				
		ModuleWifi_st.l_datasend_ub[0] = APP_TIMSCHE_BYTE;
		
		delay_us(1);//do not erase
				
		rtc_st.TaskNum_ub = numOfTask;
		ModuleWifi_st.l_Diagnostic_ub[E_CONF] = E_OK;
		//reset value
		flag = E_NOT_OK;
	}
}

/*
** Function name: RTC_TimeDataGetting_V
** Descriptions: using to solve timer of task after receiving data from wifi module
** Company: SDVietNam
*/
static
void RTC_TimeDataGetting_V(void){

	int i,numOfTask;
	
	numOfTask = rtc_st.TaskNum_ub;
	
	if(ModuleWifi_st.l_Diagnostic_ub[E_CONF]==E_NOT_OK && ModuleWifi_st.l_timsche_flag_ub==E_OK){
		unsigned long saveinfo_ul[40]={0x00000000};
		unsigned char startbyte=0xA3;
		
		intdelay(1);//do not erase
		
		for(i=0;i<numOfTask;i++){
			
				begin_time_st.Date[i] = rtc_st.Buffer[i*10 + 0];
				begin_time_st.Month[i] = rtc_st.Buffer[i*10 + 1];
				begin_time_st.Year[i] = rtc_st.Buffer[i*10 + 2];
				begin_time_st.Hour[i] = rtc_st.Buffer[i*10 + 3];
				begin_time_st.Min[i] = rtc_st.Buffer[i*10 + 4];
				
				end_time_st.Date[i] = rtc_st.Buffer[i*10 + 5];
				end_time_st.Month[i] = rtc_st.Buffer[i*10 + 6];
				end_time_st.Year[i] = rtc_st.Buffer[i*10 + 7];
				end_time_st.Hour[i] = rtc_st.Buffer[i*10 + 8];
				end_time_st.Min[i] = rtc_st.Buffer[i*10 + 9];
			
				saveinfo_ul[4*i+0] = (uint32_t)((begin_time_st.Hour[i]<<24)|(begin_time_st.Year[i]<<16)|(begin_time_st.Month[i]<<8)|(begin_time_st.Date[i]));
				saveinfo_ul[4*i+1] = (uint32_t)((end_time_st.Year[i]<<24)|(end_time_st.Month[i]<<16)|(end_time_st.Date[i]<<8)|(begin_time_st.Min[i]));
				saveinfo_ul[4*i+2] = (uint32_t)(((rtc_st.Device2[i])<<24)|(rtc_st.Device1[i]<<16)|(end_time_st.Min[i]<<8)|(end_time_st.Hour[i]));
				saveinfo_ul[4*i+3] = (uint32_t)(((startbyte)<<24)|(numOfTask<<16)|(0x00<<8)|(rtc_st.EveryDay_ub[i]));
			
				intdelay(1);//do not erase
		}
		
		delay(1);//1s
		Flash_ManyInfoDataSave(FLASH_TIME_SAVE_ADDR,saveinfo_ul,4*numOfTask);
		delay(2);//2s
		
		l_rtcgettingTimeDone_ub = E_OK;
		ModuleWifi_st.l_timsche_flag_ub = E_NOT_OK;
	}
}

/*
** Function name: RTC_LoadInitTime_V
** Descriptions: using to load all initial time from application
** Company: SDVietNam
*/
void RTC_LoadInitTime_V(void){

	unsigned char t_flagSave_ub=0,t_numTask_ub=0;
	
	t_flagSave_ub = Flash_Load(FLASH_TIME_SAVE_ADDR+15);
	
	if(t_flagSave_ub==0xA3){
		int i;
		
		t_numTask_ub = Flash_Load(FLASH_TIME_SAVE_ADDR+14);
		rtc_st.TaskNum_ub = t_numTask_ub;
		
		for(i=0;i<t_numTask_ub;i++){
			
			begin_time_st.Date[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+0));
			begin_time_st.Month[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+1));
			begin_time_st.Year[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+2));
			begin_time_st.Hour[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+3));
			begin_time_st.Min[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+4));
			
			end_time_st.Date[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+5));
			end_time_st.Month[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+6));
			end_time_st.Year[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+7));
			end_time_st.Hour[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+8));
			end_time_st.Min[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+9));
			
			rtc_st.Device1[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+10));
			rtc_st.Device2[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+11));
			
			rtc_st.EveryDay_ub[i] = Flash_Load(FLASH_TIME_SAVE_ADDR+(i*16+12));
		}
		
		l_rtcgettingTimeDone_ub = E_OK;
	}
}

/*
** Function name: RTC_DeviceProcessing_UB
** Descriptions: using to processing all scheduler data to control device
** Company: SDVietNam
*/
static
unsigned char RTC_DeviceProcessing_UB(unsigned char oldstt){

	int i,numOfTask,stt=0;
	unsigned char devBuf[3],t_rtcFlag_ub=0,t_Flag_ub=0,t_Flag1_ub=0;

	numOfTask = rtc_st.TaskNum_ub;
	l_deviceState_ub[0] = 0x01;
			
	stt = 1;
	intdelay(1);//do not erase
	
	if(l_deleteTask==0){
		
		for(i=0;i<numOfTask;i++){
			
			if(rtc_st.EveryDay_ub[i]==E_OK){
							
				//begin time processing
				if(begin_time_st.Date[i]<=RTCTime_st.Date && begin_time_st.Month[i]<=RTCTime_st.Month && begin_time_st.Year[i]<=RTCTime_st.Year){
					
					t_rtcFlag_ub = 1;
				}
				
				if(t_rtcFlag_ub==1 && \
					((begin_time_st.Hour[i]<RTCTime_st.Hours) || \
						((begin_time_st.Hour[i]==RTCTime_st.Hours) && (begin_time_st.Min[i]<=RTCTime_st.Minute))) && \
					((end_time_st.Hour[i]>RTCTime_st.Hours) || \
						((end_time_st.Hour[i]==RTCTime_st.Hours) && (end_time_st.Min[i]>RTCTime_st.Minute)))){
				
					//setting state of 2 devices: start with state setting on task
					devBuf[1] = rtc_st.Device1[i];
					devBuf[2] = rtc_st.Device2[i];
											
					l_deviceState_ub[0] = 0xA3;
					l_deviceState_ub[3] = devBuf[1]+APP_ADD_BYTE;
					l_deviceState_ub[4] = devBuf[2]+APP_ADD_BYTE;
					ModuleWifi_st.l_checkSaveData_ub = E_OK+1;
					
					Device_StartControl_V(devBuf);
					stt = 2;
							
					if(stt==oldstt && taskCnt==i)
						stt=0;
					else
						Linked_SendAfterPressingSwitch_V();
					
					taskCnt = i;
					return stt;
				}
				
				intdelay(1);//do not erase		
			}
		}

		for(i=0;i<numOfTask;i++){
			
			if(rtc_st.EveryDay_ub[i]!=E_OK){
				
				t_Flag_ub = 2;//false
				t_Flag1_ub = 2;//false
				
				if((begin_time_st.Year[i]<RTCTime_st.Year) || \
					((begin_time_st.Year[i]==RTCTime_st.Year) && (begin_time_st.Month[i]<RTCTime_st.Month)) || \
					((begin_time_st.Year[i]==RTCTime_st.Year) && (begin_time_st.Month[i]==RTCTime_st.Month) && (begin_time_st.Date[i]<RTCTime_st.Date))){
							
					t_Flag_ub = 1;//true
				}
					
				if((begin_time_st.Year[i]==RTCTime_st.Year) && (begin_time_st.Month[i]==RTCTime_st.Month) && (begin_time_st.Date[i]==RTCTime_st.Date) && \
						((begin_time_st.Hour[i]<RTCTime_st.Hours) || \
						((begin_time_st.Hour[i]==RTCTime_st.Hours) && (begin_time_st.Min[i]<=RTCTime_st.Minute)))){
					
					t_Flag_ub = 1;//true
				}
				
				if((end_time_st.Year[i]>RTCTime_st.Year) || \
					((end_time_st.Year[i]==RTCTime_st.Year) && (end_time_st.Month[i]>RTCTime_st.Month)) || \
					((end_time_st.Year[i]==RTCTime_st.Year) && (end_time_st.Month[i]==RTCTime_st.Month)&&(end_time_st.Date[i]>RTCTime_st.Date))){
							
					t_Flag1_ub = 1;//true
				}
					
				if((end_time_st.Year[i]==RTCTime_st.Year) && (end_time_st.Month[i]==RTCTime_st.Month) && (end_time_st.Date[i]==RTCTime_st.Date) && \
					((end_time_st.Hour[i]>RTCTime_st.Hours) || \
					((end_time_st.Hour[i]==RTCTime_st.Hours) && (end_time_st.Min[i]>RTCTime_st.Minute)))){
				
					t_Flag1_ub = 1;//true
				}
								
				if((t_Flag_ub==1) && \
					(t_Flag1_ub==1)){
				
					//setting state of 2 devices: start with state setting on task
					devBuf[1] = rtc_st.Device1[i];
					devBuf[2] = rtc_st.Device2[i];
											
					l_deviceState_ub[0] = 0xA3;
					l_deviceState_ub[3] = devBuf[1]+APP_ADD_BYTE;
					l_deviceState_ub[4] = devBuf[2]+APP_ADD_BYTE;
					ModuleWifi_st.l_checkSaveData_ub = E_OK+1;
					
					Device_StartControl_V(devBuf);					
					stt = 4;
							
					if(stt==oldstt && taskCnt==i)
						stt=0;
					else
						Linked_SendAfterPressingSwitch_V();
					
					taskCnt = i;
					return stt;
				}
				
				intdelay(1);//do not erase
			}
		}
	}
	
	if(stt==1){
		
		//setting state of 2 devices: restart with state setting at control command
		devBuf[1] = l_deviceState_ub[1]-APP_ADD_BYTE;
		devBuf[2] = l_deviceState_ub[2]-APP_ADD_BYTE;
		
		l_deviceState_ub[0] = 0x01;
		ModuleWifi_st.l_checkSaveData_ub = E_OK;
		
		Device_StartControl_V(devBuf);
		
		stt = 5;
						
		if(stt==oldstt)
			stt=0;
		else
			Linked_SendAfterPressingSwitch_V();
	}
	
	return stt;
}

/*
** Function name: RTC_DataProcessing_V
** Descriptions: using to processing all data from alarm/scheduler type
** Company: SDVietNam
*/
void RTC_DataProcessing_V(void){
	
	ESP8266_RTCTimeSetting_V();//calling hw RTC time setting function
	
	RTC_TimeSetting_V();//calling scheduler time setting function
	
	RTC_TaskProcessing_V();//calling task processing function
	
	RTC_TimeDataGetting_V();//calling time data getting function
		
	//checking condition of control device to control device if matched the time
	if(ModuleWifi_st.l_ControlDeviceFlag_ub==E_OK && l_rtcgettingTimeDone_ub==E_OK){
		unsigned char t_timeFlag_ub=0,stt=0;
		
		t_timeFlag_ub = DS1307_TimeGetting_UB();
				
		if(t_timeFlag_ub==BL_GETTIME_OK){
			
			stt=RTC_DeviceProcessing_UB(l_FlagStt_ub);//tested ok
			
			intdelay(3);//do not erase
			
			if(stt==2 || stt==4 || stt==5)				
				l_FlagStt_ub = stt;
		}
		
		ModuleWifi_st.l_ControlDeviceFlag_ub = E_NOT_OK;
	}
}

