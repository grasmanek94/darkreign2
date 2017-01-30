///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_math.cpp
//
// 12-FEB-2000
//

#include "vid_public.h"
#include "light_priv.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Math
  {
    // transform matrices
    Matrix worldMatrix     = Matrix::I;
    Matrix worldMatrixInv  = Matrix::I;
    Matrix cameraMatrix    = Matrix::I;
    Matrix viewMatrix      = Matrix::I;
    Matrix transformMatrix = Matrix::I;
    Matrix projMatrix      = Matrix::I;

    Vector modelViewVector;
    Vector modelViewNorm;

    // projection math
    //
    F32 nearPlane, farPlane, invDepth;
    Point<F32> origin;
    Point<F32> size;
    Point<F32> projInv;
  };
  //----------------------------------------------------------------------------

  // calculate the camera coord position corresponding of the cursor on the nearplane
  // return it in 'pos'
  // sx and sy are in view coords
  //
  void ScreenToCamera( Vector & dst, S32 sx, S32 sy)
  {
  //  dst.z = NearPlane();
    dst.z = 1.0f;
    dst.x = (F32) (sx - Math::origin.x) * dst.z * Math::projInv.x;
    dst.y = (F32) (sy - Math::origin.y) * dst.z * Math::projInv.y;
  }
  //-----------------------------------------------------------------------------

  void ScreenToWorld( Vector & dst, S32 sx, S32 sy)
  {
    ScreenToCamera( dst, sx, sy);
    CurCamera().WorldMatrix().Transform( dst);
  }
  //-----------------------------------------------------------------------------

  F32 Project( F32 y, F32 z)
  {
    return -y * Math::projMatrix.up.y * Math::size.y * RHW(z);
  }
  //-----------------------------------------------------------------------------

  F32 ProjectInv( F32 sy, F32 z)
  {
    return sy * z * Math::projInv.y;
  }
  //-----------------------------------------------------------------------------

  Bool BackCullCamera( const Vector & v0, const Vector & v1, const Vector & v2)
  {
    return BackCull( Plane( v0, v1, v2).Dot( v0));
  }
  //-----------------------------------------------------------------------------

  Bool BackCullCamera( const Vector & v0, const Vector & v1, const Vector & v2, const Vector & v3)
  {
    return BackCull( Plane( v0, v1, v2).Dot( v0))
      &&   BackCull( Plane( v0, v2, v3).Dot( v0));
  }
  //-----------------------------------------------------------------------------

  Bool BackCullWorld( const Vector & v0, const Vector & v1, const Vector & v2)
  {
	  return Plane( v0, v1, v2).Evalue( Vid::Math::modelViewVector) <= 0.0f;
  }
  //----------------------------------------------------------------------------

  Bool BackCullWorld( const Vector & v0, const Vector & v1, const Vector & v2, const Vector & v3)
  {
	  return Plane( v0, v1, v2).Evalue( Vid::Math::modelViewVector) <= 0.0f
	    &&   Plane( v0, v2, v3).Evalue( Vid::Math::modelViewVector) <= 0.0f;
  }
  //----------------------------------------------------------------------------

  void SetModelViewVector( const Vector & worldPos)
  {
	  // move the model to camera vector to model space
    // for backface culling and specular lighting
    //
    Math::modelViewVector = Math::cameraMatrix.posit - worldPos;
	  Math::worldMatrix.RotateInv( Math::modelViewVector);
    Math::modelViewNorm = Math::modelViewVector;
	  Math::modelViewNorm.Normalize();
  }
  //----------------------------------------------------------------------------

  void SetWorldTransform( const Matrix & mat )
  {
	  Math::worldMatrix = mat;
    Math::worldMatrixInv.SetInverse( mat);
    Math::transformMatrix = Math::worldMatrix * Math::viewMatrix;

    SetModelViewVector( Math::worldMatrix.posit);
//	  SetWorldTransform_D3D(mat);
  }

  void SetWorldTransform_D3D( const Matrix &mat )
  {
	  dxError = device->SetTransform( D3DTRANSFORMSTATE_WORLD, (D3DMATRIX*) &mat );
	  LOG_DXERR( ("device->SetTransform: world") );
  }
  //----------------------------------------------------------------------------

  void SetCameraTransform( const Matrix & mat )
  {
    Math::cameraMatrix = mat;
    Math::viewMatrix.SetInverse( mat);

#ifndef DODXLEANANDGRUMPY
    if (renderState.status.dxTL)
    {
      SetViewTransform_D3D( Math::viewMatrix);
    }
#endif
  }
  //----------------------------------------------------------------------------

  void SetViewTransform_D3D( const Matrix & mat )
  {
	  dxError = device->SetTransform( D3DTRANSFORMSTATE_VIEW, (D3DMATRIX*) &mat );
	  LOG_DXERR( ("device->SetTransform: view.") );
  }
  //----------------------------------------------------------------------------

  void SetProjTransform( const Matrix & mat)
  {
#ifndef DODXLEANANDGRUMPY
    Vid::SetProjTransform_D3D( mat);
#endif

    Math::projMatrix = mat;

    // screen y coords are opposite of world y coords
    //
    Math::projMatrix.up.y *= -1;

    // reset fog values
    //
    SetFogRange();
  }

  void SetProjTransform_D3D( const Matrix & mat )
  {
	  dxError = device->SetTransform( D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX*) &mat );
	  LOG_DXERR( ("device->SetTransform: project.") );
  }
  //----------------------------------------------------------------------------

  void Setup( const Camera & camera)
  {
    Math::nearPlane  = camera.NearPlane();
    Math::farPlane   = camera.FarPlane();
    Math::invDepth = camera.InvDepth();

    Math::origin = camera.Origin();
    Math::size.Set( camera.HalfWidth(), camera.HalfHeight());
    Math::projInv.Set( camera.ProjInvConstX(), camera.ProjInvConstY());

    SetCameraTransform( camera.WorldMatrix());
    SetProjTransform(   camera.ProjMatrix());
  }
  //-----------------------------------------------------------------------------

  void TransformFromModel( VertexTL * dst, U32 count)
  {
    for (VertexTL * e = dst + count; dst < e; dst++)
    {
      TransformFromModel( *dst);
    }
  }
  //-----------------------------------------------------------------------------

  void TransformFromModel( VertexTL * dst, U32 count, Color color)
  {
    for (VertexTL * e = dst + count; dst < e; dst++)
    {
      dst->diffuse  = color;
      dst->specular = 0xff000000;

      TransformFromModel( *dst);
    }
  }
  //-----------------------------------------------------------------------------

  void TransformFromModel( Vector * dst, const Vector * src, U32 count)
  {
    for (Vector * e = dst + count; dst < e; dst++, src++)
    {
      TransformFromModel( *dst, *src);
    }
  }
  //-----------------------------------------------------------------------------

  void TransformFromModel( VertexTL * dst, const Vector * src, U32 count)
  {
    for (VertexTL * e = dst + count; dst < e; dst++, src++)
    {
      TransformFromModel( *dst, *src);
    }
  }
  //-----------------------------------------------------------------------------

  void TransformFromModel( VertexTL * dst, const Vector * src, U32 count, Color color)
  {
    for (VertexTL * e = dst + count; dst < e; dst++, src++)
    {
      dst->diffuse  = color;
      dst->specular = 0xff000000;

      TransformFromModel( *dst, *src);
    }
  }
  //----------------------------------------------------------------------------

  // return rhw
  //
  F32 ProjectFromHomogeneous( Vector & dst, F32 w, const Vector & src)
  {
    return ProjectFromHomogeneous_I( dst, w, src);
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromHomogeneous( VertexTL & dst, const VertexTL & src)
  {
    return ProjectFromHomogeneous_I( dst, src);
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromHomogeneous( Vector & dst, F32 w)
  {
    return ProjectFromHomogeneous_I( dst, w);
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromHomogeneous( VertexTL & dst)
  {
    return ProjectFromHomogeneous_I( dst);
  }
  //----------------------------------------------------------------------------

  // return rhw
  //
  F32 ProjectFromModel( Vector & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromModel_I( dst, src);
    return ProjectFromHomogeneous_I( dst, w);
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromModel( Vector & dst)
  {
    F32 w = SetHomogeneousFromModel_I( dst);
    return ProjectFromHomogeneous_I( dst, w);
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromModel( VertexTL & dst)
  {
    SetHomogeneousFromModel_I( dst);
    return ProjectFromHomogeneous_I( dst);
  }
  //----------------------------------------------------------------------------

  void ProjectFromModel( VertexTL * dst, U32 count)
  {
    for (VertexTL * e = dst + count; dst < e; dst++)
    {
      ProjectFromModel_I( *dst);
    }
  }
  //----------------------------------------------------------------------------

  void ProjectFromModel( VertexTL * dst, const Vector * src, U32 count)
  {
    for (VertexTL * e = dst + count; dst < e; dst++, src++)
    {
      ProjectFromModel_I( *dst, *src);
    }
  }
  //----------------------------------------------------------------------------

  void ProjectFromModel( VertexTL * dst, const Vector * src, U32 count, Color color)
  {
    for (VertexTL * e = dst + count; dst < e; dst++, src++)
    {
      dst->diffuse  = color;
      dst->specular = 0xff000000;

      ProjectFromModel_I( *dst, *src);
    }
  }
  //----------------------------------------------------------------------------

  // return rhw
  //
  F32 ProjectFromCamera( Vector & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromCamera_I( dst, src);
    return ProjectFromHomogeneous_I( dst, w);
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromCamera( Vector & dst)
  {
    F32 w = SetHomogeneousFromCamera_I( dst, dst);
    return ProjectFromHomogeneous_I( dst, w);
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromCamera( VertexTL & dst)
  {
    SetHomogeneousFromCamera_I( dst);
    return ProjectFromHomogeneous_I( dst);
  }
  //----------------------------------------------------------------------------

  void ProjectFromCamera( VertexTL * dst, U32 count)
  {
    for (VertexTL * e = dst + count; dst < e; dst++)
    {
      ProjectFromCamera_I( *dst);
    }
  }
  //----------------------------------------------------------------------------

  void ProjectFromCamera( VertexTL * dst, U32 count, Color color)
  {
    for (VertexTL * e = dst + count; dst < e; dst++)
    {
      dst->diffuse  = color;
      dst->specular = 0xff000000;

      ProjectFromCamera_I( *dst);
    }
  }
  //----------------------------------------------------------------------------

  // return camera z
  //
  F32 ProjectFromWorld( Vector & dst, const Vector & src, F32 & rhw)
  {
    TransformFromWorld( dst, src);

    F32 camz = dst.z;

    rhw = SetHomogeneousFromCamera_I( dst);
    rhw = ProjectFromHomogeneous_I( dst, rhw);

    return camz;
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromWorld( Vector & dst, F32 & rhw)
  {
    TransformFromWorld( dst);

    F32 camz = dst.z;

    rhw = SetHomogeneousFromCamera_I( dst);
    rhw = ProjectFromHomogeneous_I( dst, rhw);

    return camz;
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromWorld( VertexTL & dst, const Vector & src)
  {
    TransformFromWorld( dst.vv, src);

    F32 camz = dst.vv.z;

    dst.rhw = SetHomogeneousFromCamera_I( dst);
    dst.rhw = ProjectFromHomogeneous_I( dst);

    return camz;
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromWorld( VertexTL & dst)
  {
    TransformFromWorld( dst.vv);

    F32 camz = dst.vv.z;

    dst.rhw = SetHomogeneousFromCamera_I( dst);
    dst.rhw = ProjectFromHomogeneous_I( dst);

    return camz;
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromWorld( Vector & dst, const Vector & src, F32 & rhw, const Matrix & world)
  {
    world.Transform( dst, src);
    TransformFromWorld( dst);

    F32 camz = dst.z;

    rhw = SetHomogeneousFromCamera_I( dst);
    rhw = ProjectFromHomogeneous_I( dst, rhw);

    return camz;
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromWorld( Vector & dst, F32 & rhw, const Matrix & world)
  {
    world.Transform( dst);
    TransformFromWorld( dst);

    F32 camz = dst.z;

    rhw = SetHomogeneousFromCamera_I( dst);
    rhw = ProjectFromHomogeneous_I( dst, rhw);

    return camz;
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromWorld( VertexTL & dst, const Vector & src, const Matrix & world)
  {
    world.Transform( dst.vv, src);
    TransformFromWorld( dst.vv);

    F32 camz = dst.vv.z;

    dst.rhw = SetHomogeneousFromCamera_I( dst);
    dst.rhw = ProjectFromHomogeneous_I( dst);

    return camz;
  }
  //----------------------------------------------------------------------------

  F32 ProjectFromWorld( VertexTL & dst, const Matrix & world)
  {
    world.Transform( dst.vv);
    TransformFromWorld( dst.vv);

    F32 camz = dst.vv.z;

    dst.rhw = SetHomogeneousFromCamera_I( dst);
    dst.rhw = ProjectFromHomogeneous_I( dst);

    return camz;
  }
  //----------------------------------------------------------------------------

  void ProjectIsoFromCamera( Vector & dst, F32 & drhw, const Vector & src)
  {
    drhw = 1.0f;
    dst.x =  src.x * Math::projMatrix.right.x * Math::size.x  + Math::origin.x;
    dst.y =  src.y * Math::projMatrix.up.y    * Math::size.y + Math::origin.y;
    dst.z = (src.z - Math::nearPlane) * Math::invDepth;
  }
  //----------------------------------------------------------------------------

  // return of NULL indicates that bucket memory couldn't be allocated
  //
  Bucket * ProjectClip( VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog, U32 clipFlags) // = TRUE, = clipALL
  {
    // set camera verts
    TransformFromModel( srcV, countV);

    return Clip::ToBucket( srcV, countV, srcI, countI, (void *) 0xffffffff, calcFog, clipFlags);
  }
  //----------------------------------------------------------------------------

  // return of NULL indicates that bucket memory couldn't be allocated
  //
  Bucket * ProjectClip( const Vector * srcV, const UVPair * srcUV, Color diffuse, Color specular, U32 countV, const U16 * srcI, U32 countI, Bool calcFog, U32 clipFlags) // = TRUE, = clipALL
  { 
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

    // set camera verts
    VertexTL * dstV;
    U32 heapSize = Vid::Heap::ReqVertex( &dstV, countV);
    VertexTL * dv = dstV;
    const UVPair * suv = srcUV;
	  for ( const Vector * sv = srcV, * ev = srcV + countV; sv < ev; sv++, dv++)
    {
      if (suv)
      {
        dv->uv = *suv++;
      }
      dv->diffuse  = diffuse;
      dv->specular = specular;

      TransformFromModel( *dv, *sv);
    }

    Bucket * bucket = Clip::ToBucket( dstV, sv - srcV, srcI, countI, (void *) 0xffffffff, calcFog, clipFlags);

    Vid::Heap::Restore( heapSize);

    return bucket;
  }
  //----------------------------------------------------------------------------

  // return of NULL indicates that bucket memory couldn't be allocated
  //
  Bucket * ProjectNoClip( const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog) // = TRUE)
  {
    if (countI == 0)
    {
      return NULL;
    }
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

    // set screen verts directly to bucket
    //
	  VertexTL * vertmem;
    U16 * indexmem;
    Bucket * bucket = LockIndexedPrimitiveMem( (void **)&vertmem, countV, &indexmem, countI);
    if (!bucket)
    {
      return NULL;
    }

    VertexTL * dv = vertmem;
	  for ( const VertexTL * sv = srcV, * ev = srcV + countV; sv < ev; sv++, dv++)
    {
      dv->uv = sv->uv;
      dv->diffuse  = sv->diffuse;
      dv->specular = sv->specular;

      ProjectFromModel_I( *dv, *sv);

      if (calcFog)
      {
        dv->SetFog();
      }
    }

    // set indices
    //
    Utils::Memcpy( indexmem, srcI, countI << 1);

    UnlockIndexedPrimitiveMem( sv - srcV, countI);

    return bucket;
  }
  //----------------------------------------------------------------------------

  // non-indexed tris
  //
  // return of NULL indicates that bucket memory couldn't be allocated
  //
  Bucket * ProjectNoClip( const VertexTL * srcV, U32 countV, Bool calcFog) // = TRUE)
  {
    if (countV == 0)
    {
      return NULL;
    }
    ASSERT( countV <= renderState.maxVerts);

    // set screen verts directly to bucket
    //
	  VertexTL * vertmem;
    Bucket * bucket = LockPrimitiveMem( (void **)&vertmem, countV);
    if (!bucket)
    {
      return NULL;
    }

    VertexTL * dv = vertmem;
	  for ( const VertexTL * sv = srcV, * ev = srcV + countV; sv < ev; sv++, dv++)
    {
      dv->uv = sv->uv;
      dv->diffuse  = sv->diffuse;
      dv->specular = sv->specular;

      ProjectFromModel_I( *dv, *sv);

      if (calcFog)
      {
        dv->SetFog();
      }
    }

    UnlockPrimitiveMem( sv - srcV);

    return bucket;
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightTransformFromModel( VertexTL * dstV, const Vector * srcN, const Color * srcC, U32 countV)
  {
    ASSERT( BucketMan::GetMaterial());
    ColorF32 & diffuse = BucketMan::GetMaterial()->Diffuse();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      diffuse.g * renderState.ambientColorF32.g,
      diffuse.b * renderState.ambientColorF32.b,
      diffuse.a
    );

    for (VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcN++, srcC++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( dstV->vv, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.ModulateInline( *srcC, diff.r, diff.g, diff.b, diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

      TransformFromModel( *dstV);
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightTransformFromModel( VertexTL * dstV, const Vector * srcN, U32 countV)
  {
    ASSERT( BucketMan::GetMaterial());
    ColorF32 & diffuse = BucketMan::GetMaterial()->Diffuse();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      diffuse.g * renderState.ambientColorF32.g,
      diffuse.b * renderState.ambientColorF32.b,
      diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcN++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( dstV->vv, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.SetInline( diff.r, diff.g, diff.b, diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

      TransformFromModel( *dstV);
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightTransformFromModel( VertexTL * dstV, const Vector * srcV, const Vector * srcN, const Color * srcC, U32 countV)
  {
    ASSERT( BucketMan::GetMaterial());
    ColorF32 & diffuse = BucketMan::GetMaterial()->Diffuse();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      diffuse.g * renderState.ambientColorF32.g,
      diffuse.b * renderState.ambientColorF32.b,
      diffuse.a
    );

    for (VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcV++, srcN++, srcC++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( *srcV, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.ModulateInline( *srcC, diff.r, diff.g, diff.b, diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

      TransformFromModel( *dstV, *srcV);
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightTransformFromModel( VertexTL * dstV, const Vector * srcV, const Vector * srcN, U32 countV)
  {
    ASSERT( BucketMan::GetMaterial());
    ColorF32 & diffuse = BucketMan::GetMaterial()->Diffuse();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      diffuse.g * renderState.ambientColorF32.g,
      diffuse.b * renderState.ambientColorF32.b,
      diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcV++, srcN++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( *srcV, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.SetInline( diff.r, diff.g, diff.b, diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

      TransformFromModel( *dstV, *srcV);
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightProjectVerts( VertexTL * dstV, const Vector * srcN, const Color * srcC, U32 countV)
  {
    ASSERT( BucketMan::GetMaterial());
    ColorF32 & diffuse = BucketMan::GetMaterial()->Diffuse();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      diffuse.g * renderState.ambientColorF32.g,
      diffuse.b * renderState.ambientColorF32.b,
      diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcN++, srcC++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( dstV->vv, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.ModulateInline( *srcC, diff.r, diff.g, diff.b, diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

      ProjectFromModel_I( *dstV);

      dstV->SetFog();
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightProjectVerts( VertexTL * dstV, const Vector * srcN, U32 countV)
  {
    ASSERT( BucketMan::GetMaterial());
    ColorF32 & diffuse = BucketMan::GetMaterial()->Diffuse();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      diffuse.g * renderState.ambientColorF32.g,
      diffuse.b * renderState.ambientColorF32.b,
      diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcN++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( dstV->vv, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.SetInline( diff.r, diff.g, diff.b, diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

      ProjectFromModel_I( *dstV);

      dstV->SetFog();
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightProjectVerts( VertexTL * dstV, const Vector * srcV, const Vector * srcN, const Color * srcC, U32 countV)
  {
    ASSERT( BucketMan::GetMaterial());
    ColorF32 & diffuse = BucketMan::GetMaterial()->Diffuse();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      diffuse.g * renderState.ambientColorF32.g,
      diffuse.b * renderState.ambientColorF32.b,
      diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcV++, srcN++, srcC++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( *srcV, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.ModulateInline( *srcC, diff.r, diff.g, diff.b, diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

      ProjectFromModel_I( *dstV, *srcV);

      dstV->SetFog();
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightProjectVerts( VertexTL * dstV, const Vector * srcV, const Vector * srcN, U32 countV)
  {
    ASSERT( BucketMan::GetMaterial());
    ColorF32 & diffuse = BucketMan::GetMaterial()->Diffuse();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      diffuse.g * renderState.ambientColorF32.g,
      diffuse.b * renderState.ambientColorF32.b,
      diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcV++, srcN++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( *srcV, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.SetInline( diff.r, diff.g, diff.b, diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

      ProjectFromModel_I( *dstV, *srcV);

      dstV->SetFog();
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void ProjectVerts( VertexTL * dstV, U32 countV, Bool setFog) // = TRUE
  {
    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++)
    {
      ProjectFromModel_I( *dstV);

      if (setFog)
      {
        dstV->SetFog();
      }
	  }
  }
  //----------------------------------------------------------------------------

/* save 'em for a rainy day

  // to buffer
  //
  void LightProjectVertsFogX( VertexTL * dstV, const Vector * srcN, const Color * srcC, U32 countV)
  {
    Material & material = *BucketMan::GetMaterial();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      diffuse.r * renderState.ambientColorF32.r,
      material.GetDesc().diffuse.g * renderState.ambientColorF32.g,
      material.GetDesc().diffuse.b * renderState.ambientColorF32.b,
      material.GetDesc().diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcN++, srcC++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( dstV->vv, *srcN, diffuse, *BucketMan::GetMaterial(), diff, spec);

      // set the colors
      dstV->diffuse.ModulateInline( *srcC, diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) dstV->specular.a);

      ProjectFromModel_I( *dstV);

      dstV->SetFogX();
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightProjectVertsFogX( VertexTL * dstV, const Vector * srcN, U32 countV)
  {
    Material & material = *BucketMan::GetMaterial();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      material.GetDesc().diffuse.r * renderState.ambientColorF32.r,
      material.GetDesc().diffuse.g * renderState.ambientColorF32.g,
      material.GetDesc().diffuse.b * renderState.ambientColorF32.b,
      material.GetDesc().diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcN++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( dstV->vv, *srcN, material.Diffuse(), material, diff, spec);

      // set the colors
  //    dv.diffuse.ModulateInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
      dstV->diffuse.SetInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) dstV->specular.a);

      ProjectFromModel_I( *dstV);

      dstV->SetFogX();
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightProjectVertsFogX( VertexTL * dstV, const Vector * srcV, const Vector * srcN, const Color * srcC, U32 countV)
  {
    Material & material = *BucketMan::GetMaterial();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      material.GetDesc().diffuse.r * renderState.ambientColorF32.r,
      material.GetDesc().diffuse.g * renderState.ambientColorF32.g,
      material.GetDesc().diffuse.b * renderState.ambientColorF32.b,
      material.GetDesc().diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcV++, srcN++, srcC++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( *srcV, *srcN, material.Diffuse(), material, diff, spec);

      // set the colors
      dstV->diffuse.ModulateInline( *srcC, diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) dstV->specular.a);

      ProjectFromModel_I( *dstV, *srcV);

      dstV->SetFogX();
	  }
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void LightProjectVertsFogX( VertexTL * dstV, const Vector * srcV, const Vector * srcN, U32 countV)
  {
    Material & material = *BucketMan::GetMaterial();

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      material.GetDesc().diffuse.r * renderState.ambientColorF32.r,
      material.GetDesc().diffuse.g * renderState.ambientColorF32.g,
      material.GetDesc().diffuse.b * renderState.ambientColorF32.b,
      material.GetDesc().diffuse.a
    );

    for ( VertexTL * ev = dstV + countV; dstV < ev; dstV++, srcV++, srcN++)
    {
      // calculate lighting
      diff = diffInit;

      Vid::Light::LightModel( *srcV, *srcN, material.Diffuse(), material, diff, spec);

      // set the colors
  //    dv.diffuse.ModulateInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
      dstV->diffuse.SetInline( diff.r, diff.g, diff.b, material.GetDesc().diffuse.a);
      dstV->specular.SetInline( spec.r, spec.g, spec.b, (U32) dstV->specular.a);

      ProjectFromModel_I( *dstV, *srcV);

      dstV->SetFogX();
	  }
  }
  //----------------------------------------------------------------------------

  // return of NULL indicates that bucket memory couldn't be allocated
  //
  Bucket * ProjectClip( const Plane * srcP, const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog, U32 clipFlags) // = TRUE, = clipALL
  {
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // backcull; light and set homogeneous for valid verts

    // clear out indexer
	  memset( tmpVI, 0xff, countV * sizeof( U16));

    const U16 * si = srcI, * se = srcI + countI;
    countV = countI = 0;
	  for (U16 fi = 0; si < se; si += 3)
    {
	    U16 i0 = si[0];
      U16 i1 = si[1];
      U16 i2 = si[2];

      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
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

          TransformFromModel( dv);

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

          TransformFromModel( dv);

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

          TransformFromModel( dv);

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

    return Clip::ToBucket( tmpV, countV, tmpI, countI, (void *) 0xffffffff, calcFog, clipFlags);
  }
  //----------------------------------------------------------------------------

  // to buffer
  //
  void ProjectClip( VertexTL * dstV, U16 * dstI, const Plane * srcP, const VertexTL * srcV, U32 &countV, const U16 * srcI, U32 & countI, Bool calcFog, U32 clipFlags) // = TRUE, = clipALL
  {
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

    // backcull; light and set homogeneous for valid verts
    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // clear out indexer
	  memset( tmpVI, 0xff, countV * sizeof( U16));

    const U16 * si = srcI, * se = srcI + countI;
    countV = countI = 0;
	  for (U16 fi = 0; si < se; si += 3)
    {
	    U16 i0 = si[0];
      U16 i1 = si[1];
      U16 i2 = si[2];

      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
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

          TransformFromModel( dv);

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

          TransformFromModel( dv);

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

          TransformFromModel( dv);

          // set indexer
          tmpVI[i2] = (U16)countV;
          countV++;
        }
        tmpI[countI++] = tmpVI[i2];
		  }
	  }
    Clip::ToBuffer( dstV, dstI, tmpV, countV, tmpI, countI, calcFog, clipFlags);
  }
  //----------------------------------------------------------------------------


  Bucket * ProjectClipBias( const Plane * srcP, const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog, U32 clipFlags) // = TRUE, = clipALL
  {
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // backcull; light and set homogeneous for valid verts

    // clear out indexer
	  memset( tmpVI, 0xff, countV * sizeof( U16));

    const U16 * si = srcI, * se = srcI + countI;
    countV = countI = 0;
	  for (U16 fi = 0; si < se; si += 3)
    {
	    U16 i0 = si[0];
      U16 i1 = si[1];
      U16 i2 = si[2];

      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
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

          TransformFromModel( dv);

          dv.vv.z *= renderState.zBias;

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

          TransformFromModel( dv);

          dv.vv.z *= renderState.zBias;

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

          TransformFromModel( dv);

          dv.vv.z *= renderState.zBias;

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

    return Clip::ToBucket( tmpV, countV, tmpI, countI, (void *) 0xffffffff, calcFog, clipFlags);
  }
  //----------------------------------------------------------------------------

  Bucket * ProjectNoClipBias( const Plane * srcP, const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog) // = TRUE)
  {
    calcFog;

    if (countI == 0)
    {
      return NULL;
    }
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

	  VertexTL * vertmem;
    U16 * indexmem;
    Bucket * bucket = LockIndexedPrimitiveMem( (void **)&vertmem, countV, &indexmem, countI);
    if (!bucket)
    {
      return NULL;
    }

    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // backcull; light and set homogeneous for valid verts

    // clear out indexer
	  memset( tmpVI, 0xff, countV * sizeof( U16));

    const U16 * si = srcI, * se = srcI + countI;
    countV = countI = 0;
	  for (U16 fi = 0; si < se; si += 3)
    {
	    U16 i0 = si[0];
      U16 i1 = si[1];
      U16 i2 = si[2];

      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
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
          ProjectFromModel_I( dv, srcV[i0].vv);

          dv.vv.z *= renderState.zBias;

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
          ProjectFromModel_I( dv, srcV[i1].vv);

          dv.vv.z *= renderState.zBias;

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
          ProjectFromModel_I( dv, srcV[i2].vv);

          dv.vv.z *= renderState.zBias;

          // set vertex fog
          dv.SetFog();

          // set indexer
          tmpVI[i2] = (U16)countV;
          countV++;
        }
        indexmem[countI++] = tmpVI[i2];
		  }
	  }

    UnlockIndexedPrimitiveMem( countV, countI);

    return bucket;
  }
  //----------------------------------------------------------------------------

  Bucket * ProjectNoClip( const Plane * srcP, const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog) // = TRUE)
  {
    if (countI == 0)
    {
      return NULL;
    }
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

	  VertexTL * vertmem;
    U16 * indexmem;
    Bucket * bucket = LockIndexedPrimitiveMem( (void **)&vertmem, countV, &indexmem, countI);
    if (!bucket)
    {
      return NULL;
    }

    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // backcull; light and set homogeneous for valid verts

    // clear out indexer
	  memset( tmpVI, 0xff, countV * sizeof( U16));

    const U16 * si = srcI, * se = srcI + countI;
    countV = countI = 0;
	  for (U16 fi = 0; si < se; si += 3)
    {
	    U16 i0 = si[0];
      U16 i1 = si[1];
      U16 i2 = si[2];

      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
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
          ProjectFromModel_I( dv, srcV[i0].vv);

          if (calcFog)
          {
            // set vertex fog
            dv.SetFog();
          }

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
          ProjectFromModel_I( dv, srcV[i1].vv);

          if (calcFog)
          {
            // set vertex fog
            dv.SetFog();
          }

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
          ProjectFromModel_I( dv, srcV[i2].vv);

          if (calcFog)
          {
            // set vertex fog
            dv.SetFog();
          }

          // set indexer
          tmpVI[i2] = (U16)countV;
          countV++;
        }
        indexmem[countI++] = tmpVI[i2];
		  }
	  }

    UnlockIndexedPrimitiveMem( countV, countI);

    return bucket;
  }
  //----------------------------------------------------------------------------

  // non-indexed tris
  //
  Bucket * ProjectNoClip( const Plane * srcP, const VertexTL * srcV, U32 countV, Bool calcFog) // = TRUE)
  {
    if (countV == 0)
    {
      return NULL;
    }
    ASSERT( countV <= renderState.maxVerts);

	  VertexTL * vertmem;
    Bucket * bucket = LockPrimitiveMem( (void **)&vertmem, countV);
    if (!bucket)
    {
      return NULL;
    }

    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // backcull; light and set homogeneous for valid verts

    const VertexTL * sv = srcV, * se = srcV + countV;
    VertexTL * dv = vertmem;;
    countV = 0;
	  for (U16 fi = 0; sv < se; sv += 3)
    {
      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
      {
        // forward face

        const VertexTL * ss = sv;

        // do vert 0
        dv->uv = ss->uv;
        dv->diffuse  = ss->diffuse;
        dv->specular = ss->specular;

        // project
        ProjectFromModel_I( *dv, ss->vv);

        if (calcFog)
        {
          // set vertex fog
          dv->SetFog();
        }
        dv++, ss++, countV++;

        // do vert 1
        dv->uv = ss->uv;
        dv->diffuse  = ss->diffuse;
        dv->specular = ss->specular;

        // project
        ProjectFromModel_I( *dv, ss->vv);

        if (calcFog)
        {
          // set vertex fog
          dv->SetFog();
        }
        dv++, ss++, countV++;

        // do vert 2
        dv->uv = ss->uv;
        dv->diffuse  = ss->diffuse;
        dv->specular = ss->specular;

        // project
        ProjectFromModel_I( *dv, ss->vv);

        if (calcFog)
        {
          // set vertex fog
          dv->SetFog();
        }
        dv++, ss++, countV++;
		  }
	  }

    UnlockPrimitiveMem( countV);

    return bucket;
  }
  //----------------------------------------------------------------------------


  // to buffer
  //
  void Light( VertexTL * dstV, U16 * dstI, const Plane * srcP, const Vector * srcVect, const Vector * srcN, const UVPair * srcUV, U32 & countV, const U16 * srcI, U32 & countI)
  {
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

    Material &material = *BucketMan::GetMaterial();
    U32 diffa = Utils::FtoL( material.GetDesc().diffuse.a  * 255.0f);

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      material.GetDesc().diffuse.r * renderState.ambientColorF32.r,
      material.GetDesc().diffuse.g * renderState.ambientColorF32.g,
      material.GetDesc().diffuse.b * renderState.ambientColorF32.b,
      material.GetDesc().diffuse.a
    );

    // backcull; light and project for valid verts
    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // clear out indexer
	  memset( tmpVI, 0xff, countV * sizeof( U16));

    const U16 * si = srcI, * se = srcI + countI;
    countV = countI = 0;
	  for (U16 fi = 0; si < se; si += 3)
    {
	    U16 i0 = si[0];
      U16 i1 = si[1];
      U16 i2 = si[2];

      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
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
          spec.Set( 1, 1, 1);
          Vid::Light::LightModel( dv.vv, srcN[i0], material.Diffuse(), material, diff, spec);

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
    
          Vid::Light::LightModel( dv.vv, srcN[i1], material.Diffuse(), material, diff, spec);

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
    
          Vid::Light::LightModel( dv.vv, srcN[i2], material.Diffuse(), material, diff, spec);

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
  void LightProject( VertexTL * dstV, U16 * dstI, const Plane * srcP, const Vector * srcVect, const Vector *srcN, const UVPair * srcUV, U32 & countV, const U16 * srcI, U32 & countI, U32 clipFlags)
  {
    clipFlags;

    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

    Material &material = *BucketMan::GetMaterial();
    U32 diffa = Utils::FtoL( material.GetDesc().diffuse.a  * 255.0f);

    // calculate the parts of the diffuse color that are the same for all output vertexes
    ColorF32 diff, spec, diffInit
    (
      material.GetDesc().diffuse.r * renderState.ambientColorF32.r,
      material.GetDesc().diffuse.g * renderState.ambientColorF32.g,
      material.GetDesc().diffuse.b * renderState.ambientColorF32.b,
      material.GetDesc().diffuse.a
    );

    // backcull; light and project for valid verts
    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // clear out indexer
	  memset( tmpVI, 0xff, countV * sizeof( U16));

    const U16 * si = srcI, * se = srcI + countI;
    countV = countI = 0;
	  for (U16 fi = 0; si < se; si += 3)
    {
	    U16 i0 = si[0];
      U16 i1 = si[1];
      U16 i2 = si[2];

      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
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
    
          Vid::Light::LightModel( dv.vv, srcN[i0], material.Diffuse(), material, diff, spec);

          // set the colors
          dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
          dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

          TransformFromModel( dv);

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
    
          Vid::Light::LightModel( dv.vv, srcN[i1], material.Diffuse(), material, diff, spec);

          // set the colors
          dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
          dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

          TransformFromModel( dv);

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
    
          Vid::Light::LightModel( dv.vv, srcN[i2], material.Diffuse(), material, diff, spec);

          // set the colors
          dv.diffuse.SetInline(  diff.r, diff.g, diff.b, diffa);
          dv.specular.SetInline( spec.r, spec.g, spec.b, (U32) 255);

          TransformFromModel( dv);

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
  void ProjectNoClip( VertexTL * dstV, U16 * dstI, const Plane * srcP, const VertexTL * srcV, U32 & countV, const U16 * srcI, U32 & countI, Bool calcFog) //  = FALSE)
  {
    ASSERT( countV < renderState.maxVerts && countI <= Vid::renderState.maxIndices);

    // backcull; light and set homogeneous for valid verts
    Bool nobackcull = !srcP || (BucketMan::GetPrimitiveFlags() & RS_2SIDED);

    // clear out indexer
	  memset( tmpVI, 0xff, countV * sizeof( U16));

    const U16 * si = srcI, * se = srcI + countI;
    countV = countI = 0;
	  for (U16 fi = 0; si < se; si += 3)
    {
	    U16 i0 = si[0];
      U16 i1 = si[1];
      U16 i2 = si[2];

      if (nobackcull || srcP[fi++].Evalue( Math::modelViewVector) >= 0.0f)
      {
        // forward face

        if (tmpVI[i0] == 0xffff)
        {
          // do vert 0

          // copy
          VertexTL &dv = dstV[countV];
          const VertexTL &sv = srcV[i0];
          dv.uv = sv.uv;

          ProjectFromModel_I( dv, sv.vv);

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

          ProjectFromModel_I( dv, sv.vv);

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

          ProjectFromModel_I( dv, sv.vv);

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

*/
}
//----------------------------------------------------------------------------

