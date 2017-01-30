///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// mesh.cpp
//
// 07-JUL-1998
//

#include "mesh.h"
#include "IMESHUtil.h"
#include "perfstats.h"
#include "statistics.h"
//----------------------------------------------------------------------------

#if 0
void MeshRoot::RenderLightProjectCamera( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  ASSERT( _buckys.count < MAXBUCKYS);
  ASSERT( vertCount < MAXVERTS && normCount < MAXVERTS && textCount < MAXVERTS);

  static ColorValue specInit = { 0.0f, 0.0f, 0.0f, 1.0f };

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;

  if (doModelSpace)
  {
    SetVertsModel( stateArray, tranys, verts);
  }
  else
  {
    SetVertsView( stateArray, tranys, verts);
  }

  // clear indexer lists
  U32 i;
  for (i = 0; i < _buckys.count; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[i][0], 0xfe, sizeof(U16) * normCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);
  }

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | renderFlags,
    TRUE);

  Camera &cam = Vid::CurCamera();

  if (clipFlags == clipNONE)
  {
    Bucky buckyArray[MAXMESHPERGROUP];

    for (i = 0; i < _buckys.count; i++)
    {
      *((BuckyDesc *) &buckyArray[i]) = _buckys[i];
    }

    // setup and lock _buckys
    if (!Vid::LockBuckys( &buckyArray[0], _buckys.count))
    {
      LOG_WARN( ("Can't lock buckets for %s", name) );
      return;
    }

    for (i = 0; i < _buckys.count; i++)
    {
      // calc constant diffuse component for each bucky
      Bucky &bucky = buckyArray[i];
      MaterialDesc &desc = bucky.material->GetDesc();

      bucky.diff.r = Vid::renderState.ambientR * desc.diffuse.r * DxLight::Manager::darken;
      bucky.diff.g = Vid::renderState.ambientG * desc.diffuse.g * DxLight::Manager::darken;
      bucky.diff.b = Vid::renderState.ambientB * desc.diffuse.b * DxLight::Manager::darken;
      bucky.diffa  = (U32) Utils::FastFtoL(desc.diffuse.a * (F32) Mesh::Manager::translucent);

      // clear bucky counts
      bucky.vertCount = 0;
      bucky.indexCount = 0;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj &face = faces[i];
      ASSERT( face.buckyIndex < _buckys.count);

      Bucky &bucky = buckyArray[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (Vid::renderState.status.cull && !(bucky.flags & RS_2SIDED))
      {
        Plane plane;
        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
        if (doModelSpace)
        {
	  	    if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
        }
        else if (plane.Dot( verts[ivi[0]]) >= 0.0f)
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
        Vector norm;
        ColorValue diff, spec;

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
            dv.uv = _uvs[iuj];

            bucky.SetIndex( (U16) bucky.vertCount);
            iv[ivj] = (U16) bucky.vertCount;
            in[inj] = (U16) bucky.vertCount;
            iu[iuj] = (U16) bucky.vertCount;
            bucky.vertCount++;
          }
        }
        else
        {
          // new vertex; light and project
          Vector &sv = verts[ivj];
          dv.uv = _uvs[iuj];

          diff = bucky.diff;
          spec = specInit;          

          if (doModelSpace)
          {
            DxLight::Manager::LightModelSpace( sv, normals[inj], *bucky.material, diff, spec);

            cam.TransformProjectFromModelSpace( dv, sv);
          }
          else
          {
            tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
            DxLight::Manager::LightCameraSpace( sv, norm, *bucky.material, diff, spec);

            cam.ProjectFromCameraSpace( dv, sv);
          }
          // set the colors
          dv.diffuse.Set(  diff.r, diff.g, diff.b, bucky.diffa);
          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

          // set vertex fog
          dv.SetFog();

          bucky.SetIndex( (U16) bucky.vertCount);
          iv[ivj] = (U16) bucky.vertCount;
          in[inj] = (U16) bucky.vertCount;
          iu[iuj] = (U16) bucky.vertCount;
          bucky.vertCount++;
        }
      }
    }
    Vid::UnLockBuckys( buckyArray, _buckys.count);
  }
  else
  {
    ColorValue  diffInits[MAXBUCKYS];
    U32         diffInitAs[MAXBUCKYS];

    for (i = 0; i < _buckys.count; i++)
    {
      MaterialDesc &desc = _buckys[i].material->GetDesc();

      diffInits[i].r = Vid::renderState.ambientR * desc.diffuse.r * DxLight::Manager::darken;
      diffInits[i].g = Vid::renderState.ambientG * desc.diffuse.g * DxLight::Manager::darken;
      diffInits[i].b = Vid::renderState.ambientB * desc.diffuse.b * DxLight::Manager::darken;
      diffInitAs[i] = (U32) Utils::FastFtoL(desc.diffuse.a * (F32) Mesh::Manager::translucent);

      // setup counters
      tempBuckets[i].vertCount = 0;
      tempBuckets[i].indexCount = 0;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj &face = faces[i];
      ASSERT( face.buckyIndex < _buckys.count);

      BuckyDesc &bucky = _buckys[face.buckyIndex];
      TempBucket &tbucket = tempBuckets[face.buckyIndex];

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (Vid::renderState.status.cull && !(bucky.flags & RS_2SIDED))
      {
        Plane plane;
        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
        if (doModelSpace)
        {
	  	    if (plane.Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
        }
        else if (plane.Dot( verts[ivi[0]]) >= 0.0f)
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
        Vector norm;
        ColorValue diff, spec;

	      U16 inj = face.norms[j];
	      U16 iuj = face.uvs[j];

        VertexTL &dv = tbucket.CurrVertexTL();

        U16 ivj = ivi[j];

        if (iv[ivj] == in[inj])
        {
          if (iv[ivj] == iu[iuj])
          {
            // identical vertex; just set index
            tbucket.SetIndex( iv[ivj]);
          }
          else
          {
            // already lit; copy vert
            dv = tbucket.GetVertexTL(iv[ivj]);
            dv.uv = uvs[iuj];

            tbucket.SetIndex( (U16) tbucket.vertCount);
            iv[ivj] = (U16) tbucket.vertCount;
            in[inj] = (U16) tbucket.vertCount;
            iu[iuj] = (U16) tbucket.vertCount;
            tbucket.vertCount++;
          }
        }
        else
        {
          // new vertex; light and produce homogeneous coords
          Vector &sv = verts[ivj];
          dv.uv = uvs[iuj];

          diff = diffInits[face.buckyIndex];
          spec = specInit;

          if (doModelSpace)
          {
            DxLight::Manager::LightModelSpace( sv, normals[inj], *bucky.material, diff, spec);

            cam.SetHomogeneousFromModelSpace( dv, sv);
          }
          else
          {
            tranys[vertToState[ivj].index[0]].Rotate( norm, normals[inj]);
            DxLight::Manager::LightCameraSpace( sv, norm, *bucky.material, diff, spec);

            cam.SetHomogeneousFromCameraSpace( dv, sv);
          }
            // set the colors
          dv.diffuse.Set(  diff.r, diff.g, diff.b, diffInitAs[face.buckyIndex]);
          dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

          tbucket.SetIndex( (U16) tbucket.vertCount);
          iv[ivj] = (U16) tbucket.vertCount;
          in[inj] = (U16) tbucket.vertCount;
          iu[iuj] = (U16) tbucket.vertCount;
          tbucket.vertCount++;
        }
      }
    }

    for (i = 0; i < _buckys.count; i++)
    {
      BuckyDesc &bucky = _buckys[i];
      Vid::SetBucketTexture(  bucky.texture);

      TempBucket &tbucket = tempBuckets[i];

      if (!cam.ClipToBucket( tbucket.verts, tbucket.vertCount, tbucket.indices, tbucket.indexCount))
      {
        LOG_WARN( ("Can't lock buckets for %s", name) );
      }
    }
  }

  if (Mesh::Manager::doNormals)
  {
    F32 normLenFactor = ObjectSphere().Radius() * 0.08f;
    F32 NORMDRAWWID = normLenFactor * 0.12f;

    Camera &cam = Vid::CurCamera();

    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_GLOW);

    // set the polygon material
    // force translucency
    Vid::SetBucketMaterialTexture( NULL, NULL, TRUE);

    // clear indexer lists
    memset( &ivA[0][0], 0xff, sizeof(U16) * vertCount);
    memset( &inA[0][0], 0xfe, sizeof(U16) * normCount);

	  for (i = 0; i < faceCount; i++)
    {
      FaceObj &face = faces[i];

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

        Vector &vert = verts[ivj];
        VertexTL vertmem[5];

	      vertmem[0].vv.x = vert.x + norm.x;
	      vertmem[0].vv.y = vert.y + norm.y;
	      vertmem[0].vv.z = vert.z + norm.z;
	      vertmem[0].diffuse  = color;
	      vertmem[0].specular = RGBA_MAKE( 0, 0, 0, 255);

        cam.SetHomogeneousFromCameraSpace( vertmem[0]);

	      vertmem[1].vv.x = vert.x + NORMDRAWWID;
	      vertmem[1].vv.y = vert.y;
	      vertmem[1].vv.z = vert.z;
	      vertmem[1].diffuse  = color;
	      vertmem[1].specular = RGBA_MAKE( 0, 0, 0, 255);

        cam.SetHomogeneousFromCameraSpace( vertmem[1]);

	      vertmem[2].vv.x = vert.x - NORMDRAWWID;
	      vertmem[2].vv.y = vert.y;
	      vertmem[2].vv.z = vert.z;
	      vertmem[2].diffuse  = color;
	      vertmem[2].specular = RGBA_MAKE( 0, 0, 0, 255);

        cam.SetHomogeneousFromCameraSpace( vertmem[2]);

	      vertmem[3].vv.x = vert.x;
	      vertmem[3].vv.y = vert.y;
	      vertmem[3].vv.z = vert.z + NORMDRAWWID;
	      vertmem[3].diffuse  = color;
	      vertmem[3].specular = RGBA_MAKE( 0, 0, 0, 255);

        cam.SetHomogeneousFromCameraSpace( vertmem[3]);

	      vertmem[4].vv.x = vert.x;
	      vertmem[4].vv.y = vert.y;
	      vertmem[4].vv.z = vert.z - NORMDRAWWID;
	      vertmem[4].diffuse  = color;
	      vertmem[4].specular = RGBA_MAKE( 0, 0, 0, 255);

        cam.SetHomogeneousFromCameraSpace( vertmem[4]);

        static U16 normalIndexMem[] = { 0, 1, 2, 0, 3, 4 };

        Vid::CurCamera().ClipToBucket( vertmem, 5, normalIndexMem, 6);
      }
    }
  }
}
//----------------------------------------------------------------------------


