///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 03-JUN-1998
//

#ifndef __WORLDCTRL_H
#define __WORLDCTRL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "blockfile.h"
#include "ai_map.h"
#include "mapobjdec.h"
#include "unitobjdec.h"
#include "resourceobj.h"
#include "mathtypes.h"

///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Cell size definitions
const U32 WC_CELLSIZE             = 8;
const F32 WC_CELLSIZEF32          = (F32)WC_CELLSIZE;
const F32 WC_CELLSIZEF32INV       = 1.0F/WC_CELLSIZEF32;

// Cluster size definitions
const U32 WC_CLUSTERSIZECELLSHIFT = 2;
const U32 WC_CLUSTERSIZEINCELLS   = (1 << WC_CLUSTERSIZECELLSHIFT);
const F32 WC_CLUSTERSIZEINCELLSF  = (F32)WC_CLUSTERSIZEINCELLS;
const F32 WC_CLUSTERSIZE          = WC_CLUSTERSIZEINCELLSF * WC_CELLSIZEF32;
const F32 WC_CLUSTERSIZEINV       = 1 / (WC_CLUSTERSIZE);
const F32 WC_CLUSTERSIZEHALF      = WC_CLUSTERSIZE / 2;
const F32 WC_CLUSTERRADIUS2       = WC_CLUSTERSIZEHALF * WC_CLUSTERSIZEHALF * 2;
const F32 WC_CLUSTERRADIUS        = F32(sqrt(WC_CLUSTERRADIUS2));

const U32 WC_CLUSTERCELLSHIFT     = 2;
const U32 WC_CLUSTERCELLMASK      = 0x3;


///////////////////////////////////////////////////////////////////////////////
//
// Struct MapCluster - A single gameplay map cluster
//
struct MapCluster
{
  // Extents of cluster
  F32 x0, x1, z0, z1;

  // Index of MapObj ClusterNode that this cluster uses
  U32 nodeIndex;

  // x- and z- index of this cluster in the cluster array
  U32 xIndex, zIndex;

  // AI information for this cluster
  AI::Map::Cluster ai;

  // List of ALL non-bullet map objects occupying this cluster
  NList<MapObj> listObjs;

  // List of units centred in this cluster
  NList<UnitObj> unitList;

  // List of all resources centred in this cluster
  NList<ResourceObj> resourceList;

  // Constructor
  MapCluster();

  // Return the centre of the cluster
  Point<F32> GetMidPoint()
  {
    return (Point<F32>((x0 + x1) * 0.5f, (z0 + z1) * 0.5f));
  }

  // Return the index
  inline U32 GetIndex();

  // Return the next cluster
  inline MapCluster * GetNext();

  // Returns the previous X cluster
  inline MapCluster * GetPreviousX();

  // Returns the next X cluster
  inline MapCluster * GetNextX();

  // Returns the previous Z cluster
  inline MapCluster * GetPreviousZ();

  // Returns the next Z cluster
  inline MapCluster * GetNextZ();

};




///////////////////////////////////////////////////////////////////////////////
//
// Class WorldCtrl - Manages the game-play map data
//

class WorldCtrl
{
private:

  // Is the system initialized
  static Bool sysInit;

  // Is the world data initialized
  static Bool worldInitialized;

  // Map size in metres
  static F32 metreMapX;
  static F32 metreMapZ;
  static F32 metreMapDiag2;
  static F32 metreMapDiag2Inv;
  static F32 metreMapDiag;
  static F32 metreMapDiagInv;
  static F32 metreMapArea;
  static F32 metreMapAreaInv;

  // Map extents in metres
  static Area<F32> mapArea;
  static Area<F32> playField;
  static Area<U32> playFieldCells;

  // Game cell data
  static U32 cellMapX;
  static U32 cellMapZ;
  static F32 cellArea;
  static F32 cellAreaInv;

  // Game cluster data
  static U32 clusterMapX;
  static U32 clusterMapZ;
  static U32 clusterCount;
  static F32 clusterArea;
  static F32 clusterAreaInv;

