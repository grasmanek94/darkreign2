///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 08-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gameobjctrl.h"
#include "gamebabel.h"
#include "gametime.h"



///////////////////////////////////////////////////////////////////////////////
//
// Class GameObjCtrl - Manages all game object instances
//

namespace GameObjCtrl
{

  // Has the system been initialized
  static Bool initialized;

  // Death tracker for game objects
  static DTrack *dTracker;

  // All registered properties
  static BinTree<GameIdent> properties;

  // Object think list
  static NList<GameObj> listThink(&GameObj::thinkNode);

  // Object death list
  static NList<GameObj> deathList(&GameObj::deathNode);

  // Types which haven't been post loaded
  NBinTree<GameObjType> waitingPostLoadTypes(&GameObjType::ctrlNode);

  // Object lists
  NBinTree<GameObjType> objTypes(&GameObjType::ctrlNode);


  List<GameObjType> objTypesList;
  NList<GameObj> listAll(&GameObj::allNode);

  //
  // Register the construction of a game object
  //
  void RegisterConstruction(GameObj *obj, U32 id)
  {
    ASSERT(initialized);
    ASSERT(obj);

    // Register object construction
    if (id)
    {
      // Claim 'id' using DTrack system
      dTracker->RegisterConstruction(obj->dTrack, id);
    }
    else
    {
      // Get DTrack to allocate a new unique id
      dTracker->RegisterConstruction(obj->dTrack);
    }

    // Add to the all objects list
    listAll.Append(obj);
  }
  

  //
  // Register the destruction of a game object
  //
  void RegisterDestruction(GameObj *obj)
  {
    ASSERT(initialized);
    ASSERT(obj);
    ASSERT(!obj->OnThinkList());
    ASSERT(!obj->deathNode.InUse());

    // Register object destruction
    dTracker->RegisterDestruction(obj->dTrack);

    // Remove from the all objects list
    listAll.Unlink(obj);
  }

  
  //
  // Add an object to the thinking list
  //
  void AddToThinkList(GameObj *obj)
  {
    listThink.Append(obj);
  }


  //
  // Remove an object from the thinking list
  //
  void RemoveFromThinkList(GameObj *obj)
  {
    listThink.Unlink(obj);
  }


