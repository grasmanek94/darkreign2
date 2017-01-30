///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// terrain.cpp     DR2 terrain system
//
// 04-MAY-1998
//

#include "vid_private.h"
#include "vid_cmd.h"
#include "terrain_priv.h"
#include "light_priv.h"
#include "random.h"
#include "console.h"
#include "terrain.h"
#include "statistics.h"
#include "meshent.h"
//----------------------------------------------------------------------------

namespace Terrain
{
  void RotateUVs( const Vector & front, UVPair & uv0, UVPair & uv1, UVPair & uv2)
  {
    Vector f = front;
    f.y =0.0f;
    f.Normalize();
    Matrix m;
    m.ClearData();
    m.SetFromFront( f);

    F32 du = (F32) fabs(uv2.u + uv0.u) * 0.5f;
    F32 dv = (F32) fabs(uv2.v + uv0.v) * 0.5f;
    uv0.u -= du;
    uv0.v -= dv;
    uv1.u -= du;
    uv1.v -= dv;
    uv2.u -= du;
    uv2.v -= dv;

#if 0
    m.Rotate( uv0);
    m.Rotate( uv1);
    m.Rotate( uv2);
#else
    m.UnRotate( uv0);
    m.UnRotate( uv1);
    m.UnRotate( uv2);
#endif

    uv0.u += du;
    uv0.v += dv;
    uv1.u += du;
    uv1.v += dv;
    uv2.u += du;
    uv2.v += dv;
  }
  //----------------------------------------------------------------------------

  Bucket * RenderShadow( MeshEnt & ent, const GETHEIGHTPROCPTR getHeightProc) // = GetHeight)
  {
    ShadowInfo & si = ent.shadowInfo;
    return RenderGroundSprite( si.p, si.radx, si.rady, si.clipFlags, si.z, si.texture, si.color, si.blend, si.uv0, si.uv1, si.uv2, getHeightProc, Vid::sortSURFACE0);
  }
  //----------------------------------------------------------------------------

  Bool BoundsTestShadow( MeshEnt & ent, const GETHEIGHTPROCPTR getHeightProc, const FINDFLOORPROCPTR findFloorProc) // = 255, = GetHeight, = FindFloor 
  {
    U32 type = ent.shadowType;
    if (type == MeshRoot::shadowSEMILIVE || type == MeshRoot::shadowLIVE)
    {
      type = ent.shadowTexture ? MeshRoot::shadowSEMILIVE : MeshRoot::shadowOVAL;
      ent.shadowInfo.texture = ent.shadowTexture;
    }
    else if (type == MeshRoot::shadowGENERIC)
    {
      type = ent.Root().shadowTexture ? MeshRoot::shadowGENERIC : MeshRoot::shadowOVAL;
      ent.shadowInfo = ent.Root().shadowInfo;
      ent.shadowInfo.texture = ent.Root().shadowTexture;
    }
    Bool retVal = BoundsTestShadow( ent.shadowInfo, ent.WorldMatrixRender(), ent.Root().shadowRadius, type, ent.baseColor.a, getHeightProc, findFloorProc);

/*
    if (*Vid::Var::varNightLight && !Vid::Light::isSunUp)
    {
      ent.shadowInfo.color = Vid::Command::nightLightColor;
    }
*/
    return retVal;
  }
  //----------------------------------------------------------------------------

