//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// terrain_render_quick.cpp
//
// 04-MAY-1998
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

  // skip backface culling
  //
  void RenderClusterWaterQuick( Cluster & clus, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, U32 clipFlags)
  {
    // one quad covers the whole cluster with water
    //
    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | RS_BLEND_DEF);
    Vid::SetTranBucketZMax( Vid::sortWATER0);

    Color c = *Vid::Var::Terrain::waterColorBottom;
    Vid::SetBucketTexture( waterTex, waterIsAlpha);

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

    if (clipFlags == clipNONE)
    {
      VertexTL *vertmem;
      U16 *indexmem;

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
      {
        return;
      }

#ifdef DOSTATISTICS
      Statistics::noClipTris += 2;
#endif

      vertmem[0].vv.x = (F32) x0;
      vertmem[0].vv.y = y + wave0;
      vertmem[0].vv.z = (F32) z0;

      vertmem[1].vv.x = (F32) x0;
      vertmem[1].vv.y = y + wave1;
      vertmem[1].vv.z = (F32) z1;

      vertmem[2].vv.x = (F32) x1;
      vertmem[2].vv.y = y + wave0;
      vertmem[2].vv.z = (F32) z1;

      vertmem[3].vv.x = (F32) x1;
      vertmem[3].vv.y = y + wave1;
      vertmem[3].vv.z = (F32) z0;

      U32 i;
      for (i = 0; i < 4; i++)
      {
        Vid::ProjectFromModel_I( vertmem[i]);

        vertmem[i].diffuse   = normLights[0];
        vertmem[i].diffuse.a = c.a;
        vertmem[i].specular  = 0xff000000;
        vertmem[i].SetFog();

        vertmem[i].uv = waterUVList0[i];
      }

      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);

      if (waterLayer2)
      {
        // 2nd layer
        //
        VertexTL *vertmem1;

        if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem1, 4, &indexmem, 6, &clus))
        {
          return;
        }

        c = *Vid::Var::Terrain::waterColorTop;

        Utils::Memcpy( vertmem1,  vertmem, sizeof(VertexTL) * 4);
        U32 i;
        for (i = 0; i < 4; i++)
        {
          vertmem1[i].uv = waterUVList1[i];
          vertmem1[i].diffuse.a  = c.a;
        }
        Utils::Memcpy( indexmem, Vid::rectIndices, 12);

        Vid::UnlockIndexedPrimitiveMem( 4, 6);
      }
    }
    else
    {
      VertexTL vertmem[4], temp[4];

      temp[0].vv.x = (F32) x0;
      temp[0].vv.y = y + wave0;
      temp[0].vv.z = (F32) z0;

      temp[1].vv.x = (F32) x0;
      temp[1].vv.y = y + wave1;
      temp[1].vv.z = (F32) z1;

      temp[2].vv.x = (F32) x1;
      temp[2].vv.y = y + wave0;
      temp[2].vv.z = (F32) z1;

      temp[3].vv.x = (F32) x1;
      temp[3].vv.y = y + wave1;
      temp[3].vv.z = (F32) z0;


      Vector norms[4];
      U32 i;
      for (i = 0; i < 4; i++)
      {
        Vid::TransformFromModel( temp[i]);

        temp[i].diffuse   = normLights[0];
        temp[i].diffuse.a = c.a;
        temp[i].specular  = 0xff000000;

        vertmem[i] = temp[i];
        vertmem[i].uv = waterUVList0[i];
      }
      Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, TRUE, clipFlags);

      if (waterLayer2)
      {
        // 2nd layer
        //
        c = *Vid::Var::Terrain::waterColorTop;

        for (U32 i = 0; i < 4; i++)
        {
          vertmem[i] = temp[i];
          vertmem[i].diffuse.a  = c.a;
          vertmem[i].uv = waterUVList1[i];
        }
        Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, TRUE, clipFlags);
      }
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterQuick( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, U32 clipFlags) // = clipALL
  {
    // build list of vertex positions, normals, and colors
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    if (clus.status.water && Vid::Var::Terrain::water)
    {
      if (!Vid::renderState.status.mirror)
      {
        RenderClusterWaterQuick( clus, x, z, xend, zend, clus.waterHeight, clipFlags);
	    }
    }

    S32 meterStrideX    = abs( cellStrideX) * heightField.meterPerCell;
    S32 meterStrideZ    = abs( cellStrideZ) * heightField.meterPerCell;
    S32 cellStrideWidth = cellStrideZ * heightField.cellPitch;

    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags);
    Vid::SetTranBucketZMax( Vid::sortTERRAIN0);

