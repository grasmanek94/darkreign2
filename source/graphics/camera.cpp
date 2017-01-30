///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// camera.cpp
//
// 08-SEP-1998
//

#include "vid_public.h"
#include "meshent.h"
#include "perfstats.h"
//-----------------------------------------------------------------------------

#define TRANMAXBUCKETCOUNT    22
//-----------------------------------------------------------------------------

void Camera::Mirror( const Plane & plane)
{
  SaveMatrix();

//  SetProjTransform( nearPlane, farPlane + 10.0f, fov);

  Matrix matrix = WorldMatrix();
  matrix.Mirror( plane);
  
  SetWorldAll( matrix);
}
//-----------------------------------------------------------------------------

void Camera::ClearData()
{
  FamilyNode::ClearData();

  nodeType = nodeCamera;

  // setup FamilyNode::state and FamilyState::node pointers
  SetState( state);

#ifdef DOOCCULSION
  Occlusion = NULL;
#endif

#ifdef __DO_XMM_BUILD
  xmmMem = FALSE;
#endif

  memset(&rect, 0, sizeof(rect));
  last = rect;
  clearCount = 0;
  projMatrix.ClearData();

  nearPlane = 1.0f;
  farPlane  = 2000.0f;
  maxZ = farPlane - nearPlane;

  fov = 45.0f;
  zoom = 1.0f;

  viewMatrix.ClearData();
	tranMatrix.ClearData();

  isIso = FALSE;
}
//-----------------------------------------------------------------------------

Camera::Camera( const char * _name)
{
  SetName( _name);

  ClearData();

  SetProjTransform( STARTNEARPLANE, STARTFARPLANE, STARTFOV);
}
//-----------------------------------------------------------------------------

Camera::~Camera()
{
#ifdef __DO_XMM_BUILD
  FreeXmm();
#endif

#ifdef DOOCCULSION
  // destroy Occlusion struct
  ReleaseOcclusion();
#endif
}
//-----------------------------------------------------------------------------

void Camera::SetProjTransformIso( F32 n, F32 f, F32 fv, F32 w, F32 h)
{
  if (f <= n)
  {
    f = n + 1.0f;
  }
  isIso = TRUE;

  nearPlane = n;
	farPlane  = f;
	fov		    = fv;
  maxZ      = farPlane * zoom - nearPlane;

  fv *= 0.5f;
	F32 s = (F32) sin(fv);
	F32 qv = s * f / (f - n);
	  
  // set up the isometric projection matrix
	projMatrix.Zero();
  projMatrix.right.x = 2.0f / h;
	projMatrix.up.y    = 2.0f / w;
	projMatrix.front.z = 1.0f;
//	projMatrix.frontw = 1.0f;
	projMatrix.posit.z = -qv * n;
	projMatrix.positw = 1.0f;

  halfHeight = ((F32) rect.Height()) * 0.5f;
  halfWidth  = ((F32) rect.Width())  * 0.5f;
  aspectHW   = halfHeight / halfWidth;

  viewMatrix.Transform( tranMatrix, projMatrix);

  SetupMatrix();
  SetupView();
}
//-----------------------------------------------------------------------------

void Camera::SetProjTransform( F32 n, F32 f, F32 fv)
{
  if (f <= n)
  {
    f = n + 1.0f;
  }
  isIso = FALSE;

  nearPlane = n;
	farPlane  = f;
	fov		    = fv;
  maxZ      = farPlane * zoom - nearPlane;

	Vid::SetTranBucketMaxZ( farPlane * zoom);
	Vid::SetTranBucketMinZ(0.0f);
  Vid::SetTranMaxBucketCount( TRANMAXBUCKETCOUNT);

  halfHeight = ((F32) rect.Height()) * 0.5f;
  halfWidth  = ((F32) rect.Width())  * 0.5f;
  aspectHW   = halfHeight / halfWidth;

  projMatrix.SetProjection( nearPlane, farPlane * zoom, fov / zoom, aspectHW);

  viewMatrix.Transform( tranMatrix, projMatrix);

  SetupMatrix();
  SetupView();
}
//-----------------------------------------------------------------------------

void Camera::Setup( const Camera & cam)
{
  zoom = cam.zoom;
  SetProjTransform( cam.nearPlane, cam.farPlane, cam.fov);
  Setup( cam.rect);
  SetWorldAll( cam.WorldMatrix());
}
//-----------------------------------------------------------------------------

