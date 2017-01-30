///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Bucket.h
//
//

#ifndef __BUCKETDESC_H
#define __BUCKETDESC_H

#include "primitive.h"
#include "vertex.h"
//-----------------------------------------------------------------------------

#define DEF_BUCKET_RATIO		  0.95f
#define DEF_PRIM_SIZE_RATIO		1.0f
#define DEF_VERTEX_TYPE				FVF_TLVERTEX
#define DEF_TEXTURE_COUNT			0
#define DEF_FULL_BUCKET_RATIO	0.90f
//-----------------------------------------------------------------------------

class Bucket : public PrimitiveDesc
{
//protected:
public:
  friend class BucketMan;
  friend class TranBucketMan;

  void									*vMem, *vMemCur;
	U16										*iMem, *iMemCur;

  char                  *memEnd;

  U32										vCount, iCount, memSize;
  U32										vCountLock, iCountLock;
  U32										vCountLeft, iCountLeft;
  U32										vCountMax,  iCountMax;
  U32                   vCountFull, iCountFull;

  U32                   oversize : 1;

  BucketMan             *manager;

  void *                id;
  U32                   offset;

  inline Bool HasRoomFor( U32 _vCount, U32 _iCount)
  {
    return _vCount <= vCountLeft && _iCount <= iCountLeft;
  }
  inline Bool HasRoomForWhenFlushed( U32 _vCount, U32 _iCount)
  {
    return _vCount <= vCountMax && _iCount <= iCountMax;
  }
	inline Bool	IsFull()
  {
    return vCount > vCountFull || iCount > iCountFull ? TRUE : FALSE;
  }

  void LockIndexedPrimitiveMem( void **_vMem, U16 **_iMem, U32 _vCount, U32 _iCount);
  void UnlockIndexedPrimitiveMem( U32 _vCount, U32 _iCount, Bool doOffset = TRUE);
  void UnlockIndexedPrimitiveMemManager( U32 _vCount, U32 _iCount, Bool doOffset = TRUE);

  void LockPrimitiveMem( void **_vMem, U32 _vCount);
  void UnlockPrimitiveMem( U32 _vCount);
  void UnlockPrimitiveMemManager( U32 _vCount);

public:
  NList<Bucket>::Node    listNode;         // node for BucketManager

	Bucket( BucketMan *man);
  ~Bucket() {};

	void						      ClearData();
	void						      Reset();
  void						      ResetMem();
	void						      ResetCounts();

  void                  Sort();

	inline void *	GetLockedVertexMem()
  {
	  ASSERT( IsVertexMemLocked() );
	  return vMemCur;
  }
	inline U16 * GetLockedIndexMem()
  {
	  ASSERT( IsIndexMemLocked() );
	  return iMemCur;
  }
	inline Bool	IsVertexMemLocked()
  {
	  return ((Bool) vCountLock != 0);
  }
	inline Bool	IsIndexMemLocked()
  {
	  return ((Bool) iCountLock != 0);
  }
	inline Bool	IsPrimitiveMemLocked()
  {
	  return (IsVertexMemLocked() || IsIndexMemLocked());
  }
  inline Bool	IsLocked()
  {
    return IsVertexMemLocked();
  }

  friend int _cdecl ComparePrimitives( const void *e1, const void *e2);
};
//-----------------------------------------------------------------------------

// description of how to lock memory for a group of polys
//
struct BucketDesc
{
  Material *    material;
  Bitmap *      texture0;
  Bitmap *      texture1;

  U32           flags0;
  U32           flags1;

  U32           teamColor   : 1;
  U32           envMap      : 1;
  U32           overlay     : 1;

  ColorF32      diff;             // temp lighting; F32's

  U32           vertCount;        // max possible
  U32           indexCount;       // max possible

  void ClearData()
  {
    material = NULL;
    texture0 = texture1 = NULL;

    teamColor = envMap = overlay = FALSE;
  }
};
//-----------------------------------------------------------------------------

// data for an actual bucket memory lock for a group of polys
//
struct BucketLock : BucketDesc
{
  // initial color values (ambient)
  // 
  ColorF32      diffInitF32;
  Color         diffInitC;

  // offset to subtract when copying indices
  //
  U32           offset;

  // live rendered data for MeshEffects reuse
  //
  Bucket *      bucket;
  VertexTL *    vert;     // pointer into bucket
  U32           vCount;
  U16 *         index;
  U32           iCount;

  void *        data;

  typedef void (BucketLock::* LIGHTPROC)( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const;
  LIGHTPROC     lightProc;

  inline void Light( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const
  {
    (this->*lightProc)( dst, src, norm, color);
  }

  void LightCam( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const;
  void LightMod( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const;
  void LightDecal( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const;

  // in bucket_inline.h
  //
  inline void LightCamInline( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const;
  inline void LightModInline( VertexTL & dst, const Vector & src, const Vector & norm, Color color) const;

  VertexTL * VertexTLMem()
  {
    return ((VertexTL *)vert);
  }
  VertexTL  GetVertexTL( U32 index)
  {
    return VertexTLMem()[index];
  }
  VertexTL & CurrVertexTL()
  {
    return VertexTLMem()[vCount];
  }

  VertexL * VertexLMem()
  {
    return ((VertexL *)vert);
  }
  VertexL & GetVertexL( U32 index)
  {
    return VertexLMem()[index];
  } 
  VertexL & CurrVertexL()
  {
    return VertexLMem()[vCount];
  }

  Vertex * VertexMem()
  {
    return ((Vertex *)vert);
  }
  Vertex & GetVertex( U32 index)
  {
    return VertexMem()[index];
  }
  Vertex & CurrVertex()
  {
    return VertexMem()[vCount];
  }

  VertexT2 * VertexT2Mem()
  {
    return ((VertexT2 *)vert);
  }
  VertexT2 & GetVertexT2( U32 index)
  {
    return VertexT2Mem()[index];
  }
  VertexT2 & CurrVertexT2()
  {
    return VertexT2Mem()[vCount];
  }

  VertexC * CVertexMem()
  {
    return ((VertexC *)vert);
  }
  VertexC & GetCVertex( U32 index)
  {
    return CVertexMem()[index];
  }
  VertexC & CurrCVertex()
  {
    return CVertexMem()[vCount];
  }

  VertexT2C * VertexT2CMem()
  {
    return ((VertexT2C *)vert);
  }
  VertexT2C & GetVertexT2C( U32 index)
  {
    return VertexT2CMem()[index];
  }
  VertexT2C & CurrVertexT2C()
  {
    return VertexT2CMem()[vCount];
  }

  U16 * IndexMem()
  {
    return index;
  }

  void SetIndex( U16 index)
  {
    IndexMem()[iCount] = index;
    iCount++;
  }
};
//-----------------------------------------------------------------------------

// faces grouped by material
//
struct FaceGroup : BucketLock
{
  Array<FaceObj>     faces;

  GeoCache           geo;

  U32 faceCount;

  void ClearData()
  {
    BucketLock::ClearData();
    faces.ClearData();
    geo.ClearData();
  }

  void Release()
  {
    faces.Release();
    geo.Release();
  }
};
//----------------------------------------------------------------------------

#endif // __BUCKETDESC_H