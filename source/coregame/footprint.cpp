///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 25-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "footprint.h"
#include "mapobj.h"
#include "terrain.h"
#include "terraindata.h"
#include "meshent.h"
#include "wallobj.h"
#include "promote.h"
#include "unitevacuate.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FootPrint - Object footprinting system
//
namespace FootPrint
{
  // System init flag
  static Bool initialized = FALSE;

  // Number of current instances
  static U32 instanceCount;

  // Index used to scan for free slots
  static U32 nextIndex;

  // Instance array
  static Instance **instances;

  // Thumping mutex array
  static BitArray2d *thumpMutex;


  //
  // SetupMaximums
  //
  // Setup axis maximums using the given direction
  //
  void SetupMaximums(S32 &xMax, S32 &zMax, S32 xSize, S32 zSize, WorldCtrl::CompassDir dir)
  {
    // Maximums are swapped for EAST and WEST directions
    switch (dir)
    {
      case WorldCtrl::NORTH:
      case WorldCtrl::SOUTH:
        xMax = xSize;
        zMax = zSize;
        break;

      default:
        xMax = zSize;
        zMax = xSize;
        break;
    }     
  }

  
  //
  // CalculateOffsets
  //
  // Used to calculate zipping and footprint offsets
  //
  void CalculateOffsets
  (
    U32 dir,                  // Direction object is facing
    S32 xArray, S32 zArray,   // Size of the actual footprint array
    S32 x, S32 z,             // Terrain offsets to convert
    S32 &xZip, S32 &zZip,     // Returns the zipping cell index
    S32 &xFoot, S32 &zFoot    // Returns the footprint index
  )
  {
    switch (dir)
    {
      case WorldCtrl::NORTH:
        xZip = xFoot = x;
        zZip = zFoot = z;
        break;

      case WorldCtrl::EAST:
        xZip = xFoot = xArray - z;
        zZip = zFoot = x;
        xFoot--;
        break;

      case WorldCtrl::SOUTH:
        xZip = xFoot = xArray - x;
        zZip = zFoot = zArray - z;
        xFoot--;
        zFoot--;
        break;

      case WorldCtrl::WEST:
        xZip = xFoot = z;
        zZip = zFoot = zArray - x;
        zFoot--;
        break;
    }
  }


  //
  // IncrementIndex
  //
  // Increments the given index, wrapping if reached limit
  //
  static void IncrementIndex(U32 &index)
  {
    ASSERT(index >= INSTANCE_MIN);

    index++;

    if (index >= MAX_INSTANCES)
    {
      index = INSTANCE_MIN;
    }
  }


  //
  // GetFreeIndex
  //
  // Returns a free instance index
  //
  static U32 GetFreeIndex()
  {
    ASSERT(initialized);

    // Fatal if limit exceeded
    if (instanceCount >= MAX_INSTANCES)
    {
      ERR_FATAL(("Maximum footprint instances exceeded!! (%d)", MAX_INSTANCES));
    }

    // Find next available slot
    for (U32 count = 0, index = nextIndex; count < MAX_INSTANCES; count++)
    {
      ASSERT(ValidInstanceIndex(index));

      // Is this space available
      if (!instances[index])
      {
        nextIndex = index;
        IncrementIndex(nextIndex);
        return (index);
      }

      // Wrap index around
      IncrementIndex(index);
    }

    // This should never happen, but paranoia has set in...
    ERR_FATAL(("Instance scan found no available index! (%d)", instanceCount));
  }


  //
  // CreateInstance
  //
  // Create a new footprint instance
  //
  Instance & CreateInstance(MapObj *obj, Placement &place)
  {
    ASSERT(initialized);
    ASSERT(obj);
    ASSERT(!obj->GetFootInstance());

    // Get a free index
    U32 index = GetFreeIndex();

    ASSERT(!instances[index]);

    // Create new instance
    Instance *i = new Instance(obj, index, place);

    // Add to array
    instances[index] = i;

    // Increase instance count
    instanceCount++;

    // Return a reference
    return (*i);
  }


