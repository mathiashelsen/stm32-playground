#include "io.h"

static int set_interface_attribs (int fd, int speed, int parity);
static void set_blocking (int fd, int should_block);

int myReadfull( int fd, void *buffer, int nbytes )
{
    memset( buffer, 0, nbytes );
    uint8_t *buffercast = (uint8_t *) buffer;
    int bytesread = 0;
    bytesread = read(fd, buffercast, nbytes);
    while( bytesread < nbytes && bytesread >= 0 )
    {
	bytesread += read(fd, &(buffercast[bytesread]), nbytes - bytesread);
    }
    assert(bytesread > 0);
    return bytesread;
}

int myRead( int fd, void *buffer, int nbytes )
{
    memset( buffer, 0, nbytes );
    uint8_t *buffercast = (uint8_t *) buffer;
    int bytesread = 0;
    bytesread = read(fd, buffercast, nbytes);
    return bytesread;
}



int initSerial( int *fd, int baudrate, char *devname )
{
    *fd = open( devname, O_RDWR | O_NOCTTY | O_SYNC );
    if (*fd < 0)
    {
        fprintf(stderr, "! ERROR: Could not open serial port!\n");
        perror("! ERROR Message from system");
        return EXIT_FAILURE;
    }
	
    set_interface_attribs (*fd, baudrate, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (*fd, 0);                // set no blockin  

    return EXIT_SUCCESS;
}

static int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof(struct termios) ); 
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // ignore break signal
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        //tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf("error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

static void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                printf("error %d setting term attributes", errno);
}

int myWrite( int fd, const char *format, ...)
{
    char buffer[1024];
    memset(buffer, 0, 1024);
    va_list vl;
    va_start( vl, format );
    int ret = vsnprintf( buffer, 1024, format, vl );
    va_end(vl);
    assert( (ret > 0) && (ret < 1024) );
    write( fd, buffer, strlen(buffer) );
    return SUCCESS;
}
