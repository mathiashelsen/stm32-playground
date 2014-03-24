#ifndef _TTY_H_
#define _TTY_H_

extern char* TTYErr;

int openTTY(char* file, int baud);

int readTTY(int fd, void *buf, int N);

void closeTTY(int fd);

#endif
