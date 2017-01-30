//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// terrain_rendertl.cpp
//
// 05-APR-2000
//

#include "vid_private.h"
#include "light_priv.h"
#include "vidclip.h"
#include "random.h"
#include "console.h"
#include "statistics.h"
#include "terrain_priv.h"
//----------------------------------------------------------------------------

namespace Terrain
{
  void RenderShroudWaterVtl( Cluster & clus, S32 x0, S32 z0, VertexC * verts0, U32 clipFlags)
  {
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( NULL, FALSE);

    VertexC * vertmem;
    U16 * indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
    {
      return;
    }

    F32 wave0 = 0, wave1 = 0;
    if (*Vid::Var::Terrain::waveActive)
    {
      // alternate wave0, wave1 in a checkerboard over the entire terrain grid
      //
      if ((S32(x0 * clusPerMeter) + (S32(z0 * clusPerMeter) % 2)) % 2)
      {
        wave0 = waterWave1;
        wave1 = waterWave0;
      }
      else
      {
        wave0 = waterWave0;
        wave1 = waterWave1;
      }
    }

    vertmem[0].vv = verts0[0].vv;
    vertmem[0].vv.y = clus.waterHeight + wave0;
    vertmem[0].nv = waterNorms[0];
    vertmem[0].diffuse  = 0xff000000;
    vertmem[1].vv = verts0[20].vv;
    vertmem[1].vv.y = clus.waterHeight + wave1;
    vertmem[1].nv = waterNorms[0];
    vertmem[1].diffuse  = 0xff000000;
    vertmem[2].vv = verts0[24].vv;
    vertmem[2].vv.y = clus.waterHeight + wave0;
    vertmem[2].nv = waterNorms[0];
    vertmem[2].diffuse  = 0xff000000;
    vertmem[3].vv = verts0[4].vv;
    vertmem[3].vv.y = clus.waterHeight + wave1;
    vertmem[3].nv = waterNorms[0];
    vertmem[3].diffuse  = 0xff000000;

#ifdef DOSTATISTICS
    if (clipFlags == clipNONE)
    {
      Statistics::noClipTris += 2;
    }
    else
    {
      Statistics::clipTris += 2;
    }
#endif

    Utils::Memcpy( indexmem, Vid::rectIndices, 12);

    Vid::UnlockIndexedPrimitiveMem( 4, 6);
  }
  //----------------------------------------------------------------------------

  void RenderCellWaterV( Cluster & clus, S32 x0, S32 z0, VertexC * verts0, F32 * fogs, U32 vcount, U32 clipFlags)
  {
    clipFlags &= ~clipPLANE0;

    if (Vid::Var::Terrain::shroud && clus.shroudCount == 25)
    {
      RenderShroudWaterVtl( clus, x0, z0, verts0, clipFlags);
      return;
    }

    Vid::SetTranBucketZMax( Vid::sortWATER0 + 1);
    Vid::SetBucketMaterial( waterMaterial);
    Vid::SetBucketTexture( waterTex, waterIsAlpha);

    VertexC * vertmem;
    U16 * indexmem;

    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 25, &indexmem, 96, &clus))
    {
      return;
    }

    F32 wave0 = 0, wave1 = 0;
    if (*Vid::Var::Terrain::waveActive)
    {
      // alternate wave0, wave1 in a checkerboard over the entire terrain grid
      //
      if ((S32(x0 * clusPerMeter) + (S32(z0 * clusPerMeter) % 2)) % 2)
      {
        wave0 = waterWave1;
        wave1 = waterWave0;
      }
      else
      {
        wave0 = waterWave0;
        wave1 = waterWave1;
      }
    }
    Color c = *Vid::Var::Terrain::waterColorBottom;
    VertexC * v = vertmem, * ve = vertmem + 25, * sv = verts0;
    F32 dw = (wave1 - wave0) / 4;
    UVPair * uvlist = waterUVList0;
    for ( ; v < ve; wave0 += dw, wave1 -= dw)
    {
      VertexC * vee = v + 4;
      F32 dww = (wave1 - wave0) / 4;
      for (F32 w = wave0; v <= vee; v++, sv++, w += dww, uvlist++, fogs++)
      {
        v->vv.x = sv->vv.x;
        v->vv.y = clus.waterHeight + w;
        v->vv.z = sv->vv.z;
        v->nv = waterNorms[0];
        v->diffuse.Modulate( c, *fogs, *fogs, *fogs);
        v->uv = *uvlist;
      }
    }
    Utils::Memcpy( indexmem, clusterI, 96 << 1);

    Vid::UnlockIndexedPrimitiveMem( vcount, 96);

