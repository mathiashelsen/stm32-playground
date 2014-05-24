#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "io.h"

int main(int argc, char **argv)
{
    int fd;
    initSerial( &fd, 115200, "/dev/ttyUSB0" );
    uint16_t out[6];
    out[0]	= 0x0000;
    out[1]	= 0x0020;
    out[2]	= 0x1000;

    out[3]	= 0x0000;
    out[4]	= 0x0200;
    out[5]	= 0x1000;

    uint8_t magic = 0xA3;
    write(fd, &magic, 1);
    int i = 0;
    for( i = 0 ; i < 6 ; i++ )
    {
	write(fd, &(out[i]), 2);
    }

    for( i = 0; i < 8; i++ )
    {
	uint16_t returnValues[128];		
	myReadfull( fd, (void *)returnValues, 128*sizeof(uint16_t) );
	int j = 0;
	for( j = 0; j < 128; j++ )
	{
	    printf("%d\t%d\t%d\n", i*0x0200, j*0x0020, returnValues[j]);
	}
	printf("\n\n");
    }

    return 0;
}
