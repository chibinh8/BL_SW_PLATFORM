#include "adc.h"
#include "string.h"
#include "BL_Define.h"

#define NumofSampling 4
#define NumofSensor (uint8_t)(NumOfSensor1+NumOfSensor2)
#define NumOfSensor1 7
#define NumOfSensor2 1

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

//ring buffer for each sensor sor will be allocated to make signal smooth
//there are 8 sensors

uint16_t ringbuff[NumofSensor][NumofSampling] = {0};
uint16_t FilteredSensorVal[NumofSensor]={0};

static uint8_t IsFilterDone = FALSE;

static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
/*Configure table of sensor channel which s mapping with ADC channel*/
const uint8_t SensorChannelADC1tbl[NumOfSensor1] = {0};
const uint8_t SensorChannelADC2tbl[NumOfSensor2] = {0};
static uint8_t EleBuffIndex = 0;

static void InitRingbuffsensor(void){
		memset((uint16_t*)ringbuff, 0, (uint8_t)(NumofSampling*NumofSensor));
		memset((uint16_t*)FilteredSensorVal, 0, NumofSensor);
}

void BL_ADCInit(void){
	
	MX_ADC1_Init(); //configure ADC 1,2 
	MX_ADC2_Init();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_Start(&hadc2);
	InitRingbuffsensor();
}


/* ADC1 init function */
static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION12b;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = EOC_SINGLE_CONV;
  HAL_ADC_Init(&hadc1);

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  HAL_ADC_ConfigChannel(&hadc1, &sConfig);

}

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


void ReadSensor(volatile uint16_t* outsensorval, ADC_HandleTypeDef *hadc, uint8_t channel){
	
	uint32_t tempsensor;
	ADC_ChannelConfTypeDef sConfig;
	sConfig.Channel = channel;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;	
  HAL_ADC_ConfigChannel(hadc, &sConfig);	
	HAL_ADC_Start(hadc);
	 if(HAL_ADC_PollForConversion(hadc,0xFF)==HAL_OK){
	   tempsensor = HAL_ADC_GetValue(hadc);
		 
	 }else{
		 tempsensor = 0u;
	 }
		HAL_ADC_Stop(hadc);
	 *outsensorval = (uint16_t)(tempsensor&0x0FFF);	 
}

/*Run in task 10 ms*/

void ReadAllRawSensorfromLine(void){
	uint16_t SensorRawVal;
	uint16_t total_t=0;
	for(int i=0; i<NumOfSensor1; i++){
			ReadSensor(&SensorRawVal,&hadc1,SensorChannelADC1tbl[i]);
			/*update sensor val for ring buff at index EleBuffIndex*/
			ringbuff[i][EleBuffIndex] = SensorRawVal;
			for(int j=0; i<NumofSampling;j++)
					total_t += ringbuff[i][j];
			FilteredSensorVal[i] = total_t/NumofSampling;		
	}	
	for(int i=0; i<NumOfSensor2; i++){
			ReadSensor(&SensorRawVal,&hadc2,SensorChannelADC2tbl[i]);
			/*update sensor val for ring buff at index EleBuffIndex*/
			ringbuff[NumOfSensor1+i][EleBuffIndex] = SensorRawVal;
			for(int j=0; i<NumofSampling;j++)
					total_t += ringbuff[NumOfSensor1+i][j];
			FilteredSensorVal[NumOfSensor1+i] = total_t/NumofSampling;					
	}	
	
	if(EleBuffIndex==NumofSampling) {		
		IsFilterDone = TRUE; //reading is ok only if it's already sampled 4 times
		EleBuffIndex = 0; //reset buffer index
	}else{
		EleBuffIndex++;
	}
}

/*piblic API to get final val for all sensor and reading status*/

uint8_t ReadAllFinalSensorfromLine(uint16_t *AllsensorFinalVal){
	
	if(IsFilterDone==TRUE) AllsensorFinalVal = FilteredSensorVal;
	
	return IsFilterDone;	
}