  // Default map sizes
  static U32 defaultCellMapX;
  static U32 defaultCellMapZ;

  // Minimap rotation
  static F32 miniMapRotation;

  // The cluster based map
  static MapCluster *clusterMap;

private:

  // Setup the world values
  static void SetupWorldData();

  // Allocate the cluster map
  static void AllocateClusterMap();

  // Allocate and release world data structures
  static void AllocateWorld();
  static void ReleaseWorld();

public:

  // Compass directions (values are significant, do not change)
  enum CompassDir 
  { 
    NORTH = 0, 
    EAST  = 1, 
    SOUTH = 2, 
    WEST  = 3
  };

  // Returns the name of the given direction
  static const char * GetCompassDirName(CompassDir d);

  // Inc/Dec a compass direction
  static CompassDir SlideCompassDir(CompassDir d, Bool inc = TRUE);

  // Returns a compass direction for 'v' (must have no y component)
  static CompassDir GetCompassDirection(const Vector &v);

  // Returns a compass direction from an angle
  static CompassDir GetCompassDirection(F32 angle);

  // Convert a compass angle into an angle
  static F32 GetCompassAngle(F32 angle);
  
  // Convert an angle into a compass angle
  static F32 GetAngle(F32 compass);

  // Convert a compass angle into an angle
  static F32 GetCompassAngle(CompassDir dir);

  // Sets the vector 'v' to point in the given compass direction
  static void SetFromCompassDirection(Vector &v, CompassDir dir);

  // Setup a matrix using a position and direction
  static void SetupWorldMatrix(Matrix &m, const Vector &pos, CompassDir dir);

  // Snap matrix rotation
  static CompassDir SnapRotation(Matrix &m);

  // Is the given world size valid
  static Bool CheckWorldSize(U32 x, U32 z);

  // Initialize and shutdown system
  static void Init();
  static void Done();

  // Load active mission, or create new world using default values
  static void CreateWorld();

  // Load optional playfield information
  static void InitPlayField();

  // Save current world data, FALSE on error
  static Bool SaveWorld();

  // Load world information
  static void LoadInfo(FScope *fScope);

  // Save current world information
  static void SaveInfo(FScope *fScope);

public:

  // Is the world data currently initialized
  static Bool WorldInitialized()
  {
    return (worldInitialized);
  }

  // Map size in metres
  static F32 MetreMapX() { return (metreMapX); }
  static F32 MetreMapZ() { return (metreMapZ); }
  static F32 MetreMapDiag2() { return (metreMapDiag2); }
  static F32 MetreMapDiag2Inv() { return (metreMapDiag2Inv); }
  static F32 MetreMapDiag() { return (metreMapDiag); }
  static F32 MetreMapDiagInv() { return (metreMapDiagInv); }
  static F32 MetreMapArea() { return (metreMapArea); }
  static F32 MetreMapAreaInv() { return (metreMapAreaInv); }

  // Map extents in metres
  static F32 MetreMapXMin() { return (mapArea.p0.x); }
  static F32 MetreMapXMax() { return (mapArea.p1.x); }
  static F32 MetreMapZMin() { return (mapArea.p0.z); }
  static F32 MetreMapZMax() { return (mapArea.p1.z); }

  // Game cell size (in metres) and the size of the map in game cells
  static F32 CellSize() { return (WC_CELLSIZEF32); }
  static U32 CellMapX() { return (cellMapX); }
  static U32 CellMapZ() { return (cellMapZ); }
  static F32 CellArea() { return (cellArea); }
  static F32 CellAreaInv() { return (cellAreaInv); }

  // Minimap rotation
  static F32 MiniMapRotation() { return (miniMapRotation); }

  // Game cluster size (in metres) and the size of the map in game clusters
  static U32 ClusterMapX() { return (clusterMapX); }
  static U32 ClusterMapZ() { return (clusterMapZ); }
  static U32 ClusterCount() { return (clusterCount); }
  static F32 ClusterSize() { return (WC_CLUSTERSIZE); }
  static F32 ClusterSizeInv() { return (WC_CLUSTERSIZEINV); }
  static U32 ClusterSizeInCells() { return (WC_CLUSTERSIZEINCELLS); }
  static F32 ClusterSizeHalf() { return (WC_CLUSTERSIZEHALF); }
  static F32 ClusterRadius() { return (WC_CLUSTERRADIUS); }
  static F32 ClusterRadius2() { return (WC_CLUSTERRADIUS2); }
  static F32 ClusterArea() { return (clusterArea); }
  static F32 ClusterAreaInv() { return (clusterAreaInv); }

