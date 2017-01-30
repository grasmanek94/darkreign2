///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Unit Evacuation
//
// 18-APR-2000
//

#ifndef __UNITEVACUATE_H
#define __UNITEVACUATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "utiltypes.h"
#include "claim.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class UnitEvacuate - Used to move units off a given list of cells
//

class UnitEvacuate
{
public:

  enum Result
  {
    AVAILABLE,    // All registered cells are available
    UNKNOWN,      // A claim with an unknown owner was found
    STATIC,       // A static unit was found
    MOBILE,       // A mobile unit was found that may evacuate
  };

protected:

  // A single registered cell
  struct Cell : public Point<S32>
  {
    // The list node
    NList<Cell>::Node node;

    // The layer to use for this cell
    Claim::LayerId layer;

    // Constructor
    Cell(const Point<S32> &p, Claim::LayerId layer) : Point<S32>(p), layer(layer)
    { 
    }
  };

  // The list of all registered cells
  NList<Cell> list;

public:

  // Constructor and destructor
  UnitEvacuate();
  ~UnitEvacuate();

  // Register a cell
  void Register(const Point<S32> &p, Claim::LayerId layer = Claim::LAYER_LOWER);

  // Clear all registered cells
  void Clear();

  // Attempt to evacuate all units claiming the registered cells
  Result Process(Team *team, UnitObj *ignore = NULL);
};


#endif