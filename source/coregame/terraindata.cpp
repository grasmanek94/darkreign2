///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 25-FEB-1999
//  


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "terraindata.h"
#include "terraingroup.h"
#include "terrain.h"
#include "random.h"
#include "unitobjiter.h"
#include "movement_pathfollow.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace TerrainData - Game-play terrain management
//
namespace TerrainData
{
  LOGDEFLOCAL("TerrainData");

  // Number of successors that lead from a single cell
  enum { NUM_SUCCESSORS = 8 };

  // Number of segments around a cell
  enum { NUM_SEGMENTS = 4 };

  // Number of cells per segment
  enum { NUM_CELLPERSEG = 3 };

  // UV index table for blend directions
  U8 UVTable[BD_MAXIMUM] = { 24, 20, 16, 28 };

  // The maximum value of a blend axis/diag flag
  enum { MAX_BLENDFLAG = 16 };

  // Blend direction tables
  BlendDir blendTable[TerrainGroup::BLEND_COUNT][MAX_BLENDFLAG];

  // Offsets of each successor
  struct SuccOffset { S8 x, z; } sOffsets[NUM_SUCCESSORS] =
  {
    {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}
  };

  // Neighbour segments in successor indexes
  struct Segment { U8 offset[NUM_CELLPERSEG]; } segments[NUM_SEGMENTS] =
  {
    {0, 1, 2}, {2, 3, 4}, {4, 5, 6}, {6, 7, 0}
  };

  // System initialization flag
  Bool initialized = FALSE;

  // Is a cell modification session in progress
  static Bool sessionActive;

  // Session update flags
  static Bool sessionHeight;
  static Bool sessionLayer;

  // List used for cell modification sessions
  static PointList sessionList;

  // How many cells total
  static U32 cellMapSize;
  
  // The name of the cell map block in the terrain file
  static const char * cellBlockName = "Zwagabeast Terrain Ver 1.0";

  // The name of the terrain index mapping table
  static const char * mappingBlockName = "TerrainData::Mapping";

  // Water surfaces
  static U8 sWater;
  static U8 mWater;
  static U8 dWater;
  static U8 sShore;
  static U8 mShore;
  static U8 dShore;

  // Water depths
  static const F32 SHALLOW_WATER = 0.0F;
  static const F32 MEDIUM_WATER = 1.0F;
  static const F32 DEEP_WATER  = 2.5F;

  // The terrain cell map (extern for inlines)
  Cell *cellMap;

  // Bit arrays
  BitArray2d *bitArrayPaint;
  BitArray2d *bitArraySecondLayer;
  BitArray2d *bitArraySession;
  BitArray2d *bitArrayBlockLOS;

  // Change packing for structure written to disk
  #pragma pack(push, 1)

  //
  // Struct SaveCell - Save format for a single data cell
  //
  struct SaveCell
  {
    // The terrain type index
    U8 terrain;
  };

  // Restore packing
  #pragma pack(pop)


  //
  // PlaceTerrain
  //
  // Places terrain on all given cells and generates a dirty list
  //
  static void PlaceTerrain(const PaintList &paint, PointList &dirty)
  {
    // Use the bit array to only dirty a cell once
    bitArrayPaint->Reset();

    // Iterate each cell
    for (PaintList::Iterator i(&paint); *i; i++)
    {
      // Get position
      S32 x = (*i)->x;
      S32 z = (*i)->z;

      // Get the terrain type
      TerrainGroup::Type &type = TerrainGroup::GetActiveType((*i)->type);

      // Is this cell on the map
      if (WorldCtrl::CellOnMap(x, z))
      {
        // Modify the cell
        GetCell(x, z).ModifyTerrainType(type);

        // Dirty the cell and neighbors
        for (S32 zi = z - 1; zi <= z + 1; zi++)
        {
          for (S32 xi = x - 1; xi <= x + 1; xi++)
          {       
            // Is this cell on the map and not already dirty
            if (WorldCtrl::CellOnMap(xi, zi))
            {
              if (!bitArrayPaint->Get2(xi, zi))
              {
                // Add to the list
                dirty.Append(new NPoint(xi, zi));

                // Set the dirty bit
                bitArrayPaint->Set2(xi, zi);
              }
            }
          }
        }
      }
    }
  }


