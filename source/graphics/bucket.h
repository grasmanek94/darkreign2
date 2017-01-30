///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// bucket.h
//
//

#ifndef __BUCKET_H
#define __BUCKET_H

#include "vertex.h"
#include "Bucketdesc.h"

#define VERTICES_LOCKED(b)			((b) ? ((b)->IsVertexMemLocked()) : FALSE )
#define INDICES_LOCKED(b)				((b) ? ((b)->IsIndexMemLocked()) : FALSE )
#define PRIMITIVE_LOCKED(b)			((b) ? ((b)->IsPrimitiveMemLocked()) : FALSE )

#define DO_APPEND								0
#define DO_PREPEND							1

//////////////////////////////////////////////////////////////////////////////
//
// BucketMan declaration
//
//////////////////////////////////////////////////////////////////////////////

class BucketMan
{
protected:
  // used by all bucket managers
	static PrimitiveDesc		primitive;
  
  NList<Bucket>				    bucketList;

	Bucket				          *currentBucket;
	Bucket				          *lastUsedBucket;

  char                    *memBlock, *curMem, *lastMem;
  U32                     memSize, curSize, lastSize;

	U32											sizeofBucket;
	F32											memRatio;

public:
  U32         						flushWhenFull : 1;
  
  static Bool             forceTranslucent;
  static Bitmap          * texture;

protected:
	void AddNewBucket( Bucket * newBucket, U8 placement = DO_PREPEND);

  Bool GetMem( Bucket & bucket, U32 _vCount, U32 _iCount);

  void SetCurrentBucket( Bucket * bucket);

public:
  BucketMan()
  {
	  ClearData();
  } 
  ~BucketMan()
  {
	  DisposeAll();
  }
  void ClearData();
	void DisposeAll();

  virtual void Flush(Bool doDraw = TRUE);
  virtual void FlushTex( const Bitmap * texture, Bool doDraw = TRUE);
	virtual void SetPrimitiveDesc( const PrimitiveDesc & primitive);
	virtual void SetPrimitiveDesc( Bucket & bucket, const PrimitiveDesc & primitive);
	virtual Bool CompareRenderState( const PrimitiveDesc & prim) const;

	void Init( U32 _count, U32 _size, F32 _ratio, Bool _flushWhenFull);
	void FlushBucket( Bucket & bucket, Bool doDraw = TRUE);

	static inline void	SetTexture( const Bitmap *texture, U32 stage = 0, U32 blend = RS_BLEND_DEF)
  {
    primitive.SetTexture( texture, stage, blend);
  }

	Bucket * LockIndexedPrimitiveMem( void **vMem, U32 vCount, U16 ** iMem, U32 iCount, const void *id = (const void *)0xcdcdcdcd);
  void UnlockIndexedPrimitiveMem( Bucket & bucket, U32 vCount, U32 iCount, Bool doOffset = TRUE);

  inline void UnlockIndexedPrimitiveMem( U32 vCount, U32 iCount, Bool doOffset = TRUE)
  {
    ASSERT( currentBucket);
    UnlockIndexedPrimitiveMem( *currentBucket, vCount, iCount, doOffset);
  }

	Bucket * LockPrimitiveMem( void **vMem, U32 vCount, const void *id = (const void *)0xcdcdcdcd);
  void UnlockPrimitiveMem( Bucket & bucket, U32 vCount);

  inline void UnlockPrimitiveMem( U32 vCount)
  {
    ASSERT( currentBucket);
    UnlockPrimitiveMem( *currentBucket, vCount);
  }

  inline Bool IsBucketLastStack( const Bucket & bucket)
  {
    return bucket.memEnd == curMem;
  }

  inline Bucket *CurrentBucket()
  {
    return currentBucket;
  }

  inline static const PrimitiveDesc & GetPrimitiveDesc()
  {
    return primitive;
  }
  inline static U32 GetPrimitiveFlags()
  {
    return primitive.flags;
  }

  //  static set primitive desc functions
  //
	inline static void SetPrimitiveDesc(	
    PRIMITIVE_TYPE primitiveType, 
    VERTEX_TYPE vertexType,
		U32 flags) 
  {
	  primitive.SetPrimitiveDesc( primitiveType, vertexType, flags);
  }
	inline static void	SetPrimitiveType( PRIMITIVE_TYPE primitiveType)
  {
  	primitive.primitive_type = primitiveType;
  }
	inline static void	SetVertexType( VERTEX_TYPE vertexType)
  {
  	primitive.vertex_type = vertexType;
  }
	inline static void	SetFlags( U32 flags)
  {
	  primitive.flags = flags;
  }

	inline static U16   GetTag()
  {
	  return primitive.tag0;
  }
	inline static void	SetTag( U16 _tag)
  {
	  primitive.tag0 = _tag;
  }
	inline static U16   GetTag1()
  {
	  return primitive.tag1;
  }
	inline static void	SetTag1( U16 _tag1)
  {
	  primitive.tag1 = _tag1;
  }

	inline static void	SetMaterial( const Material *material)
  {
	  primitive.material = (Material *) material;
  }
	inline static Material * GetMaterial()
  {
	  return primitive.material;
  }
	inline static void	ClearTextures()
  {
	  primitive.ClearTextures();
  }
};
//----------------------------------------------------------------------------

#endif // __BUCKET_H

