///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_heap.h
//
// 17-MAR-2000
//

#ifndef __VIDHEAP_H
#define __VIDHEAP_H

#include "vid_decl.h"
#include "vidclip.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Heap
  {
    extern char * memBlock;
    extern char * endMem;
    extern char * current;

    extern VertexTL * tempVtx;
    extern U16      * tempIdx;

    void Init( U32 size);
    void Init( U32 maxVtx, U32 maxIdx);
    void Done();
    //-----------------------------------------------------------------------------

    inline U32 Size()
    {
      return endMem - memBlock;
    }
    //-----------------------------------------------------------------------------

    inline Bool Check()
    {
      ASSERT( Vid::Heap::current == Vid::Heap::memBlock);
      return Vid::Heap::current == Vid::Heap::memBlock;
    }
    //-----------------------------------------------------------------------------

    inline char * Request( U32 size)
    {
      ASSERT( current && current <= endMem);

      if (current + size > endMem)
      {
        ERR_FATAL(("Vid::Heap::Request: out of space!"));
      }
      char * last = current;

      current += size;
  
      return last;
    }
    //-----------------------------------------------------------------------------

    inline void Restore( U32 size)
    {
      current -= size;

      ASSERT( current >= memBlock);
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVector( Vector ** vector, U32 size)
    {
      size *= sizeof( Vector);

      *vector = (Vector *) Request( size);

      return size;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertex( VertexTL ** vertex, U32 size)
    {
      size *= sizeof( VertexTL);

      *vertex = (VertexTL *) Request( size);

      return size;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertex( Vector ** vector, U32 size0, VertexTL ** vertex, U32 size1)
    {
      size1 = size0 * sizeof( Vector) + size1 * sizeof( VertexTL);

      *vector = (Vector *) Request( size1);
      *vertex = (VertexTL *) *vector + size0;

      return size1;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertex( VertexTL ** vertex)
    {
      U32 size = renderState.maxVerts * sizeof( VertexTL);

      *vertex = (VertexTL *) Request( size);

      return size;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqU16( U16 ** idx, U32 size)
    {
      size <<= 1;

      *idx = (U16 *) Request( size);

      return size;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqU16( U16 ** idx0, U32 size0, U16 ** idx1, U32 size1)
    {
      size1 = (size0 << 1) + (size1 << 1);

      *idx0 = (U16 *) Request( size1 );
      *idx1 = *idx0 + size0;

      return size1;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqU16( U16 ** idx0, U32 size0, U16 ** idx1, U32 size1, U16 ** idx2, U32 size2)
    {
      size2 = (size0 << 1) + (size1 << 1) + (size2 << 1);

      *idx0 = (U16 *) Request( size2 );
      *idx1 = *idx0 + size0;
      *idx2 = *idx1 + size1;

      return size2;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqU16( U16 ** idx0, U16 ** idx1, U16 ** idx2)
    {
      return ReqU16( idx0, renderState.maxVerts, idx1, renderState.maxVerts, idx2, renderState.maxVerts);
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertexI( VertexI ** vertex, U32 sizeV, U16 ** idx0, U32 size0)
    {
      size0 = sizeV * sizeof( VertexI) + (size0 << 1) ;

      *vertex = (VertexI *) Request( size0);
      *idx0 = (U16 *) (*vertex + sizeV); 

      return size0;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertexI2( VertexI2 ** vertex, U32 sizeV, U16 ** idx0, U32 size0)
    {
      size0 = sizeV * sizeof( VertexI2) + (size0 << 1) ;

      *vertex = (VertexI2 *) Request( size0);
      *idx0 = (U16 *) (*vertex + sizeV); 

      return size0;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertexC( VertexC ** vertex, U32 sizeV, U16 ** idx0, U32 size0)
    {
      size0 = sizeV * sizeof( VertexC) + (size0 << 1) ;

      *vertex = (VertexC *) Request( size0);
      *idx0 = (U16 *) (*vertex + sizeV); 

      return size0;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertexV( Vertex ** vertex, U32 sizeV, U16 ** idx0, U32 size0)
    {
      size0 = sizeV * sizeof( Vertex) + (size0 << 1) ;

      *vertex = (Vertex *) Request( size0);
      *idx0 = (U16 *) (*vertex + sizeV); 

      return size0;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertex( VertexTL ** vertex, U32 sizeV, U16 ** idx0, U32 size0)
    {
      size0 = sizeV * sizeof( VertexTL) + (size0 << 1) ;

      *vertex = (VertexTL *) Request( size0);
      *idx0 = (U16 *) (*vertex + sizeV); 

      return size0;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertex( VertexTL ** vertex, U16 ** idx0, U16 ** idx1, U32 size1, U16 ** idx2, U32 size2 )
    {
      size2 = sizeof( VertexTL) * renderState.maxVerts + 
        (renderState.maxIndices << 1) +
        (size1 << 1) + (size2 << 1);

      *vertex = (VertexTL *) Request( size2);
      *idx0 = (U16 *) (*vertex + renderState.maxVerts); 
      *idx1 = *idx0 + renderState.maxIndices;
      *idx2 = *idx1 + size1;

      return size2;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVertex( VertexTL ** vertex, U16 ** idx0)
    {
      U32 sizeV = renderState.maxVerts * sizeof( VertexTL);
      U32 size0 = sizeV + (renderState.maxIndices << 1) ;

      *vertex = (VertexTL *) Request( size0);
      *idx0 = (U16 *) (((char *)*vertex) + sizeV); 

      return size0;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVector( Vector ** vector, U32 sizeV, U8 ** idx0, U32 size0)
    {
      size0 = sizeV * sizeof( Vector) + size0;

      *vector = (Vector *) Request( size0);
      *idx0 = (U8 *) (*vector + sizeV); 

      return size0;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVector( Vector ** vector, U32 sizeV, U16 ** idx0, U32 size0)
    {
      sizeV *= sizeof( Vector);
      size0 = sizeV + (size0 << 1);

      *vector = (Vector *) Request( size0);
      *idx0 = (U16 *) (((char *)*vector) + sizeV); 

      return size0;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVector( Vector ** vector, U32 sizeV, U16 ** idx0, U32 size0, U16 ** idx1, U32 size1)
    {
      sizeV *= sizeof( Vector);
      size1 = sizeV + (size0 << 1) + (size1 << 1);

      *vector = (Vector *) Request( size1);
      *idx0 = (U16 *) (((char *)*vector) + sizeV); 
      *idx1 = *idx0 + size0;

      return size1;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqVector( Vector ** vector, U32 sizeV, U16 ** idx0, U32 size0, U16 ** idx1, U32 size1, U16 ** idx2, U32 size2)
    {
      sizeV *= sizeof( Vector);
      size2 = sizeV + (size0 << 1) + (size1 << 1) + (size2 << 1);

      *vector = (Vector *) Request( size2);
      *idx0 = (U16 *) (((char *)*vector) + sizeV); 
      *idx1 = *idx0 + size0;
      *idx2 = *idx1 + size1;

      return size2;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqMesh( Vector ** vector, U32 sizeV, U16 ** idx0, U32 size0, U16 ** idx1, U32 size1, U16 ** idx2, U32 size2)
    {
      size2 = 
        renderState.maxVerts * sizeof( VertexTL) +  // temp bucky verts
        (renderState.maxIndices << 1) +             // temp bucky idx
        sizeV * sizeof( Vector) +                   // tranform verts
        (size0 << 1) + (size1 << 1) + (size2 << 1); // used verts, norms, uvs

      tempVtx = (VertexTL *) Request( size2);
      tempIdx = (U16 *) (tempVtx + renderState.maxVerts);
      *vector = (Vector *)(tempIdx + renderState.maxIndices);
      *idx0 = (U16 *) (*vector + sizeV); 
      *idx1 = *idx0 + size0;
      *idx2 = *idx1 + size1;

      return size2;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqMesh( Vector ** vector, U32 sizeV, U16 ** idx0, U32 size0, U16 ** idx1, U32 size1)
    {
      size1 = 
        renderState.maxVerts * sizeof( VertexTL) +  // temp bucky verts
        (renderState.maxIndices << 1) +             // temp bucky idx
        sizeV * sizeof( Vector) +                   // tranform verts
        (size0 << 1) + (size1 << 1);                // used verts, uvs

      tempVtx = (VertexTL *) Request( size1);
      tempIdx = (U16 *) (tempVtx + renderState.maxVerts);
      *vector = (Vector *)(tempIdx + renderState.maxIndices);
      *idx0 = (U16 *) (*vector + sizeV); 
      *idx1 = *idx0 + size0;

      return size1;
    }
    //-----------------------------------------------------------------------------

    inline U32 ReqMesh( Vector ** vector, U32 sizeV)
    {
      U32 hs = 
        renderState.maxVerts * sizeof( VertexTL) +  // temp bucky verts
        (renderState.maxIndices << 1) +             // temp bucky idx
        sizeV * sizeof( Vector);                    // tranform verts

      tempVtx = (VertexTL *) Request( hs);
      tempIdx = (U16 *) (tempVtx + renderState.maxVerts);
      *vector = (Vector *)(tempIdx + renderState.maxIndices);

      return hs;
    }
    //-----------------------------------------------------------------------------
  }
};
//----------------------------------------------------------------------------

#endif		// __VIDHEAP_H

