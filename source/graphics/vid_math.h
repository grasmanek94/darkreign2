///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_math.h
//
// 12-FEB-2000
//

#ifndef __VIDMATH_H
#define __VIDMATH_H

#include "vidclip.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Math
  {
	  extern Matrix	worldMatrix;
	  extern Matrix	worldMatrixInv;   // inverse
	  extern Matrix	cameraMatrix;     // camera in world
	  extern Matrix	viewMatrix;       // world to view (camera)
    extern Matrix	transformMatrix;  // model to view 
	  extern Matrix	projMatrix;       // view to screen
    extern Vector modelViewVector;  // for backface cullling
	  extern Vector modelViewNorm;    // for specular lighting

    extern F32 nearPlane;
    extern F32 farPlane;

    extern Point<F32> size;
    extern Point<F32> origin;

  #ifdef __DO_XMM_BUILD
    extern MatrixXmm *          modelProjMatrixXMM;
  #endif

  };
  //----------------------------------------------------------------------------
  
  // calculate the camera coord position corresponding of the cursor on the nearplane
  // return it in 'pos'
  // sx and sy are in view coords
  //
  void ScreenToCamera( Vector & dst, S32 sx, S32 sy);
  void ScreenToWorld(  Vector & dst, S32 sx, S32 sy);

  Bool BackCullWorld(  const Vector & v0, const Vector & v1, const Vector & v2); 
  Bool BackCullWorld(  const Vector & v0, const Vector & v1, const Vector & v2, const Vector & v3); 
  Bool BackCullCamera( const Vector & v0, const Vector & v1, const Vector & v2); 
  Bool BackCullCamera( const Vector & v0, const Vector & v1, const Vector & v2, const Vector & v3); 

  // returns TRUE if culled, FALSE otherwise
  //
  inline Bool BackCull( F32 value)
  {
    if (renderState.status.mirrorIn)
    {
      // through the looking glass
      return value >= 0;
    }
    else
    {
      return value <= 0;
    }
  }
  //----------------------------------------------------------------------------

  inline void TransformFromModel( Vector & dst, const Vector & src)
  {
    Math::transformMatrix.Transform( dst, src);
  }
  inline void TransformFromModel( Vector & dst)
  {
    Math::transformMatrix.Transform( dst);
  }
  inline void TransformFromModel( VertexTL & dst, const Vector & src)
  {
    TransformFromModel( dst.vv, src);
  }
  inline void TransformFromModel( VertexTL & dst)
  {
    Math::transformMatrix.Transform( dst.vv);
  }
  void TransformFromModel( VertexTL * dst, U32 count);
  void TransformFromModel( VertexTL * dst, U32 count, Color color);
  void TransformFromModel( Vector * dst, const Vector * src, U32 count);
  void TransformFromModel( VertexTL * dst, const Vector * src, U32 count);
  void TransformFromModel( VertexTL * dst, const Vector * src, U32 count, Color color);
  //----------------------------------------------------------------------------

  inline void TransformFromWorld( Vector & dst, const Vector & src)
  {
    Math::viewMatrix.Transform( dst, src);
  }
  inline void TransformFromWorld( Vector & dst)
  {
    Math::viewMatrix.Transform( dst);
  }
  //----------------------------------------------------------------------------

  inline F32 W( F32 z)
  {
    F32 w = z * Math::projMatrix.frontw + Math::projMatrix.positw;

    if ( fabs(w) < F32_EPSILON )
    {
      w = F32_EPSILON * Utils::FSign(w);
    }
    return w;
  }
  inline F32 RHW( F32 z)
  {
    return ( 1 / W( z) );
  }
  inline F32 SetHomogeneousZ( F32 sz)
  {
    return ( sz *  Math::projMatrix.front.z + Math::projMatrix.posit.z );
  }
  inline F32 ProjectZ( F32 sz)
  {
    return SetHomogeneousZ( sz) * RHW( sz);
  }
  inline F32 ProjectRHW( F32 y, F32 rhw)
  {
    return ( -y * Math::projMatrix.up.y * Math::size.y * rhw );
  }
  F32 Project(  F32 y, F32 z);
  F32 ProjectInv( F32 sy, F32 z);

