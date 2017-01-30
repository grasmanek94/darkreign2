///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// camverts.cpp
//
// 18-JAN-1999      John Cooke
//

#include "vid.h"
#include "vidclip.h"
//-----------------------------------------------------------------------------

  // temp pools : FIXME : eliminate temp copying
static VertexTL  tmpV[MAXVERTS];
static U16       tmpI[MAXINDICES];
static U16       tmpVI[MAXVERTS];
static ColorValue specInit( 0.0f, 0.0f, 0.0f);

// return of NULL indicates that bucket memory couldn't be allocated
//
Bucket * Camera::ProjectClip( const Plane *srcP, const VertexL *srcV, U32 countV, const U16 *srcI, U32 countI, U32 clipFlags)
{
  ASSERT( countI <= MAXINDICES);

  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // backcull; light and set homogeneous for valid verts

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i0].vv;
        dv.uv = srcV[i0].uv;
        dv.diffuse  = srcV[i0].diffuse;
        dv.specular = srcV[i0].specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i1].vv;
        dv.uv = srcV[i1].uv;
        dv.diffuse  = srcV[i1].diffuse;
        dv.specular = srcV[i1].specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i2].vv;
        dv.uv = srcV[i2].uv;
        dv.diffuse  = srcV[i2].diffuse;
        dv.specular = srcV[i2].specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i2];
		}
	}

  if (!countV)
  {
    // no forward faces
    return FALSE;
  }

  return Vid::Clip::ToBucket( tmpV, countV, tmpI, countI, (void *) 0xffffffff, clipFlags);
}
//----------------------------------------------------------------------------

Bucket * Camera::ProjectClipBias( const Plane *srcP, const VertexL *srcV, U32 countV, const U16 *srcI, U32 countI, U32 clipFlags)
{
  ASSERT( countI <= MAXINDICES);

  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // backcull; light and set homogeneous for valid verts

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i0].vv;
        dv.uv = srcV[i0].uv;
        dv.diffuse  = srcV[i0].diffuse;
        dv.specular = srcV[i0].specular;

        TransformFromModelSpace( dv);

        dv.vv.z *= Vid::renderState.zBias;

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i1].vv;
        dv.uv = srcV[i1].uv;
        dv.diffuse  = srcV[i1].diffuse;
        dv.specular = srcV[i1].specular;

        TransformFromModelSpace( dv);

        dv.vv.z *= Vid::renderState.zBias;

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i2].vv;
        dv.uv = srcV[i2].uv;
        dv.diffuse  = srcV[i2].diffuse;
        dv.specular = srcV[i2].specular;

        TransformFromModelSpace( dv);

        dv.vv.z *= Vid::renderState.zBias;

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i2];
		}
	}

  if (!countV)
  {
    // no forward faces
    return NULL;
  }

  return Vid::Clip::ToBucket( tmpV, countV, tmpI, countI, (void *) 0xffffffff, clipFlags);
}
//----------------------------------------------------------------------------

Bucket * Camera::ProjectNoClip( const Plane *srcP, const VertexL *srcV, U32 countV, const U16 *srcI, U32 countI)
{
  if (countI == 0)
  {
    return NULL;
  }
  ASSERT( countI <= MAXINDICES);

	VertexTL * vertmem;
  U16 *      indexmem;
  Bucket * bucket = Vid::LockIndexedPrimitiveMem( (void **)&vertmem, countV, &indexmem, countI);
  if (!bucket)
  {
    return NULL;
  }

  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // backcull; light and set homogeneous for valid verts

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = vertmem[countV];
//        dv.vv = srcV[i0].vv;
        dv.uv = srcV[i0].uv;
        dv.diffuse  = srcV[i0].diffuse;
        dv.specular = srcV[i0].specular;

        // project
        TransformProjectFromModelSpace( dv, srcV[i0].vv);

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = vertmem[countV];
//        dv.vv = srcV[i1].vv;
        dv.uv = srcV[i1].uv;
        dv.diffuse  = srcV[i1].diffuse;
        dv.specular = srcV[i1].specular;

        // project
        TransformProjectFromModelSpace( dv, srcV[i1].vv);

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = vertmem[countV];
//        dv.vv = srcV[i2].vv;
        dv.uv = srcV[i2].uv;
        dv.diffuse  = srcV[i2].diffuse;
        dv.specular = srcV[i2].specular;

        // project
        TransformProjectFromModelSpace( dv, srcV[i2].vv);

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i2];
		}
	}

  Vid::UnlockIndexedPrimitiveMem( countV, countI);

  return bucket;
}
//----------------------------------------------------------------------------

