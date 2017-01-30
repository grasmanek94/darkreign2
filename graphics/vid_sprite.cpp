///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// vidrend.cpp      non-setup 3d rendering specific Vid stuff
//
// 22-APR-1998      John Cooke
//

#include "vid_public.h"
#include "statistics.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  //
  // Rectangle
  //
  // Draws a rectangle flat on the screen
  //
  void RenderRectangleOutline( const Area<F32> & rect, Color color, U32 blend, U16 sorting, F32 vz, F32 rhw) // = RS_BLEND_GLOW, = sortEFFECT
  {
	  // set the primitive description
	  SetBucketPrimitiveDesc( PT_LINELIST, FVF_TLVERTEX,
//    SetBucketPrimitiveDesc( PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_NOINDEXED | blend);

	  // set material
    SetBucketMaterial( defMaterial);
    SetBucketTexture( NULL, color < 255 ? TRUE:FALSE, 0, blend);

	  // set bucket depth
	  SetTranBucketZNorm( vz, sorting);

  	// lock primitive memory
	  VertexTL * vertmem;
	  if (!LockPrimitiveMem( (void **)&vertmem, 8))
	  {
      return;
    }

	  // top left corner
	  vertmem[0].vv.x     = rect.p0.x;
	  vertmem[0].vv.y     = rect.p0.y;
	  vertmem[0].vv.z     = vz;
	  vertmem[0].rhw      = rhw;
	  vertmem[0].diffuse  = color;
	  vertmem[0].specular = 0xff000000;
	  vertmem[0].u        = 0.0f;
	  vertmem[0].v        = 0.0f;

	  // top right corner
	  vertmem[1].vv.x     = rect.p1.x;
	  vertmem[1].vv.y     = rect.p0.y;
	  vertmem[1].vv.z     = vz;
	  vertmem[1].rhw      = rhw;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = 0xff000000;
	  vertmem[1].u        = 1.0f;
	  vertmem[1].v        = 0.0f;

    vertmem[2] = vertmem[1];

	  // bottom right corner
	  vertmem[3].vv.x     = rect.p1.x;
	  vertmem[3].vv.y     = rect.p1.y;
	  vertmem[3].vv.z     = vz;
	  vertmem[3].rhw      = rhw;
	  vertmem[3].diffuse  = color;
	  vertmem[3].specular = 0xff000000;
	  vertmem[3].u        = 1.0f;
	  vertmem[3].v        = 1.0f;

    vertmem[4] = vertmem[3];

	  // bottom left corner
	  vertmem[5].vv.x     = rect.p0.x;
	  vertmem[5].vv.y     = rect.p1.y;
	  vertmem[5].vv.z     = vz;
	  vertmem[5].rhw      = rhw;
	  vertmem[5].diffuse  = color;
	  vertmem[5].specular = 0xff000000;
	  vertmem[5].u        = 0.0f;
	  vertmem[5].v        = 1.0f;

    vertmem[6] = vertmem[5];
    vertmem[7] = vertmem[0];

	  // submit the polygons
	  UnlockPrimitiveMem( 8);
  }
  //----------------------------------------------------------------------------

  //
  // Rectangle
  //
  // Draws a rectangle flat on the screen
  //
  void RenderRectangle( const Area<F32> & inRect, Color color, Bitmap * tex, U32 blend, U16 sorting, F32 vz, F32 rhw, Bool immediate)
  {
    // 3d clip
    if (vz < 0 || vz > 1)
    {
      return;
    }
    // 2d clip
    Area<F32> rect = inRect;
    if (!rect.Clip( Vid::clipRect))
    {
      return;
    }
	  VertexTL * vertmem, temp[4];
    U16 * indexmem = 0;

    blend |= RS_TEXCLAMP;

    if (immediate)
    {
      SetTexture( tex, 0, blend);
      vertmem = temp;
    }
    else
    {
	    // set the primitive description
	    SetBucketPrimitiveDesc( PT_TRIANGLELIST, FVF_TLVERTEX,
		    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | blend);

	    // set material
      SetBucketMaterial( defMaterial);
      SetBucketTexture( NULL, color < 255 ? TRUE:FALSE, 0, blend);

	    // set bucket depth
	    SetTranBucketZ(0.0f, sorting);

  	  // lock primitive memory
	    if (!LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6))
	    {
        return;
      }
    }
	  // top left corner
	  vertmem[0].vv.x     = rect.p0.x;
	  vertmem[0].vv.y     = rect.p0.y;
	  vertmem[0].vv.z     = vz;
	  vertmem[0].rhw      = rhw;
	  vertmem[0].diffuse  = color;
	  vertmem[0].specular = 0xff000000;
	  vertmem[0].u        = 0.0f;
	  vertmem[0].v        = 0.0f;

	  // top right corner
	  vertmem[1].vv.x     = rect.p1.x;
	  vertmem[1].vv.y     = rect.p0.y;
	  vertmem[1].vv.z     = vz;
	  vertmem[1].rhw      = rhw;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = 0xff000000;
	  vertmem[1].u        = 1.0f;
	  vertmem[1].v        = 0.0f;

	  // bottom right corner
	  vertmem[2].vv.x     = rect.p1.x;
	  vertmem[2].vv.y     = rect.p1.y;
	  vertmem[2].vv.z     = vz;
	  vertmem[2].rhw      = rhw;
	  vertmem[2].diffuse  = color;
	  vertmem[2].specular = 0xff000000;
	  vertmem[2].u        = 1.0f;
	  vertmem[2].v        = 1.0f;

	  // bottom left corner
	  vertmem[3].vv.x     = rect.p0.x;
	  vertmem[3].vv.y     = rect.p1.y;
	  vertmem[3].vv.z     = vz;
	  vertmem[3].rhw      = rhw;
	  vertmem[3].diffuse  = color;
	  vertmem[3].specular = 0xff000000;
	  vertmem[3].u        = 0.0f;
	  vertmem[3].v        = 1.0f;

    if (immediate)
    {
      Vid::DrawIndexedPrimitive( 
        PT_TRIANGLELIST,
        FVF_TLVERTEX,
        vertmem, 4, Vid::rectIndices, 6, 
        DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);
    }
    else
    {
      Utils::Memcpy( indexmem, Vid::rectIndices, 12);
	    UnlockIndexedPrimitiveMem( 4, 6);
    }
  }
  //----------------------------------------------------------------------------

  void RenderSprite( Bool doFog, const Vector & pos, F32 size, const Bitmap * texture, Color color, U32 blend, U16 sorting, const Vector & rotate) // = TRUE, RS_BLEND_GLOW, sortEFFECT, Matrix::I::right
  {
    Vector p;
    F32 rhw, camz = Vid::ProjectFromWorld( p, pos, rhw);

    // project/scale size
    size = Vid::ProjectRHW( size, rhw);

    RenderSpriteProjected( doFog, p, camz, rhw, size, texture, color, blend, sorting, rotate);
  }
  //----------------------------------------------------------------------------

  void RenderSpriteProjected( Bool doFog, const Vector & pos, F32 camz, F32 rhw, F32 size, const Bitmap *texture, Color color, U32 blend, U16 sorting, const Vector & rotate) // = TRUE, RS_BLEND_GLOW, sortEFFECT, Matrix::I::right
  {
    // 3d clip
    if (pos.z < 0 || pos.z > 1
    // 2d clip
     || pos.x >= clipRect.p1.x || pos.x + size <= clipRect.p0.x
     || pos.y >= clipRect.p1.y || pos.y + size <= clipRect.p0.y)
    {
//      LOG_DIAG(( "Vid::RenderSpriteProjected: z out of range %f", pos.z));
      return;
    }
    blend |= RS_TEXCLAMP;

 	  // set the primitive description
	  SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | blend);

	  // set material
    SetBucketMaterial( defMaterial);
    SetBucketTexture( texture, TRUE, 0, blend);
	  // set bucket depth
	  SetTranBucketZ(camz, sorting);

	  // lock primitive memory
	  VertexTL *vertmem;
	  U16 *index;
	  if (!LockIndexedPrimitiveMem( (void **)&vertmem, 4, &index, 6))
	  {
      return;
    }

    Vector r = rotate;
    r.z = 0;
    r.Normalize();
    r *= size;

	  // get corners of sprite
