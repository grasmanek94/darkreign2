///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// mesh_rendertl.cpp
//
// 30-MAR-2000
//

#include "vid_public.h"
#include "terrain.h"
#include "bucket_inline.h"
#include "IMESHUtil.h"
#include "perfstats.h"
#include "statistics.h"
//----------------------------------------------------------------------------

void MeshRoot::SetVertsWorldAnim( const Array<FamilyState> & stateArray, Matrix * tranys, VertexI * verts, U32 vCount, Bool doMultiWeight) const
{
  SetMatricesWorld( stateArray, tranys);

  // transform verts to world space
  U32 i;
  for (i = 0; i < vCount; i++)
  {
    SetVert( tranys, i, verts[i].vv, doMultiWeight);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightAnimV1( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags)
{
  clipFlags;
  vCount;

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

  Rebuild();

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  vCount = vertex.count;

  VertexI * verts;
  U16 * iv;
  U32 heapSize = Vid::Heap::ReqVertexI( &verts, vCount, &iv, vCount);

  Matrix tranys[MAXMESHPERGROUP];
  SetVertsWorldAnim( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);
/*
      if (hasTread)
      {
        d->uv.v += vOffsets[vmap->index[0]];
      }
*/
//  if ((_controlFlags & controlTRANSLUCENT) || baseColor.a < 255) 
  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_VERTEX,
    RS_NOSORT | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP));

  ColorF32 base( baseColor);

  // setup _buckys and fill them
  //
//  FaceGroup * b = _buckys.data, * be = _buckys.data + _buckys.count;
  FaceGroup * b = groups.data, * be = groups.data + groups.count;
  for (U16 sort = Vid::sortNORMAL0; b < be; b++, sort++)
  {
    FaceGroup & bucky = *b;

    // keep various textures in a fixed sort order for no-pop translucency
    BucketMan::SetTag1( sort);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipNONE, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }
    bucky.vCount = 0;
    bucky.iCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);

    // for all the faces in this group
    //
    U16 * ii, * ie = bucky.geo.idx + bucky.geo.iCount;
    for (ii = bucky.geo.idx; ii < ie; ii++)
    {
      U16 ivj = *ii;
      ASSERT( ivj < vertex.count);

      if (iv[ivj] == 0xffff)
      {
        Vertex  & dv = bucky.CurrVertex();
        VertexI & sv = verts[ivj];

        if (hasTread)
        {
          dv.uv.v += vOffsets[sv.vi.index[0]];
        }

        bucky.SetIndex( (U16) bucky.vCount);
        bucky.vCount++;
      }
      else
      {
        // same old vert 
        //
        bucky.SetIndex( iv[ivj]);
      }
    }
    // flush memory
    // 
    if (!Vid::UnLockBucket( bucky, clipNONE, &stateArray) && bucky.vCount > 0)
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }
  BucketMan::forceTranslucent = FALSE;

  Vid::Heap::Restore( heapSize);

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
  if (mrm)
  {
    Statistics::mrmTris = Statistics::mrmTris + Statistics::tempTris;
  }
  else
  {
    Statistics::nonMRMTris = Statistics::nonMRMTris + Statistics::tempTris;
  }
#endif
}
//----------------------------------------------------------------------------


void MeshRoot::RenderLightNoAnimV1( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags)
{
  clipFlags;
  vCount;

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

  Rebuild();

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  vCount = vertex.count;

  VertexI * verts;
  U16 * iv;
  U32 heapSize = Vid::Heap::ReqVertexI( &verts, vCount, &iv, vCount);

  Matrix tranys[MAXMESHPERGROUP];
  SetVertsWorldAnim( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);
/*
      if (hasTread)
      {
        d->uv.v += vOffsets[vmap->index[0]];
      }
*/
//  if ((_controlFlags & controlTRANSLUCENT) || baseColor.a < 255) 
  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_VERTEX,
    RS_NOSORT | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP));

  ColorF32 base( baseColor);

  // setup _buckys and fill them
  //
