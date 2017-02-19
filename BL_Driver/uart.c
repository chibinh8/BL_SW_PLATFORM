#include "uart.h"
#include "string.h"

UART_HandleTypeDef BL_UART;
static uint8_t IsRxcpltFlag_b = FALSE ;
static char Rx_data[2], Rx_Buffer[BL_BUFFSIZE];
uint8_t Rx_indx =0;

/* USART init function */
void MX_USART_UART_Init(void)
{
	
  BL_UART.Instance = BL_UARTIncstance;
  BL_UART.Init.BaudRate = BAUDRATE;
  BL_UART.Init.WordLength = UART_WORDLENGTH_8B;
  BL_UART.Init.StopBits = UART_STOPBITS_1;
  BL_UART.Init.Parity = UART_PARITY_NONE;
  BL_UART.Init.Mode = UART_MODE_TX_RX;
  BL_UART.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  BL_UART.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&BL_UART);
	HAL_UART_Receive_IT(&BL_UART, (uint8_t*)Rx_data, 1);	//activate UART receive interrupt every time
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	/*Once 1 charater is recieved, this callback function will be invoked*/
		/*Indicate data received*/	
	  if (huart->Instance == BL_UARTIncstance)	//current UART
		{	if(Rx_data[0]=='O') HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
			if ((Rx_data[0]!=13)&&(Rx_data[0]!=10))	//if received data different from ascii 13 (enter)
				{	
					if(Rx_indx==(BL_BUFFSIZE-1)) Rx_indx=0; //reset buffer index in case lengh of input string over buffer
					Rx_Buffer[Rx_indx++]=Rx_data[0];	//add data to Rx_Buffer
				}
			else			//if received data = 13
				{
				Rx_indx=0;
				IsRxcpltFlag_b=TRUE;//transfer complete, data is ready to read
				}
			 __HAL_UART_CLEAR_OREFLAG(&BL_UART); //clear overrun flag
			 HAL_UART_Receive_IT(&BL_UART, (uint8_t*)Rx_data, 1);	//activate UART receive interrupt every time
		}
}

uint8_t GetDataRXcomplete(UART_HandleTypeDef *huart, const char *outbuffer, uint8_t Readoption, uint8_t size){
		uint8_t datarecev_b = FALSE;
		if (huart->Instance == BL_UARTIncstance){	//current UART
				if((Readoption==1)||((Readoption==0)&&(IsRxcpltFlag_b==TRUE))) //if data has been already available then update buffer
				{ //if Readoption =1, read all incoming data else only read data if \r\n is detected
					memcpy((char *)outbuffer,Rx_Buffer,size); 
					datarecev_b = TRUE;
				}
				else
					memset((char *)outbuffer,0,size); 
				return datarecev_b;
		}
		return FALSE;
}

void ClearRxBuffer(void){
	memset((char *)Rx_Buffer,0,BL_BUFFSIZE);
	IsRxcpltFlag_b = FALSE;
}

void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
	/*Do not use printf function in any interrup function*/
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&BL_UART);
  /* USER CODE BEGIN USART2_IRQn 1 */
	
  /* USER CODE END USART2_IRQn 1 */
}	
#ifdef DEBUG
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart){
	
	printf ("Error code: %d\n", huart->ErrorCode);		
	
}
#endif
PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	HAL_UART_Transmit(&BL_UART, (uint8_t *)&ch, 1, 100);
	
	return ch;
}
