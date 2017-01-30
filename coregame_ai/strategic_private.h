/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI Private Parts
//
// 31-AUG-1998
//


#ifndef __STRATEGIC_PRIVATE_H
#define __STRATEGIC_PRIVATE_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  // RegisterObject
  void RegisterObject(class Object *object);

  // RegisterPersonality
  void RegisterPersonality(class Personality *personality);

  // Register the construction of an AI item
  void RegisterConstruction(DTrack::Info &info);

  // Register the destruction of the AI item 'info'
  void RegisterDestruction(DTrack::Info &info);

}


#endif
