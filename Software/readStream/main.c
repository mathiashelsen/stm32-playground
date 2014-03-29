#include <stdio.h>
#include <stdlib.h>
#include "io.h"

int main(void)
{
    int fd = 0;
    initSerial(&fd, 115200, "/dev/ttyUSB0");
    uint8_t bytes[1024*2*2];
    myReadfull( fd, bytes, 1024*2*2);
    for(int i = 0 ; i < 1024*2; i+=2 )
    {
	printf("0x%x\t0x%x\n", bytes[i], bytes[i+1]);	
    }
}
