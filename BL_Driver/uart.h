#ifndef STM32F4XX_HAL
#define STM32F4XX_HAL
#include "stm32f4xx_hal.h"
#endif 
#include "BL_Define.h"


#ifndef BL_UART_H
#define BL_UART_H
#define BL_UART huart2
#define BL_UARTIncstance   USART2
#define BL_UARTIRQFuncHandler USART2_IRQHandler
#define BL_BUFFSIZE 10
#define BAUDRATE  9600

void ClearRxBuffer(void);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void USART2_IRQHandler(void);
void MX_USART_UART_Init(void);

uint8_t GetDataRXcomplete(UART_HandleTypeDef *huart, char *outbuffer, uint8_t Readoption, uint8_t size);

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
 
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
	
#endif
	
	