Matrix & Camera::GetProjTransform()
{
  return projMatrix;
}
//-----------------------------------------------------------------------------

void Camera::SetZoom( F32 z)
{
  if (isIso)
  {
    return;     // FIXME
  }

  halfHeight = ((F32) rect.Height()) * 0.5f;
  halfWidth  = ((F32) rect.Width())  * 0.5f;
  aspectHW   = halfHeight / halfWidth;

  zoom = z;
  projMatrix.SetProjection( nearPlane, farPlane * zoom, fov / zoom, aspectHW);

	Vid::SetTranBucketMaxZ( farPlane * zoom);
	Vid::SetTranBucketMinZ(0.0f);
  Vid::SetTranMaxBucketCount( TRANMAXBUCKETCOUNT);

  maxZ = farPlane * zoom - nearPlane;

  SetupMatrix();
  SetupView();
}
//-----------------------------------------------------------------------------

void Camera::Setup( const Area<S32> & r)
{
  // save the last for clear borders
  //
  if (clearCount == 0)
  {
    last = r;
  }
  rect = r;
  clearCount = 3;

  x0 = (F32) rect.p0.x / (F32) Vid::viewRect.Width();
  x1 = (F32) rect.p1.x / (F32) Vid::viewRect.Width();
  y0 = (F32) rect.p0.y / (F32) Vid::viewRect.Height();
  y1 = (F32) rect.p1.y / (F32) Vid::viewRect.Height();

  SetupView();

#ifdef DOOCCULSION
  if (Occlusion)
  {
	  Occlusion->Setup ();
  }
#endif
}
//-----------------------------------------------------------------------------

void Camera::OnModeChange()
{
  rect.p0.x = (S32) (x0 * (F32) Vid::viewRect.Width());
  rect.p1.x = (S32) (x1 * (F32) Vid::viewRect.Width());
  rect.p0.y = (S32) (y0 * (F32) Vid::viewRect.Height());
  rect.p1.y = (S32) (y1 * (F32) Vid::viewRect.Height());   

  SetupView();
}
//-----------------------------------------------------------------------------

