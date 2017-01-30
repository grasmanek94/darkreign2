// xmm.cpp

#ifdef __DO_XMM_BUILD

#include "xmm.h"
#include "debug_memory.h" // AligningAlloc() and AligningFree()

void *new_m128(U32 c)
{
  return (void*) Debug::Memory::Aligning::AligningAlloc( c * sizeof(__m128), 4);
}

void delete_m128(__m128 *data)
{
  Debug::Memory::Aligning::AligningFree(data);
}

#endif __DO_XMM_BUILD