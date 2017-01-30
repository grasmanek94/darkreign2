///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-AUG-1998
//

#ifndef __SQUADOBJ_H
#define __SQUADOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gameobj.h"
#include "squadobjdec.h"
#include "unitobjdec.h"
#include "team.h"
#include "formation.h"
#include "pathsearch.h"
#include "tactical.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class SquadObjType
//
class SquadObjType : public GameObjType
{
  PROMOTE_LINK(SquadObjType, GameObjType, 0xCB825DA0) // "SquadObjType"

public:

  // Constructor
  SquadObjType(const char *name, FScope *fScope);

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

};


///////////////////////////////////////////////////////////////////////////////
//
// Class SquadObj - Instance class for above type
//
class SquadObj : public GameObj
{
public:

  struct ListNode : public UnitObjPtr
  {
    // List node
    NList<ListNode>::Node node;

    // Task id
    U32 task;

    // Data used by tasks
    S32 data;

    // Has the unit completed its current task
    Bool completed;

    // Formation slot
    Formation::Slot slot;

    // Save state
    void SaveState(FScope *scope)
    {
      StdSave::TypeU32(scope, "Task", task);
      StdSave::TypeU32(scope, "Data", U32(data));
      StdSave::TypeU32(scope, "Completed", completed);
      slot.SaveState(scope->AddFunction("Slot"));
    }

    // Load state
    void LoadState(FScope *scope)
    {
      task = StdLoad::TypeU32(scope, "Task");
      data = S32(StdLoad::TypeU32(scope, "Data"));
      completed = StdLoad::TypeU32(scope, "Completed");
      slot.LoadState(scope->GetFunction("Slot"));
    }
  };
  typedef ReaperList<UnitObj, ListNode> UnitList;

private:

  // Team which this squad belongs to
  Team *team;

  // List of objects in this squad
  UnitList list;

  // Pathsearching object
  PathSearch::Finder pathFinder;

public:

  // Tactical modifiers of the squad
  Tactical::ModifierSettings settings;

public:
  
  // Constructor and destructor
  SquadObj(SquadObjType *objType, U32 id);
  ~SquadObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Set the team this squad belongs to
  void SetTeam(Team *team);

  // Get the team this squad belongs to
  Team * GetTeam();

  // Add a single unit to the squad object
  void AddUnitObj(UnitObj *obj);

  // Add a list of units to the squad
  void AddUnitObjList(UnitObjList *objList);

  // Remove a single unit from the squad
  void RemoveUnitObj(UnitObj *obj);

  // Empty the squad
  void Empty();

  // Get the unit list inside the squad
  const UnitList & GetList();

  // Get the central location of the squad
  Bool GetLocation(Vector &location);

  // Get the overall health of the squad
  S32 GetHitPoints();

  // Get the maximum health of the squad
  S32 GetMaxHitPoints();

  // Get the average health of the squad
  F32 GetAverageHealth();

  // Get the threat of the squad to the given armour class
  U32 GetThreat(U32 ac);

  // Notification sent to the squad
  void PostEvent(const Task::Event &event, Bool idle = FALSE);

  // Notification for the squad to send to the player
  Bool NotifyPlayer(U32 message, U32 param1 = 0, U32 param2 = 0);

  // Set tactical
  void SetTacticalModifierSetting(U8 modifier, U8 index);

public:

  // Get the path searching object
  PathSearch::Finder &GetPathFinder()
  {
    return (pathFinder);
  }

};


#endif