#if 1
	  F32 x0 = pos.x - size;
	  F32 x1 = pos.x + size;
	  F32 y0 = pos.y - size;
	  F32 y1 = pos.y + size;
#else
	  F32 x0 = pos.x - r.x + r.y;
	  F32 x1 = pos.x + r.x - r.y;
	  F32 y0 = pos.y - r.y - r.x;
	  F32 y1 = pos.y + r.y - r.x;
#endif

	  // top left corner
	  vertmem[0].vv.x     = x0;
	  vertmem[0].vv.y     = y0;
	  vertmem[0].vv.z     = pos.z;
	  vertmem[0].rhw      = rhw;
	  vertmem[0].diffuse  = color;
	  vertmem[0].specular = 0xff000000;
	  vertmem[0].u        = 0.0f;
	  vertmem[0].v        = 0.0f;

    if (doFog)
    {
      vertmem->SetFog();
    }
    
	  // top right corner
	  vertmem[1].vv.x     = x1;
	  vertmem[1].vv.y     = y0;
	  vertmem[1].vv.z     = pos.z;
	  vertmem[1].rhw      = rhw;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = vertmem->specular;
	  vertmem[1].u        = 1.0f;
	  vertmem[1].v        = 0.0f;

	  // bottom right corner
	  vertmem[2].vv.x     = x1;
	  vertmem[2].vv.y     = y1;
	  vertmem[2].vv.z     = pos.z;
	  vertmem[2].rhw      = rhw;
	  vertmem[2].diffuse  = color;
	  vertmem[2].specular = vertmem->specular;
	  vertmem[2].u        = 1.0f;
	  vertmem[2].v        = 1.0f;

	  // bottom left corner
	  vertmem[3].vv.x     = x0;
	  vertmem[3].vv.y     = y1;
	  vertmem[3].vv.z     = pos.z;
	  vertmem[3].rhw      = rhw;
	  vertmem[3].diffuse  = color;
	  vertmem[3].specular = vertmem->specular;
	  vertmem[3].u        = 0.0f;
	  vertmem[3].v        = 1.0f;

    // upper left triangle
	  index[0] = 0;
	  index[1] = 1;
	  index[2] = 2;

	  // lower right triangle
	  index[3] = 0;
	  index[4] = 2;
	  index[5] = 3;

	  // submit the polygons
	  UnlockIndexedPrimitiveMem(4, 6);

#ifdef DOSTATISTICS
    Statistics::spriteTris = Statistics::spriteTris + 2;
