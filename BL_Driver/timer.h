#ifndef BL_TIMER_H
#define BL_TIMER_H 
#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 
#include "BL_Define.h"

void MX_TIM6_Init(void);
void TIM6_DAC_IRQHandler(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void GetCurrentTimestamp(uint32_t *timecurr_u32);

uint8_t CheckTimestampElapsed(uint32_t timestart_u32, uint32_t period_u32);

#endif

