///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Line Of Sight System
//
// 24-NOV-1998
//


#ifndef __SIGHT_TABLES_H
#define __SIGHT_TABLES_H


#include "sight_bytemap.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sight - The line of sight system
//
namespace Sight
{

  //
  // Traversal Info structure
  //
  struct TravInfo
  {
    S16 first;
    S16 length;
  };

  typedef TravInfo TravArray[MAXR][MAXR];


  // Traversal info tables
  extern TravArray posYTravInfo;//TravInfo posYTravInfo[MAXR][MAXR];
  extern U16 posYTravHeap[2265];

  extern TravArray negYTravInfo;//TravInfo negYTravInfo[MAXR][MAXR];
  extern U16 negYTravHeap[2001];

}

#endif