  Bool BoundsTestShadow( ShadowInfo & si, const Matrix & world, F32 radius, U32 shadowType, U32 translucency, const GETHEIGHTPROCPTR getHeightProc, const FINDFLOORPROCPTR findFloorProc) // = 255, = GetHeight, = FindFloor 
  {
    getHeightProc;

    shadowType = Min<U32>( shadowType, (U32) Vid::renderState.status.shadowType);

    if (shadowType == MeshRoot::shadowLIVE || shadowType == MeshRoot::shadowSEMILIVE)
    {
      shadowType = si.texture && si.texture != shadowTex ? MeshRoot::shadowSEMILIVE : MeshRoot::shadowOVAL;
    }
    else if (shadowType == MeshRoot::shadowGENERIC)
    {
      shadowType = si.texture && si.texture != shadowTex ? MeshRoot::shadowGENERIC : MeshRoot::shadowOVAL;
    }
    else
    {
      shadowType = MeshRoot::shadowOVAL;
    }

    Vector pos = world.posit;

    F32 fade = 1;
    Vector vcam;
    Vid::Math::viewMatrix.Transform( vcam, pos);
    
    F32 dist = vcam.z - Vid::renderState.shadowFadeDist;
    if (dist > 0)
    {
      fade = Max<F32>( 0, 1 - (dist * Vid::renderState.shadowFadeDepthInv));
    }
    U8 alpha = U8( Utils::FtoL( fade * (F32) translucency * (F32) Vid::Light::shadowColor.a) >> 8);
    if (alpha < Vid::renderState.shadowFadeCutoff || !MeterOnMap( pos.x, pos.z))
    {
      si.clipFlags = clipNONE;
      return FALSE;
    }

    // does the shadow hit the terrain?
    //
    Vector p = pos, v = Vid::Light::shadowMatrix.front;
    F32 y = (*findFloorProc)( p.x, p.z);

/*
    if ((*Vid::Var::varNightLight && !Vid::Light::isSunUp) || shadowType == MeshRoot::shadowSEMILIVE)
    {
      p.y = y;
      v.Set( 0, -1.2f, 0);
    }
    else
*/
    if (shadowType >= MeshRoot::shadowSEMILIVE)
    {
      p.y = y;
      v.Set( 0, -1.2f, 0);
    }
    else
    {
      if (p.y < y + 0.1f)
      {
        p.y = y + 0.1f;
      }
      if (!Intersect( p, v, 1.0f, findFloorProc))
      {
        si.clipFlags = clipNONE;
        return FALSE;
      }
    }

    // calculate stretched radius
    F32 stretch = -1 / v.y;
/*
    if (stretch > 3.0f)
    {
      stretch = 3.0f;
    }
*/
    F32 radx = radius * stretch;

    // bounds check
    Vector viewpos;
    si.clipFlags = Vid::CurCamera().SphereTest( p, radx, &viewpos);
    if (si.clipFlags == clipOUTSIDE)
    {
      return FALSE;
    }

    // setup
    si.z = viewpos.z;
    si.color = Vid::Light::shadowColor;
    si.color.a = alpha;
    si.blend = RS_BLEND_DEF;

    if (shadowType >= MeshRoot::shadowSEMILIVE)
    {
      // live cast shadow
      si.p = p + si.p1;
      si.uv0.Set( 0, 0);
      si.uv1.Set( 1, 0);
      si.uv2.Set( 1, 1);
    }
    else if (shadowType == MeshRoot::shadowGENERIC)
    {
      // non-animating shadow

      si.uv0.Set( 0, 0);
      si.uv1.Set( 1, 0);
      si.uv2.Set( 1, 1);
      RotateUVs( world.front, si.uv0, si.uv1, si.uv2);

//      si.rady = si.radx = Max<F32>(si.radxRender, si.radyRender);
      si.radx = si.radxRender * stretch;
      si.rady = si.radyRender;

    // calculate delta radius
      F32 dr = (si.radx - si.radxRender);

      // shift shadow by delta radius
      v.y = 0.0f;
      si.p = p + si.p1 + v * dr;
    }
    else
    {
      si.p = p;

      // calculate delta radius
      F32 dr = radx - radius;

      // fake shadow
      si.texture = shadowTex;

      si.radx = radx;
      si.rady = radx;

      // calculate u stretch
      stretch = si.radx / radius - 1.0f;

      v.x = (F32) fabs( v.x);
      v.z = (F32) fabs( v.z);

      // shift shadow by delta radius
      v.y = 0.0f;
      v.Normalize();
      si.p += v * dr;

      si.uv0.Set( 0.0f - stretch * v.z, 0.0f - stretch * v.x);
      si.uv1.Set( 1.0f + stretch * v.z, 0.0f - stretch * v.x);
      si.uv2.Set( 1.0f + stretch * v.z, 1.0f + stretch * v.x);
    }
    return TRUE;
  }
  //----------------------------------------------------------------------------

