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
#include "footprint_private.h"
#include "unitobjiter.h"
#include "promote.h"
#include "coregame.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FootPrint - Object footprinting system
//
namespace FootPrint
{

  // Claiming key
  static const U32 CLAIM_KEY = 0x5E38554A; // "Footprint"

  // Debug logging
  #define LOG_FOOTPRINT(x)
  //#define LOG_FOOTPRINT(x) LOG_DIAG(x)


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Instance - Footprint data for each instance
  //

  //
  // Constructor
  // 
  Instance::Instance(MapObj *obj, U32 index, Placement &place) : obj(obj), index(index), claimInfo(NULL), losBlockHeight(F32_MAX)
  {
    ASSERT(obj);
    ASSERT(!obj->GetFootInstance());
    ASSERT(place.IsSetup());

    // Point to the footprint type
    type = &place.GetType();

    // Get direction to face
    dir = place.GetDir();

    // Setup extents using placement direction
    SetupMaximums(size.x, size.z, place.Size().x, place.Size().z, dir);

    // Allocate the cell array
    array = new Cell[size.x * size.z];

    // Copy placement position
    origin = min = place.GetMin();
    max = place.GetMax();

    // Move origin to point to bottom left of fringe
    origin -= 1;

    // Copy array data
    for (S32 z = 0; z < size.z; z++)
    {
      for (S32 x = 0; x < size.x; x++)
      {
        S32 xZip, zZip, xFoot, zFoot;

        // Get the instance cell
        Cell &cell = GetCell(x, z);

        // Convert the terrain offset to footprint offsets
        CalculateOffsets
        (
          place.GetDir(), place.Size().x, place.Size().z, x, z, xZip, zZip, xFoot, zFoot
        );

        // Set the footprint offset
        cell.foot.Set(xFoot - 1, zFoot - 1);
      }
    }

    // Turn instance effects on
    ToggleEffects(TRUE);

    // Register with the object
    obj->SetFootInstance(this);
  }


  //
  // Destructor
  //
  Instance::~Instance()
  {
    // Turn instance effects off
    ToggleEffects(FALSE);

    // Unregister with the object
    obj->SetFootInstance(NULL);

    // Delete the array
    delete [] array;
  }


  //
  // GetCell
  //
  // Returns the data for the given cell
  //
  Instance::Cell & Instance::GetCell(S32 x, S32 z)
  {
    ASSERT(x >= 0 && x < size.x);
    ASSERT(z >= 0 && z < size.z);

    return (array[z * size.x + x]);
  }


  //
  // GetFootPosition
  //
  // Returns the footprint point for the given map position
  //
  const Point<S32> & Instance::GetFootPosition(S32 xMap, S32 zMap)
  {
    return (GetCell(xMap - origin.x, zMap - origin.z).foot);
  }


  //
  // GetTypeCell
  //
  // Returns the type cell for the given map position.
  //
  Type::Cell & Instance::GetTypeCell(S32 xMap, S32 zMap)
  {
    // Get the foot location
    const Point<S32> &p = GetFootPosition(xMap, zMap);

    // Return the type cell
    return (type->GetCell(p.x, p.z));
  }


