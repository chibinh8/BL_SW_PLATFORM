#ifndef BL_PWM_H
#define BL_PWM_H

#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

#ifndef STM32F4XX_MOTORTYPE
#define STM32F4XX_MOTORTYPE
enum motor_en{
	MOTORLEFT,
	MOTORRIGHT,
	MOTORSERVO
};
#endif 

#ifndef STM32F4XX_MOTORDIRECTION
#define STM32F4XX_MOTORDIRECTION
enum MotorDirec_en{
	MOTORFORWARD =0,
	MOTORBACK,
};
#endif 
#define MOTORDIRECTLEFT_PIN GPIO_PIN_10
#define MOTORDIRECTRIGHT_PIN  GPIO_PIN_11
#define MOTORDIRECT_PORT GPIOD

void InitRCServo(void);
void SetAngleRCServo(uint32_t duty);

void InitPwm2Motors(void);

void SetDutyCycleMotor(uint32_t duty, uint8_t motorindex);

void SetDirectionMotor(uint8_t direction, uint8_t motorindex);

static void MX_TIM1_Init(void);
#endif