  //
  // DeleteInstance
  //
  // Delete an existing footprint instance
  //
  void DeleteInstance(MapObj *obj)
  {
    ASSERT(initialized);
    ASSERT(obj);

    // Get the instance
    Instance *instance = obj->GetFootInstance();

    ASSERT(instance);
    ASSERT(ValidInstanceIndex(instance->GetIndex()));
    ASSERT(instances[instance->GetIndex()] == instance);

    // If this object is a wall, break all links
    if (WallObj *wall = Promote::Object<WallObjType, WallObj>(obj))
    {
      wall->BreakLinks(FALSE);
    }

    // Remove from the array
    instances[instance->GetIndex()] = NULL;

    // Decrease instance count
    instanceCount--;

    // Delete the instance
    delete instance;
  }


  //
  // GetInstance
  //
  // Get a footprint instance
  //
  Instance & GetInstance(U32 index)
  {
    ASSERT(initialized);

    #ifdef DEVELOPMENT
      if (!ValidInstanceIndex(index))
      {
        ERR_FATAL(("Invalid footprint instance index (%d/%d)", index, instanceCount));
      }

      if (!instances[index])
      {
        ERR_FATAL(("Instance index hit empty slot (%d/%d)", index, instanceCount));
      }
    #endif

    return (*instances[index]);
  }


  //
  // CellHeights
  //
  // Returns the cell heights for the second level at the given cell
  //
  void CellHeights(S32 x, S32 z, F32 *heights)
  {
    // Get the data cell
    TerrainData::Cell &dataCell = TerrainData::GetCell(x, z);

    // Get the footprint instance
    Instance &instance = GetInstance(dataCell.footIndex);

    // Get the footprint position of this cell
    const Point<S32> &p = instance.GetFootPosition(x, z);

    // Get the layer
    Layer &layer = instance.GetType().GetLayer(Type::LAYER_UPPER);

    // Function should never be called unless an upper layer exists
    ASSERT(layer.IsSetup());

    // Get heights that define the cell
    layer.CellHeights(p.x, p.z, instance.GetDir(), heights);
 
    // Get the height of the object
    F32 h = instance.GetObj().WorldMatrix().posit.y;

    // Add to each vertex height
    for (U32 i = 0; i < 4; heights[i++] += h);
  }


  //
  // FindFloor
  //
  // Find floor for footprinted objects
  //
  F32 FindFloor(F32 x, F32 z, S32 cx, S32 cz, Vector *surfNormal)
  {
    F32 heights[4];

    // Get the cell heights
    CellHeights(cx, cz, heights);

    // Calculate the deltas
    F32 dx = x - ((F32)cx * WorldCtrl::CellSize());
    F32 dz = z - ((F32)cz * WorldCtrl::CellSize());

    // Call lower level function
    return (Terrain::heightField.FindFloor(heights, dx, dz, surfNormal));
  }


  //
  // GetHeight
  //
  // Returns the cell height for the second level at the given cell
  //
  F32 GetHeight(S32 cx, S32 cz)
  {
    // Get the data cell
    TerrainData::Cell &dataCell = TerrainData::GetCell(cx, cz);

    // Get the footprint instance
    Instance &instance = GetInstance(dataCell.footIndex);

    // Get the footprint position of this cell
    const Point<S32> &p = instance.GetFootPosition(cx, cz);

    // Get the layer
    Layer &layer = instance.GetType().GetLayer(Type::LAYER_UPPER);

    // Function should never be called unless an upper layer exists
    ASSERT(layer.IsSetup());

    // Get heights that define the cell
    F32 y = layer.GetHeight(p.x, p.z, instance.GetDir());
 
    // Get the height of the object
    F32 h = instance.GetObj().WorldMatrix().posit.y;

    return y + h;
  }