  //
  // Toggle
  //
  // Toggle instance effects
  //
  void Instance::ToggleEffects(Bool toggle)
  {
    // Lists of cells that need terrain effects applied
    TerrainData::PaintList paintList;
    TerrainData::PaintList surfaceList;

    // Does this footprint have a second layer
    Bool secondLayer = type->GetLayer(Type::LAYER_UPPER).IsSetup();

    // Step over footprint space
    for (S32 z = 1; z < size.z - 1; z++)
    {
      for (S32 x = 1; x < size.x - 1; x++)
      {
        // Get the map position of this cell
        Point<S32> map(x + origin.x, z + origin.z);

        ASSERT(WorldCtrl::CellOnMap(map.x, map.z));

        // Get the terrain cell at this location
        ::Cell *terrainCell = Terrain::GetCell(map.x, map.z);

        // Get the instance cell
        Cell &cell = GetCell(x, z);

        // Get the type cell
        Type::Cell &typeCell = type->GetCell(cell.foot.x, cell.foot.z);

        // Toggle cell display
        if (typeCell.GetFlag(Type::HIDE) || !toggle)
        {
          // Toggle cell display
          terrainCell->SetVisible(!toggle);
        }

        // Change terrain type to base
        if (toggle && typeCell.GetFlag(Type::SETBASE))
        {
          paintList.Append(new TerrainData::PaintCell(map, 0));
        }

        // Change surface type
        if (typeCell.GetFlag(Type::SURFACE))
        {
          surfaceList.Append(new TerrainData::PaintCell(map, typeCell.surface));
        }

        // Change second layer flag
        TerrainData::SessionModifySecondLayer
        (
          map.x, map.z, (toggle && secondLayer && typeCell.GetFlag(Type::SECOND))
        );

        // Change LOS flag
        TerrainData::SessionModifyBlockLOS
        (
          map.x, map.z, (toggle && typeCell.GetFlag(Type::BLOCKLOS))
        );

        // Claiming
        if (toggle)
        {
          Point<S32> g;
          
          // Convert cell to grain
          WorldCtrl::CellToFirstGrain(map.x, map.z, g.x, g.z);

          // Claim lower grains
          if (typeCell.GetFlag(Type::CLAIMLO))
          {
            claimInfo.ChangeDefaultLayer(Claim::LAYER_LOWER);
            claimInfo.Claim(g.x, g.z, g.x + 1, g.z + 1, CLAIM_KEY);
          }

          // Claim upper grains
          if (typeCell.GetFlag(Type::CLAIMHI))
          {
            claimInfo.ChangeDefaultLayer(Claim::LAYER_UPPER);
            claimInfo.Claim(g.x, g.z, g.x + 1, g.z + 1, CLAIM_KEY);
          }
        }

        // Get the terrain data cell
        TerrainData::Cell &dataCell = TerrainData::GetCell(map.x, map.z);

        // Modify the instance index
        if (toggle)
        {
          ASSERT(AvailableInstanceIndex(dataCell.footIndex))
          ASSERT(ValidInstanceIndex(index))

          // Store this instances index in the cell
          dataCell.footIndex = (U16)index;
        }
        else
        {
          ASSERT(dataCell.footIndex == index);

          // Clear the instance index
          dataCell.footIndex = INSTANCE_NONE;
        }
      }
    }

    if (toggle)
    {
      // Change all cells to be base type
      TerrainData::Paint(paintList);

      // Recalculate line of sight height
      losBlockHeight = GetObj().Mesh().Origin().y + GetObj().Mesh().ObjectBounds().Height();
    }
    else
    {
      // Release all claims
      claimInfo.Release(CLAIM_KEY);
    }

    // Modify surface types
    for (TerrainData::PaintList::Iterator i(&surfaceList); *i; i++)
    {
      // Get the terrain data cell
      TerrainData::Cell &cell = TerrainData::GetCell((*i)->x, (*i)->z);

      // Change or restore
      if (toggle)
      {
        // Paint cell contains destination surface type
        cell.surface = (*i)->type;
      }
      else
      {
        // Get the terrain type
        TerrainGroup::Type &type = TerrainGroup::GetActiveType(cell.terrain);

        // Change the surface
        cell.surface = type.Surface();
      }
    }

    // Update LOS for units that could see the footprint
    if (CoreGame::GetInSimulation())
    {
      Point<S32> scanMin = GetMin() - Point<S32>(Sight::MAXR, Sight::MAXR);
      Point<S32> scanMax = GetMax() + Point<S32>(Sight::MAXR, Sight::MAXR);

      WorldCtrl::ClampCellPoint(scanMin);
      WorldCtrl::ClampCellPoint(scanMax);

      // Iterate through each cluster in this range
      Point<U32> clustMin;
      Point<U32> clustMax;

      clustMin.x = WorldCtrl::CellsToClusterX(scanMin.x);
      clustMax.x = WorldCtrl::CellsToClusterX(scanMax.x);
      clustMin.z = WorldCtrl::CellsToClusterZ(scanMin.z);
      clustMax.z = WorldCtrl::CellsToClusterZ(scanMax.z);

      LOG_FOOTPRINT
      ((
        "RescanLOS cell:%d,%d->%d,%d range:[%d,%d]->[%d,%d] clust:[%d,%d]->[%d,%d]", 
        GetMin().x, GetMin().z, GetMax().x, GetMax().z,
        scanMin.x, scanMin.z, scanMax.x, scanMax.z,
        clustMin.x, clustMin.z, clustMax.x, clustMax.z
      ))

      for (U32 z = clustMin.z; z <= clustMax.z; z++)
      {
        MapCluster *cluster = WorldCtrl::GetCluster(clustMin.x, z);

        for (U32 x = clustMin.x; x <= clustMax.x; x++)
        {
          for (NList<UnitObj>::Iterator i(&cluster->unitList); *i; i++)
          {
            UnitObj *u = *i;

            S32 distX = Max<S32>(abs(u->cellX - GetMin().x), abs(u->cellX - GetMax().x));
            S32 distZ = Max<S32>(abs(u->cellZ - GetMin().z), abs(u->cellZ - GetMax().z));
            S32 dist = Max<S32>(distX, distZ);

            LOG_FOOTPRINT
            ((
              "Unit: %5d cell:%d,%d dist:%2d,%2d see:%2d clust:%d,%d [%s]", 
              u->Id(), u->cellX, u->cellZ, distX, distZ, u->UnitType()->GetSeeingRange(), 
              u->currentCluster->xIndex, u->currentCluster->zIndex, u->TypeName()
            ))

            if (U32(dist) < u->UnitType()->GetSeeingRange())
            {
              u->SetFlag(UnitObj::FLAG_UPDATELOS, TRUE);
              LOG_FOOTPRINT(("... in range"))
            }
          }
          cluster = cluster->GetNextX();
        }
      }
    }

    // Clear the lists
    paintList.DisposeAll();
    surfaceList.DisposeAll();
  }


  //
  // CellOnFringe
  //
  // Is the given map cell on the fringe of the footprint
  //
  Bool Instance::CellOnFringe(const Point<S32> &pos)
  {
    return (FootPrint::CellOnFringe(min, max, pos));
  }


  //
  // CellOnFoot
  //
  // Is the given map cell on the footprint
  //
  Bool Instance::CellOnFoot(const Point<S32> &pos)
  {
    return (FootPrint::CellOnFoot(min, max, pos));
  }


  //
  // ClampToFringe
  //
  // Clamp the given cell location to the closest fringe cell
  //
  void Instance::ClampToFringe(Point<S32> &pos)
  {
    FootPrint::ClampToFringe(min, max, pos);
  }
}
