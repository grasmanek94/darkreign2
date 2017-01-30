///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Some functions for manipulating unit obj lists
//
// 20-APR-1998
//


#ifndef __UNITOBJLIST_H
#define __UNITOBJLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobj.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace UnitObjListUtil
//
namespace UnitObjListUtil
{

  //
  // PurgeNonTeamOffMap
  //
  // Purge non team, off map and dead objects from a unitobjlist
  //
  template <class DATA, class NODE> Bool PurgeNonTeamOffMap(ReaperList<DATA, NODE> &list, const Team *team)
  {
    Bool found = FALSE;
    NODE *node;
    ReaperList<DATA, NODE>::Iterator i(&list);

    // Step through each reaper
    while ((node = i++) != NULL)
    {
      // Do we need to filter out this object
      if (!node->Alive() || !node->GetData()->OnMap() || (team && (team != node->GetData()->GetTeam())))
      {
        list.Dispose(node);
        found = TRUE;
      }
    }
    return (found);
  }

}







#endif