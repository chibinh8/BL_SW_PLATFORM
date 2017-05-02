#include "adc.h"
#include "string.h"
#include "timer.h"
#include "dem.h"
#include "cmsis_os.h" 
#include <stdlib.h>

ADC_HandleTypeDef hadc1;

#ifdef USEADC2
ADC_HandleTypeDef hadc2;
#endif

#define BLACKOFFSET   0 
#define WHITEOFFSET   0

#define ADCSENSORTHRES_REG  ((const volatile uint16_t *)ADCSENSORTHRES_BASE)

#define ADCSENSORTHRES ((const volatile BL_AdcThres_Type *)ADCSENSORTHRES_BASE)
	
#define GETBASEADDRESS(BaseAddress_u32)  ((const volatile uint8_t *)BaseAddress_u32)
#define ADCSENSORTHRES_BASE ((uint32_t)0x080E0000)


//ring buffer for each sensor sor will be allocated to make signal smooth
//there are 8 sensors

uint16_t ringbuff[NumofSensor][NumofSampling] = {0};
uint16_t FilteredSensorVal[NumofSensor]={0};
ADCMode ADCSensorRunmode = CYCLIC;
LineState SensorStateArray[NumofSensor][2] = {UNDEFINE};
LineState FinalLineSensorState[NumofSensor] = {UNDEFINE};
static BOOL IsFilterDone = FALSE;
static uint16_t ADCSensorBlackUpperThres[NumofSensor] = {0};
static uint16_t ADCSensorWhiteLowerThres[NumofSensor] = {0};
void bl_adc_SortArray(uint16_t* AdcArr);

BL_AdcThres_Type adcreadthres; //Adc threshold stored value in FLASH during learning color

static void MX_ADC1_Init(void);
#ifdef USEADC2
static void MX_ADC2_Init(void);
#endif
/*Configure table of sensor channel which s mapping with ADC channel
    PC0     ------> ADC1_IN10
    PC1     ------> ADC1_IN11
    PC2     ------> ADC1_IN12
    PA1     ------> ADC1_IN1
    PC4     ------> ADC1_IN14
    PC5     ------> ADC1_IN15
    PB0     ------> ADC1_IN8
    PB1     ------> ADC1_IN9 
*/
const uint32_t SensorChannelADC1tbl[NumOfSensor1] = { ADC_CHANNEL_8, ADC_CHANNEL_11, ADC_CHANNEL_12, ADC_CHANNEL_1, ADC_CHANNEL_10, ADC_CHANNEL_9, ADC_CHANNEL_14, ADC_CHANNEL_15};
#ifdef USEADC2
const uint32_t SensorChannelADC2tbl[NumOfSensor2] = {0};
#endif
static uint8_t EleBuffIndex = 0;

void bl_adc_DataCompareThres(void);

static void InitRingbuffsensor(void){
		memset((uint16_t*)ringbuff, 0, (uint8_t)(NumofSampling*NumofSensor*sizeof(uint16_t)));
		memset((uint16_t*)FilteredSensorVal, 0, NumofSensor*2);
		memset((uint16_t*)adcreadthres.blackupperthres, 0, NumofSensor*sizeof(uint16_t));
		memset((uint16_t*)adcreadthres.whitelowwerthres, 0, NumofSensor*sizeof(uint16_t));
}

void BL_ADCInit(void){
	BL_AdcThres_Type adcthres_t;
	ADCSensorRunmode = CYCLIC;
	MX_ADC1_Init(); //configure ADC 1,2 
	HAL_ADC_Start(&hadc1);
	#ifdef USEADC2
	MX_ADC2_Init();
	HAL_ADC_Start(&hadc2);
	#endif
	InitRingbuffsensor();
	/*Read threshold for black and White status from flash memmory*/
	ReadADCThreshold(&adcthres_t);
	memcpy(ADCSensorBlackUpperThres, (const uint16_t*)adcthres_t.blackupperthres, NumofSensor*sizeof(uint16_t));
	memcpy(ADCSensorWhiteLowerThres, (const uint16_t*)adcthres_t.whitelowwerthres, NumofSensor*sizeof(uint16_t));
	bl_adc_DataCompareThres();

}

/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}
#ifdef USEADC2
/* ADC2 init function */
static void MX_ADC2_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc2.Instance = ADC2;
  hadc2.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
  hadc2.Init.Resolution = ADC_RESOLUTION12b;
  hadc2.Init.ScanConvMode = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DMAContinuousRequests = DISABLE;
  hadc2.Init.EOCSelection = EOC_SINGLE_CONV;
  HAL_ADC_Init(&hadc2);

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc2, &sConfig);

}
#endif

void ReadSensor(volatile uint16_t* outsensorval, ADC_HandleTypeDef *hadc, uint8_t channel){
	
	uint32_t tempsensor;
	ADC_ChannelConfTypeDef sConfig;
	sConfig.Channel = channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;	
  HAL_ADC_ConfigChannel(hadc, &sConfig);	
	HAL_ADC_Start(hadc);
	 if(HAL_ADC_PollForConversion(hadc,1)==HAL_OK){
	   tempsensor = HAL_ADC_GetValue(hadc);
		 
	 }else{
		 tempsensor = 0u;
	 }

	 *outsensorval = (uint16_t)(tempsensor&0x0FFF);	 
}

