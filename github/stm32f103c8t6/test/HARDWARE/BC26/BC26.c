#include "bc26.h"
#include "string.h"
#include "usart.h"
#include "wdg.h"
#include "led.h"
char *strx,*extstrx;
extern char  RxBuffer[100],RxCounter;
BC26 BC26_Status;
unsigned char socketnum=0;//��ǰ��socket����
void Clear_Buffer(void)//��ջ���
{
		u8 i;
		Uart1_SendStr(RxBuffer);
		for(i=0;i<100;i++)
		RxBuffer[i]=0;//����
		RxCounter=0;
		IWDG_Feed();//ι��
	
}
void BC26_Init(void)
{
    printf("AT\r\n"); 
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    Clear_Buffer();	
    while(strx==NULL)
    {
        Clear_Buffer();	
        printf("AT\r\n"); 
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//����OK
    }
    printf("AT+CIMI\r\n");//��ȡ���ţ������Ƿ���ڿ�����˼���Ƚ���Ҫ��
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"460");//��460������ʶ�𵽿���
    Clear_Buffer();	
    while(strx==NULL)
    {
        Clear_Buffer();	
        printf("AT+CIMI\r\n");//��ȡ���ţ������Ƿ���ڿ�����˼���Ƚ���Ҫ��
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"460");//����OK,˵�����Ǵ��ڵ�
    }
        printf("AT+CGATT=1\r\n");//�������磬PDP
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"OK");//��OK
        Clear_Buffer();	
        printf("AT+CGATT?\r\n");//��ѯ����״̬
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"+CGATT: 1");//��1
        Clear_Buffer();	
		while(strx==NULL)
		{
            Clear_Buffer();	
            printf("AT+CGATT?\r\n");//��ȡ����״̬
            delay_ms(300);
            strx=strstr((const char*)RxBuffer,(const char*)"+CGATT: 1");//����1,����ע���ɹ�
		}
		printf("AT+CESQ\r\n");//�鿴��ȡCSQֵ
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)"+CESQ");//����CSQ
		if(strx)
			{
				BC26_Status.CSQ=(strx[7]-0x30)*10+(strx[8]-0x30);//��ȡCSQ
				if((BC26_Status.CSQ==99)||((strx[7]-0x30)==0))//˵��ɨ��ʧ��
				{
					while(1)
					{
                        BC26_Status.netstatus=0;
						Uart1_SendStr("�ź�����ʧ�ܣ���鿴ԭ��!\r\n");
                        RESET=1;//����
                        delay_ms(300);
                        delay_ms(300);	
                        RESET=0;//��λģ��
						delay_ms(300);//û���źž͸�λ
                        
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
	printf("AT+QSOCL=0\r\n");//�ر�socekt����
	delay_ms(300);
    IWDG_Feed();//ι��
}
void BC26_ConTCP(void)
{
		uint8_t i;
	printf("AT+QICLOSE=0\r\n");//�ر�socekt����
	delay_ms(300);
    Clear_Buffer();
    IWDG_Feed();//ι��
}
void BC26_CreateTCPSokcet(void)//����sokcet
{

    printf("AT+QIOPEN=1,0,\"TCP\",\"103.46.128.41\",25441,1234,1\r\n");//��������TCP,����IP�Լ��������˿ں��� ,����ֱ���³��ķ�ʽ
    delay_ms(300);
    strx=strstr((const char*)RxBuffer,(const char*)"+QIOPEN: 0,0");//����Ƿ��½�ɹ�
 	while(strx==NULL)
		{
            strx=strstr((const char*)RxBuffer,(const char*)"+QIOPEN: 0,0");//����Ƿ��½�ɹ�
            delay_ms(100);
		}  
     Clear_Buffer();	
    
}
void BC26_Senddata(uint8_t *len,uint8_t *data)//�ַ�����ʽ
{
    printf("AT+QSOSEND=0,%s,%s\r\n",len,data);
}

void BC26_Senddatahex(uint8_t *len,uint8_t *data)//����ʮ����������
{
    printf("AT+QISENDEX=0,%s,%s\r\n",len,data);
        delay_ms(300);
 	while(strx==NULL)
		{
            strx=strstr((const char*)RxBuffer,(const char*)"SEND OK");//����Ƿ��ͳɹ�
            delay_ms(100);
		}  
     Clear_Buffer();	
}

void BC26_RECData()
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"+QSONMI");//����+QSONMI���������յ�UDP���������ص�����
    if(strx)
    {
       
        BC26_Status.Socketnum=strx[8];//���
      //  BC26_Status.reclen=strx[10];//����,����10���ڵ�
        delay_ms(300);
        strx=strstr((const char*)RxBuffer,(const char*)",");//��ȡ����һ������
        for(i=0;;i++)
        { 
            if(strx[i+1]==',')
            break;
            BC26_Status.recdatalen[i]=strx[i+1];//��ȡ���ݳ���
        }
        strx=strstr((const char*)(strx+1),(const char*)",");//��ȡ���ڶ�������
        for(i=0;;i++)
        {
            if(strx[i+1]==0x0d)
            break;
            BC26_Status.recdata[i]=strx[i+1];//��ȡ��������
        }
     
      
            Clear_Buffer();
                 
    }
}


void BC26_RECTCPData()
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"+QIURC:");//����+QIURC:���������յ�TCP���������ص�����
    if(strx)
    {
    
      
            Clear_Buffer();
                 
    }
}
void BC26_ChecekConStatus(void)
{
    char i;
    static char nexti;
    strx=strstr((const char*)RxBuffer,(const char*)"socket_t is NULL");//��������ǿ�ƶϿ�����
    if(strx)
    {
         BC26_CreateTCPSokcet();//���´���һ��SOCKET����
         Clear_Buffer();	
       
    }
}