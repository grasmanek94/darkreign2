///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Line Of Sight System
//
// 24-NOV-1998
//

#include "vid_private.h"
#include "terrain_priv.h"

#include "sight.h"
#include "sight_map.h"
#include "sight_bytemap.h"
#include "sight_tables.h"
#include "utils.h"
#include "team.h"
#include "worldctrl.h"
#include "unitobj.h"
#include "icgridwindow.h"
#include "console.h"
#include "mapobjctrl.h"
#include "promote.h"
#include "gametime.h"
#include "perfstats.h"
#include "iface.h"
#include "sync.h"
#include "common.h"
#include "game.h"

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sight - The line of sight system
//
namespace Sight
{

  //
  // Macros
  //

  // Prefetch memory
  volatile U32 __prefetch;

  #define PREFETCH(x) __prefetch = *(int *)((void *)x)

  // Start/stop timers
  #ifdef DEVELOPMENT

  #define START(x) x.Start()
  #define STOP(x)  x.Stop()

  #else

  #define START(x)
  #define STOP(x)

  #endif


  #define FOG_DELTA_MIN         .01f
  #define FOG_FILTER_FACTOR     .1f
  #define FOG_FILTER_INVERSE    (FOG_FILTER_FACTOR - 1)

  //
  // Constants
  //

  // Number of units seeing this cell is in lower 15 bits
  const U16 SEEINGMASK = 0x7FFF;

  // Whether seen ever in high bit.
  const U16 SEENMASK = 0x8000;

  // Maximum viewing radius, in metres
  const F32 MAXR_METRES = MAXR * WC_CELLSIZE;

  // Height above the terrain that constitues being able to see that cell
  const F32 VisAlt = 2.0F;

  const U8 FOG_MAX = 6;

  // Name of save game block
  const U32 SAVEBLOCK = 0xFF8A262D; // "Sight::Seen"

  // Has the system been initialized
  static Bool sysInit = FALSE;

  // Editor mode?
  static Bool editMode;

  // Sight map
  static U16 **seeMap[Map::LV_MAX][Game::MAX_TEAMS];

  DEBUG_STATIC_GUARD_BLOCK;

  // Clusters that sight has changed within
  static BitArray2d *displayDirtyCells = NULL;
  static U8 *teamDirtyClust[Game::MAX_TEAMS];

  DEBUG_STATIC_GUARD_BLOCK;

  static U32 mapClustXon8;

  // Gradient table
  static F32 invRadTbl[MAXR];

  DEBUG_STATIC_GUARD_BLOCK;

  // Remap team id's (possibly non-sequential) to sequential index
  static U8 teamRemap[Game::MAX_TEAMS];

  DEBUG_STATIC_GUARD_BLOCK;

  // Remap sequential index back to team id
  static U8 invTeamRemap[Game::MAX_TEAMS];

  DEBUG_STATIC_GUARD_BLOCK;

  // Last rescan time for this team
  static U32 teamLastRescan[Game::MAX_TEAMS];

  DEBUG_STATIC_GUARD_BLOCK;

  // Number of teams in use
  static U32 teamCount;

  // Time till next display
  static S32 nextDisplay;

  // Detached LOS maps
  struct Detached
  {
    // Cycle to destroy map on
    U32 killme;

    // Map to destroy
    Map *map;

    // List node obviously
    NList<Detached>::Node node;

    // Constructor
    Detached(U32 cycle, Map *map) : killme(cycle), map(map) {}
  };

  static NList<Detached> detachedList(&Detached::node);

  #ifdef DEVELOPMENT

  // Fog display mode
  VarInteger fog0 ,fog1;
  VarInteger debugMode;
  VarInteger debugScan;

  // Debugging stuff
  Clock::CycleWatch sweepTime;
  Clock::CycleWatch unSweepTime;
  Clock::CycleWatch updateDisp;
  Clock::CycleWatch dirtyCellTime;

  #endif

  F32 r2Inv;

  // Command handler
  void CmdHandler(U32 pathCrc);

  // Show all units?
  Bool showAllUnits = FALSE;


  //
  // Dirty Cells that sight has changed within
  //
  void DirtyCells(S32 minX, S32 minZ, S32 maxX, S32 maxZ, Game::TeamBitfield teamBits)
  {
    START(dirtyCellTime);

    S32 clustMinX = Clamp<S32>(0, minX >> S32(WC_CLUSTERSIZECELLSHIFT), WorldCtrl::ClusterMapX()-1);
    S32 clustMinZ = Clamp<S32>(0, minZ >> S32(WC_CLUSTERSIZECELLSHIFT), WorldCtrl::ClusterMapZ()-1);
    S32 clustMaxX = Clamp<S32>(0, maxX >> S32(WC_CLUSTERSIZECELLSHIFT), WorldCtrl::ClusterMapX()-1);
    S32 clustMaxZ = Clamp<S32>(0, maxZ >> S32(WC_CLUSTERSIZECELLSHIFT), WorldCtrl::ClusterMapZ()-1);

    U32 index = (clustMinZ * mapClustXon8);

    // The dirty cluster array has 1 bit per cluster, arranged as
    // 1 byte per 8 clusters along the x axis.
    for (S32 z = clustMinZ; z <= clustMaxZ; z++)
    {
      for (S32 x = clustMinX; x <= clustMaxX; x++)
      {
        U32 elem = index + (x >> 3);
        U8  mask = U8(1 << (x & 7));

        // The byte offset is (x >> 3) and the bit within that 
        // byte is (1 << (x & 7))
        for (U32 bits = teamBits, team = 0; bits; ++team, bits >>= 1)
        {
          if (bits & 1)
          {
            ASSERT(team < teamCount)
            teamDirtyClust[team][elem] |= mask;
          }
        }
      }

      // Move to next row
      index += mapClustXon8;
    }

    // The display side dirty cell array has one bit per cell
    if (Team::GetDisplayTeam())
    {
      if (teamBits & (1 << teamRemap[Team::GetDisplayTeam()->GetId()]))
      {
        S32 cminX = Max<S32>(0, minX);
        S32 cminZ = Max<S32>(0, minZ);
        S32 cmaxX = Min<S32>(maxX, WorldCtrl::CellMapX()-1);
        S32 cmaxZ = Min<S32>(maxZ, WorldCtrl::CellMapZ()-1);
        S32 pitch = (displayDirtyCells->Pitch() << 3);

        for (S32 cz = cminZ; cz <= cmaxZ; cz++)
        {
          S32 zoffset = cz * pitch;

          for (S32 cx = cminX; cx <= cmaxX; cx++)
          {
            displayDirtyCells->Set1(zoffset + cx);
          }
        }
      }
    }

    STOP(dirtyCellTime);
  }