//  FaceGroup * b = _buckys.data, * be = _buckys.data + _buckys.count;
  FaceGroup * b = groups.data, * be = groups.data + groups.count;
  for (U16 sort = Vid::sortNORMAL0; b < be; b++, sort++)
  {
    FaceGroup & bucky = *b;

    // keep various textures in a fixed sort order for no-pop translucency
    BucketMan::SetTag1( sort);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipNONE, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }
    bucky.vCount = 0;
    bucky.iCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);

    // for all the faces in this group
    //
    U16 * ii, * ie = bucky.geo.idx + bucky.geo.iCount;
    for (ii = bucky.geo.idx; ii < ie; ii++)
    {
      U16 ivj = *ii;
      ASSERT( ivj < vertex.count);

      if (iv[ivj] == 0xffff)
      {
        Vertex  & dv = bucky.CurrVertex();
        VertexI & sv = verts[ivj];

        if (hasTread)
        {
          dv.uv.v += vOffsets[sv.vi.index[0]];
        }

        bucky.SetIndex( (U16) bucky.vCount);
        bucky.vCount++;
      }
      else
      {
        // same old vert 
        //
        bucky.SetIndex( iv[ivj]);
      }
    }
    // flush memory
    // 
    if (!Vid::UnLockBucket( bucky, clipNONE, &stateArray) && bucky.vCount > 0)
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }
  BucketMan::forceTranslucent = FALSE;

  Vid::Heap::Restore( heapSize);

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
  if (mrm)
  {
    Statistics::mrmTris = Statistics::mrmTris + Statistics::tempTris;
  }
  else
  {
    Statistics::nonMRMTris = Statistics::nonMRMTris + Statistics::tempTris;
  }
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorAnimV1( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort)
{
  clipFlags;
  vCount;

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

  Rebuild();

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  vCount = vertex.count;

  VertexI * verts;
  U16 * iv;
  U32 heapSize = Vid::Heap::ReqVertexI( &verts, vCount, &iv, vCount);

  Matrix tranys[MAXMESHPERGROUP];
  SetVertsWorldAnim( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);
/*
      if (hasTread)
      {
        d->uv.v += vOffsets[vmap->index[0]];
      }
*/
//  if ((_controlFlags & controlTRANSLUCENT) || baseColor.a < 255) 
  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_VERTEX,
    RS_NOSORT | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP));

  ColorF32 base( baseColor);

  // setup _buckys and fill them
  //
//  FaceGroup * b = _buckys.data, * be = _buckys.data + _buckys.count;
  FaceGroup * b = groups.data, * be = groups.data + groups.count;
  for ( /*sort = Vid::sortNORMAL0 */; b < be; b++, sort++)
  {
    FaceGroup & bucky = *b;

    // keep various textures in a fixed sort order for no-pop translucency
    BucketMan::SetTag1( sort);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipNONE, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }
    bucky.vCount = 0;
    bucky.iCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);

    // for all the faces in this group
    //
    U16 * ii, * ie = bucky.geo.idx + bucky.geo.iCount;
    for (ii = bucky.geo.idx; ii < ie; ii++)
    {
      U16 ivj = *ii;
      ASSERT( ivj < vertex.count);

      if (iv[ivj] == 0xffff)
      {
        Vertex  & dv = bucky.CurrVertex();
        VertexI & sv = verts[ivj];

        if (hasTread)
        {
          dv.uv.v += vOffsets[sv.vi.index[0]];
        }

        bucky.SetIndex( (U16) bucky.vCount);
        bucky.vCount++;
      }
      else
      {
        // same old vert 
        //
        bucky.SetIndex( iv[ivj]);
      }
    }
    // flush memory
    // 
    if (!Vid::UnLockBucket( bucky, clipNONE, &stateArray) && bucky.vCount > 0)
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }
  BucketMan::forceTranslucent = FALSE;

  Vid::Heap::Restore( heapSize);

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
  if (mrm)
  {
    Statistics::mrmTris = Statistics::mrmTris + Statistics::tempTris;
  }
  else
  {
    Statistics::nonMRMTris = Statistics::nonMRMTris + Statistics::tempTris;
  }
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorNoAnimV1( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort)
{
  clipFlags;
  vCount;

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

  Rebuild();

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  vCount = vertex.count;

  VertexI * verts;
  U16 * iv;
  U32 heapSize = Vid::Heap::ReqVertexI( &verts, vCount, &iv, vCount);

  Matrix tranys[MAXMESHPERGROUP];
  SetVertsWorldAnim( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);
/*
      if (hasTread)
      {
        d->uv.v += vOffsets[vmap->index[0]];
      }
*/
//  if ((_controlFlags & controlTRANSLUCENT) || baseColor.a < 255) 
  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_VERTEX,
    RS_NOSORT | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP));

  ColorF32 base( baseColor);

  // setup _buckys and fill them
  //
