/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Recycler
// 25-MAR-1999
//


#ifndef __STRATEGIC_BASE_RECYCLER_H
#define __STRATEGIC_BASE_RECYCLER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base.h"
#include "strategic_weighting.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Base::Recycler
  //
  class Base::Recycler
  {
  private:

    struct Type
    {
      // Type
      UnitObjType *type;

      // Minimum
      U32 minimum;

      // Node list
      NList<Type>::Node nodeList;

      Type(UnitObjType *type, U32 minimum)
      : type(type),
        minimum(minimum)
      {
      }

    };

    // Base state which this orderer belongs to
    Base::State &state;

    // Cash limit
    U32 cash;

    // Types in the recycler
    NList<Type> types;

    // List node
    NList<Recycler>::Node nodeList;

  public:

    // Constructor
    Recycler(Base::State &state, FScope *fScope);

    // Destructor
    ~Recycler();

    // Save and load state
    void Save(FScope *scope);
    void Load(FScope *scope);

    // Process the recycler
    void Process();

  public:

    // Friends
    friend class Base::State;

  };

}



#endif
