///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 25-FEB-1999
//

#ifndef __TERRAINDATA_H
#define __TERRAINDATA_H


#include "blockfile.h"
#include "worldctrl.h"
#include "utiltypes.h"
#include "terraingroup.h"
#include "bitarray.h"
#include "footprint.h"
#include "terrain_priv.h"

///////////////////////////////////////////////////////////////////////////////
//
// Namespace TerrainData - Game-play terrain management
//
namespace TerrainData
{
  // Blend directions
  enum BlendDir 
  { 
    BD_NORTH    = 0, 
    BD_EAST     = 1,
    BD_SOUTH    = 2,
    BD_WEST     = 3,
    
    BD_MAXIMUM  = 4
  };


  //
  // The terrain data cell structure
  // 
  struct Cell
  {
    // Index to terrain type
    U8 terrain;

    // Index to surface type
    U8 surface;

    // Slope at the middle of this cell (0-90)
    U8 slope;

    // Index to footprint instance, or zero if none
    U16 footIndex;

    // Change the terrain type of this cell
    void ModifyTerrainType(TerrainGroup::Type &type)
    {
      terrain = type.Index();
      surface = type.Surface();
    }

  };


  //
  // A cell position used with an NList
  //
  struct NPoint : public Point<S32>
  {
    NList<NPoint>::Node node;
    NPoint(S32 x, S32 z) : Point<S32>(x, z) {}
    NPoint(const Point<S32> &p) : Point<S32>(p) {}
  };

  
  // 
  // An NList of cell positions
  //
  class PointList : public NList<NPoint>
  {
  public:

    // Constructor 
    PointList() : NList<NPoint>(&NPoint::node)
    {
    }
  };


  //
  // A cell to be painted
  //
  struct PaintCell : public Point<S32>
  {
    U8 type;
    NList<PaintCell>::Node node;
    PaintCell(S32 x, S32 z, U8 type) : Point<S32>(x, z), type(type) {}
    PaintCell(const Point<S32> &p, U8 type) : Point<S32>(p), type(type) {}
  };

  
  // 
  // Custom paint cell list
  //
  class PaintList : public NList<PaintCell>
  {
  public:

    // Constructor 
    PaintList() : NList<PaintCell>(&PaintCell::node)
    {
    }
  };

    // The terrain cell map
  extern Cell *cellMap;

  // Bit array for map used to signal second layer usage
  extern BitArray2d *bitArraySecondLayer;

  // Bit array for line of sight blockage
  extern BitArray2d *bitArrayBlockLOS;


  //
  // System functions
  //

  // Initialize and shutdown system
  void Init();
  void Done();

  // Save/Load terrain data
  void Save(BlockFile &bFile);
  void Load(BlockFile &bFile);
  void SetupExtendedInfo();

  // Paint all cells in the given list
  void Paint(const PaintList &paintList);

  // Re-blend the entire map
  void BlendEntireMap();

  // Fill a paint list using the given area
  void GeneratePaintList(Point<S32> a, Point<S32> b, PaintList &paintList, U8 type);

  // Adjusts the blend and variation of a cell (values are wrapped)
  void AdjustCell(S32 x, S32 z, BlendDir direction, U32 variation);

  // Returns the footprint instance at the given map location (must exist)
  FootPrint::Instance & GetFootPrintInstance(S32 x, S32 z);

  // Returns the footprint instance at the map location, or NULL if none there
  FootPrint::Instance * FindFootPrintInstance(S32 x, S32 z);

  // Start\End a run of cell modifications
  void SessionStart();
  void SessionEnd();

  // Change the height of the given cell
  void SessionModifyHeight(U32 x, U32 z, F32 height);

  // Modify second layer flag for this cell
  void SessionModifySecondLayer(U32 x, U32 z, Bool toggle);

  // Set this block LOS flag for this cell
  void SessionModifyBlockLOS(U32 x, U32 z, Bool toggle);

  // Add all cells in the given area (points must be sorted)
  void SessionAddArea(const Point<S32> &a, const Point<S32> &b);

  // Restore the original surface type based on the terrain type and water
  void RestoreSurfaceType(S32 x, S32 z);

  // Is this surface type water ?
  Bool IsWater(U8 surface);

  // including footprinted objects
  // doesn't check for valid cell coords
  //
  F32 GetHeight( S32 cx, S32 cz);
  F32 GetHeightGroundSprite( S32 cx, S32 cz);

  // including footprinted objects
  //
  F32 FindFloor( F32 x, F32 z, Vector *surfNormal = NULL);
  F32 FindFloorWithWater( F32 x, F32 z, Vector *surfNormal = NULL);

