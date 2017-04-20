#include "dem.h"



void Error_Handler(void){
	
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_13);
	
}


void Dem_ErrorReportStatus(uint8_t FaultID, Errorsta_en errorsta){
	
	
}


uint8_t DebouncingError(uint16_t threshold_u16, uint16_t *deboundcnt);

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif