//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// terrain_render_vertextl.cpp
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
  UVPair overlayUVList[256] = {
  // row by row, from front to back (0 - 1)

    // 4x4
    UVPair(0,0), UVPair(.25f,0), UVPair(.5f,0), UVPair(.75f,0), UVPair(1,0),
    UVPair(0,.25f), UVPair(.25f,.25f), UVPair(.5f,.25f), UVPair(.75f,.25f), UVPair(1,.25f),
    UVPair(0,.5f), UVPair(.25f,.5f), UVPair(.5f,.5f), UVPair(.75f,.5f), UVPair(1,.5f),
    UVPair(0,.75f), UVPair(.25f,.75f), UVPair(.5f,.75f), UVPair(.75f,.75f), UVPair(1,.75f),
    UVPair(0,1), UVPair(.25f,1), UVPair(.5f,1), UVPair(.75f,1), UVPair(1,1),
  };
  //----------------------------------------------------------------------------

  U16 clusterI[96] =
  {
    0, 5, 6, 0, 6, 1,         // row0
    1, 6, 7, 1, 7, 2,
    2, 7, 8, 2, 8, 3,
    3, 8, 9, 3, 9, 4,

    5, 10, 11, 5, 11, 6,      // row1
    6, 11, 12, 6, 12, 7,
    7, 12, 13, 7, 13, 8,
    8, 13, 14, 8, 14, 9,

    10, 15, 16, 10, 16, 11,   // row2
    11, 16, 17, 11, 17, 12,
    12, 17, 18, 12, 18, 13,
    13, 18, 19, 13, 19, 14,

    15, 20, 21, 15, 21, 16,   // row3
    16, 21, 22, 16, 22, 17,
    17, 22, 23, 17, 23, 18,
    18, 23, 24, 18, 24, 19,
  };
  //----------------------------------------------------------------------------

  // skip backface culling
  //
  void RenderShroudWaterVtl( Cluster & clus, Vector * verts0, U32 clipFlags)
  {
    Vid::SetBucketTexture( NULL, FALSE, 0, RS_BLEND_DEF);

    if (clipFlags == clipNONE)
    {
      VertexTL * vertmem;
      U16 * indexmem;
      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
      {
        return;
      }
      vertmem[0].vv = verts0[0];
      vertmem[0].diffuse  = 0xff000000;
      vertmem[0].specular = 0xff000000;
      vertmem[1].vv = verts0[20];
      vertmem[1].diffuse  = 0xff000000;
      vertmem[1].specular = 0xff000000;
      vertmem[2].vv = verts0[24];
      vertmem[2].diffuse  = 0xff000000;
      vertmem[2].specular = 0xff000000;
      vertmem[3].vv = verts0[4];
      vertmem[3].diffuse  = 0xff000000;
      vertmem[3].specular = 0xff000000;

      Vid::ProjectVerts( vertmem, 4, TRUE);

#ifdef DOSTATISTICS
      Statistics::noClipTris += 2;
#endif

      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);
    }
    else
    {
      VertexTL vertmem[4];

      vertmem[0].vv = verts0[0];
      vertmem[0].diffuse  = 0xff000000;
      vertmem[0].specular = 0xff000000;
      vertmem[1].vv = verts0[20];
      vertmem[1].diffuse  = 0xff000000;
      vertmem[1].specular = 0xff000000;
      vertmem[2].vv = verts0[24];
      vertmem[2].diffuse  = 0xff000000;
      vertmem[2].specular = 0xff000000;
      vertmem[3].vv = verts0[4];
      vertmem[3].diffuse  = 0xff000000;
      vertmem[3].specular = 0xff000000;

      Vid::TransformFromModel( vertmem, 4);

      Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, TRUE, clipFlags);
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
//  void RenderClusterColor( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, Color color, U32 clipFlags) // = 0

  void RenderCellWaterVtl( VertexTL * tempmem, Cluster & clus, S32 x0, S32 z0, Vector * verts0, Color * colors, F32 * fogs, U32 vcount, U32 clipFlags)
  {
    clipFlags &= ~clipPLANE0;

    Vid::SetBucketFlags( DP_DONOTLIGHT | DP_DONOTCLIP | RS_NOSORT | RS_BLEND_DEF);
    Vid::SetTranBucketZMax( Vid::sortWATER0 + 1);
    Vid::SetBucketTexture( waterTex, waterIsAlpha, 0, RS_BLEND_DEF);

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

    Vector verts[25];
    Vector * v = verts, * ve = verts + 25, * sv = verts0;
    F32 dw = (wave1 - wave0) / 4;
    for ( ; v < ve; wave0 += dw, wave1 -= dw)
    {
      Vector * vee = v + 4;
      F32 dww = (wave1 - wave0) / 4;
      for (F32 w = wave0; v <= vee; v++, sv++, w += dww)
      {
        v->x = sv->x;
        v->y = clus.waterHeight + w;
        v->z = sv->z;
      }
    }

/*
    if (*Vid::Var::Terrain::shroud && clus.shroudCount == 25)
    {
      RenderShroudWaterVtl( clus, verts, clipFlags);
      return;
    }
*/
    Color c = *Vid::Var::Terrain::waterColorBottom;

    if (clipFlags == clipNONE)
    {
#ifdef DOTERRAINCOLOR
      Vid::LightProjectVerts( tempmem, verts, waterNorms, colors, vcount);
#else
      Vid::LightProjectVerts( tempmem, verts, waterNorms, vcount);
#endif

      if (*Vid::Var::Terrain::shroud)
      {
        VertexTL * dv = tempmem, * de = tempmem + vcount;
        for ( ; dv < de; dv++, fogs++)
        {
//          dv->diffuse.Modulate( c);
          dv->diffuse.ModulateInline( *fogs, *fogs, *fogs, F32(c.a) * U8toNormF32 );
        }
      }
      else
      {
        VertexTL * dv = tempmem, * de = tempmem + vcount;
        for ( ; dv < de; dv++)
        {
          dv->diffuse.a = c.a;
        }
      }
      VertexTL *vertmem;
      U16 *indexmem;

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 25, &indexmem, 96, &clus))
      {
        return;
      }

      VertexTL * dv = vertmem, * sv = tempmem;
      UVPair * uvlist, * uve = waterUVList0 + vcount;
      for (uvlist = waterUVList0; uvlist < uve; dv++, sv++, uvlist++)
      {
        dv->vv  = sv->vv;
        dv->rhw = sv->rhw;

        dv->diffuse  = sv->diffuse;
        dv->specular = 0xff000000;
        dv->uv = *uvlist;

        dv->SetFog();
      }
      Utils::Memcpy( indexmem, clusterI, 96 << 1);

      Vid::UnlockIndexedPrimitiveMem( vcount, 96);

      if (waterLayer2)
      {
        // 2nd layer
        //
        VertexTL *vertmem1;
        Vid::SetTranBucketZMax( Vid::sortWATER0);

        if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem1, 25, &indexmem, 96, &clus))
        {
          return;
        }
        c = *Vid::Var::Terrain::waterColorTop;

        VertexTL * dv = vertmem1, * sv = vertmem;
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
    else
    {
#ifdef DOTERRAINCOLOR
      Vid::LightTransformFromModel( tempmem, verts, waterNorms, colors, vcount);
#else
      Vid::LightTransformFromModel( tempmem, verts, waterNorms, vcount);
#endif

      VertexTL * dv = tempmem;
      UVPair * uvlist, * uve = waterUVList0 + vcount;
      for (uvlist = waterUVList0; uvlist < uve; dv++, uvlist++, fogs++)
      {
        if (*Vid::Var::Terrain::shroud)
        {
          dv->diffuse.ModulateInline( *fogs, *fogs, *fogs, F32(c.a) * U8toNormF32 );
        }
        else
        {
          dv->diffuse.a = c.a;
        }
        dv->uv = *uvlist;
      }
      Bucket * buck = Vid::Clip::ToBucket( tempmem, vcount, clusterI, 96, &clus, TRUE, clipFlags);

      if (buck)
      {
        if (waterLayer2)
        {
          Vid::SetTranBucketZMax( Vid::sortWATER0);

          // 2nd layer
          //
          c = *Vid::Var::Terrain::waterColorTop;

          VertexTL * dv = tempmem;
          Vector * sv = verts;
          UVPair * uvlist, * uve = waterUVList1 + vcount;
          for (uvlist = waterUVList1; uvlist < uve; uvlist++, dv++, sv++)
          {
            Vid::TransformFromModel( *dv, *sv);
            dv->uv = *uvlist;
            dv->diffuse.a  = c.a;
          }
          Vid::Clip::ToBucket( tempmem, vcount, clusterI, 96, &clus, TRUE, clipFlags);
        }
      }
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterVtl( Cluster & clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, U32 clipFlags) // = clipALL
  {
    // build list of vertex positions, normals, and colors
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    S32 meterStrideX    = abs( cellStrideX) * heightField.meterPerCell;
    S32 meterStrideZ    = abs( cellStrideZ) * heightField.meterPerCell;
    S32 cellStrideWidth = cellStrideZ * heightField.cellPitch;

/*
    if (Vid::Clip::BoundsTestAlphaFar( clus.zDepth, clus.bounds.Radius()))
    {
      BucketMan::forceTranslucent = TRUE;
    }
*/
    Cell *c0 = &heightField.cellList[cellOffset];

    Vector verts[25], * dv = verts;
    Vector norms[25], * dn = norms;
    Color colors[25], * dc = colors;
    F32     fogs[25], * df = fogs;
    for (z0 = z; z0 <= zend; z0 += meterStrideZ, c0 += cellStrideWidth)
    {
      Cell *c = c0;
      for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, dn++, dc++, df++, c += cellStrideX)
      {
        if (*Vid::Var::Terrain::shroud)
        {
          *df = (F32) c->GetFog() * U8toNormF32;
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
    VertexTL tempmem[25];
    U32 vcount = dv - verts;

//    Bool softS    = shroud && *softShroud;

    if (clus.status.water && Vid::Var::Terrain::water)
    {
      if (!Vid::renderState.status.mirror || !Terrain::waterList.count || Terrain::waterColorMirror.a > 253)
      {
        RenderCellWaterVtl( tempmem, clus, x, z, verts, colors, fogs, vcount, clipFlags);
      }
    }

    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags);
    Vid::SetTranBucketZMax( Vid::sortTERRAIN0 + 1);

    if (clipFlags == clipNONE)
    {
#ifdef DOTERRAINCOLOR
      Vid::LightProjectVerts( tempmem, verts, norms, colors, vcount);
#else
      Vid::LightProjectVerts( tempmem, verts, norms, vcount);
#endif
      if (*Vid::Var::Terrain::shroud)
      {
        VertexTL * dvv = tempmem + vcount;
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
          UVPair * uvList = &cellUVList[c->uv];
          VertexTL * dv = vertmem;
          for (U32 i = 0; i < 4; i++, dv++)
          {
            const VertexTL &sv = tempmem[iv[i]];

            dv->vv  = sv.vv;
            dv->rhw = sv.rhw;
            dv->diffuse  = sv.diffuse;
            dv->specular = sv.specular;
            dv->uv  = uvList[i];
          }
          Utils::Memcpy( indexmem, Vid::rectIndices, 12);

          Vid::UnlockIndexedPrimitiveMem( 4, 6);

          if (*Vid::Var::Terrain::overlay && (c->flags & Cell::cellOVERLAY))
          {
            ASSERT( c->texture1 < overlayCount && c->overlay < overlays.count);

            Overlay & overlay =  overlays[c->overlay];
            Bitmap * tex = overlayList[c->texture1];
            Bool bright = (overlay.blend & RS_TEX_MASK) == RS_TEX_DECAL ? TRUE : FALSE;

            Vid::SetTranBucketZMax( Vid::sortTERRAIN0);
            Vid::SetBucketTexture( tex, TRUE, 0, RS_BLEND_MODULATE | renderFlags);

            if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
            {
              LOG_WARN( ("Terrain::RenderCluster: can't lock overlay bucket!") );
              return;
            }

#ifdef DOSTATISTICS
            Statistics::noClipTris += 2;
#endif

            VertexTL * dv = vertmem;
            for (U32 i = 0; i < 4; i++, dv++)
            {
              U32 index = iv[i];
              const VertexTL & sv = tempmem[index];

              dv->vv  = sv.vv;
              dv->rhw = sv.rhw;

              if (bright)
              {
                dv->diffuse = 0xffffffff;

                if (*Vid::Var::Terrain::shroud)
                {
                  dv->diffuse.Modulate( fogs[index], fogs[index], fogs[index]);
                }
              }
              else
              {
                dv->diffuse = sv.diffuse;
              }
              dv->specular = sv.specular;
            }
            UVPair  * uvList  = &overlay.uvs[c->uv1];
            vertmem[0].uv = uvList[0];
            vertmem[1].uv = uvList[overlay.size.x+1];
            vertmem[2].uv = uvList[overlay.size.x+2];
            vertmem[3].uv = uvList[1];

            Utils::Memcpy( indexmem, Vid::rectIndices, 12);

            Vid::UnlockIndexedPrimitiveMem( 4, 6);
          }
/*
          if (softS)
          {
            Bool hit = 0;
            for (U32 i = 0; i < 4; i++)
            {
              if (fogs[iv[i]] < 1)
              {
                hit++;
              }
            }
            F32 df0 = fogs[iv[0]];
            F32 df1 = fogs[iv[1]];
            F32 df2 = fogs[iv[2]];
            F32 df3 = fogs[iv[3]];
            F32 ddf0 = (F32) fabs( df0 - df2);
            F32 ddf1 = (F32) fabs( df1 - df3);

            if ((ddf0 != 0 || ddf1 != 0) && ddf0 != ddf1 && hit > 1)
            {
              Vid::SetBucketTexture( shroudTex, TRUE, 0, RS_BLEND_DEF | renderFlags);

              if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
              {
                LOG_WARN( ("Terrain::RenderCluster: can't lock shroud overlay bucket!") );
                return;
              }
              U32 uvi;
              Color spec;
              if (ddf0 > ddf1)
              {
                spec = tempmem[iv[ df1 < df3 ? 3 : 1]].specular;
                uvi = df0 < df2 ? 1 : 3;
              }
              else 
              {
                spec = tempmem[iv[ df0 < df2 ? 2 : 0]].specular;
                uvi = df1 < df3 ? 2 : 0;
              }

              UVPair * uvList = &cellUVList[uvi];
              VertexTL * dv = vertmem;
              for (U32 i = 0; i < 4; i++, dv++)
              {
                const VertexTL & sv = tempmem[iv[i]];

                dv->vv  = sv.vv;
                dv->rhw = sv.rhw;
                dv->diffuse  = sv.diffuse;
                dv->specular = spec;
                dv->uv  = uvList[i];
              }
              Utils::Memcpy( indexmem, Vid::rectIndices, 12);

              Vid::UnlockIndexedPrimitiveMem( 4, 6);
            }
          }
*/
        }
      }
    }
    else
    {
#ifdef DOTERRAINCOLOR
      Vid::LightTransformFromModel( tempmem, verts, norms, colors, vcount);
#else
      Vid::LightTransformFromModel( tempmem, verts, norms, vcount);
#endif
      if (*Vid::Var::Terrain::shroud)
      {
        VertexTL *dvv = tempmem + vcount;

        {
          for (dvv--, df--; dvv >= tempmem; dvv--, df--)
          {
            dvv->diffuse.ModulateInline( *df, *df, *df);
          }
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
          U16 iv[4];

          iv[0] = (U16)(vcount);
          iv[1] = (U16)(vcount + 5);
          iv[2] = (U16)(vcount + 6);
          iv[3] = (U16)(vcount + 1);

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[3]]);

          // backcull: FIXME cull each tri separately
	  	    if (planes[0].Evalue(Vid::Math::modelViewVector) <= 0.0f
	  	     && planes[1].Evalue(Vid::Math::modelViewVector) <= 0.0f)
          {
            continue;
          }
          Bitmap *tex = texList[c->texture];
          Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);

          VertexTL vertmem[4];
          UVPair *uvList = &cellUVList[c->uv];

          VertexTL * dv = vertmem;
          for (U32 i = 0; i < 4; i++, dv++)
          {
            VertexTL & sv = tempmem[iv[i]];
      
            dv->vv  = sv.vv;
            dv->rhw = sv.rhw;
            dv->diffuse = sv.diffuse;
            dv->specular = sv.specular;

            dv->uv  = uvList[i];
          }

          {
            Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, TRUE, clipFlags);
          }

          if (*Vid::Var::Terrain::overlay && (c->flags & Cell::cellOVERLAY))
          {
            ASSERT( c->texture1 < overlayCount && c->overlay < overlays.count);

            Overlay & overlay =  overlays[c->overlay];
            Bitmap * tex = overlayList[c->texture1];
            Bool bright = (overlay.blend & RS_TEX_MASK) == RS_TEX_DECAL ? TRUE : FALSE;

            Vid::SetTranBucketZMax( Vid::sortTERRAIN0);
            Vid::SetBucketTexture( tex, TRUE, 0, RS_BLEND_MODULATE | renderFlags);

            UVPair  * uvList  = &overlay.uvs[c->uv1];
            VertexTL * dv = vertmem;
            for (U32 i = 0; i < 4; i++, dv++)
            {
              U32 index = iv[i];

              // ClipToBucket calls SetHomogeneous
              VertexTL & sv = tempmem[index];
              dv->vv  = sv.vv;
              dv->rhw = sv.rhw;

              if (bright)
              {
                dv->diffuse = 0xffffffff;

                if (*Vid::Var::Terrain::shroud)
                {
                  dv->diffuse.Modulate( fogs[index], fogs[index], fogs[index]);
                }
              }
            }
            vertmem[0].uv = uvList[0];
            vertmem[1].uv = uvList[overlay.size.x+1];
            vertmem[2].uv = uvList[overlay.size.x+2];
            vertmem[3].uv = uvList[1];

            {
              Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, 1, clipFlags);
            }
          }
