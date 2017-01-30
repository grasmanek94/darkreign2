//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// terrain.cpp     DR2 terrain system
//
// 04-MAY-1998
//

#include "random.h"
#include "console.h"
#include "statistics.h"
#include "terrain.h"
#include "terrain_priv.h"
#include "vidclip.h"
#include "vidcmd.h"
//----------------------------------------------------------------------------

namespace Terrain
{
  UVPair overlayUVList[256] = {
  // row by row, from front to back (0 - 1)

    // 4x4
    UVPair(0,0), UVPair(.25f,0), UVPair(.5f,0), UVPair(.75f,0), UVPair(1,0),
    UVPair(0,.25f), UVPair(.25f,.25f), UVPair(.5f,.25f), UVPair(.75f,.25f), UVPair(1,.25f),
    UVPair(0,.5f), UVPair(.25f,.5f), UVPair(.5f,.5f), UVPair(.75f,.5f), UVPair(1,.5f),
    UVPair(0,.75f), UVPair(.25f,.75f), UVPair(.5f,.75f), UVPair(.75f,.75f), UVPair(1,.75f),
    UVPair(0,1), UVPair(.25f,1), UVPair(.5f,1), UVPair(.75f,1), UVPair(1,1),
  };

  // skip backface culling
  //
  void RenderClusterWater( Cluster &clus, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, U32 clipFlags)
  {
    // one quad covers the whole cluster with water
    //
    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | RS_BLEND_DEF);
    Vid::SetTranBucketZMaxPlus();

    Color c = *waterColorBottom;
    Vid::SetBucketTexture( waterTex, waterIsAlpha);

    Camera &cam = Vid::CurCamera();

    F32 wave0 = 0, wave1 = 0;
    if (*waveActive)
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

      cam.LightProjectVerts( vertmem, waterNorms, 4);

      U32 i;
      for (i = 0; i < 4; i++)
      {
        vertmem[i].diffuse.a = c.a;
        vertmem[i].uv = waterUVList0[i];
      }
      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);

  #ifdef DOSTATISTICS
      Statistics::tempTris += 2;
      Statistics::noClipTris = *Statistics::noClipTris + 2;
  #endif

      if (waterLayer2)
      {
        // 2nd layer
        //
        VertexTL *vertmem1;
        U16 *indexmem1;

        if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem1, 4, &indexmem1, 6, &clus))
        {
          return;
        }

        c = *waterColorTop;

        Utils::Memcpy( vertmem1,  vertmem, sizeof(VertexTL) * 4);
        U32 i;
        for (i = 0; i < 4; i++)
        {
          vertmem1[i].uv = waterUVList1[i];
          vertmem[i].diffuse.a  = c.a;
        }
        Utils::Memcpy( indexmem1, Vid::rectIndices, 12);

        Vid::UnlockIndexedPrimitiveMem( 4, 6);
      }
    }
    else
    {
      for (S32 l2 = waterLayer2, l2 >= 0, l2--)
      {
        VertexTL vertmem[4];

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

  #ifdef DOSETHOMO
        cam.LightSetHomogeneousVerts( vertmem, waterNorms, 4);
  #else
        cam.LightTransformFromModelSpace( vertmem, waterNorms, 4);
  #endif

        for (U32 i = 0; i < 4; i++)
        {
          vertmem[i].diffuse.a = c.a;
          vertmem[i].uv = waterUVList0[i];
        }
        Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, clipFlags);

        if (waterLayer2)
        {
          // 2nd layer
          //

          c = *waterColorTop;

          for (U32 i = 0; i < 4; i++)
          {
            vertmem1[i].uv = waterUVList1[i];
            vertmem[i].diffuse.a  = c.a;
          }
          Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, clipFlags);
        }
      }
    }
  }
  //----------------------------------------------------------------------------

  // skip backface culling
  //
  void RenderClusterWaterQuick( Cluster &clus, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, U32 clipFlags)
  {
    // one quad covers the whole cluster with water
    //
    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | RS_BLEND_DEF);
    Vid::SetTranBucketZMaxPlus();

    Color c = *waterColorBottom;
    Vid::SetBucketTexture( waterTex, waterIsAlpha);

    Camera &cam = Vid::CurCamera();

    F32 wave0 = 0, wave1 = 0;
    if (*waveActive)
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
        cam.TransformProjectFromModelSpace( vertmem[i]);

        vertmem[i].diffuse   = normLights[0];
        vertmem[i].diffuse.a = c.a;
        vertmem[i].specular  = 0xff000000;
        vertmem[i].SetFog();

        vertmem[i].uv = waterUVList0[i];
      }

      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);

  #ifdef DOSTATISTICS
      Statistics::noClipTris = *Statistics::noClipTris + 2;
      Statistics::tempTris += 2;
  #endif

      if (waterLayer2)
      {
        // 2nd layer
        //
        VertexTL *vertmem1;

        if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem1, 4, &indexmem, 6, &clus))
        {
          return;
        }

        c = *waterColorTop;

        Utils::Memcpy( vertmem1,  vertmem, sizeof(VertexTL) * 4);
        U32 i;
        for (i = 0; i < 4; i++)
        {
          vertmem1[i].uv = waterUVList1[i];
          vertmem[i].diffuse.a  = c.a;
        }
        Utils::Memcpy( indexmem, Vid::rectIndices, 12);

        Vid::UnlockIndexedPrimitiveMem( 4, 6);
      }
    }
    else
    {
      for (S32 l2 = waterLayer2, l2 >= 0, l2--)
      {
        VertexTL vertmem[4];

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

        Vector norms[4];
        U32 i;
        for (i = 0; i < 4; i++)
        {
  #ifdef DOSETHOMO
          cam.SetHomogeneousFromModelSpace( vertmem[i]);
  #else
          cam.TransformFromModelSpace( vertmem[i]);
  #endif
          vertmem[i].diffuse   = normLights[0];
          vertmem[i].diffuse.a = c.a;
          vertmem[i].specular  = 0xff000000;

          vertmem[i].uv = waterUVList0[i];
        }
        Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, clipFlags);
      }
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderCluster( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, U32 clipFlags) // = clipALL
  {
    // build list of vertex positions, normals, and colors
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    if (clus.status.water && *water)
    {
      RenderClusterWater( clus, x, z, xend, zend, clus.waterHeight, clipFlags);
	  }

    S32 meterStrideX    = abs( cellStrideX) * heightField.meterPerCell;
    S32 meterStrideZ    = abs( cellStrideZ) * heightField.meterPerCell;
    S32 cellStrideWidth = cellStrideZ * heightField.cellPitch;

    Camera &cam = Vid::CurCamera();

    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags);
    Vid::SetTranBucketZMax();
