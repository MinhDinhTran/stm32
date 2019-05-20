#include "bc26.h"
#include "string.h"
#include "usart.h"
#include "wdg.h"
#include "led.h"
char *strx,*extstrx;
extern char  RxBuffer[100],RxCounter;
BC26 BC26_Status;
unsigned char socketnum=0;//当前的socket号码
void Clear_Buffer(void)//清空缓存
{
		u8 i;
		Uart1_SendStr(RxBuffer);
		for(i=0;i<100;i++)
		RxBuffer[i]=0;//缓存
		RxCounter=0;
		IWDG_Feed();//喂狗
	
}
void BC26_Init(void)
{
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    Clear_Buffer();	
    while(strx==NULL)
    {
        Clear_Buffer();	
        printf("AT\r\n"); 
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//返回OK
    }
    printf("AT+CIMI\r\n");//获取卡号，类似是否存在卡的意思，比较重要。
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"460");//返460，表明识别到卡了
    Clear_Buffer();	
    while(strx==NULL)
    {
        Clear_Buffer();	
        printf("AT+CIMI\r\n");//获取卡号，类似是否存在卡的意思，比较重要。
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"460");//返回OK,说明卡是存在的
    }
        printf("AT+CGATT=1\r\n");//激活网络，PDP
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//返OK
        Clear_Buffer();	
        printf("AT+CGATT?\r\n");//查询激活状态
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"+CGATT: 1");//返1
        Clear_Buffer();	
		while(strx==NULL)
		{
            Clear_Buffer();	
            printf("AT+CGATT?\r\n");//获取激活状态
            delay_ms(300);
            strx=strstr((const char*)RxBuffer,(const char*)"+CGATT: 1");//返回1,表明注网成功
		}
		printf("AT+CESQ\r\n");//查看获取CSQ值
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"+CESQ");//返回CSQ
		if(strx)
			{
				BC26_Status.CSQ=(strx[7]-0x30)*10+(strx[8]-0x30);//获取CSQ
				if((BC26_Status.CSQ==99)||((strx[7]-0x30)==0))//说明扫网失败
				{
					while(1)
					{
                        BC26_Status.netstatus=0;
						Uart1_SendStr("信号搜索失败，请查看原因!\r\n");
                        RESET=1;//拉低
                        delay_ms(300);
                        delay_ms(300);	
                        RESET=0;//复位模块
						delay_ms(300);//没有信号就复位
                        
					}
				}
             else
             {
                 BC26_Status.netstatus=1;
              }
                
            }
}



void BC26_ConUDP(void)
{
	uint8_t i;
	printf("AT+QSOCL=0\r\n");//关闭socekt连接
	delay_ms(300);
    IWDG_Feed();//喂狗
}
void BC26_ConTCP(void)
{
		uint8_t i;
	printf("AT+QICLOSE=0\r\n");//关闭socekt连接
	delay_ms(300);
    Clear_Buffer();
    IWDG_Feed();//喂狗
}
void BC26_CreateTCPSokcet(void)//创建sokcet
{

    printf("AT+QIOPEN=1,0,\"TCP\",\"103.46.128.41\",25441,1234,1\r\n");//创建连接TCP,输入IP以及服务器端口号码 ,采用直接吐出的方式
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"+QIOPEN: 0,0");//检查是否登陆成功
 	while(strx==NULL)
		{
            strx=strstr((const char*)RxBuffer,(const char*)"+QIOPEN: 0,0");//检查是否登陆成功
            delay_ms(100);
		}  
     Clear_Buffer();	
    
}
void BC26_Senddata(uint8_t *len,uint8_t *data)//字符串形式
{
    printf("AT+QSOSEND=0,%s,%s\r\n",len,data);
}

void BC26_Senddatahex(uint8_t *len,uint8_t *data)//发送十六进制数据
{
    printf("AT+QISENDEX=0,%s,%s\r\n",len,data);
        delay_ms(300);
 	while(strx==NULL)
		{
            strx=strstr((const char*)RxBuffer,(const char*)"SEND OK");//检查是否发送成功
            delay_ms(100);
		}  
     Clear_Buffer();	
}

void BC26_RECData()
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"+QSONMI");//返回+QSONMI，表明接收到UDP服务器发回的数据
    if(strx)
    {
       
        BC26_Status.Socketnum=strx[8];//编号
      //  BC26_Status.reclen=strx[10];//长度,低于10个内的
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)",");//获取到第一个逗号
        for(i=0;;i++)
        { 
            if(strx[i+1]==',')
            break;
            BC26_Status.recdatalen[i]=strx[i+1];//获取数据长度
        }
        strx=strstr((const char*)(strx+1),(const char*)",");//获取到第二个逗号
        for(i=0;;i++)
        {
            if(strx[i+1]==0x0d)
            break;
            BC26_Status.recdata[i]=strx[i+1];//获取数据内容
        }
     
      
            Clear_Buffer();
                 
    }
}


void BC26_RECTCPData()
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"+QIURC:");//返回+QIURC:，表明接收到TCP服务器发回的数据
    if(strx)
    {
    
      
            Clear_Buffer();
                 
    }
}
void BC26_ChecekConStatus(void)
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"socket_t is NULL");//表明电信强制断开连接
    if(strx)
    {
         BC26_CreateTCPSokcet();//重新创建一个SOCKET连接
         Clear_Buffer();	
       
    }
}