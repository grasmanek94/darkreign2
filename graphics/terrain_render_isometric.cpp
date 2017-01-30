///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// terrain.cpp     DR2 terrain system
//
// 04-MAY-1998
//

#include "vid_private.h"
#include "random.h"
#include "console.h"
#include "statistics.h"
#include "terrain_priv.h"
//----------------------------------------------------------------------------

namespace Terrain
{

  void RenderCellIsometric( Cluster &clus, Cell &c0, Cell &c1, Cell &c2, Cell &c3, S32 x0, S32 z0, S32 x1, S32 z1, U32 clipFlags, Bool doColor, Bool doOverlay) // = clipALL, FALSE
  {							
    clipFlags;

	  VertexTL * vertmem0;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem0, 4, &indexmem, 6, &clus))
    {
      LOG_DIAG( ("Terrain::RenderIso: can't lock bucket") );
      return;
    }
    UVPair *uvList = &cellUVList[c0.uv];

    vertmem0[0].vv.x = (F32) x0;
    vertmem0[0].vv.z = (F32) z0;
    vertmem0[0].vv.y = c0.height;
    vertmem0[0].uv = uvList[0];
    vertmem0[0].specular = 0xff000000;
    Vid::ProjectIsoFromWorld( vertmem0[0]);

    vertmem0[1].vv.x = (F32) x0;
    vertmem0[1].vv.z = (F32) z1;
    vertmem0[1].vv.y = c1.height;
    vertmem0[1].uv = uvList[1];
    vertmem0[1].specular = 0xff000000;
    Vid::ProjectIsoFromWorld( vertmem0[1]);

    vertmem0[2].vv.x = (F32) x1;
    vertmem0[2].vv.z = (F32) z1;
    vertmem0[2].vv.y = c2.height;
    vertmem0[2].uv = uvList[2];
    vertmem0[2].specular = 0xff000000;
    Vid::ProjectIsoFromWorld( vertmem0[2]);

    vertmem0[3].vv.x = (F32) x1;
    vertmem0[3].vv.z = (F32) z0;
    vertmem0[3].vv.y = c3.height;
    vertmem0[3].uv = uvList[3];
    vertmem0[3].specular = 0xff000000;
    Vid::ProjectIsoFromWorld( vertmem0[3]);


    if (doColor)
    {
      Color c = normLights[c0.normal];
      vertmem0[0].diffuse.Modulate( c0.color, 
        F32(c.r) * U8toNormF32,
        F32(c.g) * U8toNormF32,
        F32(c.b) * U8toNormF32);

      c = normLights[c1.normal];
      vertmem0[1].diffuse.Modulate( c1.color, 
        (F32)c.r * U8toNormF32,
        (F32)c.g * U8toNormF32,
        (F32)c.b * U8toNormF32);

      c = normLights[c2.normal];
      vertmem0[2].diffuse.Modulate( c2.color, 
        (F32)c.r * U8toNormF32,
        (F32)c.g * U8toNormF32,
        (F32)c.b * U8toNormF32);

      c = normLights[c3.normal];
      vertmem0[3].diffuse.Modulate( c3.color, 
        (F32)c.r * U8toNormF32,
        (F32)c.g * U8toNormF32,
        (F32)c.b * U8toNormF32);
    }
    else
    {
      vertmem0[0].diffuse = normLights[c0.normal];
      vertmem0[1].diffuse = normLights[c1.normal];
      vertmem0[2].diffuse = normLights[c2.normal];
      vertmem0[3].diffuse = normLights[c3.normal];
    }

    Utils::Memcpy( indexmem, Vid::rectIndices, 12);

    Vid::UnlockIndexedPrimitiveMem( 4, 6);

    if (doOverlay && (c0.flags & Cell::cellOVERLAY))
    {
      ASSERT( c0.texture1 < overlayCount && c0.overlay < overlays.count);

      Overlay & overlay =  overlays[c0.overlay];
      Bitmap * tex = overlayList[c0.texture1];
      Bool bright = (overlay.blend & RS_TEX_MASK) == RS_TEX_DECAL ? TRUE : FALSE;

      Vid::SetTranBucketZMax( Vid::sortTERRAIN0);
      Vid::SetBucketTexture( tex, TRUE, 0, RS_BLEND_MODULATE | renderFlags);

      VertexTL * vertmem;
      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
      {
        LOG_WARN( ("Terrain::RenderCluster: can't lock overlay bucket!") );
        return;
      }

      VertexTL * dv = vertmem, * de = dv + 4, * src = vertmem0;
      for ( ; dv < de; dv++)
      {
        dv->vv  = src->vv;
        dv->rhw = src->rhw;

        if (bright)
        {
          dv->diffuse = 0xffffffff;
        }
        else
        {
          dv->diffuse = src->diffuse;
        }
        dv->specular = src->specular;
      }
      UVPair  * uvList  = &overlay.uvs[c0.uv1];
      vertmem[0].uv = uvList[0];
      vertmem[1].uv = uvList[overlay.size.x+1];
      vertmem[2].uv = uvList[overlay.size.x+2];
      vertmem[3].uv = uvList[1];

      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);
    }
  }
  //----------------------------------------------------------------------------

  void RenderCellIsometric( Cluster &clus, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, UVPair *uvList, U32 clipFlags) // = clipALL
  {	
    clipFlags;

	  VertexTL *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6, &clus))
    {
      LOG_DIAG( ("Terrain::RenderIso: can't lock bucket") );
      return;
    }

    Color c = *Vid::Var::Terrain::waterColorBottom;

    vertmem[0].vv.x = (F32) x0;
    vertmem[0].vv.z = (F32) z0;
    vertmem[0].vv.y = y;
    vertmem[0].uv = uvList[0];
    vertmem[0].diffuse = normLights[0];
    vertmem[0].diffuse.a = c.a;
      vertmem[0].specular = 0xff000000;
    Vid::ProjectIsoFromWorld( vertmem[0]);

    vertmem[1].vv.x = (F32) x0;
    vertmem[1].vv.z = (F32) z1;
    vertmem[1].vv.y = y;
    vertmem[1].uv = uvList[20];
    vertmem[1].diffuse = normLights[0];
    vertmem[1].diffuse.a = c.a;
    vertmem[1].specular = 0xff000000;
    Vid::ProjectIsoFromWorld( vertmem[1]);

    vertmem[2].vv.x = (F32) x1;
    vertmem[2].vv.z = (F32) z1;
    vertmem[2].vv.y = y;
    vertmem[2].uv = uvList[24];
    vertmem[2].diffuse = normLights[0];
    vertmem[2].diffuse.a = c.a;
    vertmem[2].specular = 0xff000000;
    Vid::ProjectIsoFromWorld( vertmem[2]);

    vertmem[3].vv.x = (F32) x1;
    vertmem[3].vv.z = (F32) z0;
    vertmem[3].vv.y = y;
    vertmem[3].uv = uvList[4];
    vertmem[3].diffuse = normLights[0];
    vertmem[3].diffuse.a = c.a;
    vertmem[3].specular = 0xff000000;
    Vid::ProjectIsoFromWorld( vertmem[3]);

    Utils::Memcpy( indexmem, Vid::rectIndices, 12);

    Vid::UnlockIndexedPrimitiveMem( 4, 6);
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterIsometric( Cluster &clus, S32 x, S32 z, U32 cellOffset, U32 cellStride, U32 clipFlags, Bool doColor, Bool doOverlay) // = 1, = clipALL
  {
    clipFlags;
    Vid::Light::SetActiveList( clus.bounds.Offset(), clus.bounds);

    U32 meterStride     = cellStride * heightField.meterPerCell;
    U32 cellStrideWidth = cellStride * heightField.cellPitch;

    // fill the vertex memory
    S32 x0, x1, xend = x + meterPerClus;
    S32 z0, z1, zend = z + meterPerClus;
    U32 offset = cellOffset;
    for (z0 = z, z1 = z + meterStride; z0 < zend; z0 += meterStride, z1 += meterStride)
    {
      Cell *c0 = &heightField.cellList[offset];
      Cell *c1 = c0 + heightField.cellPitch;
      Cell *c2 = c1 + 1;
      Cell *c3 = c0 + 1;
      for (x0 = x, x1 = x + meterStride; x0 < xend; x0 += meterStride, x1 += meterStride)
      {
        Bitmap *tex = texList[c0->texture];
        ASSERT(tex)

        Vid::SetBucketTexture(tex, FALSE, 0, RS_BLEND_DEF | renderFlags);

        RenderCellIsometric( clus, *c0, *c1, *c2, *c3, x0, z0, x1, z1, clipFlags, doColor, doOverlay);

        c0 += cellStride;
        c1 += cellStride;
        c2 += cellStride;
        c3 += cellStride;
      }
      offset += cellStrideWidth;
    }

	  if (clus.status.water && Vid::Var::Terrain::water)
	  {
      // one quad covers the whole cluster with water
      Vid::SetTranBucketZMax( Vid::sortWATER0 + 1);
      Vid::SetBucketTexture( waterTex, waterIsAlpha, 0, RS_BLEND_DEF);

      RenderCellIsometric( clus, x, z, xend, zend, clus.waterHeight, waterUVList0, clipFlags);
	  }
  }
  //----------------------------------------------------------------------------

  void RenderIsometric( Bool doColor, Bool doOverlay) // = FALSE
  {
    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_BLEND_DEF);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetBucketMaterial( Vid::defMaterial);

    CalcLighting();

    // returns rect of meters that might be visible
	  Area<S32> rect;
    Vid::CurCamera().GetVisibleRect( rect);

    rect.p0.x += (S32) OffsetX();
    rect.p1.x += (S32) OffsetX();
    rect.p0.y += (S32) OffsetZ();
    rect.p1.y += (S32) OffsetZ();

    // convert it to cluster coords 
    rect.p0.x = (S32) ((F32) rect.p0.x * clusPerMeter);
    rect.p1.x = (S32) ((F32) rect.p1.x * clusPerMeter);
    rect.p0.y = (S32) ((F32) rect.p0.y * clusPerMeter);
    rect.p1.y = (S32) ((F32) rect.p1.y * clusPerMeter);
  
    // increase bounds by one along each edge to be sure everything gets drawn
    rect.p0.x--;
    rect.p1.x++;
    rect.p0.y--;
    rect.p1.y++;

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
    // calc the first cluster's and cell's offsets
    U32 clusOffset, clusOffsetStart = rect.p0.y * clusWidth + rect.p0.x;
    U32 cellOffset, cellOffsetStart = rect.p0.y * clusWidth * cellPerClus * cellPerClus + rect.p0.x * cellPerClus;

    // re-convert to meters
    rect.p0.y *= meterPerClus;
    rect.p1.y *= meterPerClus;
    rect.p0.x *= meterPerClus;
    rect.p1.x *= meterPerClus;

    rect.p0.y -= (S32) OffsetZ();
    rect.p1.y -= (S32) OffsetZ();
    rect.p0.x -= (S32) OffsetX();
    rect.p1.x -= (S32) OffsetX();

    U32 cellClusWidth = heightField.cellPitch * cellPerClus;
    while (rect.p0.y < rect.p1.y)
    {
      clusOffset = clusOffsetStart;
      cellOffset = cellOffsetStart;
      S32 x;
      for (x = rect.p0.x; x < rect.p1.x; x += meterPerClus, clusOffset++, cellOffset += cellPerClus)
      {
        Cluster & clus = clusList[ clusOffset];

/*
        U32 clipFlags = Vid::CurCamera().BoundsTestOrigin( clus.bounds.Offset(), clus.bounds);
        if (clipFlags == clipOUTSIDE)
        {
          // cluster is completely outside the view frustrum
          continue;
        }
        Vid::Light::SetActiveList( Sky::sun);
        Vid::Light::SetupLightsModel();

        RenderClusterIsometric( clus, x, rect.p0.y, cellOffset, 1, clipFlags);
*/
        RenderClusterIsometric( clus, x, rect.p0.y, cellOffset, 1, clipALL, doColor, doOverlay);
      }
      rect.p0.y += meterPerClus;
      clusOffsetStart += clusWidth;
      cellOffsetStart += cellClusWidth; 
    }
  }
  //----------------------------------------------------------------------------


  void RenderTerrainMap( const char * filename, U32 size, Bool doColor, Bool doOverlay)
  {
    size = Min<U32>( size, Vid::backBmp.Width());
    size = Min<U32>( size, Vid::backBmp.Height());
    size = Min<U32>( size, Vid::caps.maxTexWid);
    size = Min<U32>( size, Vid::caps.maxTexHgt);

    const F32 MAPVIEWNEAR = 2000.0f;
    F32 viewfar = MAPVIEWNEAR + Terrain::terrMaxHeight - Terrain::terrMinHeight + 20;
    F32 fov = STARTFOV * VALDEGTORAD;
    Area<S32> rect;

    // create the map camera
    Camera *mainCam = &Vid::CurCamera();
    Camera *camera = new Camera("map");

    F32 twid = (F32) Terrain::MeterWidth();
    F32 thgt = (F32) Terrain::MeterHeight();

    // set up normal camera parameters
    camera->SetProjTransformIso( MAPVIEWNEAR, viewfar, fov, twid, thgt);

    // set its orientation (pointing strait down)
    camera->SetWorld( 
      Quaternion( -PIBY2, Matrix::I.right), 
      Vector( 0, MAPVIEWNEAR + Terrain::terrMaxHeight, 0)
      );

    // set the camera viewport
    rect.Set(0, 0, size, size);
    camera->Setup( rect);

    // position the camera
    Vector pos = camera->WorldMatrix().Position();
    pos.x = twid * 0.5f;
    pos.z = thgt * 0.5f;
    camera->SetWorld( pos);

    // Turn off fog
    Bool isFog = Vid::SetFogStateI( FALSE);

    // Render terrain with full ambient light and no shroud
    //
    // FIXME: set light time too
    //
    F32 r, g, b;
    Vid::GetAmbientColor(r, g, b);
    Vid::SetAmbientColor(0.1F, 0.1F, 0.1F);
    Vid::Light::SetSun(0.7F, 1.0f, 1.0f, 1.0f);

    rect.Set( 0, 0, size, size);
    camera->Setup(rect);
    Vid::SetCamera(*camera);
    Vid::RenderBegin();
    Vid::RenderClear();

    Bool shroud = Vid::Var::Terrain::shroud;
    Vid::Var::Terrain::shroud = FALSE;

    Terrain::RenderIsometric( doColor, doOverlay);
    Vid::Var::Terrain::shroud = shroud;

    Vid::RenderEnd();

    Bitmap * terrainTex = new Bitmap( bitmapSURFACE | bitmapNOMANAGE);
    terrainTex->SetName( filename);
    terrainTex->Create( size, size, FALSE);
    terrainTex->InitPrimitives();
    terrainTex->Clear(terrainTex->MakeRGBA(0, 0, 0, 255));

    // Copy backbuffer to internal terrain buffer
    Vid::backBmp.CopyBits
    (
      *terrainTex, 0, 0, 0, 0, size, size
    );

    terrainTex->WriteBMP( filename, TRUE);

    // Restore ambient color
    Vid::SetCamera( *mainCam);
    Vid::SetAmbientColor(r, g, b);
    Vid::SetFogStateI( isFog);

    delete terrainTex;
    delete camera;
  }
}
//----------------------------------------------------------------------------
