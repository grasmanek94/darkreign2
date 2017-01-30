///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// terrain.cpp     DR2 terrain system
//
// 04-MAY-1998
//

#include "vid_public.h"
#include "random.h"
#include "console.h"
#include "terrain.h"
#include "statistics.h"
//----------------------------------------------------------------------------

#ifdef DOTERRAINLOD

namespace Terrain
{
  void SetClusLod( U32 offset, Bool flag)
  {
    U32 wid = CellPitch();
    Cell *c0 = &heightField.cellList[offset];

    if (flag)
    {
      (c0 + 1)->Activate();
      (c0 + 3)->Activate();
      (c0 + wid)->Activate();
      (c0 + wid + 4)->Activate();
      (c0 + 3 * wid)->Activate();
      (c0 + 3 * wid + 4)->Activate();
      (c0 + 4 * wid + 1)->Activate();
      (c0 + 4 * wid + 3)->Activate();
    }
    else
    {
      Cell *c, *ce, *ce0 = c0 + wid * cellPerClus;

      for (c = c0 + 1, ce = c0 + cellPerClus; c < ce; c++)
      {
        c->flags &= ~Cell::cellLOD;
      }
      for (c0 += wid; c0 < ce0; c0 += wid)
      {
        for (c = c0, ce = c0 + cellPerClus; c <= ce; c++)
        {
          c->flags &= ~Cell::cellLOD;
        }
      }
      for (c = c0 + 1, ce = c0 + cellPerClus; c < ce; c++)
      {
        c->flags &= ~Cell::cellLOD;
      }
    }
  }
  //----------------------------------------------------------------------------

  void ClusSetParents( U32 o0, U32 o1, U32 o2, U32 i0, U32 i1, U32 i2, U32 level = 0)
  {
    o0, o1, o2, i0, i1, i2, level;

    U32 o3 = (o1 + o2) >> 1;
    U32 i3 = (i1 + i2) >> 1;

    if (level < 3)
    {
      level++;

      ClusSetParents( o3, o2, o0, i3, i2, i0, level);
      ClusSetParents( o3, o0, o1, i3, i0, i1, level);
    }
    Cell &c3 = heightField.cellList[o3];
    Cell &c0 = heightField.cellList[o0];

    if ( (o0 / heightField.cellPitch) % cellPerClus == 0
     &&  (o0 % heightField.cellPitch) % cellPerClus == 0 )
    {
      // cluster corners
      return;
    }

    c3.parent1 = &c0;

    S32 o4 = 2 * S32(o3) - S32(o0);

    S32 clusx = (o0 % heightField.cellPitch) / cellPerClus;
    Bool hit = FALSE;
    if (o4 < 0 || o4 > S32(heightField.cellMax))
    {
      hit = TRUE;
    }
    else if (clusx == 0 && (i3 == 5 || i3 == 10 || i3 == 15))
    {
      hit = TRUE;
    }
    else if (clusx == S32(clusWidth - 1) && (i3 == 9 || i3 == 14 || i3 == 19))
    {
      hit = TRUE;
    }
    if (!hit)
    {
      Cell &c4 = heightField.cellList[o4];
      c3.parent2 = &c4;
    }
  }
  //----------------------------------------------------------------------------

  Bool ChekTri( F32 py, U32 o0, U32 o1, U32 o2, U32 level) // = 0
  {
    U32 o3 = S32((o1 + o2) >> 1);

    if (level < 3)
    {
      level++;

      ChekTri( py, o3, o0, o1, level);
      ChekTri( py, o3, o2, o0, level);
    }
    Cell &c1 = heightField.cellList[o1];
    Cell &c2 = heightField.cellList[o2];
    Cell &c3 = heightField.cellList[o3];

    if ((F32) fabs( c3.height * 2.0f - c1.height - c2.height) * py > lodThresh)
    {
      c3.Activate();

      return TRUE;
    }
    return FALSE;
  }
  //----------------------------------------------------------------------------

