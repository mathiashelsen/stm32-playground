#include "utils.h"

void memcpy16(uint16_t *dest, uint16_t *src, uint32_t nBytes)
{
    uint32_t N = nBytes >> 3; // Transfer 4 uint16_t's that are 2 bytes wide -> div by 8
    do
    {
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
    }while(--N);
}
