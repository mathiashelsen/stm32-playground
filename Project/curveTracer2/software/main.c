#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "io.h"

int main(int argc, char **argv)
{
    volatile int fd;
    initSerial( &fd, 115200, "/dev/ttyUSB0" );
    uint16_t out[6];
    out[0]	= 0x0000;
    out[1]	= 0x0020;
    out[2]	= 0x0800+19;

    out[3]	= 0x0000;
    out[4]	= 0x0080;
    out[5]	= 0x0800+79;

    uint8_t magic = 0xA3;
    write(fd, &magic, 1);
    int i = 0;
    for( i = 0 ; i < 6 ; i++ )
    {
	write(fd, &(out[i]), 2);
    }

    i = 0;
    uint16_t nBytes = 0;
    myReadfull( fd, (void *)&nBytes, 2);
    while( nBytes > 0 )
    {
	uint8_t returnValues[129*2];
	myReadfull( fd, (void *)returnValues, nBytes );
	int j = 0;
	for( j = 0; j < nBytes; j+=2 )
	{
	    double VDS = 0.0, IDS = 0.0, VGS = 0.0;
	    VDS = (double)(j * out[1] >> 1)*0.0007326007326007326;
	    VGS = (double)(i * out[4])*0.00147168881425233645;
	    IDS	= (double)((returnValues[j+1] << 8) | (returnValues[j])) * 4.69615854231238846623e-6;

	    printf("%e\t%e\t%e\n", VGS, VDS, IDS);
	}
	printf("\n\n");
	myReadfull( fd, (void *)&nBytes, 2);
	i++;
    }

    return 0;
}