/*Run in task 50 ms*/

void ReadAllRawSensorfromLine(void){
	uint16_t SensorRawVal;
	uint16_t total_t=0;
	for(int i=0; i<NumOfSensor1; i++){
			ReadSensor(&SensorRawVal,&hadc1,SensorChannelADC1tbl[i]);
			/*update sensor val for ring buff at index EleBuffIndex*/
			ringbuff[i][EleBuffIndex] = SensorRawVal;
			if(EleBuffIndex==NumofSampling) {	
				bl_adc_SortArray(&ringbuff[i][0]);
				for(int j=NumOfIgnoreEle; j<(NumofSampling-NumOfIgnoreEle);j++)
						total_t += ringbuff[i][j];
				FilteredSensorVal[i] = total_t/(NumofSampling-NumOfIgnoreEle*2);	
			}				
	}	
	#ifdef USEADC2
	for(int i=0; i<NumOfSensor2; i++){
			ReadSensor(&SensorRawVal,&hadc2,SensorChannelADC2tbl[i]);
			/*update sensor val for ring buff at index EleBuffIndex*/
			ringbuff[NumOfSensor1+i][EleBuffIndex] = SensorRawVal;
			if(EleBuffIndex==NumofSampling){	
				bl_adc_SortArray(&ringbuff[i][0]);
				for(int j=NumOfIgnoreEle; j<(NumofSampling-NumOfIgnoreEle);j++)
						total_t += ringbuff[i][j];
				FilteredSensorVal[i] = total_t/(NumofSampling-NumOfIgnoreEle*2);	
			}					
	}	
	#endif
	if(EleBuffIndex==NumofSampling) {		
		IsFilterDone = TRUE; //reading is ok only if it's already sampled 4 times
		EleBuffIndex = 0; //reset buffer index
	}else{
		EleBuffIndex++;
		
	}
}

/*public API to get final val for all sensor and reading status*/

BOOL ReadAllFinalSensorfromLine(uint16_t *AllsensorFinalVal){
	
	if(IsFilterDone==TRUE) {
		memcpy(AllsensorFinalVal, FilteredSensorVal, NumofSensor*sizeof(uint16_t));
		IsFilterDone = FALSE;
		return TRUE;	
	}	
	return FALSE;
}

/*Converting Digital input to physical status of sensor
BLACK --> if digital input is greater or equal to it's lower threshold
WHITE --> if digital input is lower or equal to it's lower threshold
Note: Threshold is calibrated value which is stored in flalsh memory before.
*/

BOOL ReadStatusofAllsensorWithOffset(uint8_t * OutStatusSS){
	
	if(IsFilterDone==TRUE) {
		for(int i=0; i<NumofSensor; i++){
				if(FilteredSensorVal[i]>=(ADCSensorBlackUpperThres[i]- BLACKOFFSET))
					OutStatusSS[i] = BLACK;
				else if(FilteredSensorVal[i]<=(ADCSensorWhiteLowerThres[i]+ WHITEOFFSET))
					OutStatusSS[i] = WHITE;
				else {
					OutStatusSS[i] = UNDEFINE;	//The input value is not in defined range.			
					//do nothing
				}	
			}
		IsFilterDone = FALSE;
		return TRUE;	
	}	
	return FALSE;	
}

BOOL ReadStatusofAllsensor(LineState * OutStatusSS){
	uint16_t MeanWhiteBlack_u16; 
	
	if(IsFilterDone==TRUE) {
		for(int i=0; i<NumofSensor; i++){
				MeanWhiteBlack_u16 = (ADCSensorBlackUpperThres[i] + ADCSensorWhiteLowerThres[i])>>1;
				if(FilteredSensorVal[i]>(MeanWhiteBlack_u16 + WHITEOFFSET))
					OutStatusSS[i] = SensorStateArray[i][0];				
				else if(FilteredSensorVal[i]<(MeanWhiteBlack_u16-BLACKOFFSET))
					OutStatusSS[i] = SensorStateArray[i][1];
				else {
					OutStatusSS[i] = UNDEFINE;	//The input value is not in defined range.			
					//do nothing
				}	
			}
		IsFilterDone = FALSE;
		return TRUE;	
	}	
	return FALSE;	
}

void ADCSensorDebouncing( void *DebouncedVal, uint16_t debouncecnt){
	/*to be defined later*/
	
}
volatile uint8_t bl_adc_Calibstat_u8 = 255;