/*
          if (softS)
          {
            Bool hit = 0;
            for (U32 i = 0; i < 4; i++)
            {
              if (fogs[iv[i]] < 1)
              {
                hit++;
              }
            }
            F32 df0 = fogs[iv[0]];
            F32 df1 = fogs[iv[1]];
            F32 df2 = fogs[iv[2]];
            F32 df3 = fogs[iv[3]];
            F32 ddf0 = (F32) fabs( df0 - df2);
            F32 ddf1 = (F32) fabs( df1 - df3);

            if ((ddf0 != 0 || ddf1 != 0) && ddf0 != ddf1 && hit > 1)
            {
              Vid::SetBucketTexture( shroudTex, TRUE, 0, RS_BLEND_DEF | renderFlags);

              U32 uvi;
              Color spec;
              if (ddf0 > ddf1)
              {
                spec = tempmem[iv[ df1 < df3 ? 3 : 1]].specular;
                uvi = df0 < df2 ? 1 : 3;
              }
              else 
              {
                spec = tempmem[iv[ df0 < df2 ? 2 : 0]].specular;
                uvi = df1 < df3 ? 2 : 0;
              }

              UVPair * uvList = &cellUVList[uvi];
              VertexTL * dv = vertmem;
              for (U32 i = 0; i < 4; i++, dv++)
              {
                const VertexTL & sv = tempmem[iv[i]];

                dv->vv  = sv.vv;
                dv->rhw = sv.rhw;
                dv->diffuse  = sv.diffuse;
                dv->specular = spec;
                dv->uv  = uvList[i];
              }

              {
                Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, 1, clipFlags);
              }
            }
          }
*/
        }
      }
    }
    BucketMan::forceTranslucent = FALSE;
  }
  //----------------------------------------------------------------------------
  F32 offMapHeight = 10.0f;

