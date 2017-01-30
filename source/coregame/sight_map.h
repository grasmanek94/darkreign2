///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Line Of Sight System
//
// 24-NOV-1998
//


#ifndef __SIGHT_MAP_H
#define __SIGHT_MAP_H


#include "sight_bytemap.h"
#include "gameconstants.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sight - The line of sight system
//
namespace Sight
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Map - Line of Sight map
  //
  class Map
  {
  public:

    // Levels of bytemaps
    enum
    {
      LV_LO,                      // Ground level
      //LV_HI,                      // Flyer level
      LV_MAX,
    };

    // Reference to allocated ByteMap
    struct ByteMapInfo
    {
      ByteMap *map;           // Pointer to ByteMap
      U8 mask;                // Bit mask of entries
    };

  public:

    // Byte map pointer for each level
    ByteMapInfo byteMapInfo[LV_MAX];

    // X coord of last fog sweep
    S32 lastX;

    // Z cell of last fog sweep
    S32 lastZ;

    // Radius of last fog sweep
    S16 lastR;                 

    // Altitude of last fog sweep
    F32 lastAlt;

    // Bitmask of teams in last fog sweep
    Game::TeamBitfield lastTeam;

  public:

    Map();
    ~Map();

    // Return a bytemap for a specific level
    U8 *GetByteMap(U32 level)
    {
      ASSERT(level < LV_MAX);
      ASSERT(byteMapInfo[level].map);

      return (byteMapInfo[level].map->GetBytes());
    }

    // Return a bitmask for a specific level
    U8 GetBitMask(U32 level)
    {
      ASSERT(level < LV_MAX);

      return (byteMapInfo[level].mask);
    }
  };

}

#endif