/*  project from camera space

    dst.vv.x = (dst.vv.x * Math::projMatrix.right.x * Math::size.x * rhw) + Math::origin.x;
    dst.vv.y = (dst.vv.y * Math::projMatrix.up.y    * Math::size.y * rhw) + Math::origin.y;
    dst.vv.z = (dst.vv.z * Math::projMatrix.front.z + Math::projMatrix.posit.z) * rhw;
*/
  //----------------------------------------------------------------------------

  // return rhw
  //
  inline F32 ProjectFromHomogeneous_I( Vector & dst, F32 w, const Vector & src)
  {
    F32 rhw = 1.0f / w;
    dst.x = src.x * Math::size.x * rhw + Math::origin.x;
    dst.y = src.y * Math::size.y * rhw + Math::origin.y;
    dst.z = src.z * rhw;
    return rhw;
  }
  inline F32 ProjectFromHomogeneous_I( VertexTL & dst, const VertexTL & src)
  {
    dst.diffuse  = src.diffuse;
    dst.specular = src.specular;
    dst.uv       = src.uv;
    dst.rhw = ProjectFromHomogeneous_I( dst.vv, src.rhw, src.vv);
    return dst.rhw;
  }
  inline F32 ProjectFromHomogeneous_I( Vector & dst, F32 w)
  {
    return ProjectFromHomogeneous_I( dst, w, dst);
  }
  inline F32 ProjectFromHomogeneous_I( VertexTL & dst)
  {
    dst.rhw = ProjectFromHomogeneous_I( dst.vv, dst.rhw, dst.vv);
    return dst.rhw;
  }
  F32 ProjectFromHomogeneous( Vector & dst, F32 w, const Vector & src);
  F32 ProjectFromHomogeneous( VertexTL & dst, const VertexTL & src);
  F32 ProjectFromHomogeneous( Vector & dst, F32 w);
  F32 ProjectFromHomogeneous( VertexTL & dst);
  //----------------------------------------------------------------------------

  // return w
  //
  inline F32 SetHomogeneousFromCamera_I( Vector & dst, const Vector & src)
  {
    F32 rhw = W( src.z);

    dst.x = src.x * Math::projMatrix.right.x;
    dst.y = src.y * Math::projMatrix.up.y;
    dst.z = src.z * Math::projMatrix.front.z + Math::projMatrix.posit.z;

    return rhw;
  }
  inline F32 SetHomogeneousFromCamera_I( Vector & dst)
  {
    F32 rhw = W( dst.z);

    dst.x = dst.x * Math::projMatrix.right.x;
    dst.y = dst.y * Math::projMatrix.up.y;
    dst.z = dst.z * Math::projMatrix.front.z + Math::projMatrix.posit.z;

    return rhw;
  }
  inline F32 SetHomogeneousFromCamera_I( VertexTL & dst, const Vector & src)
  {
    dst.rhw = SetHomogeneousFromCamera_I( dst.vv, src);
    return dst.rhw;
  }
  inline F32 SetHomogeneousFromCamera_I( VertexTL & dst)
  {
    return SetHomogeneousFromCamera_I( dst, dst.vv);
  }
  //----------------------------------------------------------------------------

  // return w
  //
  inline F32 ProjectFromCamera_II( Vector & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromCamera_I( dst, src);
    ProjectFromHomogeneous_I( dst, w);
    return w;
  }
  inline F32 ProjectFromCamera_II( Vector & dst)
  {
    return ProjectFromCamera_II( dst, dst);
  }
  inline F32 ProjectFromCamera_II( VertexTL & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromCamera_I( dst, src);
    ProjectFromHomogeneous_I( dst);
    return w;
  }
  inline F32 ProjectFromCamera_II( VertexTL & dst, const VertexTL & src)
  {
    dst.diffuse  = src.diffuse;
    dst.specular = src.specular;
    dst.uv       = src.uv;
    return ProjectFromCamera_II( dst, src.vv);
  }
  inline F32 ProjectFromCamera_II( VertexTL & dst)
  {
    F32 w = SetHomogeneousFromCamera_I( dst);
    ProjectFromHomogeneous_I( dst);
    return w;
  }
  //----------------------------------------------------------------------------

  // return w
  //
  inline F32 ProjectFromCamera_I( Vector & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromCamera_I( dst, src);
    ProjectFromHomogeneous( dst, w);
    return w;
  }
  inline F32 ProjectFromCamera_I( Vector & dst)
  {
    return ProjectFromCamera_I( dst, dst);
  }
  inline F32 ProjectFromCamera_I( VertexTL & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromCamera_I( dst, src);
    ProjectFromHomogeneous( dst);
    return w;
  }
  inline F32 ProjectFromCamera_I( VertexTL & dst, const VertexTL & src)
  {
    dst.diffuse  = src.diffuse;
    dst.specular = src.specular;
    dst.uv       = src.uv;
    return ProjectFromCamera_I( dst, src.vv);
  }
  inline F32 ProjectFromCamera_I( VertexTL & dst)
  {
    F32 w = SetHomogeneousFromCamera_I( dst);
    ProjectFromHomogeneous( dst);
    return w;
  }
  //----------------------------------------------------------------------------

  // return rhw
  //
  F32  ProjectFromCamera( Vector & dst, const Vector & src);
  F32  ProjectFromCamera( Vector & dst);
  F32  ProjectFromCamera( VertexTL & dst);

  void ProjectFromCamera( VertexTL * dst, U32 count);
  void ProjectFromCamera( VertexTL * dst, U32 count, Color color);
  //----------------------------------------------------------------------------

  // return w
  //
  inline F32 SetHomogeneousFromModel_I( Vector & dst, const Vector & src)
  {
    TransformFromModel( dst, src);
    return SetHomogeneousFromCamera_I( dst);
  }
  inline F32 SetHomogeneousFromModel_I( VertexTL & dst, const Vector & src)
  {
    TransformFromModel( dst, src);
    return SetHomogeneousFromCamera_I( dst);
  }
  inline F32 SetHomogeneousFromModel_I( Vector & dst)
  {
    TransformFromModel( dst);
    return SetHomogeneousFromCamera_I( dst);
  }
  inline F32 SetHomogeneousFromModel_I( VertexTL & dst)
  {
    TransformFromModel( dst);
    return SetHomogeneousFromCamera_I( dst);
  }
  //----------------------------------------------------------------------------

  // return w
  //
  inline F32 ProjectFromModel_II( Vector & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromModel_I( dst, src);
    ProjectFromHomogeneous_I( dst, w);
    return w;
  }
  inline F32 ProjectFromModel_II( Vector & dst)
  {
    F32 w = SetHomogeneousFromModel_I( dst);
    ProjectFromHomogeneous_I( dst, w);
    return w;
  }
  inline F32 ProjectFromModel_II( VertexTL & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromModel_I( dst, src);
    ProjectFromHomogeneous_I( dst);
    return w;
  }
  inline F32 ProjectFromModel_II( VertexTL & dst, const Vertex & src)
  {
    return ProjectFromModel_II( dst, src.vv);
  }
  inline F32 ProjectFromModel_II( VertexTL & dst)
  {
    F32 w = SetHomogeneousFromModel_I( dst);
    ProjectFromHomogeneous_I( dst);
    return w;
  }
  //----------------------------------------------------------------------------

  // return w
  //
  inline F32 ProjectFromModel_I( Vector & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromModel_I( dst, src);
    ProjectFromHomogeneous( dst, w);
    return w;
  }
  inline F32 ProjectFromModel_I( Vector & dst)
  {
    F32 w = SetHomogeneousFromModel_I( dst);
    ProjectFromHomogeneous( dst, w);
    return w;
  }
  inline F32 ProjectFromModel_I( VertexTL & dst, const Vector & src)
  {
    F32 w = SetHomogeneousFromModel_I( dst, src);
    ProjectFromHomogeneous( dst);
    return w;
  }
  inline F32 ProjectFromModel_I( VertexTL & dst, const VertexTL & src)
  {
    return ProjectFromModel_I( dst, src.vv);
  }
  inline F32 ProjectFromModel_I( VertexTL & dst)
  {
    F32 w = SetHomogeneousFromModel_I( dst);
    ProjectFromHomogeneous( dst);
    return w;
  }
  //----------------------------------------------------------------------------

  // return rhw
  //
  F32  ProjectFromModel( Vector & dst, const Vector & src);
  F32  ProjectFromModel( Vector & dst);
  F32  ProjectFromModel( VertexTL & dst);

  void ProjectFromModel( VertexTL * dst, U32 count);
  void ProjectFromModel( VertexTL * dst, const Vector * src, U32 count);
  void ProjectFromModel( VertexTL * dst, const Vector * src, U32 count, Color color);
  //----------------------------------------------------------------------------

  // return camera z
  //
  F32 ProjectFromWorld( Vector & dst, const Vector & src, F32 & rhw);
  F32 ProjectFromWorld( Vector & dst, F32 & rhw);
  F32 ProjectFromWorld( VertexTL & dst, const Vector & src);
  F32 ProjectFromWorld( VertexTL & dst);
  // world transform helpers
  F32 ProjectFromWorld( Vector & dst, const Vector & src, F32 & rhw, const Matrix & world);
  F32 ProjectFromWorld( Vector & dst, F32 & rhw, const Matrix & world);
  F32 ProjectFromWorld( VertexTL & dst, const Vector & src, const Matrix & world);
  F32 ProjectFromWorld( VertexTL & dst, const Matrix & world);
  //----------------------------------------------------------------------------

  void ProjectIsoFromCamera( Vector & dst, F32 & drhw, const Vector & src);
  //----------------------------------------------------------------------------

  inline void ProjectIsoFromWorld( Vector & dst, F32 & drhw, const Vector & src)
  {
    TransformFromWorld( dst, src);
    ProjectIsoFromCamera( dst, drhw, dst);
  }
  inline void ProjectIsoFromWorld( VertexTL & dst, const Vector & src)
  {
    ProjectIsoFromWorld( dst.vv, dst.rhw, src);
  }
  inline void ProjectIsoFromWorld( VertexTL & dst)
  {
    ProjectIsoFromWorld( dst, dst.vv);
  }
  //----------------------------------------------------------------------------

  // to bucket functions
  // return of NULL indicates that bucket memory couldn't be allocated
  //
  Bucket * ProjectClip( const Vector  * srcV, const UVPair * srcUV, Color diffuse, Color specular, U32 countV, const U16 * srcI, U32 countI, Bool calcFog = TRUE, U32 clipFlags = clipALL);
  Bucket * ProjectClip( VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog = TRUE, U32 clipFlags = clipALL);
  Bucket * ProjectNoClip( const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog = TRUE);
  Bucket * ProjectNoClip( const VertexTL * srcV, U32 countV, Bool calcFog = TRUE);

  // to buffer funtions
  //
  void ProjectVerts( VertexTL * dstV, U32 countV, Bool setFog = TRUE);  

  void LightProjectVerts( VertexTL * dstV, const Vector * srcNorm, U32 countV);
  void LightProjectVerts( VertexTL * dstV, const Vector * srcNorm, const Color * srcC, U32 countV);  
  void LightProjectVerts( VertexTL * dstV, const Vector * srcVect, const Vector * srcNorm, U32 countV);
  void LightProjectVerts( VertexTL * dstV, const Vector * srcVect, const Vector * srcNorm, const Color * srcC, U32 countV);  

  void LightTransformFromModel( VertexTL * dstV, const Vector * srcNorm, U32 countV);
  void LightTransformFromModel( VertexTL * dstV, const Vector * srcNorm, const Color * srcC, U32 countV);  
  void LightTransformFromModel( VertexTL * dstV, const Vector * srcVect, const Vector * srcNorm, U32 countV);
  void LightTransformFromModel( VertexTL * dstV, const Vector * srcVect, const Vector * srcNorm, const Color * srcC, U32 countV);  

