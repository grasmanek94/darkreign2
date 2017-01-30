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
#include "trailobj.h"
#include "gameobjctrl.h"
#include "team.h"
#include "resolver.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "TrailObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class WayPointList - A list of waypoints
//


//
// Constructor
//
TrailObj::WayPointList::WayPointList() : NList<WayPoint>()
{
  SetNodeMember(&WayPoint::node);
}


//
// AppendPoint
//
// Append a new point
//
void TrailObj::WayPointList::AppendPoint(U32 x, U32 z)
{
  Append(new WayPoint(x, z));
}


//
// AppendList
//
// Append the given list 
//
void TrailObj::WayPointList::AppendList(const WayPointList &list)
{
  for (WayPointList::Iterator i(&list); *i; i++)
  {
    AppendPoint((*i)->x, (*i)->z);
  }
}


//
// Set
//
// Clear all points then append the given list
//
void TrailObj::WayPointList::Set(const WayPointList &list)
{
  DisposeAll();
  AppendList(list);
}


//
// Find
//
// Returns TRUE if the given point is on this trail
//
Bool TrailObj::WayPointList::Find(U32 x, U32 z, U32 *index)
{
  for (WayPointList::Iterator i(this); *i; i++)
  {
    if ((*i)->x == x && (*i)->z == z)
    {
      if (index)
      {
        *index = i.GetPos();
      }

      return (TRUE);
    }
  }

  return (FALSE);
}


///////////////////////////////////////////////////////////////////////////////
//
// Class Follower - A trail follower
//


//
// Constructor
//
TrailObj::Follower::Follower() : forwards(TRUE)
{
}


//
// Set
//
// Set the follower onto a given trail
//
void TrailObj::Follower::Set(TrailObj *t, U32 index)
{
  // Setup data
  if (t)
  {
    // Setup the iterator
    iterator.SetList(&t->GetList());

    // Was an index supplied
    if (index)
    {
      // Clamp the index
      if (index >= t->GetList().GetCount())
      {
        index = t->GetList().GetCount() - 1;
      }

      // Move the iterator
      iterator.GoTo(index);
    }

    // Set the direction
    if ((t->GetMode() == MODE_ONEWAY) && (t->GetList().GetCount() - 1 == index))
    {
      // One way trail, and last point was clicked on
      forwards = FALSE;
    }
    else
    {
      // In any other situation, always move forwards
      forwards = TRUE;
    }
  }

  // Save or clear the trail
  trail = t;
}


//
// GetPos
//
// Get the current position in cells (FALSE if path is finished)
//
Bool TrailObj::Follower::GetPos(Point<U32> &p)
{
  // Is there a current point
  if (trail.Alive() && *iterator)
  {
    p.Set((*iterator)->x, (*iterator)->z);
    return (TRUE);
  }

  return (FALSE);
}


//
// AtTerminal
//
// Is the iterator at the terminal point
//
Bool TrailObj::Follower::AtTerminal()
{
  ASSERT(trail.Alive());

  return (forwards ? iterator.IsTail() : iterator.IsHead());
}


//
// Step
//
// Step to the next point
//
void TrailObj::Follower::Step()
{
  ASSERT(trail.Alive());

  forwards ? ++iterator : --iterator;
}


//
// Next
//
// Move to the next position
//
Bool TrailObj::Follower::Next()
{
  if (trail.Alive())
  {
    switch (trail->GetMode())
    {
      case MODE_ONEWAY:
      {
        if (AtTerminal())
        {
          trail = NULL;
          return (FALSE);
        }

        Step();
        return (TRUE);
      }

      case MODE_TWOWAY:
      {
        if (AtTerminal())
        {
          if (trail->MultiPoint())
          {
            forwards = !forwards;
          }
          else
          {
            trail = NULL;
            return (FALSE);
          }
        }

        Step();
        return (TRUE);
      }

      case MODE_LOOPIN:
      {
        if (AtTerminal())
        {
          if (trail->MultiPoint())
          {
            if (forwards)
            {
              iterator.GoToHead();
            }
            else
            {
              iterator.GoToTail();
            }
          }
          else
          {
            trail = NULL;
            return (FALSE);
          }
        }
        else
        {
          Step();
        }

        return (TRUE);
      }
    }
  }

  return (FALSE);
}


