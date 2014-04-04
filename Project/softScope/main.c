#include <misc.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "adc.h"
#include "clock.h"
#include "leds.h"
#include "usart.h"
#include "utils.h"

// -- sampling
#define STATE_IDLE	   ((int32_t)  0)
#define STATE_PROCESS  ((int32_t)  1)
#define STATE_OVERFLOW ((int32_t) -1)

volatile uint32_t ADC_PERIOD  = 420;  // 100kSamples
#define SAMPLES	    1024              // Number of samples for each acquisition/frame

volatile uint16_t *samplesBuffer; // The samples buffer is divided into 4 frames
volatile uint32_t triggerFrame;   // A number between 0..3 that indicates  in which frame we need to look for a trigger
volatile uint16_t triggerLevel;
volatile int32_t state;


// -- I/O protocol
#define HEADER_WORDS  8

// Frame data header
typedef struct{
	uint32_t magic;                   // identifies start of header, 0xFFFFFFFF
	uint32_t samples;                 // number of samples
	uint32_t trigLev;
	uint32_t timeBase;
	uint32_t padding[HEADER_WORDS-4]; // unused space, needed for correct total size
} header_t;

// -- outbound communication
static uint8_t *usartBuf;   // embeds outbox and outdata so they can be TX'ed in one go
static header_t *outbox;    // header written to software, first part of usartBuf
static uint16_t *outData;   // data written to software, second part of usartBuf


// -- inbound communication
static volatile header_t inbox;   // last header sent from software, values can be used
static volatile header_t _inbuf;  // receive buffer for incoming usart communication, not to be used
static volatile int _inpos = 0;   // position where next received byte should go in headerArray

// called upon usart RX to handle incoming byte
// writes to _inbuf until full, then copies to inbox
void myRXHandler(uint8_t data){
		LEDOn(LED1);
		uint8_t* arr = (uint8_t*)(&_inbuf);
 		arr[_inpos] = data;
		_inpos++;
		if (_inpos >= sizeof(header_t)){
			_inpos = 0;
			inbox = _inbuf; // header complete, copy to visible header

			// this seems like a good point to read any incoming messages
			// TODO(a): only one trigLev variable
			triggerLevel = inbox.trigLev;
			if(triggerLevel > 4094){
				triggerLevel = 4096;
			}
			if(triggerLevel < 1000){
				triggerLevel = 1000; // TODO(a): change, only for frame sync
			}
			if(inbox.timeBase < 41){
				inbox.timeBase = 41;
			}
			if(inbox.timeBase > 4199){
				inbox.timeBase = 4199;
			}
			if(inbox.timeBase != ADC_PERIOD){
				init_clock(inbox.timeBase, SAMPLES);     
				ADC_PERIOD = inbox.timeBase;
				enable_clock();
			}
		}
		LEDOff(LED1);
}


// Called at the end of TIM3_IRQHandler.
void TIM3_IRQHook(){
	if( state == STATE_PROCESS ) {
		state = STATE_OVERFLOW;
	} else {
		state = STATE_PROCESS;
	}

}
 
int main(void) {
	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	// ADC
	samplesBuffer   = malloc(sizeof(uint16_t)*SAMPLES*4);
	memset((void*)samplesBuffer, 0, sizeof(uint16_t)*SAMPLES*4);

	// outbound communication
	int headerBytes = sizeof(header_t);
	int dataBytes = SAMPLES*sizeof(outData[0]);
	usartBuf	    = malloc(headerBytes + dataBytes); 
	memset(usartBuf, 0, headerBytes + dataBytes);

	outbox = (header_t*)(usartBuf);                      // header is embedded in beginning of usart buffer
	outData = (uint16_t*)(&usartBuf[headerBytes]);       // data is embedded next

	// triggering

	triggerFrame = 3;
	transmitting = 0;
	triggerLevel = (0xFFF >> 1); // trigger halfway

	init_clock(ADC_PERIOD, SAMPLES);     
	clock_TIM3_IRQHook = TIM3_IRQHook;  // Register TIM3_IRQHook to be called at the end of TIM3_IRQHandler
	init_ADC(samplesBuffer, SAMPLES);
	init_USART1(115200);
	USART1_RXHandler = myRXHandler;
	init_analogIn();
	init_LEDs();
	
	state = STATE_IDLE;

	enable_clock();

	LEDOn(LED1);

	while(1) {
		if( state == STATE_PROCESS ) {
			GPIO_SetBits(GPIOD, GPIO_Pin_13);
			/*
			* TRIGGER DETECTION
			* The while loop has been unrolled four times, to avoid unnecessary overhead
			* and the last four samples have been cut-off from the loop to account for possible
			* pointer wrapping into the start of the circular ADC buffer.
			*/
			uint16_t *triggerPoint = NULL;
			uint16_t *sptr = (uint16_t *) (samplesBuffer + SAMPLES*triggerFrame);
			uint16_t x0, x1, x2, x3, x4;
			uint32_t N = (SAMPLES>>2)-2;

			x0 = *sptr++;
			x1 = *sptr++;
			x2 = *sptr++;
			x3 = *sptr++;
			x4 = *sptr; // x0 = x4 in the next iteration
			do {
				if(x0 > triggerLevel) {
					if( x1 > x0 ) {
						triggerPoint = sptr-4;
						N = 0;
					}
				} else if( (N != 0) && (x1 > triggerLevel)) {
					if( x2 > x1 ) {
						triggerPoint = sptr-3;
						N = 0;
					}
				} else if((N != 0) && (x2 > triggerLevel) ) {
					if( x3 > x2 ) {
						triggerPoint = sptr-2;
						N = 0;
					}
				} else if((N != 0) && (x3 > triggerLevel) ) {
					if( x4 > x3 ) {
						triggerPoint = sptr-1;
						N = 0;
					}
				}

				// Prepare the data for the next iteration
				x0 = *sptr++;
				x1 = *sptr++;
				x2 = *sptr++;
				x3 = *sptr++;
				x4 = *sptr; // x0 = x4 in the next iteration
			} while(N--);
			// Process last 4 samples manually
			// not yet implemented

			// Update the triggerFrame
			triggerFrame++;
			if( triggerFrame == 4 ) {
				triggerFrame = 0;
			}

			/*
			* DATA TRANSFER (IF USART IDLE)
			*/
			if(triggerPoint && !transmitting) {
				// Copy the data, using memcpy for speed reasons
				if(triggerFrame > 0) {
					// Data was from frame 0..2
					memcpy((void*)(outData), (void*)triggerPoint, SAMPLES*2);
				} else {
					// This is the number of samples till we wrap to the first frame
					int32_t samples = (int32_t)(samplesBuffer + 4*SAMPLES - 1 - triggerPoint);
					// A block needs to be copied from the last frame
					memcpy((void*)(outData), (void*)triggerPoint, samples*2);
					// and a part from the first frame
					memcpy((void*)(outData+samples), (void*)samplesBuffer, (SAMPLES-samples)*2);
				}

				GPIO_SetBits(GPIOD, GPIO_Pin_14);

				outbox->magic = 0xFFFFFFFF;
				outbox->samples = inbox.samples; // test transmission, TODO(a): change
				outbox->trigLev = triggerLevel;
				outbox->timeBase = ADC_PERIOD;
				USART_asyncTX(usartBuf, headerBytes + dataBytes);

			}
			GPIO_ResetBits(GPIOD, GPIO_Pin_13);
			state = STATE_IDLE;
		} else if (state == STATE_OVERFLOW ) {
			GPIO_SetBits(GPIOD, GPIO_Pin_12);
		}
	}
}



