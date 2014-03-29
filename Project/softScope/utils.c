#include "utils.h"

void memcpy32(void *dest, void *src, uint32_t nBytes)
{
    uint32_t N = nBytes >> 4;
    do
    {
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
	*dest++ = *src++;
    }while(--N);
}
