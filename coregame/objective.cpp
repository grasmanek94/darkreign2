///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Objective System
//
// 11-AUG-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "objective.h"
#include "stdload.h"
#include "resolver.h"


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

  // Config scope name for type and world files
  #define SCOPE_CONFIG   "ObjectiveObj"


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static Bool           initialized = FALSE; // Initialization flag
  static NBinTree<Type> types(&Type::node);  // Types of objectives


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //

  
  //
  // Type::Type
  //
  // Constructor
  //
  Type::Type(const char *name, FScope *fScope) : GameObjType(name, fScope)
  {
    // Get specific config scope
    FScope *specificScope = fScope->GetFunction(SCOPE_CONFIG, FALSE);

    // Either use the specific or the root scope
    if (specificScope)
    {
      fScope = specificScope;
    }

    // Condition
    FScope *sScope;
    sScope = fScope->GetFunction("Condition");
    condition = sScope->Dup();

    // Action
    sScope = fScope->GetFunction("Action");
    action = sScope->Dup();

    // Add this type to the types tree
    types.Add(typeId.crc, this);
  }


  //
  // Type::~Type
  //
  // Destructor
  //
  Type::~Type()
  {
    delete condition;
    delete action;

    // Remove this type from the tree
    types.Unlink(this);
  }


  //
  // Type::PostLoad
  //
  // Called after all types are loaded
  //
  void Type::PostLoad()
  {
    // Call parent scope first
    GameObjType::PostLoad();
  }


  //
  // Type::NewInstance
  //
  // Create a new map object instance using this type
  //
  GameObj* Type::NewInstance(U32 id)
  {
    // Allocate new object instance
    return (new Object(this, id));
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object - Instance class for above type
  //


  //
  // Object::Object
  //
  // Constructor
  //
  Object::Object(Type *objType, U32 id) : GameObj(objType, id)
  {
    // Create the condition from the information in the type
    condition = Condition::Create(objType->condition);
  }


  //
  // Object::Object
  //
  // Destructor
  //
  Object::~Object()
  {
    delete condition;
  }


  //
  // Object::PreDelete
  //
  // Pre deletion cleanup
  //
  void Object::PreDelete()
  {
    // Call parent scope last
    GameObj::PreDelete();
  }


  //
  // Object::SaveState
  //
  // Save a state configuration scope
  //
  void Object::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
  {
    // Call parent scope first
    GameObj::SaveState(fScope, theMesh);

    // Create our specific config scope
    fScope = fScope->AddFunction(SCOPE_CONFIG);

    // Give the condition its chance to save information if it wants to
    condition->Save(fScope->AddFunction("Condition"));
  }

  
  //
  // Object::LoadState
  //
  // Load a state configuration scope
  //
  void Object::LoadState(FScope *fScope)
  {
    // Call parent scope first
    GameObj::LoadState(fScope);

    // Get specific config scope
    fScope = fScope->GetFunction(SCOPE_CONFIG);

    // Load state information for the condition
    condition->Load(fScope->GetFunction("Condition"));
  }


  //
  // Object::PostLoad
  //
  // Called after all objects are loaded
  //
  void Object::PostLoad()
  {
    // Call parent scope first
    GameObj::PostLoad();

    // PostLoad the condition
    condition->PostLoad();
  }


  //
  // Create
  //
  // Create a new objective
  //
  Object * Create(const char *typeName)
  {
    // Look for this type in the type tree
    Type * type = types.Find(Crc::CalcStr(typeName));

    // If we found a type then create a new instance (safe cast)
    return (type ? (Object *) type->NewInstance(0) : NULL);
  }


  //
  // CheckType
  //
  // Does a particualr type name exist ?
  //
  Bool CheckType(const char *typeName)
  {
    return (types.Exists(Crc::CalcStr(typeName)));
  }


  //
  // GetTypeList
  //
  // Get Objective Types
  //
  const NBinTree<Type> & GetTypes()
  {
    return (types);
  }

}
