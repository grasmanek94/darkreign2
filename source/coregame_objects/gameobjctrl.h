///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 08-JUL-1998
//

#ifndef __GAMEOBJCTRL_H
#define __GAMEOBJCTRL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "dtrack.h"
#include "gameobj.h"
#include "reaperlist.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace GameObjCtrl - Manages all game object instances
//
namespace GameObjCtrl
{

  // List of all known game object types
  extern NBinTree<GameObjType> objTypes;
  extern NBinTree<GameObjType> waitingPostLoadTypes;
  extern List<GameObjType> objTypesList;

  // All known objects
  extern NList<GameObj> listAll;


  // Register the construction and destruction of a game object
  void RegisterConstruction(GameObj *obj, U32 id);
  void RegisterDestruction(GameObj *obj);

  // Add/remove objects to/from the thinking list
  void AddToThinkList(GameObj *obj);
  void RemoveFromThinkList(GameObj *obj);

  // Get the crc of the property, registering if not already known
  U32 GetProperty(const char *name);

  // Initialize system
  void Init();

  // Shutdown system
  void Done();

  // Process a single type creation scope, FALSE if ignored
  Bool ProcessCreateObjectType(FScope *fScope);
  
  // Add a code-generated object type
  void AddObjectType(GameObjType *type);

  // Post Load all of the types
  void PostLoadTypes();

  // Do all object thought processing
  void ProcessObjectThought();

  // Find the object with the given id
  GameObj * FindObject(U32 id);

  // Delete objects marked for death
  void DeleteDyingObjects();

  // Destroys all existing objects (must call before Done)
  void DeleteAll();

  // Marks an object for deletion but does not delete it immediately
  void MarkForDeletion(GameObj *obj);

  // Delete all items in the list and the objects they are pointing at
  template <class OBJECT, class NODE> void DeleteReaperList(ReaperList<OBJECT, NODE> *list)
  {
    // Dispose of all the objects
    NODE *obj;
    ReaperList<OBJECT, NODE>::Iterator i(list);

    while ((obj = i++) != NULL)
    {
      // Remove this item from the list
      list->Unlink(obj);

      // Delete its ass
      Delete(obj->GetData());

      // Delete the reaper
      delete obj;
    }
  }

  // Mark all items in the list as dead
  template <class OBJECT, class NODE> void MarkReaperListForDeletion(ReaperList<OBJECT, NODE> *list)
  {
    // Dispose of all the objects
    NODE *obj;
    ReaperList<OBJECT, NODE>::Iterator i(list);

    while ((obj = i++) != NULL)
    {
      // Remove this item from the list
      list->Unlink(obj);

      // Make it die
      MarkForDeletion(obj->GetData());

      // Delete the reaper
      delete obj;
    }
  }


  // Find a specific type (using the crc of the name)
  template <class TYPE> TYPE * FindType(U32 crc)
  {
    // Find the game objects type
    GameObjType *type = objTypes.Find(crc);

    if (!type)
    {
      // Check the types waiting for postload
      type = waitingPostLoadTypes.Find(crc);
    }

    // If found, attempt to promote
    return (type ? Promote::Type<TYPE>(type) : NULL);
  }


  // Find a specific type (using a string name)
  template <class TYPE> TYPE * FindType(const char *name)
  {
    return (FindType<TYPE>(Crc::CalcStr(name)));
  }
};

#endif