//  FaceGroup * b = _buckys.data, * be = _buckys.data + _buckys.count;
  FaceGroup * b = groups.data, * be = groups.data + groups.count;
  for (/*sort = Vid::sortNORMAL0*/; b < be; b++, sort++)
  {
    FaceGroup & bucky = *b;

    // keep various textures in a fixed sort order for no-pop translucency
    BucketMan::SetTag1( sort);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipNONE, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }
    bucky.vCount = 0;
    bucky.iCount = 0;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);

    // for all the faces in this group
    //
    U16 * ii, * ie = bucky.geo.idx + bucky.geo.iCount;
    for (ii = bucky.geo.idx; ii < ie; ii++)
    {
      U16 ivj = *ii;
      ASSERT( ivj < vertex.count);

      if (iv[ivj] == 0xffff)
      {
        Vertex  & dv = bucky.CurrVertex();
        VertexI & sv = verts[ivj];

        if (hasTread)
        {
          dv.uv.v += vOffsets[sv.vi.index[0]];
        }

        bucky.SetIndex( (U16) bucky.vCount);
        bucky.vCount++;
      }
      else
      {
        // same old vert 
        //
        bucky.SetIndex( iv[ivj]);
      }
    }
    // flush memory
    // 
    if (!Vid::UnLockBucket( bucky, clipNONE, &stateArray) && bucky.vCount > 0)
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }
  BucketMan::forceTranslucent = FALSE;

  Vid::Heap::Restore( heapSize);

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
  if (mrm)
  {
    Statistics::mrmTris = Statistics::mrmTris + Statistics::tempTris;
  }
  else
  {
    Statistics::nonMRMTris = Statistics::nonMRMTris + Statistics::tempTris;
  }
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::MrmUpdate1( Array<FaceGroup> & _groups, U32 vCountNew, U32 & _vertCount, U32 & _faceCount)
{
  while (_vertCount != vCountNew)
  {
    FaceGroup * b, * be = _groups.data + _groups.count;
    for (b = _groups.data; b < be; b++)
    {
      FaceGroup & bucky = *b;

      ASSERT( bucky.geo.mrm);

      GeoCache::Mrm * mrm = NULL;

      // update the face and vertex counts first
      U32 dir = 0;
	    if (_vertCount < vCountNew)
	    {
        mrm = bucky.geo.mrm + bucky.geo.vCount;
        bucky.geo.vCount++;
		    _vertCount++;

        bucky.geo.iCount += mrm->iCount;
	    }
	    else  // decreasing
	    {
        bucky.geo.vCount--;
		    _vertCount--;
        mrm = bucky.geo.mrm + bucky.geo.vCount;


        bucky.geo.iCount -= mrm->iCount;

        dir = 0;
	    }

      GeoCache::Mrm::Rec * r, * re = mrm->rec + mrm->rCount;
      for (r = mrm->rec; r < re; r++)
      {
        bucky.geo.idx[r->index] = r->value[dir];
      }
    }
	  if (_vertCount < vCountNew)
	  {
      _vertCount++;
    }
	  else  // decreasing
	  {
      _vertCount--;
    }        
  }
}
//----------------------------------------------------------------------------
