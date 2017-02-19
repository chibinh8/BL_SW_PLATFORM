#include "pwm.h"

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

static void MX_TIM3_Init(void);

static void InitDirectionPin(void){
  /*Configure GPIO pin : motor direction Pin */
	GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = MOTORDIRECTLEFT_PIN|MOTORDIRECTRIGHT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(MOTORDIRECT_PORT, &GPIO_InitStruct);
}

void InitRCServo(void){
	MX_TIM3_Init();
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}


/* htim3.Init.Period = 1679
RC servo angle is in range of 0 --> 90
So: Duty should be angle*1679/90
Example: 45 degree --> duty = 839
*/
void SetAngleRCServo(uint32_t duty){
	if(duty>=htim3.Init.Period)
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, htim3.Init.Period);
	else
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, duty);
}

void InitPwm2Motors(void){
	/*Init Timer 1 for chanel 1, 2*/
		MX_TIM1_Init();
		InitDirectionPin();
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
}

void SetDutyCycleMotor(uint32_t duty, uint8_t motorindex){
	if(duty>=htim1.Init.Period) duty = htim1.Init.Period;
	if(MOTORLEFT==motorindex)
		__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_1, duty);
	else if(MOTORRIGHT==motorindex){
		__HAL_TIM_SetCompare(&htim1, TIM_CHANNEL_2, duty);
	}
}

void SetDirectionMotor(uint8_t direction, uint8_t motorindex){
		
		if(motorindex==MOTORLEFT){
				if(direction==MOTORFORWARD)
					HAL_GPIO_WritePin(MOTORDIRECT_PORT, MOTORDIRECTLEFT_PIN,GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(MOTORDIRECT_PORT, MOTORDIRECTLEFT_PIN,GPIO_PIN_RESET);
		}else{
				if(direction==MOTORFORWARD)
					HAL_GPIO_WritePin(MOTORDIRECT_PORT, MOTORDIRECTRIGHT_PIN,GPIO_PIN_SET);
				else
					HAL_GPIO_WritePin(MOTORDIRECT_PORT, MOTORDIRECTRIGHT_PIN,GPIO_PIN_RESET);					
		}	
}
/* TIM1 init function */
void MX_TIM1_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
  TIM_OC_InitTypeDef sConfigOC;
	
		/*Time base parameter*/
		/*	
	TIM1 is connected to APB2 bus, which has on F407 device 84MHz clock 				
	But, 01219665828 has internal PLL, which double this frequency for timer, up to 168MHz 	
	Remember: Not each timer is connected to APB1, there are also timers connected 	
	on APB2, which works at 168MHz by default, and internal PLL increase 				
	this to up to 168MHz 															
	
	Set timer prescaller 
	Timer count frequency is set with 
	
	timer_tick_frequency = Timer_default_frequency / (prescaller_set + 1)		
	
	In our case, we want a max frequency for timer, so we set prescaller to 0 		
	And our timer will have tick frequency		
	
	timer_tick_frequency = 168000000 / (0 + 1) = 168000000 
*/		
					/*Configure timer*/
		htim1.Instance = TIM1;
		htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
		htim1.Init.Prescaler = 0;

/*
	Set timer period when it have reset
	First you have to know max value for timer
	In our case it is 16bit = 65535
	To get your frequency for PWM, equation is simple
	
	PWM_frequency = timer_tick_frequency / (TIM_Period + 1)
	
	If you know your PWM frequency you want to have timer period set correct
	
	TIM_Period = timer_tick_frequency / PWM_frequency - 1
	
	In our case, for 10Khz PWM_frequency, set Period to
	
	TIM_Period = 168000000 / 10000 - 1 = 167999
	
	If you get TIM_Period larger than max timer value (in our case 65535),
	you have to choose larger prescaler and slow down timer tick frequency
*/	
    htim1.Init.Period = 167999;	/* 10kHz PWM */
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
		HAL_TIM_PWM_Init(&htim1);
		sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
		sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
		HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

		sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
		sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
		sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
		sBreakDeadTimeConfig.DeadTime = 0;
		sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
		sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
		sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
		HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);
	  /* PWM mode 2 = Clear on compare match */
      /* PWM mode 1 = Set on compare match */
		sConfigOC.Pulse = 2099; /* 25% duty cycle */
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
		sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
		sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
		sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
		sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
		HAL_TIM_PWM_ConfigChannel(&htim1,&sConfigOC,TIM_CHANNEL_1);
		HAL_TIM_PWM_ConfigChannel(&htim1,&sConfigOC,TIM_CHANNEL_2);   

}


/* TIM3 init function */
void MX_TIM3_Init(void)
{
	/*PWM period  = 20 ms = 50 hz*/
	/*
		Set timer prescaller 
	Timer count frequency is set with 
	
	timer_tick_frequency = Timer_default_frequency / (prescaller_set + 1)		
	
	In our case, we want a max frequency for timer, so we set prescaller to 0 		
	And our timer will have tick frequency		
	
	timer_tick_frequency = 84000000 / (999 + 1) = 84000 
	*/
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
	/*
	Set timer period when it have reset
	First you have to know max value for timer
	In our case it is 16bit = 65535
	To get your frequency for PWM, equation is simple
	
	PWM_frequency = timer_tick_frequency / (TIM_Period + 1)
	
	If you know your PWM frequency you want to have timer period set correct
	
	TIM_Period = timer_tick_frequency / PWM_frequency - 1
	
	In our case, for 10Khz PWM_frequency, set Period to
	
	TIM_Period = 84000 / 50 - 1 = 1679
	*/
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1679;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_PWM_Init(&htim3);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);

}
