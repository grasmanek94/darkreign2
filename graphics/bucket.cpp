///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// bucket.cpp
//
//

#include "vid_public.h"
#include "mesh.h"
#include "bucket.h"
#include "perfstats.h"
#include "statistics.h"

#ifdef __DO_XMM_BUILD
#include "debug_memory.h"
#endif
//----------------------------------------------------------------------------

PrimitiveDesc		  BucketMan::primitive;
Bool              BucketMan::forceTranslucent;
Bitmap *          BucketMan::texture;
//----------------------------------------------------------------------------

#define DOLASTBUCKET

//#define DEF_BUCKET_COUNT				    32
//#define DEF_BUCKET_SIZE             64000
#define DEF_BUCKET_COUNT				    444
#define DEF_BUCKET_SIZE             16000

#define DEF_TRAN_BUCKET_COUNT		    200
#define DEF_TRAN_BUCKET_SIZE        16000
//-----------------------------------------------------------------------------

void Vid::InitBuckets()
{
  InitBuckets( DEF_BUCKET_COUNT, DEF_BUCKET_SIZE, DEF_BUCKET_RATIO, TRUE,
    DEF_TRAN_BUCKET_COUNT, DEF_TRAN_BUCKET_SIZE, DEF_BUCKET_RATIO);
}
//-----------------------------------------------------------------------------

void Vid::InitBuckets(U32 count, U32 size, F32 ratio, Bool flush, U32 tcount, U32 tsize, F32 tratio)
{
	currentBucketMan = &bucket;

	bucket.Init(count, size, ratio, flush);
	tranbucket.Init(tcount, tsize, tratio, FALSE);

	SetBucketPrimitiveDesc(
		PT_TRIANGLELIST,
		FVF_TLVERTEX,
		DP_DONOTUPDATEEXTENTS | RS_BLEND_DEF);
}
//-----------------------------------------------------------------------------

void Vid::DoneBuckets()
{
  bucket.DisposeAll();
  tranbucket.DisposeAll();
}
//-----------------------------------------------------------------------------

void BucketMan::ClearData()
{
	currentBucket = NULL;
	lastUsedBucket = NULL;
	primitive.ClearData();

	sizeofBucket = 0;
	memRatio = 1.0f;
	flushWhenFull = TRUE;

  memSize = curSize = lastSize = 0;
  memBlock = curMem = lastMem = NULL;

  bucketList.SetNodeMember( &Bucket::listNode);

  forceTranslucent = FALSE;
  texture = NULL;
}
//----------------------------------------------------------------------------

void BucketMan::SetPrimitiveDesc( const PrimitiveDesc &prim)
{
  primitive.SetPrimitiveDesc( prim);
}
//----------------------------------------------------------------------------

void BucketMan::SetPrimitiveDesc( Bucket &bucket, const PrimitiveDesc &prim)
{
  bucket.SetPrimitiveDesc( prim);
}
//----------------------------------------------------------------------------

Bool BucketMan::CompareRenderState( const PrimitiveDesc & other) const 
{
	if (
#ifndef DODXLEANANDGRUMPY
		 primitive.material       == other.material       &&
		 primitive.vertex_type    == other.vertex_type    &&
#endif
     primitive.primitive_type == other.primitive_type &&
  	 primitive.flags          == other.flags          &&
     primitive.texture_count  == other.texture_count)
	{
		for ( U32 i = 0; i < primitive.texture_count; i++ )
		{
			if (primitive.textureStages[i] != other.textureStages[i] )
			{
				return FALSE;
			}
		}

		return TRUE;
	}
	return FALSE;
}
//-----------------------------------------------------------------------------

void BucketMan::AddNewBucket(Bucket *new_bucket, U8 placement) //  = DO_PREPEND);
{
	ASSERT( new_bucket );

	// add bucket to list
	if ( placement == DO_APPEND )
	{
		bucketList.Append(new_bucket);
	}
	else
	{
		bucketList.Prepend(new_bucket);
	}
}
//----------------------------------------------------------------------------

