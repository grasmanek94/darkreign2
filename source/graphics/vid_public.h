///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_public.h
//
// 12-FEB-2000
//

#ifndef __VIDPUBLIC_H
#define __VIDPUBLIC_H

#include "vertex.h"
#include "vid_heap.h" // vid, vid_decl, vid_clip
#include "vid_math.h"
#include "bucket.h"
#include "tranbucket.h"
//-----------------------------------------------------------------------------

extern HRESULT dxError;
extern char *GetErrorString( HRESULT error);

#if	(defined DEVELOPMENT) || (defined DOLOGDXERROR)
#define LOG_DXERR( fmt)                             \
	if (dxError)                                      \
  {                                                 \
		LOG_ERR( fmt );                                 \
    LOG_ERR( ("...%s", GetErrorString( dxError)) ); \
  }
#else
#define LOG_DXERR( fmt) 
#endif
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Mirror
  {
    void Init();
    void Done();

    Bool Start();
    U32  LoopStart();
    void LoopStop();
    void Stop();
  };
  //-----------------------------------------------------------------------------

  extern U32                  extraFog;

  // statistics
  extern U32                  indexCount;
  extern U32                  texMemPerFrame;
  extern U32                  totalTexMemory;
  extern Bitmap *             turtleTex;
  extern Bool                 showTexSwap;
  extern Bool                 firstRun;

  extern Area<F32>            clipRect;

	// direct X
  extern SurfaceDD            front;
	extern DirectDD             ddx;
	extern Direct3D             d3d;
	extern DeviceD3D            device;
	extern ViewPortDescD3D      viewDesc;

  extern Material *           defMaterial;
  extern Material *           blackMaterial;
  extern Camera *             curCamera;
  
  extern List<Pix>            zbuffFormatList;
	extern S32						      curZbuffFormat;

  // mode change callback function
  typedef void (MODECHANGEPROC)(void);
  MODECHANGEPROC * SetModeChangeCallback( MODECHANGEPROC * proc);

  extern BucketMan            bucket;
	extern TranBucketMan  	    tranbucket;
	extern BucketMan *          currentBucketMan;

  // handy indices
  extern U16                  rectIndices[12];

	// function members
	Bool InitDD( Bool noPick = FALSE);
	Bool InitD3D( U32 driver);
	inline Bool InitD3D()
	{
		return InitD3D( CurDD().curDriver);
	}
	Bool RenderFlush();
  Bool RenderBegin();
  Bool RenderEnd();
  Bool SetRenderState( Bool checkInit = TRUE);
  void ReportMode( U32 report = 3);
  U32  FreeVidMem( U32 report = FALSE);
  void OnActivate( Bool active);

	void ReleaseDD();
	void ReleaseD3D();
	void ReleaseDX();

  // all these functions return the previous state
  //
  // update vars
  //
  Bool SetTextureState( Bool doTexture);
  Bool SetFogState( Bool fogOn);  
  Bool SetDitherState( Bool doDither);
  Bool SetSpecularState( Bool doSpecular);

  void SetFogDepth(  F32 depth);
  void SetFarPlane(  F32 farPlane);
  void SetNearPlane( F32 _nearPlane);
  Bool SetZBufferState( Bool doZBuffer);
  Bool SetZWriteState( Bool doZWrite);
  Bool SetAlphaState( Bool doAlpha);

  enum ShadeFlags
  {
    shadeWIRE         = 1,
    shadeFLAT         = 2,
    shadeGOURAUD      = 3,
  };
  U32  SetShadeState( U32 flags);

  enum FilterFlags
  {
    filterFILTER      = 1,
    filterMIPMAP      = 2,
    filterMIPFILTER   = 4,
  };
	U32  SetFilterState( U32 flags, S32 stage = 0);

  inline U32 SetFilterState( Bool filter, Bool mipmap, Bool mipfilter, S32 stage = 0)
  {
    U32 f  = filter    ? filterFILTER    : 0;
        f |= mipmap    ? filterMIPMAP    : 0;
        f |= mipfilter ? filterMIPFILTER : 0;

    return SetFilterState( f, stage);
  }

  U32  SetSrcBlendState( U32 flags);
  U32  SetDstBlendState( U32 flags);
  void SetTextureFactor( Color color);
  U32  SetTexBlendState( U32 flags, U32 stage = 0);
  U32  SetTexWrapState(  U32 flags, U32 stage = 0);

  // these are in vidcmd.cpp
  // don't update vars
  //
  Bool SetTextureStateI( Bool doTexture);
  Bool SetFogStateI( Bool fogOn);  
  Bool SetDitherStateI( Bool doDither);
  Bool SetSpecularStateI( Bool doSpecular);
  U32  SetShadeStateI( U32 flags);
	U32  SetFilterStateI( U32 flags, S32 stage = 0);

  inline U32 SetFilterStateI( Bool filter, Bool mipmap, Bool mipfilter, S32 stage = 1)
  {
    U32 f  = filter    ? filterFILTER    : 0;
        f |= mipmap    ? filterMIPMAP    : 0;
        f |= mipfilter ? filterMIPFILTER : 0;

    return SetFilterStateI( f, stage);
  }

  // internal only
  //
  Bool SetPerspectiveStateI( Bool on);
  Bool SetAntiAliasStateI( Bool on);
  Bool SetEdgeAntiAliasStateI( Bool on);
  Bool SetColorKeyStateI( Bool on);

  inline Camera & CurCamera()
  {
    return *curCamera;
  }
  Bool SetCamera( Camera & cam, Bool force = FALSE);

  inline void SetProjTransform( F32 n, F32 f, F32 fv)
  {
    CurCamera().SetProjTransform( n, f, fv);
  }
  void SetWorldTransform(     const Matrix &mat );

  void SetMaterialDX( const Material * mat);
  inline void SetMaterial( const Material * mat)
  {
    mat;
#ifndef DODXLEANANDGRUMPY
    if (renderState.status.dxTL)
    {
      SetMaterialDX( mat);
    }
#endif
  }
  Bool SetTexture( Bitmap * tex, U32 stage = 0, U32 blend = RS_BLEND_DEF);

  void SetFogColor( F32 r, F32 g, F32 b);
  void SetFogRange( F32 min, F32 max, F32 density);
  inline void SetFogRange()
  {
    SetFogRange( renderState.fogMin, renderState.fogMax, renderState.fogDensity);
  }
  void SetAmbientColor( F32 r, F32 g, F32 b);
  void GetAmbientColor( F32 &r, F32 &g, F32 &b);

  void SetCullStateD3D( Bool doCull);
  void SetFogColorD3D( U32 fogcolor);

  inline Pix & ZBufferFormat()
  {
    return *zbuffFormatList[curZbuffFormat];
  }
  inline void ClearBack( Color color = 0, Area<S32> * rect = NULL)
  {
    backBmp.Clear( color, rect);
  }

  void ClipScreen();
  void ClipRestore();

	inline void SetBucketPrimitiveType(PRIMITIVE_TYPE primitive_type)
	{
    BucketMan::SetPrimitiveType(primitive_type);
	}
	inline void SetBucketVertexType(VERTEX_TYPE vertex_type)
	{
    BucketMan::SetVertexType(vertex_type);
	}
	inline void SetTranslucent(Bool translucent)
	{
    currentBucketMan = (translucent) ? &tranbucket : &bucket;
	}

	inline void SetBucketPrimitiveDesc(
		PRIMITIVE_TYPE primitive_type,
		VERTEX_TYPE	vertex_type,
		U32 flags)
	{
    SetTranslucent( FALSE);   // reset in case material and texture are NULL
    BucketMan::SetPrimitiveDesc(primitive_type, vertex_type, flags);
  }

  inline void SetBucketFlags(U32 flags)
	{
    BucketMan::SetFlags(flags);
	}

	void SetBucketMaterialProc( const Material * material);
  inline void SetBucketMaterial( const Material * material)
  {
    material;
#ifndef DODXLEANANDGRUMPY
    SetBucketMaterialProc( material);
#else
#ifdef DOMATERIAL
    BucketMan::SetMaterial( material);
#endif
#endif
  }

  // stage 0 MUST be set first!!!
  //
	void SetBucketTexture( const Bitmap * texture, Bool translucent = FALSE, U32 stage = 0, U32 blend = RS_BLEND_DEF);

	inline void ClearBucketTextures()
	{
    BucketMan::ClearTextures();
	}		

	inline void SetTranBucketZNorm( F32 z, U16 sort = sortNORMAL0)    // z is normalized
	{
		tranbucket.SetZNorm(z);
    BucketMan::SetTag1( sort);
	}
	inline void SetTranBucketZ( F32 z, U16 sort = sortNORMAL0)        // z in view space
	{
		tranbucket.SetZ(z);
    BucketMan::SetTag1( sort);
	}
	inline void SetTranBucketZMax( U16 sort = sortNORMAL0)
	{
		tranbucket.SetZ( CurCamera().FarPlane());
    BucketMan::SetTag1( sort);
	}
	inline void SetTranBucketZMaxPlus( U16 sort = sortNORMAL0)
	{
		tranbucket.SetZ( CurCamera().FarPlane() + 1.0f);
    BucketMan::SetTag1( sort);
	}

	inline void SetTranBucketMaxZ(F32 max_z)
	{
		tranbucket.SetMaxZ(max_z);
	}
	inline void SetTranBucketMinZ(F32 min_z)
	{
		tranbucket.SetMinZ(min_z);
	}

	inline void SetTranMaxBucketCount(U16 max_bucket_count)
	{
		tranbucket.SetMaxBucketCount(max_bucket_count);
	}

	inline Bucket * LockIndexedPrimitiveMem( void ** vMem, U32 vCount, U16 ** iMem, U32 iCount, const void *id = (const void *)0xcdcdcdcd)
	{
		return currentBucketMan->LockIndexedPrimitiveMem(vMem, vCount, iMem, iCount, id);
	}

	inline void UnlockIndexedPrimitiveMem( U32 vCount, U32 iCount, Bool doOffset = TRUE)
	{
		currentBucketMan->UnlockIndexedPrimitiveMem( vCount, iCount, doOffset);
	}

	inline Bucket * LockPrimitiveMem( void ** vMem, U32 vCount, const void *id = (const void *)0xcdcdcdcd)
	{
		return currentBucketMan->LockPrimitiveMem(vMem, vCount, id);
	}

	inline void UnlockPrimitiveMem( U32 vCount)
	{
		currentBucketMan->UnlockPrimitiveMem( vCount);
	}

  void SetBucket( BucketLock & bucky, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF);

  Bool LockBucket( BucketLock & bucky, U32 _controlFlags, const void * id = (const void *) 0xcdcdcdcd, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF);
  Bool LockBucket( BucketLock & bucky, U32 _controlFlags, U32 vCount, U32 iCount, const void * id = (const void *) 0xcdcdcdcd, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF);
  void UnLockBucket( BucketLock & bucky);

  Bool LockBucket( BucketLock & bucky, U32 _controlFlags, U32 clipFlags = clipNONE, const void * id = (const void *) 0xcdcdcdcd, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF);
  Bool UnLockBucket( BucketLock & bucky, U32 clipFlags, const void * id = (const void *) 0xcdcdcdcd);

  inline void FlushBuckets ()
  {
  	bucket.Flush(TRUE);
  }

  inline void FlushTranBuckets()
  {
	  tranbucket.Flush(TRUE);
  }

  inline void FlushTranBuckets( const Bitmap * texture)
  {
	  tranbucket.FlushTex( texture, TRUE);
  }

  void RenderRectangleOutline( const Area<F32> & rect, Color color, U32 blend = RS_BLEND_GLOW, U16 sorting = sortDEBUG0, F32 vz = 0, F32 rhw = 1);
  inline void RenderRectangleOutline( const Vector & pos, F32 size, Color color, U32 blend = RS_BLEND_GLOW, U16 sorting = sortDEBUG0, F32 vz = 0, F32 rhw = 1)
  {
    Area<F32> rect( pos.x - size, pos.y - size, pos.x + size, pos.y + size);
    RenderRectangleOutline( rect, color, blend, sorting, vz, rhw);
  }
  void RenderRectangle( const Area<F32> & rect, Color color, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sorting = sortDEBUG0, F32 vz = 0, F32 rhw = 1, Bool immediate = FALSE);
  inline void RenderRectangle( const Vector & pos, F32 size, Color color, Bitmap * tex = NULL, U32 blend = RS_BLEND_GLOW, U16 sorting = sortDEBUG0, F32 vz = 0, F32 rhw = 1, Bool immediate = FALSE)
  {
    Area<F32> rect( pos.x - size, pos.y - size, pos.x + size, pos.y + size);
    RenderRectangle( rect, color, tex, blend, sorting, vz, rhw, immediate);
  }
  inline void RenderRectangle( const Area<S32> & inRect, Color color, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sorting = sortDEBUG0, F32 vz = 0, F32 rhw = 1, Bool immediate = FALSE)
  {
    Area<F32> rect( (F32) inRect.p0.x, (F32) inRect.p0.y, (F32) inRect.p1.x, (F32) inRect.p1.y); 
    RenderRectangle( rect, color, tex, blend, sorting, vz, rhw, immediate);
  }

  void RenderSprite( Bool doFog, const Vector & pos, F32 size, const Bitmap * texture, Color color, U32 blend = RS_BLEND_GLOW, U16 sorting = sortSMOKE0, const Vector & rotate = Matrix::I.right);
  void RenderSpriteProjected( Bool doFog, const Vector & pos, F32 vz, F32 rhw, F32 size, const Bitmap * texture, Color color, U32 blend = RS_BLEND_GLOW, U16 sorting = sortSMOKE0, const Vector & rotate = Matrix::I.right);
  void RenderFlareSprite( Bool doFog, const Vector &pos, F32 size, const Bitmap * texture, Color color, U32 blend = RS_BLEND_GLOW, U16 sorting = sortLIGHT0, const Vector & rotate = Matrix::I.right);
  void RenderBeam( Bool doFog, const Vector * points, U32 pointCount, F32 radius, F32 zpos, const Bitmap * texture, Color color, U32 blend = RS_BLEND_ADD, U16 sorting = sortLIGHT0, F32 u0 = 0.0f, F32 du = 1.0f, Bool taper = TRUE, Color * colorA = NULL, F32 * radiusA = NULL);
  void RenderBeamOriented( Bool doFog, const Vector * points, U32 pointCount, const Vector & orientation, F32 zpos, const Bitmap * texture, Color color, U32 blend = RS_BLEND_ADD, U16 sorting = sortLIGHT0, F32 u0 = 0.0f, F32 du = 1.0f, Bool taper = TRUE, Color * colorA = NULL, F32 * radiusA = NULL);

  Bool DrawPrimitive( 
		PRIMITIVE_TYPE prim_type,
		VERTEX_TYPE vert_type,
		LPVOID verts,
		DWORD vert_count,
		DWORD flags);

	Bool DrawFanStripPrimitive( 
		PRIMITIVE_TYPE prim_type,
		VERTEX_TYPE vert_type,
		LPVOID verts,
		DWORD vert_count,
		DWORD flags);

	Bool DrawIndexedPrimitive( 
		PRIMITIVE_TYPE prim_type,
		VERTEX_TYPE vert_type,
		LPVOID verts,
		DWORD vert_count,
		LPWORD indices,
		DWORD index_count,
		DWORD flags);

  DWORD FlagsToBitDepth( DWORD flags);

  enum ClearFlags
  {
    clearZBUFFER = D3DCLEAR_ZBUFFER,
    clearBACK    = D3DCLEAR_TARGET,
    clearSTENCIL = D3DCLEAR_STENCIL,
  };

  inline void RenderClear( U32 clearFlags, Color color, Area<S32> * rect = NULL)
  {
    color;

    Area<S32> temp;
    if (!rect)
    {
      temp.SetSize( viewDesc.dwX, viewDesc.dwY, viewDesc.dwX + viewDesc.dwWidth, viewDesc.dwY + viewDesc.dwHeight);
      rect = &temp;
    }

    dxError = Vid::device->Clear( 1UL, (LPD3DRECT) rect, clearFlags, color, 1, 0);
	  LOG_DXERR( ("Vid::ClearD3D: viewport->Clear2") );
  }
  inline void RenderClear( U32 clearFlags)
  {
    RenderClear( clearFlags, renderState.fogColor);
  }
  inline void RenderClear()
  {
    RenderClear( renderState.clearFlags);
  }

  void ValidateBlends();
  Bool ValidateBlend( U32 blend, U32 stage = 0);

	void SetCameraTransform( const Matrix & mat );
	void SetViewTransform(   const Matrix & mat );
  void SetModelViewVector( const Vector & worldPos);
  void Setup( const Camera & camera);

  void SetWorldTransform_D3D( const Matrix &mat);
	void SetViewTransform_D3D(  const Matrix & mat);
  void SetProjTransform_D3D(  const Matrix & mat);

#ifdef __DO_XMM_BUILD
  void SetModelProjTransformXmm( const Matrix &mat);
  void AllocXmm();
  void FreeXmm();
#endif

  Bool SetTextureDX( const Bitmap * tex, U32 stage = 0, U32 blend = RS_BLEND_DEF);

	void InitBuckets();
	void InitBuckets( U32 count, U32 size, F32 ratio, Bool flush, U32 tcount, U32 tsize, F32 tratio);
	void DoneBuckets();

  Bool AreVerticesInRange( const VertexTL * vertex, S32 vertexCount, const U16 * index, S32 indexCount, U32 clipFlags, Bool guard = TRUE);

  Bool SetGamma( S32 val);

  void CDECL LogPerf( const char * format, ...);

};
//----------------------------------------------------------------------------

#endif		// __VIDPUBLIC_H

