///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 03-JUN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "worldctrl.h"
#include "console.h"
#include "missions.h"
#include "gameobjctrl.h"
#include "worldload.h"
#include "terrain.h"
#include "perfstats.h"
#include "stdload.h"
#include "terraindata.h"
#include "claim.h"
#include "unitobj.h"
#include "regionobj.h"
#include "gamegod.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Block file keys
#define WC_KEY_MAPDATA   "gamemapdata"


///////////////////////////////////////////////////////////////////////////////
//
// Class MapCluster
//

//
// Constructor
//
MapCluster::MapCluster() : 
  unitList(&UnitObj::unitNode), 
  resourceList(&ResourceObj::resourceNode)
{
}


///////////////////////////////////////////////////////////////////////////////
//
// Class WorldCtrl - Manages the game-play map data
//

// 
// Static data
//
Bool WorldCtrl::sysInit = FALSE;
Bool WorldCtrl::worldInitialized;

F32 WorldCtrl::metreMapX;
F32 WorldCtrl::metreMapZ;
F32 WorldCtrl::metreMapDiag2;
F32 WorldCtrl::metreMapDiag2Inv;
F32 WorldCtrl::metreMapDiag;
F32 WorldCtrl::metreMapDiagInv;
F32 WorldCtrl::metreMapArea;
F32 WorldCtrl::metreMapAreaInv;

/*
F32 WorldCtrl::metreMapXMin;
F32 WorldCtrl::metreMapXMax;
F32 WorldCtrl::metreMapZMin;
F32 WorldCtrl::metreMapZMax;
*/

Area<F32> WorldCtrl::mapArea;
Area<F32> WorldCtrl::playField;
Area<U32> WorldCtrl::playFieldCells;

U32 WorldCtrl::cellMapX;
U32 WorldCtrl::cellMapZ;
F32 WorldCtrl::cellArea;
F32 WorldCtrl::cellAreaInv;

U32 WorldCtrl::clusterMapX;
U32 WorldCtrl::clusterMapZ;
U32 WorldCtrl::clusterCount;
F32 WorldCtrl::clusterArea;
F32 WorldCtrl::clusterAreaInv;

U32 WorldCtrl::defaultCellMapX = 128;
U32 WorldCtrl::defaultCellMapZ = 128;

F32 WorldCtrl::miniMapRotation = 315.0F;

MapCluster *WorldCtrl::clusterMap;


//
// SetupWorldData
//
// Setup the world values
//
void WorldCtrl::SetupWorldData()
{
  ASSERT(sysInit);

  // Ensure values are valid
  if (!CheckWorldSize(cellMapX, cellMapZ))
  {
    ERR_FATAL(("Supplied world size is invalid! (%u x %u)", cellMapX, cellMapZ));
  }

  // Calculate map sizes
  metreMapX = cellMapX * CellSize();
  metreMapZ = cellMapZ * CellSize();

  // Calculate map diagonal
  metreMapDiag2 = metreMapX * metreMapX + metreMapZ * metreMapZ;
  metreMapDiag2Inv = 1.0F / metreMapDiag2;

  metreMapDiag = F32(sqrt(metreMapDiag2));
  metreMapDiagInv = 1.0F / metreMapDiag;

  // Calculate map area
  metreMapArea = metreMapX * metreMapZ;
  metreMapAreaInv = 1.0F / metreMapArea;

  // Calculate map extents
  mapArea.p0.x = 0.0F;
  mapArea.p0.z = 0.0F;
  mapArea.p1.x = mapArea.p0.x + metreMapX;
  mapArea.p1.z = mapArea.p0.z + metreMapZ;

  // Initialise playfield defaults
  playField.p0 = mapArea.p0 + (3.0F * WC_CELLSIZEF32) + 0.1F;
  playField.p1 = mapArea.p1 - (3.0F * WC_CELLSIZEF32) - 0.1F;

  MetresToCellPoint(playField.p0, playFieldCells.p0);
  MetresToCellPoint(playField.p1, playFieldCells.p1);

  // Calculate cell information
  cellArea = WC_CELLSIZE * WC_CELLSIZE;
  cellAreaInv = 1 / cellArea;
}


