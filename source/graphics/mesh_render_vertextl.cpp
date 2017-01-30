///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// mesh_render_vertextl.cpp
//
// 2-JAN-2000
//
// This module contains functions that perform all lighting, transformation, and clipping 
// before submitting primitives to the base renderer.  For animating meshes, all verts are
// transformed into view space then a culled vertex and indexed tri list are produced
// directly into bucket memory.  The bucket memory is guarenteed to not flush if requested
// sequentially with the same ID, so it is possible to post process the primitive data.
// Most of DR2's simple mesh effects just copy the primitives directly into another bucket
// for a simple overlay effect.
//

#include "vid_private.h"
#include "mesh.h"
#include "IMESHUtil.h"
#include "perfstats.h"
#include "statistics.h"
#include "bucket_inline.h"
#include "terrain.h"
//----------------------------------------------------------------------------

void MeshRoot::SetMatricesView( const Array<FamilyState> & stateArray, Matrix *tranys) const
{
  U32 i;
  for (i = 0; i < stateArray.count; i++)
  {
    tranys[i] = stateMats[i] * stateArray[i].WorldMatrix() * Vid::Math::viewMatrix;
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetMatricesWorld( const Array<FamilyState> & stateArray, Matrix *tranys) const
{
  U32 i;
  for (i = 0; i < stateArray.count; i++)
  {
    tranys[i] = stateMats[i] * stateArray[i].WorldMatrix();
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetMatricesWorldInverseZ( const Array<FamilyState> & stateArray, Matrix *tranys) const
{
  Matrix mat( Quaternion( PI, Matrix::I.up));

  U32 i;
  for (i = 0; i < stateArray.count; i++)
  {
    tranys[i] = stateMats[i] * stateArray[i].WorldMatrix() * mat;
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetMatricesModel( const Array<FamilyState> & stateArray, Matrix * tranys) const
{
  Matrix temp;
  temp.SetInverse( stateArray[0].WorldMatrix());

  U32 i;
  for (i = 0; i < stateArray.count; i++)
  {
    tranys[i] = stateMats[i] * stateArray[i].WorldMatrix() * temp;
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetVertsView( const Array<FamilyState> & stateArray, Matrix * tranys, Vector * verts, U32 vCount, Bool doMultiWeight) const
{
  SetMatricesView( stateArray, tranys);

  // transform verts to view space
  U32 i;
  for (i = 0; i < vCount; i++)
  {
    SetVert( tranys, i, verts[i], doMultiWeight);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetVertsWorld( const Array<FamilyState> & stateArray, Matrix * tranys, Vector * verts, U32 vCount, Bool doMultiWeight) const
{
  SetMatricesWorld( stateArray, tranys);

  // transform verts to world space
  U32 i;
  for (i = 0; i < vCount; i++)
  {
    SetVert( tranys, i, verts[i], doMultiWeight);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetVertsWorld( const Array<FamilyState> & stateArray, Vector * verts, U32 vCount, Bool doMultiWeight) const
{
  Matrix tranys[MAXMESHPERGROUP];

  SetMatricesWorld( stateArray, tranys);

  // transform verts to world space
  U32 i;
  for (i = 0; i < vCount; i++)
  {
    SetVert( tranys, i, verts[i], doMultiWeight);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetVertsIdentity( const Array<FamilyState> & stateArray, Vector * verts, U32 vCount, Bool doMultiWeight) const
{
  if (hasAnim)
  {
    Matrix tranys[MAXMESHPERGROUP];

//    SetMatricesWorldInverseZ( stateArray, tranys);
    SetMatricesWorld( stateArray, tranys);

    Matrix * m, * me = tranys + stateArray.count;
    for (m = tranys + 1; m < me; m++)
    {
      m->posit -= tranys->posit;
    }
    tranys->posit.ClearData();

    // transform verts to world space
    VertIndex * vi = vertToState.data;
    Vector * v, * e = verts + vCount, * src = vertices.data;
    for (v = verts; v < e; v++, vi++, src++)
    {
	    // set a single vert with multi-weighting
	    
	    // get the vertex in local coordinates
      // transform by the zeroth matrix
      //
      U32 index = vi->index[0];
 			tranys[index].Transform( *v, *src);

	    // if we are doing multiple weights
      //
      if (doMultiWeight && (vi->count > 1))
	    {
		    // scale by zeroth weight
		    *v *= vi->weight[0];

		    // for each subsequent weight...
		    for (U32 j = 1; j < vi->count; j++)
		    {
			    // transform by the Jth matrix
			    Vector vertJ;

          U32 index = vi->index[j];
 		    	tranys[index].Transform( vertJ, *src);

          // scale by Jth weight
			    vertJ *= vi->weight[j];

			    // accumulate value
			    *v += vertJ;
		    }
	    }
    }
  }
  else
  {
    stateArray[0].WorldMatrix().Rotate( verts, vertices.data, vCount);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightAnimVtl( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags)
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
  SetVertsView( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

//  if ((_controlFlags & controlTRANSLUCENT) || baseColor.a < 255) 
  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | renderFlags);

  ColorF32 base( baseColor);

  // clear indexers
  //
#if 0
  memset( iv, 0xff, sizeof(U16) * bucky.vertices.count);
  memset( in, 0xfe, sizeof(U16) * bucky.vertices.count);
  memset( iu, 0xfd, sizeof(U16) * bucky.vertices.count);
#endif

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
    if (!Vid::LockBucket( bucky, _controlFlags, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

    // calc the ambient diffuse component for each material (bucky)
    //
    bucky.diffInitF32.Set( 
      bucky.diff.r * Vid::renderState.ambientColorF32.r * F32(baseColor.r) * U8toNormF32,
      bucky.diff.g * Vid::renderState.ambientColorF32.g * F32(baseColor.b) * U8toNormF32,
      bucky.diff.b * Vid::renderState.ambientColorF32.b * F32(baseColor.g) * U8toNormF32,
      F32(baseColor.a) * U8toNormF32
    );
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear indexers
    //
/*
    memset( iv, 0xff, sizeof(U16) * bucky.vertices.count);
    memset( in, 0xfe, sizeof(U16) * bucky.vertices.count);
    memset( iu, 0xfd, sizeof(U16) * bucky.vertices.count);
*/
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
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        ASSERT( ivj < vCount && inj < normals.count && iuj < uvs.count);

        if (iv[ivj] != in[inj])
        {
          // new vert
          //
          VertexTL & dv = bucky.CurrVertexTL();
          Vector & sv = verts[ivj];

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

          Vector norm;
          tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);

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

          if (clipFlags == clipNONE)
          {
            Vid::ProjectFromCamera_I( dv, sv);

            // set vertex fog
            dv.SetFog();
          }
          else
          {
            dv.vv = sv;
          }
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
          VertexTL & dv = bucky.CurrVertexTL();

          dv = bucky.GetVertexTL( iv[ivj]);

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

          bucky.SetIndex( (U16) bucky.vCount);
//          iv[ivj] = (U16) bucky.vCount;
//          in[inj] = (U16) bucky.vCount;
//          iu[iuj] = (U16) bucky.vCount;
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
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray) && bucky.vCount > 0)
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

void MeshRoot::RenderLightNoAnimVtl( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags)
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

//  if ((_controlFlags & controlTRANSLUCENT) || baseColor.a < 255) 
  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }
  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | renderFlags);

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
    if (!Vid::LockBucket( bucky, _controlFlags, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

    // calc constant diffuse component for each material (bucky)
    //
    bucky.diffInitF32.Set( 
      bucky.diff.r * Vid::renderState.ambientColorF32.r * F32(baseColor.r) * U8toNormF32,
      bucky.diff.g * Vid::renderState.ambientColorF32.g * F32(baseColor.b) * U8toNormF32,
      bucky.diff.b * Vid::renderState.ambientColorF32.b * F32(baseColor.g) * U8toNormF32,
      F32(baseColor.a) * U8toNormF32
    );
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear indexers
    //
/*
    memset( iv, 0xff, sizeof(U16) * bucky.vertices.count);
    memset( in, 0xfe, sizeof(U16) * bucky.vertices.count);
    memset( iu, 0xfd, sizeof(U16) * bucky.vertices.count);
*/
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

      // backcull
      //
      if (!(bucky.flags0 & RS_2SIDED))
      {
        // non-animating planes are pre-calced
        //
//        Plane &plane = planes[face.index];
        Plane plane;
        plane.Set( vertices[face.verts[0]], vertices[face.verts[1]], vertices[face.verts[2]]);
	  	  if (plane.Evalue(Vid::Math::modelViewVector) <= 0.0f)
        {
          continue;
        }
      }

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
          VertexTL & dv = bucky.CurrVertexTL();
          Vector &sv = vertices[ivj];

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

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

          Vid::TransformFromModel( dv, sv);

          if (clipFlags == clipNONE)
          {
            Vid::ProjectFromCamera_I( dv);

            // set vertex fog
            dv.SetFog();
          }

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
          VertexTL & dv = bucky.CurrVertexTL();

          dv = bucky.GetVertexTL( iv[ivj]);

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

          bucky.SetIndex( (U16) bucky.vCount);
//          iv[ivj] = (U16) bucky.vCount;
//          in[inj] = (U16) bucky.vCount;
//          iu[iuj] = (U16) bucky.vCount;
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
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray) && bucky.vCount > 0)
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

void MeshRoot::RenderColorAnimVtl( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort)
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
  SetVertsView( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

//  if ((_controlFlags & controlTRANSLUCENT) || baseColor.a < 255) 
  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }
  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | renderFlags);

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
    if (!Vid::LockBucket( bucky, _controlFlags, clipFlags, &stateArray, tex, blend))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

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

          VertexTL & dv = bucky.CurrVertexTL();
          Vector & sv = verts[ivj];

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

          if (colors.count)
          {
            dv.diffuse.Modulate( colors[ivj], bucky.diffInitF32.r, bucky.diffInitF32.g, bucky.diffInitF32.b, bucky.diffInitF32.a);
          }
          else
          {
            dv.diffuse  = bucky.diffInitC;
          }
          dv.specular = 0xff000000;

          if (clipFlags == clipNONE)
          {
            Vid::ProjectFromCamera_I( dv, sv);

            // set vertex fog
            dv.SetFog();
          }
          else
          {
            dv.vv = sv;
          }
          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    // flush memory and clip if necessary
    // 
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray) && bucky.vCount > 0)
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

void MeshRoot::RenderColorNoAnimVtl( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort)
{
  vCount;

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  Vector * verts;
  U16 * iv, * iu;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vCount, &iv, vertices.count, &iu, uvs.count);

//  if ((_controlFlags & controlTRANSLUCENT) || baseColor.a < 255) 
  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }
  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | renderFlags);

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
    if (!Vid::LockBucket( bucky, _controlFlags, clipFlags, &stateArray, tex, blend))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

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
//        Plane &plane = planes[face.index];
        Plane plane;
        plane.Set( vertices[face.verts[0]], vertices[face.verts[1]], vertices[face.verts[2]]);
	  	  if (plane.Evalue(Vid::Math::modelViewVector) <= 0.0f)
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

          VertexTL & dv = bucky.CurrVertexTL();
          Vector & sv = vertices[ivj];

          dv.uv = uvs[iuj];
          if (hasTread)
          {
            dv.uv.v += vOffsets[vertToState[ivj].index[0]];
          }

          if (colors.count)
          {
            dv.diffuse.Modulate( colors[ivj], bucky.diffInitF32.r, bucky.diffInitF32.g, bucky.diffInitF32.b, bucky.diffInitF32.a);
          }
          else
          {
            dv.diffuse = bucky.diffInitC;
          }
          dv.specular = 0xff000000;

          Vid::TransformFromModel( dv, sv);

          if (clipFlags == clipNONE)
          {
            Vid::ProjectFromCamera_I( dv);

            // set vertex fog
            dv.SetFog();
          }

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    // flush memory and clip if necessary
    // 
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray) && bucky.vCount > 0)
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

/*
GLE_TEXTURE_VERTEX_SPH
  Uses u = phi/(2*pi) = arctan (vy/vx)/(2*pi) as the texture "u" 
  coordinate, and v = theta/pi = (1.0 - arccos(vz))/pi as the texture "v" 
  coordinate. In the above equation, "vx","vy" and "vz" stand for the vertex's 
  x, y and z coordinates. 

GLE_TEXTURE_NORMAL_SPH
  Uses u = phi/(2*pi) = arctan (ny/nx)/(2*pi) as the texture "u" 
  coordinate, and v = theta/pi = (1.0 - arccos(nz))/pi as the texture "v" 
  coordinate. In the above equation, "nx","ny" and "nz" stand for the normal's 
  x, y and z coordinates. 
*/

void MeshRoot::RenderEnvMapVtl( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort, Bool envmap, Bool smooth, F32 rotate) // = clipALL, NULL, RS_BLEND_DEF, sortEFFECT0, TRUE, FALSE, 0
{
  rotate;
  _controlFlags;

  if (!tex)
  {
    tex = Mesh::Manager::envMap;
  }

  color.Modulate( 
    Vid::Light::sunColor.r * U8toNormF32,
    Vid::Light::sunColor.g * U8toNormF32,
    Vid::Light::sunColor.b * U8toNormF32);

  if (envmap)
  {
    Float2Int fa( color.a * Vid::renderState.envAlpha * U8toNormF32 + Float2Int::magic);
    color.a = U8(fa.i);
  }

  Vector * verts;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vCount);

  Matrix tranys[MAXMESHPERGROUP];
  SetVertsView( stateArray, tranys, verts, vCount, TRUE);

//  const Vector & cam = Vid::CurCamera().WorldMatrix().posit;
  const Matrix & mat = stateArray[0].WorldMatrix();
  Vector ocam;
  Vid::Math::viewMatrix.Transform( ocam, mat.posit);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // setup bucket desc elements common to all faces
  //
  if (color.a < 255)
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | renderFlags);

  BucketMan::SetTag1( sort);

  // setup _buckys and fill them
  //
  FaceGroup * b, * be = _buckys.data + _buckys.count;
  U32 counter = 0;
  for (b = _buckys.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

/*
    if (envmap && !bucky.envMap)
    {
      continue;
    }
*/
    // get memory
    //
    if (!Vid::LockBucket( bucky, 0, clipFlags, &stateArray, tex, blend))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

   // calc constant diffuse component for each material (bucky)
    //
    bucky.diffInitC.SetNoExpand( 
      (Vid::renderState.ambientColorF32.r + 1) * color.r, 
      (Vid::renderState.ambientColorF32.g + 1) * color.g, 
      (Vid::renderState.ambientColorF32.b + 1) * color.b, 
       bucky.diff.a * F32(color.a)
    );

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    for (f = bucky.faces.data; f < fe; f++, counter += 3)
    {
      FaceObj & face = *f;

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
	      U16 inj = face.norms[j];

        // new vertex; light and project
        VertexTL & dv = bucky.CurrVertexTL();
        Vector   & sv = verts[ivj];

        dv.vv = sv;
        dv.diffuse  = bucky.diffInitC;
        dv.specular = 0xff000000;

        Vector norm;
        smooth = TRUE;
        if (smooth)
        {
          // fully curved
          norm = ocam - dv.vv;
          norm.Normalize();
        }
        else
        {
          // possibly faceted
          tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
        }
        
        // spherical reflection mapping
        //
        dv.uv.v = norm.x / 2.0f + .5f;
        dv.uv.u = norm.y / 2.0f + .5f;
/*
        Vector dir = dv.vv * -1;
        dir.Normalize();
        dir = dir - norm * 2.0f * dir.Dot( norm);
//	      dir.Normalize();

        // spherical reflection mapping
        //
        dv.uv.v = (F32)acos( Utils::FSign(dir.y) * dir.z) / PI;
        dv.uv.u = (F32)acos( dir.x) / PI;


        // spherical reflection mapping
        //
        dv.uv.v = (F32)asin( dir.y) / PI;
        dv.uv.u = .5f + (F32)atan2( -dir.x, dir.z) / (2*PI);

        // spherical normal mapping
        //
        dv.uv.v = (F32)asin( norm.y) / PI;
        dv.uv.u = .5f + (F32)atan2( norm.z, norm.x) / (2*PI);

*/
        if (clipFlags == clipNONE)
        {
          Vid::ProjectFromCamera_I( dv);

          // set vertex fog
          dv.SetFog();
        }

        bucky.SetIndex( (U16) bucky.vCount);
        bucky.vCount++;
      }
    }

    // flush memory and clip if necessary
    // 
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray) && bucky.vCount > 0)
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }
  BucketMan::forceTranslucent = FALSE;

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

  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderShadowPlane( U32 number, const Array<FamilyState> & stateArray, Color color, U32 clipFlags) // = clipALL)
{
  clipFlags;

  Mesh * sp = NULL;
  if (number > 1)
  {
    return;
  }
  if (number == 0)
  {
    sp = FindMeshLocal( "SP-0");
  }
  else
  {
    sp = FindMeshLocal( "SP-1");
  }

  if (!sp)
  {
    return;
  }

  Bitmap * tex = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, "engine_terrain_grid.tga");

  Vid::SetWorldTransform( stateArray[sp->GetIndex()].WorldMatrix());

  Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);

  // set the polygon material
  // force translucency
  Vid::SetBucketMaterial( Vid::defMaterial);
  Vid::SetBucketTexture( tex, TRUE);

  ASSERT( sp->local->vertices.count <= Vid::renderState.maxVerts && sp->local->indices.count <= Vid::renderState.maxIndices);

  VertexTL * vertmem;
  U16 * indexmem;
  U32 heapSize = Vid::Heap::ReqVertex( &vertmem, sp->local->vertices.count, &indexmem, sp->local->indices.count);

  for (U32 i = 0; i < sp->local->vertices.count; i++)
  {
    Vid::TransformFromModel( vertmem[i], sp->local->vertices[i]);

    vertmem[i].diffuse  = color;
    vertmem[i].specular = 0xff000000;
    vertmem[i].uv.u = 0.0f;
    vertmem[i].uv.v = 0.0f;
  }
  Utils::Memcpy( indexmem, &sp->local->indices[0], 2 * sp->local->indices.count);

  Vid::Clip::ToBucket( vertmem, i, indexmem, sp->local->indices.count, (void *) sp);

  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderNormals( const Array<FamilyState> & stateArray, Color color, U32 clipFlags) // = clipALL)
{
  clipFlags;

  Bool doMultiWeight = (rootControlFlags & controlMULTIWEIGHT) && Vid::renderState.status.multiWeight ? TRUE : FALSE;

  F32 normLenFactor = ObjectBounds().Radius() * 0.04f;

//  Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_GLOW);
  Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_BLEND_GLOW);

  // set the polygon material
  // force translucency
  Vid::SetBucketMaterial( Vid::defMaterial);
  Vid::SetBucketTexture( NULL, TRUE);

  Vector * verts;
  U16 * iv, * in;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vertices.count, &iv, vertices.count, &in, vertices.count);

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  SetVertsView( stateArray, tranys, verts, vertices.count, doMultiWeight);

  // clear indexer lists
  memset( iv, 0xff, sizeof(U16) * vertices.count);
  memset( in, 0xfe, sizeof(U16) * vertices.count);

  U32 i;
	for (i = 0; i < faces.count; i++)
  {
    FaceObj & face = faces[i];

    U32 j;
    for (j = 0; j < 3; j++)
    {
      U16 ivj = face.verts[j];
      U16 inj = face.norms[j];

      if (iv[ivj] != 0xffff && in[inj] != 0xfefe)
      {
        continue;
      }
      iv[ivj] = 0x00;
      in[inj] = 0x00;

      Vector norm;
      tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
      norm *= normLenFactor;

      Vector &vert = verts[ivj];

  	  // set bucket depth
      Vid::SetTranBucketZ(vert.z, Vid::sortDEBUG0);
    
      // lock vertex memory
	    VertexTL *vertmem;
	    U16 *indexmem;
      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 3, &indexmem, 3))
	    {
        return;
      }

	    vertmem[0].vv.x = vert.x;
	    vertmem[0].vv.y = vert.y;
	    vertmem[0].vv.z = vert.z;
	    vertmem[0].diffuse  = color;
	    vertmem[0].specular = 0xff000000;

      Vid::ProjectFromCamera_I( vertmem[0]);

	    vertmem[1].vv.x = vert.x + norm.x;
	    vertmem[1].vv.y = vert.y + norm.y;
	    vertmem[1].vv.z = vert.z + norm.z;
	    vertmem[1].diffuse  = color;
	    vertmem[1].specular = 0xff000000;

      Vid::ProjectFromCamera_I( vertmem[1]);

	    vertmem[2] = vertmem[0];
      vertmem[0].vv.x -= 1.0f;
      vertmem[2].vv.x += 1.0f;

      Utils::Memcpy( indexmem, Vid::rectIndices, 6);

      // submit the polygons
      Vid::UnlockIndexedPrimitiveMem( 3, 3);
    }
  }
  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderWireframe( const Array<FamilyState> & stateArray, Color color, U32 blend, U32 clipFlags) // = RS_BLEND_DEF, clipALL)
{
  stateArray;
  clipFlags;      // FIXME: check for clip

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_LINELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | RS_NOINDEXED | blend);

  // set the polygon material
  // force translucency
  Vid::SetBucketMaterial( Vid::defMaterial);
  Vid::SetBucketTexture( NULL, TRUE, 0, blend);
  BucketMan::SetTag1( Vid::sortEFFECT0);

  U32 vCount = Min<U32>( faces.count * 3 * 2, Vid::renderState.maxVerts);

  // temp memory
  Vector * temp;
  VertexTL * dstV;
  U32 heapSize = Vid::Heap::ReqVertex( &temp, vertices.count, &dstV, vCount);
  VertexTL * dV = dstV;

  // transform verts to camera space
  Vid::TransformFromModel( temp, vertices.data, vertices.count);

	for (U32 i = 0; i < faces.count; i++)
  {
    FaceObj & face = faces[i];

#if 0
    BucketLock & bucky = groups[face.buckyIndex];
    if (!(bucky.flags0 & RS_2SIDED))
    {
      Plane &plane = planes[face.index];
	    if (plane.Evalue(Vid::Math::modelViewVector) <= 0.0f)
      {
        continue;
      }
    }
#endif

    for (U32 j = 0; j < 3; j++)
    {
      dV->vv = temp[face.verts[j]];
      dV->diffuse = color;
      dV->specular = 0xff000000;
      dV++;

      dV->vv = temp[face.verts[(j+1)%3]];
      dV->diffuse = color;
      dV->specular = 0xff000000;
      dV++;
    }
  }

  Vid::Clip::Line::ToBucket( dstV, vCount, (void *) &stateArray, TRUE, clipFlags); 

  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