  //
  // IsOrphan
  //
  // Is the given cell an orphan (should be replaced with base)
  //
  static Bool IsOrphan(S32 x, S32 z, Cell &cell)
  {
    // Ignore this cell if already base terrain type
    if (!cell.terrain)
    {
      return (FALSE);
    }

    // Bit flags and successor index used in tests
    U32 flags, s;

    // Check each segment
    for (flags = 0, s = 0; s < NUM_SEGMENTS; s++)
    {
      // Get this segment
      Segment &seg = segments[s];
   
      // Check each cell
      for (U32 match = TRUE, c = 0; match && c < NUM_CELLPERSEG; c++)
      {
        // Get the successor
        SuccOffset &so = sOffsets[seg.offset[c]];

        // Get the location of this successor
        S32 xs = x + so.x;
        S32 zs = z + so.z;

        // Get the cell
        Cell *succ = WorldCtrl::CellOnMap(xs, zs) ? &GetCell(xs, zs) : NULL;

        // If cell is on the map, is it a different terrain type
        if (succ && succ->terrain != cell.terrain)
        {
          match = FALSE;
        }
      }

      // Was this segment a match
      if (match)
      {
        // Set the coresponding flag
        flags |= (1 << s);
      }
    }

    // Is invalid if no segments, or diagonal segments
    if (flags == 0 || flags == 5 || flags == 10)
    {
      return (TRUE);
    }

    // Check each successor (loop past starting point by 3)
    for (flags = 15, s = 0; s < NUM_SUCCESSORS + 3; s++)
    {
      // Get the successor offsets for this cell
      SuccOffset &so = sOffsets[s % NUM_SUCCESSORS];

      // Get the location of this successor
      S32 xs = x + so.x;
      S32 zs = z + so.z;
  
      // Shift flags to current successor position
      flags >>= 1;

      // Get the cell
      Cell *succ = WorldCtrl::CellOnMap(xs, zs) ? &GetCell(xs, zs) : NULL;

      // If cell is on the map, is it a different terrain type
      if (succ && succ->terrain != cell.terrain)
      {
        flags &= 7;
      }
      else
      {
        flags |= 8;
      }

      // Have we hit a !this-this-this-!this case
      if (flags == 6)
      {
        return (TRUE);
      }
    }

    // Passed all tests
    return (FALSE);
  }


  //
  // KillOrphans
  //
  // Sets all orphaned cells to be the base texture
  //
  static void KillOrphans(PointList &dirty)
  {
    // Get the base terrain type
    TerrainGroup::Type &base = TerrainGroup::GetActiveType(0);

    // Process each dirty cell
    for (PointList::Iterator i(&dirty); *i; i++)
    {
      // Get the cell position
      S32 x = (*i)->x;
      S32 z = (*i)->z;

      // Get the cell at this location
      Cell &cell = GetCell(x, z);

      // Is this cell an orphan
      if (IsOrphan(x, z, cell))
      {
        // Change to base type
        cell.ModifyTerrainType(base);

        // Dirty the cell and neighbors
        for (S32 zi = z - 1; zi <= z + 1; zi++)
        {
          for (S32 xi = x - 1; xi <= x + 1; xi++)
          {       
            // Is this cell on the map
            if (WorldCtrl::CellOnMap(xi, zi))
            {
              // Add to list, do NOT filter duplicates
              dirty.Append(new NPoint(xi, zi));
            }
          }
        }
      }
    }
  }


