///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tagobj.h"
#include "stdload.h"
#include "gameobjctrl.h"
#include "resolver.h"
#include "mapobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "TagObj"



///////////////////////////////////////////////////////////////////////////////
//
// Class TagObjType - A list of objects that forms a 'tagged' group
//


//
// TagObjType::TagObjType
//
// Constructor
//
TagObjType::TagObjType(const char *name, FScope *fScope) : GameObjType(name, fScope)
{
}


//
// TagObjType::PostLoad
//
// Called after all types are loaded
//
void TagObjType::PostLoad()
{
  // Call parent scope first
  GameObjType::PostLoad();
}


//
// TagObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* TagObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new TagObj(this, id));
}


///////////////////////////////////////////////////////////////////////////////
//
// Class TagObj - Instance class for above type
//


//
// Static data
//
NBinTree<TagObj> TagObj::allTags(&TagObj::node);



//
// TagObj::FindTag
//
// Find a tag by name crc
//
TagObj * TagObj::FindTag(U32 tagNameCrc)
{
  // Find the tag
  return (allTags.Find(tagNameCrc));
}


//
// TagObj::FindTag
//
// Find a tag by name
//
TagObj* TagObj::FindTag(const char *tagName)
{
  return (FindTag(Crc::CalcStr(tagName)));
}


//
// TagObj::CreateTag
//
// Create tag using 'list' (NULL if invalid type, tag exists, or list empty)
//
TagObj * TagObj::CreateTag(const char *tagName, const MapObjList &list)
{
  // Is there already a tag with this name ?
  TagObj *obj = FindTag(tagName);

  if (!obj)
  {
    // Create a new tag
    obj = CreateTag(tagName);
  }

  // Clear out the list
  obj->list.Clear();

  // Copy the target list
  obj->list.Dup(list);

  // Purge dead from the list
  obj->list.PurgeDead();

  return (obj);
}


//
// TagObj::CreateTag
//
// Create an empty tag
//
TagObj * TagObj::CreateTag(const char *tagName)
{
  // Find the type 
  TagObjType *type = GameObjCtrl::FindType<TagObjType>("Tag");

  // Were we successful
  if (type)
  {
    // Create a new tag object (safe cast)
    TagObj *obj = (TagObj *) type->NewInstance(0);
  
    // Set the name of the tag
    obj->name = tagName;

    // Add to the tag list
    allTags.Add(obj->name.crc, obj);

    return (obj);
  }

  return (NULL);
}


//
// TagObj::TagObj
//
// Constructor
//
TagObj::TagObj(TagObjType *objType, U32 id) 
: GameObj(objType, id)
{
  // Set default tag name
  name = "No Name";
}


//
// TagObj::TagObj
//
// Destructor
//
TagObj::~TagObj()
{
  // Remove from the tag list
  allTags.Unlink(this);

  // Make sure we clear the reaper list
  list.Clear();
}


//
// TagObj::PreDelete
//
// Pre deletion cleanup
//
void TagObj::PreDelete()
{

  // Call parent scope last
  GameObj::PreDelete();
}

  
//
// TagObj::LoadState
//
// Load a state configuration scope
//
void TagObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  GameObj::LoadState(fScope);

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Get tag name
  name = StdLoad::TypeString(fScope);

  // Now that the name is know add it to the all tags list
  allTags.Add(name.crc, this);

  // Load the object list
  StdLoad::TypeReaperList(fScope, "ReaperList", list);
}


//
// TagObj::SaveState
//
// Save a state configuration scope
//
void TagObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  GameObj::SaveState(fScope);

  // Save config scope with tag name
  fScope = StdSave::TypeString(fScope, SCOPE_CONFIG, name.str);
  
  // Save the object list
  StdSave::TypeReaperList(fScope, "ReaperList", list);
}


//
// TagObj::PostLoad
//
// Called after all objects are loaded
//
void TagObj::PostLoad()
{
  // Call parent scope first
  GameObj::PostLoad();

  // Resolve the object list
  Resolver::ObjList<MapObj, MapObjType, MapObjListNode>(list);
}


