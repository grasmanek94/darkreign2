/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game Constants
//
// 03-SEP-1998
//

#ifndef __GAME_CONSTANTS_H
#define __GAME_CONSTANTS_H


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  // Team Constants
  const   U32 MAX_TEAMS = 8;
  typedef U8  TeamBitfield;

  inline Bool TeamTest(TeamBitfield bitfield, U32 teamNum)
  {
    return ((bitfield >> teamNum) & 0x01 ? TRUE : FALSE);
  }

  inline void TeamSet(TeamBitfield &bitfield, U32 teamNum)
  {
    bitfield |= (1 << teamNum); 
  }

  inline void TeamClear(TeamBitfield &bitfield, U32 teamNum)
  {
    bitfield &= ~(1 << teamNum);
  }


  // Player Constants 
  const U32 MAX_PLAYERS = (MAX_TEAMS * 5);

  // Mission configuration file
  const char FILENAME_MISSION_CONFIG[] = "game.cfg";

  // Mission objects file
  const char FILENAME_MISSION_OBJECTS[] = "objects.cfg";

  // Mission terrain file
  const char FILENAME_MISSION_TERRAIN[] = "terrain.blk";
}


#endif
