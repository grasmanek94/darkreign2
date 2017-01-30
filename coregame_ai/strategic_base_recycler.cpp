/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Recycler
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_base_recycler.h"
#include "strategic_base_state.h"
#include "strategic_object.h"
#include "strategic_util.h"
#include "orders_game.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Recycler
  //

  
  //
  // Base::Recycler::Recycler
  //
  Base::Recycler::Recycler(Base::State &state, FScope *fScope) 
  : state(state),
    types(&Type::nodeList)
  {
    cash = StdLoad::TypeU32(fScope);

    while (FScope *sScope = fScope->NextFunction())
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          UnitObjType *type = Util::LoadType(sScope, state.GetBase().GetObject().GetTeam());
          types.Append(new Type(type, StdLoad::TypeU32(sScope)));
          break;
        }
      }
    }
  }


  //
  // Base::Recycler::~Recycler
  //
  // Destructor
  //
  Base::Recycler::~Recycler()
  {
    // Delete all of the types
    types.DisposeAll();
  }


  //
  // Process the recycler
  //
  void Base::Recycler::Process()
  {
    // Work out how much resource is pending from refunds
    U32 refund = 0;
    for (UnitObjList::Iterator u(&state.GetBase().GetManager().GetRefunds()); *u; ++u)
    {
      refund += U32(F32((**u)->GetResourceValue() * (**u)->UnitType()->GetRecyclePercentage()));
    }

    Object &object = state.GetBase().GetObject();
    Team *team = object.GetTeam();
    U32 resource = team->GetResourceStore();

    // Do we have less cash now than the limit ?
    if ((resource + refund) < cash)
    {
      for (NList<Type>::Iterator t(&types); *t; ++t)
      {
        const NList<UnitObj> *units = team->GetUnitObjects((*t)->type->GetNameCrc());

        if (units && units->GetCount() > (*t)->minimum)
        {
          // Order this unit to be recycled
          UnitObj *unit = units->GetHead();
          Orders::Game::ClearSelected::Generate(object);
          Orders::Game::AddSelected::Generate(object, unit);
          Orders::Game::Recycle::Generate(object);
          state.GetBase().GetManager().AddRefund(unit);
          break;
        }
      }
    }

  }

}
