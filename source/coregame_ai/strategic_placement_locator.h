/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Placement
//


#ifndef __STRATEGIC_PLACEMENT_LOCATOR_H
#define __STRATEGIC_PLACEMENT_LOCATOR_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_placement.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Placement::Locator
  //
  class Placement::Locator
  {
  private:

    // Placement tree node
    NBinTree<Locator>::Node nodePlacementTree;

    // Placement list node
    NList<Locator>::Node nodePlacementList;

    // Base token we're locating for
    Base::Token &token;

    // Type clusters for this locator
    ClusterSet &clusterSet;

  public:

    // Constructor
    Locator(Base::Token &token, ClusterSet &clusterSet);

    // Destructor
    ~Locator();

  public:

    friend Placement;

  };

}

#endif
