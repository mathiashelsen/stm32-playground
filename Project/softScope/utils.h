#ifndef _UTILS_H
#define _UTILS_H

#include <stdint.h>


// ACHTUNG: Only works if nBytes is a multiple of 16 bytes!
void memcpy32(void *dest, void *src, uint32_t nBytes);

#endif