//
// GetIndex
//
// Returns the iterator index
//
U32 TrailObj::Follower::GetIndex()
{
  return (trail.Alive() ? iterator.GetPos() : 0);
}


//
// SaveState
//
void TrailObj::Follower::SaveState(FScope *scope)
{
  StdSave::TypeReaper(scope, "Trail", trail);
  StdSave::TypeU32(scope, "Forwards", forwards);
  StdSave::TypeU32(scope, "Index", GetIndex());
}


//
// LoadState
//
void TrailObj::Follower::LoadState(FScope *scope)
{
  FScope *sScope;
  
  while ((sScope = scope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0x82698073: // "Trail"
        StdLoad::TypeReaper(sScope, trail);
        break;

      case 0x557251DB: // "Forwards"
        forwards = StdLoad::TypeU32(sScope);
        break;

      case 0xCD634517: // "Index"
        index = StdLoad::TypeU32(sScope);
        break;
    }
  }
}


//
// PostLoad
//
void TrailObj::Follower::PostLoad()
{
  Resolver::Object<TrailObj, TrailObjType>(trail);

  if (trail.Alive())
  {
    // Remember loaded forwards value
    U32 f = forwards;

    // Set the new trail
    Set(trail, index);

    // Restore forwards value
    forwards = f;
  }
}


///////////////////////////////////////////////////////////////////////////////
//
// Class TrailObjType - A labelled list of waypoints
//


//
// Constructor
//
TrailObjType::TrailObjType(const char *name, FScope *fScope) : GameObjType(name, fScope)
{
}


//
// PostLoad
//
// Called after all types are loaded
//
void TrailObjType::PostLoad()
{
  // Call parent scope first
  GameObjType::PostLoad();
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* TrailObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new TrailObj(this, id));
}


///////////////////////////////////////////////////////////////////////////////
//
// Class TrailObj - Instance class for above type
//


//
// Static data
//
NList<TrailObj> TrailObj::trails(&TrailObj::node);


//
// Find
//
// Find a trail by crc
//
TrailObj * TrailObj::Find(U32 crc, Team *team)
{
  TrailObj *winner = NULL;

  for (NList<TrailObj>::Iterator i(&trails); *i; i++)
  {
    // Get the trail
    TrailObj *trail = *i;

    // Does the name match
    if (trail->name.crc == crc)
    {
      // Does this trail match the team (both may be NULL)
      if (trail->team == team)
      {
        return (trail);
      }

      // Only remember this trail if not on a team
      if (team)
      {
        winner = trail;
      }
    }
  }

  return (winner);
}


//
// Find
//
// Find a trail by name
//
TrailObj * TrailObj::Find(const char *name, Team *team)
{
  ASSERT(name)

  return (Find(Crc::CalcStr(name), team));
}


//
// Find
//
// Find a trail that occupies the given cell
//
TrailObj * TrailObj::Find(U32 x, U32 z, Team *team, U32 *index)
{
  for (NList<TrailObj>::Iterator i(&trails); *i; i++)
  {
    if ((*i)->team == team && (*i)->GetList().Find(x, z, index))
    {
      return (*i);
    }
  }

  return (NULL);
}


//
// Create
//
// Create a new trail (NULL if unable to create)
//
TrailObj * TrailObj::Create(Team *team, const char *name, Mode mode)
{
  // If a name is given, make sure not a duplicate
  if (!name || !Find(name, team))
  {
    // Find the type 
    TrailObjType *type = GameObjCtrl::FindType<TrailObjType>("Trail");
   
    if (type)
    {
      // Create a new trail object
      TrailObj *t = (TrailObj *) type->NewInstance(0);

      // Set the name
      if (name)
      {
        t->name = name;
      }

      // Set the team
      t->team = team;

      // And the mode
      t->mode = mode;

      return (t);
    }
  }

  return (NULL);
}