  // Returns TRUE if the specified cell position is on the map
  static Bool CellOnMapX(U32 x) { return (x < cellMapX); }
  static Bool CellOnMapZ(U32 z) { return (z < cellMapZ); }

  // Returns TRUE if the specified game cell is on the map
  static Bool CellOnMap(U32 x, U32 z) 
  { 
    return (CellOnMapX(x) && CellOnMapZ(z)); 
  }

  // Returns TRUE if the specified cell position is on the map
  static Bool CellOnMapX(S32 x) { return (x >= 0 && x < (S32)cellMapX); }
  static Bool CellOnMapZ(S32 z) { return (z >= 0 && z < (S32)cellMapZ); }

  // Returns TRUE if the specified game cell is on the map
  static Bool CellOnMap(S32 x, S32 z)
  { 
    return (CellOnMapX(x) && CellOnMapZ(z)); 
  } 

  // Returns TRUE if the specified cell position is on the map
  static Bool CellOnMapXNoEdge(S32 x) { return (x >= 1 && x < (S32)(cellMapX - 1)); }
  static Bool CellOnMapZNoEdge(S32 z) { return (z >= 1 && z < (S32)(cellMapZ - 1)); }

  // Returns TRUE if the specified game cell is on the map
  static Bool CellOnMapNoEdge(S32 x, S32 z)
  { 
    return (CellOnMapXNoEdge(x) && CellOnMapZNoEdge(z)); 
  } 
  
  // Clamp the X value to a position on the map
  static S32 ClampCellMapX(S32 x)
  {
    return (Max<S32>(0, Min<S32>(x, cellMapX - 1)));
  }

  // Clamp the Z value to a position on the map
  static S32 ClampCellMapZ(S32 z)
  {
    return (Max<S32>(0, Min<S32>(z, cellMapZ - 1)));
  }
  
  // Clamp a cell to on the map
  static const Point<S32> & ClampCellPoint(Point<S32> &p)
  {
    p.x = ClampCellMapX(p.x);
    p.z = ClampCellMapZ(p.z);
    return (p);
  }

  // Returns TRUE if the specified metre position is on the map
  static Bool MetreOnMapX(F32 x) { return (x >= mapArea.p0.x && x < mapArea.p1.x); }
  static Bool MetreOnMapZ(F32 z) { return (z >= mapArea.p0.z && z < mapArea.p1.z); }

  // Returns TRUE if the specified metre position is on the map
  static Bool MetreOnMap(F32 x, F32 z) 
  { 
    return (MetreOnMapX(x) && MetreOnMapZ(z)); 
  }

  // Returns TRUE if the specified point is on the map
  static Bool MetreOnMapPoint(const Point<F32> &val)
  {
    return (MetreOnMapX(val.x) && MetreOnMapZ(val.y));
  }

  // Clamp the X value to a position on the map
  static void ClampMetreMapX(F32 &val)
  {
    if (val >= mapArea.p1.x)
    {
      val = mapArea.p1.x - F32_MIN_MOD;
    }
    else
    if (val < mapArea.p0.x)
    {
      val = mapArea.p0.x;
    }
  }

  // Clamp the Z value to a position on the map
  static void ClampMetreMapZ(F32 &val)
  {
    if (val >= mapArea.p1.z)
    {
      val = mapArea.p1.z - F32_MIN_MOD;
    }
    else
    if (val < mapArea.p0.z)
    {
      val = mapArea.p0.z;
    }
  }

  // Clamp the given values to be on the map
  static void ClampMetreMap(F32 &x, F32 &z)
  {
    ClampMetreMapX(x);
    ClampMetreMapZ(z);
  }

