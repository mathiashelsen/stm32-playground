#ifndef _IO_H
#define _IO_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <stdarg.h>

#define SUCCESS 1

int myReadfull( int fd, void *buffer, int nBytes );

int myRead( int fd, void *buffer, int nBytes );

int myWrite( int fd, const char *format, ...);

int initSerial( int *fd, int baudrate, char *devname );

#endif