  F32 GetHeightWithWater( S32 cx, S32 cz);
  F32 GetHeightWithWaterGroundSprite( S32 cx, S32 cz);    // ignores use of 

  //
  // Inline functions
  //

  // Find the water height at this METER location (FALSE if no water)
  //
  inline Bool FindWater(F32 x, F32 z, F32 *height = NULL, F32 *depth = NULL)
  {
    return Terrain::FindWater( x, z, height, depth);
  }

  // Find the water height at this CELL location (FALSE if no water)
  //
  inline Bool GetWater(S32 cx, S32 cz, F32 *height = NULL)
  {
    return Terrain::GetWater( cx, cz, height);
  }

  // Get a particular cell (using cell coords)
  inline Cell & GetCell(U32 x, U32 z)
  {
    #ifdef DEVELOPMENT

    if (x >= WorldCtrl::CellMapX() || z >= WorldCtrl::CellMapZ())
    {
      ERR_FATAL
      ((
        "Invalid cell access (%u, %u)(%u, %u)", x, z, WorldCtrl::CellMapX(), WorldCtrl::CellMapZ()
      ));
    }

    #endif

    return (cellMap[z * WorldCtrl::CellMapX() + x]);
  } 

  // Get a particular cell (using offset)
  inline Cell & GetCell(U32 offset)
  {
    #ifdef DEVELOPMENT

    if (offset >= WorldCtrl::CellMapX() * WorldCtrl::CellMapZ())
    {
      ERR_FATAL
      ((
        "Invalid cell access (%u)(%u)", offset, WorldCtrl::CellMapX() * WorldCtrl::CellMapZ()
      ));
    }

    #endif

    return (cellMap[offset]);
  } 

  // Get a particular cell (using metres)
  inline Cell &GetCellAtLocation(F32 x, F32 z)
  {
    return (GetCell(WorldCtrl::MetresToCellX(x), WorldCtrl::MetresToCellZ(z)));
  }

  // Get the terrain cell map
  inline Cell * GetCellMap()
  {
    return (cellMap);
  } 

  // Should a second footprinted layer be used on this cell (map location)
  inline Bool UseSecondLayer(U32 x, U32 z)
  {
    ASSERT(WorldCtrl::CellOnMap(x, z))
    return (bitArraySecondLayer->Get2(x, z));
  }

  // Should a second footprinted layer be used on this cell (map offset)
  inline Bool UseSecondLayer(U32 offset)
  {
    return (bitArraySecondLayer->Get1(offset));
  }

  // Does a building block LOS at this cell
  inline Bool BlockLOS(U32 x, U32 z)
  {
    ASSERT(WorldCtrl::CellOnMap(x, z))
    return (bitArrayBlockLOS->Get2(x, z));
  }

  // including footprinted objects
  //
  inline void CellHeights(U32 x, U32 z, F32 *heights)
  {
    ASSERT(WorldCtrl::CellOnMap(x, z));

    if (UseSecondLayer(x, z))
    {
      FootPrint::CellHeights( (S32)x, (S32)z, heights);
    }
    else
    {
      Terrain::CellHeights(x, z, heights);
    }
  }

  // ignoring footprinted objects
  //
  inline F32 FindFloorLower( F32 x, F32 z, Vector *surfNormal = NULL)
  {
    return Terrain::FindFloor(x, z, surfNormal);
  }
  inline F32 FindFloorLowerWithWater( F32 x, F32 z, Vector *surfNormal = NULL)
  {
    return Terrain::FindFloorWithWater(x, z, surfNormal);
  }

  // terrain overloads that include footprinted objects
  //
  inline Bool BoundsTestShadow( MeshEnt & ent)
  {
    return Terrain::BoundsTestShadow( ent, GetHeight, FindFloor);
  }
  inline Bucket * RenderShadowWithWater( MeshEnt & ent)
  {
    return Terrain::RenderShadow( ent, GetHeightWithWater);
  }

  inline Bucket * RenderShadow( const ShadowInfo & si)
  {
    return Terrain::RenderShadow( si, GetHeight);
  }
  inline Bool BoundsTestShadowWithWater( MeshEnt & ent)
  {
    return Terrain::BoundsTestShadow( ent, GetHeightWithWater, FindFloorWithWater);
  }
  inline Bucket * RenderShadowWithWater( const ShadowInfo & si)
  {
    return Terrain::RenderShadow( si, GetHeightWithWater);
  }

