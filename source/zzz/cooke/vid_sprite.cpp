///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// vidrend.cpp      non-setup 3d rendering specific Vid stuff
//
// 22-APR-1998      John Cooke
//

#include "vid.h"
#include "statistics.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  //
  // Rectangle
  //
  // Draws a rectangle flat on the screen
  //
  void RenderRectangle( const Area<S32> &rect, Color color, U32 blend, U16 sorting) // = RS_BLEND_GLOW, = sortEFFECT
  {
#if 1
	  // set the primitive description
	  SetBucketPrimitiveDesc( PT_LINELIST, FVF_TLVERTEX,
//    SetBucketPrimitiveDesc( PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_NOINDEXED | blend);

	  // set material
    SetBucketMaterial( defMaterial);
    SetBucketTexture( NULL, color < 255 ? TRUE:FALSE, 0, blend);

	  // set bucket depth
	  SetTranBucketZ(0.0f, sorting);

	  // lock primitive memory
	  VertexTL *vertmem;
	  if (!LockPrimitiveMem( (void **)&vertmem, 8))
	  {
      return;
    }

	  // top left corner
	  vertmem[0].vv.x     = (F32) rect.p0.x;
	  vertmem[0].vv.y     = (F32) rect.p0.y;
	  vertmem[0].vv.z     = 0.0f;
	  vertmem[0].rhw      = 1.0f;
	  vertmem[0].diffuse  = color;
	  vertmem[0].specular = 0xff000000;
	  vertmem[0].u        = 0.0f;
	  vertmem[0].v        = 0.0f;

	  // top right corner
	  vertmem[1].vv.x     = (F32) rect.p1.x;
	  vertmem[1].vv.y     = (F32) rect.p0.y;
	  vertmem[1].vv.z     = 0.0f;
	  vertmem[1].rhw      = 1.0f;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = 0xff000000;
	  vertmem[1].u        = 1.0f;
	  vertmem[1].v        = 0.0f;

    vertmem[2] = vertmem[1];

	  // bottom right corner
	  vertmem[3].vv.x     = (F32) rect.p1.x;
	  vertmem[3].vv.y     = (F32) rect.p1.y;
	  vertmem[3].vv.z     = 0.0f;
	  vertmem[3].rhw      = 1.0f;
	  vertmem[3].diffuse  = color;
	  vertmem[3].specular = 0xff000000;
	  vertmem[3].u        = 1.0f;
	  vertmem[3].v        = 1.0f;

    vertmem[4] = vertmem[3];

	  // bottom left corner
	  vertmem[5].vv.x     = (F32) rect.p0.x;
	  vertmem[5].vv.y     = (F32) rect.p1.y;
	  vertmem[5].vv.z     = 0.0f;
	  vertmem[5].rhw      = 1.0f;
	  vertmem[5].diffuse  = color;
	  vertmem[5].specular = 0xff000000;
	  vertmem[5].u        = 0.0f;
	  vertmem[5].v        = 1.0f;

    vertmem[6] = vertmem[5];
    vertmem[7] = vertmem[0];

	  // submit the polygons
	  UnlockPrimitiveMem( 8);

#else

    // Draw the group selection box
    SetBucketPrimitiveDesc
    (
      PT_TRIANGLELIST, FVF_TLVERTEX, 
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF, TRUE
    );

    SetBucketMaterial( defMaterial);
    SetBucketTexture( NULL, color.a < 255 ? TRUE:FALSE);
    SetTranBucketZ(0.0f);

    VertexTL *vertmem;
    U16 *indexmem;

    if (Vid::LockIndexedPrimitiveMem((void **)&vertmem, 4, &indexmem, 6))
    {
      vertmem[0].vv.x = (F32) rect.p0.x;
      vertmem[0].vv.y = (F32) rect.p0.y;
      vertmem[1].vv.x = (F32) rect.p1.x;
      vertmem[1].vv.y = (F32) rect.p0.y;
      vertmem[2].vv.x = (F32) rect.p1.x;
      vertmem[2].vv.y = (F32) rect.p1.y;
      vertmem[3].vv.x = (F32) rect.p0.x;
      vertmem[3].vv.y = (F32) rect.p1.y;

      // Assign a color to each vertex
      vertmem[0].diffuse = color;
      vertmem[1].diffuse = color;
      vertmem[2].diffuse = color;
      vertmem[3].diffuse = color;

      for (U32 i = 0; i < 4; i++)
      {
        vertmem[i].vv.z = 0.0f;
        vertmem[i].rhw  = 1.0f;
        vertmem[i].specular = 0xff000000;
      }

      Utils::Memcpy( indexmem, rectIndices, 12);

      UnlockIndexedPrimitiveMem(4, 6);
    }
#endif
  }
  //----------------------------------------------------------------------------

  void RenderSprite( Bool doFog, const Vector &pos, F32 size, const Bitmap *texture, Color color, U32 blend, U16 sorting, F32 rotation ) // = TRUE, = RS_BLEND_GLOW, = sortEFFECT, = 0
  {
    Camera &cam = CurCamera();

    Vector p;
    F32 rhw, vz;
    cam.TransformProjectFromWorldSpace( p, vz, rhw, pos);

    if (vz > cam.NearPlane() && vz < cam.FarPlane())
    {
      // project/scale size
      size *= cam.ProjConstX() * rhw;

      if (p.x < clipRect.p1.x && p.x + size > clipRect.p0.x
       && p.y < clipRect.p1.y && p.y + size > clipRect.p0.y)
      {
        RenderSpriteProjected( doFog, p, vz, rhw, size, texture, color, blend, sorting, rotation);
      }
    }
  }
  //----------------------------------------------------------------------------

  void RenderSpriteProjected( Bool doFog, const Vector &pos, F32 vz, F32 rhw, F32 size, const Bitmap *texture, Color color, U32 blend, U16 sorting, F32 rotation) // = TRUE, = RS_BLEND_GLOW, = sortEFFECT, = 0
  {
    rotation;

	  // set the primitive description
	  SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | blend);

	  // set material
    SetBucketMaterial( defMaterial);
    SetBucketTexture( texture, TRUE, 0, blend);
	  // set bucket depth
	  SetTranBucketZ(vz, sorting);

	  // lock primitive memory
	  VertexTL *vertmem;
	  U16 *index;
	  if (!LockIndexedPrimitiveMem( (void **)&vertmem, 4, &index, 6))
	  {
      return;
    }

#ifdef DEVELOPMENT
    if (pos.z < 0 || pos.z > 1)
    {
      LOG_DIAG(( "Vid::RenderSpriteProjected: z out of range %f", pos.z));
    }
#endif

	  // get corners of sprite
	  F32 x0 = pos.x - size;
	  F32 x1 = pos.x + size;
	  F32 y0 = pos.y - size;
	  F32 y1 = pos.y + size;

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

  void RenderFlareSprite( Bool doFog, const Vector &pos, F32 size, const Bitmap *texture, Color color, U32 blend, U16 sorting, F32 dx, F32 dy) // = RS_BLEND_GLOW, = sortEFFECT, = 0.5f, = 0.5f
  {
    Camera & cam = CurCamera();

    Vector p;
    F32 rhw, vz;
    cam.TransformProjectFromWorldSpace( p, vz, rhw, pos);

    if (vz < cam.NearPlane() && vz > cam.FarPlane())
    {
      return;
    }

    // project/scale the size
    size *= cam.ProjConstX() * rhw;

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
	  SetTranBucketZ(vz, sorting);

    // lock vertex memory
	  VertexTL *vertmem;
	  U16 *indexmem;
	  if (!LockIndexedPrimitiveMem( (void **)&vertmem, 5, &indexmem, 12))
	  {
      return;
    }

#ifdef DEVELOPMENT
    if (p.z < 0 || p.z > 1)
    {
      LOG_DIAG(( "Vid::RenderSpriteProjected: z out of range %f", p.z));
    }
#endif

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

	  vertmem[1].vv.x = x0;
	  vertmem[1].vv.y = y1;
	  vertmem[1].vv.z = p.z;
    vertmem[1].rhw  = rhw;
	  vertmem[1].diffuse  = 0xFF000000;
	  vertmem[1].specular = vertmem->specular;
	  vertmem[1].u = 0.5f + dx;
	  vertmem[1].v = 0.5f + dy;

	  vertmem[2].vv.x = x1;
	  vertmem[2].vv.y = y1;
	  vertmem[2].vv.z = p.z;
    vertmem[2].rhw  = rhw;
	  vertmem[2].diffuse  = vertmem[1].diffuse;
	  vertmem[2].specular = vertmem->specular;
	  vertmem[2].u = 0.5f + dy;
	  vertmem[2].v = 0.5f - dx;

	  vertmem[3].vv.x = x1;
	  vertmem[3].vv.y = y0;
	  vertmem[3].vv.z = p.z;
    vertmem[3].rhw  = rhw;
	  vertmem[3].diffuse  = vertmem[1].diffuse;
	  vertmem[3].specular = vertmem->specular;
	  vertmem[3].u = 0.5f - dx;
	  vertmem[3].v = 0.5f - dy;

	  vertmem[4].vv.x = x0;
	  vertmem[4].vv.y = y0;
	  vertmem[4].vv.z = p.z;
    vertmem[4].rhw  = rhw;
	  vertmem[4].diffuse  = vertmem[1].diffuse;
	  vertmem[4].specular = vertmem->specular;
	  vertmem[4].u = 0.5f - dy;
	  vertmem[4].v = 0.5f + dx;

    Utils::Memcpy( indexmem, rectIndices, 24);

    // submit the polygons
	  UnlockIndexedPrimitiveMem( 5, 12);

#ifdef DOSTATISTICS
    Statistics::spriteTris = Statistics::spriteTris + 4;
#endif
  }
  //----------------------------------------------------------------------------

  void RenderBoxProjected( const Vector &pos, F32 vz, F32 rhw, F32 size, Color color, U32 blend, U16 sorting) // = RS_BLEND_GLOW, sortNORMAL
  {
	  // set the primitive description
	  SetBucketPrimitiveDesc( PT_LINELIST, FVF_TLVERTEX,
//    SetBucketPrimitiveDesc( PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_NOINDEXED | blend);

	  // set material
    SetBucketMaterial( defMaterial);
    SetBucketTexture( NULL);

	  // set bucket depth
	  SetTranBucketZ(vz, sorting);

	  // lock primitive memory
	  VertexTL *vertmem;
	  if (!LockPrimitiveMem( (void **)&vertmem, 8))
	  {
      return;
    }

	  // get corners of sprite
	  F32 x0 = pos.x - size - 1;
	  F32 x1 = pos.x + size;
	  F32 y0 = pos.y - size - 1;
	  F32 y1 = pos.y + size;

	  // top left corner
	  vertmem[0].vv.x     = x0;
	  vertmem[0].vv.y     = y0;
	  vertmem[0].vv.z     = pos.z;
	  vertmem[0].rhw      = rhw;
	  vertmem[0].diffuse  = color;
	  vertmem[0].specular = 0xff000000;
	  vertmem[0].u        = 0.0f;
	  vertmem[0].v        = 0.0f;

	  // top right corner
	  vertmem[1].vv.x     = x1;
	  vertmem[1].vv.y     = y0;
	  vertmem[1].vv.z     = pos.z;
	  vertmem[1].rhw      = rhw;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = 0xff000000;
	  vertmem[1].u        = 1.0f;
	  vertmem[1].v        = 0.0f;

    vertmem[2] = vertmem[1];

	  // bottom right corner
	  vertmem[3].vv.x     = x1;
	  vertmem[3].vv.y     = y1;
	  vertmem[3].vv.z     = pos.z;
	  vertmem[3].rhw      = rhw;
	  vertmem[3].diffuse  = color;
	  vertmem[3].specular = 0xff000000;
	  vertmem[3].u        = 1.0f;
	  vertmem[3].v        = 1.0f;

    vertmem[4] = vertmem[3];

	  // bottom left corner
	  vertmem[5].vv.x     = x0;
	  vertmem[5].vv.y     = y1;
	  vertmem[5].vv.z     = pos.z;
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

#if 1
  // render a psuedo 3-D beam clipping at the nearplane 
  //
  void RenderBeam( Bool doFog, const Vector *points, U32 pointCount, F32 radius, F32 zpos, const Bitmap *texture, Color color, U32 blend, U16 sorting, F32 u0, F32 du, Bool taper) // RS_BLEND_ADD, = sortEFFECT, = 0.0f, = 1.0f, = TRUE
  {
    Camera &cam = Vid::CurCamera();
    F32 nearplane = Vid::CurCamera().NearPlane() + 0.1f;

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
    U32 vcount, icount;
    vcount = pointCount * 2;
    icount = (pointCount - 1) * 6;

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
    cam.TransformFromWorldSpace( tVertCam, *tVertP);
    // setup prev cam vert
    pVertCam = tVertCam;

    // setup projected vert
    cam.TransformProjectFromModelSpace( *v1, *tVertP);
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
    cam.TransformFromWorldSpace( nextCam, *(tVertP + 1));

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
      cam.ProjectFromCameraSpace( nextTL, nextCam);

      // wid1 is 2D cross of vector from this to next
      wid1.y = -(v1->vv.x - nextTL.vv.x);
      wid1.x =   v1->vv.y - nextTL.vv.y;

      // radius for this vert
      rad = radius * cam.RHW( tVertCam.z) * -cam.ProjConstY();

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

    // advance uvScale
    u0 += du;
    F32 pu = u0;
    // setup prev world vert
    pVert = *tVertP;
    // next vert
    tVertP++;
    // setup this vert
    tVert = *tVertP;
    // setup cam vert
    cam.TransformFromWorldSpace( tVertCam, tVert);

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

        cam.ProjectFromCameraSpace( *v1, pVertCam);
        v1->u += du * t;
        if (doFog)
        {
          v1->SetFog();
        }
        if (!sTaper)
        {
          // wid1 is 2D cross of vector from this to next
          cam.ProjectFromCameraSpace( nextTL, tVertCam);
          wid1.y = -(v1->vv.x - nextTL.vv.x);
          wid1.x =   v1->vv.y - nextTL.vv.y;

          // radius for this vert
          rad = radius * cam.RHW( pVertCam.z) * -cam.ProjConstY();

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

      pu = pu - du + du * t;
    }
    // setup prev cam vert
    pVertCam = tVertCam;
    // next vertmem
    v1++;

    // setup projected vert
    cam.ProjectFromCameraSpace( *v1, tVertCam);

    // radius for this vert
    rad = radius * cam.RHW( tVertCam.z) * -cam.ProjConstY();

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

    // advance uvScale
    u0 += du;
    pu = u0;
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
    for ( ; tVertP < pe; tVertP++, u0 += du, pu = u0)
    {
      // setup prev
      pVert = tVert;
      // setup this
      tVert = *tVertP;
      // setup cam vert
      cam.TransformFromWorldSpace( tVertCam, tVert);

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
          cam.ProjectFromCameraSpace( *v1, pVertCam);
          // adjust texture coord
          v1->u += du * t;
          if (doFog)
          {
            v1->SetFog();
          }

          // wid1 is 2D cross of vector from this to next
          cam.ProjectFromCameraSpace( nextTL, tVertCam);
          wid1.y = -(v1->vv.x - nextTL.vv.x);
          wid1.x =   v1->vv.y - nextTL.vv.y;

          // radius for this vert
          rad = radius * cam.RHW( pVertCam.z) * -cam.ProjConstY();

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

        pu = pu - du + du * t;
      }

      // setup prev cam vert
      pVertCam = tVertCam;
      // next vertmem
      v1++;

      // setup projected vert
      cam.ProjectFromCameraSpace( *v1, tVertCam);

      wid1.y = -(pVertProj.vv.x - v1->vv.x);
      wid1.x =   pVertProj.vv.y - v1->vv.y;

	    rad = radius * cam.RHW( tVertCam.z) * -cam.ProjConstY();

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
    
    // setup cam vert
    cam.TransformFromWorldSpace( tVertCam, tVert);

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

        cam.ProjectFromCameraSpace( *v1, pVertCam);
        if (doFog)
        {
          v1->SetFog();
        }
        v1->u += du * t;

        // wid1 is 2D cross of vector from this to next
        cam.ProjectFromCameraSpace( nextTL, tVertCam);
        wid1.y = -(v1->vv.x - nextTL.vv.x);
        wid1.x =   v1->vv.y - nextTL.vv.y;

        // radius for this vert
        rad = radius * cam.RHW( pVertCam.z) * -cam.ProjConstY();

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

      pu = pu - du + du * t;
    }
    v1++;

    if (dorend)
    {
      // setup prev cam vert
  //    pVertCam = tVertCam;

      // setup projected vert
      cam.ProjectFromCameraSpace( *v1, tVertCam);
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
        rad = radius * cam.RHW( tVertCam.z) * -cam.ProjConstY();

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
#else
  void RenderBeam( Bool doFog, const Vector *points, U32 pointCount, F32 radius, F32 zpos, const Bitmap *texture, Color color, U32 blend, F32 u0, F32 du) // RS_BLEND_ADD, = 0.0f, = 1.0f
  {
    Camera &cam = Vid::CurCamera();
    F32 nearplane = Vid::CurCamera().NearPlane() + 0.1f;

    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | blend,
      TRUE);

    Vid::SetWorldTransform( Matrix::I);
    SetBucketMaterial( defMaterial);
    SetBucketTexture( texture, TRUE, 0, blend);
    Vid::SetTranBucketZ( zpos);

    U32 vcount = (pointCount - 1) * 2;
    U32 icount = (pointCount - 2) * 6;

    VertexTL *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem((void **)&vertmem, vcount, &indexmem, icount, points))
    {
      return;
    }

    VertexTL *v1 = vertmem;
    U16 *i1 = indexmem;
    U32 j1 = 0;
    const Vector *pp = points;

    F32 dist;
    Vector wid1;
    wid1.z = 0.0f;

    Vector p, ptemp, ppp, ppptemp;
    cam.TransformFromWorldSpace( ptemp, *pp);

    cam.TransformProjectFromModelSpace( *v1, *pp);
    VertexTL temp = *v1;
    v1->diffuse  = color;
    v1->specular = 0xff000000;
    v1->u = u0;
    v1->v = 0.5f;
    if (doFog)
    {
      v1->SetFog();
    }

    u0 += du;
    ppptemp = *pp;
    pp++;

    ppp = *pp;
    cam.TransformFromWorldSpace( p, ppp);

    Bool dorend = TRUE;
    if (ptemp.z < nearplane)
    {
      if (p.z < nearplane)
      {
        dorend = FALSE;
      }
      else
      {
        Vector dp = p - ptemp;
        F32 t = (nearplane - ptemp.z) / (p.z - ptemp.z);
        ptemp = ptemp + dp * t;

        dp = ppp - ppptemp;
        ppptemp = ppptemp + dp * t;

        cam.TransformProjectFromModelSpace( *v1, ppp);
        if (doFog)
        {
          v1->SetFog();
        }
        temp = *v1;
        v1->u += du * t;
      }
    }
    else if (p.z < nearplane)
    {
      Vector dp = p - ptemp;
      F32 t = (nearplane - ptemp.z) / (p.z - ptemp.z);
      p = ptemp + dp * t;

      dp  = ppp - ppptemp;
      ppp = ppptemp + dp * t;
    }

    v1++;
    ptemp = p;

	  F32 rad = radius * cam.RHW( p.z) * -cam.ProjConstY();

    cam.TransformProjectFromModelSpace( *v1, ppp);
    wid1.y = -(temp.vv.x - v1->vv.x);
    wid1.x =   temp.vv.y - v1->vv.y;
    temp = *v1;
    dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
    if (dist == 0.0f)
    {
      dist = 1;
    }
    else
    {
      dist = rad / dist;
    }
    wid1 *= dist;

    v1->vv -= wid1;
    v1->diffuse  = color;
    v1->specular = 0xff000000;
    v1->u = u0;
    v1->v = 0.0f;
    if (doFog)
    {
      v1->SetFog();
    }

    v1++;
    *v1 = temp;
    v1->vv += wid1;
    v1->diffuse  = color;
    v1->specular = 0xff000000;
    v1->u = u0;
    v1->v = 1.0f;
    if (doFog)
    {
      v1->SetFog();
    }

    u0 += du;
    pp++;

    if (dorend)
    {
      *i1++ = (U16)(j1);
      *i1++ = (U16)(j1 + 1);
      *i1++ = (U16)(j1 + 2);
    }
    j1 += 1;

    const Vector *pe = points + pointCount - 1;
    for ( ; pp < pe; pp++, u0 += du)
    {
      ppptemp = ppp;
      ppp = *pp;
      cam.TransformFromWorldSpace( p, ppp);

      dorend = TRUE;
      if (ptemp.z < nearplane)
      {
        if (p.z < nearplane)
        {
          dorend = FALSE;
        }
        else
        {
          Vector dp = p - ptemp;
          F32 t = (nearplane - ptemp.z) / (p.z - ptemp.z);
          ptemp = ptemp + dp * t;

          dp = ppp - ppptemp;
          ppptemp = ppptemp + dp * t;

          cam.TransformProjectFromModelSpace( *v1, ppp);
          v1->u += du * t;
          if (doFog)
          {
            v1->SetFog();
          }

          temp = *v1;
          *(v1 - 1) = temp;
          v1->vv += wid1;
          (v1 - 1)->vv -= wid1;
        }
      }
      else if (p.z < nearplane)
      {
        Vector dp = p - ptemp;
        F32 t = (nearplane - ptemp.z) / (p.z - ptemp.z);
        p = ptemp + dp * t;

        dp  = ppp - ppptemp;
        ppp = ppptemp + dp * t;
      }

      v1++;
      ptemp = p;

	    rad = radius * cam.RHW( p.z) * -cam.ProjConstY();
      cam.TransformProjectFromModelSpace( *v1, ppp);

      wid1.y = -(temp.vv.x - v1->vv.x);
      wid1.x =   temp.vv.y - v1->vv.y;
      temp = *v1;

      dist = (F32)sqrt(wid1.y * wid1.y + wid1.x * wid1.x);
      if (dist == 0.0f)
      {
        dist = 1;
      }
      else
      {
        dist = rad / dist;
      }
      wid1 *= dist;

      v1->vv -= wid1;
      v1->diffuse  = color;
      v1->specular = 0xff000000;
      v1->u = u0;
      v1->v = 0.0f;
      if (doFog)
      {
        v1->SetFog();
      }
      v1++;

      *v1 = temp;
      v1->vv += wid1;
      v1->diffuse  = color;
      v1->specular = 0xff000000;
      v1->u = u0;
      v1->v = 1.0f;
      if (doFog)
      {
        v1->SetFog();
      }

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

    ppptemp = ppp;
    ppp = *pp;
    cam.TransformFromWorldSpace( p, ppp);

    dorend = TRUE;
    if (ptemp.z < nearplane)
    {
      if (p.z < nearplane)
      {
        dorend = FALSE;
      }
      else
      {
        Vector dp = p - ptemp;
        F32 t = (nearplane - ptemp.z) / (p.z - ptemp.z);
        ptemp = ptemp + dp * t;

        dp = ppp - ppptemp;
        ppptemp = ppptemp + dp * t;

        cam.TransformProjectFromModelSpace( *v1, ppp);
        if (doFog)
        {
          v1->SetFog();
        }
        v1->u += du * t;
        temp = *v1;
        *(v1 - 1) = temp;
        v1->vv += wid1;
        (v1 - 1)->vv -= wid1;
      }
    }
    else if (p.z < nearplane)
    {
      Vector dp = p - ptemp;
      F32 t = (nearplane - ptemp.z) / (p.z - ptemp.z);
      p = ptemp + dp * t;

      dp  = ppp - ppptemp;
      ppp = ppptemp + dp * t;
    }
    v1++;
    ptemp = p;

    cam.TransformProjectFromModelSpace( *v1, ppp);
    v1->diffuse  = color;
    v1->specular = 0xff000000;
    v1->u = u0;
    v1->v = 0.5f;
    if (doFog)
    {
      v1->SetFog();
    }
    v1++;

    if (dorend)
    {
      *i1++ = (U16)(j1);
      *i1++ = (U16)(j1 + 1);
      *i1++ = (U16)(j1 + 2);
    }

    Vid::UnlockIndexedPrimitiveMem(v1 - vertmem, i1 - indexmem);

#ifdef DOSTATISTICS
    Statistics::spriteTris = Statistics::spriteTris + (i1 - indexmem) / 3;
#endif
  }
  //----------------------------------------------------------------------------
#endif

}
//----------------------------------------------------------------------------

