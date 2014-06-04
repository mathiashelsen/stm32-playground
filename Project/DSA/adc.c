#include "adc.h"


void readChannel(char channel, int16_t * value)
{
    //while ( !(GPIOA->IDR & 0x0003) ); // Wait while PA3 (~BUSY) is low
    GPIOA->BSRRL |= GPIO_Pin_1;
    int nCount = 0x00ff;
    while(nCount--);
    GPIOA->BSRRH |= GPIO_Pin_1;
    while( !GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) );

    //Drive CONVST (PA1) low
    //GPIOA->BSRRH |= GPIO_Pin_1;

    volatile adc_tx_struct adc;
    adc.sleep   = 0;
    adc.nap     = 0;
    adc.gain    = 0;
    adc.uni     = 0;
    adc.sgl     = 1;
    adc.select  = channel / 2;
    adc.odd     = channel % 2;

    volatile uint8_t * buffer = (uint8_t *)(&adc);
    uint8_t tmp;
    // During this first I/O operation the results from the previous conversion are discarded!
    SPI1->DR = *buffer;
    while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while (!(SPI1->SR & SPI_I2S_FLAG_RXNE ) );
    while( SPI1->SR & SPI_I2S_FLAG_BSY );
    SPI1->DR = 0x00; // Transmit dummy byte
    while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while (!(SPI1->SR & SPI_I2S_FLAG_RXNE ) );
    while( SPI1->SR & SPI_I2S_FLAG_BSY );
   
    // Wait for ~5us 
    nCount = 0x00ff;
    while(nCount--);
    //Drive CONVST high and back low
    GPIOA->BSRRL |= GPIO_Pin_1;
    nCount = 0x00ff;
    while(nCount--);
    GPIOA->BSRRH |= GPIO_Pin_1;

    //while ( !(GPIOA->IDR & 0x0003) ); // Wait while PA4 (~BUSY) is low
    while( !( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) ) );
    // During this second I/O operation the results from the previous conversion are saved
    SPI1->DR = *buffer;
    while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until transmit complete
    tmp = SPI1->DR;
    SPI1->DR = 0x00; // Transmit dummy byte
    while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until transmit complete

    uint16_t tmp2 = tmp | ( SPI1->DR << 8 );
    memcpy(value, &tmp2, 2);
}

void readChannels(int16_t *value)
{
    // Start a conversion, just to purge possible buffers and reset ADC state machine
    GPIOA->BSRRL |= GPIO_Pin_1;
    int nCount = 0x00ff;
    while(nCount--);
    GPIOA->BSRRH |= GPIO_Pin_1;
    while( !GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) );


    volatile adc_tx_struct adc;
    volatile uint8_t * buffer = (uint8_t *)(&adc);
    uint8_t tmp[2], channel = 0, tmr;
    adc.sleep   = 0;
    adc.nap     = 0;
    adc.gain    = 0;
    adc.uni     = 0;
    adc.sgl     = 1;

    for(channel = 0; channel < 8; channel++ )
    {
	// Set for channel
	adc.select  = channel / 2;
	adc.odd     = channel % 2;

	// During this I/O operation the results from the previous conversion are saved
        SPI1->DR = *buffer;
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
        while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until transmit complete
	tmp[0] = SPI1->DR;
        SPI1->DR = 0x00; // Transmit dummy byte
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
        while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until transmit complete
	tmp[1] = SPI1->DR;

	// Copy the previous measurement
	if( channel > 0 )
	{
	    memcpy(&(value[channel-1]), tmp, 2);
	}

	tmr = 0xff;
	while(tmr--);

	// Start a conversion
        GPIOA->BSRRL |= GPIO_Pin_1;
        while( !GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) );
	GPIOA->BSRRH |= GPIO_Pin_1;

	tmr = 0xff;
	while(tmr--);
    }

    // Channel 7 has not been read, this we do seperatly
    channel = 0; 
    // Set for channel
    adc.select  = channel / 2;
    adc.odd     = channel % 2;

    // During this second I/O operation the results from the previous conversion are saved
    SPI1->DR = *buffer;
    while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until transmit complete
    tmp[0] = SPI1->DR;
    SPI1->DR = 0x00; // Transmit dummy byte
    while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until transmit complete
    tmp[1] = SPI1->DR;

    memcpy(&(value[7]), tmp, 2);

    // Start a conversion
    GPIOA->BSRRL |= GPIO_Pin_1;
    while( !GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) );
    GPIOA->BSRRH |= GPIO_Pin_1;

}