#ifdef DOSTATISTICS
    if (clipFlags == clipNONE)
    {
      Statistics::noClipTris += 32;
    }
    else
    {
      Statistics::clipTris += 32;
    }
#endif

    if (waterLayer2)
    {
      // 2nd layer
      //
      VertexC *vertmem1;
      Vid::SetTranBucketZMax( Vid::sortWATER0);

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem1, 25, &indexmem, 96, &clus))
      {
        return;
      }
      c = *Vid::Var::Terrain::waterColorTop;

      VertexC * dv = vertmem1, * sv = vertmem;
      UVPair * uvlist, * uve = waterUVList1 + vcount;
      for (uvlist = waterUVList1; uvlist < uve; dv++, sv++, uvlist++)
      {
        *dv = *sv;
        dv->uv = *uvlist;
        dv->diffuse.a  = c.a;
      }
      Utils::Memcpy( indexmem, clusterI, 96 << 1);

      Vid::UnlockIndexedPrimitiveMem( vcount, 96);
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterV( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, U32 clipFlags) // = clipALL
  {
    // build list of vertex positions, normals, and colors
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    S32 meterStrideX    = abs( cellStrideX) * heightField.meterPerCell;
    S32 meterStrideZ    = abs( cellStrideZ) * heightField.meterPerCell;
    S32 cellStrideWidth = cellStrideZ * heightField.cellPitch;

    Vid::SetBucketFlags( RS_BLEND_DEF | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP));
    Vid::SetTranBucketZMax( Vid::sortTERRAIN0 + 1);
    Vid::SetBucketMaterial( Vid::defMaterial);

/*    
    if (Vid::Clip::BoundsTestAlphaFar( clus.zDepth, clus.bounds.Radius()))
    {
      BucketMan::forceTranslucent = TRUE;
    }
*/
    Cell * c0 = &heightField.cellList[cellOffset];

    VertexC verts[25], * dv = verts;
    F32 fogs[25], * f = fogs;
    for (z0 = z; z0 <= zend; z0 += meterStrideZ, c0 += cellStrideWidth)
    {
      Cell * c = c0;
      for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, c += cellStrideX, f++)
      {
        dv->vv.x = (F32) x0;
        dv->vv.z = (F32) z0;
        dv->vv.y = c->height;

        dv->nv = normList[c->normal];

        dv->diffuse = c->color;
        if (Vid::Var::Terrain::shroud)
        {
          *f = (F32) c->GetFog() * U8toNormF32;
          dv->diffuse.Modulate( *f, *f, *f);
        }
      }
    }

    // reset loop variables
    c0 = &heightField.cellList[cellOffset];

    // submit cells
    U32 vcount = dv - verts;

