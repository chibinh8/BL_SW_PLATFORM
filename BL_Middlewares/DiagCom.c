#include "DiagCom.h"
#include "uart.h"
#include "string.h"
#include <stdlib.h>
#include "adc.h"
#include "cmsis_os.h" 



#define DAREADBUFFER  (uint8_t)(BL_BUFFSIZE/10)
extern ADCMode ADCSensorRunmode;

typedef struct {	
	uint8_t length;
	uint8_t serviceID ;	
	uint8_t DID;
}DiagCommand_typedef;

typedef enum{
	INVALLENGTH,
	INVALDID,
	INVALSERID,
	RAWCHECK_OK,
	RAWCHECK_NOTOK,
	DA_OK
}Diagstate_en;

typedef struct {	
	uint8_t SerDID;
	uint8_t DID;
	uint8_t (*callbaclfunc)(char* response,const DiagCommand_typedef Outcommand);	
	
}DIDConfig;

uint8_t FlowcontrolADCHandler(char* response,const DiagCommand_typedef Outcommand);
uint8_t SavethresholdADCRequest(char* response,const DiagCommand_typedef Outcommand);
uint8_t Esp8266ConfigurationHandler(char* response,const DiagCommand_typedef Outcommand);
uint8_t LearnSensorThresRequest(char* response,const DiagCommand_typedef Outcommand);

extern UART_HandleTypeDef BL_UART;


const DIDConfig DIDConfigtbl[NumofDID] ={ //service ID, DID, function pointer
	{
		0x01,
		0x01,
		&FlowcontrolADCHandler,
	},		
	{
		0x02,
		0x02,
		&Esp8266ConfigurationHandler,
	},	
	{
		0x03,
		0x03,
		&LearnSensorThresRequest,
	},

};


static uint16_t ConvertChartoInt(char *inputchar);

static void ProcessSubcommand(char *bufferconvert);

Diagstate_en CheckRequestPrecondition(const DiagCommand_typedef Outcommand){
		uint8_t temdid = Outcommand.serviceID;
		if(Outcommand.length!=MINSIZE) 
				return INVALLENGTH;
		else if(temdid>MAXDIDNUM||Outcommand.serviceID!=DIDConfigtbl[temdid-1].SerDID)
				return INVALDID;
		else
			  return DA_OK;
}	

char *characprocess_ptr;
Diagstate_en GetCommandfromQueue(DiagCommand_typedef *Outcommand){
	char Rx_Buffer_t[BL_BUFFSIZE];
	uint8_t length;
	char bufferconvert[MINSIZE];
	Diagstate_en retstate = RAWCHECK_NOTOK;
	memset(Outcommand, 0, sizeof(DiagCommand_typedef));
	if(TRUE==GetDataRXcomplete(&BL_UART, Rx_Buffer_t,0,DAREADBUFFER)){
			/*check if this is a diag request*/
			characprocess_ptr = &Rx_Buffer_t[0];
			while(*characprocess_ptr!=0) {
				length++;
				characprocess_ptr++;
			}
			characprocess_ptr = &Rx_Buffer_t[0];
			if(length<MINSIZECOMMAND) return FALSE;
			while((*characprocess_ptr)==' ') characprocess_ptr++; //find the fisrt position different from space
			if((*characprocess_ptr=='D')&&(*(characprocess_ptr+1)=='A')){
				/*next thing is to check and get other elements of DiagCommand_typedef: length, serviceID, DID*/
					ProcessSubcommand(bufferconvert);
					Outcommand->length = (uint8_t)ConvertChartoInt(bufferconvert);
					ProcessSubcommand(bufferconvert);					
					Outcommand->serviceID = (uint8_t)ConvertChartoInt(bufferconvert);
					ProcessSubcommand(bufferconvert);					
					Outcommand->DID = (uint8_t)ConvertChartoInt(bufferconvert);
					if((Outcommand->length*Outcommand->serviceID)!=0){
						retstate = RAWCHECK_OK;
						ClearRxBuffer();
					}

			}					
	}
	return retstate;	
}
/*Command format:  DA-Length-ServiceID-DID*/
void ProcessDiagserviceCyclicMain(void){
	uint8_t DIDpos =0;
	DiagCommand_typedef Outcommand;
	char response[BL_BUFFSIZE];
	Diagstate_en diagstate;
	diagstate = GetCommandfromQueue(&Outcommand); 
	if(diagstate==RAWCHECK_OK)
	{		diagstate = CheckRequestPrecondition(Outcommand);
			switch(diagstate){
				case DA_OK:
						DIDpos = Outcommand.serviceID;
						(*DIDConfigtbl[DIDpos-1].callbaclfunc)(response,Outcommand);
						break;
				case INVALLENGTH:
						strcpy(response,"Invalid lengh is requested");
						#ifdef DIAGRESPONSE
						printf("%s\n",response); //response to tester		
						#endif
					break;
				case INVALDID:
						strcpy(response,"Invalid Service ID is requested");
						#ifdef DIAGRESPONSE
						printf("%s\n",response); //response to tester		
						#endif
					break;		
				default:
					break;		
				}						
	}
}

static uint16_t ConvertChartoInt(char *inputchar){
		uint16_t retval = 0;
		retval = (uint16_t)atoi(inputchar);
		return retval;
}

static void ProcessSubcommand(char *bufferconvert){
	uint8_t i=0;
	memset(bufferconvert, 0,MINSIZE);
	while((*characprocess_ptr!=0)&&(*characprocess_ptr!='-')) characprocess_ptr++; //first "-"
	characprocess_ptr++;
	while((*characprocess_ptr!=0)&&(*characprocess_ptr!='-')&&(*characprocess_ptr!=' ')) {
	if(i>(MINSIZE-1)) return;
	bufferconvert[i] = *characprocess_ptr;
	characprocess_ptr++; 
	i++;
	}
}

uint8_t FlowcontrolADCHandler(char* response,const DiagCommand_typedef Outcommand){
  #ifdef DIAGRESPONSE
	printf("DA-%d-%d\n", Outcommand.serviceID, Outcommand.DID);	
	#endif
	taskENTER_CRITICAL();
	/*CALIB: 0, CYCLIC: 1, UNDEFINE: 2*/
	if((CYCLIC==Outcommand.DID)&&(ADCSensorRunmode == CALIB)) 
		ADCSensorRunmode = CYCLIC;
	else if((CALIB==Outcommand.DID)&&(ADCSensorRunmode == CYCLIC))
		ADCSensorRunmode = CALIB;
	else if(ADCUNDEFINE==Outcommand.DID){
		ADCSensorRunmode = ADCUNDEFINE;	
	}else{
		//do nothing
	}
	taskEXIT_CRITICAL();
	return 0;
}

uint8_t Esp8266ConfigurationHandler(char* response,const DiagCommand_typedef Outcommand){
	 if(Outcommand.DID==1)
		 printf("AT");	
	 return 0;
}
extern volatile uint8_t bl_adc_Calibstat_u8;

uint8_t LearnSensorThresRequest(char* response,const DiagCommand_typedef Outcommand){	
  #ifdef DIAGRESPONSE
	printf("DA-%d-%d\n", Outcommand.serviceID, Outcommand.DID);	
	#endif
	taskENTER_CRITICAL();
	bl_adc_Calibstat_u8 = Outcommand.DID; // set calib state as DID input
	taskEXIT_CRITICAL();
	return 0;
}

uint8_t CopyRXDataESPClbkDiag(char* RXbuffer){
	;
}