/*
    if (Vid::Clip::BoundsTestAlphaFar( clus.zDepth, clus.bounds.Radius()))
    {
      BucketMan::forceTranslucent = TRUE;
    }
*/

    Cell *c0 = &heightField.cellList[cellOffset];

    // transform verts
    //
    Vector verts[25], * v;
    VertexTL tempmem[25];
    VertexTL * dvv = tempmem;
    for (v = verts, z0 = z; z0 <= zend; z0 += meterStrideZ, c0 += cellStrideWidth)
    {
      Cell *c = c0;
      for (x0 = x; x0 <= xend; x0 += meterStrideX, c += cellStrideX, dvv++, v++)
      {
        v->x = (F32) x0;
        v->z = (F32) z0;
        v->y = c->height;

#ifdef DOTERRAINCOLOR
        Color col = c->color;
        dvv->diffuse.ModulateInline( normLights[c->normal], 
          (F32)col.r * U8toNormF32,
          (F32)col.g * U8toNormF32,
          (F32)col.b * U8toNormF32,
          (F32)col.a * U8toNormF32);
#else
        dvv->diffuse = normLights[c->normal]; 
#endif
        dvv->specular = 0xff000000;

        if (Vid::Var::Terrain::shroud)
        {
          F32 dff = (F32) c->GetFog() * U8toNormF32;
          dvv->diffuse.ModulateInline( dff, dff, dff);
        }

        if (clipFlags == clipNONE)
        {
          Vid::ProjectFromModel_I( *dvv, *v);
          dvv->SetFog();
        }
        else
        {
          Vid::TransformFromModel( *dvv, *v);
        }
      }
    }

    // reset loop variables
    c0 = &heightField.cellList[cellOffset];

    U32 vcount = dvv - tempmem;

    if (clipFlags == clipNONE)
    {
      for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth)
      {
        Cell *c = c0;
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX)
        {
          if ((!c->GetVisible() && c->GetFog() != 0) || (*Vid::Var::Terrain::invisibleShroud && c->GetFog() == 0))
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

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[3]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::Math::modelViewVector) <= 0.0f
	  	     && planes[1].Evalue(Vid::Math::modelViewVector) <= 0.0f)
          {
            continue;
          }

          Bitmap *tex = texList[c->texture];
          Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);

          VertexTL * vertmem;
          U16 *      indexmem;
          if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
          {
            LOG_WARN( ("Terrain::RenderCluster: can't lock bucket!") );
            return;
          }

#ifdef DOSTATISTICS
          Statistics::noClipTris += 2;
#endif

          UVPair *uvList = &cellUVList[c->uv];

          U32 i;
          for (i = 0; i < 4; i++)
          {
            VertexTL &dv = vertmem[i];
            const VertexTL &sv = tempmem[iv[i]];

            dv.vv  = sv.vv;
            dv.rhw = sv.rhw;
            dv.diffuse  = sv.diffuse;
            dv.specular = sv.specular;

            dv.uv  = uvList[i];
          }
          Utils::Memcpy( indexmem, Vid::rectIndices, 12);

          Vid::UnlockIndexedPrimitiveMem( 4, 6);

        }
      }
    }
    else
    {
      for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth)
      {
        Cell *c = c0;
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX)
        {
          if ((!c->GetVisible() && c->GetFog() != 0) || (*Vid::Var::Terrain::invisibleShroud && c->GetFog() == 0))
          {
            continue;
          }
          U16 iv[4];

          iv[0] = (U16)(vcount);
          iv[1] = (U16)(vcount + 5);
          iv[2] = (U16)(vcount + 6);
          iv[3] = (U16)(vcount + 1);

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[3]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::Math::modelViewVector) <= 0.0f
	  	     && planes[1].Evalue(Vid::Math::modelViewVector) <= 0.0f)
          {
            continue;
          }
          Bitmap *tex = texList[c->texture];
          Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);

          VertexTL vertmem[4];
          UVPair *uvList = &cellUVList[c->uv];

          U32 i;
          for (i = 0; i < 4; i++)
          {
            VertexTL &dv = vertmem[i];
            VertexTL &sv = tempmem[iv[i]];
          
            dv.vv  = sv.vv;
            dv.rhw = sv.rhw;
            dv.diffuse  = sv.diffuse;
            dv.specular = 0xff000000;

            dv.uv  = uvList[i];
          }

          Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, TRUE, clipFlags);
        }
      }
    }
    BucketMan::forceTranslucent = FALSE;
  }
  //----------------------------------------------------------------------------

  // draw the terrain
  //
  void RenderQuick()
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
      FVF_TLVERTEX,