  //
  // Dirty all cells on the map
  //
  static void DirtyAllCells()
  {
    Game::TeamBitfield bits = 0;

    for (U32 t = 0; t < teamCount; t++)
    {
      bits |= (1 << t);
    }

    DirtyCells(0, 0, WorldCtrl::CellMapX()-1, WorldCtrl::CellMapZ()-1, bits);
  }


  //
  // Return maximum seeing range in cells
  //
  U32 MaxRangeCells()
  {
    return (MAXR);
  }


  //
  // Return maximum seeing range in metres
  //
  F32 MaxRangeMetres()
  {
    return (MAXR_METRES);
  }


  //
  // Save
  //
  // Save system data
  //
  void Save(BlockFile &bFile)
  {
    // Size of required buffer
    U32 bufferSize = WorldCtrl::CellMapX() * WorldCtrl::CellMapZ();

    // Allocate a buffer
    Game::TeamBitfield *buffer = new Game::TeamBitfield[bufferSize];
    Game::TeamBitfield *ptr = buffer;

    // Iterate the seen map
    for (U32 i = 0; i < bufferSize; ++i)
    {
      // Clear the data for this cell
      *ptr = 0;

      // Now set a bit for each team that has seen this cell
      for (U32 team = 0; team < teamCount; team++)
      {
        if (seeMap[0][team][0][i] & SEENMASK)
        {
          Game::TeamSet(*ptr, team);
        }
      }

      // Move to next cell
      ++ptr;
    }

    // Save the buffer to the blockfile
    bFile.OpenBlock(SAVEBLOCK);
    bFile.WriteToBlock(buffer, bufferSize * sizeof(Game::TeamBitfield));
    bFile.CloseBlock();    

    // Free up the buffer
    delete [] buffer;
  }


  //
  // Load
  //
  // Load system data
  //
  void Load(BlockFile &bFile)
  {
    // Get the size of the block
    U32 size;

    // Open the data block
    bFile.OpenBlock(SAVEBLOCK, TRUE, &size);
  
    // Expected size of buffer
    U32 bufferSize = WorldCtrl::CellMapX() * WorldCtrl::CellMapZ();

    // Check the block size
    if (bufferSize == size)
    {
      // Get the start of the buffer
      Game::TeamBitfield *ptr = (Game::TeamBitfield *)(bFile.GetBlockPtr());

      // Iterate through each cell
      for (U32 i = 0; i < bufferSize; ++i)
      {
        // Setup the team bits
        for (U32 team = 0; team < teamCount; team++)
        {
          if (Game::TeamTest(*ptr, team))
          {
            seeMap[0][team][0][i] |= SEENMASK;
          }
        }

        // Move to next cell
        ++ptr;
      }
    }
    else
    {
      LOG_WARN(("Sight: Save block was unexpected size (%d/%d)", size, bufferSize));
    }

    // Done 
    bFile.CloseBlock();

    // Ensure display gets updated for new data
    DirtyAllCells();
  }


  //
  // Return debug mode flag
  //
  #ifdef DEVELOPMENT

  Bool DebugMode()
  {
    return (debugMode);
  }

  #endif


  //
  // Convert XZ coordinates to see map index
  //
  static inline XZToSeemap(S32 x, S32 z)
  {
    return ((z + MAXR - 1) * MAPSIDE + x + MAXR - 1);
  }


  //
  // Return the altitude in metres at cell x,z
  //
  static F32 GetAltitude(S32 x, S32 z, UnitObj *filterUnit)
  {
    if (WorldCtrl::CellOnMap(x, z))
    {
      F32 h[4];

      if (TerrainData::BlockLOS(x, z))
      {
        FootPrint::Instance &foot = TerrainData::GetFootPrintInstance(x, z);

        // filter out unit that is providing the blocking LOS
        if (!(filterUnit && (foot.GetObj().Id() == filterUnit->Id())))
        {
          return (foot.GetLOSBlockHeight());
        }
      }

      // Otherwise use cell heights
      TerrainData::CellHeights(x, z, h);

      // Check for water
      F32 waterHeight;

      if (TerrainData::GetWater(x, z, &waterHeight))
      {
        if (h[0] < waterHeight) h[0] = waterHeight;
        if (h[1] < waterHeight) h[1] = waterHeight;
        if (h[2] < waterHeight) h[2] = waterHeight;
        if (h[3] < waterHeight) h[3] = waterHeight;
      }

      // Return average height of 4 corners
      return ((h[0] + h[1] + h[2] + h[3]) * 0.25F);
    }
    else
    {
      return (0.0F);
    }
  }


  //
  // Scan around a unit, adding 1 to the seeing map and setting the seen flag.
  //
  static void CanSee(S32 x, S32 z, S32 dx, S32 dz, U8* b, U8 bitmask, Game::TeamBitfield teamBits, U32 level)
  {
    if (!WorldCtrl::CellOnMap(x, z))
    {
      return;
    }

    // Set bit in unit's seeing map
    U32 index = XZToSeemap(dx, dz);

    // Make sure not already swept
    ASSERT(!(b[index] & bitmask))

    b[index] |= bitmask;

    // Update map for all teams
    for (U32 team = 0; teamBits; ++team, teamBits >>= 1)
    {
      if (teamBits & 1)
      {
        ASSERT(team < teamCount)

        U16 * seeingMapEntry = &seeMap[level][team][z][x];

        // Increase count in seeing map
        (*seeingMapEntry)++;

        // Set bit in seen map
        (*seeingMapEntry) |= SEENMASK;
      }
    }
  }

  
  //
  // Decrement the number of units that can see the tile
  //
  static void CantSee(S32 x, S32 z, S32 dx, S32 dz, Game::TeamBitfield teamBits, U32 level)
  {
    // Note: no need to check x,y against map boundaries as they
    // are set according to the last seemap with which clipping
    // has already been done.

    dx;
    dz;

    // update seeing map for all allied teams
    for (U32 team = 0; teamBits; ++team, teamBits >>= 1)
    {
      if (teamBits & 1)
      {
        ASSERT(team < teamCount)

        // decrease seeing count
        seeMap[level][team][z][x]--;
      }
    }
  }


