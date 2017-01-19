#include "adc.h"

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);

const uint8_t SensorChannelADC1tbl[NumOfSensor1] = {0};
const uint8_t SensorChannelADC2tbl[NumOfSensor2] = {0};
	
void BL_ADCInit(void){
	
	MX_ADC1_Init(); //configure ADC 1,2 
	MX_ADC2_Init();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_Start(&hadc2);
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


void ReadAllSensorfromLine(uint16_t *AllsensorRawVal){
	
	for(int i=0; i<NumOfSensor1; i++){
			ReadSensor(AllsensorRawVal,&hadc1,SensorChannelADC1tbl[i]);
			AllsensorRawVal++; //make sure pass an static array to function
	}	
	for(int i=0; i<NumOfSensor2; i++){
			ReadSensor(AllsensorRawVal,&hadc2,SensorChannelADC2tbl[i]);
			AllsensorRawVal++; //make sure pass an static array to function
	}	
	/*Noise filter technique*/
	
}
