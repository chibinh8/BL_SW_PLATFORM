#include "watchdog.h"

#include "dem.h"
#include "cmsis_os.h" 

IWDG_HandleTypeDef hiwdg;

uint8_t bl_wd_TaskAliFlag_u8 = 0;


/* IWDG init function */
void MX_IWDG_Init(void)
{

  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_4;// 32000/4 = 8000 Hz
  hiwdg.Init.Reload = 1600;	// Period = 200ms --> Reload = 200*8000/1000 = 1600
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }

}

void bl_wd_Init(void){
	
	 MX_IWDG_Init();
	
}

void bl_wd_TaskAlive(uint8_t Indicator){
	taskENTER_CRITICAL();
	bl_wd_TaskAliFlag_u8 |=Indicator;
	taskEXIT_CRITICAL();
}

BOOL bl_wd_IsAllTaskAli(void){
	
	return (bl_wd_TaskAliFlag_u8==0x03)?TRUE:FALSE;
	
}


void bl_wd_KickWatchdog(void){
	/* Reload IWDG counter */
	IWDG->KR = 0xAAAA;
}