void MeshRoot::RenderLight( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  Bucky buckyArray[MAXMESHPERGROUP];
  U32 i;

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_LVERTEX,
    // FIXME for clipper
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | renderFlags | (clipFlags == clipNONE ? DP_DONOTCLIP : 0),
    TRUE);

  for (i = 0; i < _buckys.count; i++)
  {
    *((BuckyDesc *) &buckyArray[i]) = _buckys[i];
  }

  // setup and lock _buckys
  if (!Vid::LockBuckys( &buckyArray[0], _buckys.count))
  {
    return;
  }

  for (i = 0; i < _buckys.count; i++)
  {
    // calc constant diffuse component for each bucky
    Bucky &bucky = buckyArray[i];
    MaterialDesc &desc = bucky.material->GetDesc();

    bucky.diff.r = Vid::renderState.ambientR * desc.diffuse.r * DxLight::Manager::darken;
    bucky.diff.g = Vid::renderState.ambientG * desc.diffuse.g * DxLight::Manager::darken;
    bucky.diff.b = Vid::renderState.ambientB * desc.diffuse.b * DxLight::Manager::darken;
    bucky.diffa  = (U32) Utils::FastFtoL(desc.diffuse.a * (F32) Mesh::Manager::translucent);

    // clear bucky counts
    bucky.vertCount = 0;
    bucky.indexCount = 0;
  }
  static ColorValue specInit = { 0.0f, 0.0f, 0.0f, 1.0f };

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts);

  ASSERT( _buckys.count < MAXBUCKYS);

  // clear indexer lists
  for (i = 0; i < _buckys.count; i++)
  {
    // !!! FIXME
    // need to know max possible verts to make these as quick as possible
    U32 memsize = sizeof(U16) * faceCount * 3;
    memset( &ivA[i][0], 0xff, memsize);
    memset( &inA[i][0], 0xfe, memsize);
    memset( &iuA[i][0], 0xfd, memsize);
  }

  // backcull, transform and light
  Bool doCull = !(renderFlags & RS_2SIDED) && Vid::renderState.status.cull;
	for (i = 0; i < faceCount; i++)
  {
    FaceObj &face = faces[i];
    ASSERT( face.buckyIndex < _buckys.count);

    Bucky &bucky = buckyArray[face.buckyIndex];

	  U16 ivi[3];
    ivi[0] = face.verts[0];
    ivi[1] = face.verts[1];
    ivi[2] = face.verts[2];
    ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

    if (doCull)
    {
      Plane plane;
      plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
      if (plane.Dot( verts[ivi[0]]) >= 0.0f)
      {
        continue;
      }
    }
    Vector norm;
    ColorValue diff, spec;
    U16 ini, iui;

    // indexers for this bucky
    U16 *iv = ivA[face.buckyIndex];
    U16 *in = inA[face.buckyIndex];
    U16 *iu = iuA[face.buckyIndex];

    U32 j;
    for (j = 0; j < 3; j++)
    {
	    ini = face.norms[j];
	    iui = face.uvs[j];
      VertexL &dv = bucky.CurrVertexL();

      U16 ivj = ivi[j];

      if (iv[ivj] == in[ini])
      {
        if (iv[ivj] == iu[iui])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // already lit; copy vert
          dv = bucky.GetVertexL(iv[ivj]);
          dv.uv = uvs[iui];

          bucky.SetIndex( (U16) bucky.vertCount);
          iv[ivj] = (U16) bucky.vertCount;
          in[ini] = (U16) bucky.vertCount;
          iu[iui] = (U16) bucky.vertCount;
          bucky.vertCount++;
        }
      }
      else
      {
        // new vertex; light
        dv.vv = verts[ivj];
        dv.uv = uvs[iui];
        tranys[vertToState[ivj].index[0]].Rotate( norm, normals[ini]);
        diff = bucky.diff;
        spec = specInit;
        DxLight::Manager::LightCameraSpace( dv.vv, norm, *bucky.material, diff, spec);

          // set the colors
        dv.diffuse.Set(  diff.r, diff.g, diff.b, bucky.diffa);
        dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

        bucky.SetIndex( (U16) bucky.vertCount);
        iv[ivj] = (U16) bucky.vertCount;
        in[ini] = (U16) bucky.vertCount;
        iu[iui] = (U16) bucky.vertCount;
        bucky.vertCount++;
      }
    }
  }

  Vid::UnLockBuckys( buckyArray, _buckys.count);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightIndexed( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  Bucky buckyArray[MAXMESHPERGROUP];
  U32 i;

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_LVERTEX,
    // FIXME for clipper
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | renderFlags | (clipFlags == clipNONE ? DP_DONOTCLIP : 0),
    TRUE);

  for (i = 0; i < _buckys.count; i++)
  {
    *((BuckyDesc *) &buckyArray[i]) = _buckys[i];
  }

  // setup and lock _buckys
  if (!Vid::LockBuckys( &buckyArray[0], _buckys.count))
  {
    return;
  }

  for (i = 0; i < _buckys.count; i++)
  {
    // calc constant diffuse component for each bucky
    Bucky &bucky = buckyArray[i];
    MaterialDesc &desc = bucky.material->GetDesc();

    bucky.diff.r = Vid::renderState.ambientR * desc.diffuse.r * DxLight::Manager::darken;
    bucky.diff.g = Vid::renderState.ambientG * desc.diffuse.g * DxLight::Manager::darken;
    bucky.diff.b = Vid::renderState.ambientB * desc.diffuse.b * DxLight::Manager::darken;
    bucky.diffa  = (U32) Utils::FastFtoL(desc.diffuse.a * (F32) Mesh::Manager::translucent);

    // clear bucky counts
    bucky.vertCount = 0;
    bucky.indexCount = 0;
  }
  static ColorValue specInit = { 0.0f, 0.0f, 0.0f, 1.0f };

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts);

  ASSERT( _buckys.count < MAXBUCKYS);

  // clear indexer lists
  for (i = 0; i < _buckys.count; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
  }

  // backcull, transform normals, and light
  Bool doCull = !(renderFlags & RS_2SIDED) && Vid::renderState.status.cull;
	for (i = 0; i < faceCount; i++)
  {
    FaceObj &face = faces[i];
    ASSERT( face.buckyIndex < _buckys.count);

    Bucky &bucky = buckyArray[face.buckyIndex];

	  U16 ivi[3];
    ivi[0] = face.verts[0];
    ivi[1] = face.verts[1];
    ivi[2] = face.verts[2];
    ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

    if (doCull)
    {
      Plane plane;
      plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//        if (plane.Evalue( Vid::model_view_vector) < 0.0f)
//        if (srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
      if (plane.Dot( verts[ivi[0]]) >= 0.0f)
      {
        continue;
      }
    }
    Vector norm;
    ColorValue diff, spec;

    // indexer for this bucky
    U16 *iv = ivA[face.buckyIndex];

    U32 j;
    for (j = 0; j < 3; j++)
    {
      U16 ivj = ivi[j];

      VertexL &dv = bucky.CurrVertexL();

      if (iv[ivj] == 0xffff)
      {
        // new vertex; light
        dv.vv = verts[ivj];
        dv.uv = uvs[ivj];

        tranys[vertToState[ivj].index[0]].Rotate( norm, normals[ivj]);
        diff = bucky.diff;
        spec = specInit;
        DxLight::Manager::LightCameraSpace( dv.vv, norm, *bucky.material, diff, spec);

          // set the colors
        dv.diffuse.Set(  diff.r, diff.g, diff.b, bucky.diffa);
        dv.specular.Set( spec.r, spec.g, spec.b, (U32) 255);

        bucky.SetIndex( (U16) bucky.vertCount);
        iv[ivj] = (U16) bucky.vertCount;
        bucky.vertCount++;
      }
      else
      {
        // already done
        bucky.SetIndex( iv[ivj]);
      }
    }
	}

  Vid::UnLockBuckys( buckyArray, _buckys.count);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderDo( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_VERTEX,
    // FIXME for clipper
    DP_DONOTUPDATEEXTENTS | renderFlags | (clipFlags == clipNONE ? DP_DONOTCLIP : 0),
    TRUE);

  // setup and lock _buckys
  Bucky buckyArray[MAXMESHPERGROUP];
  U32 i;
  for (i = 0; i < _buckys.count; i++)
  {
    *((BuckyDesc *) &buckyArray[i]) = _buckys[i];
  }
  if (!Vid::LockBuckys( &buckyArray[0], _buckys.count))
  {
    return;
  }

  for (i = 0; i < _buckys.count; i++)
  {
    Bucky &bucky = buckyArray[i];
    // clear bucky counts
    bucky.vertCount = 0;
    bucky.indexCount = 0;
  }

  // transform verts to world space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsWorld( stateArray, tranys, verts);

  ASSERT( _buckys.count < MAXBUCKYS);

  // clear indexer lists
  for (i = 0; i < _buckys.count; i++)
  {
    // !!! FIXME
    // need to know max possible verts to make these as quick as possible
    U32 memsize = sizeof(U16) * faceCount * 3;
    memset( &ivA[i][0], 0xff, memsize);
    memset( &inA[i][0], 0xfe, memsize);
    memset( &iuA[i][0], 0xfd, memsize);
  }

  // fill _buckys
	for (i = 0; i < faceCount; i++)
  {
    FaceObj &face = faces[i];
    ASSERT( face.buckyIndex < _buckys.count);

    Bucky &bucky = buckyArray[face.buckyIndex];

	  U16 ivi[3];
    ivi[0] = face.verts[0];
    ivi[1] = face.verts[1];
    ivi[2] = face.verts[2];
    ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

    U16 ini, iui;

    // indexers for this bucky
    U16 *iv = ivA[face.buckyIndex];
    U16 *in = inA[face.buckyIndex];
    U16 *iu = iuA[face.buckyIndex];

    U32 j;
    for (j = 0; j < 3; j++)
    {
	    ini = face.norms[j];
	    iui = face.uvs[j];
      Vertex &dv = bucky.CurrVertex();

      U16 ivj = ivi[j];

      if (iv[ivj] == in[ini])
      {
        if (iv[ivj] == iu[iui])
        {
          // identical vertex; just set index
          bucky.SetIndex( iv[ivj]);
        }
        else
        {
          // already added; copy vert
          dv = bucky.GetVertex(iv[ivj]);
          dv.uv = uvs[iui];

          bucky.SetIndex( (U16) bucky.vertCount);
          iv[ivj] = (U16) bucky.vertCount;
          in[ini] = (U16) bucky.vertCount;
          iu[iui] = (U16) bucky.vertCount;
          bucky.vertCount++;
        }
      }
      else
      {
        // new vertex; rotate normal
        dv.vv = verts[ivj];
        dv.uv = uvs[iui];
        tranys[vertToState[ivj].index[0]].Rotate( dv.nv, normals[ini]);

        bucky.SetIndex( (U16) bucky.vertCount);
        iv[ivj] = (U16) bucky.vertCount;
        in[ini] = (U16) bucky.vertCount;
        iu[iui] = (U16) bucky.vertCount;
        bucky.vertCount++;
      }
    }
  }

  Vid::UnLockBuckys( buckyArray, _buckys.count);
}
//----------------------------------------------------------------------------

