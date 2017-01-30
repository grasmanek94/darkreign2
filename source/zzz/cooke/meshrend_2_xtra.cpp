///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshrend.cpp
//
// 07-JUL-1998
//

#include "mesh.h"
#include "IMESHUtil.h"
#include "perfstats.h"
#include "statistics.h"
//----------------------------------------------------------------------------

// indexer lists for verts (one for each bucky)
static U16 ivA[MAXBUCKYS][MAXVERTS];
static U16 inA[MAXBUCKYS][MAXVERTS];
static U16 iuA[MAXBUCKYS][MAXVERTS];

static FaceObj *fA[MAXBUCKYS][MAXTRIS];
static U16 vvA[MAXBUCKYS][MAXINDICES];

static Vector verts[MAXVERTS];
//----------------------------------------------------------------------------

void MeshRoot::RenderLightAnimI_2( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;

  // indexers
  U16 *iv = ivA[0];
  U16 *in = inA[0];
  U16 *iu = iuA[0];

  for (i = 0; i < buckyCount; i++, iv++, in++, iu++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[i][0], 0xfe, sizeof(U16) * normCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;
    bucky.fCount = 0;

    bucky.usedVCount = 0;
    bucky.usedICount = 0;

    if (clipFlags == clipNOCLIP)
    {
      if (!Vid::LockBucket( bucky, _controlFlags, vertCount, faceCount, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
        return;
      }
    }
    else
    {
      bucky.vert  = Vid::tempVertices;
      bucky.index = Vid::tempIndices;
    }

    // backcull
    FaceObj * f, * fe = bucky.faces.data + bucky.faces.count;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;

      ASSERT( face.verts[0] < vertCount && face.verts[1] < vertCount && face.verts[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
        Plane plane;
        plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        {
          continue;
        }
      }
      bucky.fCount++;

      U32 j;
      for (j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        if (iv[ivj] != in[inj])
        {
          // new vert
          //
          VertexTL & dv = bucky.CurrVertexTL();

          Vector &sv = verts[ivj];
          dv.uv = uvs[iuj];

          Vector norm;
          tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
          bucky.Light( dv, sv, norm, baseColor); 

          if (clipFlags == clipNOCLIP)
          {
            cam.ProjectFromCameraSpace( dv, sv);
          }
          else
          {
            cam.SetHomogeneousFromCameraSpace( dv, sv);
          }

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          bucky.vCount++;
        }
        else if (iv[ivj] != iu[iuj])
        {
          // old vert with new uv
          //
          VertexTL & dv = bucky.CurrVertexTL();

          dv = bucky.GetVertexTL( *vv);
          dv.uv = uvs[iuj];

          bucky.SetIndex( (U16) bucky.vCount);
          bucky.vCount++;
        }
        else
        {
          // same old vert 
          //
          bucky.SetIndex( *vv);
        }
      }

      if (clipFlags == clipNOCLIP)
      {
        Vid::UnLockBucket( bucky);
      }
      else
      {
        Vid::SetBucket( bucky, _controlFlags);

        if (!cam.ClipToBucket( bucky, &stateArray))
        {
          LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
        }
      }

#ifdef DOSTATISTICS
      Statistics::tempTris += bucky.fCount;
#endif
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightAnimColorsI_2( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF                                      
{
  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();
  ColorValue base( baseColor);

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[i][0], 0xfe, sizeof(U16) * normCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;
    bucky.fCount = 0;

    bucky.usedVCount = 0;
    bucky.usedICount = 0;
  }

  // backcull
//	for (i = 0; i < faceCount; i++)
  FaceObj * f, * fe = faces.data + faceCount;
  for (f = faces.data; f < fe; f++)
  {
//    FaceObj & face = faces[i];
    FaceObj & face = *f;
    ASSERT( face.buckyIndex < buckyCount);

    BucketLock & bucky = _buckys[face.buckyIndex];

	  U16 ivi[3];
    ivi[0] = face.verts[0];
    ivi[1] = face.verts[1];
    ivi[2] = face.verts[2];
    ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

    if (!(bucky.flags0 & RS_2SIDED))
    {
      Plane plane;
//      plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//      if (plane.Dot( verts[ivi[0]]) >= 0.0f)
      plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
      if (plane.Dot( verts[ivi[0]]) <= 0.0f)
      {
        continue;
      }
    }
    fA[face.buckyIndex][bucky.fCount] = f;
    bucky.fCount++;

    face.state = 0;

    // indexers for this bucky
    U16 *iv = ivA[face.buckyIndex];
    U16 *in = inA[face.buckyIndex];
    U16 *iu = iuA[face.buckyIndex];
    U16 *vv = &vvA[face.buckyIndex][bucky.usedICount];

    U32 j;
    for (j = 0; j < 3; j++, vv++, bucky.usedICount++)
    {
      U16 ivj = ivi[j];
	    U16 inj = face.norms[j];
	    U16 iuj = face.uvs[j];

      if (iv[ivj] != in[inj])
      {
//        *vv = (U16) bucky.usedVCount;
        iv[ivj] = (U16) bucky.usedVCount;
        in[inj] = (U16) bucky.usedVCount;
        iu[iuj] = (U16) bucky.usedVCount;
        bucky.usedVCount++;
      }
      else if (iv[ivj] != iu[iuj])
      {
        *vv = iv[ivj];
        in[inj] = (U16) bucky.usedVCount;
        iu[iuj] = (U16) bucky.usedVCount;
        bucky.usedVCount++;

        face.state |= (1 << (j*2));
      }
      else
      {
        *vv = iv[ivj];

        face.state |= (2 << (j*2));
      }
    }
  }

  if (clipFlags == clipNONE)
  {
    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];

      if (!bucky.usedICount)
      {
        continue;
      }

      Vid::LockBucket( bucky, _controlFlags, &stateArray);

#ifdef DOSTATISTICS
      Statistics::tempTris += bucky.fCount;
#endif

      // indexers for this bucky
      U16 *vv = vvA[i];

//      U32 j;
//      for (j = 0; j < bucky.fCount; j++)
      FaceObj ** f, ** fe = fA[i] + bucky.fCount;
      for (f = fA[i]; f < fe; f++)
      {
//        FaceObj & face = faces[ff[j]];
        FaceObj & face = **f;

        U32 j;
        for (j = 0; j < 3; j++, vv++)
        {
          U16 ivj = face.verts[j];
	        U16 inj = face.norms[j];
	        U16 iuj = face.uvs[j];

          VertexTL &dv = bucky.CurrVertexTL();

          U32 state = U32((face.state >> (j*2)) & 3);
          switch (state)
          {
          case 2:
            // identical vertex; just set index
            bucky.SetIndex( *vv);
            break;
          case 1:
            // already lit; copy vert
            dv = bucky.GetVertexTL( *vv);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            bucky.vCount++;
            break;
          case 0:
            // new vertex; light and project
            Vector &sv = verts[ivj];
            dv.uv = uvs[iuj];

            Color c;
            c.Modulate( colors[ivj], base.r, base.g, base.b);

            bucky.Light( dv, sv, normals[inj], c); 

            cam.ProjectFromCameraSpace( dv, sv);

            // set vertex fog
            dv.SetFog();

            bucky.SetIndex( (U16) bucky.vCount);
            bucky.vCount++;
            break;
          }
        }
      }
      Vid::UnLockBucket( bucky);
    }
  }
  else
  {
    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];

      if (!bucky.usedICount)
      {
        continue;
      }

      bucky.vert  = Vid::tempVertices;
      bucky.index = Vid::tempIndices;

      // indexers for this bucky
      U16 *vv = vvA[i];

//      U32 j;
//      for (j = 0; j < bucky.fCount; j++)
      FaceObj ** f, ** fe = fA[i] + bucky.fCount;
      for (f = fA[i]; f < fe; f++)
      {
//        FaceObj & face = faces[ff[j]];
        FaceObj & face = **f;

        U32 j;
        for (j = 0; j < 3; j++, vv++)
        {
          U16 ivj = face.verts[j];
	        U16 inj = face.norms[j];
	        U16 iuj = face.uvs[j];

          VertexTL &dv = bucky.CurrVertexTL();

          U32 state = U32((face.state >> (j*2)) & 3);
          switch (state)
          {
          case 2:
            // identical vertex; just set index
            bucky.SetIndex( *vv);
            break;
          case 1:
            // already lit; copy vert
            dv = bucky.GetVertexTL( *vv);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            bucky.vCount++;
            break;
          case 0:
            // new vertex; light and project
            Vector &sv = verts[ivj];
            dv.uv = uvs[iuj];

            Color c;
            c.Modulate( colors[ivj], base.r, base.g, base.b);

            bucky.Light( dv, sv, normals[inj], c); 

            cam.SetHomogeneousFromCameraSpace( dv, sv);

            bucky.SetIndex( (U16) bucky.vCount);
            bucky.vCount++;
            break;
          }
        }
      }
      Vid::SetBucket( bucky, _controlFlags);

      if (!cam.ClipToBucket( bucky, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

#if 0
#define DOCALCPLANES

void MeshRoot::RenderLightNoAnimI( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  baseColor;
  stateArray;

  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[i][0], 0xfe, sizeof(U16) * normCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = tempBuckets[i].verts;
    bucky.index = tempBuckets[i].indices;

#ifdef _DEBUG
//    MSWRITEV(22, (5+i, 0,  "texture0 : %s          ", bucky.texture0 ? bucky.texture0->GetName() : "null"));
//    MSWRITEV(22, (5+i, 44, "texture1 : %s          ", bucky.texture1 ? bucky.texture1->GetName() : "null"));
#endif
  }

  if (clipFlags == clipNONE)
  {
    // setup and lock _buckys
    if (!Vid::LockBuckets( &_buckys[0], buckyCount, _controlFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );

      BucketMan::forceTranslucent = FALSE;
      return;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

#ifdef DOSTATISTICS
      Statistics::tempTris++;
#endif

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *in = inA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            bucky.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = bucky.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            iv[ivj] = (U16) bucky.vCount;
            in[inj] = (U16) bucky.vCount;
            iu[iuj] = (U16) bucky.vCount;
            bucky.vCount++;
          }
        }
        else
        {
          // new vertex; light and project
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

#if 1
          bucky.Light( dv, sv, normals[inj], baseColor); 

          cam.TransformProjectFromModelSpace( dv, sv);
#else

          Vector norm;
          ColorValue diff, spec = { 0.0f, 0.0f, 0.0f, 1.0f };

          diff = bucky.diffInit;

          DxLight::Manager::LightModelSpace( sv, normals[inj], bucky.diff, *bucky.material, diff, spec);

          cam.TransformProjectFromModelSpace( dv, sv);

          // set the colors
          dv.diffuse.SetNoExpand( 
            baseColor.r * diff.r, 
            baseColor.g * diff.g, 
            baseColor.b * diff.b,      
            bucky.diffInitC);

          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

#endif

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }
    Vid::UnLockBuckets( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *in = inA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            bucky.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = bucky.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            iv[ivj] = (U16) bucky.vCount;
            in[inj] = (U16) bucky.vCount;
            iu[iuj] = (U16) bucky.vCount;
            bucky.vCount++;
          }
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

#if 1
          bucky.Light( dv, sv, normals[inj], baseColor); 

          cam.SetHomogeneousFromModelSpace( dv, sv);
#else

          Vector norm;
          ColorValue diff, spec = { 0.0f, 0.0f, 0.0f, 1.0f };

          diff = bucky.diffInit;

          DxLight::Manager::LightModelSpace( sv, normals[inj], bucky.diff, *bucky.material, diff, spec);

          cam.SetHomogeneousFromModelSpace( dv, sv);

            // set the colors
          dv.diffuse.SetNoExpand( 
            baseColor.r * diff.r, 
            baseColor.g * diff.g, 
            baseColor.b * diff.b,      
            bucky.diffInitC);

          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

#endif

#ifdef DOSETFOGH
          dv.SetFogH();
#endif

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];
      Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | bucky.flags0);
      Vid::SetBucketTexture( bucky.texture0, FALSE, 0, bucky.flags0);

      if (Vid::isStatus.multitext && bucky.overlay && (_controlFlags & controlOVERLAY))
      {
        Vid::SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
      }

      if (!cam.ClipToBucket( bucky, &stateArray))
//      if (!cam.ClipToBucket( tbucket.verts, bucky.vCount, tbucket.indices, tbucket.indexCount))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightNoAnimColorsI( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  baseColor;
  stateArray;

  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[i][0], 0xfe, sizeof(U16) * normCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = tempBuckets[i].verts;
    bucky.index = tempBuckets[i].indices;
  }

  if (clipFlags == clipNONE)
  {
    // setup and lock _buckys
    if (!Vid::LockBuckets( &_buckys[0], buckyCount, _controlFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );

      BucketMan::forceTranslucent = FALSE;
      return;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

#ifdef DOSTATISTICS
      Statistics::tempTris++;
#endif

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *in = inA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            bucky.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = bucky.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            iv[ivj] = (U16) bucky.vCount;
            in[inj] = (U16) bucky.vCount;
            iu[iuj] = (U16) bucky.vCount;
            bucky.vCount++;
          }
        }
        else
        {
          // new vertex; light and project
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

#if 1
          Color c;
          c.Modulate( colors[ivj], 
            F32(baseColor.r) * U8toNormF32, 
            F32(baseColor.g) * U8toNormF32, 
            F32(baseColor.b) * U8toNormF32);

          bucky.Light( dv, sv, normals[inj], c); 

          cam.TransformProjectFromModelSpace( dv, sv);
#else
          diff = bucky.diffInit;

          DxLight::Manager::LightModelSpace( sv, normals[inj], bucky.diff, *bucky.material, diff, spec);

          cam.TransformProjectFromModelSpace( dv, sv);

          // set the colors
//          dv.diffuse.Set(  diff.r, diff.g, diff.b, bucky.diffInitC);
          dv.diffuse.Modulate( colors[ivj], 
            diff.r * F32(baseColor.r) * U8toNormF32, 
            diff.g * F32(baseColor.g) * U8toNormF32, 
            diff.b * F32(baseColor.b) * U8toNormF32, 
                 F32(bucky.diffInitC) * U8toNormF32);

          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);
#endif

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }
    Vid::UnLockBuckets( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *in = inA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            bucky.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = bucky.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            iv[ivj] = (U16) bucky.vCount;
            in[inj] = (U16) bucky.vCount;
            iu[iuj] = (U16) bucky.vCount;
            bucky.vCount++;
          }
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

#if 1
          Color c;
          c.Modulate( colors[ivj], 
            F32(baseColor.r) * U8toNormF32, 
            F32(baseColor.g) * U8toNormF32, 
            F32(baseColor.b) * U8toNormF32);

          bucky.Light( dv, sv, normals[inj], c); 

          cam.SetHomogeneousFromModelSpace( dv, sv);
#else

          diff = bucky.diffInit;

          DxLight::Manager::LightModelSpace( sv, normals[inj], bucky.diff, *bucky.material, diff, spec);

          cam.SetHomogeneousFromModelSpace( dv, sv);

            // set the colors
//          dv.diffuse.Set(  diff.r, diff.g, diff.b, bucky.diffInitC);
          dv.diffuse.Modulate( colors[ivj], 
            diff.r * F32(baseColor.r) * U8toNormF32, 
            diff.g * F32(baseColor.g) * U8toNormF32, 
            diff.b * F32(baseColor.b) * U8toNormF32, 
                 F32(bucky.diffInitC) * U8toNormF32);

          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

#endif

#ifdef DOSETFOGH
          dv.SetFogH();
#endif

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];
      Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | bucky.flags0);
      Vid::SetBucketTexture( bucky.texture0, FALSE, 0, bucky.flags0);

      if (Vid::isStatus.multitext && bucky.overlay && (_controlFlags & controlOVERLAY))
      {
        Vid::SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
      }

      if (!cam.ClipToBucket( bucky, &stateArray))