//    Bool softS    = shroud && *softShroud;

    if (clus.status.water && Vid::Var::Terrain::water)
    {
      lowWaterHeight += clus.waterHeight;
      lowWaterCount++;

      if (!Vid::renderState.status.mirror)
      {
        RenderCellWaterV( clus, x, z, verts, fogs, vcount, clipFlags);
      }
    }

    for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth)
    {
      Cell *c = c0;
      for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX)
      {
        if ((!c->GetVisible() && c->GetFog() >= *Vid::Var::Terrain::shroudFog) /* || (isInvisS && c->GetFog() >= *Vid::Var::Terrain::shroudFog) */)
        {
          continue;
        }

        // 0*\--*3
        //  | \ |
        // 1*--\*2
        //
        U16 iv[4];

        iv[0] = (U16)(vcount);
        iv[1] = (U16)(vcount + 5);
        iv[2] = (U16)(vcount + 6);
        iv[3] = (U16)(vcount + 1);

        Bitmap *tex = texList[c->texture];
        Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);

        VertexC * vertmem;
        U16 *     indexmem;
        if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
        {
          LOG_WARN( ("Terrain::RenderCluster: can't lock bucket!") );
          return;
        }

        UVPair * uvList = &cellUVList[c->uv];
        VertexC * dv = vertmem;
        for (U32 i = 0; i < 4; i++, dv++)
        {
          U32 index = iv[i];

          const VertexC & sv = verts[index];

          dv->vv = sv.vv;
          dv->nv = sv.nv;
          dv->diffuse = sv.diffuse;

          dv->uv = uvList[i];
        }
        Utils::Memcpy( indexmem, Vid::rectIndices, 12);

        Vid::UnlockIndexedPrimitiveMem( 4, 6);

    #ifdef DOSTATISTICS
        if (clipFlags == clipNONE)
        {
          Statistics::noClipTris += 2;
        }
        else
        {
          Statistics::clipTris += 2;
        }
    #endif

        if (*Vid::Var::Terrain::overlay && (c->flags & Cell::cellOVERLAY))
        {
          ASSERT( c->texture1 < overlayCount && c->overlay < overlays.count);

          Overlay & overlay =  overlays[c->overlay];
          Bitmap * tex = overlayList[c->texture1];
//          Bool bright = (overlay.blend & RS_TEX_MASK) == RS_TEX_DECAL ? TRUE : FALSE;

          Vid::SetTranBucketZMax( Vid::sortTERRAIN0);
          Vid::SetBucketFlags( RS_BLEND_MODULATE | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP));
          Vid::SetBucketTexture( tex, TRUE, 0, RS_BLEND_MODULATE | renderFlags);

          VertexC * vertmem1;
          if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem1, 4, &indexmem, 6, &clus))
          {
            LOG_WARN( ("Terrain::RenderCluster: can't lock overlay bucket!") );
            return;
          }

          UVPair  * uvList  = &overlay.uvs[c->uv1];
          Utils::Memcpy( vertmem1, vertmem, sizeof(VertexC) * 4);

          vertmem1[0].uv = uvList[0];
          vertmem1[1].uv = uvList[overlay.size.x+1];
          vertmem1[2].uv = uvList[overlay.size.x+2];
          vertmem1[3].uv = uvList[1];

          Utils::Memcpy( indexmem, Vid::rectIndices, 12);

          Vid::UnlockIndexedPrimitiveMem( 4, 6);

    #ifdef DOSTATISTICS
        if (clipFlags == clipNONE)
        {
          Statistics::noClipTris += 2;
        }
        else
        {
          Statistics::clipTris += 2;
        }
    #endif

        }
      }
    }
    BucketMan::forceTranslucent = FALSE;
  }
  //----------------------------------------------------------------------------

  void RenderClusterOffMapV( S32 x, S32 z)
  {
    S32 xc = x / (S32) heightField.meterPerCell;
    S32 zc = z / (S32) heightField.meterPerCell;
    ASSERT(xc != 0 || zc != 0);
/*
    if (zc > 0 || xc < heightField.cellWidth)
    {
      return;
    }
*/
    F32 xs = F32(x) - OffsetX();
    F32 zs = F32(z) - OffsetZ();

    F32 xend = xs + (F32) meterPerClus;
    F32 zend = zs + (F32) meterPerClus;

    F32 xy[5], zy[5], dx, dxx, dz = 0, dzz = 0, ymin = F32_MAX, ymax = -F32_MAX;

    S32 zcc = zc;
    S32 xcc = xc;

    S32 corners = 0;

    if (xc < 0)
    {
      for (U32 i = 0; i < 5; i++, zcc++)
      {
        S32 tzc = zcc < 0 ? 0 : zcc > (S32) heightField.cellHeight ? heightField.cellHeight : zcc;

        xy[i] = heightField.cellList[ tzc * heightField.cellPitch].height;

        ymin = Min<F32>( ymin, xy[i]);
        ymax = Max<F32>( ymax, xy[i]);
      }
      dx = (F32) -xc * (F32) heightField.meterPerCell;
      dxx =  - (F32) heightField.meterPerCell;

      corners |= 1;
    }
    else if (xc >= (S32) heightField.cellWidth)
    {
      for (U32 i = 0; i < 5; i++, zcc++)
      {
        S32 tzc = zcc < 0 ? 0 : zcc > (S32) heightField.cellHeight ? heightField.cellHeight : zcc;

        xy[i] = heightField.cellList[ tzc * heightField.cellPitch + heightField.cellWidth].height;

        ymin = Min<F32>( ymin, xy[i]);
        ymax = Max<F32>( ymax, xy[i]);
      }
      dx = (F32(xc) - F32(heightField.cellWidth)) * (F32) heightField.meterPerCell;
      dxx = (F32) heightField.meterPerCell;

      corners |= 1;
    }
    else
    {
      for (U32 i = 0; i < 5; i++, zcc++)
      {
        S32 tzc = zcc < 0 ? 0 : zcc > (S32) heightField.cellHeight ? heightField.cellHeight : zcc;

        xy[i] = heightField.cellList[ tzc * heightField.cellPitch + xc].height;

        ymin = Min<F32>( ymin, xy[i]);
        ymax = Max<F32>( ymax, xy[i]);
      }
      dx = 0;
      dxx = (F32) heightField.meterPerCell;
    }

    if (zc < 0)
    {
      for (U32 i = 0; i < 5; i++, xcc++)
      {
        S32 txc = xcc < 0 ? 0 : xcc > (S32) heightField.cellWidth  ? heightField.cellWidth : xcc;

        zy[i] = heightField.cellList[ txc].height;

        ymin = Min<F32>( ymin, zy[i]);
        ymax = Max<F32>( ymax, zy[i]);
      }
      dz = (F32) -zc * (F32) heightField.meterPerCell;
      dzz = - (F32) heightField.meterPerCell;

      corners |= 2;
    }
    else if (zc >= (S32) heightField.cellHeight)
    {
      for (U32 i = 0; i < 5; i++, xcc++)
      {
        S32 txc = xcc < 0 ? 0 : xcc > (S32) heightField.cellWidth  ? heightField.cellWidth : xcc;

        zy[i] = heightField.cellList[ heightField.cellHeight * heightField.cellPitch + txc].height;
        
        ymin = Min<F32>( ymin, zy[i]);
        ymax = Max<F32>( ymax, zy[i]);
      }

      dz = F32(zc - heightField.cellHeight) * (F32) heightField.meterPerCell;
      dzz = (F32) heightField.meterPerCell;

      corners |= 2;
    }

//  DyDx = (dy12 * dz02 - dy02 * dz12) *  dx;
//	DyDz = (dy12 * dx02 - dy02 * dx12) * -dx;

    Vector offset( (xend + xs) * .5f, (ymin + ymax) * .5f, (zend + zs) * .5f); 
    Bounds bounds;
    bounds.Set( (F32) fabs( xend - xs), ymax - ymin, (F32) fabs( zend - zs)); 

    U32 clipFlags = Vid::CurCamera().BoundsTestBox( offset, bounds);
    if (clipFlags == clipOUTSIDE)
    {
      // cluster is completely outside the view frustrum
      return;
    }

    Vid::SetBucketFlags( RS_BLEND_DEF | DP_DONOTLIGHT | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP) );

    Vid::SetTranBucketZMax( Vid::sortTERRAIN0 + 1);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( NULL, FALSE);

    F32 dxy[5], dzy[5];
    for (U32 iz = 0; iz < 5; iz++)
    {
      dxy[iz] = (offMapHeight - xy[iz]) / 1000;
      dzy[iz] = (offMapHeight - zy[iz]) / 1000;
    }

    VertexC * vertmem;
    U16 *    indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 25, &indexmem, 96))
    {
      LOG_WARN( ("Terrain::RenderCluster: can't lock bucket!") );
      return;
    }
    VertexC * v = vertmem;

    iz = 0;
    for ( zs; zs <= zend; zs += heightField.meterPerCell, iz++, dz += dzz, zc++)
    {
      U32 ix = 0;
      F32 ddx = dx;
      S32 xcc = xc;
      for (F32 x = xs; x <= xend; x += heightField.meterPerCell, ix++, v++, ddx += dxx, xcc++)
      {
        v->vv.x = x;
        v->vv.z = zs;
        v->nv.Zero();
        v->diffuse  = 0xff000000;

        switch (corners)
        {
        case 3:
        {
          v->vv.y = xy[iz] + dxy[iz] * ddx;

          F32 zz = zs < 0 ? -zs : zs >= (S32) heightField.meterHeight ? zs - heightField.meterHeight : zs;
          F32 xx = x  < 0 ? -x  : x  >= (S32) heightField.meterWidth  ? x  - heightField.meterWidth  : x;

          if (zz > xx)
          {
            F32 dy = zy[ix] + dzy[ix] * dz;
            dy = (dy - v->vv.y) / zz;

            v->vv.y += dy * (zz - xx);
          }
          break;
        }
        case 2:
          v->vv.y = zy[ix] + dzy[ix] * dz;
          break;
        case 1:
          v->vv.y = xy[iz] + dxy[iz] * ddx;
          break;
        }
      }
    }
    Utils::Memcpy( indexmem, clusterI, 96 << 1);

    Vid::UnlockIndexedPrimitiveMem( 25, 96);