#if 0
  void RenderClusterOffMapVtl( S32 x, S32 z)
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

    F32 xy[5], zy[5], dx, dxx, dz = 0, dzz = 0;

    S32 zcc = zc;
    S32 xcc = xc;

    S32 corners = 0;

    if (xc < 0)
    {
      for (U32 i = 0; i < 5; i++, zcc++)
      {
        S32 tzc = zcc < 0 ? 0 : zcc > (S32) heightField.cellHeight ? heightField.cellHeight : zcc;

        xy[i] = heightField.cellList[ tzc * heightField.cellPitch].height;
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
      }

      dz = F32(zc - heightField.cellHeight) * (F32) heightField.meterPerCell;
      dzz = (F32) heightField.meterPerCell;

      corners |= 2;
    }

    F32 dxy[5], dzy[5], ddx = dx, ddz = dz, ymin = F32_MAX, ymax = -F32_MAX;
    for (U32 i = 0; i < 5; i++, ddx += dxx, ddz += dzz)
    {
      dxy[i] = (offMapHeight - xy[i]) / 1000;
      dzy[i] = (offMapHeight - zy[i]) / 1000;

      F32 y = zy[i] + dzy[i] * ddz;
      
      ymin = Min<F32>( ymin, y);
      ymax = Max<F32>( ymax, y);

      y = xy[i] + dxy[i] * ddx;
      
      ymin = Min<F32>( ymin, y);
      ymax = Max<F32>( ymax, y);
    }
    Vector offset( xs + F32( meterPerClus) * .5f, (ymin + ymax) * .5f, zs + F32( meterPerClus) * .5f); 
    F32 yy = ymax - ymin;
    Bounds bounds( offset, Max<F32>( F32( meterPerClus), yy));
    bounds.Set( F32( meterPerClus), yy * .5f, F32( meterPerClus)); 

    bounds.RenderBox( Matrix::I, 0x88ff0000);

    U32 clipFlags = Vid::CurCamera().BoundsTestBox( offset, bounds);
    if (clipFlags == clipOUTSIDE)
    {
      // cluster is completely outside the view frustrum
      return;
    }
    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags);
    Vid::SetTranBucketZMax( Vid::sortTERRAIN0 + 1);
    Vid::SetBucketTexture( NULL, FALSE, 0, RS_BLEND_DEF);

    if (clipFlags == clipNONE)
    {
      VertexTL * vertmem;
      U16 *      indexmem;
      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 25, &indexmem, 96))
      {
        LOG_WARN( ("Terrain::RenderCluster: can't lock bucket!") );
        return;
      }
      VertexTL * v = vertmem;

      U32 iz = 0;
      for ( zs; zs <= zend; zs += heightField.meterPerCell, iz++, dz += dzz, zc++)
      {
        U32 ix = 0;
        F32 ddx = dx;
        S32 xcc = xc;
        for (F32 x = xs; x <= xend; x += heightField.meterPerCell, ix++, v++, ddx += dxx, xcc++)
        {
          v->vv.x = x;
          v->vv.z = zs;
          v->diffuse  = 0xff000000;
          v->specular = 0xff000000;

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
/*
        if (xcc != 0 && xcc != heightField.cellWidth && zc != 0 && zc != heightField.cellHeight)
        {
          S32 xccc = xcc % S32( cellPerClus);
          if (xccc < 0)
          {
            xccc += cellPerClus;
          }
          S32 zccc = zc % S32( cellPerClus);
          if (zccc < 0)
          {
            zccc += cellPerClus;
          }
          v->vv.y += randomField.cellList[zccc * randomField.cellPitch + xccc].height;
        }
*/
      }
      Vid::ProjectVerts( vertmem, 25);

      Utils::Memcpy( indexmem, clusterI, 96 << 1);

      Vid::UnlockIndexedPrimitiveMem( 25, 96);

#ifdef DOSTATISTICS
      Statistics::noClipTris += 32;
#endif
    }
    else
    {
      VertexTL vertmem[25], * v = vertmem;
      U32 iz = 0;
      for ( zs; zs <= zend; zs += heightField.meterPerCell, iz++, dz += dzz, zc++)
      {
        U32 ix = 0;
        F32 ddx = dx;
        S32 xcc = xc;
        for (F32 x = xs; x <= xend; x += heightField.meterPerCell, ix++, v++, ddx += dxx, xcc++)
        {
          v->vv.x = x;
          v->vv.z = zs;
          v->diffuse  = 0xff000000;
          v->specular = 0xff000000;

          switch (corners)
          {
//            v->vv.y = zy[ix] + dzy[ix] * dz;
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
/*
        if (xcc != 0 && xcc != heightField.cellWidth && zc != 0 && zc != heightField.cellHeight)
        {
          S32 xccc = xcc % S32( cellPerClus);
          if (xccc < 0)
          {
            xccc += cellPerClus;
          }
          S32 zccc = zc % S32( cellPerClus);
          if (zccc < 0)
          {
            zccc += cellPerClus;
          }
          v->vv.y += randomField.cellList[zccc * randomField.cellPitch + xccc].height;
        }
*/
      }
      Vid::TransformFromModel( vertmem, 25);

      Vid::Clip::ToBucket( vertmem, 25, clusterI, 96, NULL, TRUE, clipFlags);
    }
  }
  //----------------------------------------------------------------------------


#else

//	dx = 1.0f / (dx12 * dz02 - dx02 * dz12);
//  DyDx = (dy12 * dz02 - dy02 * dz12) *  dx;
//	DyDz = (dy12 * dx02 - dy02 * dx12) * -dx;

  void RenderClusterOffMapVtl( S32 x, S32 z)
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
      dx = (F32) -xc * (F32) heightField.meterPerCell;
      dxx =  - (F32) heightField.meterPerCell;

      for (U32 i = 0; i < 5; i++, zcc++)
      {
        S32 tzc = zcc < 0 ? 0 : zcc > (S32) heightField.cellHeight ? heightField.cellHeight : zcc;

        xy[i] = heightField.cellList[ tzc * heightField.cellPitch].height;
      }
      corners |= 1;
    }
    else if (xc >= (S32) heightField.cellWidth)
    {
      dx = (F32(xc) - F32(heightField.cellWidth)) * (F32) heightField.meterPerCell;
      dxx = (F32) heightField.meterPerCell;

      for (U32 i = 0; i < 5; i++, zcc++)
      {
        S32 tzc = zcc < 0 ? 0 : zcc > (S32) heightField.cellHeight ? heightField.cellHeight : zcc;

        xy[i] = heightField.cellList[ tzc * heightField.cellPitch + heightField.cellWidth].height;
      }
      corners |= 1;
    }
    else
    {
      for (U32 i = 0; i < 5; i++, zcc++)
      {
        S32 tzc = zcc < 0 ? 0 : zcc > (S32) heightField.cellHeight ? heightField.cellHeight : zcc;

        xy[i] = heightField.cellList[ tzc * heightField.cellPitch + xc].height;
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
      }

      dz = F32(zc - heightField.cellHeight) * (F32) heightField.meterPerCell;
      dzz = (F32) heightField.meterPerCell;

      corners |= 2;
    }

    F32 dxy[5], dzy[5];
    for (U32 i = 0; i < 5; i++)
    {
      dzy[i] = (offMapHeight - zy[i]) / 1000;
      dxy[i] = (offMapHeight - xy[i]) / 1000;
    }

    VertexTL points[25], * v = points;
    U32 iz = 0;
    for (F32 fz = zs; fz <= zend; fz += heightField.meterPerCell, iz++, dz += dzz, zc++)
    {
      U32 ix = 0;
      F32 ddx = dx;
      S32 xcc = xc;
      for (F32 fx = xs; fx <= xend; fx += heightField.meterPerCell, ix++, v++, ddx += dxx, xcc++)
      {
        v->vv.x = fx;
        v->vv.z = fz;
        v->diffuse  = 0xff000000;
        v->specular = 0xff000000;

        switch (corners)
        {
        case 3:
        {
          v->vv.y = xy[iz] + dxy[iz] * ddx;

          F32 zz = fz < 0 ? -fz : fz >= (S32) heightField.meterHeight ? fz - heightField.meterHeight : fz;
          F32 xx = fx < 0 ? -fx : fx >= (S32) heightField.meterWidth  ? fx - heightField.meterWidth  : fx;

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
        ymin = Min<F32>( ymin, v->vv.y);
        ymax = Max<F32>( ymax, v->vv.y);
      }
    }

    // setup bounds
    Vector offset( (xend + xs) * .5f, (ymin + ymax) * .5f, (zend + zs) * .5f); 
    F32 yy = ymax - ymin;
    Bounds bounds( offset, Max<F32>( F32(meterPerClus), yy) * 1.8f);
    bounds.Set( F32(meterPerClus), yy * .5f, F32(meterPerClus)); 

//    bounds.RenderBox( Matrix::I, 0x88ff0000);

    U32 clipFlags = Vid::CurCamera().BoundsTestOrigin( bounds.Offset(), bounds);
    if (clipFlags == clipOUTSIDE)
    {
      // cluster is completely outside the view frustrum
      return;
    }
    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | renderFlags);
    Vid::SetTranBucketZMax( Vid::sortTERRAIN0 + 1);
    Vid::SetBucketTexture( NULL, FALSE, 0, RS_BLEND_DEF);

    if (clipFlags == clipNONE)
    {
      VertexTL * vertmem;
      U16 *      indexmem;
      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 25, &indexmem, 96))
      {
        LOG_WARN( ("Terrain::RenderCluster: can't lock bucket!") );
        return;
      }
      VertexTL * v = vertmem, * ve = v + 25, * src = points;
      for ( ; v < ve; v++, src++)
      {
        *v = *src;
      }
      Vid::ProjectVerts( vertmem, 25);

      Utils::Memcpy( indexmem, clusterI, 96 << 1);

      Vid::UnlockIndexedPrimitiveMem( 25, 96);

