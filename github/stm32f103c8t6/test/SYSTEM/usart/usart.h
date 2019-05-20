#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
	
void uart_init(u32 bound);
void uart2_init(u32 bound);
void uart3_init(u32 bound);
void UART1_send_byte(u8 data);
void UART2_send_byte(u8 data);
void Uart1_SendStr(char *SendBuf);
void UART_Send(u8 *Buffer, uint32_t Length);
#endif


