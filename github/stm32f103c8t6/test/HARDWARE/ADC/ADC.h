#ifndef __ADC_H
#define __ADC_H	
#include <stm32f10x.h>
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#define u8  unsigned char 
#define u16 unsigned int
#define u32 unsigned long
void ADC_Config(void);
u16 Get_BatteryAdc(void);
float Get_VREFINT(void);
#endif 
  