#include "timer.h"
#include "dem.h"

TIM_HandleTypeDef htim6;

/* TIM6 init function */
void MX_TIM6_Init(void)
{
//	In our case, for 1 hz PWM_frequency, set Period to
//	TIM_Period = 168000000 / 1 - 1 
  TIM_MasterConfigTypeDef sMasterConfig;
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 168000;//1 ms
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 1; //500 ms
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
	HAL_TIM_Base_Start_IT(&htim6);

}

/**
* @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
*/
void TIM6_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM6_DAC_IRQn 0 */
	
  /* USER CODE END TIM6_DAC_IRQn 0 */
  HAL_TIM_IRQHandler(&htim6);
  /* USER CODE BEGIN TIM6_DAC_IRQn 1 */
	HAL_TIM_Base_Start_IT(&htim6);
  /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM7 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
/* USER CODE BEGIN Callback 0 */
	
/* USER CODE END Callback 0 */
  if (htim->Instance == TIM7) {
      HAL_IncTick();
  }
/* USER CODE BEGIN Callback 1 */
	if(htim->Instance==TIM6){
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
		
	}		
/* USER CODE END Callback 1 */
}
/**
  * @}
  */ 
inline uint32_t BL_GetTickTimerBase(void){
	 return HAL_GetTick();
}
/*Timebased is 1 milisecond for 1 tick */
inline BOOL CheckTimestampElapsed(uint32_t timestart_u32, uint32_t period_u32){
	return ((BL_GetTickTimerBase()-timestart_u32)>=period_u32)?TRUE:FALSE;
}

inline void GetCurrentTimestamp(uint32_t *timecurr_u32){
		*timecurr_u32 = BL_GetTickTimerBase();
}

