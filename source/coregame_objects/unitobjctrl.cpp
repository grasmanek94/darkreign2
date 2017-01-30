///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 10-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobjctrl.h"
#include "unitobj.h"
#include "gameobjctrl.h"
#include "sight.h"
#include "gamegod.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace UnitObjCtrl
//
namespace UnitObjCtrl
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Initialized Flag
  static Bool initialized;

  // Maximum resource cost a unit type
  static U32 maxCost;
  static F32 maxCostInv;

  // The maximum seeing ranges of certain unit types
  static F32 maxAntiClandestineSeeingRange;
  static F32 maxDisruptorSeeingRange;

  // Types which require notification that the time has changed
  static List<UnitObjType> typesTimeSensitive;

  // List of types which have changed their seeing range
  static NList<UnitObjType> typesSightChanged(&UnitObjType::sightChangeNode);

  // Types which can be infiltrated
  static List<UnitObjType> infiltratable;


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized)

    // Reset maximum resource cost
    maxCost = 0;
    maxCostInv = 0.0f;

    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized)

    // Clear time sensitive types
    typesTimeSensitive.UnlinkAll();
    ASSERT(!typesSightChanged.GetCount())

    // Unlink all infiltratable types
    infiltratable.UnlinkAll();

    initialized = FALSE;
  }


  //
  // PostLoad
  //
  // Perform postload processing
  //
  void PostLoad()
  {
    ASSERT(initialized)
  }
  

  //
  // RegisterCost
  //
  // Register Resource Cost
  //
  void RegisterCost(U32 cost)
  {
    ASSERT(initialized)

    maxCost = Max(maxCost, cost);
    maxCostInv = maxCost ? 1.0f / ((F32) maxCost) : 0.0f;
  }


  //
  // GetMaximumCost
  //
  // Get the maximum resource cost of a type
  //
  U32 GetMaximumCost()
  {
    ASSERT(initialized)

    return (maxCost);
  }


  //
  // GetMaximumCostInv
  //
  // Get the inverse of the maximum resource cost of a type
  //
  F32 GetMaximumCostInv()
  {
    ASSERT(initialized)

    return (maxCostInv);
  }


  //
  // RegisterAntiClandestineSeeingRange
  //
  // Register Line of sight of an anit-clandestine unit
  //
  void RegisterAntiClandestineSeeingRange(F32 val)
  {
    ASSERT(initialized)

    maxAntiClandestineSeeingRange = Max(maxAntiClandestineSeeingRange, val);
  }

  
  //
  // RegisterDisruptorSeeingRange
  //
  // Register Line of sight of a disruptor unit
  //
  void RegisterDisruptorSeeingRange(F32 val)
  {
    ASSERT(initialized)

    maxDisruptorSeeingRange = Max(maxDisruptorSeeingRange, val);
  }


  //
  // GetAntiClandestingSeeingRange
  //
  // Get the maximum seeing range of an anti-clandestine unit
  //
  F32 GetAntiClandestineSeeingRange()
  {
    return (maxAntiClandestineSeeingRange);
  }

  
  //
  // GetDisruptorSeeingRange
  //
  // Get the maximum seeing range of a disruptor unit
  //
  F32 GetDisruptorSeeingRange()
  {
    return (maxDisruptorSeeingRange);
  }


  //
  // RegisterTimeSensitive
  //
  // Register a type as one which is sensitve to time changes
  //
  void RegisterTimeSensitive(UnitObjType *unitType)
  {
    typesTimeSensitive.Append(unitType);
  }


  //
  // UpdateTimeSensitive
  //
  // Update all types which require notifaction when the light time changes
  //
  void UpdateTimeSensitive(F32 val)
  {
    for (List<UnitObjType>::Iterator i(&typesTimeSensitive); *i; i++)
    {
      if ((*i)->UpdateSeeingRange(val))
      {
        typesSightChanged.Append(*i);
      }
    }

    if (typesSightChanged.GetCount())
    {
      Sight::RescanAllTypes(typesSightChanged);
      typesSightChanged.UnlinkAll();
    }
  }

  
  //
  // InitializeResources
  //
  // Initialize resources for all units
  //
  void InitializeResources()
  {
    GameGod::Loader::SubSystem("#game.loader.resources", GameObjCtrl::objTypesList.GetCount());

    for (List<GameObjType>::Iterator t(&GameObjCtrl::objTypesList); *t; ++t)
    {
      if (UnitObjType *ut = Promote::Type<UnitObjType>(*t))
      {
        ut->InitializeResources();
      }

      GameGod::Loader::Advance();
    }
  }


  //
  // Register a type as one which can be infiltrated
  //
  void RegisterInfiltratable(UnitObjType *unitType)
  {
    infiltratable.Append(unitType);
  }


  //
  // Get the list of types which can be infiltrated
  //
  const List<UnitObjType> & GetInfiltratableTypes()
  {
    return (infiltratable);
  }

}