//
// Delete
//
// Delete all trails belonging to the given team
//
void TrailObj::Delete(Team *team)
{
  for (NList<TrailObj>::Iterator i(&trails); *i; i++)
  {
    if ((*i)->team == team)
    {
      (*i)->MarkForDeletion();
    }
  }
}


//
// StringToMode
//
// Converts a string mode name to the enumeration value
//
TrailObj::Mode TrailObj::StringToMode(const char *str)
{
  switch (Crc::CalcStr(str))
  {
    case 0xAC2E91AD: // "OneWay"
      return (TrailObj::MODE_ONEWAY);

    case 0x50BEDEC9: // "Loopin"
      return (TrailObj::MODE_LOOPIN);

    default:
      return (TrailObj::MODE_TWOWAY);
  }
}


//
// ModeToString
//
// Converts an enumerated mode to the string value
//
const char * TrailObj::ModeToString(Mode mode)
{
  switch (mode)
  {
    case MODE_ONEWAY: 
      return ("OneWay");

    case MODE_LOOPIN: 
      return ("Loopin");
    
    default : 
      return ("TwoWay");
  }
}


//
// Constructor
//
TrailObj::TrailObj(TrailObjType *objType, U32 id) : GameObj(objType, id), team(NULL)
{
  // Set default trail name
  Utils::Sprintf(name.str, name.GetSize(), "Trail-%d", Id());

  // Ensure crc is updated
  name.Update();

  // Add to the trail list
  trails.Append(this);
}


//
// Destructor
//
TrailObj::~TrailObj()
{
  list.DisposeAll();

  // Remove from the trail list
  trails.Unlink(this);
}


//
// PreDelete
//
// Pre deletion cleanup
//
void TrailObj::PreDelete()
{

  // Call parent scope last
  GameObj::PreDelete();
}


//
// SaveState
//
// Save a state configuration scope
//
void TrailObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  GameObj::SaveState(fScope);

  // Save config scope with trail name
  fScope = StdSave::TypeString(fScope, SCOPE_CONFIG, name.str);

  // Save the team
  if (team)
  {
    StdSave::TypeString(fScope, "Team", team->GetName());
  }

  // Save the points
  FScope *sScope = fScope->AddFunction("Points");

  for (WayPointList::Iterator i(&list); *i; i++)
  {
    FScope *ssScope = sScope->AddFunction("Add");
    ssScope->AddArgInteger((*i)->x);
    ssScope->AddArgInteger((*i)->z);
  }

  StdSave::TypeU32(fScope, "Mode", mode);
}

  
//
// LoadState
//
// Load a state configuration scope
//
void TrailObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  GameObj::LoadState(fScope);

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Get trail name
  name = StdLoad::TypeString(fScope); 

  // Get the team
  FScope *sScope = fScope->GetFunction("Team", FALSE);

  if (sScope)
  {
    team = Team::Name2Team(StdLoad::TypeString(sScope));
  }

  // Save the points
  sScope = fScope->GetFunction("Points");

  FScope *ssScope;

  while ((ssScope = sScope->NextFunction()) != NULL)
  {
    switch (ssScope->NameCrc())
    {
      case 0x9F1D54D0: // "Add"
      {
        U32 x = StdLoad::TypeU32(ssScope);
        U32 z = StdLoad::TypeU32(ssScope);
        list.AppendPoint(x, z);
        break;
      }
    }
  }

  mode = Mode(StdLoad::TypeU32(fScope, "Mode", MODE_TWOWAY));
}


//
// PostLoad
//
// Called after all objects are loaded
//
void TrailObj::PostLoad()
{
  // Call parent scope first
  GameObj::PostLoad();

}


