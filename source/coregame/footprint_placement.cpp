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
#include "sight.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FootPrint - Object footprinting system
//
namespace FootPrint
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement - Returns information about the placement of a footprint
  //


  //
  // Acceptable
  //
  // Is the given result fatal
  //
  Bool Placement::Acceptable(Result r)
  {
    switch (r)
    {
      case PR_NONE:
      case PR_OFFMAP:
      case PR_FOOTON:
      case PR_CLAIM:
        return (FALSE);

      case PR_OK:
      case PR_SHROUDED:
      case PR_CANMOVETO:
      case PR_FOOTOFF:
      case PR_THUMPING:
        return (TRUE);

      default : 
        LOG_WARN(("Unknown placement result (%d)", r));
        return (FALSE);
    }
  }


  //
  // Constructor
  //
  Placement::Placement(Type *placeType, Team *team) 
  : type(NULL)
  {
    thumped.SetPlacement(this);
    Init(placeType, team);
  }


  //
  // Constructor
  //
  Placement::Placement() 
  : type (NULL)
  {
    thumped.SetPlacement(this);
  }


  //
  // Destructor
  //
  Placement::~Placement()
  {
    Done();
  }


  //
  // Init
  //
  // Setup with a footprint type
  //
  void Placement::Init(Type *placeType, Team *teamIn)
  {
    ASSERT(placeType);

    // Make sure we're shutdown
    Done();

    // Save the setup data
    type = placeType;
    team = teamIn;

    // Setup array sizes (includes border around footprint
    size.x = type->Size().x + 2;
    size.z = type->Size().z + 2;

    // Allocate the cell array
    array = new Cell[size.x * size.z];

    // Allocate thumped data
    thumped.Init(size);

    // Set default values
    Reset();
  }


  //
  // Done
  //
  // Shutdown if setup (called from destructor)
  //
  void Placement::Done()
  {
    // Reset last check result
    result = PR_NONE;

    // Delete data if currently setup
    if (IsSetup())
    {
      delete [] array;
      type = NULL;
    }

    // Release thumped data
    thumped.Done();
  }


  //
  // Reset
  //
  // Set default values in all cells
  //
  void Placement::Reset()
  {
    ASSERT(IsSetup());

    for (S32 z = 0; z < size.z; z++)
    {
      for (S32 x = 0; x < size.x; x++)
      {
        // Get the required cell
        Cell &cell = GetCell(x, z);

        // Does this cell lie on the footprint
        cell.onFoot = ((x > 0) && (x < size.x - 1) && (z > 0) && (z < size.z - 1));

        // Set default data
        cell.result = PR_NONE;
        cell.map.Set(0, 0);
        cell.zip.Set(-1, -1);
        cell.type.Set(x - 1, z - 1);
      }
    }   

    // Reset best height
    thumpHeight = 0.0F;
  }


  //
  // AdjustLocation
  //
  // Adjust the given origin location to be correct for this footprint
  //
  void Placement::AdjustLocation(Matrix &location)
  {
    ASSERT(IsSetup());

    Vector pos;
    S32 cellX, cellZ;

    // Snap rotation
    WorldCtrl::SnapRotation(location);

    // Get the world position of the first shadow vertex
    type->GetVertexPosition(pos, location, 0, 0);
    
    // Get the closest cell point is within
    Terrain::CellCoords
    (
      pos.x + WorldCtrl::CellSize() / 2,
      pos.z + WorldCtrl::CellSize() / 2,
      cellX, cellZ
    );

    // Adjust the object matrix using the delta from shadow vertex
    location.posit.x += (F32)(cellX * WC_CELLSIZE) - pos.x;
    location.posit.z += (F32)(cellZ * WC_CELLSIZE) - pos.z;
  }


  //
  // FindOrigin
  //
  // Returns the lower left corner in terrain space
  //
  void Placement::FindOrigin(const Matrix &location, WorldCtrl::CompassDir d, Point<S32> &o)
  {
    ASSERT(IsSetup());

    S32 x = 0, z = 0;
    Vector pos;

    // Which vert is bottom left in terrain space
    switch (d)
    {
      case WorldCtrl::EAST:
        x = type->Size().x;
        break;

      case WorldCtrl::SOUTH:
        x = type->Size().x;
        z = type->Size().z;
        break;

      case WorldCtrl::WEST:
        z = type->Size().z;
        break;
    }     

    // Get the world position of the first shadow vertex
    type->GetVertexPosition(pos, location, x, z);

    // Get the closest cell point is within
    Terrain::CellCoords
    (
      pos.x + WorldCtrl::CellSize() / 2, pos.z + WorldCtrl::CellSize() / 2, o.x, o.z
    );
  }

  
  //
  // ClaimBlock
  //
  // Are we blocked by claiming
  //
  Bool Placement::ClaimBlock(const Point<S32> &g, Claim::LayerId layer, U32 flags)
  {
    // Iterate each grain in the cell
    for (S32 z = g.z; z <= g.z + 1; ++z)
    {
      for (S32 x = g.x; x <= g.x + 1; ++x)
      {
        // Has something claimed this grain
        if (!Claim::Probe(x, z, x, z, layer))
        {
          // Can we ignore mobile units
          if (flags & CHECK_IGNOREMOBILE)
          {
            // Try and find the owner
            if (UnitObj *unit = Claim::GetOwner(x, z, layer))
            {
              // Does this unit block us
              if (!unit->CanEverMove())
              {
                // Blocked by an immobile unit
                return (TRUE);
              }
            }
            else
            {
              // Blocked by an unknown claim
              return (TRUE);
            }
          }
          else
          {
            // Blocked by a claim
            return (TRUE);
          }
        }
      }
    }

    // Not blocked by any claim
    return (FALSE);
  }


  //
  // Check
  //
  // Check if allowed to build at the given location
  //
  Placement::Result Placement::Check(const Matrix &location, U32 flags)
  {
    ASSERT(IsSetup());

    S32 xZip, zZip, xFoot, zFoot;
    S32 xMax, zMax;
    S32 x, z;

    // Reset the array
    Reset();

    // Set default result
    result = PR_OK;

    // Get direction
    dir = WorldCtrl::GetCompassDirection(location.front);

    // Get cell in bottom left corner of footprint
    FindOrigin(location, dir, min);

    // Setup extents using current direction
    SetupMaximums(xMax, zMax, size.x, size.z, dir);

    // Save max point
    max.Set(min.x + xMax - 3, min.z + zMax - 3);

    // Step over footprint in terrain space
    // This loop must occur before the thump simulation, as cell.map has to be set up
    for (z = 0; z < zMax; z++)
    {
      for (x = 0; x < xMax; x++)
      {
        // Convert the terrain offset to footprint offsets
        CalculateOffsets(dir, size.x, size.z, x, z, xZip, zZip, xFoot, zFoot);

        // Get the placement cell at this position
        Cell &cell = GetCell(xFoot, zFoot);

        // Is this a zipping location (not on a lower fringe)
        if (x > 0 && z > 0)
        {
          // Save zipping value, adjusting for fringe
          cell.zip.Set(xZip - 1, zZip - 1);

          ASSERT(cell.zip.x >= 0);
          ASSERT(cell.zip.z >= 0);
        }

        // Get actual terrain cell position
        cell.map.Set(min.x + x - 1, min.z + z - 1);
      }
    }

    // If we are doing a thumping simulation, then thumped.valid is TRUE and the
    // real TerrainData::Cell will be substituted for the thumped version when
    // performing the placement check.
    // Otherwise, the real terrain cell will be used
    if (flags & CHECK_NOTHUMPTEST)
    {
      // Thumped is no longer valid
      thumped.Invalidate();
    }
    else
    {
      // Copy terrain into thumped buffer
      if (thumped.CopyTerrain())
      {
        // Calculate the best height to build at
        thumpHeight = thumped.CalcBestHeight();

        // Thump terrain into the thumped buffer
        ThumpTerrain(thumpHeight, 0.0F, THUMP_TOBUFFER);

        // Update slopes
        thumped.UpdateCells();
      }
    }

    // UnitObjType is used inside the loop
    UnitObjType *unitObjType = Promote::Type<UnitObjType>(&type->GetMapType());

    // Step over footprint in terrain space
    for (z = 0; z < zMax; z++)
    {
      for (x = 0; x < xMax; x++)
      {
        // Default cell result to ok
        Result r = PR_OK;

        // Convert the terrain offset to footprint offsets
        CalculateOffsets(dir, size.x, size.z, x, z, xZip, zZip, xFoot, zFoot);

        // Get the placement cell at this position
        Cell &cell = GetCell(xFoot, zFoot);

        // Is this position on the map
        if (WorldCtrl::CellOnMapNoEdge(cell.map.x, cell.map.z))
        {
          // Get the map cell at this location
          TerrainData::Cell &origCell = TerrainData::GetCell(cell.map.x, cell.map.z);

          // Get the thumped cell at this location
          TerrainData::Cell *thumpCell = thumped.IsValid() ? thumped.GetCell(x, z) : NULL;

          // Is there another footprint at this location
          if (!AvailableInstanceIndex(origCell.footIndex))
          {           
            r = (cell.onFoot) ? PR_FOOTON : PR_FOOTOFF;
          }
          else

          if (!TestThumpArea(cell.map, cell.map))
          {
            r = PR_THUMPING;
          }
          else

          // Is this position on the footprint
          if (cell.onFoot)
          {
            Point<S32> g;
          
            // Convert cell to grain
            WorldCtrl::CellToFirstGrain(cell.map.x, cell.map.z, g.x, g.z);

            // Get the type cell
            Type::Cell &typeCell = GetType().GetCell(cell.type.x, cell.type.z);

            // Probe lower grains
            if (typeCell.GetFlag(Type::CLAIMLO) && ClaimBlock(g, Claim::LAYER_LOWER, flags))
            {
              r = PR_CLAIM;
            }
            else

            // Probe upper grains
            if (typeCell.GetFlag(Type::CLAIMHI) && ClaimBlock(g, Claim::LAYER_UPPER, flags))
            {
              r = PR_CLAIM;
            }
            else

            // Can this object build on this location
            if (!PathSearch::CanMoveToCell(type->GetMapType().GetTractionIndex(type->GetMapType().GetDefaultLayer()), origCell))
            {
              r = PR_CANMOVETO;
            }
            else

            if (thumpCell && !PathSearch::CanMoveToCell(type->GetMapType().GetTractionIndex(type->GetMapType().GetDefaultLayer()), *thumpCell))
            {
              r = PR_CANMOVETO;
            }
            else

            // Is this cell shrouded
            if (!(flags & CHECK_IGNORESHROUD) && team && !Sight::Seen(cell.map.x, cell.map.z, team))
            {
              r = PR_SHROUDED;
            }
          }
          else
          {
            if (unitObjType)
            {
              if (unitObjType->CanBoard())
              {
                // The pathability check is extended to the fringe cells for objects that can have a board manager
                if (!PathSearch::CanMoveToCell(type->GetMapType().GetTractionIndex(type->GetMapType().GetDefaultLayer()), origCell))
                {
                  r = PR_CANMOVETO;
                }
                else

                if (thumpCell && !PathSearch::CanMoveToCell(type->GetMapType().GetTractionIndex(type->GetMapType().GetDefaultLayer()), *thumpCell))
                {
                  r = PR_CANMOVETO;
                }
              }
              else
              {
                // Not allowed to change the pathability state from passabable to impassable
                if (PathSearch::CanMoveToCell(type->GetMapType().GetTractionIndex(type->GetMapType().GetDefaultLayer()), origCell))
                {
                  if (thumpCell && !PathSearch::CanMoveToCell(type->GetMapType().GetTractionIndex(type->GetMapType().GetDefaultLayer()), *thumpCell))
                  {
                    r = PR_CANMOVETO;
                  }
                }
              }
            }
          }
        }
        else
        {
          r = PR_OFFMAP;
        }

        // Save result in cell
        cell.result = r;

        // Save significant results
        if (!Acceptable(r) || result == PR_OK)
        {
          result = r;
        }
      }
    }

    return (result);
  }


  //
  // Check
  //
  // Check if allowed to build at the given location
  //
  Placement::Result Placement::Check(const Vector &location, WorldCtrl::CompassDir d, U32 flags)
  {
    Matrix m;

    // Generate the suitable matrix
    WorldCtrl::SetupWorldMatrix(m, location, d);

    // Call primary method
    return (Check(m, flags));
  }


  //
  // SetupEvacuator
  //
  // Add all the claiming cells to the given unit evacuator
  //
  void Placement::SetupEvacuator(UnitEvacuate &evacuate)
  { 
    ASSERT(IsSetup())

    // Step over entire placement array
    for (S32 z = 0; z < size.z; z++)
    {
      for (S32 x = 0; x < size.x; x++)
      {
        // Get the placement cell at this position
        Cell &cell = GetCell(x, z);

        // Is this position on the footprint
        if (cell.onFoot)
        {
          ASSERT(WorldCtrl::CellOnMapX(cell.map.x));
          ASSERT(WorldCtrl::CellOnMapZ(cell.map.z));      

          // Get the type data
          Type::Cell &typeCell = GetType().GetCell(cell.type.x, cell.type.z);

          // Claim lower grains
          if (typeCell.GetFlag(Type::CLAIMLO))
          {
            evacuate.Register(cell.map, Claim::LAYER_LOWER);
          }

          // Claim upper grains
          if (typeCell.GetFlag(Type::CLAIMHI))
          {
            evacuate.Register(cell.map, Claim::LAYER_UPPER);
          }
        }
      }
    }
  }


  //
  // Claim
  //
  // Claim or release the cells to be built on
  //
  void Placement::Claim(Claim::Manager &info, Bool toggle, U32 key)
  {
    ASSERT(IsSetup())
    ASSERT(LastResult() == PR_OK)

    // Step over entire placement array
    for (S32 z = 0; z < size.z; z++)
    {
      for (S32 x = 0; x < size.x; x++)
      {
        // Get the placement cell at this position
        Cell &cell = GetCell(x, z);

        // Toggle the thump mutex
        if (toggle)
        {
          LockThumpArea(cell.map, cell.map);
        }
        else
        {
          UnlockThumpArea(cell.map, cell.map);
        }

        // Is this position on the footprint
        if (cell.onFoot)
        {
          ASSERT(WorldCtrl::CellOnMapX(cell.map.x));
          ASSERT(WorldCtrl::CellOnMapZ(cell.map.z));

          Point<S32> g;
       
          // Get the terrain data cell
          TerrainData::Cell &dataCell = TerrainData::GetCell(cell.map.x, cell.map.z);

          // Claim the cells
          if (toggle)
          {
            ASSERT(AvailableInstanceIndex(dataCell.footIndex))

            // Claim the foot index
            dataCell.footIndex = INSTANCE_CLAIM;

            // Convert cell to grain
            WorldCtrl::CellToFirstGrain(cell.map.x, cell.map.z, g.x, g.z);

            // Get the type data
            Type::Cell &typeCell = GetType().GetCell(cell.type.x, cell.type.z);

            // Claim lower grains
            if (typeCell.GetFlag(Type::CLAIMLO))
            {
              info.ChangeDefaultLayer(Claim::LAYER_LOWER);
              info.Claim(g.x, g.z, g.x + 1, g.z + 1, key);
            }

            // Claim upper grains
            if (typeCell.GetFlag(Type::CLAIMHI))
            {
              info.ChangeDefaultLayer(Claim::LAYER_UPPER);
              info.Claim(g.x, g.z, g.x + 1, g.z + 1, key);
            }
          }
          else
          {
            // Clear the instance index
            dataCell.footIndex = INSTANCE_NONE;

            // Release the grains
            info.Release();
          }
        }
      }
    }
  }


  //
  // ThumpTerrain
  //
  // Thump the terrain using current zip values and the given origin height
  //
  Bool Placement::ThumpTerrain(F32 height, F32 rate, U32 flags)
  {
    ASSERT(IsSetup());
    ASSERT(Acceptable(LastResult()));
    ASSERT(rate >= 0.0F);

    // Are we finished zipping
    Bool finished = TRUE;

    // Calculate distance (in metres) each cell should change
    //F32 mod = rate ? rate / ((size.x - 2) * (size.z - 2)) : 0;
    F32 mod = rate;

    ASSERT(mod >= 0.0F);

    // Get the layer used for zipping
    Layer &layer = type->GetLayer(Type::LAYER_LOWER);

    // Step over entire placement array
    for (S32 z = 0; z < size.z; z++)
    {
      for (S32 x = 0; x < size.x; x++)
      {
        // Get the placement cell at this position
        Cell &cell = GetCell(x, z);

        ASSERT(WorldCtrl::CellOnMapX(cell.map.x));
        ASSERT(WorldCtrl::CellOnMapZ(cell.map.z));

        // Is this a zipping position
        if (cell.Zippable())
        {
          // Get the terrain cell at this location
          ::Cell *tCell = Terrain::GetCell(cell.map.x, cell.map.z);
     
          // Get the layer cell
          Layer::Cell &layerCell = layer.GetCell(cell.zip.x, cell.zip.z);

          // Should this cell be zipped
          if (layerCell.GetFlag(Layer::ZIP))
          {
            // Get the current height
            F32 current = tCell->GetHeight();

            // Calculate the target height
            F32 target = height + layerCell.pos.y;

            // If there is a rate, we may need to move a segment
            if (mod)
            {
              // Get the distance between actual and target
              F32 distance = current - target;

              // Is the distance significant
              if (fabs(distance) > mod)
              {
                target = current + ((distance < 0.0F) ? mod : -mod);
                finished = FALSE;
              }
            }

            // Only modify if difference
            if (flags & THUMP_TOTERRAIN)
            {
              if (target != current)
              {
                TerrainData::SessionModifyHeight(cell.map.x, cell.map.z, target);
              }
            }

            if (flags & THUMP_TOBUFFER)
            {
              thumped.SetHeight(cell.map.x - min.x + 1, cell.map.z - min.z + 1, target);
            }
          }
        }
      }
    }

    return (finished);
  }


  //
  // GetCell
  //
  // Returns the data for the given cell
  //
  Placement::Cell & Placement::GetCell(S32 x, S32 z)
  {
    ASSERT(IsSetup());
    ASSERT(x >= 0 && x < size.x);
    ASSERT(z >= 0 && z < size.z);

    return (array[z * size.x + x]);
  }


  //
  // CellOnFringe
  //
  // Is the given map cell on the fringe of the footprint
  //
  Bool Placement::CellOnFringe(const Point<S32> &pos)
  {
    return (FootPrint::CellOnFringe(min, max, pos));
  }


  //
  // CellOnFoot
  //
  // Is the given map cell on the footprint
  //
  Bool Placement::CellOnFoot(const Point<S32> &pos)
  {
    return (FootPrint::CellOnFoot(min, max, pos));
  }

  
  //
  // ClampToFringe
  //
  // Clamp the given cell location to the closest fringe cell
  //
  void Placement::ClampToFringe(Point<S32> &pos)
  {
    FootPrint::ClampToFringe(min, max, pos);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Placement::Thumped
  //


  //
  // Constructor
  //
  Placement::Thumped::Thumped()
  : heights(NULL),
    cells(NULL),
    placement(NULL)
  {
  }


  //
  // Init
  //
  // Initialise data structures
  //
  void Placement::Thumped::Init(const Point<S32> &size)
  {
    ASSERT(!heights)
    ASSERT(!cells)

    heights = new F32[(size.x + 1) * (size.z + 1)];
    cells = new TerrainData::Cell[size.x * size.z];
    valid = FALSE;
  }


  //
  // Done
  //
  // Release memory
  //
  void Placement::Thumped::Done()
  {
    if (heights)
    {
      delete[] heights;
      heights = NULL;
    }
    if (cells)
    {
      delete[] cells;
      cells = NULL;
    }

    valid = FALSE;
  }


  //
  // CopyTerrain
  //
  // Copy terrain from placement.Min() into buffers
  //
  Bool Placement::Thumped::CopyTerrain()
  {
    ASSERT(placement)

    const Point<S32> &min = placement->GetMin();
    const Point<S32> &size = placement->Size();

    // Setup extents using current direction
    SetupMaximums(localMax.x, localMax.z, size.x, size.z, placement->GetDir());

    for (S32 z = 0; z <= localMax.z; z++)
    {
      for (S32 x = 0; x <= localMax.x; x++)
      {
        // Terrain position
        Point<S32> terrPos(min.x + x - 1, min.z + z - 1);

        if (WorldCtrl::CellOnMap(terrPos.x, terrPos.z))
        {
          //F32 h[4];
          // Update bottom left vertex only
          //TerrainData::CellHeights(terrPos.x, terrPos.z, h);
          //SetHeight(x, z, h[0]);

          SetHeight(x, z, TerrainData::GetHeightWithWater(terrPos.x, terrPos.z));
        }
        else
        {
          return (valid = FALSE);
        }
      }
    }

    return (valid = TRUE);
  }


  //
  // CalcBestHeight
  //
  // Calculate best height to thump to
  //
  F32 Placement::Thumped::CalcBestHeight()
  {
    ASSERT(placement)
    ASSERT(heights)
    ASSERT(valid)

    F32 sum = 0.0F;
    U32 count = 0;

    for (S32 x = 1; x < localMax.x; x++)
    {
      sum += GetHeight(x, 1) + GetHeight(x, localMax.z-1);
      count += 2;
    }

    for (S32 z = 2; z < localMax.z - 1; z++)
    {
      sum += GetHeight(1, z) + GetHeight(localMax.x-1, z);
      count += 2;
    }

    ASSERT(count)

    return (sum / F32(count));
  }


  //
  // UpdateCells
  //
  // Update slopes into TerrainData cells
  //
  void Placement::Thumped::UpdateCells()
  {
    ASSERT(placement)
    ASSERT(valid)
    ASSERT(cells)
    ASSERT(heights)

    const Point<S32> &min = placement->GetMin();

    for (S32 z = 0; z < localMax.z; z++)
    {
      for (S32 x = 0; x < localMax.x; x++)
      {
        Point<S32> terrPos(min.x + x - 1, min.z + z - 1);

        // Get thumped cell at this location
        TerrainData::Cell &c = *GetCell(x, z);

        // Copy cell information from terrain cell
        c = TerrainData::GetCell(terrPos.x, terrPos.z);

        // Orientation is same as terrain cell
        //
        // 1+-------+2
        //  |       |
        //  |       |
        //  |       |
        // 0+-------+3

        //Vector v1, v2, v3, norm1, norm2, norm;
        F32 h[4];
        Vector norm;

        h[0] = GetHeight(x, z);
        h[1] = GetHeight(x, z+1);
        h[2] = GetHeight(x+1, z+1);
        h[3] = GetHeight(x+1, z);

        Terrain::heightField.FindFloor(h, WC_CELLSIZEF32 * 0.5F, WC_CELLSIZEF32 * 0.5F, &norm);

        Vector v(norm);
        norm.y = 0.0F;
        norm.Normalize();

        c.slope = (U8)(asin(fabs(v.Dot(norm))) * RAD2DEG);
      }
    }
  }


  //
  // Set height of a vertex
  //
  void Placement::Thumped::SetHeight(S32 x, S32 z, F32 height)
  {
    ASSERT(placement)
    ASSERT(x >= 0 && x <= localMax.x)
    ASSERT(z >= 0 && z <= localMax.z)

    heights[z * (localMax.x + 1) + x] = height;
  }


  //
  // Get height of a vertex
  //
  F32 Placement::Thumped::GetHeight(S32 x, S32 z) const
  {
    ASSERT(valid)
    ASSERT(placement)
    ASSERT(x >= 0 && x <= localMax.x)
    ASSERT(z >= 0 && z <= localMax.z)

    return (heights[z * (localMax.x + 1) + x]);
  }


  //
  // Get terrain data cell
  //
  TerrainData::Cell *Placement::Thumped::GetCell(S32 x, S32 z) const
  {
    ASSERT(valid)
    ASSERT(placement)
    ASSERT(x >= 0 && x < localMax.x)
    ASSERT(z >= 0 && z < localMax.z)

    return (&cells[z * localMax.x + x]);
  }
}
