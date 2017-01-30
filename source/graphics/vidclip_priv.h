///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// vidclip.h
//
//


#ifndef __VIDCLIP_PRIV_H
#define __VIDCLIP_PRIV_H

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Vid
//
namespace Vid
{
  namespace Clip
  {
    #define MAXCLIPCOUNT      44 // is this enough?

    #define MIN_X  0x01
    #define MAX_X  0x10
    #define MIN_Y  0x02
    #define MAX_Y  0x20
    #define MIN_Z  0x04
    #define MAX_Z  0x40
    #define CLIPMASK    (MIN_X | MAX_X | MIN_Y | MAX_Y | MAX_Z | MIN_Z)

    #define PLANE_MIN_X 0x00000000
    #define PLANE_MAX_X 0x80000000
    #define PLANE_MIN_Y 0x00000001
    #define PLANE_MAX_Y 0x80000001
    #define PLANE_MIN_Z 0x00000002
    #define PLANE_MAX_Z 0x80000002

    #define INDEX_MASK  0x00000003
    #define SIGN_MASK   0x80000000
    //-----------------------------------------------------------------------------

    extern VertexTL *   clipPool0;
    extern VertexTL *   clipPool1;
    extern VertexTL *   clipPool2;
    extern VertexTL *   clipDst;

    extern U32          inPoolCount;
    extern VertexTL **  inPool;
    extern VertexTL **  outPool;
    extern VertexTL **  outDst;

    extern VertexTL **  tmpVertPtrs;
    extern U16 *        tmpIndices0;
    extern U16 *        tmpIndices1;
    extern U16 *        idx;
    extern U8 *         clipFlagA;
    extern U8 *         faceCountV;

    extern U32          tempMemSize;
    //-----------------------------------------------------------------------------

    void SetClipFlags( VertexTL * srcV, U32 vCount);

    //-----------------------------------------------------------------------------

    inline U32 GetTempMem()
    {
/*
      tempMemSize = 2 * renderState.maxVerts * sizeof( VertexTL)
        +   renderState.maxVerts * sizeof( VertexTL *)
        + ((renderState.maxIndices + renderState.maxVerts) << 1)
        +   renderState.maxVerts + renderState.maxTris;
*/

      clipPool0   = (VertexTL *) Vid::Heap::Request( tempMemSize);
      clipPool1   = clipPool0 + renderState.maxVerts;
      clipPool2   = clipPool1 + renderState.maxVerts;
      tmpVertPtrs = (VertexTL **) (clipPool2 + renderState.maxVerts);
      tmpIndices0 = (U16 *) (tmpVertPtrs + renderState.maxVerts);
      tmpIndices1 = (U16 *) (tmpIndices0 + renderState.maxIndices);
      idx         = tmpIndices1 + renderState.maxIndices;
      clipFlagA   = (U8 *) (idx + renderState.maxVerts);
      faceCountV  = clipFlagA + renderState.maxVerts;

      return tempMemSize;
    }
    //-----------------------------------------------------------------------------

    inline void RestoreTempMem()
    {
      Vid::Heap::Restore( tempMemSize);
    }
    //-----------------------------------------------------------------------------

    inline void SetupPool( VertexTL ** vPtrs0, VertexTL ** vPtrs1)
    {
	    inPool  = vPtrs0;
	    outPool = vPtrs1;
      outDst = outPool;
    }
    //-----------------------------------------------------------------------------

    inline void SwapPool()
    {
      ASSERT( inPoolCount  < MAXCLIPCOUNT);

      VertexTL ** tmp = inPool;
      inPool  = outPool;
      inPoolCount  = outDst - outPool;
      outPool = tmp;
      outDst = outPool;

      ASSERT( inPoolCount  < MAXCLIPCOUNT);
    }
    //-----------------------------------------------------------------------------

    inline F32 Distance( U32 plane, const VertexTL & v)
    { 
      U32 index = plane & INDEX_MASK;           // mask everything except LS 2 bits
      U32 sign  = plane & SIGN_MASK;            // mask everything except sign bit

      ASSERT( index < 3 );

      F32 val_F32 = ((F32 *) &(v.vv.x))[index]; // copy the value using the index from plane
      U32 & val_U32 = *((U32 *) &val_F32);      // for doing bit-wise operations on val_F32's memory
      val_U32 ^= sign;                          // set the sign of the floating point value

      // nearplane is the only exception
      // switch (plane)
      //  case PLANE_MIN_X:
      //    distance = v.rhw + v.vv.x;
		  //  case PLANE_MAX_X:
			//    distance = v.rhw - v.vv.x;

      return plane == PLANE_MIN_Z ? v.vv.z : (v.rhw + val_F32); // calculate the distance
    }
    //-----------------------------------------------------------------------------

  };
};

#endif
