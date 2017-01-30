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

void MeshRoot::SetVertsWorldAnim( const Array<FamilyState> & stateArray, Matrix * tranys, VertexTL * verts, U32 vCount, Bool doMultiWeight) const
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

void MeshRoot::RenderLightAnimV( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags)
{
  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  Vector * verts;
  U16 * iv, * in, * iu;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vCount, &iv, vertices.count, &in, normals.count, &iu, uvs.count);

  // set up transform matrices and transform verts to view space
  //
  Matrix tranys[MAXMESHPERGROUP];
  SetVertsWorld( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

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
  FaceGroup * b, * be = _buckys.data + _buckys.count;
  U16 sort = Vid::sortNORMAL0;
  for (b = _buckys.data; b < be; b++, sort++)
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

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( in, 0xfe, sizeof(U16) * normals.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        ASSERT( ivj < vCount && inj < normals.count && iuj < uvs.count);

        if (iv[ivj] != in[inj])
        {
          // new vert
          //
          Vertex & dv = bucky.CurrVertex();
          Vector & sv = verts[ivj];

          dv.vv = sv;

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

          tranys[vertToState[ivj].index[0]].Rotate( dv.nv, normals[inj]);

/*
          if (colors.count)
          {
            Color c;
            c.Modulate( colors[ivj], base.r, base.g, base.b);
            bucky.LightCamInline( dv, sv, norm, c); 
          }
          else
          {
            bucky.LightCamInline( dv, sv, norm, baseColor); 
          }
*/
          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
        else if (iv[ivj] != iu[iuj])
        {
          // old vert with new uv
          //
          Vertex & dv = bucky.CurrVertex();

          dv = bucky.GetVertex( iv[ivj]);

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
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

void MeshRoot::RenderLightNoAnimV( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags)
{
  vCount;

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  Vector * verts;
  U16 * iv, * in, * iu;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vCount, &iv, vertices.count, &in, normals.count, &iu, uvs.count);

  // set up transform matrices and transform verts to world space
  //
  SetVertsWorldNoAnim( stateArray, verts, vCount);

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
  FaceGroup * b, * be = _buckys.data + _buckys.count;
  U16 sort = Vid::sortNORMAL0;
  for (b = _buckys.data; b < be; b++, sort++)
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

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( in, 0xfe, sizeof(U16) * normals.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        ASSERT( ivj < vCount && inj < normals.count && iuj < uvs.count);

        if (iv[ivj] != in[inj])
        {
          // new vert
          //
          Vertex & dv = bucky.CurrVertex();
          Vector & sv = verts[ivj];

          dv.vv = sv;

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

          stateArray.data->WorldMatrix().Rotate( dv.nv, normals[inj]);
/*
          if (colors.count)
          {
            Color c;
            c.Modulate( colors[ivj], base.r, base.g, base.b);
            bucky.LightModInline( dv, sv, normals[inj], c); 
          }
          else
          {
            bucky.LightModInline( dv, sv, normals[inj], baseColor); 
          }
*/
          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
        else if (iv[ivj] != iu[iuj])
        {
          // old vert with new uv
          //
          Vertex & dv = bucky.CurrVertex();

          dv = bucky.GetVertex( iv[ivj]);

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
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
    }

    // flush memory and clip if necessary
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

void MeshRoot::RenderColorAnimV( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort)
{
  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  Vector * verts;
  U16 * iv, * iu;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vCount, &iv, vertices.count, &iu, uvs.count);

  // set up transform matrices and transform verts to view space
  //
  Matrix tranys[MAXMESHPERGROUP];
  SetVertsWorld( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

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
    DP_DONOTLIGHT | RS_NOSORT | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP));

  // setup _buckys and fill them
  //
  FaceGroup * b, * be = _buckys.data + _buckys.count;
  for (b = _buckys.data; b < be; b++, sort++)
  {
    FaceGroup & bucky = *b;

    // keep various textures in a fixed sort order for no-pop translucency
    BucketMan::SetTag1( sort);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipNONE, &stateArray, tex, blend))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

/*
    if (tex)
    {
      // calc constant diffuse component for each material (bucky)
      //
      bucky.diffInitC.SetNoExpand( 
        (Vid::renderState.ambientColorF32.r + 1) * baseColor.r, 
        (Vid::renderState.ambientColorF32.g + 1) * baseColor.g, 
        (Vid::renderState.ambientColorF32.b + 1) * baseColor.b, 
         bucky.diff.a * F32(baseColor.a)
      );
    }
    else
    {
      // calc constant diffuse component for each material (bucky)
      //
      bucky.diffInitC.SetNoExpand( 
        (bucky.diff.r * Vid::renderState.ambientColorF32.r + bucky.diff.r) * baseColor.r, 
        (bucky.diff.g * Vid::renderState.ambientColorF32.g + bucky.diff.g) * baseColor.g, 
        (bucky.diff.b * Vid::renderState.ambientColorF32.b + bucky.diff.b) * baseColor.b, 
         bucky.diff.a * F32(baseColor.a)
      );
    }

    if (colors.count)
    {
      // calc constant diffuse component for each material (bucky)
      bucky.diffInitF32.Set(
        bucky.diffInitC.r * U8toNormF32,
        bucky.diffInitC.g * U8toNormF32,
        bucky.diffInitC.b * U8toNormF32,
        bucky.diffInitC.a * U8toNormF32
      );
    }
*/
    // clear indexers
    //
/*
    memset( iv, 0xff, sizeof(U16) * bucky.vertices.count);
    memset( iu, 0xfd, sizeof(U16) * bucky.vertices.count);
*/
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      // backcull
      //
      if (!(bucky.flags0 & RS_2SIDED))
      {
        Plane plane;
        plane.Set( verts[face.verts[0]], verts[face.verts[2]], verts[face.verts[1]]);
        if (Vid::BackCull( plane.Dot( verts[face.verts[0]])))
        {
          continue;
        }
      }

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 iuj = face.uvs[j];

        ASSERT( ivj < vCount && iuj < uvs.count);

        if (iv[ivj] == iu[iuj])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and project

          Vertex & dv = bucky.CurrVertex();
          Vector & sv = verts[ivj];

          dv.vv = sv;

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

/*
          if (colors.count)
          {
            dv.diffuse.Modulate( colors[ivj], bucky.diffInitF32.r, bucky.diffInitF32.g, bucky.diffInitF32.b, bucky.diffInitF32.a);
          }
          else
          {
            dv.diffuse  = bucky.diffInitC;
          }
          dv.specular = 0xff000000;
*/

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    // flush memory and clip if necessary
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

void MeshRoot::RenderColorNoAnimV( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort)
{
  vCount;

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  Vector * verts;
  U16 * iv, * in, * iu;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vCount, &iv, vertices.count, &in, normals.count, &iu, uvs.count);

  // set up transform matrices and transform verts to view space
  //
  SetVertsWorldNoAnim( stateArray, verts, vCount);

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

  // setup _buckys and fill them
  //
  FaceGroup * b, * be = _buckys.data + _buckys.count;
  for (b = _buckys.data; b < be; b++, sort++)
  {
    FaceGroup & bucky = *b;

    // keep various textures in a fixed sort order for no-pop translucency
    BucketMan::SetTag1( sort);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipNONE, &stateArray, tex, blend))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }
/*
    if (tex)
    {
      // calc constant diffuse component for each material (bucky)
      //
      bucky.diffInitC.SetNoExpand( 
        (Vid::renderState.ambientColorF32.r + 1) * baseColor.r, 
        (Vid::renderState.ambientColorF32.g + 1) * baseColor.g, 
        (Vid::renderState.ambientColorF32.b + 1) * baseColor.b, 
         bucky.diff.a * F32(baseColor.a)
      );
    }
    else
    {
      // calc constant diffuse component for each material (bucky)
      //
      bucky.diffInitC.SetNoExpand( 
        (bucky.diff.r * Vid::renderState.ambientColorF32.r + bucky.diff.r) * baseColor.r, 
        (bucky.diff.g * Vid::renderState.ambientColorF32.g + bucky.diff.g) * baseColor.g, 
        (bucky.diff.b * Vid::renderState.ambientColorF32.b + bucky.diff.b) * baseColor.b, 
         bucky.diff.a * F32(baseColor.a)
      );
    }

    if (colors.count)
    {
      // calc constant diffuse component for each material (bucky)
      bucky.diffInitF32.Set(
        bucky.diffInitC.r * U8toNormF32,
        bucky.diffInitC.g * U8toNormF32,
        bucky.diffInitC.b * U8toNormF32,
        bucky.diffInitC.a * U8toNormF32
      );
    }
*/
    // clear indexers
    //
/*
    memset( iv, 0xff, sizeof(U16) * bucky.vertices.count);
    memset( iu, 0xfd, sizeof(U16) * bucky.vertices.count);
*/
    memset( iv, 0xff, sizeof(U16) * vertices.count);
    memset( iu, 0xfd, sizeof(U16) * uvs.count);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 iuj = face.uvs[j];

        ASSERT( ivj < vCount && iuj < uvs.count);

        if (iv[ivj] == iu[iuj])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and project

          Vertex & dv = bucky.CurrVertex();
          Vector & sv = verts[ivj];

          dv.vv = sv;

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

/*
          if (colors.count)
          {
            dv.diffuse.Modulate( colors[ivj], bucky.diffInitF32.r, bucky.diffInitF32.g, bucky.diffInitF32.b, bucky.diffInitF32.a);
          }
          else
          {
            dv.diffuse = bucky.diffInitC;
          }
          dv.specular = 0xff000000;
*/

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    // flush memory and clip if necessary
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