/*
void readChannels(int16_t *value)
{
    // Initialisation for the first measurement 
    while ( !(GPIOA->IDR & 0x0003) ); // Wait while PA3 (~BUSY) is low
    //Drive CONVST (PA1) low
    GPIOA->BSRRH |= GPIO_Pin_1;
    adc_tx_struct adc;
    adc.sleep   = 0;
    adc.nap     = 0;
    adc.gain    = 0;
    adc.uni     = 0;
    adc.sgl     = 1;

    // Sample channel 0
    char channel = 0;

    adc.select  = channel / 2;
    adc.odd     = channel % 2;

    uint8_t * buffer = (uint8_t *)(&adc);
    // During this first I/O operation the results from the previous conversion are discarded!
    SPI1->DR = *buffer;
    while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    SPI1->DR = 0x00; // Transmit dummy byte
    while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
    // Wait for ~5us 
    int nCount = 0x00ff;
    while(nCount--);
    //Drive CONVST high
    GPIOA->BSRRL |= GPIO_Pin_1;
 
    //
    // Iterate over the other channels
    // whilst reading the result from the previous samplings
    // 
    while( channel < 8 )
    {
        // Increase the channel (wrap around at 8)
        channel += 1;
        adc.select  = (channel % 8) / 2;
        adc.odd     = channel % 2;

        while ( !(GPIOA->IDR & 0x0003) ); // Wait while PA3 (~BUSY) is low

        // During this second I/O operation the results from the previous conversion are saved
        SPI1->DR = *buffer;
        while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
        while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete
        uint8_t tmp = SPI1->DR;
        SPI1->DR = 0x00; // Transmit dummy byte
        while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
        while( !(SPI1->SR & SPI_I2S_FLAG_RXNE) ); // wait until receive complete

        uint16_t tmp2 = (tmp << 8) | SPI1->DR;
        value[channel-1] = (int16_t) tmp2;

        // Wait for ~5us 
        int nCount = 0x00ff;
        while(nCount--);
        //Drive CONVST high
        GPIOA->BSRRL |= GPIO_Pin_1;
        nCount = 0x00ff;
        while(nCount--);
        GPIOA->BSRRH |= GPIO_Pin_1; // CONVST -> low
    }
}
*/


void init_ADC(void){

	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;

	// enable clock for used IO pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* configure pins used by SPI1
	 * PA5 = SCK
	 * PA6 = MISO
	 * PA7 = MOSI
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	// connect SPI1 pins to SPI alternate function
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	// enable clock for used IO pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOE, &GPIO_InitStruct);

	GPIOE->BSRRL |= GPIO_Pin_3; // set PE3 high (disables LIS302DL)

        // PA1..3 as output (CONVST, RESET, ~CS)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIOA->BSRRH |= GPIO_Pin_1; // Bring CONVST low

	// PA4 as input (BUSY)
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	// enable peripheral clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* configure SPI1 in Mode 0
	 * CPOL = 0 --> clock is low when idle
	 * CPHA = 0 --> data is sampled at the first edge
	 */
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     // transmit in master mode, NSS pin has to be always high
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; // one packet of data is 8 bits wide
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;        // clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_2Edge;      // data sampled at first edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
	SPI_InitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // SPI frequency is APB2 frequency / 4
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;// data is transmitted MSB first
	SPI_Init(SPI1, &SPI_InitStruct);

	SPI_Cmd(SPI1, ENABLE); // enable SPI1

	// Write a word to the LTC1859 and start a conversion
	volatile adc_tx_struct adc;
	adc.sleep   = 0;
	adc.nap     = 0;
	adc.gain    = 0;
	adc.uni     = 0;
	adc.sgl     = 1;
	adc.select  = 0;
	adc.odd     = 0;
    
	volatile uint8_t * buffer = (uint8_t *)(&adc);
	uint8_t tmp;
	// During this first I/O operation the results from the previous conversion are discarded!
	SPI1->DR = *buffer;
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
	while (!(SPI1->SR & SPI_I2S_FLAG_RXNE ) );
	while( SPI1->SR & SPI_I2S_FLAG_BSY );
	SPI1->DR = 0x00; // Transmit dummy byte
	while( !(SPI1->SR & SPI_I2S_FLAG_TXE) ); // wait until transmit complete
	while (!(SPI1->SR & SPI_I2S_FLAG_RXNE ) );
	while( SPI1->SR & SPI_I2S_FLAG_BSY );
    
	// Wait for ~5us 
	uint32_t nCount = 0x00ff;
	while(nCount--);
	//Drive CONVST high and back low
	GPIOA->BSRRL |= GPIO_Pin_1;
	nCount = 0x00ff;
        while(nCount--);
	GPIOA->BSRRH |= GPIO_Pin_1;
}
