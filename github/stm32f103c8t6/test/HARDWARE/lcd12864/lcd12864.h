#ifndef __LCD12864_H
#define __LCD12864_H
#include "sys.h"
#define LCD_GOTO_LINE1 0x80
#define LCD_GOTO_LINE2 0x90
#define LCD_GOTO_LINE3 0x88
#define LCD_GOTO_LINE4 0x98
#define LCD_CMD_BASE 0x30
#define LCD_CMD_EXT 0x34
#define LCD_RS PBout(3)
#define LCD_RW PBout(4)
#define LCD_EN PBout(5)
#define PSB PBout(6)
#define RST PBout(7)
void LCD_PinInit(void);
void LCD_SendByte(u8 byte);
void LCD_WriteCommand(u8 Cbyte);
void LCD_WriteData(u8 Cbyte);
void LCD_Init(void);
void LCD_Print(char *s);
void LCD_Clear(void);

#endif
