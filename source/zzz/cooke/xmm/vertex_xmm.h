// vertex_xmm.h

#ifndef __VERTEX_XMM_H
#define __VERTEX_XMM_H

#include "XmmUtil.h"
#ifdef __DO_XMM_BUILD

#ifdef _USE_INTEL_COMPILER

#include "debug_memory.h"
#include "vector_xmm.h"
#include "ColorValue_xmm.h"

struct VertexXmm
{
  union
  {
    struct
    {
      VectorXmm VV;

      union
      {
        __m128 RHW;
        __m128 W;
      };
    };
    
    struct
    {
      __m128 V0, V1, V2, V3;
    };
  };

  union
  {
    struct
    {
      VectorXmm NV;
    };

    struct
    {
      __m128 N0, N1, N2, N3;
    };
  };

  ColorValueXmm DIFFUSE, SPECULAR;

  void *operator new(size_t c)
  {
    return (void*) Debug::Memory::Aligning::AligningAlloc( c * sizeof(VertexXmm), 4);
  }

  void operator delete(void *data)
  {
    Debug::Memory::Aligning::AligningFree( data);
    data = NULL;
  }
};

#else

struct VertexXmm;

#endif _USE_INTEL_COMPILER

#endif __DO_XMM_BUILD

#endif __VERTEX_XMM_H