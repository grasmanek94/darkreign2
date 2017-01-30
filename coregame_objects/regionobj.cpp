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
#include "regionobj.h"
#include "stdload.h"
#include "gameobjctrl.h"
#include "worldctrl.h"
#include "team.h"
#include "promote.h"
#include "unitobj.h"
#include "tagobj.h"
#include "sight.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "RegionObj"



///////////////////////////////////////////////////////////////////////////////
//
// Class RegionObjType - A labelled region composed of (disjoint) map areas
//


//
// RegionObjType::RegionObjType
//
// Constructor
//
RegionObjType::RegionObjType(const char *name, FScope *fScope) : GameObjType(name, fScope)
{
}


//
// RegionObjType::PostLoad
//
// Called after all types are loaded
//
void RegionObjType::PostLoad()
{
  // Call parent scope first
  GameObjType::PostLoad();
}


//
// RegionObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* RegionObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new RegionObj(this, id));
}


///////////////////////////////////////////////////////////////////////////////
//
// Class RegionObj - Instance class for above type
//


//
// Static data
//
NList<RegionObj> RegionObj::allRegions(&RegionObj::node);


//
// RegionObj::FindRegion
//
// Find a region by name
//
RegionObj* RegionObj::FindRegion(const char *regionName)
{
  // Use the crc of the name
  U32 crc = Crc::CalcStr(regionName);

  // Check each existing tag
  for (NList<RegionObj>::Iterator i(&allRegions); *i; i++)
  {
    // Is this the one we're after
    if ((*i)->name.crc == crc)
    {
      return (*i);
    }
  }

  return (NULL);
}


//
// RegionObj::CreateRegion
//
// Create a new blank region
//
RegionObj* RegionObj::CreateRegion(const char *regionName, const Area<F32> &a)
{
  // Find the type 
  RegionObjType *type = GameObjCtrl::FindType<RegionObjType>("Region");

  // Were we successful
  if (type)
  {
    // If this region already exists, reuse it
    RegionObj *obj = FindRegion(regionName);
    if (!obj)
    {
      // Create a new region object (safe cast)
      obj = (RegionObj *) type->NewInstance(0);
  
      // Set the name of the region
      obj->name = regionName;
    }

    // Set the area of the region
    obj->SetArea(a);

    return (obj);
  }

  return (NULL);
}


//
// RegionObj::CreateRegion
//
// Create a new blank region
//
RegionObj* RegionObj::CreateRegion(const char *regionName, const Point<F32> &p)
{
  // Find the type 
  RegionObjType *type = GameObjCtrl::FindType<RegionObjType>("Region");

  // Were we successful
  if (type)
  {
    // If this region already exists, reuse it
    RegionObj *obj = FindRegion(regionName);
    if (!obj)
    {
      // Create a new region object (safe cast)
      obj = (RegionObj *) type->NewInstance(0);
  
      // Set the name of the region
      obj->name = regionName;
    }

    // Set the area of the region
    obj->SetArea(Area<F32>(p, p));

    return (obj);
  }

  return (NULL);
}


//
// RegionObj::RegionObj
//
// Constructor
//
RegionObj::RegionObj(RegionObjType *objType, U32 id) : 
  GameObj(objType, id)
{
  // Set default region name
  name = "No Name";

  // Add to the region list
  allRegions.Append(this);
}


//
// RegionObj::RegionObj
//
// Destructor
//
RegionObj::~RegionObj()
{
  // Remove from the region list
  allRegions.Unlink(this);

  // Unlink the clusters
  clusters.UnlinkAll();
}


//
// RegionObj::PreDelete
//
// Pre deletion cleanup
//
void RegionObj::PreDelete()
{

  // Call parent scope last
  GameObj::PreDelete();
}

  
//
// RegionObj::LoadState
//
// Load a state configuration scope
//
void RegionObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  GameObj::LoadState(fScope);

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Get region name
  name = StdLoad::TypeString(fScope);

  // Get the area
  StdLoad::TypeArea(fScope, "Area", area);

  // Call the set area routine to update the cluster list and the mid point of the region
  SetArea(area);
}


//
// RegionObj::SaveState
//
// Save a state configuration scope
//
void RegionObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  GameObj::SaveState(fScope);

  // Save config scope with region name
  fScope = StdSave::TypeString(fScope, SCOPE_CONFIG, name.str);

  // Save the area
  StdSave::TypeArea<F32>(fScope, "Area", area);
}