void Camera::SetupView()
{
  if (rect.Width() && rect.Height())
  {
    halfHeight = ((F32) rect.Height()) * 0.5f;
    halfWidth  = ((F32) rect.Width())  * 0.5f;
    aspectHW   = halfHeight / halfWidth;

    if (!isIso)
    {
      projMatrix.SetProjection( nearPlane, farPlane * zoom, fov / zoom, aspectHW);

      if (Vid::renderState.status.clipVis)
      {
        halfHeight -= (F32) 64 * aspectHW;
        halfWidth  -= (F32) 64;
      }
      aspectHW   = halfHeight / halfWidth;

      invProjX = aspectHW / ( projMatrix.right.x * halfHeight);
      invProjY = 1 / (-projMatrix.up.y * halfHeight);
    }
    else
    {
      if (Vid::renderState.status.clipVis)
      {
        halfHeight -= (F32) 64 * aspectHW;
        halfWidth  -= (F32) 64;
      }
      aspectHW   = halfHeight / halfWidth;

      invProjX = 1 / ( projMatrix.right.x * halfHeight);
      invProjY = 1 / (-projMatrix.up.y * halfHeight);
    }

    invDepth  = 1.0f / (farPlane - nearPlane);

    F32 proj23 = projMatrix.frontw == 0 ? 1 : projMatrix.frontw;

    invProjX *= proj23;
    invProjY *= proj23;

  #ifdef __DO_XMM_BUILD
    AllocXmm();
    SetupXmm();
  #endif

    F32 guard = (F32) Vid::renderState.clipGuardSize;

    // guard planes
    //
    F32 kx, ky, z = nearPlane;
    if (isIso)
    {
      kx = invProjX;
      ky = invProjY;
    }
    else
    {
      kx = (z * projMatrix.frontw + projMatrix.positw) * invProjX;
      ky = (z * projMatrix.frontw + projMatrix.positw) * invProjY;
    }

    frustrum[0].x = (-halfWidth  - guard) * kx;
    frustrum[0].y = (-halfHeight - guard) * ky;
    frustrum[0].z = z;

    frustrum[1].x = ( halfWidth  + guard) * kx;
    frustrum[1].y = frustrum[0].y;
    frustrum[1].z = z;

    frustrum[2].x = frustrum[1].x;
    frustrum[2].y = ( halfHeight + guard) * ky;
    frustrum[2].z = z;

    frustrum[3].x = frustrum[0].x;
    frustrum[3].y = frustrum[2].y;
    frustrum[3].z = z;

    z  = farPlane * zoom;
    if (isIso)
    {
      kx = 1.0f * invProjX;
      ky = 1.0f * invProjY;
    }
    else
    {
      kx = (z * projMatrix.frontw + projMatrix.positw) * invProjX;
      ky = (z * projMatrix.frontw + projMatrix.positw) * invProjY;
    }

    frustrum[4].x = (-halfWidth  - guard) * kx;
    frustrum[4].y = (-halfHeight - guard) * ky;
    frustrum[4].z = z;

    frustrum[5].x = ( halfWidth  + guard) * kx;
    frustrum[5].y = frustrum[4].y;
    frustrum[5].z = z;

    frustrum[6].x = frustrum[5].x;
    frustrum[6].y = ( halfHeight + guard) * ky;
    frustrum[6].z = z;

    frustrum[7].x = frustrum[4].x;
    frustrum[7].y = frustrum[6].y;
    frustrum[7].z = z;

    Vector origin;
    origin.Set( 0.0, 0.0, 0.0);

    // near
    guardPlanes[0].Set( frustrum[0], frustrum[1], frustrum[2]);
    guardPlanes[0].Normalize();
    
    // far
    guardPlanes[1].Set( frustrum[4], frustrum[5], frustrum[6]);
    guardPlanes[1].Normalize();

    // left
    guardPlanes[2].Set( origin, frustrum[4], frustrum[7]);
    guardPlanes[2].Normalize();

    // right
    guardPlanes[3].Set( origin, frustrum[5], frustrum[6]);
    guardPlanes[3].Normalize();

    // top
    guardPlanes[4].Set( origin, frustrum[4], frustrum[5]);
    guardPlanes[4].Normalize();

    // bottom
    guardPlanes[5].Set( origin, frustrum[6], frustrum[7]);
    guardPlanes[5].Normalize();

    // fixup direction of plane equations
    U32 i;
    for (i = 0 ; i < 8; i++)
    {
      origin += frustrum[i];
    }
    // all plane equation normals must point towards 'origin'
    // which is the center of all the frustrum points
    origin *= 0.125f;

    for (i = 0; i < 6 ; i++)
    {
      if (guardPlanes[i].Evalue( origin) < 0.0f)
      {
        // change sign of plane equation
        guardPlanes[i] *= -1.0f;
      }
    }

    // real planes
    //
    z = nearPlane;
    if (isIso)
    {
      kx = 1.0f * invProjX;
      ky = 1.0f * invProjY;
    }
    else
    {
      kx = (z * projMatrix.frontw + projMatrix.positw) * invProjX;
      ky = (z * projMatrix.frontw + projMatrix.positw) * invProjY;
    }

    frustrum[0].x = (-halfWidth)  * kx;
    frustrum[0].y = (-halfHeight) * ky;
    frustrum[0].z = z;

    frustrum[1].x = ( halfWidth)  * kx;
    frustrum[1].y = frustrum[0].y;
    frustrum[1].z = z;

    frustrum[2].x = frustrum[1].x;
    frustrum[2].y = ( halfHeight) * ky;
    frustrum[2].z = z;

    frustrum[3].x = frustrum[0].x;
    frustrum[3].y = frustrum[2].y;
    frustrum[3].z = z;

    z  = farPlane * zoom;
    if (isIso)
    {
      kx = 1.0f * invProjX;
      ky = 1.0f * invProjY;
    }
    else
    {
      kx = (z * projMatrix.frontw + projMatrix.positw) * invProjX;
      ky = (z * projMatrix.frontw + projMatrix.positw) * invProjY;
    }

    frustrum[4].x = (-halfWidth)  * kx;
    frustrum[4].y = (-halfHeight) * ky;
    frustrum[4].z = z;

    frustrum[5].x = ( halfWidth)  * kx;
    frustrum[5].y = frustrum[4].y;
    frustrum[5].z = z;

    frustrum[6].x = frustrum[5].x;
    frustrum[6].y = ( halfHeight) * ky;
    frustrum[6].z = z;

    frustrum[7].x = frustrum[4].x;
    frustrum[7].y = frustrum[6].y;
    frustrum[7].z = z;

    origin.Set( 0.0, 0.0, 0.0);

    // near
    planes[0].Set( frustrum[0], frustrum[1], frustrum[2]);
    planes[0].Normalize();

    // far
    planes[1].Set( frustrum[4], frustrum[5], frustrum[6]);
    planes[1].Normalize();

    // left
    planes[2].Set( origin, frustrum[4], frustrum[7]);
    planes[2].Normalize();

    // right
    planes[3].Set( origin, frustrum[5], frustrum[6]);
    planes[3].Normalize();

    // top
    planes[4].Set( origin, frustrum[4], frustrum[5]);
    planes[4].Normalize();

    // bottom
    planes[5].Set( origin, frustrum[6], frustrum[7]);
    planes[5].Normalize();

    // fixup direction of plane equations
    for (i = 0 ; i < 8; i++)
    {
      origin += frustrum[i];
    }
    // all plane equation normals must point towards 'origin'
    // which is the center of all the frustrum points
    origin *= 0.125f;

    for (i = 0; i < 6 ; i++)
    {
      if (planes[i].Evalue( origin) < 0.0f)
      {
        // change sign of plane equation
        planes[i] *= -1.0f;
      }
    }
  }
  if (Vid::curCamera == this)
  {
    Vid::SetCamera( *this, TRUE);
  }
}
//----------------------------------------------------------------------------

