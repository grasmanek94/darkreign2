///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-AUG-1998
//

#ifndef __REGIONOBJ_H
#define __REGIONOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "regionobjdec.h"
#include "gameobj.h"
#include "tagobjdec.h"
#include "worldctrl.h"
#include "relationaloperator.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//


///////////////////////////////////////////////////////////////////////////////
//
// Forward delcarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// Class RegionObjType - A labelled region composed of (disjoint) map areas
//
class RegionObjType : public GameObjType
{
  PROMOTE_LINK(RegionObjType, GameObjType, 0x8EC31934); // "RegionObjType"

public:

  // Constructor
  RegionObjType(const char *name, FScope *fScope);

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class RegionObj - Instance class for above type
//
class RegionObj : public GameObj
{
private:

  // Name of this region
  GameIdent name;

  // List node
  NList<RegionObj>::Node node;

  // Area which this region covers
  Area<F32> area;

  // Midpoint of the region
  Point<F32> midpoint;

  // List of map clusters which the region's area covers
  List<MapCluster> clusters;

public:

  // List of all current regions
  static NList<RegionObj> allRegions;

  // Find a region by name
  static RegionObj* FindRegion(const char *regionName);
  
  // Create a new blank region
  static RegionObj* CreateRegion(const char *regionName, const Area<F32> &a);

  // Create a new blank region
  static RegionObj* CreateRegion(const char *regionName, const Point<F32> &p);

public:

  // Constructor and destructor
  RegionObj(RegionObjType *objType, U32 id);
  ~RegionObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Set the area
  void SetArea(const Area<F32> &a);

  // Are the tagged objects inside an area of this region
  Bool CheckTag(Team *canBeSeenBy, TagObj *tag, U32 amount, RelationalOperator<U32> &oper);

  // Are objects from the team inside an area of this region
  Bool CheckTeam(Team *team, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper);

  // Are objects from the team inside an area of this region
  Bool CheckTeam(Team *team, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper, MapObjType *type);

  // Are objects from the team inside an area of this region
  Bool CheckTeam(Team *team, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper, U32 property);

  // Are objects from the team inside an area of this region
  Bool CheckTeams(const List<Team> &teams, Bool combine, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper);

  // Are objects from the team inside an area of this region
  Bool CheckTeams(const List<Team> &teams, Bool combine, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper, MapObjType *type);

  // Are objects from the team inside an area of this region
  Bool CheckTeams(const List<Team> &teams, Bool combine, Team *canBeSeenBy, U32 amount, RelationalOperator<U32> &oper, U32 property);

  // How much threat is there in this region from the given team to a particular ac
  Bool CheckThreat(Team *team, U32 ac, U32 amount, RelationalOperator<U32> &oper);

  // How much threat is there in this region from the given team
  Bool CheckTotalThreat(Team *team, U32 amount, RelationalOperator<U32> &oper);

  // How much threat is there in this region from the given teams to a particular ac
  Bool CheckThreats(const List<Team> &teams, Bool combine, U32 ac, U32 amount, RelationalOperator<U32> &oper);

  // How much threat is there in this region from the given teams
  Bool CheckTotalThreats(const List<Team> &teams, Bool combine, U32 amount, RelationalOperator<U32> &oper);

public:

  // Returns the name of this tag
  const char * RegionName()
  {
    return (name.str);
  }

  // Is the given point contained within the region
  Bool CheckPoint(const Point<F32> &p)
  {
    return (area.In(p));
  }

  // Gets the area
  const Area<F32> & GetArea()
  {
    return (area);
  }

  // Return the midpoint of the region
  const Point<F32> & GetMidPoint()
  {
    return (midpoint);
  }

};

#endif  