#if 0
void MeshRoot::RenderLightOrdered( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  Bucky buckyArray[MAXMESHPERGROUP];
  U32 i;

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_LVERTEX,
    // FIXME for clipper
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | renderFlags | (clipFlags == clipNONE ? DP_DONOTCLIP : 0),
    FALSE);

  for (i = 0; i < _buckys.count; i++)
  {
    *((BuckyDesc *) &buckyArray[i]) = _buckys[i];
    buckyArray[i].indexCount = 0;
  }

  // setup and lock _buckys
  if (!Vid::LockBuckys( &buckyArray[0], _buckys.count))
  {
    return;
  }

  for (i = 0; i < _buckys.count; i++)
  {
    // calc constant diffuse component for each bucky
    Bucky &bucky = buckyArray[i];
    MaterialDesc &desc = bucky.material->GetDesc();

    bucky.diff.r = Vid::renderState.ambientR * desc.diffuse.r * DxLight::Manager::darken;
    bucky.diff.g = Vid::renderState.ambientG * desc.diffuse.g * DxLight::Manager::darken;
    bucky.diff.b = Vid::renderState.ambientB * desc.diffuse.b * DxLight::Manager::darken;
    bucky.diffa  = (U32) Utils::FastFtoL(desc.diffuse.a * (F32) Mesh::Manager::translucent);

    // clear bucky counts
    bucky.vertCount = 0;
    bucky.indexCount = 0;
  }
  static ColorValue specInit = { 0.0f, 0.0f, 0.0f, 1.0f };

  // set up transform matrices and transform verts to view space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsView( stateArray, tranys, verts);

  // backcull, transform and light
  Bool doCull = !(renderFlags & RS_2SIDED) && Vid::renderState.status.cull;
	for (i = 0; i < faceCount; i++)
  {
    FaceObj &face = faces[i];
    ASSERT( face.buckyIndex < _buckys.count);

    Bucky &bucky = buckyArray[face.buckyIndex];

	  U16 ivi[3];
    ivi[0] = face.verts[0];
    ivi[1] = face.verts[1];
    ivi[2] = face.verts[2];
    ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

    if (doCull)
    {
      Plane plane;
      plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
//        if (plane.Evalue( Vid::model_view_vector) < 0.0f)
      if (plane.Dot( verts[ivi[0]]) >= 0.0f)
      {
        continue;
      }
    }
    Vector norm;
    VertexL *vert;
    ColorValue diff, spec;

    U32 j;
    for (j = 0; j < 3; j++)
    {
      U16 ivj = ivi[j];

      vert = &bucky.CurrVertexL();
      vert->vv = verts[ivj];
      vert->uv = uvs[face.uvs[j]];
      tranys[vertToState[ivj].index[0].index].Rotate( norm, normals[face.norms[j]]);
      diff = bucky.diff;
      spec = specInit;
      DxLight::Manager::LightCameraSpace( vert->vv, norm, *bucky.material, diff, spec);

      vert->Set( diff, bucky.diffa, spec);
      bucky.vertCount++;
    }
	}

  Vid::UnLockBuckys( buckyArray, _buckys.count);
}
//----------------------------------------------------------------------------

