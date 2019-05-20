#ifndef __RS485_H
#define __RS485_H			 
#include "sys.h"	 								  

#define RS485_TX_EN		PBout(3)	//485ģʽ����.0,����;1,����.//ģʽ����

#define EN_USART2_RX 	1			//0,������;1,����.//����봮���жϽ��գ�����EN_USART2_RXΪ1����������Ϊ0

extern u8 RS485_RX_BUF[64]; 		//���ջ���,���64���ֽ�
extern u8 RS485_RX_CNT;   			//���յ������ݳ���
extern unsigned char  Rs485_Recok; 														 
void RS485_Init(u32 bound);
void RS485_Send_Data(u8 *buf,u8 len);
void RS485_Receive_Data(u8 *buf,u8 *len);		 
#endif	   
