  Bucket * RenderShadow( const Matrix & world, F32 radius, U32 shadowType, U32 translucency, const GETHEIGHTPROCPTR getHeightProc, const FINDFLOORPROCPTR findFloorProc) // = 0xfffffff, = GetHeight, = FindFloor 
  {
    return RenderShadow( *Vid::Light::sun, world, radius, shadowType, translucency, getHeightProc, findFloorProc);
  }
  //----------------------------------------------------------------------------

  Bucket * RenderShadow( const Vid::Light::Obj & light, const Matrix & world, F32 radius, U32 shadowType, U32 translucency, const GETHEIGHTPROCPTR getHeightProc, const FINDFLOORPROCPTR findFloorProc) // = 0xfffffff, = GetHeight, = FindFloor 
  {
    light;

    Bucket * bucket = NULL;
    ShadowInfo si;
    if (BoundsTestShadow( si, world, radius, shadowType, translucency, getHeightProc, findFloorProc))
    {
      bucket = RenderShadow( si, getHeightProc);
    }
    return bucket;
  }
  //----------------------------------------------------------------------------

  Bool BoundsTestLight( ShadowInfo & si, const Vid::Light::Obj & light, const GETHEIGHTPROCPTR getHeightProc, const FINDFLOORPROCPTR findFloorProc) // = GetHeight, = FindFloor 
  {
    getHeightProc;

    si.p = light.WorldMatrix().posit;

    if (!MeterOnMap( si.p.x, si.p.z))
    {
      si.clipFlags = clipNONE;
      return FALSE;
    }
    U32 type = light.GetType();

    F32 y = (*findFloorProc)( si.p.x, si.p.z);
    if (y > si.p.y)
    {
      si.p.y = y;
      type = Vid::Light::lightPOINT;
    }

    Vector v;
    F32 dvr, radius, stretch;
    F32 range = light.GetRange();
    if (type == Vid::Light::lightPOINT)
    {
      dvr = si.p.y - y;
      if (dvr > range)
      {
        si.clipFlags = clipNONE;
        return FALSE;
      }
      radius = dvr * 0.707f * 0.5f;

      range *= range;
      dvr *= dvr;

      v = Matrix::I.up * -1.0f;

      // calculate stretched radius
      stretch = -1.0f;
    }
    else if (type == Vid::Light::lightSPOT)
    {
      v = light.WorldMatrix().front;

      // does the light hit the terrain?
      //
      if (v.y > -F32_EPSILON || !Intersect( si.p, v, 1.0f, findFloorProc))
      {
        si.clipFlags = clipNONE;
        return FALSE;
      }

      Vector dv = light.WorldMatrix().posit - si.p;
      range *= range;
      dvr = dv.Magnitude2();
      if (dvr > range)
      {
        si.clipFlags = clipNONE;
        return FALSE;
      }
      radius = (F32) sqrt( dvr) * light.GetCosTheta() * 0.5f;
        
      // calculate stretched radius
      stretch = -1.0f / v.y;
      if (stretch > 3.0f)
      {
        stretch = 3.0f;
      }
    }
    else
    {
      return FALSE;
    }

    si.radx = radius * stretch;

    // bounds check
    Vector viewpos;
    si.clipFlags = Vid::CurCamera().SphereTest( si.p, si.radx, &viewpos);
    if (si.clipFlags == clipOUTSIDE)
    {
      return FALSE;
    }
    si.z = viewpos.z;

    // calculate delta radius
    F32 dr = si.radx - radius;

    // shift shadow by delta radius
    v.y = 0.0f;
    v.Normalize();
    si.p += v * dr;

    // calculate u stretch
    stretch = si.radx / radius - 1.0f;

    F32 vx = (F32) fabs( v.x);
    F32 vz = (F32) fabs( v.z);

    si.color = light.GetBeamColor();
//    S32 translucency = Utils::FtoL( dvr / range * 255.0f);
//    si.color.a = U8((translucency * si.color.a) >> 8);
//    si.color.a = (U8)Max<U32>( 255, si.color.a << 1);
    si.texture = shadowTex;
    si.blend = RS_BLEND_ADD;

    si.uv0.Set( 0.0f - stretch * vz, 0.0f - stretch * vx);
    si.uv1.Set( 1.0f + stretch * vz, 0.0f - stretch * vx);
    si.uv2.Set( 1.0f + stretch * vz, 1.0f + stretch * vx);

    return TRUE;
  }
  //----------------------------------------------------------------------------

