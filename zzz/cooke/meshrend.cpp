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

void MeshRoot::SetMatricesView( const Array<FamilyState> & stateArray, Matrix *tranys) const
{
  U32 i;
  for (i = 0; i < stateArray.count; i++)
  {
#if 0
    Matrix temp;
    stateMats[i].Transform( temp, stateArray[i].WorldMatrix());
    temp.Transform( tranys[i], Vid::view_matrix);
#else
    tranys[i] = stateMats[i] * stateArray[i].WorldMatrix() * Vid::view_matrix;
#endif
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

void MeshRoot::SetMatricesModel( const Array<FamilyState> & stateArray, Matrix *tranys) const
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

void MeshRoot::SetVertsView( const Array<FamilyState> & stateArray, Matrix *tranys, Vector *verts, Bool doMultiWeight) const
{
  SetMatricesView( stateArray, tranys);

  // transform verts to view space
  U32 i, hit = FALSE;
  for (i = 0; i < vertCount; i++)
  {
    if (vertToState[i].index[0] != 0)
    {
      hit = TRUE;
    }
    SetVert( tranys, i, verts[i], doMultiWeight);
  }
#if 0
  if (!hit)
  {
    LOG_DIAG( ("all vertToState == 0") );
  }
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::SetVertsWorld( const Array<FamilyState> & stateArray, Matrix *tranys, Vector *verts, Bool doMultiWeight) const
{
  SetMatricesWorld( stateArray, tranys);

  // transform verts to world space
  U32 i;
  for (i = 0; i < vertCount; i++)
  {
    SetVert( tranys, i, verts[i], doMultiWeight);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetVertsWorld( const Array<FamilyState> & stateArray, Vector *verts, Bool doMultiWeight) const
{
  Matrix tranys[MAXMESHPERGROUP];

  SetMatricesWorld( stateArray, tranys);

  // transform verts to world space
  U32 i;
  for (i = 0; i < vertCount; i++)
  {
    SetVert( tranys, i, verts[i], doMultiWeight);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::SetVertsModel( const Array<FamilyState> & stateArray, Matrix *tranys, Vector *verts, Bool doMultiWeight) const
{
  SetMatricesModel( stateArray, tranys);

  // transform verts from model space, tranform for anim, back to model space
  U32 i;
  for (i = 0; i < vertCount; i++)
  {
    SetVert( tranys, i, verts[i], doMultiWeight);
  }
}
//----------------------------------------------------------------------------

// indexer lists for verts (one for each bucky)
static U16 ivA[MAXBUCKYS][MAXVERTS];
static U16 inA[MAXBUCKYS][MAXVERTS];
static U16 iuA[MAXBUCKYS][MAXVERTS];

static U16 ifA[MAXBUCKYS][MAXVERTS];
static U8  tvA[MAXBUCKYS][MAXINDICES];
static U16 vvA[MAXBUCKYS][MAXINDICES];

void MeshRoot::RenderLightAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
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
    FaceGroup & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR * F32(baseColor.r) * U32toNormF3;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG * F32(baseColor.b) * U32toNormF3;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB * F32(baseColor.g) * U32toNormF3;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup bucky memory
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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

          Vector norm;
          tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
          bucky.Light( dv, sv, norm, baseColor); 

          cam.ProjectFromCameraSpace( dv, sv);

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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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

          Vector norm;
          tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
          bucky.Light( dv, sv, norm, baseColor); 

          cam.SetHomogeneousFromCameraSpace( dv, sv);

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
      FaceGroup & bucky = _buckys[i];

      Vid::SetBucket( bucky, _controlFlags);

      if (!cam.ClipToBucket( bucky, (void *) &stateArray))
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

void MeshRoot::RenderLightAnimColorsI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  ASSERT( buckyCount < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
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
    FaceGroup & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR * F32(baseColor.r) * U32toNormF3;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG * F32(baseColor.b) * U32toNormF3;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB * F32(baseColor.g) * U32toNormF3;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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

          Color c;
          c.Modulate( colors[ivj], base.r, base.g, base.b);

          bucky.Light( dv, sv, normals[inj], c); 

          cam.ProjectFromCameraSpace( dv, sv);

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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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

          Color c;
          c.Modulate( colors[ivj], base.r, base.g, base.b);

          bucky.Light( dv, sv, normals[inj], c); 

          cam.SetHomogeneousFromCameraSpace( dv, sv);

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
      FaceGroup & bucky = _buckys[i];

      Vid::SetBucket( bucky, _controlFlags);

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

#define DOCALCPLANES

void MeshRoot::RenderLightNoAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
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
    FaceGroup & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR * F32(baseColor.r) * U32toNormF3;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG * F32(baseColor.b) * U32toNormF3;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB * F32(baseColor.g) * U32toNormF3;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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

          bucky.Light( dv, sv, normals[inj], baseColor); 

          cam.TransformProjectFromModelSpace( dv, sv);

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

      FaceGroup & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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

          bucky.Light( dv, sv, normals[inj], baseColor); 

          cam.SetHomogeneousFromModelSpace( dv, sv);

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
      FaceGroup & bucky = _buckys[i];

      Vid::SetBucket( bucky, _controlFlags);

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

void MeshRoot::RenderLightNoAnimColorsI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
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
    FaceGroup & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR * F32(baseColor.r) * U32toNormF3;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG * F32(baseColor.b) * U32toNormF3;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB * F32(baseColor.g) * U32toNormF3;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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

          Color c;
          c.Modulate( colors[ivj], base.r, base.g, base.b);
          
          bucky.Light( dv, sv, normals[inj], c); 

          cam.TransformProjectFromModelSpace( dv, sv);

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

      FaceGroup & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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

          Color c;
          c.Modulate( colors[ivj], base.r, base.g, base.b);

          bucky.Light( dv, sv, normals[inj], c); 

          cam.SetHomogeneousFromModelSpace( dv, sv);

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
      FaceGroup & bucky = _buckys[i];

      Vid::SetBucket( bucky, _controlFlags);

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

void MeshRoot::RenderColorAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  ASSERT( buckyCount < MAXBUCKYS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

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
    FaceGroup & bucky = _buckys[i];

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
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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
      FaceGroup & bucky = _buckys[i];

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

void MeshRoot::RenderColorNoAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
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
    FaceGroup & bucky = _buckys[i];

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
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

      U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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
      FaceGroup & bucky = _buckys[i];

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

void MeshRoot::RenderColorAnimColorsI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  ASSERT( buckyCount < MAXBUCKYS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

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
    FaceGroup & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * amb.r + bucky.diff.r * col.r;
    bucky.diffInit.g = bucky.diff.g * amb.g + bucky.diff.g * col.g;
    bucky.diffInit.b = bucky.diff.b * amb.b + bucky.diff.b * col.b;
    bucky.diffInit.a = bucky.diff.a * col.a;

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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
      FaceGroup & bucky = _buckys[i];

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

void MeshRoot::RenderColorNoAnimColorsI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
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
    FaceGroup & bucky = _buckys[i];
    bucky.diffInit.r = bucky.diff.r * amb.r + bucky.diff.r * col.r;
    bucky.diffInit.g = bucky.diff.g * amb.g + bucky.diff.g * col.g;
    bucky.diffInit.b = bucky.diff.b * amb.b + bucky.diff.b * col.b;
    bucky.diffInit.a = bucky.diff.a * col.a;

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

      U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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
      FaceGroup & bucky = _buckys[i];

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

  ASSERT( sp->localVertices.count < MAXVERTS && sp->localIndices.count < MAXINDICES);

  Camera & cam = Vid::CurCamera();

  VertexTL *vertmem = Vid::tempVertices;
  U16 *indexmem = Vid::tempIndices;

  U32 i;
  for (i = 0; i < sp->localVertices.count; i++)
  {
    cam.SetHomogeneousFromModelSpace( vertmem[i], sp->localVertices[i]);

    vertmem[i].diffuse  = color;
    vertmem[i].specular = 0xff000000;
    vertmem[i].uv.u = 0.0f;
    vertmem[i].uv.v = 0.0f;
  }
  Utils::Memcpy( indexmem, &sp->localIndices[0], 2 * sp->localIndices.count);

  cam.ClipToBucket( vertmem, i, indexmem, sp->localIndices.count, (void *) sp);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderNormals( const Array<FamilyState> & stateArray, Color color, U32 clipFlags) // = clipALL)
{
  clipFlags;

  Bool doMultiWeight = (rootControlFlags & controlMULTIWEIGHT) && Mesh::Manager::doMultiWeight ? TRUE : FALSE;

  F32 normLenFactor = ObjectBounds().Radius() * 0.04f;

  Camera &cam = Vid::CurCamera();

//  Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_GLOW);
  Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_BLEND_GLOW);

  // set the polygon material
  // force translucency
  Vid::SetBucketMaterial( Vid::defMaterial);
  Vid::SetBucketTexture( NULL, TRUE);

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts, doMultiWeight);

  // clear indexer lists
  memset( &ivA[0][0], 0xff, sizeof(U16) * vertCount);
  memset( &inA[0][0], 0xfe, sizeof(U16) * normCount);

#if 1
  U32 i;
	for (i = 0; i < faceCount; i++)
  {
    FaceObj & face = faces[i];

    U32 j;
    for (j = 0; j < 3; j++)
    {
      U16 ivj = face.verts[j];
      U16 inj = face.norms[j];

      if (ivA[0][ivj] != 0xffff && inA[0][inj] != 0xfefe)
      {
        continue;
      }
      ivA[0][ivj] = 0x00;
      inA[0][inj] = 0x00;

      Vector norm;
      tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
      norm *= normLenFactor;

      Vector &vert = verts[ivj];

  	  // set bucket depth
      Vid::SetTranBucketZ(vert.z);
    
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

      cam.ProjectFromCameraSpace( vertmem[0]);

	    vertmem[1].vv.x = vert.x + norm.x;
	    vertmem[1].vv.y = vert.y + norm.y;
	    vertmem[1].vv.z = vert.z + norm.z;
	    vertmem[1].diffuse  = color;
	    vertmem[1].specular = 0xff000000;

      cam.ProjectFromCameraSpace( vertmem[1]);

	    vertmem[2] = vertmem[0];
      vertmem[0].vv.x -= 1.0f;
      vertmem[2].vv.x += 1.0f;

      Utils::Memcpy( indexmem, Vid::rectIndices, 6);

      // submit the polygons
      Vid::UnlockIndexedPrimitiveMem( 3, 3);
    }
  }
#else
  U32 i;
	for (i = 0; i < faceCount; i++)
  {
    FaceObj & face = faces[i];

    U32 j;
    for (j = 0; j < 3; j++)
    {
      U16 ivj = face.verts[j];
      U16 inj = face.norms[j];

      if (ivA[0][ivj] != 0xffff && inA[0][inj] != 0xfefe)
      {
        continue;
      }
      ivA[0][ivj] = 0x00;
      inA[0][inj] = 0x00;

      static Color color = RGBA_MAKE( 255, 255, 0, 255);

      Vector norm;
      tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
      norm *= normLenFactor;

      Vector normX;
      normX.x =  norm.y;
      normX.y = -norm.x;
      normX.z =  norm.z;
      normX  *=  NORMDRAWWID;

      Vector normZ;
      normZ.x =  norm.x;
      normZ.y = -norm.z;
      normZ.z =  norm.y;
      normZ  *=  NORMDRAWWID;

      Vector &vert = verts[ivj];
      VertexTL vertmem[5];

	    vertmem[0].vv.x = vert.x + norm.x;
	    vertmem[0].vv.y = vert.y + norm.y;
	    vertmem[0].vv.z = vert.z + norm.z;
	    vertmem[0].diffuse  = color;
	    vertmem[0].specular = RGBA_MAKE( 0, 0, 0, 255);

      cam.SetHomogeneousFromCameraSpace( vertmem[0]);

//	    vertmem[1].vv.x = vert.x + NORMDRAWWID;
      vertmem[1].vv.x = vert.x;
	    vertmem[1].vv.y = vert.y;
	    vertmem[1].vv.z = vert.z;
	    vertmem[1].diffuse  = color;
	    vertmem[1].specular = RGBA_MAKE( 0, 0, 0, 255);
      vertmem[1].vv += normX;

      cam.SetHomogeneousFromCameraSpace( vertmem[1]);

//	    vertmem[2].vv.x = vert.x - NORMDRAWWID;
      vertmem[2].vv.x = vert.x;
	    vertmem[2].vv.y = vert.y;
	    vertmem[2].vv.z = vert.z;
	    vertmem[2].diffuse  = color;
	    vertmem[2].specular = RGBA_MAKE( 0, 0, 0, 255);
      vertmem[2].vv -= normX;

      cam.SetHomogeneousFromCameraSpace( vertmem[2]);

	    vertmem[3].vv.x = vert.x;
	    vertmem[3].vv.y = vert.y;
//	    vertmem[3].vv.z = vert.z + NORMDRAWWID;
      vertmem[3].vv.z = vert.z;
	    vertmem[3].diffuse  = color;
	    vertmem[3].specular = RGBA_MAKE( 0, 0, 0, 255);
      vertmem[3].vv += normZ;

      cam.SetHomogeneousFromCameraSpace( vertmem[3]);

	    vertmem[4].vv.x = vert.x;
	    vertmem[4].vv.y = vert.y;
//	    vertmem[4].vv.z = vert.z - NORMDRAWWID;
      vertmem[4].vv.z = vert.z;
	    vertmem[4].diffuse  = color;
	    vertmem[4].specular = RGBA_MAKE( 0, 0, 0, 255);
      vertmem[4].vv -= normZ;

      cam.SetHomogeneousFromCameraSpace( vertmem[4]);

      static U16 normalIndexMem[] = { 0, 1, 2, 0, 3, 4 };

      Vid::CurCamera().ClipToBucket( vertmem, 5, normalIndexMem, 6);
    }
  }
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::SetupRenderProc()
{
#if 0
  if (animCycles.GetCount())
  {
    renderProc      = colors.count ? MeshRoot::RenderLightAnimColorsI : MeshRoot::RenderLightAnimI;
    renderColorProc = colors.count ? MeshRoot::RenderColorAnimColorsI : MeshRoot::RenderColorAnimI;
  }
  else
#endif
  {
    renderProc      = colors.count ? MeshRoot::RenderLightNoAnimColorsI : MeshRoot::RenderLightNoAnimI;
    renderColorProc = colors.count ? MeshRoot::RenderColorNoAnimColorsI : MeshRoot::RenderColorNoAnimI;
  }

  BucketLock * b = & buckys[0];
  BucketLock * e = b + buckys.count;

  for ( ; b < e; b++)
  {
    if (b->flags0 == RS_BLEND_DECAL)
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

void MeshRoot::Render( const Matrix &world, Color baseColor, U32 _controlFlags) // 0xffffffff, = controlDEF
{
	SetWorldAll( world);

  Vector origin;
  world.Transform( origin, ObjectBounds().Offset());
  U32 clipFlags = Vid::CurCamera().BoundsTest( origin, ObjectBounds());
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup controlFlags
  //
  if (Mesh::Manager::showOverlay && baseColor.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlOVERLAY);
  }
  else
  {
    _controlFlags &= ~controlOVERLAY;
  }
  if (Mesh::Manager::showEnvMap && baseColor.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Mesh::Manager::doMultiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  DxLight::Manager::SetActiveList( origin, ObjectBounds());

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    DxLight::Manager::SetupLightsCameraSpace();

    RenderLightAnim( states, baseColor, clipFlags, _controlFlags);
  }
  else
  {
    DxLight::Manager::SetupLightsModelSpace();

    RenderLightNoAnim( states, baseColor, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColor( const Matrix &world, Color color, U32 _controlFlags) // = controlDEF)
{
	SetWorldAll( world);

  Vector origin;
  world.Transform( origin, ObjectBounds().Offset());
  U32 clipFlags = Vid::CurCamera().BoundsTest( origin, ObjectBounds());
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup controlFlags
  //
  if (Mesh::Manager::showOverlay && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlOVERLAY);
  }
  else
  {
    _controlFlags &= ~controlOVERLAY;
  }
  if (Mesh::Manager::showEnvMap && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Mesh::Manager::doMultiWeight)
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
    RenderColorAnim( states, color, clipFlags, _controlFlags);
  }
  else
  {
    RenderColorNoAnim( states, color, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorLight( const Matrix &world, Color color, U32 _controlFlags) // = controlDEF)
{
	SetWorldAll( world);

  Vector origin;
  world.Transform( origin, ObjectBounds().Offset());
  U32 clipFlags = Vid::CurCamera().BoundsTest( origin, ObjectBounds());
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup controlFlags
  //
  if (Mesh::Manager::showOverlay && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlOVERLAY);
  }
  else
  {
    _controlFlags &= ~controlOVERLAY;
  }
  if (Mesh::Manager::showEnvMap && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Mesh::Manager::doMultiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  DxLight::Manager::SetActiveList( origin, ObjectBounds());

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    DxLight::Manager::SetupLightsCameraSpace();

    RenderColorLightAnim( states, color, clipFlags, _controlFlags);
  }
  else
  {
    DxLight::Manager::SetupLightsModelSpace();

    RenderColorLightNoAnim( states, color, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightSun( const Matrix &world, Color baseColor, U32 _controlFlags) // = controlDEF
{
	SetWorldAll( world);

  Vector origin;
  world.Transform( origin, ObjectBounds().Offset());
  U32 clipFlags = Vid::CurCamera().BoundsTest( origin, ObjectBounds());
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup controlFlags
  //
  if (Mesh::Manager::showOverlay && baseColor.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlOVERLAY);
  }
  else
  {
    _controlFlags &= ~controlOVERLAY;
  }
  if (Mesh::Manager::showEnvMap && baseColor.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Mesh::Manager::doMultiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  DxLight::Manager::SetActiveListSun();

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    DxLight::Manager::SetupLightsCameraSpace();

    RenderLightAnim( states, baseColor, clipFlags, _controlFlags);
  }
  else
  {
    DxLight::Manager::SetupLightsModelSpace();

    RenderLightNoAnim( states, baseColor, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorLightSun( const Matrix &world, Color color, U32 _controlFlags) // = controlDEF)
{
	SetWorldAll( world);

  Vector origin;
  world.Transform( origin, ObjectBounds().Offset());
  U32 clipFlags = Vid::CurCamera().BoundsTest( origin, ObjectBounds());
  if (clipFlags == clipOUTSIDE)
  {
    // cluster is completely outside the view frustrum
    return;
  }

  // setup controlFlags
  //
  if (Mesh::Manager::showOverlay && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlOVERLAY);
  }
  else
  {
    _controlFlags &= ~controlOVERLAY;
  }
  if (Mesh::Manager::showEnvMap && color.a == 255)
  {
    _controlFlags |= (rootControlFlags & controlENVMAP);
  }
  else
  {
    _controlFlags &= ~controlENVMAP;
  }
  if (Mesh::Manager::doMultiWeight)
  {
    _controlFlags |= (rootControlFlags & controlMULTIWEIGHT);
  }
  else
  {
    _controlFlags &= ~controlMULTIWEIGHT;
  }

  DxLight::Manager::SetActiveListSun();

  Vid::SetWorldTransform( states[0].WorldMatrix());

  if (animCycles.GetCount())
  {
    DxLight::Manager::SetupLightsCameraSpace();

    RenderColorLightAnim( states, color, clipFlags, _controlFlags);
  }
  else
  {
    DxLight::Manager::SetupLightsModelSpace();

    RenderColorLightNoAnim( states, color, clipFlags, _controlFlags);
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderSelVerts( Bool showverts, const List<U16> * verts, const Array<FamilyState> & stateArray, Color color0, Color color1, U32 clipFlags) // = clipALL)
{
  clipFlags;

  Bool doMultiWeight = (rootControlFlags & controlMULTIWEIGHT) && Mesh::Manager::doMultiWeight ? TRUE : FALSE;

  // set up transform matrices
  Matrix tranys[MAXMESHPERGROUP];
  SetMatricesView( stateArray, tranys);

  Camera & cam = Vid::CurCamera();

  U8 hits[MAXVERTS];
  Utils::Memset( hits, 0, sizeof( hits));

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

      if (vect.z < cam.NearPlane() || vect.z >= cam.FarPlane())
      {
        // 3D clip
        continue;
      }

      // project
      //
      cam.ProjectFromCameraSpace( vertex, vect);
      vertex.vv.z *= Vid::renderState.zBias * Vid::renderState.zBias;

      Vid::RenderBoxProjected( vertex.vv, vect.z, vertex.rhw, 3, color0);
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

      if (vect.z < cam.NearPlane() || vect.z >= cam.FarPlane())
      {
        // 3D clip
        continue;
      }

      // project
      //
      cam.ProjectFromCameraSpace( vertex, vect);
      vertex.vv.z *= Vid::renderState.zBias * Vid::renderState.zBias;

      Vid::RenderBoxProjected( vertex.vv, vect.z, vertex.rhw, 3, color1);
    }
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderSelFaces( Bool show, const List<U16> * faces, const Array<FamilyState> & stateArray, Color color0, Color color1, U32 clipFlags) // = clipALL)
{
  show;
  clipFlags;
  color0;
  color1;
  faces;

  // setup controlFlags
  //
  U32 _controlFlags;

  if (Mesh::Manager::doMultiWeight)
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
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

}
//----------------------------------------------------------------------------

#if 0
void MeshRoot::RenderColorLightAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
//  ASSERT( _buckys.count < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

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
    FaceGroup & bucky = _buckys[i];

    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = Utils::FtoL(bucky.diff.a * F32(color.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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

          bucky.Light( dv, sv, normals[inj], color); 

          cam.ProjectFromCameraSpace( dv, sv);

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

      FaceGroup & bucky = _buckys[face.buckyIndex];

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
//        if (plane.Dot( verts[ivi[0]]) <= 0.0f)
        if (Vid::BackcullTest( plane.Dot( verts[ivi[0]])))
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

          bucky.Light( dv, sv, normals[inj], color); 

          cam.SetHomogeneousFromCameraSpace( dv, sv);

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
      FaceGroup & bucky = _buckys[i];

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

void MeshRoot::RenderColorLightNoAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
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
    FaceGroup & bucky = _buckys[i];

    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = Utils::FtoL(bucky.diff.a * F32(color.a));

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    // setup memory
    bucky.vert  = Mesh::Manager::tempBuckets[i].verts;
    bucky.index = Mesh::Manager::tempBuckets[i].indices;
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

      FaceGroup & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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

          bucky.Light( dv, sv, normals[inj], color); 

          cam.TransformProjectFromModelSpace( dv, sv);

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

      FaceGroup & bucky = _buckys[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
//        Plane plane;
//        plane.Set( vertices[ivi[0]], vertices[ivi[1]], vertices[ivi[2]]);
        Plane &plane = planes[i];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
//        if (Vid::BackcullTest( plane.Evalue(Vid::model_view_vector)))
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

          bucky.Light( dv, sv, normals[inj], color); 

          cam.SetHomogeneousFromModelSpace( dv, sv);

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
      FaceGroup & bucky = _buckys[i];

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
#endif


#if 0

void MeshRoot::RenderLightNoAnimColorsI( FaceGroup *buckys, U32 buckys.count, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  ASSERT( groups.count == buckys.count);

  ASSERT( buckys.count <= MAXBUCKYS);
  ASSERT( vCount <= MAXVERTS && fCount < MAXTRIS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }
  ColorValue base( baseColor);

  Camera &cam = Vid::CurCamera();

  // setup buckys and fill them
  //
  FaceGroup * b, * be = buckys.data + buckys.count;
  for (b = buckys.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    // calc constant diffuse component for each material (bucky)
    //
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * bucky.vertCount);
    memset( in, 0xfe, sizeof(U16) * bucky.vertCount);
    memset( iu, 0xfd, sizeof(U16) * bucky.vertCount);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

    // backcull, light, project...
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faces.count;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
        Plane &plane = planes[face.index];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

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

          Vector &sv = vertices[ivj];
          dv.uv = _uvs[iuj];

          Color c;
          c.Modulate( colors[ivj], base.r, base.g, base.b);

          bucky.LightModInline( dv, sv, normals[inj], c); 

          if (clipFlags == clipNONE)
          {
            cam.TransformProjectFromModelSpace( dv, sv);

            // set vertex fog
            dv.SetFog();
          }
          else
          {
            cam.SetHomogeneousFromModelSpace( dv, sv);
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
          dv.uv = _uvs[iuj];

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
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
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }

  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightAnimColorsI( FaceGroup *buckys, U32 buckys.count, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // = clipALL, = controlDEF
{
  ASSERT( groups.count == buckys.count);

  ASSERT( buckys.count <= MAXBUCKYS);
  ASSERT( vCount <= MAXVERTS && fCount < MAXTRIS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  //
  Matrix tranys[MAXMESHPERGROUP];
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }
  ColorValue base( baseColor);

  Camera &cam = Vid::CurCamera();

  // setup buckys and fill them
  //
  FaceGroup * b, * be = buckys.data + buckys.count;
  for (b = buckys.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    // calc constant diffuse component for each material (bucky)
    //
    bucky.diffInit.r = bucky.diff.r * Vid::renderState.ambientR;
    bucky.diffInit.g = bucky.diff.g * Vid::renderState.ambientG;
    bucky.diffInit.b = bucky.diff.b * Vid::renderState.ambientB;
    bucky.diffInitC  = (U32) Utils::FtoL(bucky.diff.a * F32(baseColor.a));

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * bucky.vertCount);
    memset( in, 0xfe, sizeof(U16) * bucky.vertCount);
    memset( iu, 0xfd, sizeof(U16) * bucky.vertCount);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

    // backcull, light, project...
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faces.count;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
        Plane plane;
        plane.Set( verts[face.verts[0]], verts[face.verts[2]], verts[face.verts[1]]);
        if (plane.Dot( verts[face.verts[0]]) <= 0.0f)
        {
          continue;
        }
      }

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

          Vector & sv = verts[ivj];
          dv.uv = uvs[iuj];

          Vector norm;
          tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);

          Color c;
          c.Modulate( colors[ivj], base.r, base.g, base.b);
          bucky.LightCamInline( dv, sv, norm, c); 

          if (clipFlags == clipNONE)
          {
            cam.ProjectFromCameraSpace( dv, sv);

            // set vertex fog
            dv.SetFog();
          }
          else
          {
            cam.SetHomogeneousFromCameraSpace( dv, sv);
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

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          in[inj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
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
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }

  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderColorAnimColorsI( FaceGroup *buckys, U32 buckys.count, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  ASSERT( groups.count == buckys.count);

  ASSERT( buckys.count <= MAXBUCKYS);
  ASSERT( vCount <= MAXVERTS && fCount < MAXTRIS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // set up transform matrices and transform verts to view space
  //
  Matrix tranys[MAXMESHPERGROUP];
  SetVertsView( stateArray, tranys, verts, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }
  ColorValue base( baseColor);

  Camera &cam = Vid::CurCamera();

  ColorValue col, amb;
  col.r = baseColor.r * U8toNormF32;
  col.g = baseColor.g * U8toNormF32;
  col.b = baseColor.b * U8toNormF32;
  col.a = baseColor.a * U8toNormF32;
  amb.r = Vid::renderState.ambientR * col.r;
  amb.g = Vid::renderState.ambientG * col.g;
  amb.b = Vid::renderState.ambientB * col.b;

  // setup buckys and fill them
  //
  FaceGroup * b, * be = buckys.data + buckys.count;
  for (b = buckys.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    // calc constant diffuse component for each material (bucky)
    bucky.diffInit.r = bucky.diff.r * amb.r + bucky.diff.r * col.r;
    bucky.diffInit.g = bucky.diff.g * amb.g + bucky.diff.g * col.g;
    bucky.diffInit.b = bucky.diff.b * amb.b + bucky.diff.b * col.b;
    bucky.diffInit.a = bucky.diff.a * col.a;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * bucky.vertCount);
    memset( iu, 0xfd, sizeof(U16) * bucky.vertCount);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

    // backcull, light, project...
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faces.count;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
        Plane plane;
        plane.Set( verts[face.verts[0]], verts[face.verts[2]], verts[face.verts[1]]);
        if (plane.Dot( verts[face.verts[0]]) <= 0.0f)
        {
          continue;
        }
      }

      U32 j;
      for (j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 iuj = face.uvs[j];

        if (iv[ivj] != iu[iuj])
        {
          // new vert
          //
          VertexTL & dv = bucky.CurrVertexTL();

          Vector & sv = verts[ivj];
          dv.uv = uvs[iuj];

          dv.diffuse.Modulate( colors[ivj], bucky.diffInit.r, bucky.diffInit.g, bucky.diffInit.b, bucky.diffInit.a);
          dv.specular = 0xff000000;

          if (clipFlags == clipNONE)
          {
            cam.ProjectFromCameraSpace( dv, sv);

            // set vertex fog
            dv.SetFog();
          }
          else
          {
            cam.SetHomogeneousFromCameraSpace( dv, sv);
          }

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
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
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }

  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------


void MeshRoot::RenderColorNoAnimColorsI( FaceGroup *buckys, U32 buckys.count, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags) // 0xffffffff, = clipALL, = controlDEF
{
  ASSERT( groups.count == buckys.count);

  ASSERT( buckys.count <= MAXBUCKYS);
  ASSERT( vCount <= MAXVERTS && fCount < MAXTRIS);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  // setup bucket desc elements common to all faces
  //
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  if (baseColor.a < 255) 
  {
    BucketMan::forceTranslucent = TRUE;
  }
  ColorValue base( baseColor);

  Camera &cam = Vid::CurCamera();

  ColorValue col, amb;
  col.r = baseColor.r * U8toNormF32;
  col.g = baseColor.g * U8toNormF32;
  col.b = baseColor.b * U8toNormF32;
  col.a = baseColor.a * U8toNormF32;
  amb.r = Vid::renderState.ambientR * col.r;
  amb.g = Vid::renderState.ambientG * col.g;
  amb.b = Vid::renderState.ambientB * col.b;

  // setup buckys and fill them
  //
  FaceGroup * b, * be = buckys.data + buckys.count;
  for (b = buckys.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    // calc constant diffuse component for each material (bucky)
    bucky.diffInit.r = bucky.diff.r * amb.r + bucky.diff.r * col.r;
    bucky.diffInit.g = bucky.diff.g * amb.g + bucky.diff.g * col.g;
    bucky.diffInit.b = bucky.diff.b * amb.b + bucky.diff.b * col.b;
    bucky.diffInit.a = bucky.diff.a * col.a;

    // clear indexers
    //
    memset( iv, 0xff, sizeof(U16) * bucky.vertCount);
    memset( iu, 0xfd, sizeof(U16) * bucky.vertCount);

    // get memory
    //
    if (!Vid::LockBucket( bucky, _controlFlags, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
      continue;
    }

    // backcull, light, project...
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faces.count;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      if (!(bucky.flags0 & RS_2SIDED))
      {
        Plane &plane = planes[face.index];
	  	  if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
        {
          continue;
        }
      }

      U32 j;
      for (j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 iuj = face.uvs[j];

        if (iv[ivj] != iu[iuj])
        {
          // new vert
          //
          VertexTL & dv = bucky.CurrVertexTL();

          Vector & sv = verts[ivj];
          dv.uv = uvs[iuj];

          dv.diffuse.Modulate( colors[ivj], bucky.diffInit.r, bucky.diffInit.g, bucky.diffInit.b, bucky.diffInit.a);
          dv.specular = 0xff000000;

          if (clipFlags == clipNONE)
          {
            cam.TransformProjectFromModelSpace( dv, sv);

            // set vertex fog
            dv.SetFog();
          }
          else
          {
            cam.SetHomogeneousFromModelSpace( dv, sv);
          }

          bucky.SetIndex( (U16) bucky.vCount);
          iv[ivj] = (U16) bucky.vCount;
          iu[iuj] = (U16) bucky.vCount;
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
    if (!Vid::UnLockBucket( bucky, clipFlags, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", xsiName.str) );
    }
  }

  BucketMan::forceTranslucent = FALSE;

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------


#else

void MeshRoot::RenderShadowTexture( Bitmap & dstTex, Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, U32 _controlFlags, Color color, Bitmap * tex, U32 blend) //  = 0x00000000, NULL, RS_BLEND_DEF
{
  Vector v = DxLight::Manager::shadowMatrix.Front();
  v.y -= 0.8f;
//  v.y -= .2f;
  v.z = 0;
  v.Normalize();

  Vector * verts = (Vector * ) Vid::tempHeap.Request( vCount * sizeof( Vector));
  Bool hasAnim = animCycles.GetCount();
  if (hasAnim)
  {
    Matrix tranys[MAXMESHPERGROUP];
    SetVertsWorld( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);
  }
  else
  {
    stateArray[0].WorldMatrix().Transform( verts, vertices.data, vCount);
  }

  Vector vvv;
  vvv.x = 1;
//  vvv.y = .2f;
  vvv.y = .8f;
  vvv.z = 0;
  vvv.Normalize();

  Vector vo = bigBounds.Offset();
  vo.y += bigBounds.Radius();
  vo.x = (vo.x + (vo.y / vvv.y) * vvv.x);

  const Matrix & world = stateArray[0].WorldMatrix();

  Vector y1, v1( (F32) fabs(v.x), (F32) fabs(v.y), (F32) fabs(v.z)); 
  y1.y = (F32) si.texture->Height() / (2 * vo.x);
  y1.x = (F32) si.texture->Width()  / (2 * vo.x);
/*
  y1.x = (v.x < 0 ? -1 : 1) * bigBounds.Width();
  F32 t = y1.y / (F32)fabs(v.y);
  y1.x = (y1.x + (F32)fabs(v.x) * t);
  y1.y = (y1.z + (F32)fabs(v.z) * t) + (F32)fabs(v.x) * bigBounds.Width(); 
  y1.y = si.texture->Height() / y1.y;
  y1.x = si.texture->Width() / (2 * bigBounds.Width());
*/

  Vector * src = verts;
  Vector * ve = src + vCount;
  for (src; src < ve; src++)
  {
    *src -= world.posit;

    F32 t = v->y / v1.y;
    v->x = (v->x + v.x * t + vo.x) * y1.x; 
    src->y = (src->z + v.z * t + vo.x) * y1.y; 
    src->z = 0;
  }

  Vid::RenderBegin();
  Area<S32> r( 0, 0, dstTex.Width(), dstTex.Height());
  Vid::ClearBack( Color( (U32)255, (U32)255, (U32)0, (U32)255), &r);

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  VertexTL * vertmem = Vid::tempVertices;
  U16 * indexmem = Vid::tempIndices;

//  Vid::SetTexture( tex);

  // setup _buckys and fill them
  //
  FaceGroup * b, * be = _buckys.data + _buckys.count;
  for (b = _buckys.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    VertexTL * dv = vertmem;
    U16 * di = indexmem;

    memset( iv, 0xff, sizeof(U16) * vCount);

    color = 0xffffffff;
    Color c = color;
    Float2Int fa( color.a * bucky.diff.a + Float2Int::magic);
    c.a = (U8) fa.i;

    Vid::SetTexture( bucky.texture0);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    U32 count = 0;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];

        ASSERT( ivj < vCount);

        if (iv[ivj] == 0xffff)
        {
          // new vertex; light and project
          dv->diffuse  = c;
          dv->specular = 0xff000000;
          dv->uv = uvs[face.uvs[j]];
          dv->vv = verts[ivj];
          dv->rhw = 1;

          iv[ivj] = (U16)count++;
          dv++;
        }
        *di++ = iv[ivj];
      }
    }
    if (count)
    {
		  Vid::DrawIndexedPrimitive(
            PT_TRIANGLELIST,
            FVF_TLVERTEX,
            vertmem, count, indexmem, di - indexmem, 
            DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | blend);
    }
  }
  Vid::RenderEnd();

  Vid::tempHeap.Restore();

  // Copy backbuffer to shadow texture
  Vid::backBmp.CopyBits
  (
    dstTex, 0, 0, 0, 0, dstTex.Width(), dstTex.Height(), FALSE
  );
}
//----------------------------------------------------------------------------


void MeshRoot::RenderShadowTexture( Bitmap & dstTex, Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, U32 _controlFlags, Color color, Bitmap * tex, U32 blend) //  = 0x00000000, NULL, RS_BLEND_DEF
{
  Camera & lastcam = Vid::CurCamera(), cam;

  Area<S32> rect( 0, 0, dstTex.Width(), dstTex.Height()), vRect = cam.ViewRect();
  cam.Setup( rect);
  cam.SetProjTransform( lastcam.NearPlane(), lastcam.FarPlane(), lastcam.FOV());

  Matrix trans, world = stateArray[0].WorldMatrix();
  Vector pos = bigBounds.Offset() + world.posit;

  Vector v = DxLight::Manager::shadowMatrix.Front();

  Matrix mat;
  mat.ClearData();
  mat.SetFromFront( v * -1);

  world.posit.ClearData();
  trans.SetInverse( mat);
  trans = world * trans;

  Vector size( bigBounds.Breadth(), bigBounds.Height(), bigBounds.Width());
  trans.Rotate( size);

//  F32 xz = -size.x / (Vid::Math::projInvX  * ((F32) dstTex.Width()  + 5.0f)); 
//  F32 yz = -size.y / (Vid::Math::projInvY  * ((F32) dstTex.Height() + 5.0f)); 
//  mat.posit = pos + v * (xz > yz ? xz : yz) * bigBounds.Radius() * .8f;
  F32 zz = -bigBounds.Radius() * .52f / (Vid::Math::projInvY  * ((F32) dstTex.Height() + 2.0f)); 
  mat.posit = pos + v * zz;
  mat.posit.x -= v.x * bigBounds.Width();

  cam.SetWorldAll( mat);
  Vid::SetCamera( cam);

  Matrix tranys[MAXMESHPERGROUP];
  Vector * verts;
  Bool hasAnim = animCycles.GetCount();
  if (hasAnim)
  {
    // setup Math::modelViewVector
//    pos = pos - mat.posit;

    // set up transform matrices and transform verts to view space
    //
    verts = (Vector * ) Vid::tempHeap.Request( vCount * sizeof( Vector));
    SetVertsView( stateArray, tranys, verts, vCount, (_controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE);
  }
  else
  {
    Vid::SetWorldTransform( stateArray[0].WorldMatrix());
  }

  Material material;
  material.SetDiffuse( 1, 1, 0, 1);
  material.SetupD3D();
  Vid::viewport->SetBackground( material.GetMaterialHandle());

  Vid::RenderBegin();
  Area<S32> r( 0, 0, dstTex.Width(), dstTex.Height());
  Vid::RenderClear( Vid::renderState.clearFlags, &r);

  Bool alpha = Vid::SetAlphaState( TRUE);

  ASSERT( _buckys.count <= MAXBUCKYS);
  ASSERT( _buckys.count == groups.count);

#ifdef DOSTATISTICS
  Statistics::tempTris = 0;
#endif

  VertexTL * vertmem = Vid::tempVertices;
  U16 * indexmem = Vid::tempIndices;

//  Vid::SetTexture( tex);

  // setup _buckys and fill them
  //
  FaceGroup * b, * be = _buckys.data + _buckys.count;
  for (b = _buckys.data; b < be; b++)
  {
    FaceGroup & bucky = *b;

    // clear bucky counts
    bucky.vCount = 0;
    bucky.iCount = 0;

    VertexTL * dv = vertmem;
    U16 * di = indexmem;

    memset( iv, 0xff, sizeof(U16) * vCount);
    memset( iu, 0xfd, sizeof(U16) * textCount);

    color = 0xffffffff;
    Color c = color;
    Float2Int fa( color.a * bucky.diff.a + Float2Int::magic);
    c.a = (U8) fa.i;

    Vid::SetTexture( bucky.texture0);

    // for all the faces in this group
    //
    FaceObj * f, * fe = bucky.faces.data + bucky.faceCount;
    U32 count = 0;
    for (f = bucky.faces.data; f < fe; f++)
    {
      FaceObj & face = *f;
      ASSERT( face.verts[0] < vCount && face.verts[1] < vCount && face.verts[2] < vCount);

      if (hasAnim)
      {
        // backcull
        //
        if (!(bucky.flags0 & RS_2SIDED))
        {
          Plane plane;
          plane.Set( verts[face.verts[0]], verts[face.verts[1]], verts[face.verts[2]]);
          if (Vid::BackcullTest( plane.Dot( verts[face.verts[0]])))
          {
            continue;
          }
        }
      }
      else
      {
        // backcull
        //
        if (!(bucky.flags0 & RS_2SIDED))
        {
  //        Plane &plane = planes[face.index];
          Plane plane;
          plane.Set( vertices[face.verts[0]], vertices[face.verts[2]], vertices[face.verts[1]]);
	  	    if (plane.Evalue(Vid::Math::modelViewVector) <= 0.0f)
          {
            continue;
          }
        }
      }

      // light, project...
      //
      for (U32 j = 0; j < 3; j++)
      {
        U16 ivj = face.verts[j];
	      U16 iuj = face.uvs[j];

        ASSERT( ivj < vCount && iuj < uvs.count);

        if (iv[ivj] != iu[iuj])
        {
          // new vertex; light and project

          Vector & sv = hasAnim ? verts[ivj] : vertices[ivj];

          dv->diffuse  = c;
          dv->specular = 0xff000000;
          dv->uv = uvs[iuj];

          if (hasAnim)
          {
            Vid::ProjectFromCameraSpace( *dv, sv);
          }
          else
          {
            Vid::TransformProjectFromModelSpace( *dv, sv);
          }

          iv[ivj] = (U16)count;
          iu[iuj] = (U16)count++;
          dv++;
        }
        *di++ = iv[ivj];
      }
    }
    if (count)
    {
		  Vid::DrawIndexedPrimitive(
            PT_TRIANGLELIST,
            FVF_TLVERTEX,
            vertmem, count, indexmem, di - indexmem, 
            DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | blend);
    }


  }
  Vid::RenderEnd();
  Vid::SetAlphaState( alpha);
  Vid::SetCamera( lastcam);
  Vid::viewport->SetBackground( Vid::fogMaterial->GetMaterialHandle());
  material.ReleaseD3D();

  Vid::tempHeap.Restore();

  // Copy backbuffer to shadow texture
  Vid::backBmp.CopyBits
  (
    dstTex, 0, 0, 0, 0, dstTex.Width(), dstTex.Height(), FALSE
  );
}
//----------------------------------------------------------------------------

#endif
#endif