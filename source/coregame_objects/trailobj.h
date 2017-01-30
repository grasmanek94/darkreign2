///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-AUG-1998
//

#ifndef __TRAILOBJ_H
#define __TRAILOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "trailobjdec.h"
#include "gameobj.h"
#include "iclistbox.h"


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
// Class TrailObjType - A labelled list of waypoints
//
class TrailObjType : public GameObjType
{
  PROMOTE_LINK(TrailObjType, GameObjType, 0xAC7AEE06); // "TrailObjType"

public:

  // Constructor
  TrailObjType(const char *name, FScope *fScope);

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class TrailObj - Instance class for above type
//
class TrailObj : public GameObj
{
public:

  //
  // Possible trail modes
  //
  enum Mode
  {
    MODE_ONEWAY,    // To end of path, then stop
    MODE_TWOWAY,    // To end of path, then back along path, repeat
    MODE_LOOPIN,    // To end of path, then directly to start, repeat
  };

  //
  // A single trail waypoint
  //
  struct WayPoint : Point<U32>
  {  
    // List node
    NList<WayPoint>::Node node;

    // Constructor
    WayPoint()
    {
    }

    // Constructor
    WayPoint(U32 x, U32 z) : Point<U32>(x, z)
    {
    }
  };

  //
  // A list of waypoints
  //
  class WayPointList : public NList<WayPoint>
  {
  public:

    // Constructor 
    WayPointList();

    // Append a new point
    void AppendPoint(U32 x, U32 z);

    // Append the given list 
    void AppendList(const WayPointList &list);

    // Clear all points then append the given list
    void Set(const WayPointList &list);

    // Returns TRUE if the given point is on this trail
    Bool Find(U32 x, U32 z, U32 *index = NULL);
  };

  //
  // A trail follower
  //
  class Follower
  {
  protected:

    // The trail being followed
    TrailObjPtr trail;

    // Waypoint iterator
    WayPointList::Iterator iterator;

    // Are we going forwards
    Bool forwards;

    // The loaded iterator index (gh3yn355)
    U32 index;

    // Is the iterator at the terminal point
    Bool AtTerminal();

    // Step to the next point
    void Step();

  public:

    // Constructor
    Follower();

    // Set the follower onto a given trail
    void Set(TrailObj *t, U32 index = 0);

    // Get the current position in cells (FALSE if path is finished)
    Bool GetPos(Point<U32> &p);

    // Move to the next position
    Bool Next();

    // Returns the iterator index
    U32 GetIndex();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);
    void PostLoad();

    // Returns the current trail, or NULL
    TrailObj * GetTrail()
    {
      return (trail.GetPointer());
    }
  };

protected:

  // List node
  NList<TrailObj>::Node node;

  // Name of this region
  GameIdent name;

  // The trail mode
  Mode mode;

  // The team this trail belongs to, or NULL
  Team *team;

  // The list of waypoints
  WayPointList list;

  // List of all trails
  static NList<TrailObj> trails;

public:

  // Find a trail by crc
  static TrailObj * Find(U32 crc, Team *team);

  // Find a trail by name
  static TrailObj * Find(const char *name, Team *team);

  // Find a trail that occupies the given cell
  static TrailObj * Find(U32 x, U32 z, Team *team, U32 *index = NULL);
  
  // Create a new trail (NULL if unable to create)
  static TrailObj * Create(Team *team = NULL, const char *name = NULL, Mode mode = MODE_TWOWAY);

  // Delete all trails belonging to the given team
  static void Delete(Team *team);

  // Converts a string mode name to the enumeration value
  static Mode StringToMode(const char *str);

  // Converts an enumerated mode to the string value
  static const char * ModeToString(Mode mode);

  // Returns the list of trails
  static const NList<TrailObj> & GetTrails()
  {
    return (trails);
  }

public:

  // Constructor and destructor
  TrailObj(TrailObjType *objType, U32 id);
  ~TrailObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

public:

  // Returns the name of this trail
  const char * GetName()
  {
    return (name.str);
  }

  // Returns the current trail mode
  Mode GetMode()
  {
    return (mode);
  }

  // Sets a new trail mode
  void SetMode(Mode m)
  {
    mode = m;
  }

  // Returns the team this trail belongs to, or NULL
  Team * GetTeam()
  {
    return (team);
  }

  // Returns the current waypoint list
  WayPointList & GetList()
  {
    return (list);
  }

  // Is there multiple points on the list
  Bool MultiPoint()
  {
    return (list.GetCount() > 1);
  }
};

#endif  