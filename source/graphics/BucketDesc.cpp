///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Bucket.cpp
//
//

#include "vid.h"
#include "Bucket.h"
//-----------------------------------------------------------------------------

Bucket::Bucket( BucketMan *man)
{
  ClearData();
  manager = man;
}
//-----------------------------------------------------------------------------

void Bucket::ClearData()
{
	PrimitiveDesc::ClearData();

  ResetMem();
}
//-----------------------------------------------------------------------------

void Bucket::Reset()
{
  vCount = 0;
  vMem = NULL;

  oversize = FALSE;

  id = 0;
}
//-----------------------------------------------------------------------------

void Bucket::ResetMem()
{
  vCountMax  = iCountMax	= 0;
  vCountFull = iCountFull = 0;

  vMem = NULL;
	iMem = NULL;

  oversize = FALSE;

  ResetCounts();
}
//-----------------------------------------------------------------------------

void Bucket::ResetCounts()
{
  vMemCur = vMem;
  iMemCur = iMem;

  vCountLeft = vCountMax;
  iCountLeft = iCountMax;

  vCount = vCountLock = 0;
  iCount = iCountLock = 0;

  id = 0;
}
//-----------------------------------------------------------------------------

void Bucket::LockIndexedPrimitiveMem( void **_vMem, U16 **_iMem, U32 _vCount, U32 _iCount)
{
  *_vMem = vMemCur;
  *_iMem = iMemCur;

  vCountLock = _vCount;
  iCountLock = _iCount;

  ASSERT( (void *)(((VertexTL *)vMemCur) + _vCount) <= (void *)iMem);
}
//-----------------------------------------------------------------------------

void Bucket::UnlockIndexedPrimitiveMem( U32 _vCount, U32 _iCount, Bool doOffset) // = TRUE
{
  ASSERT( _vCount <= vCountLock);
  ASSERT( _iCount <= iCountLock);

  offset = vCount;
  if (vCount && doOffset)
  {
    U16 * s = iMemCur, * e = s + _iCount;
    while (s < e)
    {
      *s = (U16)(*s + vCount);
      s++;
    }
  }

  iMemCur += _iCount;
  iCountLeft -= _iCount;
  iCountLock = 0;
  iCount += _iCount;

#ifdef DOVERTEXNON32
  vMemCur = (void *) ((char *)vMemCur + _vCount * sizeofVertex);
#else
  vMemCur = (void *) ((char *)vMemCur + _vCount * sizeof( VertexTL));
#endif

  vCountLeft -= _vCount;
  vCountLock = 0;
  vCount += _vCount;
}
//-----------------------------------------------------------------------------

void Bucket::LockPrimitiveMem( void **_vMem, U32 _vCount)
{
  *_vMem = vMemCur;

  vCountLock = _vCount;
  iCountLock = 0;
}
//-----------------------------------------------------------------------------

void Bucket::UnlockPrimitiveMem( U32 _vCount)
{
  ASSERT( _vCount <= vCountLock);

#ifdef DOVERTEXNON32
  vMemCur = (void *) ((char *)vMemCur + _vCount * sizeofVertex);
#else
  vMemCur = (void *) ((char *)vMemCur + _vCount * sizeof(VertexTL));
#endif

  vCountLeft -= _vCount;
  vCountLock = 0;
  vCount += _vCount;
}
//-----------------------------------------------------------------------------

static Bucket *curBucket = NULL;

//  this function is sensitive to vertex type (i.e. VertexL vs VertexTL)
//
int _cdecl ComparePrimitives( const void *e1, const void *e2)
{
	U16 *index1 = (U16 *)e1;
	U16 *index2 = (U16 *)e2;

  VertexTL *verts = (VertexTL *) curBucket->vMem;
  F32 z1 = (verts[index1[0]].vv.z + verts[index1[1]].vv.z + verts[index1[2]].vv.z); 
  F32 z2 = (verts[index2[0]].vv.z + verts[index2[1]].vv.z + verts[index2[2]].vv.z); 

  if (z1 < z2)
	{
		return 1;
	}
	if (z1 > z2)
	{
		return -1;
	}
	return 0;
}
//----------------------------------------------------------------------------

void Bucket::Sort()
{
  curBucket = this;
  qsort( iMem, iCount / 3, 3 * sizeof( U16), ComparePrimitives);
}
//----------------------------------------------------------------------------