  //
  // GetProperty
  //
  // Get the crc of the property, registering if not already known
  //
  U32 GetProperty(const char *name)
  {
    ASSERT(initialized);
    
    U32 crc = Crc::CalcStr(name);

    // Does it need to be registered
    if (!properties.Exists(crc))
    {
      properties.Add(crc, new GameIdent(name));
    }

    return (crc);
  }


  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized);
    ASSERT(!objTypes.GetCount());
    ASSERT(!waitingPostLoadTypes.GetCount());
    ASSERT(!objTypesList.GetCount());
    ASSERT(!listAll.GetCount());
    ASSERT(!listThink.GetCount());
    ASSERT(!properties.GetCount());

    // Allocate a death tracker
    dTracker = new DTrack("GameObj", 1024);

    // System now initialized
    initialized = TRUE;
  }

 
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized);
    ASSERT(!listAll.GetCount());
    ASSERT(!listThink.GetCount());
    ASSERT(!deathList.GetCount());
    ASSERT(!waitingPostLoadTypes.GetCount());

    // Delete the properties
    properties.DisposeAll();

    // Delete all known types
    objTypes.UnlinkAll();
    objTypesList.DisposeAll();

    // Delete the death tracker
    delete dTracker;

    // System now shutdown
    initialized = FALSE;
  }

  
  //
  // Process a single object creation scope, returning FALSE
  // if new definition is ignored because type already exists
  //
  Bool ProcessCreateObjectType(FScope *fScope)
  {
    ASSERT(initialized);
    ASSERT(fScope);

    // Get type identifier
    GameIdent typeId = fScope->NextArgString();

    // Get class identifier
    GameIdent classId = fScope->NextArgString();

    // Does this type already exist
    if (objTypes.Exists(typeId.crc) || waitingPostLoadTypes.Exists(typeId.crc))
    {
      ERR_CONFIG(("CreateObjectType(%s, %s) already defined", typeId.str, classId.str))
      return (FALSE);
    }

    // Get new type instance from the game babel
    GameObjType *newType = GameBabel::NewGameObjType(classId, typeId.str, fScope);

    // Ensure class type was recognized
    if (!newType)
    {
      LOG_DIAG(("Class type '%s' (%08X) unrecognized", classId.str, classId.crc))
//      fScope->ScopeError("Class type '%s' (0x%08X) unrecognized", classId.str, classId.crc);
      return (FALSE);
    }

    // Add to type tree
    waitingPostLoadTypes.Add(typeId.crc, newType);
    objTypesList.Append(newType);

    // Success
    return (TRUE);
  }


  //
  // AddObjectType
  //
  // Add a code-generated object type
  //
  void AddObjectType(GameObjType *type)
  {
    ASSERT(type);

    // This type should not exist!
    if (objTypes.Exists(type->GetNameCrc()) || waitingPostLoadTypes.Exists(type->GetNameCrc()))
    {
      ERR_FATAL(("Code-generated object type '%s' already defined!", type->GetName()))
    }

    // Add to type tree
    waitingPostLoadTypes.Add(type->GetNameCrc(), type);
    objTypesList.Append(type);
  }


  //
  // Post Load all of the types
  //
  void PostLoadTypes()
  {
    NBinTree<GameObjType>::Iterator i(&waitingPostLoadTypes);

    while (GameObjType *type = i++)
    {
      type->PostLoad();
      waitingPostLoadTypes.Unlink(type);
      objTypes.Add(type->GetNameCrc(), type);
    }
  }


  //
  // Do all object thought processing
  //
  void ProcessObjectThought()
  {
    // Iterate optimally over thinking list
    NList<GameObj>::Node *node = listThink.GetHeadNode();

    while (node)
    {
      // Get the game object from this node
      GameObj *obj = node->GetData();
      
      // Is this object ready for thought processing
      if (GameTime::SimCycle() >= obj->NextThinkTime())
      {
        // Set next think time
        obj->SetNextThinkTime(GameTime::SimCycle() + obj->ThinkInterval());

        // Do the processing
        obj->ProcessThought();
      }

      // Are there any events for this object
      if (obj->GetEventCount())
      {
        obj->ProcessEvents();
      }

      // Proceed to next object
      node = node->GetNext();
    }
  }


  //
  // Destroys a single object (must be called instead of delete)
  //
  void Delete(GameObj *obj)
  {
    ASSERT(obj);

    // Remove from the dead objects list
    if (obj->deathNode.InUse())
    {
      deathList.Unlink(obj);
    }

    // Call the dispose method, which deletes this
    obj->PreDelete();
  }


  //
  // FindObject
  //
  // Find the object with the given id
  //
  GameObj * FindObject(U32 id)
  {
    NList<GameObj>::Iterator i(&listAll);

    while (GameObj *obj = i++)
    {
      if (obj->Id() == id)
      {
        return (obj);
      }
    }

    return (NULL);
  }


  //
  // Delete objects that have been marked for deletion
  //
  void DeleteDyingObjects()
  {
    if (deathList.GetCount())
    {
      // Dispose of all the objects
      GameObj *obj;
      NList<GameObj>::Iterator i(&deathList);

      while ((obj = i++) != NULL)
      {
        // Delete its sorry ass
        Delete(obj);
      }
    }
  }


  //
  // Deletes all existing objects
  //
  void DeleteAll()
  {
    NList<GameObj>::Iterator i(&listAll);

    // Mark each object for deletion
    while (GameObj *obj = i++)
    {
      MarkForDeletion(obj);
    }

    // Purge death list
    DeleteDyingObjects();
  }


  //
  // Marks an object for deletion but does not delete it immediately
  //
  void MarkForDeletion(GameObj *obj)
  {
    ASSERT(obj)
    VALIDATE(obj)

    // Whack it on the dying list
    if (!obj->deathNode.InUse())
    {
      // Tell object it is being marked for deletion
      obj->MarkedForDeletion();

      // Add it to the death list
      deathList.Append(obj);
    }
  }

}
