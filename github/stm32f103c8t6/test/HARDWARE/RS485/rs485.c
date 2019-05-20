#include "sys.h"		    
#include "rs485.h"	 
#include "delay.h"
#include "usart.h"	
#define countof(a) (sizeof(a) / sizeof(*(a)))//计算数组内的成员个数
u8 RS485_RX_BUF[64]; 		//接收缓冲,最大64个字节
u8 RS485_RX_CNT;   			//接收到的数据长度
unsigned char  Rs485_Recok; 	  
void RS485_Init(u32 bound)
{  	 
     //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	//使能，GPIOA时钟,GPIOB
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//USART2
 	USART_DeInit(USART2);  //复位串口2
	 //USART2_TX   PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
    //USART2_RX	  PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3
    	 //PB3 ENABLE
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; //PB.3
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB3
   //Usart2 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//115200
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
    USART_Init(USART2, &USART_InitStructure); //初始化串口
		
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启空闲中断
    USART_Cmd(USART2, ENABLE);                    //使能串口 

	
	RS485_TX_EN=0;				//默认为接收模式	
}

//RS485发送len个字节.
//buf:发送区首地址
//len:发送的字节数(为了和本代码的接收匹配,这里建议不要超过64个字节)
void RS485_Send_Data(u8 *buf,u8 len)
{
	u8 t;
	RS485_TX_EN=1;			//设置为发送模式
  	for(t=0;t<len;t++)		//循环发送数据
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //等待发送结束		
    USART_SendData(USART2,buf[t]); //发送数据
	}	 
	while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //等待发送结束		
	RS485_RX_CNT=0;	  
	RS485_TX_EN=0;				//设置为接收模式	
}

void USART2_IRQHandler(void)
{
	u8 Res;static char i=0, start=0;
	 if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) 
	 {      Res =USART_ReceiveData(USART2); //读取接收到的数据
  	  
            if(Res == 0x42) //如果接收的第一位数据是0X42（这个是查看传感器的手册得知的）
            {
                RS485_RX_CNT = 0;     //让数组索引值从0开始
                start = 1;  //这个变量是来确定第二位是否接收到了0X4D（这个也是查看传感器的手册得知的）
            }

            if(start == 1)
            {
                RS485_RX_BUF[RS485_RX_CNT] = Res ; //把接收到的数据存到数组里面
                RS485_RX_CNT++;
                if(RS485_RX_CNT >= 32 && (RS485_RX_BUF[1]==0x4d))
                    {
                       
										UART1_send_byte(RS485_RX_BUF[13]);	UART1_send_byte(RS485_RX_BUF[15]);
                      
                        start  = 0;
                        RS485_RX_CNT=0;//重新开始接收   
                        RS485_RX_BUF[0] = 0;
  
		                    Rs485_Recok=1;//表明接收完成
	                  }   
            } 
   }

   	if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)//空闲
	{	 	
	  Res =USART_ReceiveData(USART2);//;读取接收到的数据USART2->DR
		Rs485_Recok=1;//表明接收完成
	}   
}
