#include "usart.h"

// Configure for 115200 baud
void init_USART(void)
{
    // Enable APB1 peripheral clock for USART3
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    // Enable peripheral clock for the USART pins
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // Sets up TX and RX pins
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;              // Alternate Function
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;         // Better 25(?)
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;            // Push-Pull
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;              // Pullup
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Hand over pin control to usart.
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3);

    USART_InitTypeDef usartInit = {0, };
    usartInit.USART_BaudRate = 9600;
    usartInit.USART_WordLength = USART_WordLength_8b;
    usartInit.USART_StopBits = USART_StopBits_1;
    usartInit.USART_Parity = USART_Parity_No;
    usartInit.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init( USART3, &usartInit );


    // Configure interrupts
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;        // Configure USART3 interrupts
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// Priority group
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;       // Subpriority inside the group
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;          // enable globally
    NVIC_Init(&NVIC_InitStructure);

    // Enable receive interrupt
    USART_ClearFlag(USART3, USART_IT_RXNE);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    // Enable USART
    USART_Cmd(USART3, ENABLE);
}

uint32_t readWord(void)
{
    int i = 0;
    uint32_t tmp = 0;
    while( i < 4 )
    {
	while( !USART_GetFlagStatus(USART3, USART_FLAG_RXNE) );
	tmp |= (USART3->DR << (8*i));
	i++;
    }
    return tmp;
}

uint16_t readHalfword(void)
{
    volatile USART_TypeDef* USARTx = USART3;
    int i = 0;
    uint32_t tmp = 0;
    while( i < 2 )
    {
	while( !USART_GetFlagStatus(USART3, USART_FLAG_RXNE) );
	tmp |= (USART3->DR << (8*i));
	i++;
    }
    return tmp;
}

void transmit(uint32_t size, volatile uint8_t *buffer)
{
    volatile uint8_t *ptr = buffer;
    uint32_t N = size;
    do
    {
	USART3->DR = *ptr++;
	while(!USART_GetFlagStatus(USART3, USART_FLAG_TXE));

    }while(--N);
}
