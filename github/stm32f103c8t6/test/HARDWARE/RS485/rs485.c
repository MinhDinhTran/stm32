#include "sys.h"		    
#include "rs485.h"	 
#include "delay.h"
#include "usart.h"	
#define countof(a) (sizeof(a) / sizeof(*(a)))//���������ڵĳ�Ա����
u8 RS485_RX_BUF[64]; 		//���ջ���,���64���ֽ�
u8 RS485_RX_CNT;   			//���յ������ݳ���
unsigned char  Rs485_Recok; 	  
void RS485_Init(u32 bound)
{  	 
     //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	//ʹ�ܣ�GPIOAʱ��,GPIOB
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//USART2
 	USART_DeInit(USART2);  //��λ����2
	 //USART2_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
   
    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3
    	 //PB3 ENABLE
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //PB.3
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//�������
    GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PB3
   //Usart2 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
    USART_Init(USART2, &USART_InitStructure); //��ʼ������
		
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//���������ж�
    USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 

	
	RS485_TX_EN=0;				//Ĭ��Ϊ����ģʽ	
}

//RS485����len���ֽ�.
//buf:�������׵�ַ
//len:���͵��ֽ���(Ϊ�˺ͱ�����Ľ���ƥ��,���ｨ�鲻Ҫ����64���ֽ�)
void RS485_Send_Data(u8 *buf,u8 len)
{
	u8 t;
	RS485_TX_EN=1;			//����Ϊ����ģʽ
  	for(t=0;t<len;t++)		//ѭ����������
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //�ȴ����ͽ���		
    USART_SendData(USART2,buf[t]); //��������
	}	 
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //�ȴ����ͽ���		
	RS485_RX_CNT=0;	  
	RS485_TX_EN=0;				//����Ϊ����ģʽ	
}

void USART2_IRQHandler(void)
{
	u8 Res;static char i=0, start=0;
	 if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	 {      Res =USART_ReceiveData(USART2); //��ȡ���յ�������
  	  
            if(Res == 0x42) //������յĵ�һλ������0X42������ǲ鿴���������ֲ��֪�ģ�
            {
                RS485_RX_CNT = 0;     //����������ֵ��0��ʼ
                start = 1;  //�����������ȷ���ڶ�λ�Ƿ���յ���0X4D�����Ҳ�ǲ鿴���������ֲ��֪�ģ�
            }

            if(start == 1)
            {
                RS485_RX_BUF[RS485_RX_CNT] = Res ; //�ѽ��յ������ݴ浽��������
                RS485_RX_CNT++;
                if(RS485_RX_CNT >= 32 && (RS485_RX_BUF[1]==0x4d))
                    {
                       
										UART1_send_byte(RS485_RX_BUF[13]);	UART1_send_byte(RS485_RX_BUF[15]);
                      
                        start  = 0;
                        RS485_RX_CNT=0;//���¿�ʼ����   
                        RS485_RX_BUF[0] = 0;
  
		                    Rs485_Recok=1;//�����������
	                  }   
            } 
   }

   	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)//����
	{	 	
	  Res =USART_ReceiveData(USART2);//;��ȡ���յ�������USART2->DR
		Rs485_Recok=1;//�����������
	}   
}
