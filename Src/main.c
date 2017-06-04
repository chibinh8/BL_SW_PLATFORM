/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/

#include  "BL_SystemConfig.h"


/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/





osThreadId defaultTaskHandle, BackgroundTaskTaskHandle, RTRealTimeTaskHandle, WatchdogTaskHandle;
				
/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/


void StartDefaultTask(void const * argument);
void StartBackgroundTask(void const * argument);
void StartRealtimeTask(void const * argument);
void StartWatchdogTask(void const * argument);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
 extern UART_HandleTypeDef BL_UART;
 extern ADC_HandleTypeDef hadc1;


/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */	
	/*system initialization*/
   BL_SystemInit();	
	
  /* USER CODE END 1 */


  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 50);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
	 osThreadDef(BackgroundTask, StartBackgroundTask, osPriorityAboveNormal, 1, 128);//128*4 = 512 byte
   BackgroundTaskTaskHandle = osThreadCreate(osThread(BackgroundTask), NULL);	 
	 
	 osThreadDef(RTRealTimeTask, StartRealtimeTask, osPriorityAboveNormal, 2, 256);
   RTRealTimeTaskHandle = osThreadCreate(osThread(RTRealTimeTask), NULL);
	 osThreadDef(WatchdogTask, StartWatchdogTask, osPriorityNormal, 0, 50);
	 WatchdogTaskHandle = osThreadCreate(osThread(WatchdogTask), NULL);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}


/* USER CODE BEGIN 4 */
uint16_t bl_db_rcduty_u8 = 0;

void StartBackgroundTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
	
	/* USER CODE END 5 */
  /* Infinite loop */
		for(;;)
		{  

			ESPOperationCyclic();
			bl_al_AlarmCyclic();
			ProcessDiagserviceCyclicMain();		
			bl_fl_NVMOperationCyclic();
			bl_wd_TaskAlive(0x01);
			osDelay(100);
		}
}
	/* StartDefaultTask function */
void StartRealtimeTask(void const * argument)
{
	
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {	

		ADCSensorMaincyclic();
		bl_pid_FollowLineContrWithPIDCyclic();
		bl_app_AutoRacerCyclic();
		bl_wd_TaskAlive(0x02);
    osDelay(5); 
  }
  /* USER CODE END 5 */ 
}
	
  /* USER CODE END 5 */ 
/* USER CODE END 4 */

/* StartDefaultTask function */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {	
		
    osDelay(1000);
  }
  /* USER CODE END 5 */ 
}

void StartWatchdogTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
	
	/* USER CODE END 5 */
  /* Infinite loop */
		for(;;)
		{  
			if(bl_wd_IsAllTaskAli()==TRUE)
				bl_wd_KickWatchdog();
			//else that is a failure in system (task is dead) then Watchdog reachs Reset threshold then do a Reset system
			
			osDelay(150);
		}
}
/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
