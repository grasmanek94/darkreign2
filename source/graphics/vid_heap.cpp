///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vidcmd.cpp
//
// 14-NOV-1999
//

#include "vid_public.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Heap
  {
    char * memBlock = NULL;
    char * endMem   = NULL;
    char * current  = NULL;

    VertexTL * tempVtx = NULL;
    U16      * tempIdx = NULL;
    //-----------------------------------------------------------------------------

    void Init( U32 size)
    {
      if (memBlock)
      {
        Done();
      }
      current = memBlock = new char[size];
      endMem  = memBlock + size;
    }
    //-----------------------------------------------------------------------------

    void Init( U32 maxVtx, U32 maxIdx)
    {
      maxVtx = Max<U32>( 256, Min<U32>( U32_MAX >> 8, maxVtx)); 
      maxIdx = Max<U32>( 256, Min<U32>( U32_MAX >> 8, maxIdx)); 

      renderState.maxVerts = maxVtx;
      renderState.maxIndices = maxIdx;
      renderState.maxTris = maxIdx / 3;

      U32 iface = sizeof(VertexTL) * maxVtx + sizeof(U16) * maxIdx;
      U32 mesh  = sizeof(Vector)   * maxVtx + sizeof(U16) * 3 * maxVtx
        +         sizeof(VertexTL) * maxVtx + sizeof(U16) * 3 * maxIdx;

      U32 clip  = Clip::GetTempMemSize();

      Init( iface + mesh + clip);
    }
    //-----------------------------------------------------------------------------

    void Done()
    {
      if (memBlock)
      {
        delete [] memBlock;
      }
      memBlock = NULL;
    }
    //-----------------------------------------------------------------------------
  };
  //----------------------------------------------------------------------------
};