void MeshRoot::SetupRenderProc()
{
#if 0
  if (animCycles.GetCount())
  {
    renderProc = MeshRoot::RenderLightAnimVtl;
  }
  else
#endif
  {
    renderProc = MeshRoot::RenderLightNoAnimVtl;
  }

  BucketLock * b, * be = buckys.data + buckys.count;
  for (b = buckys.data; b < be; b++)
  {
    U32 flags = b->flags0 & RS_TEX_MASK; 
    if (flags == RS_TEX_DECAL)
//    if (flags < RS_TEX_MODULATE || flags >= RS_TEX_ADD)
    {
      b->lightProc = FaceGroup::LightDecal;
    }
    else if (animCycles.GetCount())
    {
      b->lightProc = FaceGroup::LightCam;
    }
    else
    {
      b->lightProc = FaceGroup::LightMod;
    }
  }
}
//----------------------------------------------------------------------------

void MeshRoot::Render( const Matrix & world, Color baseColor, U32 _controlFlags) // 0xffffffff, = controlDEF
{
	SetWorldAll( world);

  U32 clipFlags = Vid::CurCamera().BoundsTest( world, bigBounds);
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup _controlFlags
  //
  if (Vid::renderState.status.overlay && (rootControlFlags & controlOVERLAY))
  {
    if (!Vid::renderState.status.texMulti)
    {
      _controlFlags = controlOVERLAY2PASS;
    }
    else
    {
      _controlFlags = controlOVERLAY1PASS;
    }
  }
  else
  {
    _controlFlags &= ~(controlOVERLAY1PASS | controlOVERLAY2PASS);
  }

  if (Vid::renderState.status.envmap)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Vid::renderState.status.multiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  Vector origin;
  world.Transform( origin, ObjectBounds().Offset());
  Vid::Light::SetActiveList( origin, ObjectBounds());

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    Vid::Light::SetupLightsCamera();

    RenderLightAnim( baseColor, clipFlags, _controlFlags);
  }
  else
  {
    Vid::Light::SetupLightsModel();

    RenderLightNoAnim( baseColor, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColor( const Matrix &world, Color color, U32 _controlFlags) // = controlDEF)
{
	SetWorldAll( world);

  U32 clipFlags = Vid::CurCamera().BoundsTest( world, bigBounds);
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup _controlFlags
  //
  if (Vid::renderState.status.overlay && color.a == 255 && (rootControlFlags & controlOVERLAY))
  {
    if (!Vid::renderState.status.texMulti)
    {
      _controlFlags = controlOVERLAY2PASS;
    }
    else
    {
      _controlFlags = controlOVERLAY1PASS;
    }
  }
  else
  {
    _controlFlags &= ~(controlOVERLAY1PASS | controlOVERLAY2PASS);
  }

  if (Vid::renderState.status.envmap && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Vid::renderState.status.multiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    RenderColorAnim( color, clipFlags, _controlFlags);
  }
  else
  {
    RenderColorNoAnim( color, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorLight( const Matrix &world, Color color, U32 _controlFlags) // = controlDEF)
{
	SetWorldAll( world);

  U32 clipFlags = Vid::CurCamera().BoundsTest( world, bigBounds);
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup _controlFlags
  //
  if (Vid::renderState.status.overlay && color.a == 255 && (rootControlFlags & controlOVERLAY))
  {
    if (!Vid::renderState.status.texMulti)
    {
      _controlFlags = controlOVERLAY2PASS;
    }
    else
    {
      _controlFlags = controlOVERLAY1PASS;
    }
  }
  else
  {
    _controlFlags &= ~(controlOVERLAY1PASS | controlOVERLAY2PASS);
  }

  if (Vid::renderState.status.envmap && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Vid::renderState.status.multiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  Vector origin;
  world.Transform( origin, ObjectBounds().Offset());
  Vid::Light::SetActiveList( origin, ObjectBounds());

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    Vid::Light::SetupLightsCamera();

    RenderLightAnim( color, clipFlags, _controlFlags);
  }
  else
  {
    Vid::Light::SetupLightsModel();

    RenderLightNoAnim( color, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightSun( const Matrix &world, Color baseColor, U32 _controlFlags) // = controlDEF
{
	SetWorldAll( world);

  U32 clipFlags = Vid::CurCamera().BoundsTest( world, bigBounds);
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup _controlFlags
  //
  if (Vid::renderState.status.overlay && (rootControlFlags & controlOVERLAY))
  {
    if (!Vid::renderState.status.texMulti)
    {
      _controlFlags = controlOVERLAY2PASS;
    }
    else
    {
      _controlFlags = controlOVERLAY1PASS;
    }
  }
  else
  {
    _controlFlags &= ~(controlOVERLAY1PASS | controlOVERLAY2PASS);
  }

  if (Vid::renderState.status.envmap)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Vid::renderState.status.multiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  Vid::Light::SetActiveListSun();

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    Vid::Light::SetupLightsCamera();

    RenderLightAnim( baseColor, clipFlags, _controlFlags);
  }
  else
  {
    Vid::Light::SetupLightsModel();

    RenderLightNoAnim( baseColor, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorLightSun( const Matrix &world, Color color, U32 _controlFlags) // = controlDEF)
{
	SetWorldAll( world);

  U32 clipFlags = Vid::CurCamera().BoundsTest( world, bigBounds);
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup _controlFlags
  //
  if (Vid::renderState.status.overlay && color.a == 255 && (rootControlFlags & controlOVERLAY))
  {
    if (!Vid::renderState.status.texMulti)
    {
      _controlFlags = controlOVERLAY2PASS;
    }
    else
    {
      _controlFlags = controlOVERLAY1PASS;
    }
  }
  else
  {
    _controlFlags &= ~(controlOVERLAY1PASS | controlOVERLAY2PASS);
  }

  if (Vid::renderState.status.envmap && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Vid::renderState.status.multiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  Vid::Light::SetActiveListSun();

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    Vid::Light::SetupLightsCamera();

    RenderLightAnim( color, clipFlags, _controlFlags);
  }
  else
  {
    Vid::Light::SetupLightsModel();

    RenderLightNoAnim( color, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderSelVerts( Bool showverts, const List<U16> * verts, const Array<FamilyState> & stateArray, Color color0, Color color1, U32 clipFlags) // = clipALL)
{
  clipFlags;

  Bool doMultiWeight = (rootControlFlags & controlMULTIWEIGHT) && Vid::renderState.status.multiWeight ? TRUE : FALSE;

  // set up transform matrices
  Matrix tranys[MAXMESHPERGROUP];
  SetMatricesView( stateArray, tranys);

  U32 heapSize = vertices.count;
  U8 * hits = (U8 *) Vid::Heap::Request( heapSize);
  Utils::Memset( hits, 0, heapSize);

  Vector vect;
  VertexTL vertex;

  if (verts)
  {
    // transform verts and clip in Z
    //
    List<U16>::Iterator vi( verts);
    while (U16 * index = vi++)
    {
      SetVert( tranys, *index, vect, doMultiWeight);

      hits[*index] = TRUE;

      if (vect.z < Vid::Math::nearPlane || vect.z >= Vid::Math::farPlane)
      {
        // 3D clip
        continue;
      }

      // project
      //
      Vid::ProjectFromCamera_I( vertex, vect);
//      vertex.vv.z *= Vid::renderState.zBias * Vid::renderState.zBias;

      Vid::RenderRectangleOutline( vertex.vv, 3, color0, RS_BLEND_DEF, Vid::sortDEBUG0, vertex.vv.z, vertex.rhw);
    }
  }
  if (showverts)
  {
    // show all verts
    //
    U32 i;
    for (i = 0; i < vertices.count; i++)
    {
      if (hits[i])
      {
        continue;
      }
      SetVert( tranys, i, vect, doMultiWeight);

      if (vect.z < Vid::Math::nearPlane || vect.z >= Vid::Math::farPlane)
      {
        // 3D clip
        continue;
      }

      // project
      //
      Vid::ProjectFromCamera_I( vertex, vect);
//      vertex.vv.z *= Vid::renderState.zBias * Vid::renderState.zBias;

      Vid::RenderRectangleOutline( vertex.vv, 3, color1, RS_BLEND_DEF, Vid::sortDEBUG0, vertex.vv.z, vertex.rhw);
    }
  }

  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderSelFaces( Bool show, const List<U16> * faces, const Array<FamilyState> & stateArray, Color color0, Color color1, U32 clipFlags) // = clipALL)
{
  show;
  clipFlags;
  color0;
  color1;
  faces;
  stateArray;

/*
  // setup _controlFlags
  //
  U32 _controlFlags;

  if (Vid::renderState.status.multiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts, vertices.count, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

*/
}
//----------------------------------------------------------------------------

void MeshRoot::RenderShadowTextureGeneric( Color color, U32 blend, U32 jitter, Bool doBuild) // = FALSE
{
  jitter;

  GameIdent gi;
  sprintf( gi.str, "shadow_%s.tga", xsiName.str);

  shadowTexture = Bitmap::Manager::FindCreate( Bitmap::reduceHIGH, gi.str, 0, bitmapTEXTURE, 0, TRUE, FALSE);
  if (!shadowTexture)
  {
    if (!doBuild)
    {
      return;
    }
    shadowTexture = new Bitmap( Bitmap::reduceNONE, gi.str, 0, bitmapTEXTURE | bitmapNORELOAD);
    ASSERT( shadowTexture);
    shadowTexture->Create( Vid::renderState.texShadowSize, Vid::renderState.texShadowSize, TRUE, 0, 0);
  }
  else if (!doBuild)
  {
    return;
  }
  Vector * verts;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vertices.count);

  // clear the dirty rectangle
  //
  shadowInfo.size.Set( F32_MAX, F32_MAX, -F32_MAX, -F32_MAX);

  // for all the light sources
  //
  RenderShadowVerts( Vector(0,-1,0), shadowInfo.size, 0, verts, vertices.count, states, rootControlFlags, color, blend);

  // store the size and position of the shadow rectangle
  //
  shadowInfo.size.p1.x = shadowInfo.size.p1.x - shadowInfo.size.p0.x;
  shadowInfo.size.p1.y = shadowInfo.size.p1.y - shadowInfo.size.p0.y;

  shadowInfo.radxRender = 0;
  shadowInfo.radyRender = 0;

  if (shadowInfo.size.p1.x != 0 && shadowInfo.size.p1.y != 0)
  {
    RenderShadowTex( *shadowTexture, shadowInfo.size, verts, 1, vertices.count, &color, groups, blend, jitter, TRUE);

    shadowInfo.radxRender = F32(shadowInfo.size.p1.x) * .5f;
    shadowInfo.radyRender = F32(shadowInfo.size.p1.y) * .5f;
    shadowInfo.p1.Set( (F32)shadowInfo.size.p0.x + shadowInfo.radxRender, 0, (F32)shadowInfo.size.p0.y + shadowInfo.radyRender);

    shadowInfo.radxRender *= (F32(shadowTexture->Width()) + 4.4f) / F32(shadowTexture->Width());
    shadowInfo.radyRender *= (F32(shadowTexture->Width()) + 4.4f) / F32(shadowTexture->Width());
  }
  shadowInfo.p2 = shadowInfo.p1;

  // restore temp memory
  //
  Vid::Heap::Restore( heapSize);  

  Dir::MakeFull( Vid::Var::godFilePath);

  FileDrive drive;
  FileDir dir;
  FileName name;
  FileExt ext;
  Dir::PathExpand( gi.str, drive, dir, name, ext);  

  FilePath path;
  Dir::PathMake( path, NULL, Vid::Var::godFilePath, name.str, "tga");

  shadowTexture->WriteTGA( path.str, TRUE);
};
//----------------------------------------------------------------------------

void MeshRoot::RenderShadowTexture( ShadowInfo & si, const Matrix ** lightA, U32 lCount, Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, U32 _controlFlags, Color color, U32 blend) //  = 0x00000000, NULL, RS_BLEND_DEF
{
  Vector * verts;
  U32 heapSize = Vid::Heap::ReqMesh( &verts, vCount * lCount);

  // clear the dirty rectangle
  //
  si.size.Set( F32_MAX, F32_MAX, -F32_MAX, -F32_MAX);

  // for all the light sources
  //
  const MAXSHADOWS = 5;
  Color cA[MAXSHADOWS];
  const Matrix ** m = lightA, ** me = lightA + lCount;
  Vector * vA, l1 = (*m)->front;
  U32 i = 0;
  for (vA = verts; i < MAXSHADOWS && m < me; vA += vCount, m++, i++)
  {
    cA[i] = color;

    if (m == lightA)
    {
      // assumes the first light is a direct light (the sun)
      //
//      RenderShadowVerts( (*m)->front, si.size, 0, vA, vCount, stateArray, _controlFlags, cA[i], blend);
      RenderShadowVerts( Vid::Light::shadowMatrix.front, si.size, 0, vA, vCount, stateArray, _controlFlags, cA[i], blend);
    }
    else
    {
      // all other lights are point sources
      //
      RenderShadowVerts( (*m)->posit, si.size, 1, vA, vCount, stateArray, _controlFlags, cA[i], blend);
    }
  }

  // store the size and position of the shadow rectangle
  //
  si.size.p1.x = si.size.p1.x - si.size.p0.x;
  si.size.p1.y = si.size.p1.y - si.size.p0.y;

  if (si.size.p1.x != 0 && si.size.p1.y != 0)
  {
    ASSERT( si.texture);
    RenderShadowTex( *si.texture, si.size, verts, lCount, vCount, cA, _buckys, blend);

    si.radx = si.radxRender = (F32)si.size.p1.x * .5f;
    si.rady = si.radyRender = (F32)si.size.p1.y * .5f;
    si.p1.Set( (F32)si.size.p0.x + si.radx, 0, (F32)si.size.p0.y + si.rady);
  }

  // restore temp memory
  //
  Vid::Heap::Restore( heapSize);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderShadowVerts( const Vector & vect, Area<F32> & size, U32 type, Vector * vA, U32 vCount, const Array<FamilyState> & stateArray, U32 _controlFlags, Color color, U32 blend) //  = 0x00000000, NULL, RS_BLEND_DEF
{
  blend;

  // get world verts
  //
  SetVertsIdentity( stateArray, vA, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

  Vector light = vect;
  if (type)
  {
    // and all subsequent lights are point sources
    //
    light = stateArray[0].WorldMatrix().posit - light;
    F32 dist = light.Normalize(); 
    F32 shadF = bigBounds.Radius() * 14.0f;
    if (dist > bigBounds.Radius() * 3.0f / 200.0f * Vid::Math::farPlane)
    {
      // distance fade
      //
      F32 sf = (1 - Min<F32>(dist, shadF) / shadF);
      color.Modulate( sf, sf, sf, sf);

      if (color.r < Vid::renderState.shadowFadeCutoff)
      {
        // to weak to cast
        //
        return;
      }
    }
  }
  if (light.y > -Vid::renderState.shadowY)
  {
    // clamp angle
    //
    light.y = -Vid::renderState.shadowY;
    light.Normalize();
  }
  Vector light1( (F32) fabs(light.x), (F32) fabs(light.y), (F32) fabs(light.z)); 

  // project the light vector thru the verts to the ground (world.y = 0)
  // keep the 2d coords (x,z)
  //
  Vector * v, * ve = vA + vCount;
  for (v = vA; v < ve; v++)
  {
    // parametric scale of x and z by (vertex->y / light.y)
    //
    if (light1.y > F32_EPSILON)
    {
      F32 t = v->y / light1.y;
      v->x  = v->x + light.x * t; 
      v->y  = v->z + light.z * t; 
    }
    v->z  = 0;

    // keep the dirty rect
    //
    if (v->x < size.p0.x)
    {
      size.p0.x = v->x;
    }
    else if (v->x > size.p1.x)
    {
      size.p1.x = v->x;
    }
    if (v->y < size.p0.y)
    {
      size.p0.y = v->y;
    }
    else if (v->y > size.p1.y)
    {
      size.p1.y = v->y;
    }
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderShadowTex( Bitmap & dstT, Area<F32> & size, const Vector * vA, U32 lCount, U32 vCount, const Color * cA, Array<FaceGroup> & _buckys, U32 blend, U32 jitter, Bool fitOneWay) //S_BLEND_DEF)
{
  jitter;

  if (!dstT.GetSurface())
  {
    dstT.Create( Vid::renderState.texShadowSize, Vid::renderState.texShadowSize, TRUE, 0, 0);
  }

  VertexTL * dstV;
  U16 * dstI, * iv, * iu;
  U32 heapSize = Vid::Heap::ReqVertex( &dstV, &dstI, &iv, vertices.count, &iu, uvs.count);

  // calculate ground rect to texture rect scaling values
  // keep a 4 pix border
  //
  const F32 border = 2.0f;
  Area<F32> sz = size;
  if (fitOneWay)
  {
    if (sz.p1.x > sz.p1.y)
    {
      sz.p0.y -= (sz.p1.x - sz.p1.y) * .5f;
      sz.p1.y = sz.p1.x;
    }
    else
    {
      sz.p0.x -= (sz.p1.y - sz.p1.x) * .5f;
      sz.p1.x = sz.p1.y;
    }
  }
  sz.p1.x = F32( dstT.Width()  - 2 * border) / sz.p1.x;
  sz.p1.y = F32( dstT.Height() - 2 * border) / sz.p1.y;

  Color back;
  if (Vid::ValidateBlend( RS_BLEND_DECALALPHA))
  {
    // use texture alpha only and alpha replicate blt
    //
    blend =  RS_BLEND_DECALALPHA;
    back = 0xff000000;
  }
  else  // no alpha only blend, resort to color keying
  {
    blend = RS_BLEND_MODULATE;
    back = 0xff00ffff;
  }

  Vid::RenderBegin();
  Area<S32> r( 0, 0, dstT.Width(), dstT.Height());
  Vid::ClearBack( back, &r);
  Vid::SetAlphaState( TRUE);

//  Vid::SetTexture( tex, 0, blend);

  // for all the light sources
  //
  const Vector * vAe = vA + lCount * vCount;
  const Color * c = cA;
  for ( ; vA < vAe; vA += vCount, c++)
  {
    ASSERT( _buckys.count <= MAXBUCKYS);
    ASSERT( _buckys.count == groups.count);

  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    // render the tris
    //
    FaceGroup * b, * be = _buckys.data + _buckys.count;
    for (b = _buckys.data; b < be; b++)
    {
      FaceGroup & bucky = *b;

      // set up material
      //
      Vid::SetTexture( bucky.texture0, 0, blend);
      Color cc = *c;
      Float2Int fa( cc.a * bucky.diff.a + Float2Int::magic);
      cc.a = (U8) fa.i;

      // clear indexer list
      //
      memset( iv, 0xff, sizeof(U16) * vertices.count);
      memset( iu, 0xfd, sizeof(U16) * uvs.count);

      // temp memory
      //
      VertexTL * dv = dstV;
      U16 * di = dstI;
      U32 count = 0;

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

          ASSERT( ivj < vCount);

          if (iv[ivj] == 0xffff)
          {
            // new vertex
            // shift and scale verts into texture space
            //
            const Vector & src = vA[ivj];

            dv->vv.x = (src.x - sz.p0.x) * sz.p1.x + border;
            dv->vv.y = (src.y - sz.p0.y) * sz.p1.y + border;
            dv->vv.z = 0;
            dv->rhw = 1;
            dv->diffuse  = cc;
            dv->specular = 0xff000000;
            dv->uv = uvs[iuj];

            iv[ivj] = (U16)count;
            iu[iuj] = (U16)count;
            count++;
            dv++;
          }
          else if (iv[ivj] != iu[iuj])
          {
            *dv = dstV[iv[ivj]];
            dv->uv = uvs[iuj];

            iu[iuj] = (U16)count;
            count++;
            dv++;
          }
          *di++ = iv[ivj];
        }
      }
      if (count)
      {
        // draw this group
        //
		    Vid::DrawIndexedPrimitive(
              PT_TRIANGLELIST,
              FVF_TLVERTEX,
              dstV, count, dstI, di - dstI, 
              DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | blend);
      }
    }
  }
  Vid::Heap::Restore( heapSize);

  Vid::SetAlphaState( FALSE);

  // clear the shadow texture
  //
  dstT.Clear( 0);

  // finished rendering
  //
  Vid::RenderEnd();

  // copy backbuffer to shadow texture with back as a color key
  //
  Vid::backBmp.CopyBits
  ( 
    dstT, 0, 0, 0, 0, 
    dstT.Width(), dstT.Height(), &back, 0, 
    back == 0xff000000 ? 0 : 255 // alpha = 0 is alpha replicate
  );
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightAnim( Color teamColor, U32 clipFlags, U32 _controlFlags)
{
  if (Vid::renderState.status.dxTL)
  {
    RenderLightAnimV( groups, vertices.count, states, teamColor, clipFlags, _controlFlags);
  }
  else
  {
    RenderLightAnimVtl( groups, vertices.count, states, teamColor, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightNoAnim( Color teamColor, U32 clipFlags, U32 _controlFlags)
{
  if (Vid::renderState.status.dxTL)
  {
    RenderLightNoAnimV( groups, vertices.count, states, teamColor, clipFlags, _controlFlags);
  }
  else
  {
    RenderLightNoAnimVtl( groups, vertices.count, states, teamColor, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorAnim( Color color, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort)
{
  if (Vid::renderState.status.dxTL)
  {
    RenderColorAnimV( groups, vertices.count, states, color, clipFlags, _controlFlags, tex, blend, sort);
  }
  else
  {
    RenderColorAnimVtl( groups, vertices.count, states, color, clipFlags, _controlFlags, tex, blend, sort);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorNoAnim( Color color, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort)
{
  if (Vid::renderState.status.dxTL)
  {  
    RenderColorNoAnimV( groups, vertices.count, states, color, clipFlags, _controlFlags, tex, blend, sort);
  }
  else
  {
    RenderColorNoAnimVtl( groups, vertices.count, states, color, clipFlags, _controlFlags, tex, blend, sort);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderEnvMap( Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex, U32 blend, U16 sort, Bool envmap, Bool smooth, F32 rotate)
{
  if (Vid::renderState.status.dxTL)
  {
    RenderEnvMapVtl( groups, vertices.count, states, baseColor, clipFlags, _controlFlags, tex, blend, sort, envmap, smooth, rotate);
  }
  else
  {
    RenderEnvMapVtl( groups, vertices.count, states, baseColor, clipFlags, _controlFlags, tex, blend, sort, envmap, smooth, rotate);
  }
}
//----------------------------------------------------------------------------