void MeshRoot::RenderOrdered( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_VERTEX,
    // FIXME for clipper
    DP_DONOTUPDATEEXTENTS | renderFlags | (clipFlags == clipNONE ? DP_DONOTCLIP : 0),
    FALSE);

  // setup and lock _buckys
  Bucky buckyArray[MAXMESHPERGROUP];
  U32 i;
  for (i = 0; i < _buckys.count; i++)
  {
    *((BuckyDesc *) &buckyArray[i]) = _buckys[i];

    // turn off indexing request
    buckyArray[i].indexCount = 0;
  }
  if (!Vid::LockBuckys( &buckyArray[0], _buckys.count))
  {
    return;
  }

  for (i = 0; i < _buckys.count; i++)
  {
    Bucky &bucky = buckyArray[i];
    // clear bucky counts
    bucky.vertCount = 0;
    bucky.indexCount = 0;
  }

  // transform verts to world space
  Matrix tranys[MAXMESHPERGROUP];
  Vector *verts = (Vector *) Vid::tempVertices;
  SetVertsWorld( stateArray, tranys, verts);

  // fill _buckys
	for (i = 0; i < faceCount; i++)
  {
    FaceObj &face = faces[i];
    ASSERT( face.buckyIndex < _buckys.count);

    Bucky &bucky = buckyArray[face.buckyIndex];

	  U16 i0 = face.verts[0];
    U16 i1 = face.verts[1];
    U16 i2 = face.verts[2];

    Vertex *vert;

    vert = &bucky.CurrVertex();
    vert->vv = verts[i0];
    vert->uv = uvs[face.uvs[0]];
    tranys[vertToState[i0].index[0].index].Rotate( vert->nv, normals[face.norms[0]]);
    bucky.vertCount++;

    vert = &bucky.CurrVertex();
    vert->vv = verts[i1];
    vert->uv = uvs[face.uvs[1]];
    tranys[vertToState[i1].index[0].index].Rotate( vert->nv, normals[face.norms[1]]);
    bucky.vertCount++;

    vert = &bucky.CurrVertex();
    vert->vv = verts[i2];
    vert->uv = uvs[face.uvs[2]];
    tranys[vertToState[i2].index[0].index].Rotate( vert->nv, normals[face.norms[2]]);
    bucky.vertCount++;
	}

  Vid::UnLockBuckys( buckyArray, _buckys.count);
}
//----------------------------------------------------------------------------
#endif