  Bucket * RenderLight( const Vid::Light::Obj &light, const GETHEIGHTPROCPTR getHeightProc, const FINDFLOORPROCPTR findFloorProc) // = 0xfffffff, = GetHeight, = FindFloor 
  {
    Bucket * bucket = NULL;
    ShadowInfo si;
    if (BoundsTestLight( si, light, getHeightProc, findFloorProc))
    {
      bucket = RenderShadow( si, getHeightProc);
    }
    return bucket;
  }
  //----------------------------------------------------------------------------

  // draw a ground-hugging sprite; rotate the texture first
  //
  Bucket * RenderGroundSprite( const Vector & origin, F32 radx, F32 rady, const Vector & front, const Bitmap *texture, Color color, U32 blend, UVPair uv0, UVPair uv1, UVPair uv2, const GETHEIGHTPROCPTR getHeightProc, U16 sorting) // UVPair(0.0f,1.0f), UVPair(1.0f,1.0f), UVPair(1.0f,0.0f), GetHeight, sortGROUND0
  {
    RotateUVs( front, uv0, uv1, uv2);
    return RenderGroundSprite( origin, radx, rady, texture, color, blend, uv0, uv1, uv2, getHeightProc, sorting);
  }
  //----------------------------------------------------------------------------

  // draw a ground-hugging sprite
  //
  Bucket * RenderGroundSprite( const Vector & origin, F32 radx, F32 rady, const Bitmap *texture, Color color, U32 blend, UVPair uv0, UVPair uv1, UVPair uv2, const GETHEIGHTPROCPTR getHeightProc, U16 sorting) // UVPair(0.0f,1.0f), UVPair(1.0f,1.0f), UVPair(1.0f,0.0f), GetHeight, sortGROUND0
  {
    Vector viewpos;
    U32 clipFlags = Vid::CurCamera().SphereTest( origin, rady, &viewpos);
    if (clipFlags == clipOUTSIDE)
    {
      return NULL;
    }
    return RenderGroundSprite( origin, radx, rady, clipFlags, viewpos.z, texture, color, blend, uv0, uv1, uv2, getHeightProc, sorting);
  }
  //----------------------------------------------------------------------------

  // draw a ground-hugging sprite
  //
  Bucket * RenderGroundSprite( const Vector &origin, F32 radx, F32 rady, U32 clipFlags, F32 viewz, const Bitmap *texture, Color color, U32 blend, UVPair uv0, UVPair uv1, UVPair uv2, const GETHEIGHTPROCPTR getHeightProc, U16 sorting) // UVPair(0.0f,1.0f), UVPair(1.0f,1.0f), UVPair(1.0f,0.0f), GetHeight
  {
  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    clipFlags;
    viewz;

    if (texture)
    {
      uv0.u += texture->UVShiftWidth();
      uv0.v += texture->UVShiftHeight();
      uv1.u += texture->UVShiftWidth();
      uv1.v += texture->UVShiftHeight();
      uv2.u += texture->UVShiftWidth();
      uv2.v += texture->UVShiftHeight();
    }

 	  // set the primitive description
	  Vid::SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_TEXCLAMP | blend);

	  // set the world transform matrix
	  Vid::SetWorldTransform(Matrix::I);

    // set material, texture, and force translucency
    Vid::SetBucketMaterial( Vid::defMaterial);
	  Vid::SetBucketTexture( texture, TRUE, 0, RS_TEXCLAMP | blend);
//	  Vid::SetTranBucketZ(viewz);
    Vid::SetTranBucketZMax( sorting);