//      if (!cam.ClipToBucket( tbucket.verts, bucky.vCount, tbucket.indices, tbucket.indexCount))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorLightAnimI( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
//  ASSERT( _buckys.count < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts);

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (color.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[i][0], 0xfe, sizeof(U16) * normCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];

    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = Utils::FtoL(bucky.diff.a * F32(color.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = tempBuckets[i].verts;
    bucky.index = tempBuckets[i].indices;
  }

  if (clipFlags == clipNONE)
  {
    // setup and lock _buckys
    if (!Vid::LockBuckets( &_buckys[0], buckyCount, _controlFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );

      BucketMan::forceTranslucent = FALSE;
      return;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
        Plane plane;
//        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//        if (plane.Dot( verts[ivi[0]]) >= 0.0f)
        plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        {
          continue;
        }
      }
#ifdef DOSTATISTICS
      Statistics::tempTris++;
#endif

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *in = inA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            bucky.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = bucky.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            iv[ivj] = (U16) bucky.vCount;
            in[inj] = (U16) bucky.vCount;
            iu[iuj] = (U16) bucky.vCount;
            bucky.vCount++;
          }
        }
        else
        {
          // new vertex; light and project
          Vector &sv = verts[ivj];
          dv.uv = uvs[iuj];

#if 1

          bucky.Light( dv, sv, normals[inj], color); 

          cam.ProjectFromCameraSpace( dv, sv);

#else

          diff = bucky.diffInit;

          tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
          DxLight::Manager::LightCameraSpace( sv, norm, bucky.diff, *bucky.material, diff, spec);

          cam.ProjectFromCameraSpace( dv, sv);

          // set the colors
          dv.diffuse.SetNoExpand(
            color.r * diff.r, 
            color.g * diff.g, 
            color.b * diff.b,      
            bucky.diffInitC);

          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

#endif

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }
    Vid::UnLockBuckets( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
        Plane plane;
//        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//        if (plane.Dot( verts[ivi[0]]) >= 0.0f)
        plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        {
          continue;
        }
      }

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *in = inA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            bucky.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = bucky.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            iv[ivj] = (U16) bucky.vCount;
            in[inj] = (U16) bucky.vCount;
            iu[iuj] = (U16) bucky.vCount;
            bucky.vCount++;
          }
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          Vector &sv = verts[ivj];
          dv.uv = uvs[iuj];