#endif
  }
  //----------------------------------------------------------------------------

  void RenderFlareSprite( Bool doFog, const Vector & pos, F32 size, const Bitmap * texture, Color color, U32 blend, U16 sorting, const Vector & rotate) // = RS_BLEND_GLOW, sortEFFECT, Matrix::I::right
  {
    Vector p;
    F32 rhw, camz = Vid::ProjectFromWorld( p, pos, rhw);

    if (p.z < 0 || p.z > 1)
    {
//      LOG_DIAG(( "Vid::RenderSpriteProjected: z out of range %f", pos.z));
      return;
    }
    blend |= RS_TEXCLAMP;

    // project/scale the size
    size = Vid::ProjectRHW( size, rhw);

	  // get corners of sprite
	  F32 x0 = p.x - size;
	  F32 x1 = p.x + size;
	  F32 y0 = p.y - size;
	  F32 y1 = p.y + size;

  // set bucket primitive description
    SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
	    DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | blend);   // FIXME: dx clip

	  // set bucket material
    SetBucketMaterial( defMaterial);
    SetBucketTexture( texture, TRUE, 0, blend);

	  // set bucket depth
	  SetTranBucketZ( camz, sorting);

    // lock vertex memory
	  VertexTL *vertmem;
	  U16 *indexmem;
	  if (!LockIndexedPrimitiveMem( (void **)&vertmem, 5, &indexmem, 12))
	  {
      return;
    }

    // middle
	  vertmem[0].vv.x = p.x;
	  vertmem[0].vv.y = p.y;
	  vertmem[0].vv.z = p.z;
    vertmem[0].rhw  = rhw;
	  vertmem[0].diffuse = color;
	  vertmem[0].specular = 0xFF000000;
	  vertmem[0].u = 0.5f;
	  vertmem[0].v = 0.5f;

    if (doFog)
    {
      vertmem->SetFog();
    }

	  vertmem[1].vv.x     = x0;
	  vertmem[1].vv.y     = y0;
	  vertmem[1].vv.z     = p.z;
	  vertmem[1].rhw      = rhw;
	  vertmem[1].diffuse.Modulate( color, 1, 1, 1, .7f);
	  vertmem[1].specular = vertmem->specular;
	  vertmem[1].u        = 0.0f;
	  vertmem[1].v        = 0.0f;

	  // top right corner
	  vertmem[2].vv.x     = x1;
	  vertmem[2].vv.y     = y0;
	  vertmem[2].vv.z     = p.z;
	  vertmem[2].rhw      = rhw;
	  vertmem[2].diffuse  = vertmem[1].diffuse;
	  vertmem[2].specular = vertmem->specular;
	  vertmem[2].u        = 1.0f;
	  vertmem[2].v        = 0.0f;

	  // bottom right corner
	  vertmem[3].vv.x     = x1;
	  vertmem[3].vv.y     = y1;
	  vertmem[3].vv.z     = p.z;
	  vertmem[3].rhw      = rhw;
	  vertmem[3].diffuse  = vertmem[1].diffuse;
	  vertmem[3].specular = vertmem->specular;
	  vertmem[3].u        = 1.0f;
	  vertmem[3].v        = 1.0f;

	  // bottom left corner
	  vertmem[4].vv.x     = x0;
	  vertmem[4].vv.y     = y1;
	  vertmem[4].vv.z     = p.z;
	  vertmem[4].rhw      = rhw;
	  vertmem[4].diffuse  = vertmem[1].diffuse;
	  vertmem[4].specular = vertmem->specular;
	  vertmem[4].u        = 0.0f;
	  vertmem[4].v        = 1.0f;

    Utils::Memcpy( indexmem, rectIndices, 24);

    // submit the polygons
	  UnlockIndexedPrimitiveMem( 5, 12);

#ifdef DOSTATISTICS
    Statistics::spriteTris = Statistics::spriteTris + 4;
#endif
  }
  //----------------------------------------------------------------------------