void Mesh::Render( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  clipFlags;
  stateArray;
#if 0
  if (!globDoLighting || Mesh::Manager::doProjectClip)
  {
    return;
  }
//  return;

  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_LVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | renderFlags | (clipFlags == clipNONE ? DP_DONOTCLIP : 0),
    TRUE);

  Plane planes[MAXTRIS];
  U32 planeCount = localIndices.count / 3;
  ASSERT( planeCount < MAXTRIS);

  for (U32 j = 0, icount = 0; j < planeCount; j++, icount += 3)
  {
    planes[j].Set( 
      localVertices[localIndices[icount + 0]],
      localVertices[localIndices[icount + 1]],
      localVertices[localIndices[icount + 2]]);
  }

  U32 i, vertIndex = 0, indexIndex = 0;
  for (i = 0; i < localGroups.count; i++)
  {
    VertGroup &group = localGroups[i];

    Vid::SetBucketFlags( stateArray[group.stateIndex].Mesh().renderFlags 
      | DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_2SIDED | (clipFlags == clipNONE ? DP_DONOTCLIP : 0));

    Vid::SetWorldTransform( stateArray[stateIndex].WorldMatrix());
    Vid::SetBucketMaterial( group.material);
    Vid::SetBucketTexture( group.texture);

    U32 vcount = group.vertCount;
    U32 icount = group.indexCount;
		VertexL *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, vcount, &indexmem, icount))
    {
      return;
    }

    if (localColors.count)
    {
      DxLight::Manager::Light( vertmem, &planes[group.planeIndex], 
        &localVertices[vertIndex], &localNormals[vertIndex], &localColors[vertIndex], &localUvs[vertIndex],
        vcount, indexmem, &localIndices[indexIndex], icount);
    }
    else
    {
      DxLight::Manager::Light( vertmem, &planes[group.planeIndex], 
        &localVertices[vertIndex], &localNormals[vertIndex], &localUvs[vertIndex], 
        vcount, indexmem, &localIndices[indexIndex], icount);
    }

    vertIndex  += group.vertCount;
    indexIndex += group.indexCount;

    Vid::UnlockIndexedPrimitiveMem( vcount, icount, FALSE, TRUE);
  }
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightProjectOldPipe( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_TLVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP,
    TRUE);

  Camera &camera = Vid::CurCamera();

  U32 i, vertIndex = 0, indexIndex = 0;
  for (i = 0; i < groups.count; i++)
  {
    VertGroup &group = groups[i];

    Vid::SetBucketFlags( stateArray[group.stateIndex].Mesh().renderFlags 
      | DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP);

    Vid::SetWorldTransform( stateArray[group.stateIndex].WorldMatrix());
    Vid::SetBucketMaterial( group.material);
    Vid::SetBucketTexture(  group.texture);
 
    // must be after SetWorldTransform
    // assumes SetActiveList has been called (by MeshEnt::Render)
    DxLight::Manager::SetupLightsModelSpace();

    if (clipFlags == clipNONE)
    {
      if (colors.count)
      {
        camera.LightProjectNoClip( &planes[group.planeIndex], 
          &vertices[vertIndex], &normals[vertIndex], &uvs[vertIndex], &colors[vertIndex],
          group.vertCount, &indices[indexIndex], group.indexCount);
      }
#ifdef __DO_XMM_BUILD
      else if (Vid::do_xmm )
        camera.LightProjectNoClipXmm( &planes[group.planeIndex], 
          &vertices[vertIndex], &normals[vertIndex], &uvs[vertIndex], 
          group.vertCount, &indices[indexIndex], group.indexCount);
#endif
      else
        camera.LightProjectNoClip( &planes[group.planeIndex], 
          &vertices[vertIndex], &normals[vertIndex], &uvs[vertIndex], 
          group.vertCount, &indices[indexIndex], group.indexCount);
    }
    else
    {
      if (colors.count)
      {
        camera.LightProjectClip( &planes[group.planeIndex], 
          &vertices[vertIndex], &normals[vertIndex], &uvs[vertIndex], &colors[vertIndex],
          group.vertCount, &indices[indexIndex], group.indexCount);
      }
#ifdef __DO_XMM_BUILD
      else if (Vid::do_xmm )
        camera.LightProjectClipXmm( &planes[group.planeIndex], 
          &vertices[vertIndex], &normals[vertIndex], &uvs[vertIndex], 
          group.vertCount, &indices[indexIndex], group.indexCount);
#endif
      else
        camera.LightProjectClip( &planes[group.planeIndex], 
          &vertices[vertIndex], &normals[vertIndex], &uvs[vertIndex], 
          group.vertCount, &indices[indexIndex], group.indexCount);
    }

    vertIndex  += group.vertCount;
    indexIndex += group.indexCount;
  }
}
//----------------------------------------------------------------------------