  // Clamp the given point to be on the map
  static void ClampMetreMapPoint(Point<F32> &val)
  {
    ClampMetreMapX(val.x);
    ClampMetreMapZ(val.y);
  }

  // Returns TRUE if the specified metre position is on the playfield
  static Bool MetreOnPlayFieldX(F32 x) { return (x >= playField.p0.x && x < playField.p1.x); }
  static Bool MetreOnPlayFieldZ(F32 z) { return (z >= playField.p0.z && z < playField.p1.z); }

  // Returns TRUE if the specified metre position is on the playfield
  static Bool MetreOnPlayField(F32 x, F32 z) 
  { 
    return (MetreOnPlayFieldX(x) && MetreOnPlayFieldZ(z)); 
  }

  // Returns TRUE if the specified point is on the playfield
  static Bool MetreOnPlayFieldPoint(const Point<F32> &val)
  {
    return (MetreOnPlayFieldX(val.x) && MetreOnPlayFieldZ(val.y));
  }

  // Clamp the X value to a position on the playfield
  static void ClampPlayFieldX(F32 &val)
  {
    if (val >= playField.p1.x)
    {
      val = playField.p1.x - F32_MIN_MOD;
    }
    else
    if (val < playField.p0.x)
    {
      val = playField.p0.x;
    }
  }

  // Clamp the Z value to a position on the playfield
  static void ClampPlayFieldZ(F32 &val)
  {
    if (val >= playField.p1.z)
    {
      val = playField.p1.z - F32_MIN_MOD;
    }
    else
    if (val < playField.p0.z)
    {
      val = playField.p0.z;
    }
  }

  // Clamp the given values to be on the map
  static void ClampPlayField(F32 &x, F32 &z)
  {
    ClampPlayFieldX(x);
    ClampPlayFieldZ(z);
  }

  // Clamp the given point to be on the map
  static void ClampPlayFieldPoint(Point<F32> &val)
  {
    ClampPlayFieldX(val.x);
    ClampPlayFieldZ(val.y);
  }

  // Returns TRUE if the specified cell position is on the playfield
  static Bool CellOnPlayFieldX(U32 x) { return (x >= playFieldCells.p0.x && x < playFieldCells.p1.x); }
  static Bool CellOnPlayFieldZ(U32 z) { return (z >= playFieldCells.p0.z && z < playFieldCells.p1.z); }

  // Returns TRUE if the specified cell position is on the playfield
  static Bool CellOnPlayField(U32 x, U32 z) 
  { 
    return (CellOnPlayFieldX(x) && CellOnPlayFieldZ(z)); 
  }

  // Returns TRUE if the specified cell is on the playfield
  static Bool CellOnPlayFieldPoint(const Point<U32> &val)
  {
    return (CellOnPlayFieldX(val.x) && CellOnPlayFieldZ(val.y));
  }

  // Clamp the X value to a position on the playfield
  static void ClampPlayFieldCellX(U32 &val)
  {
    if (val > playFieldCells.p1.x)
    {
      val = playFieldCells.p1.x;
    }
    else
    if (val < playFieldCells.p0.x)
    {
      val = playFieldCells.p0.x;
    }
  }

  // Clamp the Z value to a position on the playfield
  static void ClampPlayFieldCellZ(U32 &val)
  {
    if (val > playFieldCells.p1.z)
    {
      val = playFieldCells.p1.z;
    }
    else
    if (val < playFieldCells.p0.z)
    {
      val = playFieldCells.p0.z;
    }
  }

  // Clamp the given values to be on the map
  static void ClampPlayFieldCell(U32 &x, U32 &z)
  {
    ClampPlayFieldCellX(x);
    ClampPlayFieldCellZ(z);
  }

  // Clamp the given point to be on the map
  static void ClampPlayFieldPoint(Point<U32> &val)
  {
    ClampPlayFieldCellX(val.x);
    ClampPlayFieldCellZ(val.y);
  }

