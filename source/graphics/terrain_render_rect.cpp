//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// terrain_render_rect.cpp
//
// 05-APR-2000
//

#include "vid_public.h"
#include "light_priv.h"
#include "vidclip.h"
#include "random.h"
#include "console.h"
#include "statistics.h"
#include "terrain.h"
#include "terrain_priv.h"
//----------------------------------------------------------------------------

namespace Terrain
{

  // draw a single cluster
  //
  void RenderClusterColor( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, Color color, U32 clipFlags) // = 0
  {
    clus;

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

          Vid::ProjectFromModel_I( *dv, *sv);

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
	  	    if (planes[0].Evalue(Vid::Math::modelViewVector) <= 0.0f
	  	     && planes[1].Evalue(Vid::Math::modelViewVector) <= 0.0f)
          {
            continue;
          }
          // set indices
          iv[3] = iv[0];
          iv[4] = iv[2];

          iv += 6;
        }
      }

      Vid::UnlockIndexedPrimitiveMem( dv - vertmem, iv - indexmem);
    }
    else
    {
      VertexTL * vertmem;
      U16 * indexmem;
      U32 heapSize = Vid::Heap::ReqVertex( &vertmem, &indexmem);

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

          Vid::TransformFromModel( *dv, *sv);
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
	  	    if (planes[0].Evalue(Vid::Math::modelViewVector) <= 0.0f
	  	     && planes[1].Evalue(Vid::Math::modelViewVector) <= 0.0f)
          {
            continue;
          }
          // set indices
          iv[3] = iv[0];
          iv[4] = iv[2];

          iv += 6;
        }
      }
      Vid::Clip::ToBucket( vertmem, dv - vertmem, indexmem, iv - indexmem, &clus, TRUE, clipFlags);

      Vid::Heap::Restore( heapSize);
    }
  }
  //----------------------------------------------------------------------------

  // draw a cell's normals; used by editing brush
  //
  void RenderCellNormal( Cell &cell, F32 x, F32 z)
  {
    static Color color = RGBA_MAKE( 0, 255, 255, 255);

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

    Vid::ProjectFromModel_I( vertmem[0]);

    vertmem[0].vv.z *= Vid::renderState.zBias;

	  vertmem[1].vv.x = x + norm.x;
	  vertmem[1].vv.y = h + norm.y;
	  vertmem[1].vv.z = z + norm.z;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = 0xff000000;

    Vid::ProjectFromModel_I( vertmem[1]);

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

        Vid::ProjectFromModel_I( dv);
      }

      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);

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

        Vid::TransformFromModel( dv);
      }
      Vid::Clip::ToBucket( vertmem, 4, Vid::rectIndices, 6, &clus, TRUE, clipFlags);
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

    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_TEXCLAMP | RS_BLEND_DEF);
    Vid::SetTranBucketZMax( Vid::sortBRUSH0);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( NULL);

    S32 wid = rect.Width();
    S32 hgt = rect.Height();

  #define MAXCELLS      ((MAXBRUSHX + 1) * (MAXBRUSHZ + 1))
    ASSERT( U32((wid + 1) * (hgt + 1)) < Vid::renderState.maxVerts && U32((wid) * (hgt) * 6) < Vid::renderState.maxIndices);

    S32 meterStride = heightField.meterPerCell;
    S32 x = rect.p0.x * meterStride - (S32)OffsetX();
    S32 z = rect.p0.y * meterStride - (S32)OffsetZ();
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

    if (Vid::renderState.status.showNormals)
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
    Matrix mat( Quaternion( rotation, Matrix::I.up));

    Vid::SetBucketTexture( tex, TRUE, 0, doTile ? RS_BLEND_DEF : RS_TEXCLAMP | RS_BLEND_DEF);
    if (dofill)
    {
      Cell *c0 = &heightField.cellList[cellOffset];

      VertexTL * vertmem;
      U16 * indexmem;
      U32 heapSize = Vid::Heap::ReqVertex( &vertmem, &indexmem);

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

          Vid::TransformFromModel( *dv);

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

        Vid::TransformFromModel( *dv);

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

        Vid::TransformFromModel( *dv);
      }

      Vid::Clip::ToBucket( vertmem, dv - vertmem, indexmem, iv - indexmem, &clus);

      Vid::Heap::Restore( heapSize);
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
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetTranBucketZMax( Vid::sortBRUSH0);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( tex ? tex : texList[cell.texture]);

    U32 offset = (U32) (&cell - heightField.cellList);

    U32 z0 = (offset / heightField.cellPitch) - (S32)OffsetZ();
    U32 x0 = (offset - z0 * heightField.cellPitch) * heightField.meterPerCell - (S32)OffsetX();
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

    rect.p0.x += Utils::FtoL(OffsetX());    // FIXME
    rect.p1.x += Utils::FtoL(OffsetX());
    rect.p0.y += Utils::FtoL(OffsetZ());
    rect.p1.y += Utils::FtoL(OffsetZ());

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

}
//----------------------------------------------------------------------------