  // terrain overloads that include footprinted objects
  //
  inline Bool BoundsTestLight( ShadowInfo & si, const Vid::Light::Obj & light)
  {
    return Terrain::BoundsTestLight( si, light, GetHeight, FindFloor);
  }
  inline Bucket * RenderLight( const Vid::Light::Obj &light)
  {
    return Terrain::RenderLight( light, GetHeight, FindFloor);
  }
  inline Bool BoundsTestLightWithWater( ShadowInfo & si, const Vid::Light::Obj & light)
  {
    return Terrain::BoundsTestLight( si, light, GetHeightWithWater, FindFloorWithWater);
  }
  inline Bucket * RenderLightWithWater( const Vid::Light::Obj & light)
  {
    return Terrain::RenderLight( light, GetHeightWithWater, FindFloorWithWater);
  }

  inline Bucket * RenderGroundSprite( const Vector &origin, F32 radx, F32 rady, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0)
  {
    return Terrain::RenderGroundSprite( origin, radx, rady, texture, color, blend, uv0, uv1, uv2, GetHeight, sorting);
  }
  inline Bucket * RenderGroundSprite( const Vector &origin, F32 radx, F32 rady, U32 clipFlags, F32 viewz, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF,
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0)
  {
    return Terrain::RenderGroundSprite( origin, radx, rady, clipFlags, viewz, texture, color, blend, uv0, uv1, uv2, GetHeight, sorting);
  }
  inline Bucket * RenderGroundSprite( const Vector &origin, F32 radx, F32 rady, const Vector &front, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0)
  {
    return Terrain::RenderGroundSprite( origin, radx, rady, front, texture, color, blend, uv0, uv1, uv2, GetHeight, sorting);
  }

  inline Bucket * RenderGroundSpriteWithWater( const Vector &origin, F32 radx, F32 rady, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return Terrain::RenderGroundSprite( origin, radx, rady, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  }
  inline Bucket * RenderGroundSpriteWithWater( const Vector &origin, F32 radx, F32 rady, U32 clipFlags, F32 viewz, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return Terrain::RenderGroundSprite( origin, radx, rady, clipFlags, viewz, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  }
  inline Bucket * RenderGroundSpriteWithWater( const Vector &origin, F32 radx, F32 rady, const Vector &front, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return Terrain::RenderGroundSprite( origin, radx, rady, front, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  }

  inline Bucket * RenderGroundSprite( const Vector &origin, F32 rad, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0)
  {
    return Terrain::RenderGroundSprite( origin, rad, rad, texture, color, blend, uv0, uv1, uv2, GetHeight, sorting);
  }
  inline Bucket * RenderGroundSprite( const Vector &origin, F32 rad, U32 clipFlags, F32 viewz, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF,
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0)
  {
    return Terrain::RenderGroundSprite( origin, rad, rad, clipFlags, viewz, texture, color, blend, uv0, uv1, uv2, GetHeight, sorting);
  }
  inline Bucket * RenderGroundSprite( const Vector &origin, F32 rad, const Vector &front, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0)
  {
    return Terrain::RenderGroundSprite( origin, rad, rad, front, texture, color, blend, uv0, uv1, uv2, GetHeight, sorting);
  }

  inline Bucket * RenderGroundSpriteWithWater( const Vector &origin, F32 rad, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return Terrain::RenderGroundSprite( origin, rad, rad, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  }
  inline Bucket * RenderGroundSpriteWithWater( const Vector &origin, F32 rad, U32 clipFlags, F32 viewz, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return Terrain::RenderGroundSprite( origin, rad, rad, clipFlags, viewz, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  }
  inline Bucket * RenderGroundSpriteWithWater( const Vector &origin, F32 rad, const Vector &front, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return Terrain::RenderGroundSprite( origin, rad, rad, front, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  }

  inline Bool Intersect( Vector &pos, Vector front, F32 stepScale = 1.0f)
  {
    return Terrain::Intersect( pos, front, stepScale, FindFloor);
  }
  inline Bool ScreenToTerrain( S32 sx, S32 sy, Vector &pos)
  {
    return Terrain::ScreenToTerrain( sx, sy, pos, FindFloor);
  }

  inline Bool IntersectWithWater( Vector &pos, Vector front, F32 stepScale = 1.0f)
  {
    return Terrain::Intersect( pos, front, stepScale, FindFloorWithWater);
  }
  inline Bool ScreenToTerrainWithWater( S32 sx, S32 sy, Vector &pos)
  {
    return Terrain::ScreenToTerrain( sx, sy, pos, FindFloorWithWater);
  }
}

#endif