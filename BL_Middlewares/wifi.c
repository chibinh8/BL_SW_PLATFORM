
#include <stdio.h>
#include <string.h>
#include "stm32f0xx_hal.h"
#include "esp8266.h"
#include "library.h"

extern ESP8266 ModuleWifi_st;
extern void intdelay (uint32_t cnt);

static
void WIFI_FlashProcess_V(void){

	if(ModuleWifi_st.l_confirm_flag_ub==E_NOT_OK && ModuleWifi_st.l_wifichange_flag_ub==E_OK){
		
		intdelay(1);//do not erase
		
		printf(" data=%c %c ",ModuleWifi_st.l_WifiInfo_ub[1],ModuleWifi_st.l_WifiInfo_ub[2]);
		
		ModuleWifi_st.l_wifichange_flag_ub = E_NOT_OK;
	}
}

void WIFI_Processing_V(void){
	
	WIFI_FlashProcess_V();
}