//    Vid::SetTranBucketZ( clus.viewOrigin.z);

#ifdef DOCLUSTERCELLS
    if (*clusterCells)
    {
      Cell * c = clus.cells;

      Vector verts[25];
      Vector norms[25];
      Color colors[25];
      F32     fogs[25];

      Vector *dv = verts;
      Vector *dn = norms;
      Color  *dc = colors;
      F32    *df = fogs;
      for (z0 = z; z0 <= zend; z0 += meterStrideZ)
      {
        for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, dn++, dc++, df++, c++)
        {
          if (shroud)
          {
            *df = fogFactorsF32[c->GetFog()];
          }

          dv->x = (F32) x0;
          dv->z = (F32) z0;
          dv->y = c->height;

          *dn = normList[c->normal];

#ifdef DOTERRAINCOLOR
          *dc = c->color;
#endif
        }
      }

      // reset loop variables
      c = clus.cells;

      // submit cells
      VertexTL * tempmem = Vid::tempVertices;
      U32 vcount = dv - verts;

      if (clipFlags == clipNONE)
      {
#ifdef DOTERRAINCOLOR
        cam.LightProjectVerts( tempmem, verts, norms, colors, vcount);
#else
        cam.LightProjectVerts( tempmem, verts, norms, vcount);
#endif
        if (shroud)
        {
          VertexTL *dvv = tempmem + vcount;
          for (dvv--, df--; dvv >= tempmem; dvv--, df--)
          {
            dvv->diffuse.ModulateInline( *df, *df, *df);
          }
        }

        for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++)
        {
          for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c++)
          {
            if (!c->GetVisible())
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
	  	      if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	       && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
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

    #ifdef DOSTATISTICS
            Statistics::noClipTris = *Statistics::noClipTris + 2;
            Statistics::tempTris += 2;
    #endif
          }
        }
      }
      else
      {
#ifdef DOSETHOMO
#ifdef DOTERRAINCOLOR
        cam.LightSetHomogeneousVerts( tempmem, verts, norms, colors, vcount);
#else
        cam.LightSetHomogeneousVerts( tempmem, verts, norms, vcount);
#endif
#else
#ifdef DOTERRAINCOLOR
        cam.LightTransformFromModelSpace( tempmem, verts, norms, colors, vcount);
#else
        cam.LightTransformFromModelSpace( tempmem, verts, norms, vcount);
#endif
#endif
        if (shroud)
        {
          VertexTL *dvv = tempmem + vcount;
          for (dvv--, df--; dvv >= tempmem; dvv--, df--)
          {
            dvv->diffuse.ModulateInline( *df, *df, *df);
          }
        }

        for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++)
        {
          for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c++)
          {
            if (!c->GetVisible())
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
	  	      if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	       && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
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
              dv.diffuse = sv.diffuse;
              dv.specular = sv.specular;

              dv.uv  = uvList[i];
            }

            Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, clipFlags);
          }
        }
      }
    }
    else
