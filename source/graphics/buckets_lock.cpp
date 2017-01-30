///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// bucky.cpp
//
//

#include "vid_public.h"
#include "mesh.h"
#include "bucket.h"
#include "perfstats.h"
#include "statistics.h"
//----------------------------------------------------------------------------

void Vid::SetBucket( BucketLock & bucky, U32 _controlFlags, Bitmap * tex, U32 blend) // = NULL, = RS_BLEND_DEF
{
  SetBucketMaterial( bucky.material);

#if 0
  if (blend & RS_TEX_DECAL)
  {
    blend &= ~RS_TEX_DECAL;
    blend |= RS_TEX_MODULATE;
  }
#endif

  if (tex)
  {
    SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | (bucky.flags0 & ~RS_BLEND_MASK) | blend);
    SetBucketTexture( tex, FALSE, 0, blend);
  }
  else
  {
    blend = bucky.flags0 & RS_BLEND_MASK;

    if (blend == RS_BLEND_DECAL)
    {
      blend = RS_BLEND_MODULATE;
    }

    SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | (bucky.flags0 & ~RS_BLEND_MASK) | blend);
    SetBucketTexture( bucky.texture0, FALSE, 0, blend);

    if (bucky.overlay && (_controlFlags & controlOVERLAY1PASS))
    {
      SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
    }
  }
}
//----------------------------------------------------------------------------

Bool Vid::LockBucket( BucketLock & bucky, U32 _controlFlags, const void * id, Bitmap * tex, U32 blend) // = clipNONE, = 0xcdcdcdcd, = NULL, = RS_BLEND_DEF
{
  SetBucket( bucky, _controlFlags, tex, blend);

  if (bucky.indexCount)
  {
    if (!LockIndexedPrimitiveMem( (void **)&bucky.vert, bucky.vertCount, &bucky.index, bucky.indexCount, id))
    {
	  	return FALSE;
    }
  	ASSERT( bucky.index );
  }
  ASSERT( bucky.vert );

  bucky.bucket = currentBucketMan->CurrentBucket();

	return TRUE;
}
//----------------------------------------------------------------------------

Bool Vid::LockBucket( BucketLock & bucky, U32 _controlFlags, U32 vCount, U32 iCount, const void * id, Bitmap * tex, U32 blend) // = clipNONE, = 0xcdcdcdcd, = NULL, = RS_BLEND_DEF
{
  SetBucket( bucky, _controlFlags, tex, blend);

  if (bucky.indexCount)
  {
    if (!LockIndexedPrimitiveMem( (void **)&bucky.vert, vCount, &bucky.index, iCount, id))
    {
	  	return FALSE;
    }
  	ASSERT( bucky.index );
  }
  ASSERT( bucky.vert );

  bucky.bucket = currentBucketMan->CurrentBucket();

	return TRUE;
}
//----------------------------------------------------------------------------

void Vid::UnLockBucket( BucketLock & bucky)
{
  ASSERT( bucky.bucket && !(bucky.bucket->flags & RS_NOINDEXED));

  bucky.bucket->UnlockIndexedPrimitiveMemManager( bucky.vCount, bucky.iCount, TRUE);
  bucky.offset = bucky.bucket->offset;
}
//----------------------------------------------------------------------------

Bool Vid::LockBucket( BucketLock & bucky, U32 _controlFlags, U32 clipFlags, const void * id, Bitmap * tex, U32 blend) // = clipNONE, = 0xcdcdcdcd, = NULL, = RS_BLEND_DEF
{
  // clear bucky counts
  bucky.vCount = 0;
  bucky.iCount = 0;

  if (clipFlags == clipNONE)
  {
    if (!LockBucket( bucky, _controlFlags, id, tex, blend))
    {
      return FALSE;
    }
  }
  else
  {
    SetBucket( bucky, _controlFlags, tex, blend);

    bucky.bucket = NULL;
    bucky.vert  = Heap::tempVtx;
    bucky.index = Heap::tempIdx;
  }

	return TRUE;
}
//----------------------------------------------------------------------------

Bool Vid::UnLockBucket( BucketLock & bucky, U32 clipFlags, const void * id) // = 0xcdcdcdcd
{
  ASSERT( bucky.vCount <= Vid::renderState.maxVerts && bucky.iCount <= Vid::renderState.maxIndices);

  if (clipFlags == clipNONE)
  {
    UnLockBucket( bucky);

#ifdef DOSTATISTICS
    Statistics::noClipTris += bucky.iCount / 3;
#endif
  
    return TRUE;
  }
  // clipFlags != clipNONE      

  return Clip::ToBucket( bucky, id, clipFlags) != NULL;
}
//----------------------------------------------------------------------------

/*

Bool Vid::LockBuckets( FaceGroup * buckys, U32 count, U32 _controlFlags, const void * id) // = 0xcdcdcdcd
{
  for ( FaceGroup * b = buckys, * e = buckys + count; b < e; b++)
  {
    if (!LockBucket( *b, _controlFlags, id))
    {
      for (--b; b >= buckys; b--)
      {
        b->vCount = b->iCount = 0;
        UnLockBucket( *b);
      }
      return FALSE;
    }
	}
	return TRUE;
}
//----------------------------------------------------------------------------

void Vid::UnLockBuckets( FaceGroup * buckys, U32 count)
{
  // unlock in reverse order of lock
  //
  for ( FaceGroup * b = buckys + count - 1; b >= buckys; b--)
  {
    UnLockBucket( *b);
	}
}
//----------------------------------------------------------------------------

*/