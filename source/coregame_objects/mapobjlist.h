///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Some functions for manipulating map obj lists
//
// 20-APR-1998
//


#ifndef __MAPOBJLIST_H
#define __MAPOBJLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MapObjListUtil
//
namespace MapObjListUtil
{

  //
  // PurgeOffMap
  //
  // Purge off map and dead objects from a mapobjlist
  //
  template <class DATA, class NODE> Bool PurgeOffMap(ReaperList<DATA, NODE> &list)
  {
    Bool found = FALSE;
    NODE *node;
    ReaperList<DATA, NODE>::Iterator i(&list);

    // Step through each reaper
    while ((node = i++) != NULL)
    {
      // Do we need to filter out this object
      if (!node->Alive() || !node->GetData()->OnMap())
      {
        list.Dispose(node);
        found = TRUE;
      }
    }
    return (found);
  }

}







#endif