#if 1

  // render a psuedo 3-D beam clipping at the nearplane 
  // FIXME: farplane
  //
  void RenderBeam( Bool doFog, const Vector * points, U32 pointCount, F32 radius, F32 zpos, const Bitmap *texture, Color color, U32 blend, U16 sorting, F32 u0, F32 du, Bool taper, Color * colorA, F32 * radiusA) // RS_BLEND_ADD, = sortEFFECT, = 0.0f, = 1.0f, = TRUE, = NULL, = NULL
  {
    ASSERT( pointCount > 1);

    if (pointCount < 3)
    {
      taper = FALSE;
    }
/*
    const Vector * pp = points, * ppe = pp + pointCount;
    for ( ; pp < ppe; pp++)
    {
      if (pp->z == 0)
      {
        LOG_DIAG(("pp: %f", pp->z))
      }
    }
*/

    F32 nearplane = Math::nearPlane + 0.1f;

    // setup buckets
    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | blend);

    Vid::SetWorldTransform( Matrix::I);
    SetBucketMaterial( defMaterial);
    SetBucketTexture( texture, TRUE, 0, blend);
    Vid::SetTranBucketZ( zpos, sorting);

    // figure vert and index counts (without taper)
    U32 vcount = pointCount * 2;
    U32 icount = (pointCount - 1) * 6;

    // lock memory
    VertexTL *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem((void **)&vertmem, vcount, &indexmem, icount, points))
    {
      return;
    }

    // loop variables
	  F32 dist, rad;
    U32 j1 = 0;
    VertexTL pVertProj;
    const Vector *tVertP = points;
    Vector tVert, tVertCam, pVert, pVertCam, wid1;
    wid1.z = 0.0f;

    VertexTL *v1 = vertmem;
    U16 *i1 = indexmem;

    if (colorA)
    {
      ASSERT(radiusA);
      color  = *colorA++;
      radius = *radiusA++;
    }

    // setup cam vert
    Vid::TransformFromWorld( tVertCam, *tVertP);
    // setup prev cam vert
    pVertCam = tVertCam;

    // setup projected vert
    Vid::ProjectFromWorld( *v1, *tVertP);
    // fill vertmem
    v1->diffuse  = color;
    v1->specular = 0xff000000;
    v1->u = u0;
    v1->v = 0.5f;
    if (doFog)
    {
      v1->SetFog();
    }
    // save proj vert
    pVertProj = *v1;

    Vector nextCam;
    VertexTL nextTL;
    Vid::TransformFromWorld( nextCam, *(tVertP + 1));

    Bool sTaper = taper;
    if (!sTaper)
    {
      if (nextCam.z < nearplane)
      {
        // this vert behind; prev vert not
        Vector dp = nextCam - pVertCam;
        F32 dzz = (nextCam.z - pVertCam.z);

        F32 t = dzz == 0 ? 0 : (nearplane - pVertCam.z) / dzz;
        nextCam = pVertCam + dp * t;
        nextCam.z = nearplane;
      }
      Vid::ProjectFromCamera_I( nextTL, nextCam);

      // wid1 is 2D cross of vector from this to next
      wid1.y = -(v1->vv.x - nextTL.vv.x);
      wid1.x =   v1->vv.y - nextTL.vv.y;

      // radius for this vert
      rad = Vid::Project( radius, tVertCam.z);

      // adjust wid1
      dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
      dist = (dist == 0.0f) ? 1 : rad / dist;
      wid1 *= dist;

      // top vertmem
      v1->vv -= wid1;
      v1->v = 0.0f;

      // bot vertmem
      v1++;
      *v1 = pVertProj;
      v1->vv += wid1;
      v1->v = 1.0f;
    }

    // setup prev world vert
    pVert = *tVertP;
    // next vert
    tVertP++;
    // setup this vert
    tVert = *tVertP;

    if (colorA)
    {
      ASSERT(radiusA);
      color  = *colorA++;
      radius = *radiusA++;
    }

    // this segment dist
    F32 dd = Vector( tVert - pVert).Magnitude();

    // advance uvScale
    u0 += du * dd;
    F32 pu = u0;

    // setup cam vert
    Vid::TransformFromWorld( tVertCam, tVert);

    Bool dorend = TRUE;
    if (pVertCam.z <= nearplane)
    {
      // prev vert behind nearplane

      if (tVertCam.z < nearplane)
      {
        // this vert behind nearplane
        dorend = FALSE;
      }
      else
      {
        // clip prev to this vert
        Vector dp = tVertCam - pVertCam;
        F32 dzz = (tVertCam.z - pVertCam.z);

        F32 t = dzz == 0 ? 0 : (nearplane - pVertCam.z) / dzz;
        pVertCam = pVertCam + dp * t;

        dp = tVert - pVert;
        pVert = pVert + dp * t;

        Vid::ProjectFromCamera_I( *v1, pVertCam);
        v1->u += du * dd * t;
        if (doFog)
        {
          v1->SetFog();
        }
        if (!sTaper)
        {
          // wid1 is 2D cross of vector from this to next
          Vid::ProjectFromCamera_I( nextTL, tVertCam);
          wid1.y = -(v1->vv.x - nextTL.vv.x);
          wid1.x =   v1->vv.y - nextTL.vv.y;

          // radius for this vert
          rad = Vid::Project( radius, pVertCam.z);    // fixme: use ProjectRHW

          // adjust wid1
          dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
          dist = (dist == 0.0f) ? 1 : rad / dist;
          wid1 *= dist;

          // setup prev proj vert
          pVertProj = *v1;

          // setup the other proj vert
          *(v1 - 1) = *v1;

          // setup position offsets
          v1->vv += wid1;
          v1->v = 1.0f;
          (v1 - 1)->vv -= wid1;
          (v1 - 1)->v = 0;
        }
      }
    }
    else if (tVertCam.z < nearplane)
    {
      // this vert behind; prev vert not
      Vector dp = tVertCam - pVertCam;
      F32 dzz = (tVertCam.z - pVertCam.z);

      F32 t = dzz == 0 ? 0 : (nearplane - pVertCam.z) / dzz;
      tVertCam = pVertCam + dp * t;
      tVertCam.z = nearplane;

      dp  = tVert - pVert;
      tVert = pVert + dp * t;

      pu = pu - du * dd + du * dd * t;
    }
    // setup prev cam vert
    pVertCam = tVertCam;
    // next vertmem
    v1++;

    // setup projected vert
    Vid::ProjectFromCamera_I( *v1, tVertCam);

    // radius for this vert
    rad = Vid::Project( radius, tVertCam.z);

    // wid1 is 2D cross of vector from this to prev
    wid1.y = -(pVertProj.vv.x - v1->vv.x);
    wid1.x =   pVertProj.vv.y - v1->vv.y;

    // adjust wid1
    dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
    dist = (dist == 0.0f) ? 1 : rad / dist;
    wid1 *= dist;

    // top vertmem
    v1->diffuse  = color;
    v1->specular = 0xff000000;
    v1->u = pu;
    v1->v = 0.0f;
    if (doFog)
    {
      v1->SetFog();
    }


    // setup prev projected
    pVertProj = *v1;

    // top vertmem
    v1->vv -= wid1;

    // bot vertmem
    v1++;
    *v1 = pVertProj;
    v1->vv += wid1;
    v1->v = 1.0f;
    if (doFog)
    {
      v1->SetFog();
    }

    // next vert
    tVertP++;

    if (colorA)
    {
      ASSERT(radiusA);
      color  = *colorA++;
      radius = *radiusA++;
    }

    if (sTaper)
    {
      // taper to single tri at the ends
      if (dorend)
      {
        // not clipped; fill indexmem
        *i1++ = (U16)(j1);
        *i1++ = (U16)(j1 + 1);
        *i1++ = (U16)(j1 + 2);
      }
      j1 += 1;
    }
    else
    {
      // 2 tris at the ends
      if (dorend)
      {
        // not clipped; fill indexmem
        *i1++ = (U16)(j1);
        *i1++ = (U16)(j1 + 1);
        *i1++ = (U16)(j1 + 3);
        *i1++ = (U16)(j1);
        *i1++ = (U16)(j1 + 3);
        *i1++ = (U16)(j1 + 2);
      }
      j1 += 2;
    }

    if (pointCount >= 3)
    {
      // loop through the middle points; advancing uvScale
      const Vector *pe = points + pointCount - 1;
      for ( ; tVertP < pe; tVertP++)
      {
        // setup prev
        pVert = tVert;
        // setup this
        tVert = *tVertP;

        dd = Vector( tVert - pVert).Magnitude();

        // advance uvScale
        u0 += du * dd;
        pu = u0;

        // setup cam vert
        Vid::TransformFromWorld( tVertCam, tVert);

        F32 dd = Vector(tVert - pVert).Magnitude();

        // clip
        dorend = TRUE;
        if (pVertCam.z <= nearplane)
        {
          // prev behind

          if (tVertCam.z < nearplane)
          {
            // this behind
            dorend = FALSE;
          }
          else
          {
            // prev behind; this not
            // delta
            Vector dp = tVertCam - pVertCam;
            // parametric delta
            F32 dzz = (tVertCam.z - pVertCam.z);
            F32 t = dzz == 0 ? 0 : (nearplane - pVertCam.z) / dzz;

            // adjust prev cam vert
            pVertCam = pVertCam + dp * t;

            // delta
            dp = tVert - pVert;
            // adjust prev vert
            pVert = pVert + dp * t;

            // setup adjusted screen vert
            Vid::ProjectFromCamera_I( *v1, pVertCam);
            // adjust texture coord
            v1->u += du * dd * t;
            if (doFog)
            {
              v1->SetFog();
            }

            // wid1 is 2D cross of vector from this to next
            Vid::ProjectFromCamera_I( nextTL, tVertCam);
            wid1.y = -(v1->vv.x - nextTL.vv.x);
            wid1.x =   v1->vv.y - nextTL.vv.y;

            // radius for this vert
            rad = Vid::Project( radius, pVertCam.z);

            // adjust wid1
            dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
            dist = (dist == 0.0f) ? 1 : rad / dist;
            wid1 *= dist;

            // reset prev
            pVertProj = *v1;
            // setup other vert
            *(v1 - 1) = *v1;

            // add offsets
            v1->vv += wid1;
            (v1 - 1)->vv -= wid1;
            (v1 - 1)->v = 0;
          }
        }
        else if (tVertCam.z < nearplane)
        {
          // last in front; this behind
          Vector dp = tVertCam - pVertCam;
          F32 dzz = (tVertCam.z - pVertCam.z);

          F32 t = dzz == 0 ? 0 : (nearplane - pVertCam.z) / dzz;
          tVertCam = pVertCam + dp * t;
          tVertCam.z = nearplane;

          dp  = tVert - pVert;
          tVert = pVert + dp * t;

          pu = pu - du * dd + du * dd * t;
        }

        // setup prev cam vert
        pVertCam = tVertCam;
        // next vertmem
        v1++;

        // setup projected vert
        Vid::ProjectFromCamera_I( *v1, tVertCam);

        wid1.y = -(pVertProj.vv.x - v1->vv.x);
        wid1.x =   pVertProj.vv.y - v1->vv.y;

        rad = Vid::Project( radius, tVertCam.z);

        dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
        dist = (dist == 0.0f) ? 1 : rad / dist;
        wid1 *= dist;

        v1->diffuse  = color;
        v1->specular = 0xff000000;
        v1->u = pu;
        v1->v = 0.0f;
        if (doFog)
        {
          v1->SetFog();
        }
        pVertProj = *v1;

        v1->vv -= wid1;

        v1++;
        *v1 = pVertProj;
        v1->vv += wid1;
        v1->v = 1.0f;

        if (dorend)
        {
          *i1++ = (U16)(j1);
          *i1++ = (U16)(j1 + 1);
          *i1++ = (U16)(j1 + 3);
          *i1++ = (U16)(j1);
          *i1++ = (U16)(j1 + 3);
          *i1++ = (U16)(j1 + 2);
        }
        j1 += 2;

        if (colorA)
        {
          ASSERT(radiusA);
          color  = *colorA++;
          radius = *radiusA++;
        }
      }

      pVert = tVert;
      tVert = *tVertP;

      dd = Vector( tVert - pVert).Magnitude();

      // advance uvScale
      u0 += du * dd;
      pu = u0;

      // setup cam vert
      Vid::TransformFromWorld( tVertCam, tVert);

      dorend = TRUE;
      if (pVertCam.z <= nearplane)
      {
        if (tVertCam.z < nearplane)
        {
          dorend = FALSE;
        }
        else
        {
          Vector dp = tVertCam - pVertCam;
          F32 dzz = (tVertCam.z - pVertCam.z);

          F32 t = dzz == 0 ? 0 : (nearplane - pVertCam.z) / dzz;
          pVertCam = pVertCam + dp * t;

          dp = tVert - pVert;
          pVert = pVert + dp * t;

          Vid::ProjectFromCamera_I( *v1, pVertCam);
          if (doFog)
          {
            v1->SetFog();
          }
          v1->u += du * dd * t;

          // wid1 is 2D cross of vector from this to next
          Vid::ProjectFromCamera_I( nextTL, tVertCam);
          wid1.y = -(v1->vv.x - nextTL.vv.x);
          wid1.x =   v1->vv.y - nextTL.vv.y;

          // radius for this vert
          rad = Vid::Project( radius, pVertCam.z);

          // adjust wid1
          dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
          dist = (dist == 0.0f) ? 1 : rad / dist;
          wid1 *= dist;

          pVertProj = *v1;
          *(v1 - 1) = *v1;

          v1->vv += wid1;
          v1->v = 1;
          (v1 - 1)->vv -= wid1;
          (v1 - 1)->v = 0;
        }
      }
      else if (tVertCam.z < nearplane)
      {
        Vector dp = tVertCam - pVertCam;
        F32 dzz = (tVertCam.z - pVertCam.z);

        F32 t = dzz == 0 ? 0 : (nearplane - pVertCam.z) / dzz;
        tVertCam = pVertCam + dp * t;
        tVertCam.z = nearplane;

        dp  = tVert - pVert;
        tVert = pVert + dp * t;

        pu = pu - du * dd + du * dd * t;
      }
      v1++;

      if (dorend)
      {
        // setup prev cam vert
    //    pVertCam = tVertCam;

        // setup projected vert
        Vid::ProjectFromCamera_I( *v1, tVertCam);
        v1->diffuse  = color;
        v1->specular = 0xff000000;
        v1->u = pu;
        if (doFog)
        {
          v1->SetFog();
        }

        if (taper)
        {
          v1->v = 0.5f;

          if (dorend)
          {
            *i1++ = (U16)(j1);
            *i1++ = (U16)(j1 + 1);
            *i1++ = (U16)(j1 + 2);
          }
        }
        else
        {
          v1->v = 0.0f;

          // wid1 is 2D cross of vector from this to prev
          wid1.y = -(pVertProj.vv.x - v1->vv.x);
          wid1.x =   pVertProj.vv.y - v1->vv.y;

          // radius for this vert
          rad = Vid::Project( radius, tVertCam.z);

          // adjust wid1
          dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
          dist = (dist == 0.0f) ? 1 : rad / dist;
          wid1 *= dist;

          // setup prev projected
          pVertProj = *v1;

          // top vertmem
          v1->vv -= wid1;

          // bot vertmem
          v1++;
          *v1 = pVertProj;
          v1->vv += wid1;
          v1->v = 1.0f;

          if (dorend)
          {
            *i1++ = (U16)(j1);
            *i1++ = (U16)(j1 + 1);
            *i1++ = (U16)(j1 + 3);
            *i1++ = (U16)(j1);
            *i1++ = (U16)(j1 + 3);
            *i1++ = (U16)(j1 + 2);
          }
        }
      }
    }
    v1++;

	  if (renderState.status.checkVerts && !AreVerticesInRange( vertmem, v1 - vertmem, indexmem, i1 - indexmem, clipNONE))
    {
      LOG_DIAG(("BEAM: count %d; v %d; i %d", pointCount, v1 - vertmem, i1 - indexmem))
    }

    Vid::UnlockIndexedPrimitiveMem(v1 - vertmem, i1 - indexmem);