//
// AllocateClusterMap
//
// Allocate the cluster based map
//
void WorldCtrl::AllocateClusterMap()
{
  ASSERT(sysInit);

  // Calculate extended map information
  clusterMapX = ((cellMapX + ClusterSizeInCells()) - 1) / ClusterSizeInCells();
  clusterMapZ = ((cellMapZ + ClusterSizeInCells()) - 1) / ClusterSizeInCells();
  clusterCount = clusterMapX * clusterMapZ;
  clusterArea = WC_CLUSTERSIZE * WC_CLUSTERSIZE;
  clusterAreaInv = 1 / clusterArea;

  // Allocate the map
  clusterMap = new MapCluster[clusterMapX * clusterMapZ];

  // Set up each cluster's information
  for (U32 z = 0; z < clusterMapZ; z++)
  {
    for (U32 x = 0; x < clusterMapX; x++)
    {
      MapCluster *thisCluster = GetCluster(x, z);

      // Set the array index of this cluster to avoid having to recalculate it
      thisCluster->xIndex = x;
      thisCluster->zIndex = z;

      // Set the extents of this cluster
      thisCluster->x0 = F32(x) * ClusterSize();
      thisCluster->z0 = F32(z) * ClusterSize();
      thisCluster->x1 = thisCluster->x0 + ClusterSize();
      thisCluster->z1 = thisCluster->z0 + ClusterSize();

      // Set up the cluster NList node member offset
      thisCluster->nodeIndex = (x & 1) ? ((z & 1) ? 3 : 1) : ((z & 1) ? 2 : 0);

      switch (thisCluster->nodeIndex)
      {
        case 0: 
          thisCluster->listObjs.SetNodeMember(&MapObj::clustNode0); 
          break;

        case 1: 
          thisCluster->listObjs.SetNodeMember(&MapObj::clustNode1); 
          break;

        case 2: 
          thisCluster->listObjs.SetNodeMember(&MapObj::clustNode2);
          break;

        case 3: 
          thisCluster->listObjs.SetNodeMember(&MapObj::clustNode3);
          break;

        default:
          ERR_FATAL(("NodeIndex[%d] is not valid", thisCluster->nodeIndex));
      }
    }
  }
}


//
// AllocateWorld
//
// Initialize world data using current terrain values
//
void WorldCtrl::AllocateWorld()
{
  ASSERT(sysInit);

  // Release any existing world data
  ReleaseWorld();

  // Setup the world values
  SetupWorldData();

  // Allocate the cluster based map
  AllocateClusterMap();

  // Initialize the game-play terrain system
  TerrainData::Init();

  // Initialize the claiming system
  Claim::Init();

  // Flag world initialized
  worldInitialized = TRUE;
}


//
// ReleaseWorld
//
// Release current world data
//
void WorldCtrl::ReleaseWorld()
{
  ASSERT(sysInit);

  // Do we currently have a world loaded
  if (worldInitialized)
  {
    // Shutdown the claiming system
    Claim::Done();

    // Shutdown the game-play terrain system
    TerrainData::Done();

    // Delete the maps
    delete [] clusterMap;

    // Flag world released
    worldInitialized = FALSE;
  }
}  


//
// GetCompassDirName
//
// Returns the name of the given direction
//
const char * WorldCtrl::GetCompassDirName(CompassDir d)
{
  switch (d)
  {
    case WorldCtrl::NORTH : return ("North");
    case WorldCtrl::EAST  : return ("East");
    case WorldCtrl::SOUTH : return ("South");
    case WorldCtrl::WEST  : return ("West");
  } 

  return ("Invalid");
}


//
// SlideCompassDir
//
// Inc/Dec a compass direction
//
WorldCtrl::CompassDir WorldCtrl::SlideCompassDir(CompassDir d, Bool inc)
{
  if (inc)
  {
    switch (d)
    {
      case WorldCtrl::NORTH : return (WorldCtrl::EAST);
      case WorldCtrl::EAST  : return (WorldCtrl::SOUTH);
      case WorldCtrl::SOUTH : return (WorldCtrl::WEST);
      case WorldCtrl::WEST  : return (WorldCtrl::NORTH);
    }
  }
  else
  {
    switch (d)
    {
      case WorldCtrl::NORTH : return (WorldCtrl::WEST);
      case WorldCtrl::WEST  : return (WorldCtrl::SOUTH);
      case WorldCtrl::SOUTH : return (WorldCtrl::EAST);
      case WorldCtrl::EAST  : return (WorldCtrl::NORTH);
    }
  }

  // Unknown direction
  return (d);
}