void BucketMan::DisposeAll()
{
	if (memBlock)
	{
    Debug::Memory::Aligning::AligningFree( memBlock);
		memBlock = NULL;
	}

	bucketList.DisposeAll();

	currentBucket = NULL;
	lastUsedBucket = NULL;
	primitive.ClearData();
}
//----------------------------------------------------------------------------

void BucketMan::Init( U32 _count, U32 _size, F32 _ratio, Bool _flushWhenFull)
{
	U32 bucketCount = bucketList.GetCount();

	if (_count         == bucketCount      
   && _size          == sizeofBucket
   && _ratio         == memRatio         
   && _flushWhenFull == (Bool) flushWhenFull)
	{
		return;
	}

	ASSERT( _ratio > 0.0f );
	ASSERT( _ratio <= 1.0f );

	sizeofBucket  = _size;
	memRatio      = _ratio;
	flushWhenFull = _flushWhenFull;

	if (memBlock)
	{
    Debug::Memory::Aligning::AligningFree( memBlock);
		memBlock = NULL;
	}

  memSize = (_count + 5) * _size;
  memBlock = (char *) Debug::Memory::Aligning::AligningAlloc( memSize, 4);
	ASSERT( memBlock );

	U32 i;
	NList<Bucket>::Iterator li(&bucketList); 

	if ( bucketCount < _count )
	{
		for ( i = bucketCount; i < _count; i++ )
		{
			Bucket *temp = new Bucket( this);
			ASSERT( temp );
			AddNewBucket(temp);
		}
	}
	else if ( bucketCount > _count )
	{
		for ( i = bucketCount-1; i > _count-1; i-- )
		{
			Bucket *temp = bucketList[i];
			bucketList.Unlink(temp);
			delete temp;
		}
	}
}
//----------------------------------------------------------------------------

#define FULLBUCKETRATIO   (2.0f / 3.0f)

Bool BucketMan::GetMem( Bucket &bucket, U32 _vCount, U32 _iCount)
{
  ASSERT(!bucket.vMem);

#ifdef DOVERTEXNON32
  U32 vSize = _vCount * bucket.sizeofVertex;
#else
  U32 vSize = _vCount * sizeof( VertexTL);
#endif
  U32 iSize = _iCount << 1;
  U32 size = vSize + iSize;

  if (curSize < size)
  {
    bucket.ClearData();

#ifdef DEVELOPMENT
    static time_t lastT = 0;
    time_t thisT = clock();
    if (thisT - lastT > 5 * CLOCKS_PER_SEC)
    {
      LOG_WARN( ("BucketMan::GetMem: out of bucket memory") );
      lastT = thisT;
    }
#endif
    return FALSE;
  }
  if (size < sizeofBucket)
  {
    U32 bsize = curSize >= sizeofBucket + 10 ? sizeofBucket : curSize;

    U32 extra = bsize - size;
    U32 vs = (U32) Utils::FtoLDown(((F32) extra) * memRatio);
    vSize += vs;
    iSize += extra - vs;
    size = vSize + iSize;

#ifdef DOVERTEXNON32
    bucket.vCountLeft = bucket.vCountMax = vSize / bucket.sizeofVertex;
#else
    bucket.vCountLeft = bucket.vCountMax = vSize / sizeof( VertexTL);
#endif
    bucket.iCountLeft = bucket.iCountMax = iSize >> 1;
    bucket.vCountFull = (U32)Utils::FtoLDown((F32) bucket.vCountMax * FULLBUCKETRATIO);
    bucket.iCountFull = (U32)Utils::FtoLDown((F32) bucket.iCountMax * FULLBUCKETRATIO);
  }
  else 
  {
    bucket.vCountLeft = bucket.vCountMax = _vCount;
    bucket.iCountLeft = bucket.iCountMax = _iCount;
    bucket.vCountFull = (U32)Utils::FtoLDown((F32) bucket.vCountMax * FULLBUCKETRATIO);
    bucket.iCountFull = (U32)Utils::FtoLDown((F32) bucket.iCountMax * FULLBUCKETRATIO);

    bucket.oversize = TRUE;
  }
  bucket.memSize = size;
  bucket.vCount = bucket.iCount = bucket.vCountLock = bucket.iCountLock = 0;

  bucket.vMem = bucket.vMemCur = curMem;
  bucket.iMem = bucket.iMemCur = (U16*)(curMem + vSize);
  bucket.memEnd = curMem + size;

  lastMem  = curMem;
  lastSize = size;

  curMem  += size;
  curSize -= size;

  ASSERT( (void *)(((VertexTL *)bucket.vMemCur) + _vCount) <= (void *)bucket.iMem);
  ASSERT( (void *)(bucket.iMemCur + _iCount) <= (void *)bucket.memEnd);
  ASSERT( (char *)(((VertexTL *)bucket.vMemCur) + _vCount) <= memBlock + memSize);
  ASSERT( (char *)(bucket.iMemCur + _iCount) <= memBlock + memSize);
    
  return TRUE;
}
//-----------------------------------------------------------------------------

