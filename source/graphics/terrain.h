///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// terrain.h       DR2 terrain system
//
// 04-MAY-1998
//

#ifndef TERRAIN_H_
#define TERRAIN_H_

#include "vid_decl.h"
//----------------------------------------------------------------------------

#define DOTERRAINCOLOR
#define DOTERRAINOVERLAYS
//#define DOTERRAINLOD

#define CELLSIZE                      8     // meters
#define CELLPERCLUSTERSHIFT           2     // in one dimension
#define DEFAULTHEIGHT   						  10.0f
#define NORMALCOUNT							      256
#define TEXTURECOUNT                  256
#define MAXBRUSHX                     20
#define MAXBRUSHZ                     20
//----------------------------------------------------------------------------

struct Cluster;
//----------------------------------------------------------------------------

// a single cell of a height field 
//
struct Cell
{
  F32             height;   // actual height of this point in the field

  U8              normal;   // index into a normal array
  U8              uv;       // index into a uv array; handles rotation
  U8              texture;  // index into texture array
  U8              flags;    // fog of war and flag values

  S32             fog;      // current fog of war value

#ifdef DOTERRAINCOLOR
  Color           color;    // per vertex diffuse color
#endif

#ifdef DOTERRAINOVERLAYS
  U16             overlay;  // index into overlay description array
  U8              uv1;      // index into overlay's uv array
  U8              texture1; // index in secondary texture array
#endif

#ifdef DOTERRAINLOD
  Cell *          parent1;  // each sub-tri in an lod'ed cluster is controled
  Cell *          parent2;  // by 2 parent tris
#endif

  enum CellFlags
  {
    cellFOG          = 0x07,
    cellVISIBLE      = 0x08,
    cellOVERLAY      = 0x10,
    cellCOMPRESSED   = 0x20,
    cellLOD          = 0x40,
  };

  Cell()
  {
    ClearData();
  }
 
  void ClearData();

  void SetParents( U32 level);
  void Activate( U32 level);
  
  void Activate()
  {
#ifdef DOTERRAINLOD
    if (!(flags & cellLOD))
    {
      flags |= cellLOD;

      ActivateParents();
    }
#endif
  }
  void ActivateParents()
  {
#ifdef DOTERRAINLOD
    if (parent1)
    {
      parent1->Activate();
    }
    if (parent2)
    {
      parent2->Activate();
    }
#endif
  }
  inline void Deactivate()
  {
    flags &= ~cellLOD;
  }

  inline F32 GetHeight()
  {
    return height;
  }
  inline F32 SetHeight( F32 h)
  {
    F32 retValue = height;
    height = h;
    return retValue;
  }

  inline Bool GetVisible()
  {
    return flags & cellVISIBLE;
  }
  Bool SetVisible( Bool vis)
  {
    Bool retValue = flags & cellVISIBLE;
    if (vis)
    {
      flags |= cellVISIBLE;
    }
    else
    {
      flags &= ~cellVISIBLE;
    }
    return retValue;
  }
/*
  U8 GetFog()
  {
    return (U8) (flags & cellFOG);
  }
  U8 SetFog( U8 fog)
  {
    U8 retValue = (U8) (flags & cellFOG);
    flags &= ~cellFOG;
    flags |= (fog & cellFOG);
    return retValue;
  }
*/
  inline S32 GetFog()
  {
    return fog;
  }
  S32 SetFog( S32 _fog);

  inline Color GetColor()
  {
#ifdef DOTERRAINCOLOR
    return color;
#else
    return 0xffffffff;
#endif
  }
  inline Color SetColor( Color c)
  {
#ifdef DOTERRAINCOLOR
    Color oldColor = color;
    color = c;
    return oldColor;
#else
    return 0xffffffff;
#endif
  }
};
//----------------------------------------------------------------------------

// Clusters group 16 height cells together for quick render culling, and to define
// multiple height water.
//
struct Cluster
{
  Bounds          bounds;

  struct Status
  {
    U16           water  : 1;
    U16           right  : 1;
    U16           left   : 1;
    U16           top    : 1;
    U16           bottom : 1;
    U16           shroud : 1;   // fully shrouded

  }               status;

  U8              shroudCount;  // reference counting
  U8              waterIndex;

  F32             waterHeight;
  F32             zDepth;

#ifdef DOTERRAINLOD
  S32             x, z;
  U32             offset;
  U32             clipFlags;
#endif

#ifdef DOCLUSTERCELLS
  Cell            cells[25];  // no performance improvement
#endif

  GeoCache        geo;        // experimental caching of render data

  Cluster()
  {
    ClearData();
  }

  void ClearData();

  void Release()
  {
    geo.Release();
  }
  void Alloc();
};
//----------------------------------------------------------------------------

// a group of height cells
//
// The terrain namespace uses a single large height field as its base data.  The terrain
// editor uses smaller height field 'brushes' to modify it in realtime.
//
class HeightField
{
public:
  // types of code generated height brushes
  enum BrushType
  {
    BELL,
    FLAT,
    PLASMA,
    SMOOTH,
    CUSTOM,
    BRUSHTYPECOUNT
  };
  // flags to control height field cut/paste editing
  enum EditFlags
  {
    EDITNONE      = 0x00,
    EDITADD       = 0x01,
    EDITHEIGHTS   = 0x02,
    EDITTEXTURES  = 0x04,
    EDITRANDOM    = 0x08,
    EDITPLASMA    = 0x10,
    EDITSMOOTH    = 0x20,
  };

