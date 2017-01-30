///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// heightfield.cpp     DR2 terrain system
//
// 04-MAY-1998
//
#include "terrain_priv.h"
#include "random.h"
#include "console.h"
#include "environment.h"
#include "environment_light.h"
#include "perfstats.h"
//----------------------------------------------------------------------------

// clear cell's values
//
void Cell::ClearData()
{
  height = DEFAULTHEIGHT;
  normal = 0;
  uv = 0;
  texture = 0;

  fog = 0;

#ifdef DOTERRMRM
  parent1 = parent2 = NULL;
#endif

  flags = cellLOD | cellVISIBLE;

#ifdef DOTERRAINCOLOR
  color = 0xffffffff;
#endif
}
//----------------------------------------------------------------------------

// clear cluster's values
//
void Cluster::ClearData()
{
  Utils::Memset( &status, 0, sizeof( status));

  status.shroud = TRUE;
  shroudCount = 25;
  waterIndex = 255;
}
//----------------------------------------------------------------------------

// clear heightfield's values
// 
void HeightField::ClearData()
{
  cellWidth  = 0;
  cellPitch  = 0;
  cellHeight = 0;
  cellMax    = 0;
  meterPerCell = 0;
  cellList = NULL;

  offsetX = offsetZ = 0.0f;
}
//----------------------------------------------------------------------------

// destroy a heightfield
//
void HeightField::Release()
{
  if (cellList)
  {
    delete [] cellList;
  }
  cellList = NULL;
}
//----------------------------------------------------------------------------

// return a offset in cellList based on 'x, z' coord in meters
//
U32 HeightField::CellOffset( F32 x, F32 z)
{
  S32 cx, cz;
  if (!CellCoords( x, z, cx, cz))
  {
		return cellMax;
  }
  return (U32) (cz * cellPitch + cx);
}
//----------------------------------------------------------------------------

F32 HeightField::GetCellHeight( U32 offset)
{
  ASSERT( offset < cellMax);

  // FIXME
  //
  if (offset < cellMax)
  {
    return cellList[offset].GetHeight();
  }
  return F32_MAX;
}
//----------------------------------------------------------------------------

F32 HeightField::SetCellHeight( U32 offset, F32 h)
{
  ASSERT( offset < cellMax);

  // FIXME
  //
  F32 retValue = F32_MAX;
  if (offset < cellMax)
  {
    retValue = cellList[offset].SetHeight( h);
  }
  return retValue;
}
//----------------------------------------------------------------------------

Bool HeightField::GetCellVisible( U32 offset)
{
  if (offset < cellMax)
  {
    return cellList[offset].GetVisible();
  }
  return FALSE;
}
//----------------------------------------------------------------------------

Bool HeightField::SetCellVisible( U32 offset, Bool vis)
{
  Bool retValue = FALSE;
  if (offset < cellMax)
  {
    retValue = cellList[offset].SetVisible( vis);
  }
  return retValue;
}
//----------------------------------------------------------------------------

Color HeightField::GetCellColor( U32 offset)
{
  offset;

#ifdef DOTERRAINCOLOR
  if (offset < cellMax)
  {
    return cellList[offset].GetColor();
  }
#endif

  return 0xffffffff;
}
//----------------------------------------------------------------------------

Color HeightField::SetCellColor( U32 offset, Color c)
{
  offset, c;
  Color retValue = 0xffffffff;

#ifdef DOTERRAINCOLOR
  if (offset < cellMax)
  {
    retValue = cellList[offset].SetColor( c);
  }
#endif

  return retValue;
}
//----------------------------------------------------------------------------

// return heights that define the cell indexed by offset
//
void HeightField::CellHeights( U32 offset, F32 *heights)
{
  Cell *cell = cellList + offset;

  heights[0] = cell->height;
  cell += cellPitch;
  heights[1] = cell->height;
  cell += 1;
  heights[2] = cell->height;
  cell -= cellPitch;
  heights[3] = cell->height;
}
//----------------------------------------------------------------------------

