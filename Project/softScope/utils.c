#include "utils.h"

void memcpy32(void *dest, void *src, uint32_t nBytes)
{
    uint32_t N = nBytes >> 4;
    uint32_t *d = (uint32_t *) dest;
    uint32_t *s = (uint32_t *) src;
    do
    {
	*d++ = *s++;
	*d++ = *s++;
	*d++ = *s++;
	*d++ = *s++;
    }while(--N);
}