#endif
    {
      Cell *c0 = &heightField.cellList[cellOffset];

      Vector verts[25];
      Vector norms[25];
      Color colors[25];
      F32     fogs[25];

      Vector *dv = verts;
      Vector *dn = norms;
      Color  *dc = colors;
      F32    *df = fogs;
      for (z0 = z; z0 <= zend; z0 += meterStrideZ, c0 += cellStrideWidth)
      {
        Cell *c = c0;
        for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, dn++, dc++, df++, c += cellStrideX)
        {
          if (shroud)
          {
            *df = fogFactorsF32[c->GetFog()];
          }

          dv->x = (F32) x0;
          dv->z = (F32) z0;
          dv->y = c->height;

          *dn = normList[c->normal];

#ifdef DOTERRAINCOLOR
          *dc = c->color;
#endif
        }
      }

      // reset loop variables
      c0 = &heightField.cellList[cellOffset];

      // submit cells
      VertexTL * tempmem = Vid::tempVertices;

      U32 vcount = dv - verts;

      if (clipFlags == clipNONE)
      {
#ifdef DOTERRAINCOLOR
        cam.LightProjectVerts( tempmem, verts, norms, colors, vcount);
#else
        cam.LightProjectVerts( tempmem, verts, norms, vcount);
#endif
        if (shroud)
        {
          VertexTL *dvv = tempmem + vcount;
          for (dvv--, df--; dvv >= tempmem; dvv--, df--)
          {
            dvv->diffuse.ModulateInline( *df, *df, *df);
          }
        }

        for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth)
        {
          Cell *c = c0;
          for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX)
          {
            if (!c->GetVisible())
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
	  	      if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	       && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
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

            UVPair * uvList = &cellUVList[c->uv];
            for (U32 i = 0; i < 4; i++)
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

            if (*overlay && (c->flags & Cell::cellOVERLAY))
            {
              ASSERT( c->texture1 < overlayCount && c->overlay < overlays.count);

              Bitmap * tex = overlayList[c->texture1];
              Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);

              if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
              {
                LOG_WARN( ("Terrain::RenderCluster: can't lock overlay bucket!") );
                return;
              }

              for (U32 i = 0; i < 4; i++)
              {
                VertexTL &dv = vertmem[i];
                const VertexTL &sv = tempmem[iv[i]];

                dv.vv  = sv.vv;
                dv.rhw = sv.rhw;
                dv.diffuse  = sv.diffuse;
                dv.specular = sv.specular;
              }
              Overlay & overlay =  overlays[c->overlay];
              UVPair  * uvList  = &overlay.uvs[c->uv1];
              vertmem[0].uv = uvList[0];
              vertmem[1].uv = uvList[overlay.size.x+1];
              vertmem[2].uv = uvList[overlay.size.x+2];
              vertmem[3].uv = uvList[1];

              Utils::Memcpy( indexmem, Vid::rectIndices, 12);
  
              Vid::UnlockIndexedPrimitiveMem( 4, 6);

    #ifdef DOSTATISTICS
              Statistics::noClipTris = *Statistics::noClipTris + 2;
              Statistics::tempTris += 2;
    #endif
            }

    #ifdef DOSTATISTICS
            Statistics::noClipTris = *Statistics::noClipTris + 2;
            Statistics::tempTris += 2;
    #endif
          }
        }
      }
      else
      {
#ifdef DOSETHOMO
#ifdef DOTERRAINCOLOR
        cam.LightSetHomogeneousVerts( tempmem, verts, norms, colors, vcount);
#else
        cam.LightSetHomogeneousVerts( tempmem, verts, norms, vcount);
#endif
#else
#ifdef DOTERRAINCOLOR
        cam.LightTransformFromModelSpace( tempmem, verts, norms, colors, vcount);
#else
        cam.LightTransformFromModelSpace( tempmem, verts, norms, vcount);
#endif
#endif
        if (shroud)
        {
          VertexTL *dvv = tempmem + vcount;
          for (dvv--, df--; dvv >= tempmem; dvv--, df--)
          {
            dvv->diffuse.ModulateInline( *df, *df, *df);
          }
        }

        for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth)
        {
          Cell *c = c0;
          for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX)
          {
            if (!c->GetVisible())
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
	  	      if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	       && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
            {
              continue;
            }
            Bitmap *tex = texList[c->texture];
            Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);

            VertexTL vertmem[4];
            UVPair *uvList = &cellUVList[c->uv];
            for (U32 i = 0; i < 4; i++)
            {
              VertexTL &dv = vertmem[i];
              VertexTL &sv = tempmem[iv[i]];
          
              dv.vv  = sv.vv;
              dv.rhw = sv.rhw;
              dv.diffuse = sv.diffuse;
              dv.specular = sv.specular;

              dv.uv  = uvList[i];
            }

            Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, clipFlags);

            if (*overlay && (c->flags & Cell::cellOVERLAY))
            {
              ASSERT( c->texture1 < overlayCount && c->overlay < overlays.count);

              Bitmap * tex = overlayList[c->texture1];
              Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);

              Overlay & overlay =  overlays[c->overlay];
              UVPair  * uvList  = &overlay.uvs[c->uv1];
              for (U32 i = 0; i < 4; i++)
              {
                // ClipToBucket calls SetHomogeneous
                VertexTL &dv = vertmem[i];
                VertexTL &sv = tempmem[iv[i]];
                dv.vv  = sv.vv;
                dv.rhw = sv.rhw;
              }
              vertmem[0].uv = uvList[0];
              vertmem[1].uv = uvList[overlay.size.x+1];
              vertmem[2].uv = uvList[overlay.size.x+2];
              vertmem[3].uv = uvList[1];

              Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, clipFlags);
            }
          }
        }
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

    if (clus.status.water && *water)
    {
      RenderClusterWaterQuick( clus, x, z, xend, zend, clus.waterHeight, clipFlags);
	  }

    S32 meterStrideX    = abs( cellStrideX) * heightField.meterPerCell;
    S32 meterStrideZ    = abs( cellStrideZ) * heightField.meterPerCell;
    S32 cellStrideWidth = cellStrideZ * heightField.cellPitch;

    Camera &cam = Vid::CurCamera();

    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags);
    Vid::SetTranBucketZMax();