#ifdef DOSTATISTICS
    if (clipFlags == clipNONE)
    {
      Statistics::noClipTris += 2;
    }
    else
    {
      Statistics::clipTris += 2;
    }
#endif

  }
  //----------------------------------------------------------------------------

  // draw the terrain
  //
  void RenderV()
  {
    if (Vid::renderState.status.mirror)
    {
      waterInView = FALSE;
      waterRect.p0.x = waterRect.p0.y =  S32_MAX;
      waterRect.p1.x = waterRect.p1.y = -S32_MAX;
//      lowWaterHeight = F32_MAX;
    }

    if (lowWaterFirst)
    {
      lowWaterHeight = 0;
      lowWaterCount = 0;
    }

    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_CVERTEX,
      RS_BLEND_DEF | renderFlags);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetTranBucketZMax( Vid::sortTERRAIN0 + 1);

    // returns rect of meters that might be visible
	  Area<S32> rect;
    GetVisibleClusterRect( rect);

    S32 l = rect.p0.x;
    S32 t = rect.p0.y;
    if (l < 0)
    {
      l = 0;
    }
    if (t < 0)
    {
      t = 0;
    }

  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    // clip to the actual terrain rectangle
/*
    if (rect.p0.x < 0)
    {
      rect.p0.x = 0;
    }
    if (rect.p1.x > (S32) clusWidth)
    {
      rect.p1.x = clusWidth;
    }
    if (rect.p0.y < 0)
    {
      rect.p0.y = 0;
    }
    if (rect.p1.y > (S32) clusHeight)
    {
      rect.p1.y = clusHeight;
    }
*/
    // re-convert to meters
    rect.p0.y *= meterPerClus;
    rect.p1.y *= meterPerClus;
    rect.p0.x *= meterPerClus;
    rect.p1.x *= meterPerClus;

    S32 z, x;
    for (z = rect.p0.y; z < rect.p1.y; z += meterPerClus)
    {
      for (x = rect.p0.x; x < rect.p1.x; x += meterPerClus)
      {
        F32 coz = (F32) z * clusPerMeter;
        F32 cox = (F32) x * clusPerMeter;

        S32 clusOffz = Utils::FtoL( coz);
        S32 clusOffx = Utils::FtoL( cox);

        if (clusOffz < 0 || clusOffz >= (S32) clusHeight || clusOffx < 0 || clusOffx >= (S32) clusWidth)
        {
          if (!Vid::renderState.status.mirrorIn)
          {
            RenderClusterOffMapV( x, z);
          }
          continue;
        }

        Cluster & clus = clusList[clusOffz * clusWidth + clusOffx];
        Bounds  & bounds = clus.bounds;

        Vector viewOrigin;
        U32 clipFlags = Vid::CurCamera().BoundsTestOrigin( bounds.Offset(), bounds, &viewOrigin);
        if (clipFlags == clipOUTSIDE)
        {
          // cluster is completely outside the view frustrum
          continue;
        }
        clus.zDepth = viewOrigin.z;
        U32 offset = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;
        S32 xs = x - Utils::FtoL(OffsetX());    // FIXME
        S32 zs = z - Utils::FtoL(OffsetZ());

        if (clus.status.water)
        {
          if (Vid::renderState.status.mirror)
          {
            waterInView = TRUE;
            if (clusOffx < waterRect.p0.x)
            {
              waterRect.p0.x = clusOffx;
            }
            if (clusOffz < waterRect.p0.y)
            {
              waterRect.p0.y = clusOffz;
            }
            if (clusOffx > waterRect.p1.x)
            {
              waterRect.p1.x = clusOffx;
            }
            if (clusOffz > waterRect.p1.y)
            {
              waterRect.p1.y = clusOffz;
            }
          }
        }

        Vid::Light::SetActiveList( bounds.Offset(), bounds);

        RenderClusterV( clus, xs, zs, offset, 1, 1, clipFlags);
      }
    }

  #ifdef DOSTATISTICS
    Statistics::terrainTris = Statistics::tempTris;
  #endif
  }
  //----------------------------------------------------------------------------

  void RenderShroudMirrorMaskV( Cluster & clus, VertexC * verts0, U32 clipFlags)
  {
    clus;

    VertexC vertmem[4];
    vertmem[0] = verts0[0];
    vertmem[1] = verts0[20];
    vertmem[2] = verts0[25];
    vertmem[3] = verts0[4];
    vertmem[0].diffuse = 0xff000000;
    vertmem[1].diffuse = 0xff000000;
    vertmem[2].diffuse = 0xff000000;
    vertmem[3].diffuse = 0xff000000;

    Vid::DrawIndexedPrimitive( 
      PT_TRIANGLELIST,
      FVF_VERTEX,
      vertmem, 4, Vid::rectIndices, 6, RS_BLEND_MODULATE | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP) );
  }
  //----------------------------------------------------------------------------

  void RenderCellMirrorMaskV( Cluster & clus, U32 cellOffset, S32 x, S32 z, U32 clipFlags)
  {
    clipFlags &= ~clipPLANE0;

    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    S32 meterStrideX    = heightField.meterPerCell;
    S32 meterStrideZ    = heightField.meterPerCell;
    S32 cellStrideWidth = heightField.cellPitch;

    Cell * c0 = &heightField.cellList[cellOffset];

    VertexC vertmem[25], * dv = vertmem, * ve = vertmem + 25;
    F32 fogs[25], * f = fogs;
    UVPair * uvlist = waterUVList0;
    for (z0 = z; z0 <= zend; z0 += meterStrideZ, c0 += cellStrideWidth)
    {
      Cell * c = c0;
      for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, c += 1, f++, uvlist++)
      {
        dv->vv.x = (F32) x0;
        dv->vv.z = (F32) z0;
        dv->vv.y = clus.waterHeight;

        dv->nv = waterNorms[0];

        dv->uv = *uvlist;

        dv->diffuse = waterColorMirror;

        if (Vid::Var::Terrain::shroud)
        {
          *f = (F32) c->GetFog() * U8toNormF32;
          dv->diffuse.Modulate( *f, *f, *f);
        }
      }
    }

    if (Vid::Var::Terrain::shroud && clus.shroudCount == 25)
    {
      RenderShroudMirrorMaskV( clus, vertmem, clipFlags);
      return;
    }

    Vid::DrawIndexedPrimitive( 
      PT_TRIANGLELIST,
      FVF_VERTEX,
      vertmem, 25, clusterI, 96, RS_BLEND_MODULATE | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP) );

    if (waterLayer2)
    {
      U32 a = U32(waterColorMirror.a * *Vid::Var::Terrain::waterAlphaTopFactor);

      UVPair * uvlist = waterUVList1;
      for (dv = vertmem; dv < ve; dv++, uvlist++)
      {
        dv->diffuse.a  = U8(a);
        dv->uv = *uvlist;
      }
      Vid::DrawIndexedPrimitive( 
        PT_TRIANGLELIST,
        FVF_TLVERTEX,
        vertmem, 25, clusterI, 96, RS_BLEND_MODULATE | renderFlags | ((clipFlags & clipALL) ? 0 : DP_DONOTCLIP) );
    }