  U32             cellWidth, cellPitch, cellHeight, cellMax;
  U32             meterPerCell, meterWidth, meterHeight;
  F32             cellPerMeter, celldx;
  Cell *          cellList;

  F32             offsetX, offsetZ;

  HeightField()
  {
    ClearData();
  }
  ~HeightField()
  {
    Release();
  }

  void ClearData();
  Bool Setup( U32 wid, U32 hgt, U32 csize, F32 offsetX = 0.0f, F32 offsetZ = 0.0f, F32 startHeight = 10.0f);
  void Release();

  U32   CellOffset( F32 x, F32 z);
  void  CellHeights( U32 offset, F32 * heights);
  F32   FindFloor( F32 x, F32 z, Vector * surfNormal = NULL);
  F32   FindFloor( F32 * heights, F32 dx, F32 dz, Vector * surfNormal = NULL);
  Bool  ImportBitmap( char * buffer, U32 bwid, U32 bhgt, F32 scale, Area<S32> & rect);

  void  CalcCellNormal( U32 offset, Vector &norm);
  U32   CalcCellNormal( U32 offset);

  F32 GetCellHeight( U32 offset);
  F32 SetCellHeight( U32 offset, F32 height);
  F32 GetCellHeight( F32 x, F32 z)
  {
    return GetCellHeight( CellOffset( x, z));
  }
  F32 SetCellHeight( F32 x, F32 z, F32 h)
  {
    return SetCellHeight( CellOffset( x, z), h);
  }
  Bool GetCellVisible( U32 offset);
  Bool SetCellVisible( U32 offset, Bool vis);
  Bool GetCellVisible( F32 x, F32 z)
  {
    return GetCellVisible( CellOffset( x, z));
  }
  Bool SetCellVisible( F32 x, F32 z, Bool vis)
  {
    return SetCellVisible( CellOffset( x, z), vis);
  }
  U8 GetCellFog( U32 offset);
  U8 SetCellFog( U32 offset, U8 fog);
  U8 GetCellFog( F32 x, F32 z)
  {
    return GetCellFog( CellOffset( x, z));
  }
  U8 SetCellFog( F32 x, F32 z, U8 fog)
  {
    return SetCellFog( CellOffset( x, z), fog);
  }
  Color GetCellColor( U32 offset);
  Color SetCellColor( U32 offset, Color color);
  Color GetCellColor( F32 x, F32 z)
  {
    return GetCellColor( CellOffset( x, z));
  }
  Color SetCellColor( F32 x, F32 z, Color color)
  {
    return SetCellColor( CellOffset( x, z), color);
  }

  Cell *GetCell( S32 cx, S32 cz)
  {
    return &cellList[ cz * cellPitch + cx];
  }

  void CellCoords( Cell & cell, S32 & cx, S32 & cz)
  {
    U32 offset = (U32) (&cell - cellList);
    cz = offset / cellPitch;
    cx = offset - cz * cellPitch;
  }

  Bool CellCoords( F32 x, F32 z, S32 & cx, S32 & cz)
  {
    Bool retValue = TRUE;

    x += offsetX;
    z += offsetZ;

  	if (x < 0.0f)
    {
      x = 0.0f;
		  retValue = FALSE;
    }
    if (z < 0.0f)
    {
      z = 0;
		  retValue = FALSE;
    }
    if (x >= (F32) meterWidth)
    {
      x = (F32) (meterWidth - 1);
		  retValue = FALSE;
    }
    if (z >= (F32) meterHeight)
    {
      z = (F32) (meterHeight - 1);
		  retValue = FALSE;
    }

    cx = Utils::FtoL(x * cellPerMeter);
    cz = Utils::FtoL(z * cellPerMeter);

    return retValue;
  }

  void GetPos( Cell & cell, Vector & pos)
  {
    S32 cx, cz;
    CellCoords( cell, cx, cz);

	  pos.x = (F32) (cx * meterPerCell) - offsetX;
	  pos.z = (F32) (cz * meterPerCell) - offsetZ;
  }

  Bool CellOnMap( S32 cellX, S32 cellZ)
  {
    return (cellX >= 0 && cellX <= (S32)cellWidth && cellZ >= 0 && cellZ <= (S32)cellHeight);
  }
  Bool MeterOnMap(F32 x, F32 z)
  {
    x += offsetX;
    z += offsetZ;

    return (x >= 0.0f && x < (F32)meterWidth && z >= 0.0f && z < (S32)meterHeight);
  }

  // editing related
  void  Cut(   Area<S32> & dstRect, HeightField & buf, Area<S32> & bufRect, F32 scale, U32 flags = EDITHEIGHTS | EDITTEXTURES);
  void  Paste( Area<S32> & dstRect, HeightField & buf, Area<S32> & bufRect, F32 scale, U32 flags = EDITHEIGHTS, F32 atHeight = 0);

  Bool  Setup( U32 wid, U32 hgt, U32 csize, BrushType type, F32 scale = 1.0f, Bitmap * custom = NULL);
  void  Make( BrushType type, F32 scale = 1.0f, Bitmap * custom = NULL);
};
//----------------------------------------------------------------------------

#endif
