#include "adc.h"

void init_ADC1(void)
{
    // Enable the respective clocks
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC, ENABLE);

    // (1) Prepare PB0 for ADC mode
    GPIO_InitTypeDef gpio_init;
    memset( (void *) &gpio_init, 0, sizeof(GPIO_InitTypeDef) );
    gpio_init.GPIO_Pin	= GPIO_Pin_0;
    gpio_init.GPIO_Mode = GPIO_Mode_AN;
    gpio_init.GPIO_PuPd	= GPIO_PuPd_NOPULL;
    GPIO_Init( GPIOB, &gpio_init );

    // (2) Initialize the ADC common struct
    ADC_CommonInitTypeDef adc_common_init;
    memset( (void *) &adc_common_init, 0, sizeof( ADC_CommonInitTypeDef ) );
    adc_common_init.ADC_Mode = ADC_Mode_Independent;
    adc_common_init.ADC_Prescaler = ADC_Prescaler_Div4;
    ADC_CommonInit( &adc_common_init );

    // (3) Initialize the ADC-1 specific struct
    ADC_InitTypeDef adc_init;
    memset( (void *) &adc_init, 0, sizeof(ADC_InitTypeDef) );
    adc_init.ADC_Resolution	    = ADC_Resolution_12b;
    adc_init.ADC_ContinuousConvMode = DISABLE;
    adc_init.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
    adc_init.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_T3_TRGO;
    adc_init.ADC_DataAlign	    = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfConversion    = 1;
    ADC_Init( ADC1, &adc_init );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_28Cycles);

    // Configure interrupts
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;        // Configure USART1 interrupts
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;// Priority group
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       // Subpriority inside the group
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          // enable globally
    NVIC_Init(&NVIC_InitStructure);

    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

    // Enable the ADC
    ADC_Cmd(ADC1, ENABLE);
}
