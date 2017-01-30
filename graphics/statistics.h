///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// mesh stats
//
// 19-NOV-1998
//

#ifndef __MESHSTATS_H
#define __MESHSTATS_H

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#define DOSTATISTICS

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Statistics - game statistics
//
namespace Statistics
{
  extern U32 terrainTris;
  extern U32 groundSpriteTris;
  extern U32 spriteTris;

  extern U32 objectTris;
  extern U32 overlayTris;

  extern U32 mrmTris;
  extern U32 nonMRMTris;

  extern U32 ifaceTris;
  extern U32 totalTris;

  extern U32 clipTris;
  extern U32 noClipTris;

  // used to gather tri data from low level routines
  extern U32 tempTris;

  // reset the values at the beginning of frame
  void Reset();

  void SetTotal();

  void Init();
  void Done();
}

#endif