#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 

#include "stm32f4xx_hal_i2c.h"

#define DS1307_I2C_ADDR				0xD0
#define TIMEOUT_RTC			10000


//structure of time
typedef struct {
 uint8_t second;
 uint8_t Minute;
 uint8_t Hours;	
 uint8_t Day;	
 uint8_t Date;	
 uint8_t Mouth;		
 uint8_t Year;		
} bl_rtc_time; 

//I2C DS1307 status
typedef enum {
BL_WRITE_OK = 0 ,
BL_WRITE_NOTOK,
BL_READ_OK,	
BL_READ_NOTOK,
BL_SETTIME_OK,	
BL_SETTIME_NOTOK,
BL_GETTIME_OK,
BL_GETTIME_NOTOK
} DS1307IntfacState;

#define REG_SECONDS 		0x00
#define REG_MINUTES 		0x01
#define REG_HOURS   		0x02
#define REG_DAY 				0x03
#define REG_DATE 				0x04
#define REG_MONTH 			0x05
#define REG_YEAR 				0x06
#define REG_CONTROL 		0x07


static DS1307IntfacState BL_WriteByte2RTC_I2C(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint8_t size);

static DS1307IntfacState BL_ReadByteRTC_I2C(I2C_HandleTypeDef *hi2c,uint8_t *pData, uint8_t size);

DS1307IntfacState BL_GetTime_RTC(bl_rtc_time*  t_CurrTime,I2C_HandleTypeDef *hi2c);

DS1307IntfacState BL_SetTime_RTC(bl_rtc_time t_CurrTime ,I2C_HandleTypeDef *hi2c);

static uint8_t BL_ConvertBCD2Dec(uint8_t bcd);

static uint8_t BL_ConvertDec2Bcd(uint8_t Dec);

static uint8_t BL_TimeBound(uint8_t min, uint8_t max, uint8_t  time );



