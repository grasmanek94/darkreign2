///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Line Of Sight System
//
// 24-NOV-1998
//


#include "sight_map.h"
#include "sight_bytemap.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sight - The line of sight system
//
namespace Sight
{

  //
  // Constructor
  //
  Map::Map() : lastTeam(0), lastX(0), lastZ(0), lastR(0), lastAlt(F32_MAX)
  {
    for (U32 i = 0; i < LV_MAX; i++)
    {
      byteMapInfo[i].map = NULL;
      ByteMap::Get(byteMapInfo[i].map, byteMapInfo[i].mask);
    }
  }


  //
  // Destructor
  //
  Map::~Map()
  {
    // Make sure sight has been unswept
    ASSERT(lastTeam == 0);

    // Release bytemaps
    for (U32 i = 0; i < LV_MAX; i++)
    {
      ByteMap::Unget(byteMapInfo[i].map, byteMapInfo[i].mask);
    }
  }

}