  //
  // Set all terrain cell fog values
  //
  static void SetAllFog( S32 f)
  {
    Cell *pCell = Terrain::GetCell(0, 0);

    U32 w = Terrain::CellWidth();
    U32 h = Terrain::CellHeight();
    U32 s = Terrain::CellPitch() - Terrain::CellWidth();

    for (U32 z = 0; z < h; ++z)
    {
      for (U32 x = 0; x < w; ++x)
      {
        pCell->SetFog(f);
        pCell++;
      }
      pCell += s;
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Sweeper - The Sweep function and other local variables
  //                     (to cut down on parameter passing between functions)
  //
  namespace Sweeper
  {
    // Quadrant constants
    const S32 NEG = -1;
    const S32 POS =  1;

    // Gradient map of terrain
    F32 gradMap[MAXR][MAXR];

    // Gradient map visalt metres above terrain
    F32 gradMapU[MAXR][MAXR];

    // Maximum gradient map for calculating obscuring gradients
    F32 maxGradMap[MAXR][MAXR];

    F32 *gMap = &gradMap[0][0];
    F32 *maxgMap = &maxGradMap[0][0];
    F32 g, maxg;

    // Current unit
    UnitObj *unit;

    // Current unit's pos and view radius
    S32 cellX;
    S32 cellZ;
    S32 r;
    S32 r2;

    // World Y position of unit's eye
    F32 eyePos;

    // Sight map and mask
    U8 maskLo;
    U8 *mapLo;

    // Teams that LOS will be provided for
    Game::TeamBitfield teamBits;

    // Loop variables
    S32 x, z, x2, z2, dx, dz, idx;

    // Quadrant being processed
    S32 quadrantX;
    S32 quadrantZ;


    //
    // Fill gradient map for given quadrant
    //
    void FillGradMap()
    {
      for (z = 0, z2 = 0, dz = 0; z2 < r2; z2 += 2 * z + 1, ++z, dz += quadrantZ)
      {
        for (x = 0, x2 = 0, dx = 0; x2 + z2 < r2; x2 += 2 * x + 1, ++x, dx += quadrantX)
        {
          // Calculate the altitude difference at the cell x,z
          F32 altd = GetAltitude(cellX + dx, cellZ + dz, unit) - eyePos;

          idx = x > z ? x : z;

          // Gradient on ground at cell x,y
          gradMap[z][x] = invRadTbl[idx] * altd;

          // Gradient "visibleAlt" metres above ground
          gradMapU[z][x] = invRadTbl[idx] * (altd + VisAlt);
        }
      }

      maxGradMap[0][0] = gradMap[0][0];
    }


    //
    // Fill gradient map for given quadrant, rotated 90 degrees
    //
    void FillGradMapRotated()
    {
      for (z = 0, z2 = 0, dz = 0; z2 < r2; z2 += 2 * z + 1, ++z, dz += quadrantZ)
      {
        for (x = 0, x2 = 0, dx = 0; x2 + z2 < r2; x2 += 2 * x + 1, ++x, dx += quadrantX)
        {
          // Calculate the altitude difference at the cell x,z
          F32 altd = GetAltitude(cellX - dz, cellZ - dx, unit) - eyePos;

          idx = x > z ? x : z;

          // Gradient on ground at cell x,y
          gradMap[z][x] = invRadTbl[idx] * altd;

          // Gradient "visibleAlt" metres above ground
          gradMapU[z][x] = invRadTbl[idx] * (altd + VisAlt);
        }
      }

      maxGradMap[0][0] = gradMap[0][0];
    }


    //
    // Fill Max Gradient map using traversal info along positive z axis
    //
    void FillMaxGradMapPosZ()
    {
      for (z = 0, z2 = 0; z2 < r2; z2 += 2 * z + 1, ++z)
      {
        for (x = 0, x2 = 0; x2 + z2 < r2; x2 += 2 * x + 1, ++x)
        {
          TravInfo* trav = &posYTravInfo[z][x];
          U16* h = &posYTravHeap[trav->first];
          U16* hend = h + trav->length;

          // take maxg from end of approximating line
          maxg = maxgMap[*h];

          // max against cells near extended line
          while (++h < hend)
          {
            g = gMap[*h];
            if (maxg < g) maxg=g;
          }

          // max against current cell
          maxGradMap[z][x] = maxg;
        }
      }
    }

    //
    // Fill Max Gradient map using traversal info along negative z axis
    //
    void FillMaxGradMapNegZ()
    {
      for (z = 0, z2 = 0; z2 < r2; z2 += 2 * z + 1, ++z)
      {
        for (x = 0, x2 = 0; x2 + z2 < r2; x2 += 2 * x + 1, ++x)
        {
          TravInfo* trav = &negYTravInfo[z][x];
          U16* h = &negYTravHeap[trav->first];
          U16* hend = h + trav->length;

          // take maxg from end of approximating line
          maxg = maxgMap[*h];

          // max against cells near extended line
          while (++h<hend)
          {
            g = gMap[*h];
            if (maxg<g) maxg=g;
          }
          // max against current cell
          maxGradMap[z][x] = maxg;
        }
      }
    }


    //
    // Set visible cells in ++ quadrant
    //
    void CompareGradPosPos()
    {
      for (z = 0, z2 = 0; z2 < r2; z2 += 2 * z + 1, ++z)
      {
        for (x = 1, x2 = 1; x2 + z2 < r2; x2 += 2 * x + 1, x++)
        {
          dx = x;
          dz = z;

          if (maxGradMap[z][x] <= gradMapU[z][x])
          {
            CanSee(cellX + dx, cellZ + dz, dx, dz, mapLo, maskLo, teamBits, Map::LV_LO);
          }
        }
      }
    }


    //
    // Set visible cells in -+ quadrant
    //
    void CompareGradNegPos()
    {
      for (z = 0, z2 = 0; z2 < r2; z2 += 2 * z + 1, ++z)
      {
        for (x = 1, x2 = 1; x2 + z2 < r2; x2 += 2 * x + 1, ++x)
        {
          dx = -z;
          dz = x;

          if (maxGradMap[z][x]<=gradMapU[z][x])
          {
            CanSee(cellX + dx, cellZ + dz, dx, dz, mapLo, maskLo, teamBits, Map::LV_LO);
          }
        }
      }
    }


    //
    // Set visible cells in -- quadrant
    //
    void CompareGradNegNeg()
    {
      for (z = 0, z2 = 0; z2 < r2; z2 += 2 * z + 1, ++z)
      {
        for (x = 1, x2 = 1; x2 + z2 < r2; x2 += 2 * x + 1, ++x)
        {
          dx = -x;
          dz = -z;

          if (maxGradMap[z][x]<=gradMapU[z][x])
          {
            CanSee(cellX + dx, cellZ + dz, dx, dz, mapLo, maskLo, teamBits, Map::LV_LO);
          }
        }
      }
    }


    //
    // Set visible cells in +- quadrant
    //
    void CompareGradPosNeg()
    {
      for (z = 0, z2 = 0; z2 < r2; z2 += 2 * z + 1, ++z)
      {
        for (x = 1, x2 = 1; x2 + z2 < r2; x2 += 2 * x + 1, ++x)
        {
          dx = z;
          dz = -x;

          if (maxGradMap[z][x]<=gradMapU[z][x])
          {
            CanSee(cellX + dx, cellZ + dz, dx, dz, mapLo, maskLo, teamBits, Map::LV_LO);
          }
        }
      }
    }


    //
    // New and improved sweep
    //
    void Sweep(UnitObj *u)
    {
      ASSERT(sysInit);
      ASSERT(u);
      //ASSERT(u->GetTeam());
      ASSERT(u->OnMap());

      START(sweepTime);

      r = u->GetSeeingRange();
      unit = u;
      cellX = u->cellX;
      cellZ = u->cellZ;

      ASSERT(r < MAXR)

      // Get sight map and mask
      maskLo = u->sightMap->GetBitMask(Map::LV_LO);
      mapLo = u->sightMap->GetByteMap(Map::LV_LO);

      // Reset max radius seen
      u->sightMap->lastR = S16_MIN;

      // Set teams that this sweep will provide LOS for
      Team *myTeam = u->GetTeam();
      teamBits = 0;

      if (myTeam)
      {
        for (U32 team = 0; team < Game::MAX_TEAMS; ++team)
        {
          Team *other = Team::Id2Team(team);

          if (other)
          {
            if
            (
              // Other team is an ally
              Team::TestUnitRelation(u, other, Relation::ALLY)

              ||

              // Other team is giving us line of sight
              myTeam->GivingSightTo(other->GetId())
            )
            {
              ASSERT(teamRemap[team] < teamCount)
              Game::TeamSet(teamBits, teamRemap[team]);
            }
          }
        }
      }

      // Dirty cells that line of sight has changed in
      DirtyCells(cellX - r, cellZ - r, cellX + r, cellZ + r, teamBits);

      // Set up viewing radius
      if (r > 0)
      {
        r2 = r * r;
        r2Inv = 1.0f / F32(r2);

        // Get eye position
        eyePos = EyePosition(u);

        // In the code below the row and column of cells with the
        // same y and x value of the unit are processed twice but
        // their viewing information is only updated once. Stuff
        // the slight inneficiency (2*r extra comparisons). This
        // way we require 1/4 the memory and the code is more
        // elegant.

        // scan ++ quadrant
        quadrantX = POS;
        quadrantZ = POS;

        FillGradMap();
        FillMaxGradMapPosZ();
        CompareGradPosPos();

        // Scan -+ quadrant
        quadrantX = NEG;

        FillGradMapRotated();
        FillMaxGradMapPosZ();
        CompareGradNegPos();

        // Scan -- quadrant
        quadrantZ = NEG;

        FillGradMap();
        FillMaxGradMapNegZ();
        CompareGradNegNeg();

        // Scan +- quadrant
        quadrantX = POS;

        FillGradMapRotated();
        FillMaxGradMapNegZ();
        CompareGradPosNeg();
      }

      // Can always see cell that unit is occupying
      CanSee(cellX, cellZ, 0, 0, mapLo, maskLo, teamBits, Map::LV_LO);

      // Update scan info in unit's sight map
      u->sightMap->lastTeam = teamBits;
      u->sightMap->lastR = S16(r);
      u->sightMap->lastX = cellX;
      u->sightMap->lastZ = cellZ;
      u->sightMap->lastAlt = eyePos;

      STOP(sweepTime);
    }
  }


  //
  // Unsweep for a sight map
  //
  static void Unsweep(Map *map)
  {
    START(unSweepTime);

    PREFETCH(&__prefetch);

    // Last sweep position
    S32 tileX = map->lastX;
    S32 tileZ = map->lastZ;

    // Last sweep radius
    S32 r = map->lastR;

    // Get viewing mask for lower layer
    U8 *mapLo = map->GetByteMap(Map::LV_LO);
    U8 maskLo = map->GetBitMask(Map::LV_LO);

    // Dirty cells that line of sight has changed in
    DirtyCells(tileX - r, tileZ - r, tileX + r, tileZ + r, map->lastTeam);

    // iterate over all tiles within last scan radius
    S32 first = XZToSeemap(-r, -r);

    for (S32 y = -r; y <= r; y++, first += MAPSIDE)
    {
      PREFETCH(&mapLo[first+MAPSIDE]);

      for (S32 x = -r, index = first; x <= r; x++, index++)
      {
        // Unsweep tile on ground level if was swept
        if (mapLo[index] & maskLo)
        {
          // Unsweep for all teams last swept
          CantSee(tileX + x, tileZ + y, x, y, map->lastTeam, Map::LV_LO);

          // Clear seen bit
          mapLo[index] &= ~maskLo;
        }
      }
    }

    // Reset last radius and team mask
    map->lastR = 0;
    map->lastTeam = 0;
    map->lastAlt = F32_MAX;

    STOP(unSweepTime);
  }


  //
  // Detach a sight map from a unit (after it dies)
  //
  void Detach(Map **map)
  {
    if (editMode)
    {
      //delete straight away
      Unsweep(*map);
      delete *map;
    }
    else
    {
      // Append to detached list
      detachedList.Append(new Detached(GameTime::SimCycle() + 20, *map));
    }

    // Unit no longer owns this
    *map = NULL;
  }


  //
  // General unsweep
  //
  void UnSweep(UnitObj *u)
  {
    ASSERT(sysInit);
    ASSERT(u);
    ASSERT(u->OnMap());

    Unsweep(u->sightMap);
  }


  //
  // Return the eye position of an object
  //
  F32 EyePosition(UnitObj *u)
  {
    // Viewing position is the top of the bounding box
    F32 y = u->Mesh().Origin().y + u->UnitType()->GetSeeingHeight();

    // Clamp it to at least a litle above the level of the center 
    // of the cell that the unit is on.
    return (Max<F32>(y, GetAltitude(u->cellX, u->cellZ, NULL) + 0.1F));
  }


  //
  // Initialize system
  //
  void Init(Bool editModeIn)
  {
    ASSERT(!sysInit);

    editMode = editModeIn;

    U16 initialVal = U16(0);

    ASSERT(WorldCtrl::CellMapX());
    ASSERT(WorldCtrl::CellMapZ());

    // Initialise arrays
    for (U32 t = 0; t < Game::MAX_TEAMS; t++)
    {
      teamRemap[t]      = Game::MAX_TEAMS;
      invTeamRemap[t]   = Game::MAX_TEAMS;
      teamDirtyClust[t] = NULL;
      teamLastRescan[t] = U32_MAX;
    }

    if (editMode)
    {
      // If we're in the editor, setup for all teams
      teamCount = Game::MAX_TEAMS;

      for (U32 t = 0; t < Game::MAX_TEAMS; t++)
      {
        teamRemap[t] = U8(t);
        invTeamRemap[t] = U8(t);
      }
    }
    else
    {
      teamCount = Team::NumTeams();

      // Fill in remap entries for teams with id's
      for (U32 t = 0, t2 = 0; t < Game::MAX_TEAMS; t++)
      {
        Team *teamPtr = Team::Id2Team(t);

        if (teamPtr)
        {
          ASSERT(teamPtr->GetId() < Game::MAX_TEAMS)

          // Map team id back to sequential index
          teamRemap[teamPtr->GetId()] = U8(t2);

          // Map sequential team index to team id
          invTeamRemap[t2] = U8(teamPtr->GetId());

          ++t2;
        }
      }
      ASSERT(t2 == teamCount)
    }

    #ifdef DEVELOPMENT
    {
      // Log remap tables and test sync
      char s1[Game::MAX_TEAMS+1];
      char s2[Game::MAX_TEAMS+1];

      s1[Game::MAX_TEAMS] = s2[Game::MAX_TEAMS] = 0;
      for (U32 i = 0; i < Game::MAX_TEAMS; i++)
      {
        s1[i] = teamRemap[i] < Game::MAX_TEAMS ? char(teamRemap[i] + '0') : '.';
        s2[i] = invTeamRemap[i] < Game::MAX_TEAMS ? char(invTeamRemap[i] + '0') : '.';
      }

      //LOG_DIAG(("SightRemap [%s] [%s]", s1, s2))
      SYNC("SightRemap [" << s1 << "] [" << s2 << ']')
    }
    #endif

    // Allocate seeing array
    for (U32 level = 0; level < Map::LV_MAX; ++level)
    {
      for (U32 team = 0; team < teamCount; ++team)
      {
        seeMap[level][team] = new U16*[WorldCtrl::CellMapZ()];
        seeMap[level][team][0] = new U16[WorldCtrl::CellMapX() * WorldCtrl::CellMapZ()];

        for (U32 i = 1; i < WorldCtrl::CellMapZ(); ++i)
        {
          seeMap[level][team][i] = &seeMap[level][team][i - 1][WorldCtrl::CellMapX()];
        }

        // Set initial fog value for all tiles
        for (U32 z = 0; z < WorldCtrl::CellMapZ(); ++z)
        {
          for (U32 x = 0; x < WorldCtrl::CellMapX(); ++x)
          {
            seeMap[level][team][z][x] = initialVal;
          }
        }
      }
    }

    // Round x clusters up to the nearest 8 for the bit array
    mapClustXon8 = ((WorldCtrl::ClusterMapX() + 7) & (~7)) >> 3;

    // Allocate cluster update array
    displayDirtyCells = new BitArray2d(WorldCtrl::CellMapX(), WorldCtrl::CellMapZ());

    for (t = 0; t < teamCount; t++)
    {
      // Allocate team cluster update array
      teamDirtyClust[t] = new U8[mapClustXon8 * WorldCtrl::ClusterMapZ()];

      // Undirty all clusters
      memset(teamDirtyClust[t], 0, mapClustXon8 * WorldCtrl::ClusterMapZ());
    }

    // Initialise radius division lookup table
    for (U32 r = 0; r < MAXR; ++r)
    {
      invRadTbl[r] = (r == 0) ? 10000.0F : (1.0F / F32(r * WC_CELLSIZE));
    }

    // Initialise the terrain - set it all to shrouded
    SetAllFog(Terrain::fogFactorsS32[0]);

    // Create varsys commands
    VarSys::RegisterHandler("coregame.sight", CmdHandler);

    #ifdef DEVELOPMENT

    // Development commands
    VarSys::CreateCmd("coregame.sight.info");
    VarSys::CreateCmd("coregame.sight.map");
    VarSys::CreateInteger("coregame.sight.debugmode", FALSE, VarSys::DEFAULT, &debugMode);
    VarSys::CreateInteger("coregame.sight.debugscan", FALSE, VarSys::DEFAULT, &debugScan);

    #endif

   
    // The update rate of LOS display   moved to terrain.shroud.rate
//    VarSys::CreateInteger("coregame.sight.updaterate", 2, VarSys::DEFAULT, &displayRate);
//    displayRate->SetIntegerRange(0, 100);

    #ifdef DEVELOPMENT

    sweepTime.Reset();
    unSweepTime.Reset();
    updateDisp.Reset();
    dirtyCellTime.Reset();

    #endif

    // Should all units be visible?
    showAllUnits = FALSE;

    // System is initialised
    sysInit = TRUE;
  }


  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(sysInit);

    // Delete detached maps
    NList<Detached>::Iterator itr(&detachedList);
    Detached *item;

    while ((item = itr++) != NULL)
    {
      Unsweep(item->map);
      delete item->map;
      detachedList.Dispose(item);
    }

    // Delete sight maps
    for (U32 team = 0; team < teamCount; ++team)
    {
      for (U32 level = 0; level < Map::LV_MAX; ++level)
      {
        ASSERT(seeMap[level][team])

        delete[] seeMap[level][team][0];
        seeMap[level][team][0] = NULL;

        delete[] seeMap[level][team];
        seeMap[level][team] = NULL;
      }

      delete[] teamDirtyClust[team];
    }

    // Delete dirty cluster map
    delete displayDirtyCells;

    // Delete var scope
    VarSys::DeleteItem("coregame.sight");

    // System is shut down
    sysInit = FALSE;
  }


  //
  // Rescan all line of sight for all units on a team
  //
  void RescanAll(Team *team, Bool force)
  {
    ASSERT(team)

    // Avoid multiple rescans on the same cycle
    if (force || (GameTime::GameCycle() != teamLastRescan[team->GetId()]))
    {    
      for (NList<UnitObj>::Iterator i(&team->GetUnitObjects()); *i; i++)
      {
        UnitObj *unitObj = *i;

        if (unitObj->OnMap())
        {
          UnSweep(unitObj);
          Sweep(unitObj);
        }
      }

      // Update scan time
      teamLastRescan[team->GetId()] = GameTime::GameCycle();
    }
    else
    {
      //LOG_DIAG(("Ignoring multiple team rescan for [%s] on cycle %d", team->GetName(), GameTime::GameCycle()))
    }
  }


  //
  // Rescan All objects which are of the given types
  //
  void RescanAllTypes(const NList<UnitObjType> &types)
  {
    for (U32 team = 0; team < Game::MAX_TEAMS; ++team)
    {
      Team *teamPtr = Team::Id2Team(team);

      if (teamPtr)
      {
        for (NList<UnitObjType>::Iterator type(&types); *type; type++)
        {
          const NList<UnitObj> *unitList = teamPtr->GetUnitObjects((*type)->GetNameCrc());

          if (unitList)
          {
            for (NList<UnitObj>::Iterator obj(unitList); *obj; obj++)
            {
              UnitObj *unitObj = *obj;

              if (unitObj->OnMap())
              {
                UnSweep(unitObj);
                Sweep(unitObj);
              }
            }
          }
        }
      }
    }
  }


  #ifdef DEVELOPMENT

  //
  // Rescan all objects on the map, forcefully
  //
  static void RescanAllDebug()
  {
    for (U32 team = 0; team < Game::MAX_TEAMS; ++team)
    {
      Team *teamPtr = Team::Id2Team(team);

      if (teamPtr)
      {
        for (NList<UnitObj>::Iterator i(&teamPtr->GetUnitObjects()); *i; i++)
        {
          UnitObj *unitObj = *i;

          if (unitObj->OnMap())
          {
            UnSweep(unitObj);
            Sweep(unitObj);
          }
        }
      }
    }
  }

  #endif


  //
  // To be called after the mission is loaded
  //
  void NotifyPostMissionLoad()
  {
    for (U32 team = 0; team < Game::MAX_TEAMS; ++team)
    {
      Team *teamPtr = Team::Id2Team(team);

      if (teamPtr)
      {
        RescanAll(teamPtr, TRUE);
      }
    }
  }


  //
  // Update all items on the detached list
  //
  void ProcessDetachedList()
  {
    U32 now = GameTime::SimCycle();

    // This should be in order, with oldest first
    NList<Detached>::Iterator itr(&detachedList);
    Detached *item;

    while ((item = itr++) != NULL)
    {
      if (item->killme <= now)
      {
        Unsweep(item->map);
        delete item->map;
        detachedList.Dispose(item);
      }
      else
      {
        // All subsequent items should be older
        break;
      }
    }
  }


  //
  // Update fog display
  //
  void UpdateDisplay(Team *team, Bool force, Bool first) // = FALSE, = FALSE
  {
    #ifdef DEVELOPMENT

    if (*debugScan)
    {
      SYNC("DebugScan")
      RescanAllDebug();
    }

    #endif

    // Are we ready to cycle yet?
    if (!force && (--nextDisplay > 0))
    {
      return;
    }
    else
    {
      nextDisplay = *Vid::Var::Terrain::shroudUpdate;
    }

    // Ensure team is valid
    if (!team)
    {
      return;
    }

    // Now update display
    START(updateDisp);

    // Map team's id to seemap array index
    U32 teamId = teamRemap[team->GetId()];
    ASSERT(teamId < teamCount)

    S32 shroudFog = Vid::Var::Terrain::shroudFog;
    S32 shroudRate = Vid::Var::Terrain::shroudRate;
    U32 pitch = displayDirtyCells->Pitch();

    for (U32 cellZ = 0; cellZ < WorldCtrl::CellMapZ(); cellZ++)
    {
      // Index of first byte in this row
      U32 rowbyte = cellZ * pitch;

      // Offset of first bit in this row
      U32 rowoffset = (rowbyte << 3);

      // Process each block of 8 cells
      for (U32 block = 0; block < pitch; block++)
      {
        if (displayDirtyCells->GetBlock(rowbyte + block))
        {
          U32 cx0 = block << 3;
          U32 cx1 = Min<U32>(cx0 + 8, WorldCtrl::CellMapX());

          for (U32 cellX = cx0; cellX < cx1; cellX++)
          {
            // Bit offset into 1d bit array
            U32 bitoffset = rowoffset + cellX;

            if (displayDirtyCells->Get1(bitoffset))
            {
              U16 seeElem = seeMap[Map::LV_LO][teamId][cellZ][cellX];

              if (seeElem & SEENMASK)
              {
                Cell *pCell = Terrain::GetCell(cellX, cellZ);

                S32 newLevel = seeElem > SEENMASK ? Terrain::fogFactorsS32[7] : Terrain::fogFactorsS32[shroudFog];
                S32 oldLevel = pCell->GetFog();

                if (first || (newLevel == oldLevel))
                {
                  pCell->SetFog( newLevel);
                  displayDirtyCells->Clear1(bitoffset);
                }
                else 
              
                if (oldLevel > newLevel)
                {
                  oldLevel -= shroudRate;
                  if (oldLevel <= newLevel)
                  {
                    displayDirtyCells->Clear1(bitoffset);
                    oldLevel = newLevel;
                  }
                  pCell->SetFog( oldLevel);
                }
                else 
              
                if (oldLevel < newLevel)
                {
                  oldLevel += shroudRate;
                  if (oldLevel >= newLevel)
                  {
                    displayDirtyCells->Clear1(bitoffset);
                    oldLevel = newLevel;
                  }
                  pCell->SetFog( oldLevel);
                }
              }
              else
              {
                displayDirtyCells->Clear1(bitoffset);
              }
            }
          }
        }
      }
    }

    STOP(updateDisp);

    MSWRITEV(13, (3, 0, "Sweep     %s", sweepTime.Report()));
    MSWRITEV(13, (4, 0, "UnSweep   %s", unSweepTime.Report()));
    MSWRITEV(13, (5, 0, "UpdDisp   %s", updateDisp.Report()));
    MSWRITEV(13, (6, 0, "DirtyCell %s", dirtyCellTime.Report()));
  }


  //
  // Sweep away fog around unit
  //
  void Sweep(UnitObj *u)
  {
    Sweeper::Sweep(u);
  }


  //
  // TRUE iff top left corner of cell x,z has been seen by team
  //
  Bool Seen(U32 x, U32 z, Team *team)
  {
    ASSERT(team)
    ASSERT(sysInit)
    ASSERT(teamRemap[team->GetId()] < teamCount)
    ASSERT(seeMap[Map::LV_LO][teamRemap[team->GetId()]])
    ASSERT(WorldCtrl::CellOnMap(x, z))

    return (seeMap[Map::LV_LO][teamRemap[team->GetId()]][z][x] & SEENMASK ? TRUE : FALSE);
  }


  //
  // TRUE iff top left corner of cell x,z is currently visible by team
  //
  Bool Visible(U32 x, U32 z, Team *team)
  {
    ASSERT(team)
    ASSERT(sysInit)
    ASSERT(teamRemap[team->GetId()] < teamCount)
    ASSERT(seeMap[Map::LV_LO][teamRemap[team->GetId()]])
    ASSERT(WorldCtrl::CellOnMap(x, z))

    return (seeMap[Map::LV_LO][teamRemap[team->GetId()]][z][x] > SEENMASK ? TRUE : FALSE);
  }


  //
  // TREE iff any cell in the range is currently visible by team
  //
  Bool Visible(const Area<U32> &area, Team *team)
  {
    ASSERT(team)
    ASSERT(sysInit)
    ASSERT(teamRemap[team->GetId()] < teamCount)
    ASSERT(seeMap[Map::LV_LO][teamRemap[team->GetId()]])
    ASSERT(WorldCtrl::CellOnMap(area.p0.x, area.p0.z))
    ASSERT(WorldCtrl::CellOnMap(area.p1.x, area.p1.z))

    U16 *ptr = &seeMap[Map::LV_LO][teamRemap[team->GetId()]][area.p0.z][area.p0.x];
    U32 rowJump = WorldCtrl::CellMapX() - area.Width() - 1;

    for (U32 z = 0; z <= area.Height(); z++)
    {
      for (U32 x = 0; x <= area.Width(); x++)
      {
        if (*ptr > SEENMASK)
        {
          return (TRUE);
        }
        ++ptr;
      }
      ptr += rowJump;
    }

    return (FALSE);
  }


  //
  // Return bit mask of teams seeing this cell
  //
  Game::TeamBitfield VisibleMask(U32 x, U32 z)
  {
    ASSERT(sysInit)
    ASSERT(WorldCtrl::CellOnMap(x, z))

    Game::TeamBitfield bits = 0;

    for (U32 t = 0; t < teamCount; t++)
    {
      if (seeMap[Map::LV_LO][t][z][x] > SEENMASK)
      {
        bits |= (1 << invTeamRemap[t]);
      }
    }

    return (bits);
  }


  //
  // Return seen and visible at once
  //
  void SeenVisible(U32 x, U32 z, Team *team, Bool &seen, Bool &vis)
  {
    ASSERT(team)
    ASSERT(sysInit)
    ASSERT(teamRemap[team->GetId()] < teamCount)
    ASSERT(seeMap[Map::LV_LO][teamRemap[team->GetId()]])
    ASSERT(WorldCtrl::CellOnMap(x, z))

    U16 seeElem = seeMap[Map::LV_LO][teamRemap[team->GetId()]][z][x];

    vis  = (seeElem > SEENMASK) ? TRUE : FALSE;
    seen = (seeElem & SEENMASK) ? TRUE : FALSE;
  }


  //
  // SeenVisible
  //
  // Check if any cells are seen/visible in the given area
  //
  void SeenVisible(const Area<U32> &area, Team *team, Bool &seen, Bool &vis)
  {
    ASSERT(team)
    ASSERT(sysInit)
    ASSERT(teamRemap[team->GetId()] < teamCount)
    ASSERT(seeMap[Map::LV_LO][teamRemap[team->GetId()]])
    ASSERT(WorldCtrl::CellOnMap(area.p0.x, area.p0.z))
    ASSERT(WorldCtrl::CellOnMap(area.p1.x, area.p1.z))

    seen = FALSE;
    vis = FALSE;

    for (U32 z = 0; z <= area.Height(); z++)
    {
      U16 *ptr = &seeMap[Map::LV_LO][teamRemap[team->GetId()]][area.p0.z + z][area.p0.x];      
      
      for (U32 x = 0; x <= area.Width(); x++)
      {
        if (!vis && (*ptr > SEENMASK))
        {
          vis = TRUE;
        }

        if (!seen && (*ptr & SEENMASK))
        {
          seen = TRUE;
        }

        if (vis && seen)
        {
          return;
        }

        ptr++;
      }
    }
  }


  //
  // CanUnitSee
  //
  // Test if unit can see map cell x,z
  //
  Bool CanUnitSee(UnitObj *u, U32 x, U32 z)
  {
    ASSERT(u);
    ASSERT(u->sightMap)

    Sight::Map &sMap = *(u->sightMap);

    S32 offsetX = S32(x) - sMap.lastX;
    S32 offsetZ = S32(z) - sMap.lastZ;

    // Check if offsets are within the last scan radius
    if (abs(offsetX) > sMap.lastR || abs(offsetZ) > sMap.lastR)
    {
      // Out of range
      return (FALSE);
    }

    // Check the byte map
    U8 *map = sMap.GetByteMap(Map::LV_LO);
    U8 mask = sMap.GetBitMask(Map::LV_LO);
    U32 index = XZToSeemap(offsetX, offsetZ);

    if (map[index] & mask)
    {
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Return the dirty cluster map for a team
  //
  U8 *GetDirtyClusterMap(Team *team)
  {
    ASSERT(team)
    ASSERT(teamRemap[team->GetId()] < teamCount)

    return (teamDirtyClust[teamRemap[team->GetId()]]);
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Console commands
  //

  //
  // Draws line of sight on the grid window
  //
  static Color GridCellCallBack(void *context, U32 x, U32 z)
  {
    ASSERT(context);

    // Team pointer is in context
    Team *team = (Team *)context;

    if (Visible(x, z, team))
    {
      return (Color(U32(192), U32(192), U32(192)));
    }
    else

    if (Seen(x, z, team))
    {
      return (Color(U32(64), U32(64), U32(64)));
    }

    // Not visible, never seen
    return (Color(U32(0), U32(0), U32(0)));
  }


  //
  // Command handler
  //
  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      #ifdef DEVELOPMENT

      case 0xE5281999: // "coregame.sight.map"
      {
        S32 teamId;

        if (Console::GetArgInteger(1, teamId))
        {
          Team *teamPtr = Team::Id2Team(teamId);

          if (teamPtr)
          {
            static U32 newX, newY;
            S32 scale;

            if (!Console::GetArgInteger(2, scale))
            {
              scale = 1;
            }

            char buf[64];

            // Put Name of team in title bar
            Utils::Sprintf(buf, sizeof(buf), "SightMap[%s]", teamPtr->GetName());

            // Create a grid window
            ICGridWindow *gridWnd = new ICGridWindow(buf, WorldCtrl::CellMapX(), WorldCtrl::CellMapZ(), scale, scale);

            // Setup the grid
            gridWnd->Grid().SetCellCallBack(GridCellCallBack);
            gridWnd->Grid().SetContext(teamPtr);
            gridWnd->Grid().SetPollInterval(1000);
            gridWnd->Grid().SetAxisFlip(FALSE, TRUE);

            // Cascade the windows
            gridWnd->SetPos(newX, newY);

            if ((newX + WorldCtrl::CellMapX() > U32(IFace::ScreenWidth())) || (newY + WorldCtrl::CellMapZ() > U32(IFace::ScreenHeight())))
            {
              newX = newY = 0;
            }
            else
            {
              newX += 10; 
              newY += 10;
            }
            
            gridWnd->Activate();
          }
        }

        break;
      }

      case 0x5DD4A46E: // "coregame.sight.info"
      {
        U32 memIndex  = sizeof(U16 *) * WorldCtrl::CellMapZ() * Map::LV_MAX * teamCount;
        U32 memSeeMap = sizeof(U16) * WorldCtrl::CellMapX() * WorldCtrl::CellMapZ() * Map::LV_MAX * teamCount;

        Color c0 = Color::Std[Color::YELLOW];
        Color c1 = Color::Std[Color::LTBLUE];

        CON_DIAG(("Allocated byte maps"))
        CON_DIAG(("  Current: %6d", ByteMap::GetCount()))
        CON_DIAG(("  Overall: %6d", ByteMap::GetNextId()))
        CON_DIAG((""))
        CON_DIAG(("Allocated by see map"))
        CON_DIAG(("  Indexes: %6d", memIndex))
        CON_DIAG(("  Seemap : %6d", memSeeMap))
        CON_DIAG(("  Total  : %6d", memIndex + memSeeMap))
        CON_DIAG((""))
        CON_DIAG(("Sizeof:"))
        CON_DIAG(("  ByteMap: %6d", sizeof(ByteMap)))
        CON_DIAG(("  Map    : %6d", sizeof(Map)))
        CON_DIAG(("  Tables : %6d", sizeof(posYTravInfo) + sizeof(posYTravHeap) + sizeof(negYTravInfo) + sizeof(negYTravHeap)))
        CON_DIAG((""))
        CON_DIAG(("Max viewing range"))
        CON_DIAG(("  Cells  : %6d", MAXR))
        CON_DIAG(("  Metres : %6.1f", MAXR_METRES))

        break;
      }

      #endif
    }
  }


  #ifdef DEVELOPMENT

  //
  // Reset the terrain display
  //
  void ResetTerrainDisplay()
  {
    SetAllFog(Terrain::fogFactorsS32[0]);
    DirtyAllCells();
  }


  //
  // Render debugging info for a unit
  //
  void RenderDebug(UnitObj *u)
  {
    ASSERT(u)
    ASSERT(u->sightMap)

    const Color Clr(118L, 152L, 203L);

    Sight::Map &smap = *(u->sightMap);
    U8 *map = smap.GetByteMap(Map::LV_LO);
    U8 mask = smap.GetBitMask(Map::LV_LO);

    for (S32 x = -smap.lastR; x <= smap.lastR; x++)
    {
      for (S32 z = -smap.lastR; z <= smap.lastR; z++)
      {
        U32 index = XZToSeemap(x, z);

        if (map[index] & mask)
        {
          S32 cx = smap.lastX + x;
          S32 cz = smap.lastZ + z;
          Area<S32> rc( cx, cz, cx+1, cz+1);

          // Draw the cell
          ::Terrain::RenderCellRect(rc, Clr, TRUE, FALSE);
        }
      }
    }
  }

  #endif
}
