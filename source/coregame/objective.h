///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Objective System
//
// 11-AUG-1998
//

#ifndef __OBJECTIVE_H
#define __OBJECTIVE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gameobj.h"
#include "condition.h"
#include "action.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Objective
//
namespace Objective
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //


  //
  // Forward Declarations
  //
  class Object;
  class Type;

  // Object Reaper
  typedef Reaper<Object> Ptr;

  // Object Reaper List Node
  class ListNode : public Ptr
  {
  public:
    NList<ListNode>::Node node;
  };

  // Object Reaper List
  typedef ReaperList<Object, ListNode> ObjList;

  // Type Reaper
  typedef Reaper<Type> TypePtr;

  // Type Reaper List Node
  class TypeListNode : public TypePtr
  {
  public:
    NList<TypeListNode>::Node node;
  };

  // Type Reaper List
  typedef ReaperList<Type, TypeListNode> TypeList;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type - An objective definition
  //
  class Type : public GameObjType
  {
    PROMOTE_LINK(Type, GameObjType, 0x53719357); // "Objective::Type"

  private:

    FScope *condition;  // The condition scope which defines the conditions to meet an objective of this type
    FScope *action;     // The action scope defines the actions to perform when an objective of thie type is met

  public:

    NBinTree<Type>::Node node;  // Type tree node

  public:

    // Constructor and Destructor
    Type(const char *name, FScope *fScope);
    ~Type();

    // Called after all types are loaded
    void PostLoad();

    // Create a new instance using this type
    GameObj* NewInstance(U32 id);

    // Friends of Type
    friend Object;
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object - Instance class for above type
  //
  class Object : public GameObj
  {
  private:

    Condition *condition;

  protected:

  public:
    
    // Constructor and destructor
    Object(Type *objType, U32 id);
    ~Object();

    // Called to before deleting the object
    void PreDelete();

    // Load and save state configuration
    void LoadState(FScope *fScope);
    void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

    // Called after all objects are loaded
    void PostLoad();

  public:

    // GetType: Get the type
    Type * GetType()
    {
      // Safe Cast
      return ((Type *) type);
    }

    // Test: Test to see if the conditions are met
    Bool Test(Team *team)
    {
      if (condition->GetTeam())
      {
        return (condition->Test(condition->GetTeam()));
      }
      else
      {
        return (condition->Test(team));
      }
    }

    // Act: Perform the actions
    void Act(Team *team)
    {
      Action::Execute(team, GetType()->action);
    }

    // GetCondition: Return the condition of this objective
    Condition * GetCondition()
    {
      return (condition);
    }

  };

  // Create a new objective
  Object * Create(const char *typeName);

  // Does a particualr type name exist ?
  Bool CheckType(const char *typeName);

  // Get Objective Types
  const NBinTree<Type> & GetTypes();
}

#endif
