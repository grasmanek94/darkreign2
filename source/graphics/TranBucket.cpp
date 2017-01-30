///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// TranBucket.cpp
//
//

#include "vid_public.h"
#include "TranBucket.h"

#define TRANMAXBUCKETCOUNT    64
//----------------------------------------------------------------------------

void TranBucketMan::ClearData()
{
  BucketMan::ClearData();

	maxZ = 1.0f;
	minZ = 0.0f;
	maxBucketCount = 1.0f;
	UpdateScaleZ();
  doSort = TRUE;
}
//----------------------------------------------------------------------------

void TranBucketMan::SetPrimitiveDesc( PrimitiveDesc & prim)
{
  primitive.SetPrimitiveDescTag( prim);
}
//----------------------------------------------------------------------------

void TranBucketMan::SetPrimitiveDesc( Bucket & bucket, PrimitiveDesc & prim)
{
  bucket.SetPrimitiveDescTag( prim);
}
//----------------------------------------------------------------------------

Bool TranBucketMan::CompareRenderState( const PrimitiveDesc & other) const 
{
	if (
#ifndef DODXLEANANDGRUMPY
		   primitive.material       == other.material       &&
		   primitive.vertex_type    == other.vertex_type    &&
#endif
       primitive.tag            == other.tag            && 
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

void TranBucketMan::Flush( Bool doDraw)  // = TRUE
{ 
  if (!doDraw)
	{
		BucketMan::Flush(FALSE);
		return;
	}

  Vid::SetZWriteState( FALSE);
  Bool alpha = Vid::SetAlphaState( TRUE);

	// figure out how many buckets actually have vertices in them
	// so that we don't have to look at any empty buckets

	NList<Bucket>::Iterator li(&bucketList); 
  Bucket * list[444];
  S32 count = 0;
  while (Bucket * bucket = li++)
  {
    if (bucket->vCount)
    {
      list[count] = bucket;
      count++;

      ASSERT( count <= 444);
    }
#ifdef DOLASTBUCKET
    if (bucket == lastUsedBucket)
    {
      break;
    }
#endif
  }

	// for each bucket that is not empty...
	for (count--; count >= 0; count--)
	{
		Bucket *max = list[count];
  
		// find the bucket with the maximum z value
    S32 i, current = count;
		for (i = count - 1; i >= 0; i--)
		{
			if (list[i]->tag > max->tag)
			{
				max = list[i];
        current = i;
			}
/*
			else if (list[i]->tag == max->tag)
      {
        if (list[i]->tag1 > max->tag1)
        {
	  			max = list[i];
          current = i;
        }
      }
*/
		}

    if (!Vid::caps.noTransort && !(max->flags & RS_NOSORT) && primitive.primitive_type != PT_LINELIST)
    {
      max->Sort();
    }
		// flush the bucket with the maximum z value
    if ((max->flags & RS_DST_MASK) == RS_DST_ONE)
    {
      Vid::SetFogColorD3D( 0);

  		FlushBucket( *max);

      Vid::SetFogColorD3D( Vid::renderState.fogColor);
    }
    else
    {
  		FlushBucket( *max);
    }

    max->Reset();

    // move end of list to current position 
    //
    list[current] = list[count];
	}


  Vid::SetAlphaState( alpha);
  Vid::SetZWriteState( TRUE);

  curMem  = memBlock;
  curSize = memSize;

  currentBucket = NULL;
  lastUsedBucket = NULL;
}
//----------------------------------------------------------------------------

void TranBucketMan::FlushTex( const Bitmap * texture, Bool doDraw)  // = TRUE
{ 
  if (!doDraw)
	{
		BucketMan::Flush(FALSE);
		return;
	}

  Vid::SetZWriteState( FALSE);
  Bool alpha = Vid::SetAlphaState( TRUE);

#if 1

  NList<Bucket>::Iterator li(&bucketList); 
  for (!li; *li; li++)
	{
    Bucket *bucket = *li;

    if (bucket->texture_count && bucket->textureStages[0].texture == texture && bucket->vCount)
    {

		  // flush the bucket with the maximum z value
      if ((bucket->flags & RS_DST_MASK) == RS_DST_ONE)
      {
        Vid::SetFogColorD3D( 0);

  		  FlushBucket( *bucket);

        Vid::SetFogColorD3D( Vid::renderState.fogColor);
      }
      else
      {
  		  FlushBucket( *bucket);
      }
    }
    bucket->Reset();
	}

  currentBucket = NULL;

#else

	// figure out how many buckets actually have vertices in them
	// so that we don't have to look at any empty buckets
	NList<Bucket>::Iterator li(&bucketList); 
  Bucket *list[222];
  S32 count = 0;
#if 0
  S32 ic = 0;
	NList<Bucket>::Node *lnode = NULL;
  NList<Bucket>::Node *llnode = NULL;
#endif
  for (!li; *li; li++)
  {

		if ((*li)->vCount)
		{
      list[count] = *li;
      count++;
		}
	}

	// for each bucket that is not empty...
	for (count--; count >= 0; count--)
	{
		Bucket *max = list[count];
  
		// find the bucket with the maximum z value
    S32 i, current = count;
		for (i = count - 1; i >= 0; i--)
		{
			if (list[i]->tag > max->tag)
			{
				max = list[i];
        current = i;
			}
		}

    if (!Vid::caps.notransort && !(max->flags & RS_NOSORT))
    {
      max->Sort();
    }


		// flush the bucket with the maximum z value
    if ((max->flags & RS_DST_MASK) == RS_DST_ONE)
    {
      Vid::SetFogColorD3D( 0);

  		FlushBucket( *max);

      Vid::SetFogColorD3D( Vid::renderState.fogColor);
    }
    else
    {
  		FlushBucket( *max);
    }
    max->Reset();

    // move end of list to current position 
    //
    list[current] = list[count];
	}
#endif

  Vid::SetAlphaState( alpha);
  Vid::SetZWriteState( TRUE);

  curMem  = memBlock;
  curSize = memSize;
  currentBucket = NULL;
}
//----------------------------------------------------------------------------

void TranBucketMan::UpdateScaleZ()
{
	ASSERT( minZ <= maxZ );
	ASSERT( maxBucketCount > 0.0f );

//  scaleZ = (maxZ - minZ) / (maxBucketCount - 1);
  scaleZ = (maxBucketCount - 1) / (maxZ - minZ);
}
//----------------------------------------------------------------------------

void TranBucketMan::SetZ( F32 _z)
{
	ASSERT( scaleZ > 0.0f );
  
  U32 sort_z;

  if (_z < minZ)
  {
    sort_z = 0;
  }
  else if (_z > maxZ)
  {
    sort_z = (U32) Utils::FtoLDown(maxBucketCount);
  }
  else
  {
  	// calculate z value to use for sorting buckets
//	  sort_z = (U32) Utils::FtoLDown( ( ( _z - minZ ) / scaleZ ) + 1.0f );
	  sort_z = (U32) Utils::FtoLDown( ( _z - minZ ) * scaleZ );
  }
	
	// sort_z values start at 1 and maxBucketCount values start at 0;
	// thus, sort_z must be less-that-or-equal-to max_bucket count
#ifdef DEVELOPMENT
  if (!(sort_z <= (U32) maxBucketCount))
  {
  	ASSERT( sort_z <= (U32) maxBucketCount );  // paranoia, paranoia, don't you know they're right behind me...
  }
	ASSERT( sort_z < U16_MAX );
#endif

	BucketMan::SetTag( U16( sort_z));
}
//----------------------------------------------------------------------------