//
// TagObj::GetLocation
//
// Get the locationMessage of the tag
//
Bool TagObj::GetLocation(Vector &location)
{
  location.ClearData();
  U32 count = 0;

  // Iterate over the objects and for each one which is on the map average the location
  for (MapObjList::Iterator i(&list); *i; i++)
  {
    if ((*i)->Alive())
    {
      if ((**i)->OnMap())
      {
        count++;
        location += (**i)->Origin();
      }
    }
  }

  // If there were any objects calculate the average
  if (count)
  {
    location /= (F32) count;
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}



//
// TagObj::CheckTeamProximity
//
// Check to see if enough units of the types described are within the proximity of the tag
//
Bool TagObj::CheckTeamProximity(F32 range, Team *team, U32 amount, RelationalOperator<U32> &oper)
{
  ASSERT(team)

  Vector location;
  U32 count = 0;
  F32 range2 = range * range;

  if (GetLocation(location))
  {
    List<MapCluster> clusters;
    WorldCtrl::BuildClusterList(clusters, Point<F32>(location.x, location.z), range);

    for (List<MapCluster>::Iterator cluster(&clusters); *cluster; cluster++)
    {
      for (NList<UnitObj>::Iterator o(&(*cluster)->unitList); *o; o++)
      {
        UnitObj *unit = *o;

        if (
          (unit->GetActiveTeam() == team) && 
          (unit->Origin() - location).Magnitude2() < range2)
        {
          count++;
        }
      }
    }

    // Clean up clusters
    clusters.UnlinkAll();
  }

  // Perform the test
  return (oper(count, amount));
}


//
// TagObj::CheckTeamProximity
//
// Check to see if enough units of the types described are within the proximity of the tag
//
Bool TagObj::CheckTeamProximity(F32 range, Team *team, U32 amount, RelationalOperator<U32> &oper, MapObjType *type)
{
  ASSERT(team)

  Vector location;
  U32 count = 0;
  F32 range2 = range * range;

  if (GetLocation(location))
  {
    List<MapCluster> clusters;
    WorldCtrl::BuildClusterList(clusters, Point<F32>(location.x, location.z), range);

    for (List<MapCluster>::Iterator cluster(&clusters); *cluster; cluster++)
    {
      for (NList<UnitObj>::Iterator o(&(*cluster)->unitList); *o; o++)
      {
        UnitObj *unit = *o;

        if (
          (unit->GetActiveTeam() == team) && 
          (unit->MapType()->Id() == type->Id()) && 
          (unit->Origin() - location).Magnitude2() < range2)
        {
          count++;
        }
      }
    }

    // Clean up clusters
    clusters.UnlinkAll();
  }

  // Perform the test
  return (oper(count, amount));
}


//
// TagObj::CheckTeamProximity
//
// Check to see if enough units of the types described are within the proximity of the tag
//
Bool TagObj::CheckTeamProximity(F32 range, Team *team, U32 amount, RelationalOperator<U32> &oper, U32 property)
{
  ASSERT(team)

  Vector location;
  U32 count = 0;
  F32 range2 = range * range;

  if (GetLocation(location))
  {
    List<MapCluster> clusters;
    WorldCtrl::BuildClusterList(clusters, Point<F32>(location.x, location.z), range);

    for (List<MapCluster>::Iterator cluster(&clusters); *cluster; cluster++)
    {
      for (NList<UnitObj>::Iterator o(&(*cluster)->unitList); *o; o++)
      {
        UnitObj *unit = *o;

        if (
          (unit->GetActiveTeam() == team) && 
          (unit->MapType()->HasProperty(property)) && 
          (unit->Origin() - location).Magnitude2() < range2)
        {
          count++;
        }
      }
    }

    // Clean up clusters
    clusters.UnlinkAll();
  }

  // Perform the test
  return (oper(count, amount));
}