#ifdef DOSTATISTICS
      Statistics::noClipTris += 32;
#endif
    }
    else
    {
      Vid::TransformFromModel( points, 25);

      Vid::Clip::ToBucket( points, 25, clusterI, 96, NULL, TRUE, clipFlags);
    }
  }
  //----------------------------------------------------------------------------
#endif

  // draw the terrain
  //
  void RenderVtl()
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
            RenderClusterOffMapVtl( x, z);
          }
          continue;
        }

        Cluster & clus = clusList[clusOffz * clusWidth + clusOffx];
        Bounds  bounds = clus.bounds;

        if (clus.status.water)
        {
          bounds.Set( bounds.Width(), bounds.Height() + *Vid::Var::Terrain::waveHeight, bounds.Breadth());
        }

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
//            if (!*Vid::Var::Terrain::shroud || clus.shroudCount != 25)
            {
              lowWaterHeight += clus.waterHeight;
              lowWaterCount++;
            }
          }
        }
/*
        if (Vid::renderState.status.mirror)
        {
          return;
        }
*/
        Vid::Light::SetActiveList( bounds.Offset(), bounds);
        Vid::Light::SetupLightsModel();

        RenderClusterVtl( clus, xs, zs, offset, 1, 1, clipFlags);

/*
        if (Vid::renderState.status.showBounds && clipFlags != clipNONE)
        {
          bounds.RenderBox( Matrix::I, 0xaaff0000);
        }
*/
      }
    }

  #ifdef DOSTATISTICS
    Statistics::terrainTris = Statistics::tempTris;
  #endif
  }
  //----------------------------------------------------------------------------

  // skip backface culling
  //
  void RenderShroudMirrorMaskVtl( Cluster & clus, Vector * verts0, U32 clipFlags)
  {
    clus;

    Vid::SetTexture( NULL, 0, RS_BLEND_MODULATE);

    VertexTL vertmem[4];

    vertmem[0].vv = verts0[0];
    vertmem[0].diffuse  = 0xff000000;
    vertmem[0].specular = 0xff000000;
    vertmem[1].vv = verts0[20];
    vertmem[1].diffuse  = 0xff000000;
    vertmem[1].specular = 0xff000000;
    vertmem[2].vv = verts0[24];
    vertmem[2].diffuse  = 0xff000000;
    vertmem[2].specular = 0xff000000;
    vertmem[3].vv = verts0[4];
    vertmem[3].diffuse  = 0xff000000;
    vertmem[3].specular = 0xff000000;

    if (clipFlags == clipNONE)
    {
      Vid::ProjectVerts( vertmem, 4, TRUE);

#ifdef DOSTATISTICS
      Statistics::noClipTris += 2;
#endif

      Vid::DrawIndexedPrimitive( 
        PT_TRIANGLELIST,
        FVF_TLVERTEX,
        vertmem, 4, Vid::rectIndices, 6, 
        DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);
    }
    else
    {
      Vid::TransformFromModel( vertmem, 4);

      VertexTL * vtx;
      U16 * idx;
      U32 heapSize = Vid::Heap::ReqVertex( &vtx, &idx);

      U32 vcount = 4, icount = 6;
      Vid::Clip::ToBuffer( vtx, idx, vertmem, vcount, Vid::rectIndices, icount, TRUE, clipFlags);

      if (icount)
      {
        Vid::DrawIndexedPrimitive( 
          PT_TRIANGLELIST,
          FVF_TLVERTEX,
          vtx, vcount, idx, icount, 
          DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);
      }

      Vid::Heap::Restore( heapSize);
    }
  }
  //----------------------------------------------------------------------------

  void RenderCellMirrorMaskVtl( Cluster & clus, U32 cellOffset, S32 x, S32 z, U32 clipFlags, Bool doNearZ)
  {
    clipFlags &= ~clipPLANE0;

    Vid::SetTexture( waterTex, 0, RS_BLEND_MODULATE);

    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    S32 meterStrideX    = heightField.meterPerCell;
    S32 meterStrideZ    = heightField.meterPerCell;
    S32 cellStrideWidth = heightField.cellPitch;

    Cell * c0 = &heightField.cellList[cellOffset];

    Vector verts[25], * dv = verts;
    F32     fogs[25], * df = fogs;
    for (z0 = z; z0 <= zend; z0 += meterStrideZ, c0 += cellStrideWidth)
    {
      Cell *c = c0;
      for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, df++, c += 1)
      {
  
        if (*Vid::Var::Terrain::shroud)
        {
          *df = (F32) c->GetFog() * U8toNormF32;
        }

        dv->x = (F32) x0;
        dv->z = (F32) z0;
        dv->y = clus.waterHeight;
      }
    }

