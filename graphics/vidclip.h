///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// vidclip.h
//
//


#ifndef __VIDCLIP_H
#define __VIDCLIP_H

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Vid
//
namespace Vid
{
  namespace Clip
  {
    extern Color clipColor;

    // setup
    //
    U32 GetTempMemSize();

    // clip indexed tris to buffer
    //
    void ToBuffer( VertexTL * dstV, U16 * dstI, VertexTL * srcV, U32 & vCount, const U16 * srcI, U32 & iCount, U32 calcFog = TRUE, U32 clipFlags = clipALL);

    // clip indexed tris to bucket
    // return of 0 indicates that bucket memory couldn't be allocated
    //
    Bucket * ToBucket(     VertexTL * srcV, U32 vCount, const U16 * srcI, U32 iCount, const void * id = (void *) 0xcdcdcdcd, U32 calcFog = TRUE, U32 clipFlags = clipALL, BucketLock * bucky = NULL);
    Bucket * ToBucketFogX( VertexTL * srcV, U32 vCount, const U16 * srcI, U32 iCount, const void * id = (void *) 0xcdcdcdcd, U32 calcFog = TRUE, U32 clipFlags = clipALL, BucketLock * bucky = NULL);

    inline Bucket * ToBucket( BucketLock & bucky, const void * id = (void *) 0xcdcdcdcd, U32 clipFlags = clipALL)
    {
      return ToBucket( bucky.vert, bucky.vCount, bucky.index, bucky.iCount, id, TRUE, clipFlags, &bucky);
    }

    namespace Screen
    {
      void ToBuffer( VertexTL * dstV, U16 * dstI, VertexTL * srcV, U32 & vCount, const U16 * srcI, U32 & iCount, U32 clipFlags = clipALL, VertexTL * rect = NULL);
    };

    // in camera space
    //
    Bool BoundsTestAlpha( F32 origin, F32 radius);
    Bool BoundsTestAlphaNear( F32 origin, F32 radius);
    Bool BoundsTestAlphaFar( F32 origin, F32 radius);

    inline Bool BoundsTestAlpha( const Vector & origin, F32 radius)
    {
      return BoundsTestAlpha( origin.z, radius);
    }
    inline Bool BoundsTestAlphaNear( const Vector & origin, F32 radius)
    {
      return BoundsTestAlphaNear( origin.z, radius);
    }
    inline Bool BoundsTestAlphaFar( const Vector & origin, F32 radius)
    {
      return BoundsTestAlphaFar( origin.z, radius);
    }

    namespace Line
    {
      // clip indexed lines to bucket
      //
      Bucket * ToBucket( VertexTL * srcV, U32 vCount, const U16 * srcI, U32 iCount, const void * id = (void *) 0xcdcdcdcd, U32 calcFog = TRUE, U32 clipFlags = clipALL);

      // clip non-indexed lines to bucket
      //
      Bucket * ToBucket( VertexTL * srcV, U32 vCount, const void * id = (void *) 0xcdcdcdcd, U32 calcFog = TRUE, U32 clipFlags = clipALL);
    };

    namespace Xtra
    {
      extern Bool active;
      extern Color clipColor;

      void Init();

      void Set( const Plane & _plane);
      void Set( const Vector & v0, const Vector & v1, const Vector & v2);
      void Reset( const Plane & _plane);
      void Reset( const Vector & v0, const Vector & v1, const Vector & v2);
      void Restore();

      // disable
      //
      void Clear();

      void Invert();
      Bool Revert();

      U32  BoundsTest( const Vector & origin, F32 radius);

      // Xtra::clip indexed tris to buffer
      //
      Bool ToBuffer( VertexTL * dstV, U16 * dstI, const VertexTL * srcV, U32 & vCount, const U16 * srcI, U32 & iCount, U32 clipFlags);

      namespace Line
      {
        // Xtra::clip indexed lines to buffer
        //
        Bool ToBuffer( VertexTL * dstV, U16 * dstI, const VertexTL * srcV, U32 & vCount, const U16 * srcI, U32 & iCount, U32 clipFlags, Vector * traceOrigin = NULL);

        // Xtra::clip non-indexed lines to buffer
        //
        Bool ToBuffer( VertexTL * dstV, const VertexTL * srcV, U32 & vCount, U32 clipFlags, Vector * traceOrigin = NULL);
      }
    };

  };

};

#endif