//      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF ,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetTranBucketZMax( Vid::sortTERRAIN0 + 1);

    // recalc lighting values for normal list
    CalcLighting();

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
          lowWaterHeight += clus.waterHeight;
          lowWaterCount++;
        }

        RenderClusterQuick( clus, xs, zs, offset, 1, 1, clipFlags);
      }
    }

  #ifdef DOSTATISTICS
    Statistics::terrainTris = Statistics::tempTris;
  #endif
  }
  //----------------------------------------------------------------------------

  // skip backface culling
  //
  void RenderClusterMirrorMaskQuick( Cluster &clus, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, U32 clipFlags)
  {
    clus;

    // one quad covers the whole cluster with water
    //

/*
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
*/
    VertexTL vertmem[4];

    if (clipFlags == clipNONE)
    {
      vertmem[0].vv.x = (F32) x0;
      vertmem[0].vv.y = y;
      vertmem[0].vv.z = (F32) z0;

      vertmem[1].vv.x = (F32) x0;
      vertmem[1].vv.y = y;
      vertmem[1].vv.z = (F32) z1;

      vertmem[2].vv.x = (F32) x1;
      vertmem[2].vv.y = y;
      vertmem[2].vv.z = (F32) z1;

      vertmem[3].vv.x = (F32) x1;
      vertmem[3].vv.y = y;
      vertmem[3].vv.z = (F32) z0;

      for (U32 i = 0; i < 4; i++)
      {
        Vid::ProjectFromModel_I( vertmem[i]);

        vertmem[i].diffuse   = normLights[0];
        vertmem[i].diffuse.a = waterColorMirror.a;
        vertmem[i].specular  = 0xff000000;
        vertmem[i].SetFog();

        vertmem[i].uv = waterUVList0[i];
      }

      Vid::DrawIndexedPrimitive( 
        PT_TRIANGLELIST,
        FVF_TLVERTEX,
        vertmem, 4, Vid::rectIndices, 6, 
        DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);

  #ifdef DOSTATISTICS
      Statistics::tempTris += 2;
  #endif

      if (waterLayer2)
      {
        U32 a = U32(waterColorMirror.a * *Vid::Var::Terrain::waterAlphaTopFactor);

        for (i = 0; i < 4; i++)
        {
          vertmem[i].diffuse.a  = U8(a);
          vertmem[i].uv = waterUVList1[i];
        }
        Vid::DrawIndexedPrimitive( 
          PT_TRIANGLELIST,
          FVF_TLVERTEX,
          vertmem, 4, Vid::rectIndices, 6, 
          DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);


    #ifdef DOSTATISTICS
        Statistics::tempTris += 2;
    #endif
      }
    }
    else
    {
      VertexTL temp[4], verts[4];

      temp[0].vv.x = (F32) x0;
      temp[0].vv.y = y;
      temp[0].vv.z = (F32) z0;

      temp[1].vv.x = (F32) x0;
      temp[1].vv.y = y;
      temp[1].vv.z = (F32) z1;

      temp[2].vv.x = (F32) x1;
      temp[2].vv.y = y;
      temp[2].vv.z = (F32) z1;

      temp[3].vv.x = (F32) x1;
      temp[3].vv.y = y;
      temp[3].vv.z = (F32) z0;

      for (U32 i = 0; i < 4; i++)
      {
        Vid::TransformFromModel( temp[i]);

        temp[i].diffuse  = normLights[0];
        temp[i].diffuse.a  = waterColorMirror.a;
        temp[i].specular = 0xff000000;

        verts[i] = temp[i];
        verts[i].uv = waterUVList0[i];
      }

      U32 vcount = 4, icount = 6;

      VertexTL * vertmem;
      U16 * indexmem;
      U32 heapSize = Vid::Heap::ReqVertex( &vertmem, &indexmem);

      Vid::Clip::ToBuffer( vertmem, indexmem, verts, vcount, Vid::rectIndices, icount, TRUE, clipFlags);

      if (icount)
      {
        Vid::DrawIndexedPrimitive( 
          PT_TRIANGLELIST,
          FVF_TLVERTEX,
          vertmem, vcount, indexmem, icount, 
          DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);

    #ifdef DOSTATISTICS
        icount /= 3;
        Statistics::tempTris += icount;
    #endif

        if (waterLayer2)
        {
          U32 a = U32(waterColorMirror.a * *Vid::Var::Terrain::waterAlphaTopFactor);

          for (i = 0; i < 4; i++)
          {
            verts[i] = temp[i];
            verts[i].diffuse.a  = U8(a);
            verts[i].uv = waterUVList1[i];
          }
          vcount = 4, icount = 6;

          Vid::Clip::ToBuffer( vertmem, indexmem, verts, vcount, Vid::rectIndices, icount, TRUE, clipFlags);

          if (icount)
          {
            Vid::DrawIndexedPrimitive( 
              PT_TRIANGLELIST,
              FVF_TLVERTEX,
              vertmem, vcount, indexmem, icount, 
              DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);

        #ifdef DOSTATISTICS
            icount /= 3;
            Statistics::tempTris += icount;
        #endif
          }
        }
      }

      Vid::Heap::Restore( heapSize);
    }
  }
  //----------------------------------------------------------------------------

  // draw just the water
  //
  void RenderMirrorMaskQuick( WaterRegion * waterR)
  {
    Bool alpha = Vid::SetAlphaState( TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetMaterial( Vid::defMaterial);
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
        Vid::Light::SetupLightsModel();

//        U32 offset = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;
        S32 xs = x - Utils::FtoL(OffsetX());    // FIXME
        S32 zs = z - Utils::FtoL(OffsetZ());

        S32 xend = x + meterPerClus;
        S32 zend = z + meterPerClus;

        lowWaterHeight += clus.waterHeight;
        lowWaterCount++;

        RenderClusterMirrorMaskQuick( clus, xs, zs, xend, zend, clus.waterHeight, clipFlags);
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