	  // sprite corners in world coordinates
	  F32 meterX0 = origin.x - radx + OffsetX();
	  F32 meterZ0 = origin.z - rady + OffsetZ();
    F32 meterX1 = origin.x + radx + OffsetX();
	  F32 meterZ1 = origin.z + rady + OffsetZ();

	  // get corners of the grid covered by the sprite
    // get grid larger than sprite; adjust texture coords
    // round down to grid point
    // round up to grid point

    U16 cw = Utils::FP::SetRoundDownMode();
	  S32 cellX0 = Utils::FastFtoL( meterX0 * CellPerMeter());
	  S32 cellZ0 = Utils::FastFtoL( meterZ0 * CellPerMeter());

    Utils::FP::SetRoundUpMode();
	  S32 cellX1 = Utils::FastFtoL( meterX1 * CellPerMeter());
	  S32 cellZ1 = Utils::FastFtoL( meterZ1 * CellPerMeter());

    Utils::FP::RestoreMode( cw);

	  // dimensions of sprite in grid squares
	  S32 sizex = (cellX1 - cellX0 + 1);
	  S32 sizez = (cellZ1 - cellZ0 + 1);
    if (U32(sizex * sizez + 1) >= Vid::renderState.maxVerts || U32(sizex * sizez * 6) >= Vid::renderState.maxIndices)     // FIXME
    {
      LOG_WARN( ("Shadow too big") );
      return NULL;
    }
#if 0
    // verify buffer size
    ASSERT();
#endif

	  // lock primitive memory
    VertexTL * vertmem;
    U16 * indexmem;
    U32 heapSize = Vid::Heap::ReqVertex( &vertmem, &indexmem);

    VertexTL * pvert = vertmem;
    U16 * pindex = indexmem;

    // delta meters per grid
	  F32 dm = (F32) MeterPerCell();

	  F32 dx12 = meterX0 - meterX0;
	  F32 dx02 = meterX0 - meterX1;
    F32 dz12 = meterZ0 - meterZ1;
	  F32 dz02 = meterZ0 - meterZ1;
	  F32 dx = dx12 * dz02 - dx02 * dz12;
	  if (dx == 0.0f)
    {
      dx = F32_EPSILON;
    }
	  dx = 1.0f / dx;
	  F32 dz = -dx;

    // delta texture coords
	  F32 du12 = (F32) (uv1.u - uv2.u);
	  F32 du02 = (F32) (uv0.u - uv2.u);
	  F32 dudx = (du12 * dz02 - du02 * dz12) * dx;
	  F32 dudz = (du12 * dx02 - du02 * dx12) * dz;

	  F32 dv12 = (F32) (uv1.v - uv2.v);
	  F32 dv02 = (F32) (uv0.v - uv2.v);
	  F32 dvdx = (dv12 * dz02 - dv02 * dz12) * dx;
	  F32 dvdz = (dv12 * dx02 - dv02 * dx12) * dz;

	  // texture coordinates of southwest grid point
    dx = (cellX0 * dm - meterX0);
    dz = (cellZ0 * dm - meterZ0);
    F32 u = uv0.u + dx * dudx + dz * dudz;
	  F32 v = uv0.v + dx * dvdx + dz * dvdz;

	  // texture coordinate delta per grid
	  dudx *= dm;
	  dvdx *= dm;
	  dudz *= dm;
	  dvdz *= dm;

	  // sprite grid corners in world coordinates
	  meterX0 = cellX0 * dm - OffsetX();
	  meterZ0 = cellZ0 * dm - OffsetZ();
    meterX1 = cellX1 * dm - OffsetX();
	  meterZ1 = cellZ1 * dm - OffsetZ();