//
// GetCompassDirection
//
// Returns a compass direction for 'v' (must have no y component)
//
WorldCtrl::CompassDir WorldCtrl::GetCompassDirection(const Vector &v)
{
  ASSERT(!v.y);

  // Check north and south
  F32 dot = v.Dot(Matrix::I.front);

  if (dot >= 0.7F)
  {
    return (NORTH);
  }
  
  if (dot <= -0.7F)
  {
    return (SOUTH);
  }

  // Check east and west
  dot = v.Dot(Matrix::I.right);

  if (dot >= 0.7F)
  {
    return (EAST);
  }
  
  return (WEST);
}


//
// Convert a compass angle into an angle
//
F32 WorldCtrl::GetCompassAngle(F32 angle)
{
  return (PIBY2 - angle);
}


//
// Convert an angle into a compass angle
//
F32 WorldCtrl::GetAngle(F32 compass)
{
  return (PIBY2 - compass);
}


//
// GetCompassDirection
//
// Returns a compass direction from an angle
//
WorldCtrl::CompassDir WorldCtrl::GetCompassDirection(F32 angle)
{
  VectorDir::FixU(angle);

  if (angle > PI * 0.25f && angle < PI * 0.75f)
  {
    return (NORTH);
  }
  if (angle < PI * -0.25f && angle > PI * -0.75f)
  {
    return (SOUTH);
  }
  if (angle < PI * 0.25f && angle > PI * -0.25f)
  {
    return (EAST);
  }
  return (WEST);
}


//
// GetCompassAngle
//
// Return the angle from a compass direction
//
F32 WorldCtrl::GetCompassAngle(CompassDir dir)
{
  switch (dir)
  {
    case NORTH:
      return (0);
      break;

    case EAST:
      return (PIBY2);
      break;

    case SOUTH:
      return (PI);
      break;

    case WEST:
      return (-PIBY2);
      break;

    default:
      ERR_FATAL(("Unkown compass dir %d", dir))
  }
}


//
// SetFromCompassDirection
//
// Sets the vector 'v' to point in the given compass direction
//
void WorldCtrl::SetFromCompassDirection(Vector &v, CompassDir dir)
{
  // Remove the y component
  v.y = 0;

  // Set the x and z based on the direction
  switch (dir)
  {
    case NORTH: v.x = 0.0f; v.z = 1.0f; break;
    case EAST: v.x = 1.0f; v.z = 0.0f; break;
    case SOUTH: v.x = 0.0f; v.z = -1.0f; break;
    case WEST: v.x = -1; v.z = 0; break;
  }
}


//
// SetupWorldMatrix
//
// Setup a matrix using a position and direction
//
void WorldCtrl::SetupWorldMatrix(Matrix &m, const Vector &pos, CompassDir dir)
{
  // Work out the front vector using the given direction
  Vector front;
  WorldCtrl::SetFromCompassDirection(front, dir);

  // Setup the matrix at this position
  m.ClearData();
  m.posit = pos;
  m.front = front;
//  m.up.Cross(m.right, m.front); 
  m.right = Cross(m.up, m.front); 
}


//
// SnapRotation
//
// Snap matrix rotation
//
WorldCtrl::CompassDir WorldCtrl::SnapRotation(Matrix &m)
{
  // Make sure matrix is horizontal
  m.front.y = 0;
  m.front.Normalize();
  m.up = Matrix::I.up;

  // Get closest compass direction
  CompassDir dir = WorldCtrl::GetCompassDirection(m.front);

  // Adjust front
  switch (dir)
  {
    case WorldCtrl::NORTH : m.front = Matrix::I.front; break;
    case WorldCtrl::SOUTH : m.front = Matrix::I.front * -1.0F; break;
    case WorldCtrl::EAST  : m.front = Matrix::I.right; break;
    case WorldCtrl::WEST  : m.front = Matrix::I.right * -1.0F; break;
  }

  // Adjust right 
  m.right = Cross(m.up, m.front);

  // Return the compass direction
  return (dir);
}


