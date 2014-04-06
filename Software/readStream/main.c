#include <stdio.h>
#include <stdlib.h>
#include "io.h"

int main(void)
{
    int fd = 0;
    initSerial(&fd, 115200, "/dev/ttyUSB0");
    uint8_t bytes[1024*64];
    myReadfull( fd, bytes, 1024*64);
    for(int i = 0 ; i < 1024*64; i+=2 )
    {
	printf("%d\t0x%x\t0x%x\n", i, bytes[i], bytes[i+1]);	
    }
}