#ifdef DOSTATISTICS
    Statistics::spriteTris = Statistics::spriteTris + (i1 - indexmem) / 3;
#endif
  }
  //----------------------------------------------------------------------------

#else

  // render a psuedo 3-D beam clipping at the nearplane 
  // FIXME: farplane
  //
  void RenderBeam( Bool doFog, const Vector * points, U32 pointCount, F32 radius, F32 zpos, const Bitmap *texture, Color color, U32 blend, U16 sorting, F32 u0, F32 du, Bool taper, Color * colorA, F32 * radiusA) // RS_BLEND_ADD, = sortEFFECT, = 0.0f, = 1.0f, = TRUE, = NULL, = NULL
  {
    radiusA;
    colorA;

    F32 nearplane = Math::nearPlane + 0.1f;

    // setup buckets
    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | blend);

    Vid::SetWorldTransform( Matrix::I);
    SetBucketMaterial( defMaterial);
    SetBucketTexture( texture, TRUE, 0, blend);
    Vid::SetTranBucketZ( zpos, sorting);

    // figure vert and index counts (without taper)
    U32 vcount = pointCount * 2;
    U32 icount = (pointCount - 1) * 6;

    // lock memory
    VertexTL *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem((void **)&vertmem, vcount, &indexmem, icount, points))
    {
      return;
    }

    // loop variables
	  F32 dist, rad;
    U32 j1 = 0;
    VertexTL pVertProj;
    const Vector *tVertP = points;
    Vector tVert, tVertCam, pVert, pVertCam, wid1;
    wid1.z = 0.0f;

    VertexTL *v1 = vertmem;
    U16 *i1 = indexmem;

    // setup cam vert
    Vid::TransformFromWorld( tVertCam, *tVertP);
    // setup prev cam vert
    pVertCam = tVertCam;

    // setup projected vert
    Vid::ProjectFromWorld( *v1, *tVertP);
    // fill vertmem
    v1->diffuse  = color;
    v1->specular = 0xff000000;
    v1->u = u0;
    v1->v = 0.5f;
    if (doFog)
    {
      v1->SetFog();
    }
    // save proj vert
    pVertProj = *v1;

    Vector nextCam;
    VertexTL nextTL;
    Vid::TransformFromWorld( nextCam, *(tVertP + 1));

    Bool sTaper = taper;
    if (!sTaper)
    {
      if (nextCam.z < nearplane)
      {
        // this vert behind; prev vert not
        Vector dp = nextCam - pVertCam;
        F32 t = (nearplane - pVertCam.z) / (nextCam.z - pVertCam.z);
        nextCam = pVertCam + dp * t;
        nextCam.z = nearplane;
      }
      Vid::ProjectFromCamera_I( nextTL, nextCam);

      // wid1 is 2D cross of vector from this to next
      wid1.y = -(v1->vv.x - nextTL.vv.x);
      wid1.x =   v1->vv.y - nextTL.vv.y;

      // radius for this vert
      rad = Vid::Project( radius, tVertCam.z);

      // adjust wid1
      dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
      dist = (dist == 0.0f) ? 1 : rad / dist;
      wid1 *= dist;

      // top vertmem
      v1->vv -= wid1;
      v1->v = 0.0f;

      // bot vertmem
      v1++;
      *v1 = pVertProj;
      v1->vv += wid1;
      v1->v = 1.0f;
    }

    // setup prev world vert
    pVert = *tVertP;
    // next vert
    tVertP++;
    // setup this vert
    tVert = *tVertP;

    // this segment dist
    F32 dd = Vector( tVert - pVert).Magnitude();

    // advance uvScale
    u0 += du * dd;
    F32 pu = u0;

    // setup cam vert
    Vid::TransformFromWorld( tVertCam, tVert);

    Bool dorend = TRUE;
    if (pVertCam.z <= nearplane)
    {
      // prev vert behind nearplane

      if (tVertCam.z < nearplane)
      {
        // this vert behind nearplane
        dorend = FALSE;
      }
      else
      {
        // clip prev to this vert
        Vector dp = tVertCam - pVertCam;
        F32 t = (nearplane - pVertCam.z) / (tVertCam.z - pVertCam.z);
        pVertCam = pVertCam + dp * t;

        dp = tVert - pVert;
        pVert = pVert + dp * t;

        Vid::ProjectFromCamera_I( *v1, pVertCam);
        v1->u += du * dd * t;
        if (doFog)
        {
          v1->SetFog();
        }
        if (!sTaper)
        {
          // wid1 is 2D cross of vector from this to next
          Vid::ProjectFromCamera_I( nextTL, tVertCam);
          wid1.y = -(v1->vv.x - nextTL.vv.x);
          wid1.x =   v1->vv.y - nextTL.vv.y;

          // radius for this vert
          rad = Vid::Project( radius, pVertCam.z);    // fixme: use ProjectRHW

          // adjust wid1
          dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
          dist = (dist == 0.0f) ? 1 : rad / dist;
          wid1 *= dist;

          // setup prev proj vert
          pVertProj = *v1;

          // setup the other proj vert
          *(v1 - 1) = *v1;

          // setup position offsets
          v1->vv += wid1;
          v1->v = 1.0f;
          (v1 - 1)->vv -= wid1;
          (v1 - 1)->v = 0;
        }
      }
    }
    else if (tVertCam.z < nearplane)
    {
      // this vert behind; prev vert not
      Vector dp = tVertCam - pVertCam;
      F32 t = (nearplane - pVertCam.z) / (tVertCam.z - pVertCam.z);
      tVertCam = pVertCam + dp * t;
      tVertCam.z = nearplane;

      dp  = tVert - pVert;
      tVert = pVert + dp * t;

      pu = pu - du * dd + du * dd * t;
    }
    // setup prev cam vert
    pVertCam = tVertCam;
    // next vertmem
    v1++;

    // setup projected vert
    Vid::ProjectFromCamera_I( *v1, tVertCam);

    // radius for this vert
    rad = Vid::Project( radius, tVertCam.z);

    // wid1 is 2D cross of vector from this to prev
    wid1.y = -(pVertProj.vv.x - v1->vv.x);
    wid1.x =   pVertProj.vv.y - v1->vv.y;

    // adjust wid1
    dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
    dist = (dist == 0.0f) ? 1 : rad / dist;
    wid1 *= dist;

    // top vertmem
    v1->diffuse  = color;
    v1->specular = 0xff000000;
    v1->u = pu;
    v1->v = 0.0f;
    if (doFog)
    {
      v1->SetFog();
    }

    // setup prev projected
    pVertProj = *v1;

    // top vertmem
    v1->vv -= wid1;

    // bot vertmem
    v1++;
    *v1 = pVertProj;
    v1->vv += wid1;
    v1->v = 1.0f;
    if (doFog)
    {
      v1->SetFog();
    }

    // next vert
    tVertP++;

    if (sTaper)
    {
      // taper to single tri at the ends
      if (dorend)
      {
        // not clipped; fill indexmem
        *i1++ = (U16)(j1);
        *i1++ = (U16)(j1 + 1);
        *i1++ = (U16)(j1 + 2);
      }
      j1 += 1;
    }
    else
    {
      // 2 tris at the ends
      if (dorend)
      {
        // not clipped; fill indexmem
        *i1++ = (U16)(j1);
        *i1++ = (U16)(j1 + 1);
        *i1++ = (U16)(j1 + 3);
        *i1++ = (U16)(j1);
        *i1++ = (U16)(j1 + 3);
        *i1++ = (U16)(j1 + 2);
      }
      j1 += 2;
    }

    // loop through the middle points; advancing uvScale
    const Vector *pe = points + pointCount - 1;
    for ( ; tVertP < pe; tVertP++)
    {
      // setup prev
      pVert = tVert;
      // setup this
      tVert = *tVertP;

      dd = Vector( tVert - pVert).Magnitude();

      // advance uvScale
      u0 += du * dd;
      pu = u0;

      // setup cam vert
      Vid::TransformFromWorld( tVertCam, tVert);

      F32 dd = Vector(tVert - pVert).Magnitude();

      // clip
      dorend = TRUE;
      if (pVertCam.z <= nearplane)
      {
        // prev behind

        if (tVertCam.z < nearplane)
        {
          // this behind
          dorend = FALSE;
        }
        else
        {
          // prev behind; this not
          // delta
          Vector dp = tVertCam - pVertCam;
          // parametric delta
          F32 t = (nearplane - pVertCam.z) / (tVertCam.z - pVertCam.z);

          // adjust prev cam vert
          pVertCam = pVertCam + dp * t;

          // delta
          dp = tVert - pVert;
          // adjust prev vert
          pVert = pVert + dp * t;

          // setup adjusted screen vert
          Vid::ProjectFromCamera_I( *v1, pVertCam);
          // adjust texture coord
          v1->u += du * dd * t;
          if (doFog)
          {
            v1->SetFog();
          }

          // wid1 is 2D cross of vector from this to next
          Vid::ProjectFromCamera_I( nextTL, tVertCam);
          wid1.y = -(v1->vv.x - nextTL.vv.x);
          wid1.x =   v1->vv.y - nextTL.vv.y;

          // radius for this vert
          rad = Vid::Project( radius, pVertCam.z);

          // adjust wid1
          dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
          dist = (dist == 0.0f) ? 1 : rad / dist;
          wid1 *= dist;

          // reset prev
          pVertProj = *v1;
          // setup other vert
          *(v1 - 1) = *v1;

          // add offsets
          v1->vv += wid1;
          (v1 - 1)->vv -= wid1;
          (v1 - 1)->v = 0;
        }
      }
      else if (tVertCam.z < nearplane)
      {
        // last in front; this behind
        Vector dp = tVertCam - pVertCam;
        F32 t = (nearplane - pVertCam.z) / (tVertCam.z - pVertCam.z);
        tVertCam = pVertCam + dp * t;
        tVertCam.z = nearplane;

        dp  = tVert - pVert;
        tVert = pVert + dp * t;

        pu = pu - du * dd + du * dd * t;
      }

      // setup prev cam vert
      pVertCam = tVertCam;
      // next vertmem
      v1++;

      // setup projected vert
      Vid::ProjectFromCamera_I( *v1, tVertCam);

      wid1.y = -(pVertProj.vv.x - v1->vv.x);
      wid1.x =   pVertProj.vv.y - v1->vv.y;

      rad = Vid::Project( radius, tVertCam.z);

      dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
      dist = (dist == 0.0f) ? 1 : rad / dist;
      wid1 *= dist;

      v1->diffuse  = color;
      v1->specular = 0xff000000;
      v1->u = pu;
      v1->v = 0.0f;
      if (doFog)
      {
        v1->SetFog();
      }
      pVertProj = *v1;

      v1->vv -= wid1;

      v1++;
      *v1 = pVertProj;
      v1->vv += wid1;
      v1->v = 1.0f;

      if (dorend)
      {
        *i1++ = (U16)(j1);
        *i1++ = (U16)(j1 + 1);
        *i1++ = (U16)(j1 + 3);
        *i1++ = (U16)(j1);
        *i1++ = (U16)(j1 + 3);
        *i1++ = (U16)(j1 + 2);
      }
      j1 += 2;
    }

    pVert = tVert;
    tVert = *tVertP;
    
    dd = Vector( tVert - pVert).Magnitude();

    // advance uvScale
    u0 += du * dd;
    pu = u0;

    // setup cam vert
    Vid::TransformFromWorld( tVertCam, tVert);

    dorend = TRUE;
    if (pVertCam.z <= nearplane)
    {
      if (tVertCam.z < nearplane)
      {
        dorend = FALSE;
      }
      else
      {
        Vector dp = tVertCam - pVertCam;
        F32 t = (nearplane - pVertCam.z) / (tVertCam.z - pVertCam.z);
        pVertCam = pVertCam + dp * t;

        dp = tVert - pVert;
        pVert = pVert + dp * t;

        Vid::ProjectFromCamera_I( *v1, pVertCam);
        if (doFog)
        {
          v1->SetFog();
        }
        v1->u += du * dd * t;

        // wid1 is 2D cross of vector from this to next
        Vid::ProjectFromCamera_I( nextTL, tVertCam);
        wid1.y = -(v1->vv.x - nextTL.vv.x);
        wid1.x =   v1->vv.y - nextTL.vv.y;

        // radius for this vert
        rad = Vid::Project( radius, pVertCam.z);

        // adjust wid1
        dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
        dist = (dist == 0.0f) ? 1 : rad / dist;
        wid1 *= dist;

        pVertProj = *v1;
        *(v1 - 1) = *v1;

        v1->vv += wid1;
        v1->v = 1;
        (v1 - 1)->vv -= wid1;
        (v1 - 1)->v = 0;
      }
    }
    else if (tVertCam.z < nearplane)
    {
      Vector dp = tVertCam - pVertCam;
      F32 t = (nearplane - pVertCam.z) / (tVertCam.z - pVertCam.z);
      tVertCam = pVertCam + dp * t;
      tVertCam.z = nearplane;

      dp  = tVert - pVert;
      tVert = pVert + dp * t;

      pu = pu - du * dd + du * dd * t;
    }
    v1++;

    if (dorend)
    {
      // setup prev cam vert
  //    pVertCam = tVertCam;

      // setup projected vert
      Vid::ProjectFromCamera_I( *v1, tVertCam);
      v1->diffuse  = color;
      v1->specular = 0xff000000;
      v1->u = pu;
      if (doFog)
      {
        v1->SetFog();
      }

      if (taper)
      {
        v1->v = 0.5f;

        if (dorend)
        {
          *i1++ = (U16)(j1);
          *i1++ = (U16)(j1 + 1);
          *i1++ = (U16)(j1 + 2);
        }
      }
      else
      {
        v1->v = 0.0f;

        // wid1 is 2D cross of vector from this to prev
        wid1.y = -(pVertProj.vv.x - v1->vv.x);
        wid1.x =   pVertProj.vv.y - v1->vv.y;

        // radius for this vert
        rad = Vid::Project( radius, tVertCam.z);

        // adjust wid1
        dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
        dist = (dist == 0.0f) ? 1 : rad / dist;
        wid1 *= dist;

        // setup prev projected
        pVertProj = *v1;

        // top vertmem
        v1->vv -= wid1;

        // bot vertmem
        v1++;
        *v1 = pVertProj;
        v1->vv += wid1;
        v1->v = 1.0f;

        if (dorend)
        {
          *i1++ = (U16)(j1);
          *i1++ = (U16)(j1 + 1);
          *i1++ = (U16)(j1 + 3);
          *i1++ = (U16)(j1);
          *i1++ = (U16)(j1 + 3);
          *i1++ = (U16)(j1 + 2);
        }
      }
    }
    v1++;

    Vid::UnlockIndexedPrimitiveMem(v1 - vertmem, i1 - indexmem);