//
// RegionObj::PostLoad
//
// Called after all objects are loaded
//
void RegionObj::PostLoad()
{
  // Call parent scope first
  GameObj::PostLoad();
}


//
// RegionObj::SetArea
//
void RegionObj::SetArea(const Area<F32> &a)
{
  area = a;

  // Sort from smallest->largest values
  area.Sort();

  // Clip the area given to the extents of the map and log a warning if its out of bounds
  if (!WorldCtrl::MetreOnMapPoint(area.p0))
  {
    LOG_WARN(("Point 1 is off the map ... clamping"))
    WorldCtrl::ClampMetreMapPoint(area.p0);
  }
  if (!WorldCtrl::MetreOnMapPoint(area.p1))
  {
    LOG_WARN(("Point 2 is off the map ... clamping"))
    WorldCtrl::ClampMetreMapPoint(area.p1);
  }

  // Calculate the mid point of the region
  midpoint.x = (area.p0.x + area.p1.x) * 0.5f;
  midpoint.y = (area.p0.y + area.p1.y) * 0.5f;

  // Build the list of clusters
  WorldCtrl::BuildClusterList(clusters, a);
}


//
// RegionObj::CheckTag
//
// Are the tagged objects inside an area of this region
//
Bool RegionObj::CheckTag(Team *canBeSeenBy, TagObj *tag, U32 amount, RelationalOperator<U32> &oper)
{
  ASSERT(tag)
  U32 count = 0;

  // Count the number of objects in the tag which are inside the region
  for (MapObjList::Iterator o(&tag->list); *o; o++)
  {
    if ((*o)->Alive())
    {
      MapObj *obj = **o;

      // Test to see if the location of the object
      // is inside an area of this region
      if (CheckPoint(Point<F32>(obj->WorldMatrix().posit.x, obj->WorldMatrix().posit.z)))
      {
        // Can this team see the cell the unit is in ?
        if (!canBeSeenBy || obj->GetVisible(canBeSeenBy))
        {
          // If found increment the count found in the region
          count++;
        }
      }
    }
  }

  // Perform the test on the tag
  return (oper(count, amount));
}


//
// RegionObj::CheckTeam
//
// Are objects from the team inside an area of this region
//
Bool RegionObj::CheckTeam(Team *team, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper)
{
  ASSERT(team)
  U32 count = 0;

  // Iterate through the objects in the region and test to see
  // if "amount" of them belong to the team of interest
  
  // To do this iterate through the map clusters associated with
  // this region.  Check it's team to see if it's the one we are
  // interested in and then test to see if its actually in the 
  // region.
  for (List<MapCluster>::Iterator m(&clusters); *m; m++)
  {
    for (NList<UnitObj>::Iterator o(&(*m)->unitList); *o; o++)
    {
      UnitObj *unit = *o;

      if ((unit->GetActiveTeam() == team) && CheckPoint(Point<F32>(unit->WorldMatrix().posit.x, unit->WorldMatrix().posit.z)))
      {
        if (!canBeSeenBy || unit->TestCanSee(canBeSeenBy->GetId()))
        {
          // If found increment the count found in the region
          count++;
        }
      }
    }
  }

  // Perform the test
  return (oper(count, amount));
}


//
// RegionObj::CheckTeam
//
// Are objects from the team inside an area of this region
//
Bool RegionObj::CheckTeam(Team *team, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper, MapObjType *type)
{
  ASSERT(team)
  ASSERT(type)
  U32 count = 0;

  // Iterate through the objects in the region and test to see
  // if "amount" of them belong to the team of interest
  
  // To do this iterate through the map clusters associated with
  // this region.  Check it's team to see if it's the one we are
  // interested in and then test to see if its actually in the 
  // region.
  for (List<MapCluster>::Iterator m(&clusters); *m; m++)
  {
    for (NList<UnitObj>::Iterator o(&(*m)->unitList); *o; o++)
    {
      UnitObj *unit = *o;

      if (
        (unit->GetActiveTeam() == team) && 
        (unit->MapType()->Id() == type->Id()) && 
        CheckPoint(Point<F32>(unit->WorldMatrix().posit.x, unit->WorldMatrix().posit.z)))
      {
        if (!canBeSeenBy || unit->TestCanSee(canBeSeenBy->GetId()))
        {
          // If found increment the count found in the region
          count++;
        }
      }
    }
  }

  // Perform the test
  return (oper(count, amount));
}


