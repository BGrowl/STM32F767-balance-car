#include "bsp_bluetooth.h"
#include "app_bluetooth.h"
#include "usart.h"



//发送一个字符  Send a character
void UART5_Send_U8(uint8_t ch)
{
//	while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET)
//		;
//	USART_SendData(UART5, ch);
	HAL_UART_Transmit(&huart1, &ch, 1, 1000);
}

//发送一个字符串  Send a string
/**
 * @Brief: UART5发送数据 UART5 sends data
 * @Note: 
 * @Parm: BufferPtr:待发送的数据(Data to be sent)  Length:数据长度(Data length)
 * @Retval: 
 */
void UART5_Send_ArrayU8(uint8_t *BufferPtr, uint16_t Length)
{
//	while (Length--)
//	{
//		UART5_Send_U8(*BufferPtr);
//		BufferPtr++;
//	}
		HAL_UART_Transmit(&huart1, BufferPtr, Length, 1000);
}

/*串口5发送函数 Serial port 5 sending function*/
void USART5_Send_Byte(unsigned char byte)   //串口发送一个字节 The serial port sends a byte
{
//		USART_SendData(UART5, byte);        //通过库函数  发送数据 Send data through library functions
//		while( USART_GetFlagStatus(UART5,USART_FLAG_TC)!= SET); 
//		while (USART_GetFlagStatus(UART5, USART_FLAG_TXE) == RESET);
//	USART_SendData(UART5, byte);        //通过库函数  发送数据  Send data through library functions
	HAL_UART_Transmit(&huart1, &byte, 1, 1000);

}
void UART5_Send_Char(char *s)
{
	char *p;
	p=s;
	while(*p != '\0')
	{
		USART5_Send_Byte(*p);
		p++;
	}	
}

//串口中断服务函数  Serial port interrupt service function
//void UART5_IRQHandler(void)
//{
//	uint8_t Rx5_Temp;
//	if (USART_GetITStatus(UART5, USART_IT_RXNE) != RESET)
//	{
//		Rx5_Temp = USART_ReceiveData(UART5);
//		UART5_Send_U8(Rx5_Temp);
//		deal_bluetooth(Rx5_Temp);
//	}
//}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

    if (huart->Instance == USART1)
    {
        deal_bluetooth(Rx1_Temp);
        HAL_UART_Receive_IT(&huart1, &Rx1_Temp, 1);
    }
}