  //
  // GetBlendInfo
  //
  // Returns the blend type and direction of the given cell
  //
  static void GetBlendInfo(S32 x, S32 z, Cell &cell, U32 &blend, U32 &dir)
  {
    // If cell is base, no checks need to be done
    if (!cell.terrain)
    {
      // Use the full tile and a random direction
      blend = 0;
      dir = Random::nonSync.Integer(BD_MAXIMUM);
      return;
    }

    // The number of matching cells
    U32 axisCount = 0;
    U32 diagCount = 0;

    // The bits representing those matching cells
    U32 axisFlag = 0;
    U32 diagFlag = 0;

    // Step through each successor, building counts and bit arrays
    for (U32 s = 0; s < NUM_SUCCESSORS; s++)
    {
      // Get the successor offsets for this cell
      SuccOffset &so = sOffsets[s];

      // Get the location of this successor
      S32 xs = x + so.x;
      S32 zs = z + so.z;
  
      // Get the cell
      Cell *succ = WorldCtrl::CellOnMap(xs, zs) ? &GetCell(xs, zs) : NULL;

      // Is this a match
      if (!succ || succ->terrain == cell.terrain)
      {
        // Is this a diag cell
        if (s & 1)
        {
          diagCount++;
          diagFlag |= (1 << (s >> 1));
        }
        else
        {
          axisCount++;
          axisFlag |= (1 << (s >> 1));
        }
      }
    }
  
    // Work out the blend type
    switch (axisCount)
    {     
      // Must be a corner
      case 2: blend = TerrainGroup::BLEND_CORNER; 
        break;
      
      // Must be a straight
      case 3: blend = TerrainGroup::BLEND_STRAIGHT; 
        break;
      
      // More info required
      case 4:
      {
        switch (diagCount)
        {
          // Must be a full
          case 4: blend = TerrainGroup::BLEND_FULL; 
            break;

          // Must be a bend
          case 3: blend = TerrainGroup::BLEND_BEND; 
            break;

          // Should never happen
          default:
            ERR_FATAL(("Unexpected blend (%d,%d) (diagCount = %d)", x, z, diagCount));
        }
        break;
      }

      // Should never happen
      default:
        ERR_FATAL(("Unexpected blend (%d,%d) (axisCount = %d)", x, z, axisCount));
    }   

    // Work out direction
    switch (blend)
    {
      case TerrainGroup::BLEND_FULL:
      {
        // Since it's a full, pick a random direction
        dir = Random::nonSync.Integer(BD_MAXIMUM);
        break;
      }

      case TerrainGroup::BLEND_STRAIGHT:
      case TerrainGroup::BLEND_CORNER:
      {
        // Use the axis flag to index the blend table
        dir = blendTable[blend][axisFlag];
        break;
      }

      case TerrainGroup::BLEND_BEND:
      {
        // Use the diag flag to index the blend table
        dir = blendTable[blend][diagFlag];
        break;
      }

      // Should never happen
      default:
        ERR_FATAL(("Unexpected blend (%d,%d) (axisCount = %d)", x, z, axisCount));
    }

    // Should never happen
    if (dir >= BD_MAXIMUM)
    {
      ERR_FATAL
      ((
        "Invalid blend direction (%d,%d, %d, %d/%d, %d/%d)", 
        x, z, blend, axisCount, diagCount, axisFlag, diagFlag
      ));
    }
  }


  //
  // BlendCells
  //
  // Blends each cell in the given list
  //
  static void BlendCells(const PointList &list)
  {
    U32 blend = 0, dir = 0;

    // Use the bit array so we only blend cells once
    bitArrayPaint->Reset();

    // Blend each cell in the list
    for (PointList::Iterator i(&list); *i; i++)
    {
      // Get cell position
      S32 x = (*i)->x;
      S32 z = (*i)->z;

      // Ignore if already blended
      if (bitArrayPaint->Get2(x, z))
      {
        continue;
      }

      // Set this cell as blended
      bitArrayPaint->Set2(x, z);

      // Get the data cell
      Cell &cell = GetCell(x, z);

      // Get the blend and direction
      GetBlendInfo(x, z, cell, blend, dir);

      // Get the terrain type at this location
      TerrainGroup::Type &type = TerrainGroup::GetActiveType(cell.terrain);

      // Request index of texture
      U32 index = Terrain::AddTexture
      (
        type.BlendName(blend, type.GetRandomVariation(blend))
      );

      // Get the terrain cell
      ::Cell *tCell = Terrain::GetCell(x, z);

      // Set the texture
      tCell->texture = (U8)index;

      // Set the rotation using the uv entry for this direction
      tCell->uv = UVTable[dir];
    }
  }


  //
  // AdjustCell
  //
  // Adjusts the blend and variation of a cell (values are wrapped)
  //
  void AdjustCell(S32 x, S32 z, BlendDir direction, U32 variation)
  {
    U32 blend = 0, dir = 0;

    // Get the data cell
    Cell &cell = GetCell(x, z);

    // Get the blend and direction
    GetBlendInfo(x, z, cell, blend, dir);

    // Get the terrain type at this location
    TerrainGroup::Type &type = TerrainGroup::GetActiveType(cell.terrain);

    // Wrap the variation (count is specific to the type)
    variation %= type.GetVariations(blend);

    // Wrap the direction
    if (direction >= BD_MAXIMUM)
    {
      direction = BD_NORTH;
    }

    // Request index of texture
    U32 index = Terrain::AddTexture(type.BlendName(blend, variation));

    // Get the terrain cell
    ::Cell *tCell = Terrain::GetCell(x, z);

    // Set the texture
    tCell->texture = (U8)index;

    // Set the rotation using the uv entry for this direction
    tCell->uv = UVTable[direction];
  }


