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
extern u8 RS485_RX_BUF[64]; 		//接收缓冲,最大64个字节
extern u8 RS485_RX_CNT;   			//接收到的数据长度
extern unsigned char  Rs485_Recok; 					
void Send_ATcmd(void)//发送AT指令给到模块，从串口1接收指令，串口2控制
{
	char i;
	for(i=0;i<RxCounter1;i++)
	{
	 while((USART3->SR&0X40)==0);//等待发送完成 
	  USART3->DR = RxBuffer1[i]; 
	}
}

void OPEN_BC26(void)
{
   char *strx;
 
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
	IWDG_Feed();//喂狗
   if(strx==NULL)
	{
        PWRKEY=1;//拉低
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);
        delay_ms(300);	
        PWRKEY=0;//拉高正常开机
        IWDG_Feed();//喂狗
	}
    printf("AT\r\n"); 
    delay_ms(300);
    IWDG_Feed();//喂狗
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
     if(strx==NULL)//如果设备休眠了，就复位模块
     {
        RESET=1;//拉低
        delay_ms(300);
        delay_ms(300);	
        RESET=0;//复位模块
     }
    printf("ATE0&W\r\n"); //关闭回显
    delay_ms(300); 
    LED=0;
    IWDG_Feed();//喂狗
}
/***
对于电信卡而言，由于电信对IP的限制，TCP发送也会存在限制，所以TCP如果不是绑定IP也是会发送失败，对于移动而言不影响使用。建议移动客户进行实验
***/

//适用于NB版本
 int main(void)
 {	
	  int i=0;
    static u8 pm25,pm10;
	  
    float Vbat;//电池电压
   static  u8 SendVbat;//发送电池电压
	  char a[8]={1},b[8]={0},c[8]={0};
    u8 sendata[]="672E802961631234";
    
    delay_init();	    	 //延时函数初始化	 	
 
    NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	 	ADC_Config();
    LED_Init();		  		//初始化与LED连接的硬件接口
    BC26CTR_Init();        //初始化BC26的PWR与RESET引脚
		LCD_Init();
		uart_init(9600);//串口1初始化，可连接PC进行打印模块返回数据
    uart3_init(9600);//初始化和GPRS连接串口	
    RS485_Init (9600);

    
		IWDG_Init(7,625);    //8S一次
    OPEN_BC26();//对BC26开机
    TIM3_Int_Init(999,7199);//100ms更新一次
    TIM4_Int_Init(999,7199);//100ms更新一次  
    BC26_Init();//对设备初始化
    BC26_ConTCP();//关闭上一次连接
    BC26_CreateTCPSokcet();//创建一个SOCKET连接	
 while(1)
    {  
		pm25=RS485_RX_BUF[13];
	  pm10=RS485_RX_BUF[15];
		Vbat=Get_VREFINT();//获取到ADC电压值
    SendVbat=Vbat*100;//3.33*100=333保留2位有效数据
		if(pm25==0 | pm10==0)
		{
	    PSB=0;
      RST=0;
	    delay_ms(1);
	    RST=1;
      LCD_WriteCommand(0x30);//基本指令
      LCD_WriteCommand(0x03);//地址归位
      LCD_WriteCommand(0x0C);//
      LCD_WriteCommand(0x01);
      LCD_WriteCommand(0x06);
     	delay_ms(1);
			pm25=0;
			pm10=0;
			LCD_WriteCommand(LCD_GOTO_LINE1);
		LCD_Print("    环境监测");
		LCD_WriteCommand(LCD_GOTO_LINE2);
		sprintf (a,"pm2.5:%dug/m3",pm25);
		LCD_Print(a);
	  LCD_WriteCommand(LCD_GOTO_LINE3);
		sprintf (b,"pm10 :%dug/m3",pm10);
		LCD_Print(b);
		 LCD_WriteCommand(LCD_GOTO_LINE4);
		sprintf (c,"噪声 :%ddB",SendVbat);
		LCD_Print(c);
		}
		LCD_WriteCommand(LCD_GOTO_LINE1);
		LCD_Print("    环境监测");
		LCD_WriteCommand(LCD_GOTO_LINE2);
		sprintf (a,"pm2.5:%dug/m3",pm25);
		LCD_Print(a);
	  LCD_WriteCommand(LCD_GOTO_LINE3);
		sprintf (b,"pm10 :%dug/m3",pm10);
		LCD_Print(b);
		 LCD_WriteCommand(LCD_GOTO_LINE4);
		sprintf (c,"噪声 :%ddB",SendVbat);
		LCD_Print(c);
			
		sendata[10]=pm25/10+0x30;
		sendata[11]=pm25%10+0x30;
		sendata[12]=pm10/10+0x30;
		sendata[13]=pm10%10+0x30;
		sendata[14]=SendVbat/10+0x30;
		sendata[15]=SendVbat%10+0x30;
			                      //转成字符形式
        BC26_Senddatahex("8",sendata);//发数据，按照十六进制方式发送 
        delay_ms(1000);
        RxCounter=0; 
        BC26_ChecekConStatus();
        BC26_RECData();//收数据 
        IWDG_Feed();//喂狗
    }	 
}