Bucket * Camera::ProjectNoClipBias( const Plane *srcP, const VertexL *srcV, U32 countV, const U16 *srcI, U32 countI)
{
  if (countI == 0)
  {
    return NULL;
  }
  ASSERT( countI <= MAXINDICES);

	VertexTL * vertmem;
  U16 *      indexmem;
  Bucket * bucket = Vid::LockIndexedPrimitiveMem( (void **)&vertmem, countV, &indexmem, countI);
  if (!bucket)
  {
    return NULL;
  }

  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // backcull; light and set homogeneous for valid verts

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = vertmem[countV];
//        dv.vv = srcV[i0].vv;
        dv.uv = srcV[i0].uv;
        dv.diffuse  = srcV[i0].diffuse;
        dv.specular = srcV[i0].specular;

        // project
        TransformProjectFromModelSpace( dv, srcV[i0].vv);

        dv.vv.z *= Vid::renderState.zBias;

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = vertmem[countV];
//        dv.vv = srcV[i1].vv;
        dv.uv = srcV[i1].uv;
        dv.diffuse  = srcV[i1].diffuse;
        dv.specular = srcV[i1].specular;

        // project
        TransformProjectFromModelSpace( dv, srcV[i1].vv);

        dv.vv.z *= Vid::renderState.zBias;

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = vertmem[countV];
//        dv.vv = srcV[i2].vv;
        dv.uv = srcV[i2].uv;
        dv.diffuse  = srcV[i2].diffuse;
        dv.specular = srcV[i2].specular;

        // project
        TransformProjectFromModelSpace( dv, srcV[i2].vv);

        dv.vv.z *= Vid::renderState.zBias;

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i2];
		}
	}

  Vid::UnlockIndexedPrimitiveMem( countV, countI);

  return bucket;
}
//----------------------------------------------------------------------------