/*  save 'em for a rainy day

  void LightProjectVertsFogX( VertexTL * dstV, const Vector * srcNorm, U32 countV);
  void LightProjectVertsFogX( VertexTL * dstV, const Vector * srcNorm, const Color * srcC, U32 countV);  
  void LightProjectVertsFogX( VertexTL * dstV, const Vector * srcVect, const Vector * srcNorm, U32 countV);
  void LightProjectVertsFogX( VertexTL * dstV, const Vector * srcVect, const Vector * srcNorm, const Color * srcC, U32 countV);  

  void Light( VertexTL * dstV, U16 * dstI, const Plane * srcP, const Vector * srcVect, const Vector * srcNorm, const UVPair * srcUV, U32 & countV, const U16 * srcI, U32 & countI);
  void LightProject( VertexTL * dstV, U16 * dstI, const Plane * srcP, const Vector * srcVect, const Vector * srcNorm, const UVPair * srcUV, U32 & countV, const U16 * srcI, U32 & countI, U32 clipFlags = clipALL);
  void LightProject( VertexTL * dstV, U16 * dstI, const Plane * srcP, const Vector * srcVect, const Vector * srcNorm, const UVPair * srcUV, const Color * srcC, U32 & countV, const U16 * srcI, U32 & countI, U32 clipFlags = clipALL);
    
  Bucket * LightProjectClip(   const Plane * srcP, const Vector * srcVect, const Vector * srcNorm, const UVPair *srcUV, U32 countV, const U16 * srcI, U32 countI, U32 clipFlags = clipALL);
  Bucket * LightProjectNoClip( const Plane * srcP, const Vector * srcVect, const Vector * srcNorm, const UVPair *srcUV, U32 countV, const U16 * srcI, U32 countI);
  Bucket * LightProjectClip(   const Plane * srcP, const Vector * srcVect, const Vector * srcNorm, const UVPair *srcUV, const Color * srcC, U32 countV, const U16 * srcI, U32 countI, U32 clipFlags = clipALL);
  Bucket * LightProjectNoClip( const Plane * srcP, const Vector * srcVect, const Vector * srcNorm, const UVPair *srcUV, const Color * srcC, U32 countV, const U16 * srcI, U32 countI);

  Bucket * ProjectClip(   const Plane * srcP, const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog = TRUE, U32 clipFlags = clipALL);

  void ProjectClip(  VertexTL * dstV, U16 * dstI, const Plane * srcP, const VertexTL * srcV, U32 & countV, const U16 * srcI, U32 & countI, Bool calcFog = TRUE, U32 clipFlags = clipALL);
  void ProjectNoClip(VertexTL * dstV, U16 * dstI, const Plane * srcP, const VertexTL * srcV, U32 & countV, const U16 * srcI, U32 & countI, Bool calcFog = TRUE);

  Bucket * ProjectClipBias(   const Plane * srcP, const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog = TRUE, U32 clipFlags = clipALL);
  Bucket * ProjectNoClipBias( const Plane * srcP, const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog = TRUE);

//  Bucket * ProjectClip(   const Plane * srcP, const Vector  * srcV, const UVPair * srcUV, Color diffuse, Color specular, U32 countV, const U16 * srcI, U32 countI, Bool calcFog = TRUE, U32 clipFlags = clipALL);
//  Bucket * ProjectNoClip( const Plane * srcP, const VertexTL * srcV, U32 countV, const U16 * srcI, U32 countI, Bool calcFog = TRUE);
//  Bucket * ProjectNoClip( const Plane * srcP, const VertexTL * srcV, U32 countV, Bool calcFog = TRUE);
*/
}
//----------------------------------------------------------------------------

#endif		// __VIDMATH_H