// calculate the actual normal for a cell
// return it in 'norm'
//
void HeightField::CalcCellNormal( U32 offset, Vector &norm)
{
  F32 h[4], mpc = (F32) meterPerCell;
  CellHeights( offset, h);
  Vector v1, v2, v3;
  v1.Set( mpc,  h[3] - h[0], 0.0f);
  v2.Set( mpc,  h[2] - h[0], mpc);
  v3.Set( 0.0f, h[1] - h[0], mpc);

  norm =  v2.Cross( v1);
  norm += v3.Cross( v2);

  CellHeights( offset - 1, h);
  v1.Set( mpc,  h[3] - h[0], 0.0f);
  v2.Set( mpc,  h[2] - h[0], mpc);

  norm += v2.Cross( v1);

  CellHeights( offset - cellPitch, h);
  v2.Set( mpc,  h[2] - h[0], mpc);
  v3.Set( 0.0f, h[1] - h[0], mpc);

  norm += v3.Cross( v2);

  CellHeights( offset - cellPitch - 1, h);
  v1.Set( mpc,  h[3] - h[0], 0.0f);
  v2.Set( mpc,  h[2] - h[0], mpc);
  v3.Set( 0.0f, h[1] - h[0], mpc);

  norm += v2.Cross( v1);
  norm += v3.Cross( v2);

  norm.Normalize();
}
//----------------------------------------------------------------------------

// set the cell's normal index into the standard Terrain normal list
// via the slow algorithim
// ! this function assumes that the cell is not on the bounding edge of the heightfield
// ! i.e. cx > 0 && cx < cellwidth - 1 && cz > 0 && cz < cellheight - 1
//
U32 HeightField::CalcCellNormal( U32 offset)
{
  // find the actual normal
  Vector norm;
  CalcCellNormal( offset, norm);

  return Terrain::FindNormal( norm);
}
//----------------------------------------------------------------------------

// allocate a heightfield
//
Bool HeightField::Setup( U32 wid, U32 hgt, U32 csize, F32 _offsetX, F32 _offsetZ, F32 startHeight) // = 0.0f, 0.0f, 10.0f
{
  ASSERT( wid > 0 && hgt > 0 && csize > 0);

  // release anything already open
  Release();

  ASSERT( _offsetX >= 0 && _offsetZ >= 0);
  offsetX = _offsetX;
  offsetZ = _offsetZ;

  cellWidth  = wid;
  cellPitch  = wid + 1;
//  cellPitch  = wid;
  cellHeight = hgt;
  cellMax = cellPitch * hgt - 1;

  meterPerCell = csize;
  cellPerMeter = 1.0f / (F32) meterPerCell;

  meterWidth  = cellWidth  * meterPerCell;
  meterHeight = cellHeight * meterPerCell;

  // for height calculations inside cell's tris
  celldx = 1.0f / (meterPerCell * meterPerCell);

  // allocate a extra row at the bottom
  // cells are initialized through their constructors
  U32 cellCount = cellMax + 1 + cellWidth + 1;
  cellList = new Cell[cellCount];
  if (!cellList)
  {
    ASSERT( cellList);

    return FALSE;
  }

  U32 i;
  for (i = 0; i < cellCount; i++)
  {
    cellList[i].height = startHeight;
  }

  return TRUE;
}
//----------------------------------------------------------------------------

// allocate and build a heightfield of brushtype 'type'
//
Bool HeightField::Setup( U32 wid, U32 hgt, U32 csize, BrushType type, F32 scale, Bitmap *custom) // = 1.0f, = NULL
{
  if (!Setup( wid, hgt, csize))
  {
    return FALSE;
  }
  Make( type, scale, custom);

  return TRUE;
}
//----------------------------------------------------------------------------

