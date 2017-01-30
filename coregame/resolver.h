///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 06-AUG-1998
//

#ifndef __RESOLVER_H
#define __RESOLVER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "reaperlist.h"
#include "gameobjctrl.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Resolver
//
namespace Resolver
{

  //
  // Find an object of a particular type using an ID
  //
  template <class OBJECT, class TYPE> OBJECT * Object(U32 id)
  {
    // The resolve id is the actual id of the object
    if (GameObj *target = GameObjCtrl::FindObject(id))
    {
      OBJECT *promoted = Promote::Object<TYPE, OBJECT>(target);

      // If we succeeded, setup the reaper
      if (promoted)
      {
        return (promoted);
      }
      else
      {
        LOG_DIAG(("Object (id %u) was the wrong type", id))
      }
    }

    return (NULL);
  }


  //
  // Resolve a reaper that contains the ID of an object
  //
  template <class OBJECT, class TYPE> Bool Object(Reaper<OBJECT> &ptr)
  {
    // Does 'ptr' need to be resolved
    if (ptr.HasResolveId())
    {
      // The resolve id is the actual id of the object
      if (GameObj *target = GameObjCtrl::FindObject(ptr.GetResolveId()))
      {
        OBJECT *promoted = Promote::Object<TYPE, OBJECT>(target);

        // If we succeeded, setup the reaper
        if (promoted)
        {
          ptr.Setup(promoted);
        }
        else
        {
          LOG_DIAG(("Object (id %u) was the wrong type", ptr.GetResolveId()));
          ptr.Clear();
          return (FALSE);
        }
      }
      else
      {
        ptr.Clear();
        return (FALSE);
      }
    }
    return (TRUE);
  }


  //
  // Resolve a reaper list that contains ID's of objects
  //
  template <class OBJECT, class TYPE, class NODE> void ObjList(ReaperList<OBJECT, NODE> &list)
  {
    for (ReaperList<OBJECT, NODE>::Iterator i(&list); *i; i++)
    {
      Resolver::Object<OBJECT, TYPE>(**i);
    }

    // Always return a list of alive objects
    list.PurgeDead();
  }


  //
  // Resolve a GameObjType reaper from a type name
  //
  template <class TYPE> void Type(Reaper<TYPE> &ptr, const GameIdent &ident, Bool required = FALSE)
  {
    // Find the type
    TYPE *target = GameObjCtrl::FindType<TYPE>(ident.crc);

    if (target)
    {
      // Setup the reaper
      ptr = target;
    }
    else
    {
      if (required)
      {
        ERR_CONFIG(("Object type '%s' [%08X] does not exist or wrong code class", ident.str, ident.crc))
      }
      else
      {
        LOG_WARN(("Object type '%s' [%08X] does not exist or wrong code class", ident.str, ident.crc))
        ptr.Clear();   
      }
    }
  }

  
  //
  // Resolve a GameObjType reaper that contains a pointer to a game ident
  //
  template <class TYPE> void Type(Reaper<TYPE> &ptr, Bool required = FALSE)
  {
    // Does 'ptr' need to be resolved
    if (ptr.HasResolveId())
    {
      // The resolve id is a pointer to an allocated GameIdent
      GameIdent * ident = reinterpret_cast<GameIdent *>(ptr.GetResolveId());

      // If this triggers, then the reaper was NOT setup for this function
      VALIDATE(ident);

      // Resolve it
      Resolver::Type<TYPE>(ptr, *ident, required);

      // Our responsibility to delete the GameIdent
      delete ident;
    }
    else

    if (required)
    {
      ERR_CONFIG(("A required type reaper was missing a resolve id"));
      Debug::CallStack::Dump();
    }
  }


  //
  // Resolve a GameObjType reaper list that contains pointers to game idents
  //
  template <class TYPE, class NODE> void TypeList(ReaperList<TYPE, NODE> &list)
  {
    for (ReaperList<TYPE, NODE>::Iterator i(&list); *i; i++)
    {
      Resolver::Type<TYPE>(**i);
    }

    // Always return a list of alive objects
    list.PurgeDead();
  }
}

#endif