Bool Camera::SetupView( const Area<F32> & mirrorRect, F32 y, Area<F32> & mRect)
{
  mRect = mirrorRect;

  if (rect.Width() && rect.Height())
  {
    halfHeight = ((F32) rect.Height()) * 0.5f;
    halfWidth  = ((F32) rect.Width())  * 0.5f;
    aspectHW   = halfHeight / halfWidth;

    if (!isIso)
    {
      projMatrix.SetProjection( nearPlane, farPlane * zoom, fov / zoom, aspectHW);

      if (Vid::renderState.status.clipVis)
      {
        halfHeight -= (F32) 64 * aspectHW;
        halfWidth  -= (F32) 64;
      }
      aspectHW   = halfHeight / halfWidth;

      invProjX = aspectHW / ( projMatrix.right.x * halfHeight);
      invProjY = 1 / (-projMatrix.up.y * halfHeight);
    }
    else
    {
      if (Vid::renderState.status.clipVis)
      {
        halfHeight -= (F32) 64 * aspectHW;
        halfWidth  -= (F32) 64;
      }
      aspectHW   = halfHeight / halfWidth;

      invProjX = 1 / ( projMatrix.right.x * halfHeight);
      invProjY = 1 / (-projMatrix.up.y * halfHeight);
    }
    invDepth  = 1.0f / (farPlane - nearPlane);

    F32 proj23 = projMatrix.frontw == 0 ? 1 : projMatrix.frontw;
    invProjX *= proj23;
    invProjY *= proj23;

    VertexTL verts[4], clipV[88];
    verts[0].vv.x = mRect.p0.x;
    verts[0].vv.y = y;
    verts[0].vv.z = mRect.p0.z;

    verts[1].vv.x = mRect.p1.x;
    verts[1].vv.y = y;
    verts[1].vv.z = mRect.p0.z;

    verts[2].vv.x = mRect.p1.x;
    verts[2].vv.y = y;
    verts[2].vv.z = mRect.p1.z;

    verts[3].vv.x = mRect.p0.x;
    verts[3].vv.y = y;
    verts[3].vv.z = mRect.p1.z;

    VertexTL * v, * ve = verts + 4;
    for (v = verts; v < ve; v++)
    {
      viewMatrix.Transform( v->vv);
    }

    // generate on screen water verts
    U16 clipI[88];
    U32 vCount = 4, iCount = 6;
    Vid::Clip::ToBuffer( clipV, clipI, verts, vCount, Vid::rectIndices, iCount);

    // generate on screen water rectangle
    mRect.Set( F32_MAX, F32_MAX, -F32_MAX, -F32_MAX);
    ve = clipV + vCount;
    for (v = clipV; v < ve; v++)
    {
      v->vv.x -= Origin().x;
      v->vv.y -= Origin().y;

      mRect.p0.x = Min<F32>( mRect.p0.x, v->vv.x);
      mRect.p1.x = Max<F32>( mRect.p1.x, v->vv.x);

      // onscreen y's are sign reversed
      mRect.p0.y = Min<F32>( mRect.p0.y, v->vv.y);
      mRect.p1.y = Max<F32>( mRect.p1.y, v->vv.y);
    }

    // real planes
    //
    F32 kx, ky, z = nearPlane;
    if (isIso)
    {
      kx = 1.0f * invProjX;
      ky = 1.0f * invProjY;
    }
    else
    {
      kx = (z * projMatrix.frontw + projMatrix.positw) * invProjX;
      ky = (z * projMatrix.frontw + projMatrix.positw) * invProjY;
    }

    frustrum[0].x = (-halfWidth)  * kx;
    frustrum[0].y = (-halfHeight) * ky;
    frustrum[0].z = z;

    frustrum[2].x = ( halfWidth)  * kx;
    frustrum[2].y = ( halfHeight) * ky;
    frustrum[2].z = z;

    Area<F32> cRect( mRect.p0.x * kx, mRect.p0.y * ky, mRect.p1.x * kx, mRect.p1.y * ky);
    
    frustrum[0].x = Max<F32>( cRect.p0.x, frustrum[0].x);
    frustrum[2].x = Min<F32>( cRect.p1.x, frustrum[2].x);

    // onscreen y's are sign reversed
    frustrum[0].y = Max<F32>( cRect.p0.y, frustrum[0].y);
    frustrum[2].y = Min<F32>( cRect.p1.y, frustrum[2].y);

    frustrum[1].x = frustrum[2].x;
    frustrum[1].y = frustrum[0].y;
    frustrum[1].z = z;

    frustrum[3].x = frustrum[0].x;
    frustrum[3].y = frustrum[2].y;
    frustrum[3].z = z;

    z  = farPlane * zoom;
    if (isIso)
    {
      kx = 1.0f * invProjX;
      ky = 1.0f * invProjY;
    }
    else
    {
      kx = (z * projMatrix.frontw + projMatrix.positw) * invProjX;
      ky = (z * projMatrix.frontw + projMatrix.positw) * invProjY;
    }

    frustrum[4].x = (-halfWidth)  * kx;
    frustrum[4].y = (-halfHeight) * ky;
    frustrum[4].z = z;

    frustrum[6].x = ( halfWidth)  * kx;
    frustrum[6].y = ( halfHeight) * ky;
    frustrum[6].z = z;

    cRect.Set( mRect.p0.x * kx, mRect.p0.y * ky, mRect.p1.x * kx, mRect.p1.y * ky);

    frustrum[4].x = Max<F32>( cRect.p0.x, frustrum[4].x);
    frustrum[6].x = Min<F32>( cRect.p1.x, frustrum[6].x);

    // onscreen y's are sign reversed
    frustrum[4].y = Max<F32>( cRect.p0.y, frustrum[4].y);
    frustrum[6].y = Min<F32>( cRect.p1.y, frustrum[6].y);

    frustrum[5].x = frustrum[6].x;
    frustrum[5].y = frustrum[4].y;
    frustrum[5].z = z;

    frustrum[7].x = frustrum[4].x;
    frustrum[7].y = frustrum[6].y;
    frustrum[7].z = z;

    Vector origin( 0.0, 0.0, 0.0);

    // near
    planes[0].Set( frustrum[0], frustrum[1], frustrum[2]);
    planes[0].Normalize();

    // far
    planes[1].Set( frustrum[4], frustrum[5], frustrum[6]);
    planes[1].Normalize();

    // left
    planes[2].Set( origin, frustrum[4], frustrum[7]);
    planes[2].Normalize();

    // right
    planes[3].Set( origin, frustrum[5], frustrum[6]);
    planes[3].Normalize();

    // top
    planes[4].Set( origin, frustrum[4], frustrum[5]);
    planes[4].Normalize();

    // bottom
    planes[5].Set( origin, frustrum[6], frustrum[7]);
    planes[5].Normalize();

    // fixup direction of plane equations
    for (U32 i = 0 ; i < 8; i++)
    {
      origin += frustrum[i];
    }
    // all plane equation normals must point towards 'origin'
    // which is the center of all the frustrum points
    origin *= 0.125f;

    for (i = 0; i < 6 ; i++)
    {
      if (planes[i].Evalue( origin) < 0.0f)
      {
        // change sign of plane equation
        planes[i] *= -1.0f;
      }
    }
  }
  if (Vid::curCamera == this)
  {
    Vid::SetCamera( *this, TRUE);
  }

  mRect.p0.x += Origin().x;
  mRect.p0.y += Origin().y;
  mRect.p1.x += Origin().x;
  mRect.p1.y += Origin().y;

  return frustrum[6].x < frustrum[4].x
      || frustrum[4].x > frustrum[6].x
      || frustrum[6].y > frustrum[4].y
      || frustrum[4].y < frustrum[6].y ? FALSE : TRUE;
}
//----------------------------------------------------------------------------

