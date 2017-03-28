#ifndef BL_TIMER_H
#define BL_TIMER_H 

#include "BL_Define.h"

void MX_TIM6_Init(void);
void TIM6_DAC_IRQHandler(void);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

void GetCurrentTimestamp(uint32_t *timecurr_u32);

BOOL CheckTimestampElapsed(uint32_t timestart_u32, uint32_t period_u32);

#endif