#if 1

          bucky.Light( dv, sv, normals[inj], color); 

          cam.SetHomogeneousFromCameraSpace( dv, sv);

#else

          diff = bucky.diffInit;

          tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
          DxLight::Manager::LightCameraSpace( sv, norm, bucky.diff, *bucky.material, diff, spec);

          cam.SetHomogeneousFromCameraSpace( dv, sv);

          // set the colors
          dv.diffuse.SetNoExpand(
            color.r * diff.r, 
            color.g * diff.g, 
            color.b * diff.b,      
            bucky.diffInitC);

          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

#endif

#ifdef DOSETFOGH
          dv.SetFogH();
#endif

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];
      Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | bucky.flags0);
      Vid::SetBucketTexture( bucky.texture0, FALSE, 0, bucky.flags0);

      if (Vid::isStatus.multitext && bucky.overlay && (_controlFlags & controlOVERLAY))
      {
        Vid::SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
      }

      if (!cam.ClipToBucket( bucky, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorLightNoAnimI( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  stateArray;

  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (color.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[i][0], 0xfe, sizeof(U16) * normCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];

    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = Utils::FtoL(bucky.diff.a * F32(color.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = tempBuckets[i].verts;
    bucky.index = tempBuckets[i].indices;
  }

  if (clipFlags == clipNONE)
  {
    // setup and lock _buckys
    if (!Vid::LockBuckets( &_buckys[0], buckyCount, _controlFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );

      BucketMan::forceTranslucent = FALSE;
      return;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

#ifdef DOSTATISTICS
      Statistics::tempTris++;
#endif

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *in = inA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            bucky.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = bucky.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            iv[ivj] = (U16) bucky.vCount;
            in[inj] = (U16) bucky.vCount;
            iu[iuj] = (U16) bucky.vCount;
            bucky.vCount++;
          }
        }
        else
        {
          // new vertex; light and project
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

#if 1

          bucky.Light( dv, sv, normals[inj], color); 

          cam.TransformProjectFromModelSpace( dv, sv);

#else

        Vector norm;
        ColorValue diff, spec = { 0.0f, 0.0f, 0.0f, 1.0f };

          diff = bucky.diffInit;

          DxLight::Manager::LightModelSpace( sv, normals[inj], bucky.diff, *bucky.material, diff, spec);

          cam.TransformProjectFromModelSpace( dv, sv);

          // set the colors
          dv.diffuse.SetNoExpand(
            color.r * diff.r, 
            color.g * diff.g, 
            color.b * diff.b,      
            bucky.diffInitC);

          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

#endif

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }
    Vid::UnLockBuckets( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *in = inA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            bucky.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = bucky.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            iv[ivj] = (U16) bucky.vCount;
            in[inj] = (U16) bucky.vCount;
            iu[iuj] = (U16) bucky.vCount;
            bucky.vCount++;
          }
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

#if 1

          bucky.Light( dv, sv, normals[inj], color); 

          cam.SetHomogeneousFromModelSpace( dv, sv);

#else
        Vector norm;
        ColorValue diff, spec = { 0.0f, 0.0f, 0.0f, 1.0f };

          diff = bucky.diffInit;

          DxLight::Manager::LightModelSpace( sv, normals[inj], bucky.diff, *bucky.material, diff, spec);

          cam.SetHomogeneousFromModelSpace( dv, sv);

          // set the colors
          dv.diffuse.SetNoExpand(
            color.r * diff.r, 
            color.g * diff.g, 
            color.b * diff.b,      
            bucky.diffInitC);

          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

#endif

#ifdef DOSETFOGH
          dv.SetFogH();
#endif

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];
      Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | bucky.flags0);
      Vid::SetBucketTexture( bucky.texture0, FALSE, 0, bucky.flags0);

      if (Vid::isStatus.multitext && bucky.overlay && (_controlFlags & controlOVERLAY))
      {
        Vid::SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
      }

      if (!cam.ClipToBucket( bucky, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorAnimI( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  ASSERT( buckyCount < MAXBUCKYS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts);

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (color.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];

    bucky.diffInitC.SetNoExpand( 
      (bucky.diff.r * Vid::renderState.ambientR + bucky.diff.r) * color.r, 
      (bucky.diff.g * Vid::renderState.ambientG + bucky.diff.g) * color.g, 
      (bucky.diff.b * Vid::renderState.ambientB + bucky.diff.b) * color.b, 
       bucky.diff.a * F32(color.a)
    );

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = tempBuckets[i].verts;
    bucky.index = tempBuckets[i].indices;
  }

  if (clipFlags == clipNONE)
  {
    // setup and lock _buckys
    if (!Vid::LockBuckets( &_buckys[0], buckyCount, _controlFlags, &stateArray))
    {
      BucketMan::forceTranslucent = FALSE;
      return;
    }

    // backcull, transform and light
    Bool doCull = !(renderFlags & RS_2SIDED);
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (doCull)
      {
        Plane plane;
//        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//        if (plane.Dot( verts[ivi[0]]) >= 0.0f)
        plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        {
          continue;
        }
      }
#ifdef DOSTATISTICS
      Statistics::tempTris++;
#endif

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 iui = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == iu[iui])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and project
          dv.vv = verts[ivj];
          dv.uv = uvs[iui];

          cam.ProjectFromCameraSpace( dv);

          // set the colors
          dv.diffuse  = bucky.diffInitC;
          dv.specular = 0xff000000;

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iui] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }
    Vid::UnLockBuckets( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
    Bool doCull = !(renderFlags & RS_2SIDED);
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (doCull)
      {
        Plane plane;
//        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//        if (plane.Dot( verts[ivi[0]]) >= 0.0f)
        plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        {
          continue;
        }
      }

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 iui = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == iu[iui])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          dv.vv = verts[ivj];
          dv.uv = uvs[iui];

            // set the colors
          dv.diffuse = bucky.diffInitC;
          dv.specular = 0xff000000;

          cam.SetHomogeneousFromCameraSpace( dv);

#ifdef DOSETFOGH
          dv.SetFogH();
#endif

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iui] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];
      Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | bucky.flags0);
      Vid::SetBucketTexture( bucky.texture0, FALSE, 0, bucky.flags0);

      if (Vid::isStatus.multitext && bucky.overlay && (_controlFlags & controlOVERLAY))
      {
        Vid::SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
      }

      if (!cam.ClipToBucket( bucky, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorNoAnimI( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  stateArray;

  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (color.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];

    bucky.diffInitC.SetNoExpand( 
      (bucky.diff.r * Vid::renderState.ambientR + bucky.diff.r) * color.r, 
      (bucky.diff.g * Vid::renderState.ambientG + bucky.diff.g) * color.g, 
      (bucky.diff.b * Vid::renderState.ambientB + bucky.diff.b) * color.b, 
       bucky.diff.a * F32(color.a)
    );

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = tempBuckets[i].verts;
    bucky.index = tempBuckets[i].indices;
  }

  if (clipFlags == clipNONE)
  {
    // setup and lock _buckys
    if (!Vid::LockBuckets( &_buckys[0], buckyCount, _controlFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );

      BucketMan::forceTranslucent = FALSE;
      return;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

#ifdef DOSTATISTICS
      Statistics::tempTris++;
#endif

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == iu[iuj])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and project
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

          cam.TransformProjectFromModelSpace( dv, sv);

          // set the colors
          dv.diffuse  = bucky.diffInitC;
          dv.specular = 0xff000000;

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }
    Vid::UnLockBuckets( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

      U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == iu[iuj])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

          cam.SetHomogeneousFromModelSpace( dv, sv);

            // set the colors
          dv.diffuse = bucky.diffInitC;
          dv.specular = 0xff000000;

#ifdef DOSETFOGH
          dv.SetFogH();
#endif

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];
      Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | bucky.flags0);
      Vid::SetBucketTexture( bucky.texture0, FALSE, 0, bucky.flags0);

      if (Vid::isStatus.multitext && bucky.overlay && (_controlFlags & controlOVERLAY))
      {
        Vid::SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
      }

      if (!cam.ClipToBucket( bucky, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorAnimColorsI( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  ASSERT( buckyCount < MAXBUCKYS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts);

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (color.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  ColorValue col, amb;
  col.r = color.r * U8toNormF32;
  col.g = color.g * U8toNormF32;
  col.b = color.b * U8toNormF32;
  col.a = color.a * U8toNormF32;
  amb.r = Vid::renderState.ambientR * col.r;
  amb.g = Vid::renderState.ambientG * col.g;
  amb.b = Vid::renderState.ambientB * col.b;

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * amb.r + bucky.diff.r * col.r;
    bucky.diffInit.g = bucky.diff.g * amb.g + bucky.diff.g * col.g;
    bucky.diffInit.b = bucky.diff.b * amb.b + bucky.diff.b * col.b;
    bucky.diffInit.a = bucky.diff.a * col.a;

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = tempBuckets[i].verts;
    bucky.index = tempBuckets[i].indices;
  }

  if (clipFlags == clipNONE)
  {
    // setup and lock _buckys
    if (!Vid::LockBuckets( &_buckys[0], buckyCount, _controlFlags, &stateArray))
    {
      BucketMan::forceTranslucent = FALSE;
      return;
    }

    // backcull, transform and light
    Bool doCull = !(renderFlags & RS_2SIDED);
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (doCull)
      {
        Plane plane;
//        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//        if (plane.Dot( verts[ivi[0]]) >= 0.0f)
        plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        {
          continue;
        }
      }
#ifdef DOSTATISTICS
      Statistics::tempTris++;
#endif

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 iui = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == iu[iui])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and project
          dv.vv = verts[ivj];
          dv.uv = uvs[iui];

          cam.ProjectFromCameraSpace( dv);

          // set the colors
          dv.diffuse.Modulate( colors[ivj], bucky.diffInit.r, bucky.diffInit.g, bucky.diffInit.b, bucky.diffInit.a);
          dv.specular = 0xff000000;

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iui] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }
    Vid::UnLockBuckets( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
    Bool doCull = !(renderFlags & RS_2SIDED);
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (doCull)
      {
        Plane plane;
//        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//        if (plane.Dot( verts[ivi[0]]) >= 0.0f)
        plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        {
          continue;
        }
      }

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 iui = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == iu[iui])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          dv.vv = verts[ivj];
          dv.uv = uvs[iui];

            // set the colors
          dv.diffuse.Modulate( colors[ivj], bucky.diffInit.r, bucky.diffInit.g, bucky.diffInit.b, bucky.diffInit.a);
          dv.specular = 0xff000000;

          cam.SetHomogeneousFromCameraSpace( dv);

#ifdef DOSETFOGH
          dv.SetFogH();
#endif

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iui] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];
      Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | bucky.flags0);
      Vid::SetBucketTexture( bucky.texture0, FALSE, 0, bucky.flags0);

      if (Vid::isStatus.multitext && bucky.overlay && (_controlFlags & controlOVERLAY))
      {
        Vid::SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
      }

      if (!cam.ClipToBucket( bucky, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorNoAnimColorsI( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  stateArray;

  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (color.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  ColorValue col, amb;
  col.r = color.r * U8toNormF32;
  col.g = color.g * U8toNormF32;
  col.b = color.b * U8toNormF32;
  col.a = color.a * U8toNormF32;
  amb.r = Vid::renderState.ambientR * col.r;
  amb.g = Vid::renderState.ambientG * col.g;
  amb.b = Vid::renderState.ambientB * col.b;

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * amb.r + bucky.diff.r * col.r;
    bucky.diffInit.g = bucky.diff.g * amb.g + bucky.diff.g * col.g;
    bucky.diffInit.b = bucky.diff.b * amb.b + bucky.diff.b * col.b;
    bucky.diffInit.a = bucky.diff.a * col.a;

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = tempBuckets[i].verts;
    bucky.index = tempBuckets[i].indices;
  }

  if (clipFlags == clipNONE)
  {
    // setup and lock _buckys
    if (!Vid::LockBuckets( &_buckys[0], buckyCount, _controlFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );

      BucketMan::forceTranslucent = FALSE;
      return;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

#ifdef DOSTATISTICS
      Statistics::tempTris++;
#endif

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == iu[iuj])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and project
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

          cam.TransformProjectFromModelSpace( dv, sv);

          // set the colors
          dv.diffuse.Modulate( colors[ivj], bucky.diffInit.r, bucky.diffInit.g, bucky.diffInit.b, bucky.diffInit.a);
          dv.specular = 0xff000000;

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }
    Vid::UnLockBuckets( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj & face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      BucketLock & bucky = _buckys[face.buckyIndex];

      U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
#ifdef DOCALCPLANES
        Plane plane;
        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
#else
        Plane &plane = planes[i];
#endif
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

      // indexers for this bucky
      U16 *iv = ivA[face.buckyIndex];
      U16 *iu = iuA[face.buckyIndex];

      U32 j;
      for (j = 0; j < 3; j++)
      {
	      U16 iuj = face.uvs[j];

        VertexTL &dv = bucky.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == iu[iuj])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          Vector &sv = vertices[ivj];
          dv.uv = uvs[iuj];

          cam.SetHomogeneousFromModelSpace( dv, sv);

          // set the colors
          dv.diffuse.Modulate( colors[ivj], bucky.diffInit.r, bucky.diffInit.g, bucky.diffInit.b, bucky.diffInit.a);
          dv.specular = 0xff000000;

#ifdef DOSETFOGH
          dv.SetFogH();
#endif

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
          bucky.vCount++;
        }
      }
    }

    for (i = 0; i < buckyCount; i++)
    {
      BucketLock & bucky = _buckys[i];
      Vid::SetBucketFlags( (BucketMan::GetPrimitiveDesc().flags & ~RS_BUCKY_MASK) | bucky.flags0);
      Vid::SetBucketTexture( bucky.texture0, FALSE, 0, bucky.flags0);

      if (Vid::isStatus.multitext && bucky.overlay && (_controlFlags & controlOVERLAY))
      {
        Vid::SetBucketTexture( bucky.texture1, FALSE, 1, bucky.flags1);
      }

      if (!cam.ClipToBucket( bucky, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightAnimI_2( BucketLock *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[i][0], 0xfe, sizeof(U16) * normCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each material (bucky)
    BucketLock & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;
    bucky.fCount = 0;

    bucky.usedVCount = 0;
    bucky.usedICount = 0;
  }

  // backcull
  FaceObj * f, * fe = faces.data + faceCount;
  for (f = faces.data; f < fe; f++)
  {
    FaceObj & face = *f;
    ASSERT( face.buckyIndex < buckyCount);
    BucketLock & bucky = _buckys[face.buckyIndex];

	  U16 ivi[3];
    ivi[0] = face.verts[0];
    ivi[1] = face.verts[1];
    ivi[2] = face.verts[2];
    ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

    if (!(bucky.flags0 & RS_2SIDED))
    {
      Plane plane;
      plane.Set( verts[ivi[0]], verts[ivi[2]], verts[ivi[1]]);
      if (plane.Dot( verts[ivi[0]]) <= 0.0f)
      {
        continue;
      }
    }
    // indexers for this bucky
    U16 *iv = ivA[face.buckyIndex];
    U16 *in = inA[face.buckyIndex];
    U16 *iu = iuA[face.buckyIndex];
    U16 *vv = vvA[face.buckyIndex] + bucky.usedICount;

    fA[face.buckyIndex][bucky.fCount] = f;
    bucky.fCount++;

    face.state = 0;

    U32 j;
    for (j = 0; j < 3; j++, vv++)
    {
      U16 ivj = ivi[j];
	    U16 inj = face.norms[j];
	    U16 iuj = face.uvs[j];

      if (iv[ivj] != in[inj])
      {
        // new vert
        //
        *vv = (U16) bucky.usedVCount;
        iv[ivj] = (U16) bucky.usedVCount;
        in[inj] = (U16) bucky.usedVCount;
        iu[iuj] = (U16) bucky.usedVCount;
        bucky.usedVCount++;
      }
      else if (iv[ivj] != iu[iuj])
      {
        // old vert with new uv
        //
        *vv = iv[ivj];
//        iv[ivj] = (U16) bucky.usedVCount;
        in[inj] = (U16) bucky.usedVCount;
        iu[iuj] = (U16) bucky.usedVCount;
        bucky.usedVCount++;

        face.state |= (1 << (j*2));
      }
      else
      {
        // same old vert 
        //
        *vv = iv[ivj];

        face.state |= (2 << (j*2));
      }
    }
    bucky.usedICount += 3;
  }

  BucketLock * b, * be = _buckys + buckyCount;

  if (clipFlags == clipNONE)
  {
    U16 * vvv = vvA[0];
    FaceObj ** fff = fA[0];
    for (b = _buckys; b < be; b++, vvv += MAXINDICES, fff += MAXTRIS)
    {
      BucketLock & bucky = *b;

      if (!bucky.fCount)
      {
        continue;
      }
      Vid::LockBucket( bucky, _controlFlags, bucky.usedVCount, bucky.fCount * 3, &stateArray);

#ifdef DOSTATISTICS
      Statistics::tempTris += bucky.fCount;
#endif

      FaceObj ** f, ** fe = fff + bucky.fCount;
      U16 * vv = vvv;
      for (f = fff; f < fe; f++)
      {
        FaceObj & face = **f;

        U32 j;
        for (j = 0; j < 3; j++, vv++)
        {
          U16 ivj = face.verts[j];
	        U16 inj = face.norms[j];
	        U16 iuj = face.uvs[j];

          VertexTL & dv = bucky.CurrVertexTL();

          U32 state = U32((face.state >> (j*2)) & 3);
          switch (state)
          {
          case 2:
            // identical vertex; just set index
            bucky.SetIndex( *vv);
            break;
          case 1:
            // already lit; copy vert
            dv = bucky.GetVertexTL( *vv);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            bucky.vCount++;
            break;
          case 0:
            // new vertex; light and project
            Vector &sv = verts[ivj];
            dv.uv = uvs[iuj];

            Vector norm;
            tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
            bucky.Light( dv, sv, norm, baseColor); 

            cam.ProjectFromCameraSpace( dv, sv);

            // set vertex fog
            dv.SetFog();

            bucky.SetIndex( (U16) bucky.vCount);
            bucky.vCount++;
            break;
          }
        }
      }
      Vid::UnLockBucket( bucky);
    }
  }
  else
  {
    U16 * vvv = vvA[0];
    FaceObj ** fff = fA[0];
    for (b = _buckys; b < be; b++, vvv += MAXINDICES, fff += MAXTRIS)
    {
      BucketLock & bucky = *b;

      if (!bucky.fCount)
      {
        continue;
      }

      bucky.vert  = Vid::tempVertices;
      bucky.index = Vid::tempIndices;

      FaceObj ** f, ** fe = fff + bucky.fCount;
      U16 * vv = vvv;
      for (f = fff; f < fe; f++)
      {
        FaceObj & face = **f;

        U32 j;
        for (j = 0; j < 3; j++, vv++)
        {
          U16 ivj = face.verts[j];
	        U16 inj = face.norms[j];
	        U16 iuj = face.uvs[j];

          VertexTL & dv = bucky.CurrVertexTL();

          U32 state = U32((face.state >> (j*2)) & 3);
          switch (state)
          {
          case 2:
            // identical vertex; just set index
            bucky.SetIndex( *vv);
            break;
          case 1:
            // already lit; copy vert
            dv = bucky.GetVertexTL( *vv);
            dv.uv = uvs[iuj];

            bucky.SetIndex( (U16) bucky.vCount);
            bucky.vCount++;
            break;
          case 0:
            // new vertex; light and project
            Vector &sv = verts[ivj];
            dv.uv = uvs[iuj];

            Vector norm;
            tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
            bucky.Light( dv, sv, norm, baseColor); 

            cam.SetHomogeneousFromCameraSpace( dv, sv);

            bucky.SetIndex( (U16) bucky.vCount);
            bucky.vCount++;
            break;
          }
        }
      }
      Vid::SetBucket( bucky, _controlFlags);

      if (!cam.ClipToBucket( bucky, &stateArray))
//      if (!cam.ClipToBucket( bucky.vert, bucky.vCount, bucky.index, bucky.iCount))
      {
        LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      }
    }
  }
  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

#endif