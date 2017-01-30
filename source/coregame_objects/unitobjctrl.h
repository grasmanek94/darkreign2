///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 10-JUL-1998
//

#ifndef __UNITOBJCTRL_H
#define __UNITOBJCTRL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobjdec.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace UnitObjCtrl
//
namespace UnitObjCtrl
{

  // Initialize system
  void Init();

  // Shutdown system
  void Done();

  // Postload
  void PostLoad();
  
  // Register Resource cost of a unit
  void RegisterCost(U32 cost);

  // Get the maximum cost of a unit
  U32 GetMaximumCost();
  F32 GetMaximumCostInv();

  // Register Line of sight of an anit-clandestine unit
  void RegisterAntiClandestineSeeingRange(F32 val);

  // Register Line of sight of a disruptor unit
  void RegisterDisruptorSeeingRange(F32 val);

  // Get the maximum seeing range of an anti-clandestine unit
  F32 GetAntiClandestineSeeingRange();

  // Get the maximum seeing range of a disruptor unit
  F32 GetDisruptorSeeingRange();

  // Register a type as one which is sensitve to time changes
  void RegisterTimeSensitive(UnitObjType *unitType);

  // Update all types which require notifaction when the light time changes (val is pct day)
  void UpdateTimeSensitive(F32 val);

  // Initialize resources for all units
  void InitializeResources();


  // Register a type as one which can be infiltrated
  void RegisterInfiltratable(UnitObjType *unitType);

  // Get the list of types which can be infiltrated
  const List<UnitObjType> & GetInfiltratableTypes();

}

#endif