#ifdef DOSTATISTICS
    Statistics::spriteTris = Statistics::spriteTris + (i1 - indexmem) / 3;
#endif
  }
  //----------------------------------------------------------------------------

#endif
  // render a fully 3-D beam
  //
  void RenderBeamOriented( Bool doFog, const Vector * points, U32 pointCount, const Vector & orientation, F32 zpos, const Bitmap * texture, Color color, U32 blend, U16 sorting, F32 u0, F32 du, Bool taper, Color * colorA, F32 * radiusA) // RS_BLEND_ADD, = sortEFFECT, = 0.0f, = 1.0f, = TRUE, = NULL, = NULL
  {
    radiusA;
    colorA;

    // setup buckets
    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | blend);

    Vid::SetWorldTransform( Matrix::I);
    SetBucketMaterial( defMaterial);
    SetBucketTexture( texture, TRUE, 0, blend);
    Vid::SetTranBucketZ( zpos, sorting);

    VertexTL * vertmem;
    U16 * indexmem;
    U32 heapSize = Vid::Heap::ReqVertex( &vertmem, pointCount << 1, &indexmem, (pointCount - 1) * 6);

    const Vector * p = points, * pe = points + pointCount - 1;
    VertexTL * v = vertmem;

    if (taper)
    {
      v->vv = *p;
      v->diffuse  = color;
      v->specular = 0xff000000;
      v->u = u0;
      v->v = 0.5f;
      v++;
    }
    else
    {
      v->vv = *p + orientation;
      v->diffuse  = color;
      v->specular = 0xff000000;
      v->u = u0;
      v->v = 0;
      v++;
      v->vv = *p - orientation;
      v->diffuse  = color;
      v->specular = 0xff000000;
      v->u = u0;
      v->v = 1;
      v++;
    }
    Bool tp = taper;
    U16 * i = indexmem;
    U32 j = 0;
    for (p = points + 1; p < pe; p++, tp = 0)
    {
      // distance between points
      F32 dd = Vector( *p - *(p - 1)).Magnitude();

      // advance uvScale
      u0 += du * dd;

      v->vv = *p + orientation;
      v->diffuse  = color;
      v->specular = 0xff000000;
      v->u = u0;
      v->v = 0;
      v++;
      v->vv = *p - orientation;
      v->diffuse  = color;
      v->specular = 0xff000000;
      v->u = u0;
      v->v = 1;
      v++;

      if (tp)
      {
        *i++ = (U16)(j);
        *i++ = (U16)(j + 1);
        *i++ = (U16)(j + 2);
        j++;
      }
      else
      {
        *i++ = (U16)(j);
        *i++ = (U16)(j + 1);
        *i++ = (U16)(j + 3);
        *i++ = (U16)(j);
        *i++ = (U16)(j + 3);
        *i++ = (U16)(j + 2);
        j += 2;
      }
    }
    // distance between points
    F32 dd = Vector( *p - *(p - 1)).Magnitude();

    // advance uvScale
    u0 += du * dd;

    if (taper)
    {
      v->vv = *p;
      v->diffuse  = color;
      v->specular = 0xff000000;
      v->u = u0;
      v->v = 0.5f;
      v++;

      *i++ = (U16)(j);
      *i++ = (U16)(j + 1);
      *i++ = (U16)(j + 2);
    }
    else
    {
      v->vv = *p + orientation;
      v->diffuse  = color;
      v->specular = 0xff000000;
      v->u = u0;
      v->v = 0;
      v++;
      v->vv = *p - orientation;
      v->diffuse  = color;
      v->specular = 0xff000000;
      v->u = u0;
      v->v = 1;
      v++;

      *i++ = (U16)(j);
      *i++ = (U16)(j + 1);
      *i++ = (U16)(j + 3);
      *i++ = (U16)(j);
      *i++ = (U16)(j + 3);
      *i++ = (U16)(j + 2);
    }

    Vid::TransformFromModel( vertmem, v - vertmem);
    Vid::Clip::ToBucket( vertmem, v - vertmem, indexmem, i - indexmem, (void *) 0xffffffff, doFog, clipALL);

    Vid::Heap::Restore( heapSize);
  }
}
//----------------------------------------------------------------------------

