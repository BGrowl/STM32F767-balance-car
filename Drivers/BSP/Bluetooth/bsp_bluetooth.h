#ifndef __BSP_BLUETOOTH_H_
#define __BSP_BLUETOOTH_H_

#include "main.h"

void bluetooth_init(void);
void UART5_Send_U8(uint8_t ch);
void UART5_Send_ArrayU8(uint8_t *BufferPtr, uint16_t Length);


void UART5_Send_Char(char *s);
void USART5_Send_Byte(unsigned char byte);

#endif