void MeshRoot::RenderLightOldPipe( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  stateArray;
  clipFlags;

#ifndef DODXLEANANDGRUMPY
  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_LVERTEX,
    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | renderFlags | (clipFlags == clipNONE ? DP_DONOTCLIP : 0),
    TRUE);

  U32 i, vertIndex = 0, indexIndex = 0;
  for (i = 0; i < groups.count; i++)
  {
    VertGroup &group = groups[i];

//    Matrix temp;
//    stateMats[group.stateIndex].Transform( temp, stateArray[group.stateIndex].WorldMatrix());
//    Vid::SetWorldTransform( temp);

    Vid::SetBucketFlags( stateArray[group.stateIndex].Mesh().renderFlags 
      | DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | (clipFlags == clipNONE ? DP_DONOTCLIP : 0));

    Vid::SetWorldTransform( stateArray[group.stateIndex].WorldMatrix());
    Vid::SetBucketMaterial( group.material);
    Vid::SetBucketTexture( group.texture);

    U32 vcount = group.vertCount;
    U32 icount = group.indexCount;
		VertexL *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, vcount, &indexmem, icount))
    {
      return;
    }

    if (colors.count)
    {
      DxLight::Manager::Light( vertmem, &planes[group.planeIndex], 
        &vertices[vertIndex], &normals[vertIndex], &colors[vertIndex], &uvs[vertIndex],
        vcount, indexmem, &indices[indexIndex], icount);
    }
    else
    {
      DxLight::Manager::Light( vertmem, &planes[group.planeIndex], 
        &vertices[vertIndex], &normals[vertIndex], &uvs[vertIndex], 
        vcount, indexmem, &indices[indexIndex], icount);
    }

    vertIndex  += group.vertCount;
    indexIndex += group.indexCount;

    Vid::UnlockIndexedPrimitiveMem( vcount, icount, FALSE, TRUE);
  }
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderOldPipe( Array<AnimKey> &stateArray, U32 clipFlags) // = clipALL)
{
  stateArray;
  clipFlags;

#ifndef DODXLEANANDGRUMPY
  // setup bucket desc elements common to all faces
  Vid::SetBucketPrimitiveDesc(
    PT_TRIANGLELIST,
    FVF_VERTEX,
    DP_DONOTUPDATEEXTENTS | renderFlags | (clipFlags == clipNONE ? DP_DONOTCLIP : 0),
    TRUE);

  U32 i, vertIndex = 0, indexIndex = 0;
  for (i = 0; i < groups.count; i++)
  {
    VertGroup &group = groups[i];

    Vid::SetBucketFlags( stateArray[group.stateIndex].Mesh().renderFlags 
      | DP_DONOTUPDATEEXTENTS | (clipFlags == clipNONE ? DP_DONOTCLIP : 0));

    Vid::SetBucketWorldTransform( &stateArray[group.stateIndex].WorldMatrix());
    Vid::SetBucketMaterial( group.material);
    Vid::SetBucketTexture( group.texture);

    U32 vcount = group.vertCount;
    U32 icount = group.indexCount;
		Vertex *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, vcount, &indexmem, icount))
    {
      return;
    }
    U32 j;
    for (j = 0; j < group.vertCount; j++)
    {
      Vertex &v = vertmem[j];
      v.vv = vertices[vertIndex + j];
      v.nv = normals[vertIndex + j];
      v.uv = uvs[vertIndex + j];
    }
    Utils::Memcpy( indexmem, &indices[indexIndex], group.indexCount * sizeof( U16));

    vertIndex  += group.vertCount;
    indexIndex += group.indexCount;

    Vid::UnlockIndexedPrimitiveMem( vcount, icount, TRUE, TRUE);
  }
