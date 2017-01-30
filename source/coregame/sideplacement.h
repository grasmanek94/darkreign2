///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Placement Of Side-Based Units
//
// 17-FEB-2000
//

#ifndef __SIDEPLACEMENT_H
#define __SIDEPLACEMENT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace SidePlacement - Placement Of Side-Based Units
//
namespace SidePlacement
{
  // Initialize and shutdown system
  void Init();
  void Done();

  // Place units for the given team
  void Place(Team *team);
}

#endif