  Bool DrawTri( U32 o0, U32 o1, U32 o2, U32 i0, U32 i1, U32 i2, Vector *verts, U16 **indexList, U32 level) // = 0
  {
    Bool ret = FALSE;

    if (level < 4)
    {
      S32 o3 = S32((o1 + o2) >> 1);
      S32 i3 = S32((i1 + i2) >> 1);
      level++;

      ret |= DrawTri( o3, o0, o1, i3, i0, i1, verts, indexList, level);
      ret |= DrawTri( o3, o2, o0, i3, i2, i0, verts, indexList, level);
    }

    Cell &c0 = heightField.cellList[o0];

    if ((!ret && (c0.flags & Cell::cellLOD)))
    {
      Plane plane;
      plane.Set( verts[i0], verts[i1], verts[i2]);

      // backcull
  	  if (plane.Evalue(Vid::model_view_vector) > 0.0f)
      {
        U16 *il = *indexList;

        il[0] = U16(i0);
        il[1] = U16(i1);
        il[2] = U16(i2);

        (*indexList) += 3;
      }
      return TRUE;
    }
    return ret;
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterLOD( Cluster &clus, S32 x, S32 z, U32 cellOffset, U32 clipFlags) // = clipALL
  {
    // FIXME: check clipflags and backcull
    //
    // build list of vertex positions, normals, and colors
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    if (clus.status.water && *water)
    {
      RenderClusterWater( clus, x, z, xend, zend, clus.waterHeight, clipFlags);
	  }

    Camera &cam = Vid::CurCamera();

    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);
    Vid::SetTranBucketZMax(Vid::sortTERRAIN);

    Cell *c0 = &heightField.cellList[cellOffset];

    Vector verts[25];
    Vector norms[25];
    Color colors[25];
    F32     fogs[25];

    U32 meterStride = MeterPerCell();
    U32 cellStride  = CellPitch();

    VertexTL * vertmem;
    U32 heapSize = Vid::Heap::ReqVertex( &vertmem);

    VertexTL *dvv = vertmem;
    Vector *dv = verts;
    Vector *dn = norms;
    Color  *dc = colors;
    F32    *df = fogs;
    F32 u, v;
    for (z0 = z, v = 0.0f; z0 <= zend; z0 += meterStride, c0 += cellStride, v += 1.0f)
    {
      Cell *c = c0;
      for (x0 = x, u = 0.0f; x0 <= xend; x0 += meterStride, dv++, dn++, dc++, df++, c += 1, dvv++, u += 1.0f)
      {
        if (shroud)
        {
          *df = c->GetFog();
        }

        dv->x = (F32) x0;
        dv->z = (F32) z0;
        dv->y = c->height;

        *dn = normList[c->normal];

        *dc = c->color;

        dvv->u = u;
        dvv->v = v;
      }
    }

    // reset loop variables
    c0 = &heightField.cellList[cellOffset];

    // submit cells
    U32 vcount = dv - verts;

    cam.LightTransformFromModel( vertmem, verts, norms, colors, vcount);

    if (shroud)
    {
      for (dvv--, df--; dvv >= vertmem; dvv--, df--)
      {
        dvv->diffuse.Modulate( *df, *df, *df);
      }
    }

    U32 o0 = cellOffset;
    U32 o1 = o0 + 4 * CellPitch();
    U32 o2 = o0 + 4;
    U32 o3 = o1 + 4;

    U16 indexlist[96];
    U16 *il = indexlist;
    DrawTri( o0, o1, o2,  0, 20,  4, verts, &il);
    DrawTri( o3, o2, o1, 24,  4, 20, verts, &il);

    Cell &c = heightField.cellList[o0];
    Bitmap *tex = texList[c.texture];
//    Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);
    Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF);

    Vid::CurCamera().ClipToBucket( vertmem, vcount, indexlist, il - indexlist, &clus, clipFlags);

    Vid::Heap::Restore( heapSize);
  }
  //----------------------------------------------------------------------------


  // draw a single cluster
  //
  void RenderClusterLODQuick( Cluster &clus, S32 x, S32 z, U32 cellOffset, U32 clipFlags) // = clipALL
  {
    // build list of vertex positions, normals, and colors
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    if (clus.status.water && *water)
    {
      RenderClusterWaterQuick( clus, x, z, xend, zend, clus.waterHeight, clipFlags);
	  }

    Camera &cam = Vid::CurCamera();

    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);
    Vid::SetTranBucketZMax(Vid::sortTERRAIN);

    Cell *c0 = &heightField.cellList[cellOffset];

    U32 meterStride = MeterPerCell();
    U32 cellStride  = CellPitch();

    VertexTL * vertmem;
    U32 heapSize = Vid::Heap::ReqVertex( &vertmem);

    VertexTL * dvv = vertmem;
    Vector verts[25];
    Vector *dv = verts;
    F32 u, v;
    for (z0 = z, v = 0.0f; z0 <= zend; z0 += meterStride, c0 += cellStride, v += 1.0f)
    {
      Cell *c = c0;
      for (x0 = x, u = 0.0f; x0 <= xend; x0 += meterStride, dv++, c += 1, dvv++, u += 1.0f)
      {
        dv->x = (F32) x0;
        dv->z = (F32) z0;
        dv->y = c->height;

        cam.Transform( *dvv, *dv);

        dvv->u = u;
        dvv->v = v;

        Color col = c->color;
        dvv->diffuse.Modulate( normLights[c->normal], 
          (F32)col.r * U8toNormF32,
          (F32)col.g * U8toNormF32,
          (F32)col.b * U8toNormF32,
          (F32)col.a * U8toNormF32);

        if (shroud)
        {
          F32 df = c->GetFog();
          dvv->diffuse.Modulate( df, df, df);
        }

      }
    }

    // reset loop variables
    c0 = &heightField.cellList[cellOffset];

    // submit cells
    U32 vcount = dvv - vertmem;

    U32 o0 = cellOffset;
    U32 o1 = o0 + 4 * CellPitch();
    U32 o2 = o0 + 4;
    U32 o3 = o1 + 4;

    U16 indexlist[96];
    U16 *il = indexlist;
    DrawTri( o0, o1, o2,  0, 20,  4, verts, &il);
    DrawTri( o3, o2, o1, 24,  4, 20, verts, &il);

    Cell &c = heightField.cellList[o0];
    Bitmap *tex = texList[c.texture];
//    Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF | renderFlags);
    Vid::SetBucketTexture( tex, FALSE, 0, RS_BLEND_DEF);

    Vid::CurCamera().ClipToBucket( vertmem, vcount, indexlist, il - indexlist, &clus, clipFlags);

    Vid::Heap::Restore( heapSize);
  }
  //----------------------------------------------------------------------------

}
//----------------------------------------------------------------------------

#endif