Bool Camera::SetupMatrix()
{
  Vid::SetCameraTransform( WorldMatrix());
  viewMatrix = Vid::Math::viewMatrix;

	pyramid[0] = WorldMatrix().Position();
	WorldMatrix().Transform( &pyramid[1], &frustrum[4], 4);

  // possible meters-in-view
  //
	boxMin = boxMax = pyramid[0];

	for (U32 i = 1; i < 5; i++)
	{
		if (boxMin.x > pyramid[i].x)
    {
			boxMin.x = pyramid[i].x;
    }
		if (boxMax.x < pyramid[i].x)
    {
			boxMax.x = pyramid[i].x;
    }
		if (boxMin.z > pyramid[i].z)
    {
			boxMin.z = pyramid[i].z;
    }
		if (boxMax.z < pyramid[i].z)
    {
			boxMax.z = pyramid[i].z;
    }
#if 0
		if (boxMin.y > pyramid[i].y)
    {
			boxMin.y = pyramid[i].y;
    }
		if (boxMax.y < pyramid[i].y)
    {
      boxMax.y = pyramid[i].y;
    }
#endif
  }

#ifdef DOOCCULSION
	if (Occlusion)
  {
		Occlusion->Setup();
	}
#endif

  return TRUE;
}
//-----------------------------------------------------------------------------

