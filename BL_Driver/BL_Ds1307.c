
#include "BL_Ds1307.h"

#include <limits.h>
/*
Below is protocol to interface STM32 with DS1307
1. Write start timing to DS1307 by function HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
With parameters:
- hi2c: I2C_HandleTypeDef hi2c1
- DevAddress: DS1307 -->Address: 0XD0
- pData[]: A array of bytes with:
+ pData[0] : Address of start register --> The "Seconds" register : 0x00
+ pData[1] --> pData[6]: value of second, minutes...years which we need to wrire to DS1307
2. Read timing from DS1307 by function HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
  * @brief  Receives in master mode an amount of data in blocking mode.
  * @param  hi2c: pointer to a I2C_HandleTypeDef structure that contains
  *         the configuration information for I2C module
  * @param  DevAddress: Target device address
  * @param  pData: Pointer to data buffer
  * @param  Size: Amount of data to be sent
  * @param  Timeout: Timeout duration
  * @retval HAL status
3. Analyze value from read data
- Read second, minutes... and assign to structure
- Convert bcd to decimal
4. Finalize output structure

*/

DS1307IntfacState BL_WriteByte2RTC_I2C(I2C_HandleTypeDef *hi2c, uint8_t *pData, uint8_t size)
{
	if(sizeof(pData)==0) return BL_WRITE_NOTOK;//exception, nothing to transmit?
	else { 
		 if(HAL_I2C_Master_Transmit(hi2c, DS1307_I2C_ADDR,pData,(uint16_t)size,TIMEOUT_RTC)!=HAL_OK)
			 return BL_WRITE_NOTOK;
		 else
			 return BL_WRITE_OK;				
	}	
	
}

DS1307IntfacState BL_ReadByteRTC_I2C(I2C_HandleTypeDef *hi2c,uint8_t *pData, uint8_t size){
  if(hi2c==NULL||(pData==NULL))  return BL_READ_NOTOK;	
  else if(HAL_I2C_Master_Receive(hi2c,DS1307_I2C_ADDR,pData,(uint16_t)size,TIMEOUT_RTC)!=HAL_OK)
		  return BL_READ_NOTOK;
  else
		  return BL_READ_OK;	
}
/*
In:  
Instance of I2C_x
Struct of time: bl_rtc_time t_CurrTime
Current time saved in Struct of time
The transmited frame will be:
Register Adress -> seconds ->...year


*/
DS1307IntfacState BL_SetTime_RTC(bl_rtc_time t_CurrTime,I2C_HandleTypeDef *hi2c){

	
	uint8_t t_pData[8]={REG_SECONDS, BL_ConvertDec2Bcd(t_CurrTime.second),BL_ConvertDec2Bcd(t_CurrTime.Minute),BL_ConvertDec2Bcd(t_CurrTime.Hours),BL_ConvertDec2Bcd(t_CurrTime.Day),BL_ConvertDec2Bcd(t_CurrTime.Date),BL_ConvertDec2Bcd(t_CurrTime.Mouth),BL_ConvertDec2Bcd(t_CurrTime.Year)};
	if(hi2c!=NULL){ //precondition
		//Write bytes of current time to RTC, start from seconds register
	 	if(BL_WriteByte2RTC_I2C(hi2c, t_pData,8)!=BL_WRITE_OK) return BL_SETTIME_NOTOK;
		else
		return BL_SETTIME_OK;		
	}
	else return BL_SETTIME_NOTOK;			
}
static uint8_t BL_ConvertBCD2Dec(uint8_t bcd){
	uint8_t temp_dec;
	temp_dec = 10*(bcd >>4);
	temp_dec += bcd&0x0f;
	return temp_dec;	
}

static uint8_t BL_TimeBound(uint8_t min, uint8_t max, uint8_t  time ){
 if(time<min) time = min;
 else if(time>max) time = max;
 return time;
}

DS1307IntfacState BL_GetTime_RTC(bl_rtc_time*  t_CurrTime,I2C_HandleTypeDef *hi2c){
	uint8_t t_dataRead[7];
	uint8_t t_data =REG_SECONDS;
	uint8_t t_iswriteok =0;
	//Write First Register needs to be read
	if(BL_WriteByte2RTC_I2C(hi2c, &t_data,1)==BL_WRITE_OK)  t_iswriteok = 1;
	else t_iswriteok = 0;
	//Then Read from Reg
	if((BL_ReadByteRTC_I2C(hi2c,t_dataRead,7)==BL_READ_OK)&&t_iswriteok){
		t_CurrTime->second = BL_TimeBound(0,59,BL_ConvertBCD2Dec(t_dataRead[0]));
		t_CurrTime->Minute = BL_TimeBound(0,59,BL_ConvertBCD2Dec(t_dataRead[1]));
		t_CurrTime->Hours = BL_TimeBound(0,23,BL_ConvertBCD2Dec(t_dataRead[2]));
	  t_CurrTime->Day = BL_TimeBound(0,7,BL_ConvertBCD2Dec(t_dataRead[3]));
		t_CurrTime->Date = BL_TimeBound(0,31,BL_ConvertBCD2Dec(t_dataRead[4]));
		t_CurrTime->Mouth = BL_TimeBound(1,12,BL_ConvertBCD2Dec(t_dataRead[5]));
		t_CurrTime->Year = BL_TimeBound(0,99,BL_ConvertBCD2Dec(t_dataRead[6]));
		return BL_GETTIME_OK;
	}
	else 
		return BL_GETTIME_NOTOK;
}

static uint8_t BL_ConvertDec2Bcd(uint8_t Dec){
	uint8_t low = 0;
	uint8_t high = 0;
	
	/* High nibble */
	high = Dec / 10;
	/* Low nibble */
	low = Dec - (high * 10);
	
	/* Return */
	return high << 4 | low;
}