//
// CheckWorldSize
//
// Is the given world size valid
//
Bool WorldCtrl::CheckWorldSize(U32 x, U32 z)
{
  return (x >= 8 && x <= 1024 && z >= 8 && z <= 1024);
}


//
// Init
//
// Initialize system
//
void WorldCtrl::Init()
{
  ASSERT(!sysInit);

  // No world data
  worldInitialized = FALSE;
 
  // System now initialized
  sysInit = TRUE;
}


//
// Done
//
// Shutdown system
//
void WorldCtrl::Done()
{
  ASSERT(sysInit);

  // Release any world data
  ReleaseWorld();

  // System now shutdown
  sysInit = FALSE;
}


//
// CreateWorld
//
// Load active mission, or create new world using default values
//
void WorldCtrl::CreateWorld()
{
  ASSERT(sysInit);

  BlockFile bFile;
  PathString fileId;

  GameGod::Loader::SubSystem("#game.loader.terrain", 4);

  // Is there an active mission
  const Missions::Mission *mission = Missions::GetActive();

  GameGod::Loader::Advance();

  // Open the terrain file
  if (mission && !bFile.Open(Game::FILENAME_MISSION_TERRAIN, BlockFile::READ, FALSE))
  {
    FileSys::LogAllSources();
    ERR_FATAL((bFile.LastError()));
  }

  // If not loading from disk, set default values
  if (!mission)
  {
    cellMapX = defaultCellMapX;
    cellMapZ = defaultCellMapZ;
  }

  // Setup terrain using current values
  Terrain::SetBaseTexName( TerrainGroup::ActiveGroup().BaseType()->BlendName(0, 0));
  Terrain::Setup(cellMapX, cellMapZ, WC_CELLSIZE);

  GameGod::Loader::Advance();

  // Read the possibly adjusted values from the terrain
  cellMapX = Terrain::CellWidth();
  cellMapZ = Terrain::CellHeight();

  // Initialize gameplay world data
  AllocateWorld();

  GameGod::Loader::Advance();

  // Now load the actual saved world data
  if (mission)
  {
    // Load the low-level terrain data
    Terrain::Load(bFile);

    // Load the terrain type data
    TerrainData::Load(bFile);
  }
  else
  {
    // Setup extending terrain information for default map
    TerrainData::SetupExtendedInfo();
  }

  GameGod::Loader::Advance();

  // Close the file
  if (mission)
  {
    bFile.Close();
  }
}


//
// InitPlayField
//
// Load optional playfield information
//
void WorldCtrl::InitPlayField()
{
  // Initialise camera region
  RegionObj *region = RegionObj::FindRegion("Camera::Bounds");

  if (region)
  {
    Area<F32> newPlayField = region->GetArea();

    playField.p0.x = Max<F32>(playField.p0.x, newPlayField.p0.x);
    playField.p0.z = Max<F32>(playField.p0.z, newPlayField.p0.z);
    playField.p1.x = Min<F32>(playField.p1.x, newPlayField.p1.x);
    playField.p1.z = Min<F32>(playField.p1.z, newPlayField.p1.z);

    MetresToCellPoint(playField.p0, playFieldCells.p0);
    MetresToCellPoint(playField.p1, playFieldCells.p1);

    /*
    LOG_DIAG
    ((
      "Loading play field [%.1f,%.1f,%.1f,%.1f]", 
      playField.p0.x, playField.p0.z, playField.p1.x, playField.p1.z
    ))
    */
  }
}


//
// SaveWorld
//
// Save current world data, FALSE on error
//
Bool WorldCtrl::SaveWorld()
{
  ASSERT(sysInit);
  ASSERT(worldInitialized);

  BlockFile bFile;
  PathString fileId;
  FilePath path;

  // Generate path to terrain file
  Dir::PathMake(path, Missions::GetWritePath(), Game::FILENAME_MISSION_TERRAIN);

  // And attempt to create it
  if (!bFile.Open(path.str, BlockFile::CREATE, FALSE))
  {
    LOG_ERR((bFile.LastError()));
    CON_ERR((bFile.LastError()))
    return (FALSE);
  }

  // Save the 3D engine terrain data
  Terrain::Save(bFile);

  // Save terrain type data
  TerrainData::Save(bFile);

  // Close the blockfile
  bFile.CloseBlock();
  bFile.Close();

  return (TRUE);
}