//    Vid::SetTranBucketZ( clus.viewOrigin.z);

    Cell *c0 = &heightField.cellList[cellOffset];

    // transform verts
    //
    Vector verts[25], * v;
    VertexTL * tempmem = Vid::tempVertices;
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

        if (shroud)
        {
          F32 df = fogFactorsF32[c->GetFog()];
          dvv->diffuse.ModulateInline( df, df, df);
        }

        if (clipFlags == clipNONE)
        {
          cam.TransformProjectFromModelSpace( *dvv, *v);
          dvv->SetFog();
        }
        else
        {
#ifdef DOSETHOMO
          cam.SetHomogeneousFromModelSpace( *dvv, *v);
#else
          cam.TransformFromModelSpace( *dvv, *v);
#endif
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
          if (!c->GetVisible())
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
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
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

  #ifdef DOSTATISTICS
          Statistics::noClipTris = *Statistics::noClipTris + 2;
          Statistics::tempTris += 2;
  #endif
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
          if (!c->GetVisible())
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
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
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

          Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, clipFlags);
        }
      }
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterColor( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, Color color, U32 clipFlags) // = 0
  {
    clus;

    Camera &cam = Vid::CurCamera();

    S32 meterStrideX    = abs( cellStrideX) * heightField.meterPerCell;
    S32 meterStrideZ    = abs( cellStrideZ) * heightField.meterPerCell;
    S32 cellStrideWidth = cellStrideZ * heightField.cellPitch;

    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags);
    Vid::SetBucketTexture(defTex, FALSE, 0, RS_BLEND_DEF | renderFlags);

    // fill the vertex memory
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    Vector verts[25];

    if (clipFlags == clipNONE)
    {
      VertexTL *vertmem;
      U16 *indexmem;

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 25, &indexmem, 96, &clus))
      {
        return;
      }

      Cell *c0 = &heightField.cellList[cellOffset];
      VertexTL *dv = vertmem;
      Vector *sv = verts;
      F32 vv = 0.0f;
      for (z0 = z; z0 <= zend; z0 += meterStrideZ, vv += 1.0f, c0 += cellStrideWidth)
      {
        Cell *c = c0;

        F32 uu = 0.0f;
        for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, sv++, uu += 1.0f, c += cellStrideX)
        {
          sv->x = (F32) x0;
          sv->z = (F32) z0;
          sv->y = c->height;
          dv->diffuse  = color;
          dv->specular = 0xff000000;
          dv->uv.u = uu;
          dv->uv.v = vv;

          cam.TransformProjectFromModelSpace( *dv, *sv);

          dv->SetFog();
        }
      }

      U16 *iv = indexmem;
      U32 vcount = 0;
      for (z0 = z; z0 < zend; z0 += meterStrideZ, vcount++)
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++)
        {
          // 0*\--*3
          //  | \ |
          // 1*--\*2
          //
          iv[0] = (U16)(vcount);
          iv[1] = (U16)(vcount + 5);
          iv[2] = (U16)(vcount + 6);
          iv[5] = (U16)(vcount + 1);

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[5]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
          // set indices
          iv[3] = iv[0];
          iv[4] = iv[2];

          iv += 6;

  #ifdef DOSTATISTICS
          Statistics::noClipTris = *Statistics::noClipTris + 2;
          Statistics::tempTris += 2;
  #endif
        }
      }

      Vid::UnlockIndexedPrimitiveMem( dv - vertmem, iv - indexmem);
    }
    else
    {
      VertexTL *vertmem = Vid::tempVertices;
      U16 *indexmem = Vid::tempIndices;

      Cell *c0 = &heightField.cellList[cellOffset];
      VertexTL *dv = vertmem;
      Vector *sv = verts;
      F32 vv = 0.0f;
      for (z0 = z; z0 <= zend; z0 += meterStrideZ, vv += 1.0f, c0 += cellStrideWidth)
      {
        Cell *c = c0;

        F32 uu = 0.0f;
        for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, sv++, uu += 1.0f, c += cellStrideX)
        {
          sv->x = (F32) x0;
          sv->z = (F32) z0;
          sv->y = c->height;
          dv->diffuse  = color;
          dv->specular = 0xff000000;
          dv->uv.u = uu;
          dv->uv.v = vv;

#ifdef DOSETHOMO
          cam.SetHomogeneousFromModelSpace( *dv, *sv);
#else
          cam.TransformFromModelSpace( *dv, *sv);
#endif
        }
      }

      U16 *iv = indexmem;
      U32 vcount = 0;
      for (z0 = z; z0 < zend; z0 += meterStrideZ, vcount++)
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++)
        {
          // 0*\--*3
          //  | \ |
          // 1*--\*2
          //
          iv[0] = (U16)(vcount);
          iv[1] = (U16)(vcount + 5);
          iv[2] = (U16)(vcount + 6);
          iv[5] = (U16)(vcount + 1);

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[5]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
          // set indices
          iv[3] = iv[0];
          iv[4] = iv[2];

          iv += 6;
        }
      }
      Vid::Clip::ToBucket( vertmem, dv - vertmem, indexmem, iv - indexmem, &clus, clipFlags);
    }
  }
  //----------------------------------------------------------------------------

  // draw a cell's normals; used by editing brush
  //
  void RenderCellNormal( Cell &cell, F32 x, F32 z)
  {
    static Color color = RGBA_MAKE( 0, 255, 255, 255);

    Camera &cam = Vid::CurCamera();

    F32 h = cell.height;
    Vector norm = normList[cell.normal];
    norm *= 2.0f;

    // lock vertex memory
	  VertexTL *vertmem;
	  U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 3, &indexmem, 3, &cell))
	  {
      return;
    }

	  vertmem[0].vv.x = x;
	  vertmem[0].vv.y = h;
	  vertmem[0].vv.z = z;
	  vertmem[0].diffuse  = color;
	  vertmem[0].specular = 0xff000000;

    cam.TransformProjectFromModelSpace( vertmem[0]);

    vertmem[0].vv.z *= Vid::renderState.zBias;

	  vertmem[1].vv.x = x + norm.x;
	  vertmem[1].vv.y = h + norm.y;
	  vertmem[1].vv.z = z + norm.z;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = 0xff000000;

    cam.TransformProjectFromModelSpace( vertmem[1]);

    vertmem[1].vv.z *= Vid::renderState.zBias;

	  vertmem[2] = vertmem[0];
    vertmem[0].vv.x -= 1.0f;
    vertmem[2].vv.x += 1.0f;

    Utils::Memcpy( indexmem, Vid::rectIndices, 6);

    // submit the polygons
    Vid::UnlockIndexedPrimitiveMem( 3, 3);
  }
  //----------------------------------------------------------------------------


  // draw a single cell
  //
  void RenderCell( Cluster &clus, Cell &c0, Cell &c1, Cell &c2, Cell &c3, S32 x0, S32 z0, S32 x1, S32 z1, Color color, U32 clipFlags)
  {
    Camera &cam = Vid::CurCamera();

    UVPair *uvList = &cellUVList[c0.uv];

    if (clipFlags == clipNONE)
    {
      VertexTL *vertmem;
      U16 *indexmem;

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
      {
        return;
      }

      vertmem[0].vv.x = (F32) x0;
      vertmem[0].vv.z = (F32) z0;
      vertmem[0].vv.y = c0.height;

      vertmem[1].vv.x = (F32) x0;
      vertmem[1].vv.z = (F32) z1;
      vertmem[1].vv.y = c1.height;

      vertmem[2].vv.x = (F32) x1;
      vertmem[2].vv.z = (F32) z1;
      vertmem[2].vv.y = c2.height;

      vertmem[3].vv.x = (F32) x1;
      vertmem[3].vv.z = (F32) z0;
      vertmem[3].vv.y = c3.height;

      U32 i;
      for (i = 0; i < 4; i++)
      {
        VertexTL &dv = vertmem[i];

        dv.uv = uvList[i];
        dv.diffuse = color;
        dv.specular = 0xff000000;

        cam.TransformProjectFromModelSpace( dv);
      }

      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);

  #ifdef DOSTATISTICS
      Statistics::noClipTris = *Statistics::noClipTris + 2;
      Statistics::tempTris += 2;
  #endif
    }
    else
    {
      VertexTL vertmem[4];

      vertmem[0].vv.x = (F32) x0;
      vertmem[0].vv.z = (F32) z0;
      vertmem[0].vv.y = c0.height;

      vertmem[1].vv.x = (F32) x0;
      vertmem[1].vv.z = (F32) z1;
      vertmem[1].vv.y = c1.height;

      vertmem[2].vv.x = (F32) x1;
      vertmem[2].vv.z = (F32) z1;
      vertmem[2].vv.y = c2.height;

      vertmem[3].vv.x = (F32) x1;
      vertmem[3].vv.z = (F32) z0;
      vertmem[3].vv.y = c3.height;

      U32 i;
      for (i = 0; i < 4; i++)
      {
        VertexTL &dv = vertmem[i];

        dv.uv = uvList[i];
        dv.diffuse = color;
        dv.specular = 0xff000000;

#ifdef DOSETHOMO
        cam.SetHomogeneousFromModelSpace( dv);
#else
        cam.TransformFromModelSpace( dv);
#endif
      }
      Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, clipFlags);
    }
  }
  //----------------------------------------------------------------------------

  // draw a rectangle of cells; used by the edit brush
  //
  void RenderCellRect( Area<S32> & rect, Color color, Bool dofill, Bool doFade, Bool doTile, Bitmap * tex, F32 rotation) // = FALSE, = FALSE, = TRUE, = NULL. = 0
  {  
    ASSERT(rect.p0.x < rect.p1.x && rect.p0.y < rect.p1.y);
    ASSERT( rect.p0.x >= 0 && rect.p1.x <= (S32) heightField.cellWidth 
         && rect.p0.y >= 0 && rect.p1.y <= (S32) heightField.cellHeight);

    if (!tex)
    {
      // use the default edit texture
      tex = editTex;
    }

    Camera &cam = Vid::CurCamera();

    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_TEXCLAMP | RS_BLEND_DEF,
      TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetTranBucketZ( 22.0f);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( NULL);

    S32 wid = rect.Width();
    S32 hgt = rect.Height();

  #define MAXCELLS      ((MAXBRUSHX + 1) * (MAXBRUSHZ + 1))
    ASSERT( (wid + 1) * (hgt + 1) < MAXVERTS && (wid) * (hgt) * 6 < MAXINDICES);

    S32 meterStride = heightField.meterPerCell;
    S32 x = rect.p0.x * meterStride + (S32)OffsetX();
    S32 z = rect.p0.y * meterStride + (S32)OffsetZ();
    S32 xend = x + wid * meterStride;
    S32 zend = z + hgt * meterStride;
    Cell *c0;
    S32 x0, x1, z0, z1;
    S32 cellOffset = rect.p0.y * heightField.cellPitch + rect.p0.x;

    Cluster &clus = GetCluster( (F32) x, (F32) z);

    F32 duu = doTile ? 1.0f : 1.0f / wid;
    F32 dvv = doTile ? 1.0f : 1.0f / hgt;

    S32 wid1 = wid + 1;
    S32 hgt1 = hgt + 1;

    if (Mesh::Manager::showNormals)
    {
      Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_BLEND_DEF);
  
      S32 offset = cellOffset;

      for (z0 = z; z0 <= zend; z0 += meterStride)
      {
        c0 = &heightField.cellList[offset];
        for (x0 = x; x0 <= xend; x0 += meterStride)
        {
          if (x0 >= 0 && x0 < (S32) heightField.meterWidth && z0 >= 0 && z0 < (S32) heightField.meterHeight)
          {
            RenderCellNormal( *c0, (F32) x0, (F32) z0);
          }

          c0 += 1;
        }
        offset += heightField.cellPitch;
      }

      Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_TEXCLAMP | RS_BLEND_DEF);
    }

    S32 hue, sat, lum;
    CalculateHLS( color, hue, sat, lum);

    Color invColor = color;
    if (doFade)
    {
      invColor = lum > 128 ? Color( (U32) 0, (U32) 0, (U32) 0, color.a) : Color( (U32) 255, (U32) 255, (U32) 255, color.a);
    }

    Matrix mat;
    if (rotation)
    {
      mat.Set( Quaternion( rotation, Matrix::I.up));
    }

    Vid::SetBucketTexture( tex, TRUE, 0, doTile ? RS_BLEND_DEF : RS_TEXCLAMP | RS_BLEND_DEF);
    if (dofill)
    {
      Cell *c0 = &heightField.cellList[cellOffset];

      VertexTL *vertmem = Vid::tempVertices;
      U16 *indexmem = Vid::tempIndices;

      U16 *iv = indexmem;
      VertexTL *dv = vertmem;
      U32 vcount = 0;

      UVPair uv( doTile ? F32(wid) : 1, 0);

      for (z0 = z; z0 < zend; z0 += meterStride, uv.v += dvv, vcount++)
      {
        Cell *c = c0;
        uv.u = doTile ? F32(wid) : 1;

        for (x0 = x; x0 < xend; x0 += meterStride, dv++, uv.u -= duu, c += 1, vcount++)
        {
          dv->vv.x = (F32) x0;
          dv->vv.z = (F32) z0;
          dv->vv.y = c->height;
          dv->diffuse  = color;
          dv->specular = 0xff000000;

          if (rotation)
          {
            UVPair uv1 = uv;
            uv1.u -= .5f;
            uv1.v -= .5f;
//            mat.Rotate( dv->uv, uv1);
		        dv->uv.u = uv1.u * mat.right.x + uv1.v * mat.front.x;
        		dv->uv.v = uv1.u * mat.right.z + uv1.v * mat.front.z;
            dv->uv.u += .5f;
            dv->uv.v += .5f;
          }
          else
          {
            dv->uv = uv;
          }
          dv->uv.u += tex->UVShiftWidth();
          dv->uv.v += tex->UVShiftHeight();

#ifdef DOSETHOMO
          cam.SetHomogeneousFromModelSpace(*dv);
#else
          cam.TransformFromModelSpace( *dv);
#endif
          if (x0 >= 0 && x0 < (S32) heightField.meterWidth && z0 >= 0 && z0 < (S32) heightField.meterHeight)
          {
            // 0*\--*3
            //  | \ |
            // 1*--\*2
            //
            iv[0] = (U16)(vcount);
            iv[1] = (U16)(vcount + wid1);
            iv[2] = (U16)(vcount + wid1 + 1);
            iv[5] = (U16)(vcount + 1);

            iv[3] = iv[0];
            iv[4] = iv[2];

            iv += 6;
          }
        }

        dv->vv.x = (F32) x0;
        dv->vv.z = (F32) z0;
        dv->vv.y = c->height;
//        dv->diffuse  = color;
        dv->diffuse  = invColor;
        dv->specular = 0xff000000;

        if (rotation)
        {
          UVPair uv1 = uv;
          uv1.u -= .5f;
          uv1.v -= .5f;
//            mat.Rotate( dv->uv, uv1);
		      dv->uv.u = uv1.u * mat.right.x + uv1.v * mat.front.x;
        	dv->uv.v = uv1.u * mat.right.z + uv1.v * mat.front.z;
          dv->uv.u += .5f;
          dv->uv.v += .5f;
        }
        else
        {
          dv->uv = uv;
        }
        dv->uv.u += tex->UVShiftWidth();
        dv->uv.v += tex->UVShiftHeight();

#ifdef DOSETHOMO
        cam.SetHomogeneousFromModelSpace(*dv);
#else
        cam.TransformFromModelSpace( *dv);
#endif
        dv++;
      
        c0 += heightField.cellPitch;
      }

      Cell *c = c0;
      uv.u = doTile ? F32(wid) : 1;
      for (x0 = x; x0 <= xend; x0 += meterStride, dv++, uv.u -= duu, c += 1)
      {
        dv->vv.x = (F32) x0;
        dv->vv.z = (F32) z0;
        dv->vv.y = c->height;
//        dv->diffuse  = color;
        dv->diffuse  = invColor;
        dv->specular = 0xff000000;

        if (rotation)
        {
          UVPair uv1 = uv;
          uv1.u -= .5f;
          uv1.v -= .5f;
//            mat.Rotate( dv->uv, uv1);
		      dv->uv.u = uv1.u * mat.right.x + uv1.v * mat.front.x;
        	dv->uv.v = uv1.u * mat.right.z + uv1.v * mat.front.z;
          dv->uv.u += .5f;
          dv->uv.v += .5f;
        }
        else
        {
          dv->uv = uv;
        }
        dv->uv.u += tex->UVShiftWidth();
        dv->uv.v += tex->UVShiftHeight();

#ifdef DOSETHOMO
        cam.SetHomogeneousFromModelSpace(*dv);
#else
        cam.TransformFromModelSpace( *dv);
#endif
      }

      Vid::Clip::ToBucket( vertmem, dv - vertmem, indexmem, iv - indexmem, &clus);
    }
    else
    {
      // top cell strip
      c0 = &heightField.cellList[cellOffset];
      Cell *c1 = c0 + heightField.cellPitch;
      Cell *c2 = c1 + 1;
      Cell *c3 = c2 - heightField.cellPitch;

      z0 = z;
      z1 = z + meterStride;
      for (x0 = x, x1 = x + meterStride; x0 < xend; x0 += meterStride, x1 += meterStride)
      {
        RenderCell( clus, *c0, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);

        c0 += 1;
        c1 += 1;
        c2 += 1;
        c3 += 1;
      }

      // bottom cell strip
      z0 = zend - meterStride;
      z1 = zend;
      c0 = &heightField.cellList[cellOffset + (hgt1 - 1) * heightField.cellPitch];
      c1 = c0 + heightField.cellPitch;
      c2 = c1 + 1;
      c3 = c2 - heightField.cellPitch;
      for (x0 = x, x1 = x + meterStride; x0 < xend; x0 += meterStride, x1 += meterStride)
      {
        RenderCell( clus, *c0, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);

        c0 += 1;
        c1 += 1;
        c2 += 1;
        c3 += 1;
      }

      // left cell strip
      x0 = x;
      x1 = x + meterStride;
      z += meterStride;
      zend -= meterStride;
      c0 = &heightField.cellList[cellOffset + heightField.cellPitch];
      c1 = c0 + heightField.cellPitch;
      c2 = c1 + 1;
      c3 = c2 - heightField.cellPitch;
      for (z0 = z, z1 = z + meterStride; z0 < zend; z0 += meterStride, z1 += meterStride)
      {
        RenderCell( clus, *c0, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);

        c0 += heightField.cellPitch;
        c1 += heightField.cellPitch;
        c2 += heightField.cellPitch;
        c3 += heightField.cellPitch;
      }

      // right cell strip
      x0 = xend - meterStride;
      x1 = xend;
      c0 = &heightField.cellList[cellOffset + heightField.cellPitch + wid1 - 1];
      c1 = c0 + heightField.cellPitch;
      c2 = c1 + 1;
      c3 = c2 - heightField.cellPitch;
      for (z0 = z, z1 = z + meterStride; z0 < zend; z0 += meterStride, z1 += meterStride)
      {
        RenderCell( clus, *c0, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);

        c0 += heightField.cellPitch;
        c1 += heightField.cellPitch;
        c2 += heightField.cellPitch;
        c3 += heightField.cellPitch;
      }
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cell; used for current 'click' location display
  //
  void RenderCell( Cell &cell, Color color, Bitmap *tex) // = NULL
  {
    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF,
      TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetTranBucketZ( 100.0f);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( tex ? tex : texList[cell.texture]);

    U32 offset = (U32) (&cell - heightField.cellList);

    U32 z0 = (offset / heightField.cellPitch) + (S32)OffsetZ();
    U32 x0 = (offset - z0 * heightField.cellPitch) * heightField.meterPerCell + (S32)OffsetX();
    z0 *= heightField.meterPerCell;
    U32 z1 = z0 + heightField.meterPerCell;
    U32 x1 = x0 + heightField.meterPerCell;

    Cell *c1 = &cell + heightField.cellPitch;
    Cell *c2 = c1 + 1;
    Cell *c3 = c2 - heightField.cellPitch;

    Cluster &clus = GetCluster( (F32) x0, (F32) z0);

    RenderCell( clus, cell, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);
  }
  //----------------------------------------------------------------------------

  Bool GetVisibleWaterRect( Area<S32> & rect)
  {
    rect = waterRect;

    return waterInView;
  }
  //----------------------------------------------------------------------------

  Bool GetVisibleClusterRect( Area<S32> & rect)
  {
    Vid::CurCamera().GetVisibleRect( rect);

    rect.p0.x -= Utils::FtoL(OffsetX());    // FIXME
    rect.p1.x -= Utils::FtoL(OffsetX());
    rect.p0.y -= Utils::FtoL(OffsetZ());
    rect.p1.y -= Utils::FtoL(OffsetZ());

    // convert it to cluster coords 
    rect.p0.x = Utils::FtoL((F32) rect.p0.x * clusPerMeter);
    rect.p1.x = Utils::FtoL((F32) rect.p1.x * clusPerMeter);
    rect.p0.y = Utils::FtoL((F32) rect.p0.y * clusPerMeter);
    rect.p1.y = Utils::FtoL((F32) rect.p1.y * clusPerMeter);
  
    // increase bounds by one along each edge to be sure everything gets drawn
    rect.p0.x--;
    rect.p1.x++;
    rect.p0.y--;
    rect.p1.y++;

    return TRUE;
  }
  //----------------------------------------------------------------------------

  // draw the terrain
  //
  void Render()
  {
    if (Vid::renderState.status.mirror)
    {
      waterInView = FALSE;
      waterRect.p0.x = waterRect.p0.y =  S32_MAX;
      waterRect.p1.x = waterRect.p1.y = -S32_MAX;
    }

  //#define DODRAWSUNARROW
  #ifdef DODRAWSUNARROW
    MeshRoot *arrowMesh = Mesh::Manager::FindLoad( "engine_cone.god");

    Matrix m =  Sky::sun->WorldMatrix();
    m.posit = Vid::CurCamera().WorldMatrix().Position();
    m.posit += Vid::CurCamera().WorldMatrix().Front() * 10.0f;
    arrowMesh->Render( m, Color( 0l, 222l, 0l, 122l));  
  #endif

    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
//      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF ,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags,
      TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetTranBucketZMaxPlus();

    Bool quick = *lightQuick || *lightMap;
    if (quick)
    {
      // recalc lighting values for normal list
	    CalcLighting();
    }

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

#ifdef DOTERRAINLOD
    Cluster *lodClusList[1000];
    U32 lodClusCount = 0;
    Bool lod = *lodActive;
    lodDist = lodDistFactor * Vid::CurCamera().FarPlane();
    Area<S32> cRect;
#endif
    {
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

#ifdef DOTERRAINLOD
      if (lod)
      {
        cRect = rect;
        cRect.p0.x -= 1;
        cRect.p1.x += 1;
        cRect.p0.y -= 1;
        cRect.p1.y += 1;

        // clip to the actual terrain rectangle
        if (cRect.p0.x < 0)
        {
          cRect.p0.x = 0;
        }
        if (cRect.p1.x > (S32) clusWidth)
        {
          cRect.p1.x = clusWidth;
        }
        if (cRect.p0.y < 0)
        {
          cRect.p0.y = 0;
        }
        if (cRect.p1.y > (S32) clusHeight)
        {
          cRect.p1.y = clusHeight;
        }

        S32 wid = cRect.p1.x - cRect.p0.x;
        Cluster *c0  = &clusList[cRect.p0.y * clusWidth + cRect.p0.x];
        Cluster *ce0 = &clusList[cRect.p1.y * clusWidth + cRect.p0.x];
        for ( ; c0 < ce0; c0 += clusWidth)
        {
          Cluster *c  = c0;
          Cluster *ce = c0 + wid;
          for ( ; c < ce; c++)
          {
            SetClusLod( c->offset, FALSE);
          }
        }
      }
#endif

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

          Cluster &clus = clusList[clusOffz * clusWidth + clusOffx];
          Bounds &bounds = clus.bounds;

          Vector viewOrigin;
          U32 clipFlags = Vid::CurCamera().BoundsTest( bounds.Offset(), bounds, &viewOrigin);
          if (clipFlags == clipOUTSIDE)
          {
            // cluster is completely outside the view frustrum
            continue;
          }
          U32 offset = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;
          S32 xs = x + Utils::FtoL(OffsetX());    // FIXME
          S32 zs = z + Utils::FtoL(OffsetZ());

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

          if (!quick)
          {
            DxLight::Manager::SetActiveList( bounds.Offset(), bounds);
            DxLight::Manager::SetupLightsModelSpace();

#ifdef DOTERRAINLOD
            if (lod)
            {
              clus.clipFlags = clipFlags;

              if (viewOrigin.z >= lodDist)
              {
                lodClusList[lodClusCount] = &clus;
                lodClusCount++;
                ASSERT( lodClusCount < 1000);
              }
              else
              {
                SetClusLod( clus.offset, TRUE);
                RenderCluster( clus, clus.x, clus.z, clus.offset, 1, 1, clipFlags);
              }
            }
            else
#endif
            {
              RenderCluster( clus, xs, zs, offset, 1, 1, clipFlags);
            }
          }
          else
          {
#ifdef DOTERRAINLOD
            if (lod)
            {
              clus.clipFlags = clipFlags;

              if (viewOrigin.z >= lodDist)
              {
                lodClusList[lodClusCount] = &clus;
                lodClusCount++;
                ASSERT( lodClusCount < 1000);
              }
              else
              {
                SetClusLod( clus.offset, TRUE);
                RenderClusterQuick( clus, clus.x, clus.z, clus.offset, 1, 1, clipFlags);
              }
            }
            else
#endif
            {
              RenderClusterQuick( clus, xs, zs, offset, 1, 1, clipFlags);
            }
          }
        }
      }
    }
