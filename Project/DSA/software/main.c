#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "io.h"

int main(int argc, char **argv)
{
    volatile int fd;
    initSerial( &fd, 115200, "/dev/ttyUSB0" );

    uint8_t magic = 0xA3;
    write(fd, &magic, 1);

    uint32_t nBytes = 0;
    myReadfull( fd, (void *)&nBytes, 4);
    if( nBytes > 0 )
    {
	int16_t returnValues[nBytes/2];
	myReadfull( fd, (void *)returnValues, nBytes );
	int j = 0;
	for( j = 0; j < nBytes/2; j++ )
	{
	    printf("%d\n", returnValues[j]);
	}
	printf("\n\n");
    }

    return 0;
}