//
// SaveWorld
//
// Load world information
//
void WorldCtrl::LoadInfo(FScope *fScope)
{
  // Load map size
  cellMapX = StdLoad::TypeU32(fScope, "CellMapX");
  cellMapZ = StdLoad::TypeU32(fScope, "CellMapZ");
  miniMapRotation = StdLoad::TypeF32(fScope, "MiniMapRotation", 315.0F);
}


//
// SaveWorld
//
// Save current world information
//
void WorldCtrl::SaveInfo(FScope *fScope)
{
  // Save map header
  StdSave::TypeU32(fScope, "CellMapX", CellMapX());
  StdSave::TypeU32(fScope, "CellMapZ", CellMapZ());
  StdSave::TypeF32(fScope, "MiniMapRotation", MiniMapRotation());
}


//
// BuildClusterList
//
void WorldCtrl::BuildClusterList(List<MapCluster> &clusters, const Area<F32> &metres)
{
  clusters.UnlinkAll();

  Point<U32> ctl;
  WorldCtrl::MetresToClusterPoint(metres.p0, ctl);

  Point<U32> cbr;
  WorldCtrl::MetresToClusterPoint(metres.p1, cbr);

  for (U32 cy = ctl.y; cy <= cbr.y; cy++)
  {
    for (U32 cx = ctl.x; cx <= cbr.x; cx++)
    {
      clusters.Append(WorldCtrl::GetCluster(cx, cy));
    }
  }
}


//
// BuildClusterList
//
void WorldCtrl::BuildClusterList(List<MapCluster> &clusters, const Area<U32> &cells)
{
  clusters.UnlinkAll();

  Point<U32> ctl;
  WorldCtrl::CellsToClusterPoint(cells.p0, ctl);

  Point<U32> cbr;
  WorldCtrl::CellsToClusterPoint(cells.p1, cbr);

  for (U32 cy = ctl.y; cy <= cbr.y; cy++)
  {
    for (U32 cx = ctl.x; cx <= cbr.x; cx++)
    {
      clusters.Append(WorldCtrl::GetCluster(cx, cy));
    }
  }
}


//
// BuildClusterList
//
void WorldCtrl::BuildClusterList(List<MapCluster> &clusters, const Point<F32> &point, F32 range)
{
  clusters.UnlinkAll();

  // Top Left & Bottom Right (metres)
  Point<F32> tl(point.x - range, point.y - range);
  Point<F32> br(point.x + range, point.y + range);

  WorldCtrl::ClampMetreMapPoint(tl);
  WorldCtrl::ClampMetreMapPoint(br);

  // Top Left & Bottom Right (clusters)
  Point<U32> ctl;
  WorldCtrl::MetresToClusterPoint(tl, ctl);
  Point<U32> cbr;
  WorldCtrl::MetresToClusterPoint(br, cbr);

  F32 range2 = (range + WorldCtrl::ClusterRadius()) * (range + WorldCtrl::ClusterRadius());

  for (U32 cy = ctl.y; cy <= cbr.y; cy++)
  {
    for (U32 cx = ctl.x; cx <= cbr.x; cx++)
    {
      F32 centrex = WorldCtrl::ClusterSize() * cx + WorldCtrl::ClusterSizeHalf();
      F32 centrey = WorldCtrl::ClusterSize() * cy + WorldCtrl::ClusterSizeHalf();

      // If the range from the centre of the cluster is less 
      // than the radius of the cluster + radius of the circle
      // then the cluster should be considered
      if ((((point.x - centrex) * (point.x - centrex)) +
           ((point.y - centrey) * (point.y - centrey))) < range2)
      {
        clusters.Append(WorldCtrl::GetCluster(cx, cy));
      }
    }
  }
}


//
// SetDefaultMapSize
//
// Set default map sizes (TRUE if valid)
//
Bool WorldCtrl::SetDefaultMapSize(U32 x, U32 z)
{
  // Must be validated before this point
  if (CheckWorldSize(x, z))
  {
    defaultCellMapX = x;
    defaultCellMapZ = z;
    return (TRUE);
  }
  
  return (FALSE);
}
