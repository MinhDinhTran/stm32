#include "lcd12864.h"
#include "delay.h"


void LCD_PinInit(void){
    GPIO_InitTypeDef GPIO_InitStructure; 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = 0xf8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	  
}

void LCD_SendByte(u8 byte) {
    u8 i;
    for(i=0; i<8; i++) {
        LCD_EN=0;
		delay_ms(1);
		LCD_RW=(byte<<i)&0x80?1:0;
        LCD_EN=1;
		delay_ms(1);
    }
}

void LCD_WriteCommand(u8 Cbyte) {
    LCD_RS=1;
    LCD_SendByte(0xf8);
    LCD_SendByte(0xf0&Cbyte);
    LCD_SendByte(0xf0&Cbyte<<4);
    LCD_RS=0;
}

void LCD_WriteData(u8 Cbyte) {
    LCD_RS=1;
    LCD_SendByte(0xfa);
    LCD_SendByte(0xf0&Cbyte);
    LCD_SendByte(0xf0&Cbyte<<4);
    LCD_RS=0;
}

void LCD_Init(void) {
	LCD_PinInit(); 
	PSB=0;
  RST=0;
	delay_ms(2);
	RST=1;
 
    LCD_WriteCommand(0x30);//基本指令
    LCD_WriteCommand(0x03);//地址归位
    LCD_WriteCommand(0x0C);//
    LCD_WriteCommand(0x01);
    LCD_WriteCommand(0x06);
	delay_ms(1);
}

void LCD_Print(char *s){
	char *p=s;
	while(*p){
		LCD_WriteData(*p);
		p++;
	}
}

void LCD_Clear(void) {
    u8 i;
    LCD_WriteCommand(0x30);
    LCD_WriteCommand(0x80);
    for(i=0; i<64; i++)
        LCD_WriteData(0x20);
}
