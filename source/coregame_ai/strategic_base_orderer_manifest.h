/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Orderer Manifest
//


#ifndef __STRATEGIC_BASE_ORDERER_MANIFEST_H
#define __STRATEGIC_BASE_ORDERER_MANIFEST_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base_orderer.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Orderer::Manifest
  //
  class Base::Orderer::Manifest
  {
  protected:

    // Flags

    // Select the types in a random order
    Bool random : 1,

    // Only order types which can be built
         canBuild : 1,
                    
    // If we don't have prereqs the stall the ordering process (we're important ;)
         noPrereqStall : 1,

    // Don't build if we're beyond our weighting (useful for Ratio)
         noBeyondWeighting : 1,

    // Don't allow shuffling of our order based on prereqs
         noShuffleOrder : 1;

    // Resource Multiplier
    U32 resourceMultiplierMinimum;
    U32 resourceMultiplierIncrement;
    U32 resourceMultiplierCap;

    // Types in the manifest
    NBinTree<Type> types;

    // Orderer this manfiest belongs to
    Orderer &orderer;

    // Base
    GameIdent placement;

    // Script name to put ordered units into
    GameIdent script;

  public:

    // Constructor and Destructor
    Manifest(Orderer &orderer, FScope *fScope);
    virtual ~Manifest();

    // Enter the manifest
    virtual void Enter() = 0;

    // Process the manifest
    virtual void Process() = 0;

    // Order a single type
    void Order(UnitObjType *type, Resource *resource, Water *water);

    // Order a list of types
    void Order(List<UnitObjType> &types, Resource *resource, Water *water);

    // Get the current multiplier based on resource
    U32 GetMultiplier();

  public:

    // Get the orderer this manfiest belongs to
    Orderer & GetOrderer()
    {
      return (orderer);
    }

    // Get the building plan this manfiest is using
    const GameIdent & GetPlacement()
    {
      return (placement);
    }

    // Are we meant to block if we don't get our prereqs
    Bool NoPrereqStall()
    {
      return (noPrereqStall);
    }

    // Are we not meant to build beyond our weighting
    Bool NoBeyondWeighting()
    {
      return (noBeyondWeighting);
    }

    // Are we not meant to shuffle the order
    Bool NoShuffleOrder()
    {
      return (noShuffleOrder);
    }

  public:

    // Create a manifest from the given fscope
    static Manifest * Create(Orderer &orderer, FScope *fScope);

  };

}


#endif