  //
  // SetupBlendTable
  //
  // Sets up the cell blending tables
  //
  static void SetupBlendTable()
  {
    // Default all table entries to invalid
    for (U32 b = 0; b < TerrainGroup::BLEND_COUNT; b++)
    {
      for (U32 d = 0; d < MAX_BLENDFLAG; d++)
      {
        blendTable[b][d] = BD_MAXIMUM;
      }
    }

    //
    // There is a piece of paper somewhere that has all this on
    // it! Basically the first index is a blend type, and the
    // second index is a flag value relating to neighbors of
    // the same terrain type (generated in GetBlendInfo())
    //

    // Setup the stright
    blendTable[TerrainGroup::BLEND_STRAIGHT][11] = BD_NORTH;
    blendTable[TerrainGroup::BLEND_STRAIGHT][13] = BD_EAST;
    blendTable[TerrainGroup::BLEND_STRAIGHT][14] = BD_SOUTH;
    blendTable[TerrainGroup::BLEND_STRAIGHT][7]  = BD_WEST;

    // Setup the corner
    blendTable[TerrainGroup::BLEND_CORNER][9]  = BD_NORTH;
    blendTable[TerrainGroup::BLEND_CORNER][12] = BD_EAST;
    blendTable[TerrainGroup::BLEND_CORNER][6]  = BD_SOUTH;
    blendTable[TerrainGroup::BLEND_CORNER][3]  = BD_WEST;

    // Setup the bend
    blendTable[TerrainGroup::BLEND_BEND][13] = BD_NORTH;
    blendTable[TerrainGroup::BLEND_BEND][14] = BD_EAST;
    blendTable[TerrainGroup::BLEND_BEND][7]  = BD_SOUTH;
    blendTable[TerrainGroup::BLEND_BEND][11] = BD_WEST;
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized);

    // No cell modification session active
    sessionActive = FALSE;

    // Setup the cell blending table
    SetupBlendTable();

    // How many cells do we need to allocate
    cellMapSize = WorldCtrl::CellMapX() * WorldCtrl::CellMapZ();

    // Allocate the map
    cellMap = new Cell[cellMapSize];

    // Set all data members to zero
    Utils::Memset(cellMap, 0, cellMapSize * sizeof(Cell));

    // Allocate the bit arrays
    bitArrayPaint = new BitArray2d(WorldCtrl::CellMapX(), WorldCtrl::CellMapZ());
    bitArraySecondLayer = new BitArray2d(WorldCtrl::CellMapX(), WorldCtrl::CellMapZ());
    bitArraySession = new BitArray2d(WorldCtrl::CellMapX(), WorldCtrl::CellMapZ());
    bitArrayBlockLOS = new BitArray2d(WorldCtrl::CellMapX(), WorldCtrl::CellMapZ());

    // System now initialized
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

    // Dispose of dynamic data
    delete [] cellMap;
    delete bitArrayPaint;
    delete bitArraySecondLayer;
    delete bitArraySession;
    delete bitArrayBlockLOS;