    // generate vertices on grid cell points
    S32 cellX, cellZ;
	  for (cellZ = cellZ0; cellZ <= cellZ1; cellZ++, meterZ0 += dm, u += dudz, v += dvdz)
	  {
      F32 xx = meterX0;
      F32 uu = u;
      F32 vv = v;

		  for (cellX = cellX0; cellX <= cellX1; cellX++, xx += dm, uu += dudx, vv += dvdx)
		  {
        // allow cells at Width and Height
        //
        if (cellX < 0 || cellX > (S32)CellWidth() || cellZ < 0 || cellZ > (S32)CellHeight())
        {
          pvert++;
          continue;
        }
			  pvert->vv.x     = xx;
        pvert->vv.y     = (*getHeightProc)( cellX, cellZ);
			  pvert->vv.z     = meterZ0;
			  pvert->diffuse  = color;
			  pvert->specular = RGBA_MAKE(0x00, 0x00, 0x00, 0xFF);
			  pvert->u        = uu;
			  pvert->v        = vv;
			  pvert++;
		  }
    }

	  // generate indices
	  U32 offset = 0;

    U32 sizex1 = sizex + 1;

	  for (cellZ = cellZ0; cellZ < cellZ1; cellZ++)
	  {
		  for (cellX = cellX0; cellX < cellX1; cellX++)
		  {
        if (cellX < 0 || cellX >= (S32)CellWidth() || cellZ < 0 || cellZ >= (S32)CellHeight())
        {
          // cell off map
          offset++;
          continue;
        }
			  // upper left triangle
			  *pindex++ = (U16) offset;
			  *pindex++ = (U16)(offset + sizex);
			  *pindex++ = (U16)(offset + sizex1);

			  // lower right triangle
			  *pindex++ = (U16) offset;
			  *pindex++ = (U16)(offset + sizex1);
			  *pindex++ = (U16)(offset + 1);

			  // advance the base offset
			  offset++;
      }
		  // advance the base offset
		  offset++;
    }

    Bucket * bucket = NULL;
    // submit for projection and clipping
  #if 0
    if (clipFlags == clipNONE)
    {
      bucket = Vid::CurCamera().ProjectNoClipBias( NULL, vertmem, pvert - vertmem, indexmem, pindex - indexmem);
    }
    else
  #endif
    {
      bucket = Vid::ProjectClip( vertmem, pvert - vertmem, indexmem, pindex - indexmem);
    }

    Vid::Heap::Restore( heapSize);

  #ifdef DOSTATISTICS
    Statistics::groundSpriteTris = Statistics::groundSpriteTris + Statistics::tempTris;
  #endif

    return bucket;
  }
  //----------------------------------------------------------------------------

  // draw a water-hugging sprite; rotate the texture first
  //
  Bucket * RenderWaterSprite( const Vector & origin, F32 radius, const Vector &front, const Bitmap *texture, Color color, U32 blend, UVPair uv0, UVPair uv1, UVPair uv2, U16 sorting) // UVPair(0.0f,1.0f), UVPair(1.0f,1.0f), UVPair(1.0f,0.0f)
  {
    Vector f = front;
    f.y =0.0f;
    f.Normalize();
    Matrix m;
    m.ClearData();
    m.SetFromFront( f);

    F32 du = (F32) fabs(uv2.u + uv0.u) * 0.5f;
    F32 dv = (F32) fabs(uv2.v + uv0.v) * 0.5f;
    uv0.u -= du;
    uv0.v -= dv;
    uv1.u -= du;
    uv1.v -= dv;
    uv2.u -= du;
    uv2.v -= dv;

    m.Rotate( uv0);
    m.Rotate( uv1);
    m.Rotate( uv2);

    uv0.u += du;
    uv0.v += dv;
    uv1.u += du;
    uv1.v += dv;
    uv2.u += du;
    uv2.v += dv;

    return RenderWaterSprite( origin, radius, texture, color, blend, uv0, uv1, uv2, sorting);
  }
  //----------------------------------------------------------------------------

  // draw a water-hugging sprite
  //
  Bucket * RenderWaterSprite( const Vector & origin, F32 radius, const Bitmap *texture, Color color, U32 blend, UVPair uv0, UVPair uv1, UVPair uv2, U16 sorting) // UVPair(0.0f,1.0f), UVPair(1.0f,1.0f), UVPair(1.0f,0.0f)
  {
    if (Vid::renderState.status.mirrorIn)
    {
      // don't reflect water sprites
      return NULL;
    }
    Camera & cam = Vid::CurCamera();

    Vector viewpos;
    U32 clipFlags = cam.SphereTest( origin, radius, &viewpos);
    if (clipFlags == clipOUTSIDE)
    {
      return NULL;
    }
    return RenderWaterSprite( origin, radius, clipFlags, viewpos.z, texture, color, blend, uv0, uv1, uv2, sorting);
  }
  //----------------------------------------------------------------------------
  F32 globalWaterHeight = 0;

  // draw a water-hugging sprite
  //
  Bucket * RenderWaterSprite( const Vector & origin, F32 radius, U32 clipFlags, F32 viewz, const Bitmap *texture, Color color, U32 blend, UVPair uv0, UVPair uv1, UVPair uv2, U16 sorting) // UVPair(0.0f,1.0f), UVPair(1.0f,1.0f), UVPair(1.0f,0.0f)
  {
    F32 height;
    if (!FindWater( origin.x, origin.z, &height))
    {
      return NULL;
    }
    globalWaterHeight = height;

    return RenderGroundSprite( origin, radius, radius, clipFlags, viewz, texture, color, blend, uv0, uv1, uv2, GetHeightWater, sorting);

#if 0
  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    clipFlags;
    viewz;

    if (texture)
    {
      uv0.u += texture->UVShiftWidth();
      uv0.v += texture->UVShiftHeight();
      uv1.u += texture->UVShiftWidth();
      uv1.v += texture->UVShiftHeight();
      uv2.u += texture->UVShiftWidth();
      uv2.v += texture->UVShiftHeight();
    }

 	  // set the primitive description
	  Vid::SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_TEXCLAMP | blend);

	  // set the world transform matrix
	  Vid::SetWorldTransform(Matrix::I);

    // set material, texture, and force translucency
    Vid::SetBucketMaterial( Vid::defMaterial);
	  Vid::SetBucketTexture( texture, TRUE, 0, RS_TEXCLAMP | blend);