  // Returns the requested map cluster (args are in game clusters)
  static MapCluster* GetCluster(U32 x, U32 z)
  {
    #ifdef DEVELOPMENT

      if (x >= clusterMapX || z >= clusterMapZ)
      {
        ERR_FATAL(("Invalid cluster map access (%u, %u)(%u, %u)", x, z, clusterMapX, clusterMapZ));
      }

    #endif

    return (&clusterMap[z * clusterMapX + x]);
  }

  // Returns the requested map cluster 
  static MapCluster* GetCluster(U32 index)
  {
    #ifdef DEVELOPMENT

      if (index >= (clusterMapX * clusterMapZ))
      {
        ERR_FATAL(("Invalid cluster map access (%u)(%u, %u)", index, clusterMapX, clusterMapZ));
      }

    #endif

    return (&clusterMap[index]);
  }

  // Return the requested map cluster index
  static U32 GetClusterIndex(U32 x, U32 z)
  {
    return (z * clusterMapX + x);
  }

  // Returns the cell offset for map position
  static U32 MetresToCellX(F32 x)
  {
    ASSERT(MetreOnMapX(x));

    // trunc mode is the default
    //
    return U32(Utils::FtoL(x * WC_CELLSIZEF32INV));
  }

  // Returns the cell offset for map position
  static U32 MetresToCellZ(F32 z)
  {
    ASSERT(MetreOnMapZ(z));

    // trunc mode is the default
    //
    return U32(Utils::FtoL(z * WC_CELLSIZEF32INV));
  }

  // Converts the given point from metres to cells
  static void MetresToCellPoint(const Point<F32> &pos, Point<U32> &val)
  {
    val.x = MetresToCellX(pos.x);
    val.y = MetresToCellZ(pos.y);
  }

  // Returns the metre position for a cell X (centre of cell)
  static F32 CellToMetresX(U32 x)
  {
    ASSERT(CellOnMapX(x));
    return (((F32)x + 0.5f) * WC_CELLSIZEF32);
  }

  // Returns the metre position for a cell Z (centre of cell)
  static F32 CellToMetresZ(U32 z)
  {
    ASSERT(CellOnMapZ(z));
    return (((F32)z + 0.5f) * WC_CELLSIZEF32);
  }

  // Returns the metre point from a cell point
  static void CellToMetrePoint(const Point<U32> &cell, Point<F32> &val)
  {
    ASSERT(CellOnMapX(cell.x))
    ASSERT(CellOnMapZ(cell.z))
    val.x = (F32(cell.x) + 0.5f) * WC_CELLSIZEF32;
    val.z = (F32(cell.z) + 0.5f) * WC_CELLSIZEF32;
  }

  // Returns the X cluster offset for cell position 'x' (no extent checks)
  static U32 CellsToClusterX(U32 x)
  {
    return (x >> WC_CLUSTERCELLSHIFT);
  }

  // Returns the Z cluster offset for cell position 'z' (no extent checks)
  static U32 CellsToClusterZ(U32 z)
  {
    return (z >> WC_CLUSTERCELLSHIFT);
  }

  // Converts the given point from metres to clusters
  static void CellsToClusterPoint(const Point<U32> &pos, Point<U32> &val)
  {
    val.x = CellsToClusterX(pos.x);
    val.y = CellsToClusterZ(pos.y);
  }

  // Returns the requested map cluster (args are in cell co-ordinates)
  static MapCluster* CellsToCluster(U32 x, U32 z)
  {
    return (GetCluster(CellsToClusterX(x), CellsToClusterZ(z)));
  }

  // Returns the cluster index from cell co-orindates
  static U32 CellsToClusterIndex(U32 x, U32 z)
  {
    return ((x >> WC_CLUSTERCELLSHIFT) + (z >> WC_CLUSTERCELLSHIFT) * ClusterMapX());
  }

  // Given a cell, what position is it within a cluster
  static U32 CellToClusterCell(U32 c)
  {
    return (c & WC_CLUSTERCELLMASK);
  }

  // Returns the cell position of the left most X cell (arg is x cluster co-ord)
  static U32 ClusterToLeftCell(U32 x)
  {
    return (x * ClusterSizeInCells());
  }