void Camera::ClearBorders( Color color)
{
  color;
#if 1
  Vid::ClearBack( 0);
#else
  if (rect.p0.y > last.p0.y)
  {
    Area<S32> r = last;
    r.p1 = rect.p0;
    Vid::ClearBack( color, &r);
  }
  if (rect.p1.y < last.p1.y)
  {
    Area<S32> r;
    r.p0 = rect.p1;
    r.p1 = last.p1;
    Vid::ClearBack( color, &r);
  }
  if (clearCount > 0)
  {
    clearCount--;
  }
#endif
}
//-----------------------------------------------------------------------------

void Camera::SetWorldRecurse( const Matrix &world)
{
  FamilyNode::SetWorldRecurse( world);
  
  SetupMatrix();
}
//-----------------------------------------------------------------------------

void Camera::SetWorldRecurseRender( const Matrix &world, FamilyState *stateArray)
{
  CalcWorldMatrix( world);

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode *node;
  while ((node = kids++) != NULL)
  {
    node->SetWorldRecurseRender( statePtr->WorldMatrix(), stateArray);
  }
  
  SetupMatrix();
}
//-----------------------------------------------------------------------------

#ifdef DOOCCULSION
void Camera::CreateOcclusion()
{
	if (Occlusion)
  {
		return ;
	}
	Occlusion = new OcclusionClass(this);

#if 0
	//now that everything is initilize try to create the occlusion camera
    Camera *OcclusionCam = NULL;
	try {
		// Attent to create a harware occlusion camera
		OcclusionCam = new CameraOcclusionHardware( "occlusion");
	}	catch (char *Message)
  {
		LOG_DIAG( (Message) );
		
		// No hardware occlusion support create a software emulation
		// occlusion
		OcclusionCam = new CameraOcclusion( "occlusion");
		if (!OcclusionCam)
    {
			ERR_FATAL( ("can't create OcclusionCamera") );
		}
	}
#endif
}
//-----------------------------------------------------------------------------

void Camera::ReleaseOcclusion()
{
	if (!Occlusion)
  {
		return;
	}
	delete Occlusion;
}
//-----------------------------------------------------------------------------

Bool Camera::SetOcclusion (Bool OnOff)
{
	if (Occlusion)
  {
	  return Occlusion->SetOcclusion(OnOff);
  }
	return FALSE;
}
//-----------------------------------------------------------------------------

void Camera::RenderToOcclusion(OCCLUSION_PLANE_DATA *Context)
{
	if (Occlusion)
  {
		Occlusion->RenderToOcclusion(Context);		
	}
}
//-----------------------------------------------------------------------------

void Camera::RenderToOcclusion(MeshEnt *Ent) 
{
	if (Occlusion)
  {
		Occlusion->RenderToOcclusion(Ent);		
	}
}		
//-----------------------------------------------------------------------------
#endif