//	  Vid::SetTranBucketZ(viewz);
    Vid::SetTranBucketZMax( Vid::sortSURFACE0);

	  // sprite corners in world coordinates
	  F32 meterX0 = origin.x - radius;
	  F32 meterZ0 = origin.z - radius;
    F32 meterX1 = origin.x + radius;
	  F32 meterZ1 = origin.z + radius;

	  // lock primitive memory
    VertexTL * vertmem;
    U32 heapSize = Vid::Heap::ReqVertex( &vertmem);
    VertexTL * verts = vertmem;

    verts->vv.x = meterX0;
    verts->vv.y = height;
    verts->vv.z = meterZ0;
    verts->uv   = uv0;
    verts->diffuse  = color;
    verts->specular = 0xff000000;
    verts++;

    verts->vv.x = meterX1;
    verts->vv.y = height;
    verts->vv.z = meterZ0;
    verts->uv   = uv1;
    verts->diffuse  = color;
    verts->specular = 0xff000000;
    verts++;

    verts->vv.x = meterX1;
    verts->vv.y = height;
    verts->vv.z = meterZ1;
    verts->uv   = uv2;
    verts->diffuse  = color;
    verts->specular = 0xff000000;
    verts++;

    verts->vv.x = meterX0;
    verts->vv.y = height;
    verts->vv.z = meterZ1;
    verts->uv.u = uv0.u + uv2.u - uv1.u;
    verts->uv.v = uv2.v + uv0.v - uv1.v;
    verts->diffuse  = color;
    verts->specular = 0xff000000;

    Bucket * bucket = NULL;
/*
    // submit for projection and clipping
    if (clipFlags == clipNONE)
    {
      bucket = Vid::ProjectNoClip( vertmem, 4, Vid::rectIndices, 6);
    }
    else
*/
    {
      bucket = Vid::ProjectClip( vertmem, 4, Vid::rectIndices, 6);
    }

    Vid::Heap::Restore( heapSize);

  #ifdef DOSTATISTICS
    Statistics::groundSpriteTris = Statistics::groundSpriteTris + Statistics::tempTris;
  #endif

    return bucket;
#endif
  }
  //----------------------------------------------------------------------------

}
//----------------------------------------------------------------------------