  //
  // CellOnFringe
  //
  // Is the given map cell on the fringe of the given footprint
  //
  Bool CellOnFringe(const Point<S32> &min, const Point<S32> &max, const Point<S32> &pos)
  {
    // Get fringe bounds
    Point<S32> fMin(min - 1);
    Point<S32> fMax(max + 1);

    return 
    (
      (((pos.x == fMin.x) || (pos.x == fMax.x)) && ((pos.z >= fMin.z && pos.z <= fMax.z)))

      ||

      (((pos.z == fMin.z) || (pos.z == fMax.z)) && ((pos.x >= fMin.x && pos.x <= fMax.x)))
    );    
  }


  //
  // CellOnFoot
  //
  // Is the given map cell on the given footprint
  //
  Bool CellOnFoot(const Point<S32> &min, const Point<S32> &max, const Point<S32> &pos)
  {
    return ((pos.x >= min.x && pos.x <= max.x) && (pos.z >= min.z && pos.z <= max.z)); 
  }


  //
  // ClampToFringe
  //
  // Clamp the given cell location to the closest fringe cell
  //
  void ClampToFringe(const Point<S32> &min, const Point<S32> &max, Point<S32> &pos)
  {
    // Get fringe bounds
    Point<S32> fMin(min - 1);
    Point<S32> fMax(max + 1);

    // Clamp both values into the fringe area
    pos.x = Clamp<S32>(fMin.x, pos.x, fMax.x);
    pos.z = Clamp<S32>(fMin.z, pos.z, fMax.z);

    // For internal points, get the closest fringe for each axis
    S32 cx = (abs(fMin.x - pos.x) < abs(fMax.x - pos.x)) ? fMin.x : fMax.x;
    S32 cz = (abs(fMin.z - pos.z) < abs(fMax.z - pos.z)) ? fMin.z : fMax.z;

    // Jump to the closest fringe
    if (abs(cx - pos.x) < abs(cz - pos.z))
    {
      pos.x = cx;
    }
    else
    {
      pos.z = cz;
    }

    ASSERT(CellOnFringe(min, max, pos))
  }


  //
  // TesthumpArea
  //
  // Test a section of the map for thumping progress
  //
  Bool TestThumpArea(const Point<S32> &min, const Point<S32> &max)
  {
    // Check all cells
    for (S32 z = min.z; z <= max.z; z++)
    {
      for (S32 x = min.x; x <= max.x; x++)
      {
        if (thumpMutex->Get2(x, z))
        {
          return (FALSE);
        }
      }
    }
    return (TRUE);
  }


  //
  // LockThumpArea
  //
  // Lock a section of the map for thumping
  // 
  void LockThumpArea(const Point<S32> &min, const Point<S32> &max)
  {
    // Lock all cells
    for (S32 z = min.z; z <= max.z; z++)
    {
      for (S32 x = min.x; x <= max.x; x++)
      {
        ASSERT(!thumpMutex->Get2(x, z))
        thumpMutex->Set2(x, z);
      }
    }
  }


  //
  // UnlockThumpArea
  //
  // Unlock section of map previously locked by LockThumpArea
  //
  void UnlockThumpArea(const Point<S32> &min, const Point<S32> &max)
  {
    for (S32 z = min.z; z <= max.z; z++)
    {
      for (S32 x = min.x; x <= max.x; x++)
      {
        ASSERT(thumpMutex->Get2(x, z))
        thumpMutex->Clear2(x, z);
      }
    }
  }


  //
  // GetThumpMutex
  //
  // Return thumping mutex bit array
  //
  BitArray2d *GetThumpMutex()
  {
    ASSERT(initialized)
    return (thumpMutex);
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized);

    // Allocate an array of instance pointers
    instances = new Instance*[MAX_INSTANCES];
    instanceCount = 0;
    nextIndex = INSTANCE_MIN;

    // Set all pointers to null
    Utils::Memset(instances, 0, MAX_INSTANCES * sizeof(Instance*));

    // Intiialise thumping mutex array
    thumpMutex = new BitArray2d(WorldCtrl::CellMapX(), WorldCtrl::CellMapZ());

    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized);
    ASSERT(!instanceCount);

    // Delete thumping mutex array
    delete thumpMutex;
    thumpMutex = NULL;

    // Delete the instance array
    delete [] instances;

    initialized = FALSE;
  }
}
