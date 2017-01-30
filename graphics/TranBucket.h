///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// TranBucket.h
//
//

#ifndef __TRANBUCKET_H
#define __TRANBUCKET_H

#include "bucket.h"

class TranBucketMan : public BucketMan
{
private:

	F32											scaleZ;
	F32											maxBucketCount;
	F32											maxZ;
	F32											minZ;

	void							      UpdateScaleZ();

public:
	
  Bool                    doSort;

	TranBucketMan()
  {
    ClearData();
  }
  ~TranBucketMan() {};

  void ClearData();
  void SetZ( F32 _z);

  inline void SetZNorm( F32 _z)
  {
    SetTag( U16( _z * (maxBucketCount - 1) ));
  }
  inline void SetZSort( U16 tag1)
  {
    SetTag1( tag1);
  }

	virtual void Flush( Bool doDraw = TRUE);
  virtual void FlushTex( const Bitmap * texture, Bool doDraw = TRUE);
	virtual void SetPrimitiveDesc( PrimitiveDesc & primitive);
	virtual void SetPrimitiveDesc( Bucket & bucket, PrimitiveDesc & primitive);
	virtual Bool CompareRenderState( const PrimitiveDesc & prim) const;

  void TranBucketMan::SetMaxZ( F32 _maxZ)
  {
	  maxZ = _maxZ;
	  UpdateScaleZ();
  }

  void TranBucketMan::SetMinZ( F32 _minZ)
  {
	  minZ = _minZ;
	  UpdateScaleZ();
  }

  void TranBucketMan::SetMaxBucketCount( U16 _maxBucketCount)
  {
	  ASSERT( minZ < maxZ );
	  maxBucketCount = (F32) _maxBucketCount;
	  UpdateScaleZ();
  }
};


#endif // __TRANBUCKET_H