//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// camera.h
//
// 08-SEP-1998
//

#ifndef _CAMERA_H
#define _CAMERA_H
//-----------------------------------------------------------------------------

const F32 STARTNEARPLANE	= 2.0f;
const F32 STARTFARPLANE		= 2000.0f;
const F32 STARTFOV			  = 45.0f;

class Camera : public FamilyNode
{
#ifdef DOOCCULSION
	friend class OcclusionClass;
#endif
  friend struct Bounds;

protected:
  // viewport
  //
  Area<S32> 		              rect, last, saveRect;     // used view relative to client
  U32                         clearCount;
  F32                         x0, y0, x1, y1;     // window rect ratios

  F32                         nearPlane, farPlane, fov, zoom, maxZ;
  F32                         saveNear, saveFar, saveFov, saveZoom;
  Matrix                      projMatrix;
  F32                         halfHeight, halfWidth, aspectHW;
  F32                         invDepth, invProjX, invProjY;

  Plane                       planes[6];
  Plane                       guardPlanes[6];
  Vector                      frustrum[8];

  // camera
  //
	Matrix                      viewMatrix, tranMatrix, saveMatrix;
	Vector                      pyramid[9];
	Vector                      boxMin, boxMax;

  FamilyState                 state;

  U32                         isIso       : 1;
  
  // private setup
  //
  Bool SetupMatrix();

#ifdef DOOCCULSION
  OcclusionClass *            Occlusion;

  Bool SetOcclusion (Bool OnOff);
  void ReleaseOcclusion();
  void CreateOcclusion();
//  virtual void ClearOcclusionBuffer() {}
  void RenderToOcclusion(OCCLUSION_PLANE_DATA *Context);
  void RenderToOcclusion(MeshEnt *Ent);		
#endif

public:

	void ClearData();

  Camera::Camera()
  {
    ClearData();
  }
  Camera( const char *name);
  virtual ~Camera();

  void SetupView();

  // familynode virtuals
  //
  virtual void SetWorldRecurse( const Matrix & world);
  virtual void SetWorldRecurseRender( const Matrix & world, FamilyState * stateArray);

	// test bounds against view frustrum
  //
  // origin (world.posit) in world coords
  //
  inline U32 BoundsTestOrigin( const Vector & world, Bounds & bounds, Vector * viewPos = NULL)
  {
    return BoundsTestOrigin( Matrix( world), bounds, viewPos);
  }
  U32 BoundsTestOrigin( const Matrix & world, Bounds & bounds, Vector * viewPos = NULL);

  // origin (bounds.offset) in model coords
  //
  inline U32 BoundsTest( const Vector & world, Bounds & bounds, Vector * viewPos = NULL)
  {
    return BoundsTest( Matrix( world), bounds, viewPos);
  }
  U32 BoundsTest( const Matrix & world, Bounds & bounds, Vector * viewPos = NULL);

  // sphere only
  //
  inline U32 BoundsTestSphere( const Vector & world, Bounds & bounds, Vector * viewPos = NULL)
  {
    return BoundsTestSphere( Matrix( world), bounds, viewPos);
  }
  U32 BoundsTestSphere( const Matrix & world, Bounds & bounds, Vector * viewPos = NULL);

  // box only
  //
  inline U32 BoundsTestBox( const Vector & world, Bounds & bounds, Vector * viewPos = NULL)
  {
    return BoundsTestBox( Matrix( world), bounds, viewPos);
  }
  U32 BoundsTestBox( const Matrix & world, Bounds & bounds, Vector * viewPos = NULL);

  // legacy
  //
  inline U32 SphereTest( const Matrix & world, const Bounds & bounds, Vector * viewPos = NULL)
  {
    return SphereTest( world.posit, bounds.Radius(), viewPos);
  }
  U32 SphereTest( const Vector & origin, F32 radius, Vector * viewPos = NULL);
	U32 SphereTestCamera( const Vector & origin, F32 radius);

  // possible meters-in-view
  //
	void GetVisibleRect( Area<S32> & vRect)
	{
		vRect.p0.x = Utils::FtoL(boxMin.x);
		vRect.p1.x = Utils::FtoL(boxMax.x);
		vRect.p0.y = Utils::FtoL(boxMin.z);
		vRect.p1.y = Utils::FtoL(boxMax.z);
	}
  void OnModeChange();

  inline void Save()
  {
    SaveMatrix();
    saveRect = rect;
    saveNear = nearPlane;
    saveFar  = farPlane;
    saveFov  = fov;
    saveZoom = zoom;
  }
  inline void Restore()
  {
    SetProjTransform( saveNear, saveFar, saveFov);
    SetZoom( saveZoom);
    Setup( saveRect);
    RestoreMatrix();
  }

  inline void SaveMatrix()
  {
    saveMatrix = WorldMatrix();
  }
  inline void RestoreMatrix()
  {
    SetWorldAll( saveMatrix);
  }
  void Mirror( const Plane & plane);

  // setup
  //
	void Setup( const Area<S32> & inRect);
	void SetProjTransform( F32 n, F32 f, F32 fv);
  void SetProjTransformIso( F32 n, F32 f, F32 fv, F32 w, F32 h);
  void ClearBorders( Color color);
  Bool SetupView( const Area<F32> & mirrorRect, F32 y, Area<F32> & mRect);
  void Setup( const Camera & cam);

  void operator=( const Camera & cam)
  {
    Setup( cam);
  }

	inline void SetFarPlane( F32 f)
	{
		SetProjTransform( nearPlane, f, fov);
	}
	inline void SetNearPlane( F32 n)
	{
		SetProjTransform( n, farPlane, fov);
	}
  void SetZoom( F32 z);

  // query
  //
	Matrix & GetProjTransform();
	
  const Matrix & GetSaveMatrix() const
  {
    return saveMatrix;
  }

  inline F32 FarPlane() const
  {
    return farPlane;
  }
  inline F32 NearPlane() const
  {
    return nearPlane;
  }
  inline F32 FOV() const
  {
    return fov;
  }
  inline F32 Zoom() const
  {
    return zoom;
  }
  inline F32 MaxZ() const
  {
    return maxZ;
  }
  inline F32 ProjInvConstX() const
  {
    return invProjX;
  }
  inline F32 ProjInvConstY() const
  {
    return invProjY;
  }
  inline const Area<S32> & ViewRect() const
  {
    return rect;
  }
  inline Point<F32> Origin() const
  {
    return Point<F32>( (F32) rect.MidX(), (F32) rect.MidY());
  }
  inline F32 Width() const
  {
    return (F32) rect.Width();
  }
  inline F32 Height() const
  {
    return (F32) rect.Height();
  }
  inline F32 HalfWidth() const
  {
    return halfWidth;
  }
  inline F32 HalfHeight() const
  {
    return halfHeight;
  }
  inline const Matrix & ProjMatrix() const
  {
    return projMatrix;
  }
	inline const Matrix & ViewMatrix() const 
	{
		return viewMatrix;
	}
	inline const Vector * Frustrum() const
	{
		return frustrum;
	}
	inline const Vector * Pyramid() const
	{
		return pyramid;
	}
	inline const Plane * Planes() const
	{
		return planes;
	}
	inline const Plane * GuardPlanes() const
	{
		return guardPlanes;
	}
  inline F32 AspectHW() const
  {
    return aspectHW;
  } 
  inline F32 InvDepth() const
  {
    return invDepth;
  }
};
//-----------------------------------------------------------------------------

#endif		// _CAMERA_H