/*Calibration mode*/
void SensorThresCalib(void){
	/*Following calibration steps should be done to get the best value for sensor
		1. Read ADC with BLACK color
			- While sampling for WHITE threshold is still not requested, get ADC for BLACK color, update with bigest value for sometime.
			- Change state to read ADC for WHITE threshold if requested
		2. Read ADC with WHITE color 
			- While sampling for BLACK threshold is still not requested, get ADC for WHITE color, update with lowest value for sometime.
			- Change state to read ADC for BLACK threshold if requested			
		3. Save threshold to flash
	Note: bl_adc_Calibstat_u8 is changed by DiagCom request DA-02-03-[DID] with DID: state number
	*/
	uint32_t currtime_u32 =0;
	
	ReadAllRawSensorfromLine();
	
	switch (bl_adc_Calibstat_u8){
		
		case 1: //read ADC for BACLK color
			memcpy((void*)adcreadthres.blackupperthres, FilteredSensorVal, NumofSensor*sizeof(uint16_t));
			
			break;
		case 2: 
			memcpy((void*)adcreadthres.whitelowwerthres, FilteredSensorVal, NumofSensor*sizeof(uint16_t));

			break;
		case 3: //save ADC value to Flash			
			
			if((E_OK==bl_fl_WriteByte2NVM((const uint8_t*)&adcreadthres.blackupperthres[0],ADCSENSORTHRES_BASE, sizeof(BL_AdcThres_Type)))){
						bl_adc_Calibstat_u8 = 4;//default val, if saving process is not sucessfilly, try in next cycle
						//get current time
					  GetCurrentTimestamp(&currtime_u32);
			}
			break;
		case 4 : 			
				if(TRUE==CheckTimestampElapsed(currtime_u32, 4000)){ //1s
					//blink LED to indicate NVM writing threshold process is done						
						bl_adc_Calibstat_u8 = 255;
				}else{
					HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
				}

				break;
		case 5: //debug only
				memcpy(ADCSensorBlackUpperThres, &FilteredSensorVal[0], NumofSensor*sizeof(uint16_t));
				break;
		case 6: //debug only
				memcpy(ADCSensorWhiteLowerThres, &FilteredSensorVal[0], NumofSensor*sizeof(uint16_t));
				bl_adc_DataCompareThres();
		    break;
		default:
				/*do nothing*/
				break;
	}
	
}



void ADCSensorMaincyclic(void){
	
	switch(ADCSensorRunmode){
		case CALIB:
			 SensorThresCalib();
			 /*change state condition should be added here*/
			
		   break;
		case CYCLIC:			 
				ReadAllRawSensorfromLine();
				ReadStatusofAllsensor(FinalLineSensorState);
				/*change state condition should be added here*/
				break;
		default:			
				break;		
	}	
}


void bl_adc_DataCompareThres(void){
	uint8_t LoopIndex;
	for(LoopIndex=0;LoopIndex<NumofSensor;LoopIndex++){
			if(ADCSensorBlackUpperThres[LoopIndex]>ADCSensorWhiteLowerThres[LoopIndex]){
					SensorStateArray[LoopIndex][0] = BLACK;
					SensorStateArray[LoopIndex][1] = WHITE;
			}else if(ADCSensorBlackUpperThres[LoopIndex]<ADCSensorWhiteLowerThres[LoopIndex]){
					SensorStateArray[LoopIndex][0] = WHITE;
					SensorStateArray[LoopIndex][1] = BLACK;					
			}
		
	}
	
}

void ReadADCThreshold(	BL_AdcThres_Type *adcreadthres){
	/*need to be defined*/
	memcpy((void*)adcreadthres->blackupperthres, (void*)ADCSENSORTHRES->blackupperthres,NumofSensor*2);
	memcpy((void*)adcreadthres->whitelowwerthres, (void*)ADCSENSORTHRES->whitelowwerthres,NumofSensor*2);
}


uint8_t SaveADCThreshold2NVM(const BL_AdcThres_Type AdcThres){
	/*need to be defined*/
	uint8_t i=0;
	uint8_t RETVAL = E_NOT_OK;
	static volatile uint32_t addressflash = ADCSENSORTHRES_BASE;
	taskENTER_CRITICAL();
	HAL_FLASH_Unlock();
	for(i=0; i<NumofSensor;i++)
		{
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addressflash, AdcThres.blackupperthres[i])!=HAL_OK){
				return E_NOT_OK;
			}
			addressflash +=2; //2 bytes
		}
	for(i=0; i<NumofSensor;i++)
		{
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addressflash,AdcThres.whitelowwerthres[i])!=HAL_OK){
				return E_NOT_OK;
			}
			addressflash +=2; //2 bytes
		}
	HAL_FLASH_Lock(); 
	taskEXIT_CRITICAL();
	return E_OK;
}

void ReadADCThresholdfromNVM(uint16_t *val2write){
	*val2write = (*ADCSENSORTHRES_REG);
	
}

int CompareFunc(const void *a, const void *b){
	
	return ((*(uint16_t*)a)-(*(uint16_t*)b));
}
void bl_adc_SortArray(uint16_t* AdcArr){
	
	qsort(AdcArr, NumofSensor, sizeof(uint16_t),CompareFunc);
	
}