void BucketMan::SetCurrentBucket( Bucket * bucket)
{
  ASSERT( bucket);

  currentBucket = bucket;

#ifdef DOLASTBUCKET  
  if (lastUsedBucket == NULL)
  {
    Bucket * head = bucketList.GetHead();

    if (bucket != head)
    {
      bucketList.Unlink( bucket);
      bucketList.InsertBefore( &head->listNode, bucket);
    }
  }
  else if (lastUsedBucket != bucket)
  {
    bucketList.Unlink( bucket);

    bucketList.InsertAfter( &lastUsedBucket->listNode, bucket);
  }

  lastUsedBucket = bucket;
#endif
}
//-----------------------------------------------------------------------------

Bucket * BucketMan::LockIndexedPrimitiveMem( void **vMem, U32 vCount, U16 **iMem, U32 iCount, const void *id) // = 0xcdcdcdcd
{
  ASSERT( !(primitive.flags & RS_NOINDEXED));
	ASSERT( vMem && iMem );

  // pad to multiples of 4 for Katmai
  vCount += 3;

  Bucket * bucket = NULL;

	// see if the current bucket has a matching primitive description
	if (currentBucket && !currentBucket->IsLocked() && CompareRenderState( *currentBucket))
	{
    if (!currentBucket->vMem)
    {
      if (!GetMem( *currentBucket, vCount, iCount))
      {
        return NULL;
      }
  		bucket = currentBucket;
    }
    else if (currentBucket->HasRoomFor( vCount, iCount))
    {
      bucket = currentBucket;

#ifdef DEVELOPMENT
      if ((void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd)
      {
        ERR_FATAL( ("(void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd"));
      }
#endif
    }
    else if (currentBucket->IsFull() && currentBucket->id != id)
    {
      if (flushWhenFull)
      {
        if (IsBucketLastStack( *currentBucket))
        {
          bucket = currentBucket;
          FlushBucket( *bucket);

          // last chunk allocated; restore memory to pool
          curMem  -= bucket->memSize;
          curSize += bucket->memSize;
          bucket->Reset();

          if (!GetMem( *bucket, vCount, iCount))
          {
            return NULL;
          }

        }
        else if (currentBucket->HasRoomForWhenFlushed( vCount, iCount))
        {
          bucket = currentBucket;
          FlushBucket( *bucket);

#ifdef DEVELOPMENT
          if ((void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd)
          {
            ERR_FATAL( ("(void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd"));
          }
#endif
        }
      }
      // get bucket below
    }
    // get a bucket below
	}

  NList<Bucket>::Iterator li(&bucketList);
  Bucket *buck = NULL, *empty = NULL;
  if (!bucket)
  {
	  // look for a bucket that has a matching primitive description

    for ( !li; *li; li++ )
    {
      buck = *li;

      if (buck->IsLocked())
      {
        continue;
      }
		  else if (!buck->vMem)
		  {
			  empty = buck;
#ifdef DOLASTBUCKET
        break;
#else
        continue;
#endif
		  }
/*
		  else if (!buck->vMem)
		  {
			  bucket = buck;
        bucket->SetPrimitiveDescTag( primitive);

        if (!GetMem( *bucket, vCount, iCount))
        {
          return NULL;
        }
			  break;
		  }
      else
*/
      // else if buck->vCount == 0      FIXME
		  if (CompareRenderState( *buck))
		  {
        if (buck->HasRoomFor( vCount, iCount))
        {
          bucket = buck;

#ifdef DEVELOPMENT
          if ((void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd)
          {
            ERR_FATAL( ("(void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd"));
          }
#endif
  			  break;
        }
        else if (buck->IsFull() && buck->id != id)
        {
          if (flushWhenFull)
          {
            if (IsBucketLastStack( *buck))
            {
              bucket = buck;
              FlushBucket( *bucket);

              // last chunk allocated; restore memory to pool
              curMem  -= bucket->memSize;
              curSize += bucket->memSize;
              bucket->Reset();

              if (!GetMem( *bucket, vCount, iCount))
              {
                return NULL;
              }
              break;
            }
            else if (buck->HasRoomForWhenFlushed( vCount, iCount))
            {
              bucket = buck;
              FlushBucket( *bucket);

#ifdef DEVELOPMENT
              if ((void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd)
              {
                ERR_FATAL( ("(void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd"));
              }
#endif
              break;
            }
          }
        }
      }
	  }
    if (bucket)
    {
      SetCurrentBucket( bucket);
    }
  }
  if (!bucket)
  {
    if (empty)
    {
			bucket = empty;
    }
    else
    {
    	bucket = new Bucket( this);
	  	ASSERT( bucket );
		  AddNewBucket(bucket);

      LOG_DIAG( ("%sBucketMan::Lock:  Adding a bucket %d", 
        this == &Vid::bucket ? "" : "Tran", bucketList.GetCount()) );
    }
    bucket->SetPrimitiveDescTag( primitive);
    SetCurrentBucket( bucket);

    if (!GetMem( *bucket, vCount, iCount))
    {
      return NULL;
    }
  }

#ifdef DEVELOPMENT
  if ((void *)(((VertexTL *)bucket->vMemCur) + vCount) > (void *)bucket->iMem)
  {
    ERR_FATAL( ("(void *)(((VertexTL *)bucket->vMemCur) + vCount) > (void *)bucket->iMem"));
  }
  if ((void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd)
  {
    ERR_FATAL( ("(void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd"));
  }
#endif

  bucket->LockIndexedPrimitiveMem( vMem, iMem, vCount, iCount);
  bucket->id = (void *) id;

	ASSERT( *vMem && *iMem );

  return bucket;
}
//----------------------------------------------------------------------------

void BucketMan::UnlockIndexedPrimitiveMem( Bucket & bucket, U32 vCount, U32 iCount, Bool doOffset) // = TRUE)
{
  bucket.UnlockIndexedPrimitiveMem( vCount, iCount, doOffset);

  if (!bucket.vCount || !bucket.iCount)
  {
    if ((char *)bucket.vMem + bucket.memSize == curMem)
    {
      // last chunk allocated; restore memory to pool
      curMem  -= bucket.memSize;
      curSize += bucket.memSize;

      bucket.ResetMem();
    }
    else
    {
  	  bucket.ResetCounts();
    }
  }
#ifdef DOSTATISTICS
  else
  {
    Statistics::tempTris += iCount / 3;
  }
#endif
}
//----------------------------------------------------------------------------

void Bucket::UnlockIndexedPrimitiveMemManager( U32 _vCount, U32 _iCount, Bool doOffset) // = TRUE)
{
  manager->UnlockIndexedPrimitiveMem( *this, _vCount, _iCount, doOffset);
}
//-----------------------------------------------------------------------------

Bucket * BucketMan::LockPrimitiveMem( void **vMem, U32 vCount, const void *id) // = 0xcdcdcdcd
{
	ASSERT( vMem );

  // pad to multiples of 4 for Katmai
  vCount += 3;
  U32 iCount = 0;

  Bucket *bucket = NULL;

	// see if the current bucket has a matching primitive description
	if (currentBucket && !currentBucket->IsLocked() && CompareRenderState( *currentBucket))
	{
    if (!currentBucket->vMem)
    {
      if (!GetMem( *currentBucket, vCount, iCount))
      {
        return NULL;
      }
  		bucket = currentBucket;
    }
    else if (currentBucket->HasRoomFor( vCount, iCount))
    {
      bucket = currentBucket;

      if ((void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd)
      {
        ASSERT( (void *)(bucket->iMemCur + iCount) <= (void *)bucket->memEnd);
      }

    }
    else if (currentBucket->IsFull() && currentBucket->id != id)
    {
      if (flushWhenFull)
      {
        if (IsBucketLastStack( *currentBucket))
        {
          bucket = currentBucket;
          FlushBucket( *bucket);

          // last chunk allocated; restore memory to pool
          curMem  -= bucket->memSize;
          curSize += bucket->memSize;
          bucket->Reset();

          if (!GetMem( *bucket, vCount, iCount))
          {
            return NULL;
          }

        }
        else if (currentBucket->HasRoomForWhenFlushed( vCount, iCount))
        {
          bucket = currentBucket;
          FlushBucket( *bucket);

          if ((void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd)
          {
            ASSERT( (void *)(bucket->iMemCur + iCount) <= (void *)bucket->memEnd);
          }

        }
      }
      // get bucket below
    }
    // get a bucket below
	}

  Bucket *buck = NULL, *empty = NULL;
  if (!bucket)
  {
	  // look for a bucket that has a matching primitive description
    //
    NList<Bucket>::Iterator li(&bucketList);

    for ( !li; *li; li++ )
    {
      buck = *li;

      if (buck->IsLocked())
      {
        continue;
      }
		  else if (!buck->vMem)
		  {
			  empty = buck;
#ifdef DOLASTBUCKET
        break;
#else
        continue;
#endif
		  }
/*
		  else if (!buck->vMem)
		  {
			  bucket = buck;
        bucket->SetPrimitiveDescTag( primitive);

        if (!GetMem( *bucket, vCount, iCount))
        {
          return NULL;
        }
			  break;
		  }
      else
*/
      // else if buck->vCount == 0      FIXME
		  if (CompareRenderState( *buck))
		  {
        if (buck->HasRoomFor( vCount, iCount))
        {
          bucket = buck;

          ASSERT( (void *)(bucket->iMemCur + iCount) <= (void *)bucket->memEnd);

  			  break;
        }
        else if (buck->IsFull() && buck->id != id)
        {
          if (flushWhenFull)
          {
            if (IsBucketLastStack( *buck))
            {
              bucket = buck;
              FlushBucket( *bucket);

              // last chunk allocated; restore memory to pool
              curMem  -= bucket->memSize;
              curSize += bucket->memSize;
              bucket->Reset();

              if (!GetMem( *bucket, vCount, iCount))
              {
                return NULL;
              }
              break;
            }
            else if (buck->HasRoomForWhenFlushed( vCount, iCount))
            {
              bucket = buck;
              FlushBucket( *bucket);

              if ((void *)(bucket->iMemCur + iCount) > (void *)bucket->memEnd)
              {
                ASSERT( (void *)(bucket->iMemCur + iCount) <= (void *)bucket->memEnd);
              }

              break;
            }
          }
        }
      }
	  }
    if (bucket)
    {
      SetCurrentBucket( bucket);
    }
  }
  if (!bucket)
  {
    if (empty)
    {
			bucket = empty;
    }
    else
    {
    	bucket = new Bucket( this);
	  	ASSERT( bucket );
		  AddNewBucket(bucket);

      LOG_DIAG( ("%sBucketMan::Lock:  Adding a bucket %d", 
        this == &Vid::bucket ? "" : "Tran", bucketList.GetCount()) );
    }
    bucket->SetPrimitiveDescTag( primitive);
    SetCurrentBucket( bucket);

    if (!GetMem( *bucket, vCount, iCount))
    {
      return NULL;
    }
  }

  ASSERT( (void *)(((VertexTL *)bucket->vMemCur) + vCount) <= (void *)bucket->iMem);

  bucket->LockPrimitiveMem( vMem, vCount);
  bucket->id = (void *) id;

	ASSERT( *vMem );

  return bucket;
}
//----------------------------------------------------------------------------

void BucketMan::UnlockPrimitiveMem( Bucket &bucket, U32 vCount)
{
  bucket.UnlockPrimitiveMem( vCount);

  if (!bucket.vCount)
  {
    if ((char *)bucket.vMem + bucket.memSize == curMem)
    {
      // last chunk allocated; restore memory to pool
      curMem  -= bucket.memSize;
      curSize += bucket.memSize;

      bucket.ResetMem();
    }
    else
    {
  	  bucket.ResetCounts();
    }
  }
#if 0 // FIXME
  else if (flushWhenFull && bucket.oversize)
  {
    FlushBucket( bucket);
  }
#endif
}
//----------------------------------------------------------------------------

void Bucket::UnlockPrimitiveMemManager( U32 _vCount)
{
  manager->UnlockPrimitiveMem( *this, _vCount);
}
//-----------------------------------------------------------------------------

void BucketMan::FlushBucket( Bucket & bucket, Bool doDraw) // = TRUE)
{
  if (!doDraw)
  {
    LOG_WARN( ("Bucket::FlushBucket: !doDraw") );
    return;
  }

#ifndef DODXLEANANDGRUMPY
  if (Vid::renderState.status.dxTL)
  {
    // direct x material  
    Vid::SetMaterial( bucket.material);
  }
#endif

  // direct x texture, stage 0
  TextureStage & stage0 = bucket.textureStages[0];
  Vid::SetTexture( stage0.texture, 0, stage0.blend);

  if (Vid::renderState.status.texMulti && bucket.texture_count > 1)
  {
    // direct x texture, stage 1
    TextureStage & stage1 = bucket.textureStages[1];
    Vid::SetTexture( stage1.texture, 1, stage1.blend);
  }

  if (bucket.flags & RS_NOINDEXED)
  {
		Vid::DrawPrimitive(
										bucket.primitive_type,
										bucket.vertex_type,
										bucket.vMem, bucket.vCount,
										bucket.flags);
	}
  else
  {
		Vid::DrawIndexedPrimitive(
										bucket.primitive_type,
										bucket.vertex_type,
										bucket.vMem, bucket.vCount,
										bucket.iMem, bucket.iCount,
										bucket.flags);

  }
 	bucket.ResetCounts();
}
//----------------------------------------------------------------------------

void BucketMan::Flush( Bool doDraw) // = TRUE)
{ 
	NList<Bucket>::Iterator li(&bucketList); 
  while (Bucket * bucket = li++)
  {
    if (bucket->vCount)
    {
/*
      U32 alpha = Vid::renderState.status.alpha;

      TextureStage & stage0 = bucket->textureStages[0];
      if (stage0.texture && stage0.texture->IsTranslucent())
      {
        Vid::SetAlphaState( TRUE);
      }
*/
   		FlushBucket( *bucket, doDraw);

//      Vid::SetAlphaState( alpha);
    }
    bucket->Reset();

#ifdef DOLASTBUCKET
    if (bucket == lastUsedBucket)
    {
      break;
    }
#endif

	}

  curMem  = memBlock;
  curSize = memSize;

  currentBucket = NULL;
  lastUsedBucket = NULL;
}
//----------------------------------------------------------------------------

void BucketMan::FlushTex( const Bitmap * texture, Bool doDraw) // = TRUE)
{ 
	NList<Bucket>::Iterator li(&bucketList); 
  for (!li; *li; li++)
	{
    Bucket *bucket = *li;

    if (bucket->texture_count && bucket->textureStages[0].texture == texture && bucket->vCount)
    {
   		FlushBucket( *bucket, doDraw);
    }
    bucket->Reset();
	}

  currentBucket = NULL;
}
//----------------------------------------------------------------------------
