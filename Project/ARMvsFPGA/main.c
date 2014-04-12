#include "stm32f4xx.h"
#include "misc.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

//#define LINK_BASE   (uint16_t *) 0x6000000
#define LINK_BASE ((uint32_t) (0x60000000 )


// PD[14:15] -> D[0:1], PD[0:1] -> D[2:3], PD[8:10] -> D[13:15]
// PE[7:15] -> D[4:12]
// PD4 -> NOE, PD5 -> NWE, PD7 -> NE1
void initLink(void);

int main(void)
{
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 ); 

    initLink();

    volatile uint16_t i = 0;
    while(1)
    {
	*(uint16_t *)(0x60000000) = i++;
    }

}

void initLink(void)
{
    // Init the pins 
    // PD[14:15] -> D[0:1], PD[0:1] -> D[2:3], PD[8:10] -> D[13:15]
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_InitTypeDef gpioInit = {0, };
    gpioInit.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15 | GPIO_Pin_0 | GPIO_Pin_1
			| GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    gpioInit.GPIO_Mode = GPIO_Mode_AF;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioInit.GPIO_OType = GPIO_OType_PP;
    gpioInit.GPIO_PuPd = GPIO_PuPd_NOPULL;
    // Init
    GPIO_Init(GPIOD, &gpioInit);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);

    // PE[7:15] -> D[4:12]
    gpioInit.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9
			| GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12
			| GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOE, &gpioInit);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);

    // PD4 -> NOE, PD5 -> NWE, PD7 -> NE1
    gpioInit.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_Init(GPIOD, &gpioInit);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);
    
   
    // Init the clock to the FSMC 
    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);
    // Setup the timing
    FSMC_NORSRAMTimingInitTypeDef timingInit = {0, };
    timingInit.FSMC_AddressSetupTime	= 2;
    timingInit.FSMC_AddressHoldTime	= 2;
    timingInit.FSMC_DataSetupTime	= 10;
    timingInit.FSMC_BusTurnAroundDuration = 0;
    timingInit.FSMC_CLKDivision		= 2;
    timingInit.FSMC_DataLatency		= 0;
    timingInit.FSMC_AccessMode = FSMC_AccessMode_A;
    // Setup FSMC
    FSMC_NORSRAMInitTypeDef linkInit = {0, };
    memset( (char *)&linkInit, 0, sizeof(FSMC_NORSRAMInitTypeDef) );
    linkInit.FSMC_Bank	    = FSMC_Bank1_NORSRAM1;
    linkInit.FSMC_DataAddressMux    = FSMC_DataAddressMux_Disable;
    linkInit.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    linkInit.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    linkInit.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    linkInit.FSMC_AsynchronousWait    = FSMC_AsynchronousWait_Disable;
    linkInit.FSMC_WrapMode  = FSMC_WrapMode_Disable;
    linkInit.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    linkInit.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    linkInit.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    linkInit.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    linkInit.FSMC_ReadWriteTimingStruct = &timingInit;
    linkInit.FSMC_WriteTimingStruct = &timingInit;
    // Init
    FSMC_NORSRAMInit(&linkInit);
    // and Enable
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}