/*
    if (*Vid::Var::Terrain::shroud && clus.shroudCount == 25)
    {
      RenderShroudMirrorMaskVtl( clus, verts, clipFlags);
      return;
    }
*/

    Vid::SetTexture( waterTex, 0, RS_BLEND_MODULATE);

    VertexTL tempmem[25];
    U32 vcount = 25;

    U8 alpha = U8(doNearZ ? 0 : waterColorMirror.a);

    if (clipFlags == clipNONE)
    {
      Vid::LightProjectVerts( tempmem, verts, waterNorms, vcount);

      VertexTL * dv = tempmem;
      UVPair * uvlist, * uve = waterUVList0 + vcount;
      df = fogs;
      for (uvlist = waterUVList0; uvlist < uve; dv++, uvlist++, df++)
      {
        if (*Vid::Var::Terrain::shroud)
        {
          F32 a = *df == 0 ? 1 : F32(alpha) * U8toNormF32;
          dv->diffuse.ModulateInline( *df, *df, *df,  a);
        }
        else
        {
          dv->diffuse.a = alpha;
        }
        dv->uv = *uvlist;
        dv->SetFog();

        if (doNearZ)
        {
          dv->vv.z = 0;
          dv->rhw  = 1;
        }
      }

      Vid::DrawIndexedPrimitive( 
        PT_TRIANGLELIST,
        FVF_TLVERTEX,
        tempmem, 25, clusterI, 96, 
        DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);

  #ifdef DOSTATISTICS
      Statistics::tempTris += 32;
  #endif

      if (waterLayer2)
      {
        // 2nd layer
        //
//        color = *Vid::Var::Terrain::waterColorTop;

        VertexTL * dv = tempmem;
        UVPair * uvlist, * uve = waterUVList1 + vcount;
        for (uvlist = waterUVList1; uvlist < uve; dv++, uvlist++)
        {
          dv->uv = *uvlist;
          dv->diffuse.a  = waterColorMirror.a;
        }

        Vid::DrawIndexedPrimitive( 
          PT_TRIANGLELIST,
          FVF_TLVERTEX,
          tempmem, 25, clusterI, 96, 
          DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);
      }
    }
    else
    {
      Vid::LightTransformFromModel( tempmem, verts, waterNorms, vcount);

      VertexTL * dv = tempmem;
      UVPair * uvlist, * uve = waterUVList0 + vcount;
      df = fogs;
      for (uvlist = waterUVList0; uvlist < uve; dv++, uvlist++, df++)
      {
        if (*Vid::Var::Terrain::shroud)
        {
          F32 a = *df == 0 ? 1 : F32(alpha) * U8toNormF32;
          dv->diffuse.ModulateInline( *df, *df, *df, a );
        }
        else
        {
          dv->diffuse.a = alpha;
        }
        dv->uv = *uvlist;
      }

      VertexTL * vtx;
      U16 * idx;
      U32 icount = 96;
      U32 heapSize = Vid::Heap::ReqVertex( &vtx, &idx);

      Vid::Clip::ToBuffer( vtx, idx, tempmem, vcount, clusterI, icount, TRUE, clipFlags);

      if (doNearZ)
      {
        VertexTL * ev = vtx + vcount;
        for (dv = vtx; dv < ev; dv++)
        {
          dv->vv.z = 0;
          dv->rhw  = 1;
        }
      }

      if (icount)
      {
        Vid::DrawIndexedPrimitive( 
          PT_TRIANGLELIST,
          FVF_TLVERTEX,
          vtx, vcount, idx, icount, 
          DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_MODULATE);
      }
      Vid::Heap::Restore( heapSize);
/*
      if (buck)
      {
        if (waterLayer2)
        {
          // 2nd layer
          //
          color = *Vid::Var::Terrain::waterColorTop;

          UVPair * uvlist, * uve = waterUVList1 + vcount;
          for (uvlist = waterUVList1; uvlist < uve; uvlist++, dv++, sv++)
          {
            Vid::TransformFromModel( *dv, *sv);
            dv->uv = *uvlist;
            dv->diffuse.a  = c.a;
          }
          Vid::Clip::ToBucket( tempmem, vcount, clusterI, 96, &clus, TRUE, clipFlags);
        }
*/
    }
  }
  //----------------------------------------------------------------------------

  // skip backface culling
  //
  void RenderClusterMirrorMaskVtl( Cluster &clus, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, U32 clipFlags)
  {
    clus;

    clipFlags &= ~clipPLANE0;

    Vid::SetTexture( waterTex, 0, RS_BLEND_MODULATE);

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

      Vid::LightProjectVerts( vertmem, waterNorms, 4);

      vertmem[0].uv = waterUVList0[0];
      vertmem[0].diffuse.a  = waterColorMirror.a;

      vertmem[1].uv = waterUVList0[20];
      vertmem[1].diffuse.a  = waterColorMirror.a;

      vertmem[2].uv = waterUVList0[24];
      vertmem[2].diffuse.a  = waterColorMirror.a;

      vertmem[3].uv = waterUVList0[4];
      vertmem[3].diffuse.a  = waterColorMirror.a;

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

        vertmem[0].uv = waterUVList1[0];
        vertmem[0].diffuse.a  = (U8)a;

        vertmem[1].uv = waterUVList1[20];
        vertmem[1].diffuse.a  = (U8)a;

        vertmem[2].uv = waterUVList1[24];
        vertmem[2].diffuse.a  = (U8)a;

        vertmem[3].uv = waterUVList1[4];
        vertmem[3].diffuse.a  = (U8)a;

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

      Vid::LightTransformFromModel( temp, waterNorms, 4);

      temp[0].uv = waterUVList0[0];
      temp[0].diffuse.a  = waterColorMirror.a;

      temp[1].uv = waterUVList0[20];
      temp[1].diffuse.a  = waterColorMirror.a;

      temp[2].uv = waterUVList0[24];
      temp[2].diffuse.a  = waterColorMirror.a;

      temp[3].uv = waterUVList0[4];
      temp[3].diffuse.a  = waterColorMirror.a;

      for (U32 i = 0; i < 4; i++)
      {
        verts[i] = temp[i];
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

          temp[0].uv = waterUVList1[0];
          temp[0].diffuse.a  = U8(a);

          temp[1].uv = waterUVList1[20];
          temp[1].diffuse.a  = U8(a);

          temp[2].uv = waterUVList1[24];
          temp[2].diffuse.a  = U8(a);

          temp[3].uv = waterUVList1[4];
          temp[3].diffuse.a  = U8(a);

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
  void RenderMirrorMaskVtl( WaterRegion * waterR)
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

    if (waterR)
    {
      rect.Clip( Area<S32>( (S32) waterR->rect.p0.x, (S32) waterR->rect.p0.y, (S32) waterR->rect.p1.x, (S32) waterR->rect.p1.y));
    }

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

	      if (!clus.status.water || (waterR && clus.waterHeight != waterR->height))
        {
          // no water or fully shrouded
          continue;
        }

        Bounds bounds = clus.bounds;
        Vector viewOrigin, off = bounds.Offset();
        off.y = clus.waterHeight;
        bounds.Set( bounds.Width(), 0, bounds.Breadth());    // fixme 
        U32 clipFlags = Vid::CurCamera().BoundsTestOrigin( off, bounds, &viewOrigin);
        if (clipFlags == clipOUTSIDE)
        {
          // cluster is completely outside the view frustrum
          continue;
        }
        clus.zDepth = viewOrigin.z;

        Vid::Light::SetActiveList( clus.bounds.Offset(), clus.bounds);
        Vid::Light::SetupLightsModel();

//        U32 offset = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;
        S32 xs = x - Utils::FtoL(OffsetX());    // FIXME
        S32 zs = z - Utils::FtoL(OffsetZ());

        U32 offset = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;

        RenderCellMirrorMaskVtl( clus, offset, xs, zs, clipFlags, waterR ? TRUE : FALSE);
//        RenderCellMirrorMaskVtl( clus, offset, xs, zs, clipFlags, FALSE);

//        if (!*Vid::Var::Terrain::shroud || clus.shroudCount != 25)
//        if (!*Vid::Var::Terrain::shroud)
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