// return of NULL indicates that bucket memory couldn't be allocated
//
Bucket * Camera::ProjectClip( const Plane *srcP, const Vector *srcV, const UVPair *srcUV, Color diffuse, Color specular, U32 countV, const U16 *srcI, U32 countI, U32 clipFlags)
{
  ASSERT( countI <= MAXINDICES);

  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // backcull; set homogeneous for valid verts

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i0];
        if (srcUV)
        {
          dv.uv = srcUV[i0];
        }
        dv.diffuse  = diffuse;
        dv.specular = specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i1];
        if (srcUV)
        {
          dv.uv = srcUV[i1];
        }
        dv.diffuse  = diffuse;
        dv.specular = specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcV[i2];
        if (srcUV)
        {
          dv.uv = srcUV[i2];  
        }
        dv.diffuse  = diffuse;
        dv.specular = specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i2];
		}
	}

  if (!countV)
  {
    // no forward faces
    return NULL;
  }

  return Vid::Clip::ToBucket( tmpV, countV, tmpI, countI, (void *) 0xffffffff, clipFlags);
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::Light( VertexTL *dstV, U16 *dstI, const Plane *srcP, const Vector *srcVect, const Vector *srcNorm, const UVPair *srcUV, U32 &countV, const U16 *srcI, U32 &countI)
{
  ASSERT( countI <= MAXINDICES);

  Material &material = *DxLight::Manager::curMaterial;
  U32 diffa = Utils::FtoL( material.GetDesc().diffuse.a  * 255.0f);

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  // backcull; light and project for valid verts
  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = dstV[countV];
        dv.vv = srcVect[i0];
        dv.uv = srcUV[i0];

        // calculate lighting
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i0], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = dstV[countV];
        dv.vv = srcVect[i1];
        dv.uv = srcUV[i1];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i1], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = dstV[countV];
        dv.vv = srcVect[i2];
        dv.uv = srcUV[i2];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i2], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i2];
		}
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightProject( VertexTL *dstV, U16 *dstI, const Plane *srcP, const Vector *srcVect, const Vector *srcNorm, const UVPair *srcUV, U32 &countV, const U16 *srcI, U32 &countI, U32 clipFlags)
{
  clipFlags;

  ASSERT( countI <= MAXINDICES);

  Material &material = *DxLight::Manager::curMaterial;
  U32 diffa = Utils::FtoL( material.GetDesc().diffuse.a  * 255.0f);

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  // backcull; light and project for valid verts
  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = dstV[countV];
        dv.vv = srcVect[i0];
        dv.uv = srcUV[i0];

        // calculate lighting
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i0], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = dstV[countV];
        dv.vv = srcVect[i1];
        dv.uv = srcUV[i1];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i1], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = dstV[countV];
        dv.vv = srcVect[i2];
        dv.uv = srcUV[i2];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i2], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i2];
		}
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::ProjectClip( VertexTL *dstV, U16 *dstI, const Plane *srcP, const VertexTL *srcV, U32 &countV, const U16 *srcI, U32 &countI, Bool calcFog, U32 clipFlags) //  = FALSE)
{
  ASSERT( countI <= MAXINDICES);

  // backcull; light and set homogeneous for valid verts
  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = tmpV[countV];
        const VertexTL &sv = srcV[i0];
        dv.vv = sv.vv;
        dv.uv = sv.uv;

        dv.diffuse  = sv.diffuse;
        dv.specular = sv.specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = tmpV[countV];
        const VertexTL &sv = srcV[i1];
        dv.vv = sv.vv;
        dv.uv = sv.uv;

        dv.diffuse  = sv.diffuse;
        dv.specular = sv.specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = tmpV[countV];
        const VertexTL &sv = srcV[i2];
        dv.vv = sv.vv;
        dv.uv = sv.uv;

        dv.diffuse  = sv.diffuse;
        dv.specular = sv.specular;

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i2];
		}
	}
  Vid::Clip::ToBuffer( dstV, dstI, tmpV, countV, tmpI, countI, calcFog, clipFlags);
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::ProjectNoClip( VertexTL *dstV, U16 *dstI, const Plane *srcP, const VertexTL *srcV, U32 &countV, const U16 *srcI, U32 &countI, Bool calcFog) //  = FALSE)
{
  ASSERT( countI <= MAXINDICES);

  // backcull; light and set homogeneous for valid verts
  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = dstV[countV];
        const VertexTL &sv = srcV[i0];
        dv.uv = sv.uv;

        TransformProjectFromModelSpace( dv, sv.vv);

        dv.diffuse  = sv.diffuse;
        dv.specular = sv.specular;

        if (calcFog)
        {
          // set vertex fog
          dv.SetFog();
        }

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = dstV[countV];
        const VertexTL &sv = srcV[i1];
        dv.uv = sv.uv;

        TransformProjectFromModelSpace( dv, sv.vv);

        dv.diffuse  = sv.diffuse;
        dv.specular = sv.specular;

        if (calcFog)
        {
          // set vertex fog
          dv.SetFog();
        }

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = dstV[countV];
        const VertexTL &sv = srcV[i2];
        dv.uv = sv.uv;

        TransformProjectFromModelSpace( dv, sv.vv);

        dv.diffuse  = sv.diffuse;
        dv.specular = sv.specular;

        if (calcFog)
        {
          // set vertex fog
          dv.SetFog();
        }

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      dstI[countI++] = tmpVI[i2];
		}
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightTransformFromModelSpace( VertexTL *dstV, const Vector *srcVect, const Vector *srcNorm, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;
  U32 diffa = Utils::FtoL( material.GetDesc().diffuse.a  * 255.0f);

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL & dv = dstV[i];
    const Vector & sv = srcVect[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( sv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
    dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    TransformFromModelSpace( dv, sv);
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightTransformFromModelSpace( VertexTL *dstV, const Vector *srcVect, const Vector *srcNorm, const Color *srcC, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL &dv = dstV[i];
    const Vector &sv = srcVect[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( sv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
    dv.diffuse.ModulateInline( srcC[i], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    TransformFromModelSpace( dv, sv);
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightTransformFromModelSpace( VertexTL *dstV, const Vector *srcNorm, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL &dv = dstV[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
//    dv.diffuse.ModulateInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.diffuse.SetInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    TransformFromModelSpace( dv);
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightSetHomogeneousVerts( VertexTL *dstV, const Vector *srcVect, const Vector *srcNorm, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;
  U32 diffa = Utils::FtoL( material.GetDesc().diffuse.a  * 255.0f);

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL & dv = dstV[i];
    const Vector & sv = srcVect[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( sv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
    dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    // start projection
    SetHomogeneousFromModelSpace( dv, sv);
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightSetHomogeneousVerts( VertexTL *dstV, const Vector *srcVect, const Vector *srcNorm, const Color *srcC, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL &dv = dstV[i];
    const Vector &sv = srcVect[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( sv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
    dv.diffuse.ModulateInline( srcC[i], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    // start projection
    SetHomogeneousFromModelSpace( dv, sv);
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightSetHomogeneousVerts( VertexTL *dstV, const Vector *srcNorm, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL &dv = dstV[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
//    dv.diffuse.ModulateInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.diffuse.SetInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    // start projection
    SetHomogeneousFromModelSpace( dv);
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightProjectVerts( VertexTL *dstV, const Vector *srcVect, const Vector *srcNorm, const Color *srcC, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL &dv = dstV[i];
    const Vector &sv = srcVect[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( sv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
    dv.diffuse.ModulateInline( srcC[i], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    //TransformProjectFromModelSpace( dv, sv);
    TransformFromModelSpace( dv, sv);
    SetHomogeneousFromCameraSpace( dv);
    ProjectFromHomogeneous( dv);

    dv.SetFog();
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightProjectVerts( VertexTL *dstV, const Vector *srcVect, const Vector *srcNorm, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL &dv = dstV[i];
    const Vector &sv = srcVect[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( sv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
//    dv.diffuse.ModulateInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.diffuse.SetInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    //TransformProjectFromModelSpace( dv, sv);
    TransformFromModelSpace( dv, sv);
    SetHomogeneousFromCameraSpace( dv);
    ProjectFromHomogeneous( dv);

    dv.SetFog();
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::LightProjectVerts( VertexTL *dstV, const Vector *srcNorm, U32 countV)
{
  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  U32 i;
  for (i = 0; i < countV; i++)
  {
    // copy
    VertexTL &dv = dstV[i];

    // calculate lighting
    diff = diffInit;
    spec = specInit;
    DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i], material.Diffuse(), material, diff, spec);

    // set the colors
//    dv.diffuse.ModulateInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.diffuse.SetInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
    dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

    //TransformProjectFromModelSpace( dv);
    TransformFromModelSpace( dv);
    SetHomogeneousFromCameraSpace( dv);
    ProjectFromHomogeneous( dv);

    dv.SetFog();
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::ProjectVerts( VertexTL *dstV, const Vector *srcVect, U32 countV)
{
  U32 i;
  for (i = 0; i < countV; i++)
  {
    VertexTL &dv = dstV[i];
    const Vector &sv = srcVect[i];

    TransformProjectFromModelSpace( dv, sv);
	}
}
//----------------------------------------------------------------------------

// to buffer
//
void Camera::ProjectVerts( VertexTL *dstV, U32 countV)
{
  U32 i;
  for (i = 0; i < countV; i++)
  {
    VertexTL &dv = dstV[i];

    TransformProjectFromModelSpace( dv);
	}
}
//----------------------------------------------------------------------------

#if 0

// return of NULL indicates that bucket memory couldn't be allocated
//
Bucket * Camera::LightProjectClip( const Plane *srcP, const Vector *srcVect, const Vector *srcNorm, const UVPair *srcUV, U32 countV, const U16 *srcI, U32 countI)
{
  ASSERT( countI <= MAXINDICES);

  Material &material = *DxLight::Manager::curMaterial;
  U32 diffa = Utils::FtoL( material.GetDesc().diffuse.a  * 255.0f);

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  // backcull; light and project for valid verts
  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcVect[i0];
        dv.uv = srcUV[i0];

        // calculate lighting
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i0], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcVect[i1];
        dv.uv = srcUV[i1];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i1], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcVect[i2];
        dv.uv = srcUV[i2];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i2], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i2];
		}
	}

  if (!countV)
  {
    // no forward faces
    return NULL;
  }

  return Vid::Clip::ToBucket( tmpV, countV, tmpI, countI);
}
//----------------------------------------------------------------------------

// return of NULL indicates that bucket memory couldn't be allocated
//
Bucket * Camera::LightProjectNoClip( const Plane *srcP, const Vector *srcVect, const Vector *srcNorm, const UVPair *srcUV, U32 countV, const U16 *srcI, U32 countI)
{
  ASSERT( countI <= MAXINDICES);

	VertexTL * vertmem;
  U16 *      indexmem;
  Bucket * bucket = Vid::LockIndexedPrimitiveMem( (void **)&vertmem, countV, &indexmem, countI);
  if (!bucket)
  {
    return NULL;
  }

  Material &material = *DxLight::Manager::curMaterial;
  U32 diffa = Utils::FtoL( material.GetDesc().diffuse.a  * 255.0f);

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;
  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  // backcull; light and project for valid verts
  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = vertmem[countV];
        dv.uv = srcUV[i0];

        // project
        const Vector &sv = srcVect[i0];
        TransformProjectFromModelSpace( dv, sv);

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( sv, srcNorm[i0], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = vertmem[countV];
        dv.uv = srcUV[i1];

        // project
        const Vector &sv = srcVect[i1];
        TransformProjectFromModelSpace( dv, sv);

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( sv, srcNorm[i1], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = vertmem[countV];
        dv.uv = srcUV[i2];

        // project
        const Vector &sv = srcVect[i2];
        TransformProjectFromModelSpace( dv, sv);

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( sv, srcNorm[i2], material.Diffuse(), material, diff, spec);

        // set the colors
        dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i2];
		}
	}

  Vid::UnlockIndexedPrimitiveMem( countV, countI);

  return bucket;
}
//----------------------------------------------------------------------------

// return of NULL indicates that bucket memory couldn't be allocated
//
Bucket * Camera::LightProjectClip( const Plane *srcP, const Vector *srcVect, const Vector *srcNorm, const UVPair *srcUV, const Color *srcC, U32 countV, const U16 *srcI, U32 countI)
{
  ASSERT( countI <= MAXINDICES);

  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  // backcull; light and set homogeneous for valid verts
  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcVect[i0];
        dv.uv = srcUV[i0];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i0], material.Diffuse(), material, diff, spec);

        dv.diffuse.ModulateInline( srcC[i0], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255); 

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcVect[i1];
        dv.uv = srcUV[i1];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i1], material.Diffuse(), material, diff, spec);

        dv.diffuse.ModulateInline( srcC[i1], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255); 

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = tmpV[countV];
        dv.vv = srcVect[i2];
        dv.uv = srcUV[i2];

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( dv.vv, srcNorm[i2], material.Diffuse(), material, diff, spec);

        dv.diffuse.ModulateInline( srcC[i2], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255); 

        TransformFromModelSpace( dv);

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      tmpI[countI++] = tmpVI[i2];
		}
	}

  if (!countV)
  {
    // no forward faces
    return FALSE;
  }

  return Vid::Clip::ToBucket( tmpV, countV, tmpI, countI);
}
//----------------------------------------------------------------------------

// return of NULL indicates that bucket memory couldn't be allocated
//
Bucket * Camera::LightProjectNoClip( const Plane *srcP, const Vector *srcVect, const Vector *srcNorm, const UVPair *srcUV, const Color *srcC, U32 countV, const U16 *srcI, U32 countI)
{
  ASSERT( countI <= MAXINDICES);

	VertexTL * vertmem;
  U16 *      indexmem;
  Bucket * bucket = Vid::LockIndexedPrimitiveMem( (void **)&vertmem, countV, &indexmem, countI);
  if (!bucket)
  {
    return NULL;
  }

  Material &material = *DxLight::Manager::curMaterial;

  // calculate the parts of the diffuse color that are the same for all output vertexes
  ColorValue diffInit, diff, spec;

  diffInit.r = material.GetDesc().diffuse.r * Vid::renderState.ambientR;
  diffInit.g = material.GetDesc().diffuse.g * Vid::renderState.ambientG;
  diffInit.b = material.GetDesc().diffuse.b * Vid::renderState.ambientB;
  diffInit.a = material.GetDesc().diffuse.a;

  // backcull; light and project for valid verts
  Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

  // clear out indexer
	memset( tmpVI, 0xff, countV * sizeof( U16));

  U16 fi;
  const U16 *si, *se = srcI + countI;
  countV = countI = 0;
	for (si = srcI, fi = 0; si < se; si += 3, fi++)
  {
	  U16 i0 = si[0];
    U16 i1 = si[1];
    U16 i2 = si[2];

    if (nobackcull || srcP[fi].Evalue( Vid::model_view_vector) >= 0.0f)
    {
      // forward face

      if (tmpVI[i0] == 0xffff)
      {
        // do vert 0

        // copy
        VertexTL &dv = vertmem[countV];
        dv.uv = srcUV[i0];

        // project
        const Vector &sv = srcVect[i0];
        TransformProjectFromModelSpace( dv, sv);

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( sv, srcNorm[i0], material.Diffuse(), material, diff, spec);

        dv.diffuse.ModulateInline( srcC[i0], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255); 

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i0] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i0];

      if (tmpVI[i1] == 0xffff)
      {
        // do vert 1
        
        // copy
        VertexTL &dv = vertmem[countV];
        dv.uv = srcUV[i1];

        // project
        const Vector &sv = srcVect[i1];
        TransformProjectFromModelSpace( dv, sv);

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( sv, srcNorm[i1], material.Diffuse(), material, diff, spec);

        dv.diffuse.ModulateInline( srcC[i1], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255); 
        dv.SetFog();

        // set indexer
        tmpVI[i1] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i1];

      if (tmpVI[i2] == 0xffff)
      {
        // do vert 2

        // copy
        VertexTL &dv = vertmem[countV];
        dv.uv = srcUV[i2];

        // project
        const Vector &sv = srcVect[i2];
        TransformProjectFromModelSpace( dv, sv);

        // light
        diff = diffInit;
        spec = specInit;
        DxLight::Manager::LightModelSpace( sv, srcNorm[i2], material.Diffuse(), material, diff, spec);

        dv.diffuse.ModulateInline( srcC[i2], diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
        dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255); 

        // set vertex fog
        dv.SetFog();

        // set indexer
        tmpVI[i2] = (U16)countV;
        countV++;
      }
      indexmem[countI++] = tmpVI[i2];
		}
	}

  Vid::UnlockIndexedPrimitiveMem( countV, countI);

  return bucket;
}
//----------------------------------------------------------------------------

#endif