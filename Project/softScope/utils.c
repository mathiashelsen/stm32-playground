#include "utils.h"

void memcpy32(uint32_t *dest, uint32_t *src, uint32_t nBytes)
{
    uint32_t N = nBytes >> 4; // Transfer 4 uint32_t's that are 4 bytes wide -> div by 16
    while(N--)
    {
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
    }

    N = (nBytes & 0xF) >> 2; // Transfer the remaining uint32_t's, one by one
    while(N--)
    {
	*dest++ = *src++;
    }
}