//
// RegionObj::CheckTeam
//
// Are objects from the team inside an area of this region
//
Bool RegionObj::CheckTeam(Team *team, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper, U32 property)
{
  ASSERT(team)
  ASSERT(type)
  U32 count = 0;

  // Iterate through the objects in the region and test to see
  // if "amount" of them belong to the team of interest
  
  // To do this iterate through the map clusters associated with
  // this region.  Check it's team to see if it's the one we are
  // interested in and then test to see if its actually in the 
  // region.
  for (List<MapCluster>::Iterator m(&clusters); *m; m++)
  {
    for (NList<UnitObj>::Iterator o(&(*m)->unitList); *o; o++)
    {
      UnitObj *unit = *o;

      if (
        (unit->GetActiveTeam() == team) && 
        (unit->MapType()->HasProperty(property)) && 
        CheckPoint(Point<F32>(unit->WorldMatrix().posit.x, unit->WorldMatrix().posit.z)))
      {
        if (!canBeSeenBy || unit->TestCanSee(canBeSeenBy->GetId()))
        {
          // If found increment the count found in the region
          count++;
        }
      }
    }
  }

  // Perform the test
  return (oper(count, amount));
}


//
// RegionObj::CheckTeams
//
// Are objects from the team inside an area of this region
//
Bool RegionObj::CheckTeams(const List<Team> &teams, Bool combine, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper)
{
  if (combine)
  {
    U32 count = 0;

    // Iterate through the objects in the region and test to see
    // if "amount" of them belong to the team of interest
  
    // To do this iterate through the map clusters associated with
    // this region.  Check it's team to see if it's one of the ones
    // we are interested in and then test to see if its actually in
    // the region.
    for (List<MapCluster>::Iterator m(&clusters); *m; m++)
    {
      for (NList<UnitObj>::Iterator o(&(*m)->unitList); *o; o++)
      {
        UnitObj *unit = *o;

        for (List<Team>::Iterator t(&teams); *t; t++)
        {
          if ((*t) == unit->GetActiveTeam())
          {
            if (CheckPoint(Point<F32>(unit->WorldMatrix().posit.x, unit->WorldMatrix().posit.z)))
            {
              if (!canBeSeenBy || unit->TestCanSee(canBeSeenBy->GetId()))
              {
                // If found increment the count found in the region
                count++;
              }
            }
            break;
          }
        }
      }
    }

    // Perform the test
    return (oper(count, amount));
  }
  else
  {
    // Iterate the teams supplied then check each one
    for (List<Team>::Iterator t(&teams); *t; t++)
    {
      if (CheckTeam(*t, canBeSeenBy, amount, oper))
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }
}


//
// RegionObj::CheckTeams
//
// Are objects from the team inside an area of this region
//
Bool RegionObj::CheckTeams(const List<Team> &teams, Bool combine, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper, MapObjType *type)
{
  if (combine)
  {
    U32 count = 0;

    // Iterate through the objects in the region and test to see
    // if "amount" of them belong to the team of interest
  
    // To do this iterate through the map clusters associated with
    // this region.  Check it's team to see if it's one of the ones
    // we are interested in and then test to see if its actually in
    // the region.
    for (List<MapCluster>::Iterator m(&clusters); *m; m++)
    {
      for (NList<UnitObj>::Iterator o(&(*m)->unitList); *o; o++)
      {
        UnitObj *unit = *o;

        if (unit->MapType()->Id() == type->Id())
        {
          for (List<Team>::Iterator t(&teams); *t; t++)
          {
            if ((*t) == unit->GetActiveTeam())
            {
              if (CheckPoint(Point<F32>(unit->WorldMatrix().posit.x, unit->WorldMatrix().posit.z)))
              {
                if (!canBeSeenBy || unit->TestCanSee(canBeSeenBy->GetId()))
                {
                  // If found increment the count found in the region
                  count++;
                }
              }
              break;
            }
          }
        }
      }
    }

    // Perform the test
    return (oper(count, amount));
  }
  else
  {
    // Iterate the teams supplied then check each one
    for (List<Team>::Iterator t(&teams); *t; t++)
    {
      if (CheckTeam(*t, canBeSeenBy, amount, oper, type))
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }
}


//
// RegionObj::CheckTeams
//
// Are objects from the team inside an area of this region
//
Bool RegionObj::CheckTeams(const List<Team> &teams, Bool combine, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper, U32 property)
{
  if (combine)
  {
    U32 count = 0;

    // Iterate through the objects in the region and test to see
    // if "amount" of them belong to the team of interest
  
    // To do this iterate through the map clusters associated with
    // this region.  Check it's team to see if it's one of the ones
    // we are interested in and then test to see if its actually in
    // the region.
    for (List<MapCluster>::Iterator m(&clusters); *m; m++)
    {
      for (NList<UnitObj>::Iterator o(&(*m)->unitList); *o; o++)
      {
        UnitObj *unit = *o;

        if (unit->MapType()->HasProperty(property))
        {
          for (List<Team>::Iterator t(&teams); *t; t++)
          {
            if ((*t) == unit->GetActiveTeam())
            {
              if (CheckPoint(Point<F32>(unit->WorldMatrix().posit.x, unit->WorldMatrix().posit.z)))
              {
                if (!canBeSeenBy || unit->TestCanSee(canBeSeenBy->GetId()))
                {
                  // If found increment the count found in the region
                  count++;
                }
              }
              break;
            }
          }
        }
      }
    }

    // Perform the test
    return (oper(count, amount));
  }
  else
  {
    // Iterate the teams supplied then check each one
    for (List<Team>::Iterator t(&teams); *t; t++)
    {
      if (CheckTeam(*t, canBeSeenBy, amount, oper, property))
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }
}


//
// RegionObj::CheckThreat
//
// How much threat is there in this region from the given team to a particular ac
//
Bool RegionObj::CheckThreat(Team *team, U32 ac, U32 amount, RelationalOperator<U32> &oper)
{
  ASSERT(team)
  U32 count = 0;

  for (List<MapCluster>::Iterator m(&clusters); *m; m++)
  {
    count += (*m)->ai.GetThreat(team->GetId(), ac);
  }

  // Perform the test
  return (oper(count, amount));
}


//
// RegionObj::CheckTotalThreat
//
// How much threat is there in this region from the given team
//
Bool RegionObj::CheckTotalThreat(Team *team, U32 amount, RelationalOperator<U32> &oper)
{
  ASSERT(team)
  U32 count = 0;

  for (List<MapCluster>::Iterator m(&clusters); *m; m++)
  {
    count += (*m)->ai.GetTotalThreat(team->GetId());
  }

  // Perform the test
  return (oper(count, amount));
}


//
// RegionObj::CheckThreats
//
// How much threat is there in this region from the given teams to a particular ac
//
Bool RegionObj::CheckThreats(const List<Team> &teams, Bool combine, U32 ac, U32 amount, RelationalOperator<U32> &oper)
{
  if (combine)
  {
    U32 count = 0;

    for (List<MapCluster>::Iterator m(&clusters); *m; m++)
    {
      for (List<Team>::Iterator t(&teams); *t; t++)
      {
        count += (*m)->ai.GetThreat((*t)->GetId(), ac);
      }
    }

    // Perform the test
    return (oper(count, amount));
  }
  else
  {
    // Test each team individually
    for (List<Team>::Iterator t(&teams); *t; t++)
    {
      if (CheckThreat(*t, ac, amount, oper))
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }
}


//
// RegionObj::CheckTotalThreats
//
// How much threat is there in this region from the given teams
//
Bool RegionObj::CheckTotalThreats(const List<Team> &teams, Bool combine, U32 amount, RelationalOperator<U32> &oper)
{
  if (combine)
  {
    U32 count = 0;

    for (List<MapCluster>::Iterator m(&clusters); *m; m++)
    {
      for (List<Team>::Iterator t(&teams); *t; t++)
      {
        count += (*m)->ai.GetTotalThreat((*t)->GetId());
      }
    }

    // Perform the test
    return (oper(count, amount));
  }
  else
  {
    // Test each team individually
    for (List<Team>::Iterator t(&teams); *t; t++)
    {
      if (CheckTotalThreat((*t), amount, oper))
      {
        return (TRUE);
      }
    }
    return (FALSE);
  }
}