    // System now shutdown
    initialized = FALSE;
  }


  //
  // Save
  // 
  // Save data to a blockfile
  //
  void Save(BlockFile &bFile)
  {
    ASSERT(initialized);

    SaveCell saveCell;

    // Open the terrain data block
    bFile.OpenBlock(cellBlockName);

    // Start at first cell
    Cell *cell = cellMap;

    // Save each cell
    for (U32 c = 0; c < cellMapSize; c++, cell++)
    {
      // Setup the data to be saved
      saveCell.terrain = cell->terrain;

      // Write out the cell
      bFile.WriteToBlock(&saveCell, sizeof(SaveCell));
    }

    // Close the open block
    bFile.CloseBlock();

    // Open the terrain mapping block
    bFile.OpenBlock(mappingBlockName);

    // Get the active group
    TerrainGroup::Group &g = TerrainGroup::ActiveGroup();

    // Save the crc of the group name
    U32 data = g.NameCrc();
    bFile.WriteToBlock(&data, sizeof(data));

    // Save the number of types
    data = g.GetTypeList().GetCount();
    bFile.WriteToBlock(&data, sizeof(data));

    // Save each type crc
    for (List<TerrainGroup::Type>::Iterator i(&g.GetTypeList()); *i; i++)
    {
      data = (*i)->NameCrc();
      bFile.WriteToBlock(&data, sizeof(data));
    }

    // Close the open block
    bFile.CloseBlock();
  }
 

  //
  // UpdateCellSurface
  //
  // Update the water/slope of a cell
  //
  static void UpdateCellSurface(Cell &cell, U32 cx, U32 cz)
  {
    Vector normal;
    F32 terrain, water;
    Bool foundWater = FALSE;
   
    // Get the metre location
    F32 mx = WorldCtrl::CellToMetresX(cx);
    F32 mz = WorldCtrl::CellToMetresZ(cz);

    // Find the floor at this location
    terrain = TerrainData::FindFloor(mx, mz, &normal);

    // Setup water surfaces
    if (TerrainData::FindWater(mx, mz, &water))
    {
      F32 h[4];
      F32 highest = F32_MIN;
      F32 lowest = F32_MAX;

      TerrainData::CellHeights(cx, cz, h);

      // Find shallowest corner
      for (U32 i = 0; i < 4; i++)
      {
        h[i] -= water;

        if (lowest > h[i])
        {
          lowest = h[i];
        }
        if (highest < h[i])
        {
          highest = h[i];
        }
      }

      // If cell intersects water, set to shallow
      if (Utils::FSign(highest) != Utils::FSign(lowest))
      {
        // Apply the slope of the terrain for shore line
        if (lowest < -DEEP_WATER)  
        { 
          cell.surface = dShore;
        } 
        else

        if (lowest < -MEDIUM_WATER) 
        { 
          cell.surface = mShore;
        } 
        else

        if (lowest < -SHALLOW_WATER)          
        { 
          cell.surface = sShore; 
        }
      }
      else
      {
        if (lowest < -DEEP_WATER)  
        { 
          cell.surface = dWater;
          foundWater = TRUE;
        } 
        else

        if (lowest < -MEDIUM_WATER) 
        { 
          cell.surface = mWater; 
          foundWater = TRUE;
        } 
        else

        if (lowest < -SHALLOW_WATER)          
        { 
          // Apply the slope of the terrain instead of resetting to 0
          cell.surface = sWater; 
        }
      }
    }

    // Update the slope of the cell
    if (foundWater)
    {
      cell.slope = 0;
    }
    else
    {
      Vector v(normal);
      normal.y = 0;
      normal.Normalize();
      cell.slope = (U8)(asin(fabs(v.Dot(normal))) * RAD2DEG);
    }
  }


  //
  // SetupExtendedInfo
  //
  // Setup the extended information recorded for each cell
  //
  void SetupExtendedInfo()
  {
    // Only give warnings once
    Bool warnTerrain = TRUE;

    // Surface modifications
    sWater = MoveTable::SurfaceIndex("ShallowWater");
    mWater = MoveTable::SurfaceIndex("MediumWater");
    dWater = MoveTable::SurfaceIndex("DeepWater");
    sShore  = MoveTable::SurfaceIndex("ShallowShoreline");
    mShore  = MoveTable::SurfaceIndex("MediumShoreline");
    dShore  = MoveTable::SurfaceIndex("DeepShoreline");

    // Do modifications for each cell
    for (S32 z = 0; z < (S32)WorldCtrl::CellMapZ(); z++)
    {
      for (S32 x = 0; x < (S32)WorldCtrl::CellMapX(); x++)
      {
        // Get the cell at this location
        Cell &cell = GetCell(x, z);

        // Check the terrain type
        if (cell.terrain >= TerrainGroup::ActiveTypeCount())
        {
          // Warn the user
          if (warnTerrain)
          {
            LOG_WARN(("Terrain contained type indexes that are no longer available"));
            LOG_WARN((" - Cells will need to be re-blended!!"));
            warnTerrain = FALSE;
          }

          // Invalid, so set to the base type
          cell.terrain = 0;
        }

        // Get type at this location
        TerrainGroup::Type &type = TerrainGroup::GetActiveType(cell.terrain);

        // Set the surface type
        cell.surface = type.Surface();

        // Recalculate slope and water surface type
        UpdateCellSurface(cell, x, z);
      }
    }
  }


  //
  // Load
  // 
  // Load data from a block file
  //
  void Load(BlockFile &bFile)
  {
    ASSERT(initialized);

    SaveCell saveCell;

    //LOG_DIAG(("Loading terrain block (%dx%d - %d / %d)", WorldCtrl::CellMapX(), WorldCtrl::CellMapZ(), cellMapSize, sizeof(SaveCell)));

    // The number of entries in the mapping table
    U32 mappingEntryCount = 0;

    // The mapping array
    U32 *mappingEntries = NULL;

    // Get the active group
    TerrainGroup::Group &activeGroup = TerrainGroup::ActiveGroup();
  
    // Open the terrain mapping block
    if (bFile.OpenBlock(mappingBlockName, FALSE))
    {
      // Load the crc of the group name
      U32 group;
      bFile.ReadFromBlock(&group, sizeof(U32));

      // Does the group match
      if (activeGroup.NameCrc() == group)
      {
        // Load the number of types
        bFile.ReadFromBlock(&mappingEntryCount, sizeof(U32));

        //LOG_DIAG((" - Loaded mapping table (%d entries)", mappingEntryCount));

        if (mappingEntryCount)
        {
          // Allocate the table
          mappingEntries = new U32[mappingEntryCount];

          // Load the table
          bFile.ReadFromBlock(mappingEntries, sizeof(U32) * mappingEntryCount);
        }
      }
      else
      {
        LOG_WARN(("TerrainData: Ignoring mapping table because world type has changed"));
      }

      // Close the open block
      bFile.CloseBlock();
    }

    // Open the terrain data block
    if (bFile.OpenBlock(cellBlockName, FALSE))
    {
      // Calculate expected block size
      U32 expectSize = cellMapSize * sizeof(SaveCell);

      // Get block size
      U32 blockSize = bFile.SizeOfBlock(Crc::CalcStr(cellBlockName));

      // Mapping counts
      U32 cellsMapped = 0, cellsNotMapped = 0;

      // Ensure correct size
      if (blockSize == expectSize)
      {
        // Start at first cell
        Cell *cell = cellMap;

        // Load each cell
        for (U32 c = 0; c < cellMapSize; c++, cell++)
        {
          // Read in the cell
          bFile.ReadFromBlock(&saveCell, sizeof(SaveCell));

          // Do we have a mapping table and a valid index
          if (saveCell.terrain < mappingEntryCount)
          {
            // Get the mapping crc
            U32 crc = mappingEntries[saveCell.terrain];
            
            if 
            (
              // Index is out of range
              (saveCell.terrain >= TerrainGroup::ActiveTypeCount()) || 
              
              // Name does not match
              (crc != TerrainGroup::GetActiveType(saveCell.terrain).NameCrc())
            )
            {
              // Attempt to find the old type in the active group
              TerrainGroup::Type *type = activeGroup.FindType(crc);

              if (type)
              {
                // Change to the new index
                saveCell.terrain = type->Index();
                cellsMapped++;
              }
              else
              {
                cellsNotMapped++;
              }
            }
          }

          // Setup the cell
          cell->terrain = saveCell.terrain;
        }
      }
      else
      {
        LOG_WARN(("TerrainData: Failed : (Incorrect size %u/%u)", blockSize, expectSize));
      }

      // Were there any invalid mapping values
      if (cellsNotMapped)
      {
        LOG_WARN(("TerrainData: Config changed (%d / %d)", cellsMapped, cellsNotMapped));
      }
      else

      if (cellsMapped)
      {
        LOG_DIAG(("TerrainData: Config changed (%d / %d)", cellsMapped, cellsNotMapped));
      }

      // Close the terrain data block
      bFile.CloseBlock();

      // Setup info
      SetupExtendedInfo();
    }
    else
    {
      LOG_WARN(("TerrainData: Failed opening data block (%s)", bFile.LastError()));     
    }

    // Delete the mapping table
    if (mappingEntries)
    {
      delete mappingEntries;
    }
  }


  //
  // Blend
  //
  // Blend the given dirty list
  //
  static void Blend(PointList &dirty)
  {
    // Set all orphan cells to base type
    KillOrphans(dirty);

    // Blend the dirty cells
    BlendCells(dirty);

    // Dispose of dirty cells
    dirty.DisposeAll();   
  }


  //
  // Paint
  //
  // Paint a terrain type over the specified area (inclusive)
  //
  void Paint(const PaintList &paintList)
  { 
    // Ignore empty lists
    if (paintList.GetCount())
    {
      PointList dirty;

      // Place terrain type and generate dirty list
      PlaceTerrain(paintList, dirty);
  
      // Blend the dirty cells
      Blend(dirty);
    }
  }


  //
  // BlendEntireMap
  //
  // Re-blend the entire map
  //
  void BlendEntireMap()
  {
    PointList dirty;

    // Add all map cells to the dirty list
    for (S32 z = 0; z < S32(WorldCtrl::CellMapZ()); z++)
    {
      for (S32 x = 0; x < S32(WorldCtrl::CellMapX()); x++)
      {       
        // Add to the list
        dirty.Append(new NPoint(x, z));
      }
    }

    // Blend the dirty cells
    Blend(dirty);
  }


  //
  // Paint
  //
  // Fill a paint list using the given area
  //
  void GeneratePaintList(Point<S32> a, Point<S32> b, PaintList &paintList, U8 type)
  { 
    // Ensure sorted points
    if (a.x > b.x) { Swap(a.x, b.x); }
    if (a.z > b.z) { Swap(a.z, b.z); }  

    // Add each cell in the area to a list
    for (S32 z = a.z; z <= b.z; z++)
    {
      for (S32 x = a.x; x <= b.x; x++)
      {       
        paintList.Append(new PaintCell(x, z, type));
      }
    }
  }


  //
  // GetFootPrintInstance
  //
  // Returns the footprint instance at the given map location (must exist)
  //
  FootPrint::Instance & GetFootPrintInstance(S32 x, S32 z)
  {
    // Get the data cell
    TerrainData::Cell &cell = TerrainData::GetCell(x, z);

    // Get the footprint instance
    return (FootPrint::GetInstance(cell.footIndex));
  }


  //
  // FindFootPrintInstance
  //
  // Returns the footprint instance at the map location, or NULL if none there
  //
  FootPrint::Instance * FindFootPrintInstance(S32 x, S32 z)
  {
    // Get the data cell
    TerrainData::Cell &cell = TerrainData::GetCell(x, z);

    // Is there an instance here
    if (FootPrint::ValidInstanceIndex(cell.footIndex))
    {
      return (&FootPrint::GetInstance(cell.footIndex));
    }

    return (NULL);
  }


  //
  // SessionAddCell
  //
  // Add the given cell to the session list
  //
  static void SessionAddCell(S32 x, S32 z)
  {
    // Is this point already added
    if (!bitArraySession->Get2(x, z))
    {
      // Add to the list
      sessionList.Append(new NPoint(x, z));

      // Set the dirty bit
      bitArraySession->Set2(x, z);  
    }
  }


  //
  // SessionAddCellArea
  //
  // Add the given cell and neighbors to the session list
  //
  static void SessionAddCellArea(S32 x, S32 z)
  {
    ASSERT(sessionActive);
    ASSERT(WorldCtrl::CellOnMap(x, z));

    // Dirty the cell and neighbors
    for (S32 zi = z - 1; zi <= z; zi++)
    {
      for (S32 xi = x - 1; xi <= x; xi++)
      {       
        if (WorldCtrl::CellOnMap(xi, zi))
        {
          SessionAddCell(xi, zi);
        }
      }
    }
  }


  //
  // SessionStart
  //
  // Start a run of cell modifications
  //
  void SessionStart()
  {
    ASSERT(!sessionActive);
    ASSERT(!sessionList.GetCount());

    // Use the bit array to only add a cell once
    bitArraySession->Reset();

    // Reset session flags
    sessionHeight = FALSE;
    sessionLayer = FALSE;

    // Session now active
    sessionActive = TRUE;
  }


  //
  // SessionEnd
  //
  // End a run of cell modifications
  //
  void SessionEnd()
  {
    ASSERT(sessionActive);

    // Were any points changed
    if (sessionList.GetCount())
    {
      // Calculate bounding rectangle
      Point<S32> min = *sessionList.GetHead();
      Point<S32> max = min;

      // Go thru each point in the list
      for (PointList::Iterator i(&sessionList); *i; i++)
      {
        // Get the position of this cell
        NPoint &p = **i;

        // Update min and max
        if (p.x <= min.x && p.z <= min.z) { min = p; }
        if (p.x >= max.x && p.z >= max.z) { max = p; }

        // Do we need to update the slope of the cell
        if (sessionHeight || sessionLayer)
        {
          // Recalculate slope and water surface type
          UpdateCellSurface(GetCell(p.x, p.z), p.x, p.z);
        }
      }

      // Realign all objects in the region
      UnitObjIter::ClaimRect itr(min, max);
      UnitObj *obj;

      while ((obj = itr.Next()) != NULL)
      {
        if (obj->CanEverMove())
        {
          Matrix m = obj->WorldMatrix();
          obj->GetDriver()->AlignObject(m.front, m);
          obj->SetSimCurrent(m);
        }
      }   

      // Do we need to notify terrain system of change
      if (sessionHeight)
      {
        // Increase both dimensions by one to cater for edge of map
        max.x++;
        max.z++;

        // Generate a sorted rectangle
        Area<S32> rect( min, max);
        rect.Sort();

        // Recalculate terrain data
        Terrain::CalcCellRect( rect);
      }

      // Dispose of all points
      sessionList.DisposeAll();
    }

    // This session is done
    sessionActive = FALSE;
  }


  //
  // SessionModifyHeight
  //
  // Change the height of the given cell
  //
  void SessionModifyHeight(U32 x, U32 z, F32 height)
  {
    ASSERT(sessionActive);

    // Set height flag
    sessionHeight = TRUE;

    // Get the terrain cell at this location
    ::Cell *cell = Terrain::GetCell(x, z);

    // Modify the height
    cell->SetHeight(height);

    // Add to session list
    SessionAddCellArea(x, z);
  }


  //
  // SessionModifySecondLayer
  //
  // Set this second layer flag for this cell
  //
  void SessionModifySecondLayer(U32 x, U32 z, Bool toggle)
  {
    ASSERT(sessionActive);

    // Set layer flag
    sessionLayer = TRUE;

    // Modify the layer flag
    if (toggle)
    {
      bitArraySecondLayer->Set2(x, z);
    }
    else
    {
      bitArraySecondLayer->Clear2(x, z);
    }

    // Add the cell to the session list
    SessionAddCell(x, z);
  }


  //
  // SessionModifyBlockLOS
  //
  // Set this block LOS flag for this cell
  //
  void SessionModifyBlockLOS(U32 x, U32 z, Bool toggle)
  {
    ASSERT(sessionActive);

    // Modify the layer flag
    if (toggle)
    {
      bitArrayBlockLOS->Set2(x, z);
    }
    else
    {
      bitArrayBlockLOS->Clear2(x, z);
    }

    // Add the cell to the session list
    SessionAddCell(x, z);
  }


  //
  // SessionAddArea
  //
  // Add all cells in the given area (points must be sorted)
  //
  void SessionAddArea(const Point<S32> &a, const Point<S32> &b)
  {
    ASSERT(sessionActive);
    ASSERT(a.x <= b.x);
    ASSERT(a.z <= b.z);

    // Set height flag
    sessionHeight = TRUE;

    // Add each affected cell 
    for (S32 z = a.z - 1; z <= b.z; z++)
    {
      for (S32 x = a.x - 1; x <= b.x; x++)
      {       
        if (WorldCtrl::CellOnMap(x, z))
        {
          SessionAddCell(x, z);
        }
      }
    }
  }


  //
  // RestoreSurfaceType
  //
  // Restore the original surface type based on the terrain type and water
  //
  void RestoreSurfaceType(S32 x, S32 z)
  {
    // Get the map cell at this location
    TerrainData::Cell &cell = TerrainData::GetCell(x, z);

    // Get the terrain type
    TerrainGroup::Type &type = TerrainGroup::GetActiveType(cell.terrain);

    // Change the surface back to the original
    cell.surface = type.Surface();   

    // And update for water etc
    UpdateCellSurface(cell, x, z);
  }


  //
  // IsWater
  //
  // Is this surface type water ?
  //
  Bool IsWater(U8 surface)
  {
    ASSERT(initialized)
    return (surface == sWater || surface == mWater || surface == dWater);
  }


  // including footprinted objects
  //
  F32 FindFloor( F32 x, F32 z, Vector *surfNormal) // = NULL)
  {
    S32 cx, cz;

    ASSERT( WorldCtrl::MetreOnMap(x, z));

    if (WorldCtrl::MetreOnMap(x, z))
    {
      cx = WorldCtrl::MetresToCellX( x);
      cz = WorldCtrl::MetresToCellZ( z);

      if (TerrainData::UseSecondLayer(cx, cz))
      {
        return FootPrint::FindFloor(x, z, cx, cz, surfNormal);
      }
 
      return Terrain::FindFloor(x, z, surfNormal);
    }

    return (F32_MAX);
  }

  // including footprinted objects
  //
  F32 FindFloorWithWater( F32 x, F32 z, Vector *surfNormal) // = NULL)
  {
    S32 cx, cz;

    ASSERT( WorldCtrl::MetreOnMap(x, z));

    if (WorldCtrl::MetreOnMap(x, z))
    {
      cx = WorldCtrl::MetresToCellX( x); // _ftol
      cz = WorldCtrl::MetresToCellZ( z);

      if (TerrainData::UseSecondLayer(cx, cz))
      {
        return FootPrint::FindFloor(x, z, cx, cz, surfNormal);
      }
      return Terrain::FindFloorWithWater(x, z, surfNormal);
    }

    return (F32_MAX);
  }

  // including footprinted objects
  // doesn't check for valid cell coords
  //
  F32 GetHeight( S32 cx, S32 cz)
  {
    if (WorldCtrl::CellOnMap(cx, cz) && UseSecondLayer(cx, cz))
    {
      return FootPrint::GetHeight(cx, cz);
    }
    return Terrain::GetHeight( cx, cz);;
  }


  // including footprinted objects
  // doesn't check for valid cell coords
  //
  F32 GetHeightWithWater( S32 cx, S32 cz)
  {
    if (WorldCtrl::CellOnMap(cx, cz) && UseSecondLayer(cx, cz))
    {
      return FootPrint::GetHeight(cx, cz);
    }
    ASSERT(Terrain::CellOnMap(cx, cz))
    return Terrain::GetHeightWithWater( cx, cz);;
  }
}

