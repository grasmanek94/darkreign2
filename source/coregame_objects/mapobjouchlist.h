///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//

#ifndef __MAPOBJOUCHLIST_H
#define __MAPOBJOUCHLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "reaperlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Struct MapObjOuchListNode
//
struct MapObjOuchListNode : public Reaper<MapObj>
{
  U32 age;
  NList<MapObjOuchListNode>::Node node;
};


///////////////////////////////////////////////////////////////////////////////
//
// Class MapObjOuchList
//
// The "Ouch" list (aka the ochie mama list) is a reaper list of map objects
// who had their hitpoints recently modified.
//
class MapObjOuchList : public ReaperList<MapObj, MapObjOuchListNode>
{
public:

  // Append the given data to the list, if its already there then reset its age
  Bool AppendNoDup(MapObj *data)
  {
    MapObjOuchListNode *node = Find(data);
    
    if (!node)
    {
      node = Append(data);
    }
    node->age = 0;
    return (TRUE);
  }
};

#endif