  // Returns the cell position of the right most X cell (arg is x cluster co-ord)
  static U32 ClusterToRightCell(U32 x)
  {
    return ((x + 1) * ClusterSizeInCells() - 1);
  }

  // Returns the cell position of the top most Z cell (arg is z cluster co-ord)
  static U32 ClusterToTopCell(U32 z)
  {
    return (z * ClusterSizeInCells());
  }

  // Returns the cell position of the bottom most Z cell (arg is z cluster co-ord)
  static U32 ClusterToBottomCell(U32 z)
  {
    return ((z + 1) * ClusterSizeInCells() - 1);
  }

  // Returns the X cluster offset for map position 'x' (no extent checks)
  static U32 MetresToClusterX(F32 x)
  {
    // trunc mode is the default
    //
    return U32(Utils::FtoL(x * ClusterSizeInv()));
  }

  // Returns the Z cluster offset for map position 'z' (no extent checks)
  static U32 MetresToClusterZ(F32 z)
  {
    // trunc mode is the default
    //
    return U32(Utils::FtoL(z * ClusterSizeInv()));
  }

  // Converts the given point from metres to clusters
  static void MetresToClusterPoint(const Point<F32> &pos, Point<U32> &val)
  {
    val.x = MetresToClusterX(pos.x);
    val.y = MetresToClusterZ(pos.y);
  }

  // Returns the requested map cluster (args are in metres)
  static MapCluster* MetresToCluster(F32 x, F32 z)
  {
    return
    (
      GetCluster((U32)Utils::FtoL(x * ClusterSizeInv()), 
                 (U32)Utils::FtoL(z * ClusterSizeInv()))
    );
  }

  // Returns the requested map cluster index (args are in metres)
  static U32 MetresToClusterIndex(F32 x, F32 z)
  {
    return
    (
      GetClusterIndex((U32)Utils::FtoL(x * ClusterSizeInv()), 
                      (U32)Utils::FtoL(z * ClusterSizeInv()))
    );
  }

  // Get the list of clusters which covers the given metre area
  static void BuildClusterList(List<MapCluster> &clusters, const Area<F32> &metres);
  static void BuildClusterList(List<MapCluster> &clusters, const Area<U32> &cells);
  static void BuildClusterList(List<MapCluster> &clusters, const Point<F32> &metres, F32 range);

  // Convert cell location to first grain in that cell
  static void CellToFirstGrain(U32 cx, U32 cz, S32 &gx, S32 &gz)
  {
    gx = cx << 1;
    gz = cz << 1;
  }

  // Convert cell location to last grain in that cell
  static void CellToLastGrain(U32 cx, U32 cz, S32 &gx, S32 &gz)
  {
    gx = (cx << 1) + 1;
    gz = (cz << 1) + 1;
  }


  // Set default map sizes (TRUE if valid)
  static Bool SetDefaultMapSize(U32 cellX, U32 cellZ);

  // MapCluster is a friend of WorldCtrl
  friend struct MapCluster;
};


// Get the index of this cluster
U32 MapCluster::GetIndex()
{
  return (WorldCtrl::GetClusterIndex(xIndex, zIndex));
}

// Return the next cluster
MapCluster * MapCluster::GetNext()
{
  return ((GetIndex() < (WorldCtrl::clusterCount - 1)) ? this + 1 : NULL);
}

// Returns the previous X cluster
MapCluster * MapCluster::GetPreviousX()
{
  return ((xIndex > 0) ? this - 1 : NULL);
}

// Returns the next X cluster
MapCluster * MapCluster::GetNextX()
{
  return ((xIndex < (WorldCtrl::clusterMapX - 1)) ? this + 1 : NULL);
}

// Returns the previous Z cluster
MapCluster * MapCluster::GetPreviousZ()
{
  return ((zIndex > 0) ? this - WorldCtrl::clusterMapX : NULL);
}

// Returns the next Z cluster
MapCluster * MapCluster::GetNextZ()
{
  return ((zIndex < (WorldCtrl::clusterMapZ - 1)) ? this + WorldCtrl::clusterMapX : NULL);
}


#endif