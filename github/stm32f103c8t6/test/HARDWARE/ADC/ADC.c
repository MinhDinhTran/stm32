#include "adc.h"
#include "math.h"
#define ADC1_DR_Address               ((uint32_t)0x4001244c)//
__IO uint16_t RegularConvData_Tab[2];//存储2个电压值
__IO uint16_t  ADC1ConvertedValue = 0, ADC1ConvertedVoltage = 0;
__IO uint16_t VREFINT_CAL;
__IO uint16_t VREFINT_DATA;
__IO float Vbat_value;
void ADC_Config(void)
{
GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1,ENABLE);	
 
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_4;		 
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AIN;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;//ADC地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&RegularConvData_Tab; //内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; //方向(从外设到内存)
	DMA_InitStructure.DMA_BufferSize = 1; //传输内容的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址固定
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; //内存地址固定
	DMA_InitStructure.DMA_PeripheralDataSize = 
	DMA_PeripheralDataSize_HalfWord ; //外设数据单位
	DMA_InitStructure.DMA_MemoryDataSize = 
	DMA_MemoryDataSize_HalfWord ;    //内存数据单位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular  ; //DMA模式：循环传输
	DMA_InitStructure.DMA_Priority = DMA_Priority_High ; //优先级：高
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;   //禁止内存到内存的传输
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //配置DMA1的1通道
	DMA_Cmd(DMA1_Channel1,ENABLE);
 
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; //独立ADC模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;  //禁止扫描方式
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//开启连续转换模式 
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; //不使用外部触发转换
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //采集数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 4; //要转换的通道数目
	ADC_Init(ADC1, &ADC_InitStructure);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div8);//配置ADC时钟，为PCLK2的8分频，即9Hz
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 1, ADC_SampleTime_55Cycles5);//配置ADC1通道1为55.5个采样周期 
	
	ADC_DMACmd(ADC1,ENABLE);
	ADC_Cmd(ADC1,ENABLE);
 
	ADC_ResetCalibration(ADC1);//复位校准寄存器
	while(ADC_GetResetCalibrationStatus(ADC1));//等待校准寄存器复位完成
 
	ADC_StartCalibration(ADC1);//ADC校准
	while(ADC_GetCalibrationStatus(ADC1));//等待校准完成
 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//由于没有采用外部触发，所以使用软件触发ADC转换

}
/***************获取基准电压值*****************/
float Get_VREFINT(void)
{
     /* Test DMA1 TC flag */
    if((DMA_GetFlagStatus(DMA1_FLAG_TC1)) )
    {
    /* Clear DMA TC flag */
    DMA_ClearFlag(DMA1_FLAG_TC1);
 
    } 
      VREFINT_DATA=RegularConvData_Tab[0];//获取到ADC值
    Vbat_value =(1.2*VREFINT_DATA)/4096.0;//等待返回 电阻分压的
    return Vbat_value;//返回当前值  
}