#ifdef DOTERRAINLOD
    if (lod)
    {
      Cluster **c  = &lodClusList[0];
      Cluster **ce = &lodClusList[lodClusCount];

      for ( ; c < ce; c++)
      {
        Cluster &clus = **c;

        F32 py = (F32)fabs(Vid::CurCamera().RHW( clus.viewOrigin.z) * Vid::CurCamera().ProjConstY());

        U32 o0 = clus.offset;
        U32 o1 = o0 + 4 * CellPitch();
        U32 o2 = o0 + 4;
        U32 o3 = o1 + 4;

        ChekTri( py, o0, o1, o2);
        ChekTri( py, o3, o2, o1);
      }

      for (c = ce - 1; c >= lodClusList; c--)
      {
        Cluster &clus = **c;
        if (quick)
        {
          RenderClusterMRM( clus, clus.x, clus.z, clus.offset, clus.clipFlags);
        }
        else
        {
          RenderClusterMRMQuick( clus, clus.x, clus.z, clus.offset, clus.clipFlags);
        }
      }
    }
#endif

  #ifdef DOSTATISTICS
    Statistics::terrainTris = Statistics::tempTris;
  #endif
  }
  //----------------------------------------------------------------------------

  // skip backface culling
  //
  void RenderClusterMirrorMask( Cluster &clus, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, U32 clipFlags)
  {
    clus;

    // one quad covers the whole cluster with water
    //
    Bool quick = *lightQuick || *lightMap;

    Camera &cam = Vid::CurCamera();

    F32 wave0 = 0, wave1 = 0;
    if (*waveActive)
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

    VertexTL vertmem[4];

    if (clipFlags == clipNONE)
    {
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
      if (quick)
      {
        for (i = 0; i < 4; i++)
        {
          cam.TransformProjectFromModelSpace( vertmem[i]);

          vertmem[i].diffuse  = normLights[0];
          vertmem[i].diffuse.a  = Vid::Mirror::waterColor.a;
          vertmem[i].specular = 0xff000000;

          vertmem[i].SetFog();

          vertmem[i].uv = waterUVList0[i];
        }
      }
      else
      {
        cam.LightProjectVerts( vertmem, waterNorms, 4);

        for (i = 0; i < 4; i++)
        {
          vertmem[i].uv = waterUVList0[i];
          vertmem[i].diffuse.a  = Vid::Mirror::waterColor.a;
        }
      }

      Vid::DrawIndexedPrimitive( 
        PT_TRIANGLELIST,
        FVF_TLVERTEX,
        vertmem, 4, Vid::rectIndices, 6, 
        DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);

  #ifdef DOSTATISTICS
      Statistics::noClipTris = *Statistics::noClipTris + 2;
      Statistics::tempTris += 2;
  #endif

      if (waterLayer2)
      {
        for (i = 0; i < 4; i++)
        {
          vertmem[i].uv = waterUVList1[i];
        }
        Vid::DrawIndexedPrimitive( 
          PT_TRIANGLELIST,
          FVF_TLVERTEX,
          vertmem, 4, Vid::rectIndices, 6, 
          DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);


    #ifdef DOSTATISTICS
        Statistics::noClipTris = *Statistics::noClipTris + 2;
        Statistics::tempTris += 2;
    #endif
      }
    }
    else
    {
      for (S32 l2 = waterLayer2, l2 >= 0, l2--)
      {
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
        if (quick)
        {
          for (i = 0; i < 4; i++)
          {
  #ifdef DOSETHOMO
            cam.SetHomogeneousFromModelSpace( vertmem[i]);
  #else
            cam.TransformFromModelSpace( vertmem[i]);
  #endif

            vertmem[i].diffuse  = normLights[0];
            vertmem[i].diffuse.a  = Vid::Mirror::waterColor.a;
            vertmem[i].specular = 0xff000000;

            vertmem[i].uv = waterUVList0[i];
          }
        }
        else
        {
  #ifdef DOSETHOMO
          cam.LightSetHomogeneousVerts( vertmem, waterNorms, 4);
  #else
          cam.LightTransformFromModelSpace( vertmem, waterNorms, 4);
  #endif

          for (i = 0; i < 4; i++)
          {
            vertmem[i].uv = waterUVList0[i];
            vertmem[i].diffuse.a  = Vid::Mirror::waterColor.a;
          }
        }

        U32 vcount = 4, icount = 6;

        Vid::Clip::ToBuffer( Vid::tempVertices, Vid::tempIndices, 
          vertmem, vcount, Vid::rectIndices, icount);

        if (icount)
        {
          Vid::DrawIndexedPrimitive( 
            PT_TRIANGLELIST,
            FVF_TLVERTEX,
            Vid::tempVertices, vcount, Vid::tempIndices, icount, 
            DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);

      #ifdef DOSTATISTICS
          icount /= 3;
          Statistics::noClipTris = *Statistics::noClipTris + icount;
          Statistics::tempTris += icount;
      #endif
        }
      }
    }
  }
  //----------------------------------------------------------------------------

  // draw just the water
  //
  void RenderMirrorMask()
  {
    Bool alpha = Vid::SetAlphaState( TRUE);
    Vid::SetWorldTransform( Matrix::I);
    Vid::SetMaterial( Vid::defMaterial);
    Vid::SetTexture( waterTex, 0, RS_BLEND_MODULATE);

    Bool quick = *lightQuick || *lightMap;

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

        Cluster &clus = clusList[clusOffz * clusWidth + clusOffx];

	      if (!clus.status.water)
        {
          continue;
        }

        Vector viewOrigin;
        U32 clipFlags = Vid::CurCamera().BoundsTest( clus.bounds.Offset(), clus.bounds, &viewOrigin);
        if (clipFlags == clipOUTSIDE)
        {
          // cluster is completely outside the view frustrum
          continue;
        }

        if (!quick)
        {
          DxLight::Manager::SetActiveList( clus.bounds.Offset(), clus.bounds);
          DxLight::Manager::SetupLightsModelSpace();
        }

//        U32 offset = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;
        S32 xs = x + Utils::FtoL(OffsetX());    // FIXME
        S32 zs = z + Utils::FtoL(OffsetZ());

        S32 xend = x + meterPerClus;
        S32 zend = z + meterPerClus;

        RenderClusterMirrorMask( clus, xs, zs, xend, zend, clus.waterHeight, clipFlags);
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
