/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Orderer Type
//


#ifndef __STRATEGIC_BASE_ORDERER_TYPE_H
#define __STRATEGIC_BASE_ORDERER_TYPE_H


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
  // Class Base::Orderer::Type
  //
  struct Base::Orderer::Type
  {

    // Type
    UnitObjType *type;

    // Amount
    U32 amount;

    // Tree Node
    NBinTree<Type>::Node nodeTree;

    // Loading Constructor
    Type(FScope *fScope, Team *team = NULL);

    // Initializing Constructor
    Type(UnitObjType &type);

    // Save state
    void SaveState(FScope *scope);
  };

}

#endif