#endif
}
//----------------------------------------------------------------------------

#endif


#if 0
void MeshRoot::RenderEnvMapAnim( Bucky *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags) // = clipALL)
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

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each bucky
    Bucky &bucky = _buckys[i];
    const MaterialDesc &desc = bucky.material->GetDesc();

    (*((Color *)&bucky.diffInitC)).Modulate( color, 
      (Vid::renderState.ambientR * desc.specular.r + desc.specular.r) * DxLight::Manager::darken,
      (Vid::renderState.ambientG * desc.specular.g + desc.specular.g) * DxLight::Manager::darken,
      (Vid::renderState.ambientB * desc.specular.b + desc.specular.b) * DxLight::Manager::darken);

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
    if (!Vid::LockBuckys( &_buckys[0], buckyCount, &stateArray))
    {
      return;
    }

    // backcull, transform and light
    Bool doCull = !(renderFlags & RS_2SIDED) && Vid::renderState.status.cull;
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj &face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      Bucky &bucky = _buckys[face.buckyIndex];

      if (!bucky.material->GetStatus().specular)
      {
        continue;
      }

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (doCull)
      {
        Plane plane;
        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
        if (plane.Dot( verts[ivi[0]]) >= 0.0f)
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
    Vid::UnLockBuckys( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
    Bool doCull = !(renderFlags & RS_2SIDED) && Vid::renderState.status.cull;
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj &face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      Bucky &bucky = _buckys[face.buckyIndex];

      if (!bucky.material->GetStatus().specular)
      {
        continue;
      }

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (doCull)
      {
        Plane plane;
        plane.Set( verts[ivi[0]], verts[ivi[1]], verts[ivi[2]]);
        if (plane.Dot( verts[ivi[0]]) >= 0.0f)
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
          dv.diffuse = bucky.diffInit;
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
      Bucky &bucky = _buckys[i];
      Vid::SetBucketTexture(  bucky.texture0);

//      cam.ClipToBucket( tbucket.verts, bucky.vCount, tbucket.indices, tbucket.indexCount);
      if (!cam.ClipToBucket( bucky, &stateArray))
      {
        LOG_WARN( ("Can't lock buckets for %s", name.str) );
      }
    }
  }

#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------

void MeshRoot::RenderEnvMapNoAnim( Bucky *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags) // = clipALL)
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

  Camera &cam = Vid::CurCamera();

  // clear indexer lists
  // setup buckys
  //
  U32 i;
  for (i = 0; i < buckyCount; i++)
  {
    memset( &ivA[i][0], 0xff, sizeof(U16) * vertCount);
    memset( &iuA[i][0], 0xfd, sizeof(U16) * textCount);

    // calc constant diffuse component for each bucky
    Bucky &bucky = _buckys[i];
    const MaterialDesc &desc = bucky.material->GetDesc();

    (*((Color *)&bucky.diffInitC)).Modulate( color, 
      (Vid::renderState.ambientR * desc.specular.r + desc.specular.r) * DxLight::Manager::darken,
      (Vid::renderState.ambientG * desc.specular.g + desc.specular.g) * DxLight::Manager::darken,
      (Vid::renderState.ambientB * desc.specular.b + desc.specular.b) * DxLight::Manager::darken);

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
    if (!Vid::LockBuckys( &_buckys[0], buckyCount, &stateArray))
    {
      LOG_WARN( ("Can't lock buckets for %s", name.str) );
      return;
    }

    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj &face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      Bucky &bucky = _buckys[face.buckyIndex];

      if (!bucky.material->GetStatus().specular)
      {
        continue;
      }

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (Vid::renderState.status.cull && !(bucky.flags & RS_2SIDED))
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
    Vid::UnLockBuckys( _buckys, buckyCount);
  }
  else
  {
    // backcull, transform and light
	  for (i = 0; i < faceCount; i++)
    {
      FaceObj &face = faces[i];
      ASSERT( face.buckyIndex < buckyCount);

      Bucky &bucky = _buckys[face.buckyIndex];
      if (!bucky.material->GetStatus().specular)
      {
        continue;
      }

	    U16 ivi[3];
      ivi[0] = face.verts[0];
      ivi[1] = face.verts[1];
      ivi[2] = face.verts[2];
      ASSERT( ivi[0] < vertCount && ivi[1] < vertCount && ivi[2] < vertCount);

      if (Vid::renderState.status.cull && !(bucky.flags & RS_2SIDED))
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
          dv.diffuse = bucky.diffInit;
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
      Bucky &bucky = _buckys[i];
      Vid::SetBucketTexture(  bucky.texture0);

      if (!cam.ClipToBucket( bucky, &stateArray))
//      if (!cam.ClipToBucket( tbucket.verts, bucky.vCount, tbucket.indices, tbucket.indexCount))
      {
        LOG_WARN( ("Can't lock buckets for %s", name.str) );
      }
    }
  }
#ifdef DOSTATISTICS
  Statistics::objectTris = Statistics::objectTris + Statistics::tempTris;
#endif
}
//----------------------------------------------------------------------------
#endif