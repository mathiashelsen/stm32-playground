#include "utils.h"

void memcpy16(uint16_t *dest, uint16_t *src, uint32_t nBytes)
{
    uint32_t N = nBytes >> 3; // Transfer 4 uint16_t's that are 2 bytes wide -> div by 8
    while(N--)
    {
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
    }

    N = nBytes & 0x7; // For the remaining bytes
    while(N--)
    {
	*dest++ = *src++;
    }
    
}
