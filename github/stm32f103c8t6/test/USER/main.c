#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "math.h"			
#include "stdio.h"
#include "stm32f10x_flash.h"
#include "stdlib.h"
#include "string.h"
#include "wdg.h"
#include "timer.h"
#include "stm32f10x_tim.h"
#include "bc26.h"	 
#include "lcd12864.h"
#include "adc.h"
#include "rs485.h"	
extern char  RxBuffer[100],RxCounter;
extern unsigned char uart1_getok;
extern char RxCounter1,RxBuffer1[100];
extern unsigned char Timeout,restflag;
extern u8 RS485_RX_BUF[64]; 		//���ջ���,���64���ֽ�
extern u8 RS485_RX_CNT;   			//���յ������ݳ���
extern unsigned char  Rs485_Recok; 					
void Send_ATcmd(void)//����ATָ�����ģ�飬�Ӵ���1����ָ�����2����
{
	char i;
	for(i=0;i<RxCounter1;i++)
	{
	 while((USART3->SR&0X40)==0);//�ȴ�������� 
	  USART3->DR = RxBuffer1[i]; 
	}
}

void OPEN_BC26(void)
{
   char *strx;
 
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
	IWDG_Feed();//ι��
   if(strx==NULL)
	{
        PWRKEY=1;//����
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);	
        PWRKEY=0;//������������
        IWDG_Feed();//ι��
	}
    printf("AT\r\n"); 
    delay_ms(300);
    IWDG_Feed();//ι��
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
     if(strx==NULL)//����豸�����ˣ��͸�λģ��
     {
        RESET=1;//����
        delay_ms(300);
        delay_ms(300);	
        RESET=0;//��λģ��
     }
    printf("ATE0&W\r\n"); //�رջ���
    delay_ms(300); 
    LED=0;
    IWDG_Feed();//ι��
}
/***
���ڵ��ſ����ԣ����ڵ��Ŷ�IP�����ƣ�TCP����Ҳ��������ƣ�����TCP������ǰ�IPҲ�ǻᷢ��ʧ�ܣ������ƶ����Բ�Ӱ��ʹ�á������ƶ��ͻ�����ʵ��
***/

//������NB�汾
 int main(void)
 {	
	  int i=0;
    static u8 pm25,pm10;
	  
    float Vbat;//��ص�ѹ
   static  u8 SendVbat;//���͵�ص�ѹ
	  char a[8]={1},b[8]={0},c[8]={0};
    u8 sendata[]="672E802961631234";
    
    delay_init();	    	 //��ʱ������ʼ��	 	
 
    NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	 	ADC_Config();
    LED_Init();		  		//��ʼ����LED���ӵ�Ӳ���ӿ�
    BC26CTR_Init();        //��ʼ��BC26��PWR��RESET����
		LCD_Init();
		uart_init(9600);//����1��ʼ����������PC���д�ӡģ�鷵������
    uart3_init(9600);//��ʼ����GPRS���Ӵ���	
    RS485_Init (9600);

    
		IWDG_Init(7,625);    //8Sһ��
    OPEN_BC26();//��BC26����
    TIM3_Int_Init(999,7199);//100ms����һ��
    TIM4_Int_Init(999,7199);//100ms����һ��  
    BC26_Init();//���豸��ʼ��
    BC26_ConTCP();//�ر���һ������
    BC26_CreateTCPSokcet();//����һ��SOCKET����	
 while(1)
    {  
		pm25=RS485_RX_BUF[13];
	  pm10=RS485_RX_BUF[15];
		Vbat=Get_VREFINT();//��ȡ��ADC��ѹֵ
    SendVbat=Vbat*100;//3.33*100=333����2λ��Ч����
		if(pm25==0 | pm10==0)
		{
	    PSB=0;
      RST=0;
	    delay_ms(1);
	    RST=1;
      LCD_WriteCommand(0x30);//����ָ��
      LCD_WriteCommand(0x03);//��ַ��λ
      LCD_WriteCommand(0x0C);//
      LCD_WriteCommand(0x01);
      LCD_WriteCommand(0x06);
     	delay_ms(1);
			pm25=0;
			pm10=0;
			LCD_WriteCommand(LCD_GOTO_LINE1);
		LCD_Print("    �������");
		LCD_WriteCommand(LCD_GOTO_LINE2);
		sprintf (a,"pm2.5:%dug/m3",pm25);
		LCD_Print(a);
	  LCD_WriteCommand(LCD_GOTO_LINE3);
		sprintf (b,"pm10 :%dug/m3",pm10);
		LCD_Print(b);
		 LCD_WriteCommand(LCD_GOTO_LINE4);
		sprintf (c,"���� :%ddB",SendVbat);
		LCD_Print(c);
		}
		LCD_WriteCommand(LCD_GOTO_LINE1);
		LCD_Print("    �������");
		LCD_WriteCommand(LCD_GOTO_LINE2);
		sprintf (a,"pm2.5:%dug/m3",pm25);
		LCD_Print(a);
	  LCD_WriteCommand(LCD_GOTO_LINE3);
		sprintf (b,"pm10 :%dug/m3",pm10);
		LCD_Print(b);
		 LCD_WriteCommand(LCD_GOTO_LINE4);
		sprintf (c,"���� :%ddB",SendVbat);
		LCD_Print(c);
			
		sendata[10]=pm25/10+0x30;
		sendata[11]=pm25%10+0x30;
		sendata[12]=pm10/10+0x30;
		sendata[13]=pm10%10+0x30;
		sendata[14]=SendVbat/10+0x30;
		sendata[15]=SendVbat%10+0x30;
			                      //ת���ַ���ʽ
        BC26_Senddatahex("8",sendata);//�����ݣ�����ʮ�����Ʒ�ʽ���� 
        delay_ms(1000);
        RxCounter=0; 
        BC26_ChecekConStatus();
        BC26_RECData();//������ 
        IWDG_Feed();//ι��
    }	 
}