#ifdef DOSTATISTICS
    if (clipFlags == clipNONE)
    {
      Statistics::noClipTris += 32;
    }
    else
    {
      Statistics::clipTris += 32;
    }
#endif
  }
  //----------------------------------------------------------------------------

  // draw just the water
  //
  void RenderMirrorMaskV( WaterRegion * waterR)
  {
    Bool alpha = Vid::SetAlphaState( TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetMaterial( mirrorMaterial);
    Vid::SetTexture( waterTex, 0, RS_BLEND_MODULATE);

    lowWaterHeight = 0;
    lowWaterCount = 0;
    lowWaterFirst = FALSE;

    // returns rect of meters that might be visible
	  Area<S32> rect;
    GetVisibleClusterRect( rect);

  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    // clip to the actual terrain rectangle
    if (rect.p0.x < 0)
    {
      rect.p0.x = 0;
    }
    if (rect.p1.x > (S32) clusWidth)
    {
      rect.p1.x = clusWidth;
    }
    if (rect.p0.y < 0)
    {
      rect.p0.y = 0;
    }
    if (rect.p1.y > (S32) clusHeight)
    {
      rect.p1.y = clusHeight;
    }

    // re-convert to meters
    rect.p0.y *= meterPerClus;
    rect.p1.y *= meterPerClus;
    rect.p0.x *= meterPerClus;
    rect.p1.x *= meterPerClus;

    S32 z, x;
    for (z = rect.p0.y; z < rect.p1.y; z += meterPerClus)
    {
      for (x = rect.p0.x; x < rect.p1.x; x += meterPerClus)
      {
        F32 coz = (F32) z * clusPerMeter;
        F32 cox = (F32) x * clusPerMeter;

        S32 clusOffz = Utils::FtoL( coz);
        S32 clusOffx = Utils::FtoL( cox);

        Cluster & clus = clusList[clusOffz * clusWidth + clusOffx];

	      if (!clus.status.water || clus.waterIndex != waterR - waterList.data)
        {
          continue;
        }

        Vector viewOrigin;
        U32 clipFlags = Vid::CurCamera().BoundsTestOrigin( clus.bounds.Offset(), clus.bounds, &viewOrigin);
        if (clipFlags == clipOUTSIDE)
        {
          // cluster is completely outside the view frustrum
          continue;
        }

        Vid::Light::SetActiveList( clus.bounds.Offset(), clus.bounds);

//        U32 offset = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;
        S32 xs = x - Utils::FtoL(OffsetX());    // FIXME
        S32 zs = z - Utils::FtoL(OffsetZ());

        U32 offset = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;

        RenderCellMirrorMaskV( clus, xs, zs, offset, clipFlags);

        if (!Vid::Var::Terrain::shroud || clus.shroudCount != 25)
        {
          lowWaterHeight += clus.waterHeight;
          lowWaterCount++;
        }
      }
    }

  #ifdef DOSTATISTICS
    Statistics::terrainTris = Statistics::tempTris;
  #endif

    Vid::SetAlphaState( alpha);
  }
  //----------------------------------------------------------------------------

}
//----------------------------------------------------------------------------
