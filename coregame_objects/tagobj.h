///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-AUG-1998
//

#ifndef __TAGOBJ_H
#define __TAGOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gameobj.h"
#include "mapobjdec.h"
#include "tagobjdec.h"
#include "relationaloperator.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class TagObjType - A list of objects that forms a 'tagged' group
//
class TagObjType : public GameObjType
{
  PROMOTE_LINK(TagObjType, GameObjType, 0xD62D22B9); // "TagObjType"

public:

  // Constructor
  TagObjType(const char *name, FScope *fScope);

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class TagObj - Instance class for above type
//

class TagObj : public GameObj
{
public:

  // List of all current TagObj instances
  static NBinTree<TagObj> allTags;

  // Find a tag by namecrc
  static TagObj * FindTag(U32 tagNameCrc);

  // Find a tag by name
  static TagObj * FindTag(const char *tagName);
  
  // Create tag using 'list' (NULL if invalid type, tag exists, or list empty)
  static TagObj * CreateTag(const char *tagName, const MapObjList &list);

  // Create an empty tag
  static TagObj * CreateTag(const char *tagName);

private:

  // Tag name of this group
  GameIdent name;

  // Tree node
  NBinTree<TagObj>::Node node;

public:

  // List of objects in this tagged group
  MapObjList list;
  
  // Constructor and destructor
  TagObj(TagObjType *objType, U32 id);
  ~TagObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Get the location of the tag
  Bool GetLocation(Vector &location);

  // Check to see if enough units of the types described are within the proximity of the tag
  Bool CheckTeamProximity(F32 range, Team *team, U32 amount, RelationalOperator<U32> &oper);
  Bool CheckTeamProximity(F32 range, Team *team, U32 amount, RelationalOperator<U32> &oper, MapObjType *type);
  Bool CheckTeamProximity(F32 range, Team *team, U32 amount, RelationalOperator<U32> &oper, U32 property);

  // Returns the name of this tag
  const char * TagName()
  {
    return (name.str);
  }

};

#endif