// build a heightfield of brushtype 'type'
// 
void HeightField::Make( BrushType type, F32 scale, Bitmap *custom) // = 1.0f, = NULL
{
  ASSERT( cellList);

  if (type == CUSTOM)
  {
    // Import a custom bitmap
    if (custom)
    {
      Area<S32> rect( cellWidth, cellHeight);
      ImportBitmap((char *)custom->Data(), custom->Width(), custom->Height(), scale, rect);
    }
  } 
  else if (type == PLASMA)
  {
    /*
    if (!Editor::editMap.bmpData)
    {
      Editor::editMap.Create( 50, 50, FALSE);
    }
    if (!PLASMA_gen( (U8 *) Editor::editMap.p1.ympData, 
     (short) Editor::editMap.bmpWidth, (short) Editor::editMap.bmpHeight, 
     (U8) Editor::plasmaGrain, Editor::plasmaKey))
    {
      ERR_FATAL( ("can't make a plasma brush") );
    }
    Area<S32> rect;
    rect.Set( cellWidth, cellHeight);
    ImportBitmap( (char *)Editor::editMap.bmpData, 
      Editor::editMap.bmpWidth, Editor::editMap.bmpHeight, scale, rect);
    */
    return;
  }
  else if (type == SMOOTH)
  {
    return;
  }

  F32 radius = (F32) (cellPitch * meterPerCell) * 0.5f;
  F32 cx = meterWidth  * 0.5f;
  F32 cz = meterHeight * 0.5f;

  U32 z, x;
  Cell *cell = cellList;
  for (z = 0; z < cellHeight; z++) 
  {
    F32 mz = (F32) z * (F32) meterPerCell;
    for (x = 0; x < cellPitch; x++, cell++) 
    { 
      switch (type)
      {
      case FLAT:
        cell->height = scale;
        break;
      case BELL:
        {
          F32 mx = (F32) x * (F32) meterPerCell;
          F32 dx = mx - cx;
          F32 dz = mz - cz;
          F32 rad = (F32) sqrt( dx * dx + dz * dz);
          if (rad > radius) 
          {
            rad = 0.0f;
          }
          else
          {
            rad = (F32) cos( (F32) PIBY2 * rad / radius) * scale;
          }
          cell->height = rad;
        }
        break;
      }
    }
  }
}
//----------------------------------------------------------------------------

// perform a heightfield to heightfield paste operation
//
void HeightField::Paste( Area<S32> & dstRect, HeightField & buf, Area<S32> & bufRect, F32 scale, U32 flags, F32 atHeight)
{
  ASSERT( cellList && buf.cellList);

  // in cells
  U32 wid = dstRect.Width();
  U32 hgt = dstRect.Height();

  // in meters
  F32 sdx = (F32) bufRect.Width()  * buf.meterPerCell / (F32) wid;
  F32 sdz = (F32) bufRect.Height() * buf.meterPerCell / (F32) hgt;
  F32 sz  = (F32) bufRect.p0.y * buf.meterPerCell;

  S32 z, x;  // cells
  for (z = dstRect.p0.y; z < dstRect.p1.y; z++, sz += sdz)
  {
    F32 sx  = (F32) bufRect.p0.x * buf.meterPerCell;
    x = dstRect.p0.x;
    U32 offset = z * cellPitch + x;
    for ( ; x < dstRect.p1.x; x++, sx += sdx, offset++)
    {
      if (x < 0 || x > (S32) cellWidth || z < 0 || z > (S32) cellHeight)
      {
        continue;
      }

      Cell &cell = cellList[ offset];
      if (flags & HeightField::EDITSMOOTH)
      {
        if (x < 1 || x >= (S32) cellWidth -1  || z < 1 || z >= (S32) cellHeight -1)
        {
          continue;
        }
        Cell &c0 = cellList[offset - 1];
        Cell &c1 = cellList[offset + 1];
        Cell &c2 = cellList[offset - cellPitch];
        Cell &c3 = cellList[offset + cellPitch];

        // move towards the average of surrounding cells' heights
        F32 h = (c0.height + c1.height + c2.height + c3.height) * 0.25f - cell.height;
        // by a miniture factor of the brushscale (unfactored smoothing is too aggressive)
        h *= scale * 0.3f;
        cell.height += h;
      }
#if 1
      else if (flags & HeightField::EDITHEIGHTS)
      {
        if (flags & HeightField::EDITADD)
        {
          cell.height += buf.FindFloor( sx, sz) * scale;
        }
        else
        {
          cell.height = atHeight + (buf.FindFloor( sx, sz) - 1.0f) * scale;
        }
      }
#else
      else if (flags & HeightField::EDITHEIGHTS)
      {
        if (flags & HeightField::EDITADD)
        {
          // add heights to the current height
          atHeight = cell.height;
        }
        F32 y = atHeight + buf.FindFloor( sx, sz) * scale;
        //if (y < 0.0f)
        //{
          // clamp terrain to 0 meters or above
        //  y = 0.0f;
        //}
        cell.height = y;
      }
#endif
    }
  }
}
//----------------------------------------------------------------------------

