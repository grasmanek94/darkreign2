///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 03-JUN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "worldload.h"
#include "ptree.h"
#include "missions.h"
#include "gameobjctrl.h"
#include "mapobjdec.h"
#include "stdload.h"
#include "mapobjctrl.h"
#include "team.h"
#include "promote.h"
#include "gamegod.h"
#include "coregame.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace WorldLoad - Processes a world definition file
//

namespace WorldLoad
{

  //
  // SafeLoadMapObject
  //
  // Load the bare minimum for a map object
  //
  static void SafeLoadMapObject(MapObjType *type, FScope *fScope, U32 id)
  {
    ASSERT(type && fScope);

    FScope *sScope;
    FScope *ssScope;
    Matrix position;
    Bool zip = FALSE;

    // Get map object config scope
    if ((sScope = fScope->GetFunction("MapObj", FALSE)) == NULL)
    {
      LOG_DIAG(("SafeLoad: Didn't find MapObj scope for %s (%u)", type->typeId.str, id));
      return;
    }

    // Get the position scope
    if ((ssScope = sScope->GetFunction("Position", FALSE)) == NULL)
    {
      LOG_DIAG(("SafeLoad: Discarding %s (%u) (no position)", type->typeId.str, id));
      return;
    }

    // Get the zipping state
    if (sScope->GetFunction("Zip", FALSE))
    {
      zip = TRUE;
    }

    // Load the position
    StdLoad::TypeMatrix(ssScope, position);

    // Now have enough info to create an object
    MapObj *obj = MapObjCtrl::ObjectNewOnMap(type, position, id, zip);

    // Was the supplied type actually a unit ?
    UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(obj);

    if (unitObj)
    {
      // Get map object config scope
      if ((sScope = fScope->GetFunction("UnitObj", FALSE)) == NULL)
      {
        LOG_ERR(("SafeLoad: Didn't find UnitObj scope for %s (%u)", type->typeId.str, id));
        return;
      }

      // Optionally set the team 
      if ((ssScope = sScope->GetFunction("Team", FALSE)) != NULL)
      {
        unitObj->SetTeam(Team::Name2Team(StdLoad::TypeString(ssScope)));
      }
    }
  }


  //
  // LoadCreateObject
  //
  // Load a CreateObject function
  //
  static void LoadCreateObject(FScope *fScope, Bool doSafeLoad)
  {
    ASSERT(fScope);

    // Get type name
    const char *typeName = fScope->NextArgString();

    // Get claimed id
    U32 id = fScope->NextArgInteger();

    // The id can not be zero
    if (!id)
    {
      ERR_FATAL(("Caught zero id in CreateObject"));
    }

    // Get a pointer to the type
    GameObjType *type = GameObjCtrl::FindType<GameObjType>(typeName);

    // Make sure we found it
    if (!type)
    {
      LOG_WARN(("Ignoring unknown type in world file '%s'", typeName));
      return;
    }

    // Do we need to do a safe load
    if (doSafeLoad)
    {
      /*
      // Filter out certain types
      if 
      (
        Promote::Type<WeaponObjType>(type) // ||
      )
      {
        LOG_DIAG(("Safe Load : Filtering %s (%u)", type->typeId.str, id));
        return;
      }
      */

      // Is this a map object
      MapObjType *mapType = Promote::Type<MapObjType>(type);

      // If so, do special safe load
      if (mapType)
      {
        SafeLoadMapObject(mapType, fScope, id);
        return;
      }
    }

    // Create the object using the type
    GameObj *obj = type->NewInstance(id);

    ASSERT(obj);

    // Load full state info
    obj->LoadState(fScope);
  }


  //
  // SaveCreateObject
  //
  // Save a CreateObject function
  //
  static void SaveCreateObject(FScope *fScope, GameObj *obj)
  {
    ASSERT(fScope);

    // Only save this object if we're allowed to save it
    if (obj->Save())
    {
      // Create the function
      fScope = fScope->AddFunction("CreateObject");

      // Arguments are object type and id
      fScope->AddArgString(obj->TypeName());
      fScope->AddArgInteger(obj->Id());

      // Tell object to save its own state
      obj->SaveState(fScope);
    }
  }


  //
  // SaveObjectFile
  //
  // Save all objects, false if unable to create file
  //
  Bool SaveObjectFile(const char *name)
  {
    PTree pTree;
    FilePath path;

    // Get the global scope of the parse tree
    FScope *gScope = pTree.GetGlobalScope();

    // Save every game object
    for (NList<GameObj>::Iterator i(&GameObjCtrl::listAll); *i; i++)
    {
      SaveCreateObject(gScope, *i);
    }

    // Work out save path
    Dir::PathMake(path, Missions::GetWritePath(), name);
  
    // Write the file
    if (!CoreGame::WriteTree(pTree, path.str))
    {
      LOG_WARN(("Unable to write to file [%s]", path.str));
      return (FALSE);
    }

    // Success
    return (TRUE);
  }


  //
  // LoadObjectFile
  //
  // Load a object definition file into current state, false if not found
  //
  Bool LoadObjectFile(const char *name)
  {
    ASSERT(name);

    PTree pTree;
    Bool safe = Missions::GetSafeLoad();

    // Parse the file
    GameGod::Loader::SubSystem("#game.loader.createobjects", 1);

    if (pTree.AddFile(name))
    {
      // Get the global scope
      FScope *gScope = pTree.GetGlobalScope();
      FScope *sScope;

      // Process each function
      GameGod::Loader::SubSystem("#game.loader.createobjects", gScope->GetBodyCount());

      while ((sScope = gScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0xCAE286FA: // "CreateObject"
            LoadCreateObject(sScope, safe);
            GameGod::Loader::Advance();
            break;
        }
      }

      if (GameGod::CheckObjects())
      {
        FSCOPE_CHECK(gScope)
      }

      // Call each existing game object
      GameGod::Loader::SubSystem("#game.loader.configureobjects", GameObjCtrl::listAll.GetCount());

      for (NList<GameObj>::Iterator i(&GameObjCtrl::listAll); *i; i++)
      {
        // Are we in safe-mode and this is a map object
        if (safe && Promote::Object<MapObjType, MapObj>(*i))
        {
        }
        else
        {
          // Call virtual post-load function
          (*i)->PostLoad();
        }

        GameGod::Loader::Advance();
      }

      return (TRUE);
    }

    // Didn't find the file
    return (FALSE);
  }
}
