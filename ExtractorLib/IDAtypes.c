#include "IDAtypes.h"
#include <stdint.h>

void* qmemcpy(void* dst, const void* src, size_t n)
{
  unsigned char* d = (unsigned char*)dst;
  const unsigned char* s = (const unsigned char*)src;

  if (!n || d == s) return dst;


  for (size_t i = 0; i < n; ++i)
    d[i] = s[i];

  return dst;
}
void nullsub_1()
{

}