// calculate the height at the location 'x, z' in meters
//
F32 HeightField::FindFloor( F32 x, F32 z, Vector * surfNormal) // = NULL
{
  x += offsetX;
  z += offsetZ;

#if DEVELOPMENT
	ASSERT( x >= 0.0f && x < (F32) meterWidth && z >= 0.0f && z < (F32) meterHeight);

#else
  // clamp coords
  //
	if (x < 0.0f)
  {
    x = 0;
  }
  else if (x >= (F32) meterWidth)
  {
    x = meterWidth - .1f;
  }
	if (z < 0.0f)
  {
    z = 0;
  }
  else if (z >= (F32) meterHeight)
  {
    z = meterHeight - .1f;
  }
#endif

  // trunc mode is the default
  //
  S32 cx = Utils::FtoL(x * cellPerMeter);
  S32 cz = Utils::FtoL(z * cellPerMeter);

  U32 offset = (U32) (cz * cellPitch + cx);
  F32 heights[4];
  CellHeights( offset, heights);

  F32 dx = x - ((F32) cx * (F32) meterPerCell);
  F32 dz = z - ((F32) cz * (F32) meterPerCell);

  return FindFloor( heights, dx, dz, surfNormal);
}
//----------------------------------------------------------------------------

// find the height (y) corresponding to the 'dx dz' coord in meters
// WITHIN a single cell's 'heights'
//
F32 HeightField::FindFloor( F32 * heights, F32 dx, F32 dz, Vector * surfNormal) // = NULL
{
  // top left vertex indexes both tris
  //
  // 0*\--*3
  //  | \ |
  // 1*--\*2
  //
//	dx = 1.0f / (dx12 * dz02 - dx02 * dz12);
//  DyDx = (dy12 * dz02 - dy02 * dz12) *  dx;
//	DyDz = (dy12 * dx02 - dy02 * dx12) * -dx;

  F32 dy12, dy02, dydx, dydz;
  if (dz + meterPerCell - dx < (F32) meterPerCell)
  {
    // top tri
    // 0<---2
    //   \ |
    //    \|1
    // vert 0 is cell 0, 1 is 2, 2 is 3
    dy12 = (heights[3] - heights[2]) * meterPerCell; // * dx02
    dy02 = (heights[3] - heights[0]) * meterPerCell; // * dz12
    dydx = dy02 *  celldx;
	  dydz = dy12 * -celldx;

    if (surfNormal)
    {
      *surfNormal = Cross( Vector( (F32) meterPerCell, heights[2] - heights[0], (F32) meterPerCell),
                           Vector( (F32) meterPerCell, heights[3] - heights[0], 0.0f) );

      surfNormal->Normalize();
    }
  }
  else 
  {
    // bottom tri
    // 0|\
    //  | \
    // 1--->2
    // vert 0 is cell 0, 1 is 1, 2 is 2
    dy12 = (heights[2] - heights[1]) * meterPerCell; // * dz02 (and * dx02)
    dy02 = (heights[2] - heights[0]) * meterPerCell; // * dx12
    dydx = dy12 * celldx;
	  dydz = (dy12 - dy02) * -celldx;

    if (surfNormal)
    {
      *surfNormal = Cross( Vector( 0.0f, heights[1] - heights[0], (F32) meterPerCell),
                           Vector( (F32) meterPerCell, heights[2] - heights[0], (F32) meterPerCell) );

      surfNormal->Normalize();
    }
  }

	F32 y = dx * dydx + dz * dydz;

  return y + heights[0];
}
//----------------------------------------------------------------------------

// import a buffer of greyscale values as a heightfield
//
Bool HeightField::ImportBitmap( char * buffer, U32 bwid, U32 bhgt, F32 scale, Area<S32> & rect)
{
  U32 wid = rect.Width();
  U32 hgt = rect.Height();

  F32 dbx  = (F32) bwid / (F32) wid;  
  F32 dbz  = (F32) bhgt / (F32) hgt;  
  F32 bz = 0.0f;

  S32 cx, cz, offset = rect.p0.y * cellPitch + rect.p0.x;
  for (cz = rect.p0.y; cz < rect.p1.y; cz++, bz += dbz)
  {
    F32 boff = bz * (F32) bwid;
    for (cx = rect.p0.x; cx < rect.p1.x; cx++, offset++, boff += dbx)
    {
      F32 h = ((U8) buffer[ (U32) boff]) * scale;
      cellList[offset].height = h;
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------
