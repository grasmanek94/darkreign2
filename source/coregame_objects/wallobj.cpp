///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 06-JAN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "wallobj.h"
#include "promote.h"
#include "claim.h"
#include "terraindata.h"
#include "team.h"
#include "unitobjiter.h"
#include "render.h"
#include "resolver.h"
#include "taskctrl.h"
#include "tasks_wallidle.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG    "WallObj"

// Minimum wall range
#define MINIMUM_RANGE   2



///////////////////////////////////////////////////////////////////////////////
//
// Class WallObjType - De La Uber Wall
//

// Static data
NList<WallObjType> WallObjType::types(&WallObjType::node);


//
// Constructor
//
WallObjType::WallObjType(const char *name, FScope *fScope) : UnitObjType(name, fScope)
{
  // Add to the list of wall types
  types.Append(this);

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load config
  rangeDiagonal = Max<S32>(MINIMUM_RANGE, StdLoad::TypeU32(fScope, "Range", 5));
  rangeStraight = Max<S32>(MINIMUM_RANGE, StdLoad::TypeU32(fScope, "RangeStraight", S32(F32(rangeDiagonal) * 1.4F)));

  F32 offset = StdLoad::TypeF32(fScope, "BeamOffset", 3.0F);
  beamOffsetPrimary = StdLoad::TypeF32(fScope, "BeamOffsetPrimary", offset + 1.0F);
  beamOffsetSecondary = StdLoad::TypeF32(fScope, "BeamOffsetSecondary", offset - 1.0F);

  deviationMin = StdLoad::TypeF32(fScope, "DeviationMin", 1.0F);
  deviationMax = StdLoad::TypeF32(fScope, "DeviationMax", 10.0F);
  surface = MoveTable::SurfaceIndex(StdLoad::TypeString(fScope, "Surface", "Impassable"));
  properties.Load(fScope, "Properties", FALSE);
  originHardPoint = StdLoad::TypeString(fScope, "OriginHardPoint", "HP-WALL");
}


//
// Destructor
//
WallObjType::~WallObjType()
{
  // Remove from the type list
  types.Unlink(this);
}


//
// PostLoad
//
// Called after all types are loaded
//
void WallObjType::PostLoad()
{
  // Call parent scope first
  UnitObjType::PostLoad();

  // Find the origin hardpoint
  GetMeshRoot()->FindIdent(originHardPoint);
}


//
// NewInstance
//
// Create a new map object instance using this type
//
GameObj* WallObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new WallObj(this, id));
}


//
// GetNearbyWalls
//
// Get walls within link distance of the given position
//
void WallObjType::GetNearbyWalls(Team *team, const Vector &p, WallObjList &list, WallObj *filter)
{
  ASSERT(WorldCtrl::MetreOnMap(p.x, p.z))

  // What range should we search in
  F32 range = F32(GetRangeStraight() + 1) * WorldCtrl::CellSize();

  // Generate a unit iterator
  UnitObjIter::Tactical i(NULL, UnitObjIter::FilterData(team, Relation::ALLY, p, range));

  UnitObj *obj;

  // Check each unit
  while ((obj = i.Next()) != NULL)
  {
    // Is this a wall
    if (WallObj *wall = Promote::Object<WallObjType, WallObj>(obj))
    {
      // Should we add it
      if (wall != filter)
      {
        list.Append(wall);
      }
    }
  }  
}


//
// DisplayAvailableLinks
//
// Display the links for all walls within range of the given point
//
void WallObjType::DisplayAvailableLinks(const Vector &cursor)
{
  WallObjList list;

  // Get the list of nearby walls
  GetNearbyWalls(Team::GetDisplayTeam(), cursor, list);

  // Display links for each wall
  for (WallObjList::Iterator i(&list); *i; ++i)
  {
    // Get the wall
    WallObj *wall = **i;

    // Can we link with this type
    if (CanLinkWith(wall->WallType()) && wall->WallType()->CanLinkWith(this))
    {
      wall->DisplayLinks(this, cursor);
    }
  }

  // Clean up the list
  list.Clear();
}


//
// UpdatePowerStatus
//
// Check to see if any walls need to be powered up or down
//
void WallObjType::UpdatePowerStatus(Team *team, Bool ignorePower)
{
  ASSERT(team)

  // For each existing wall type
  for (NList<WallObjType>::Iterator t(&types); *t; t++)
  {
    // Get all the units of this type on the given team
    const NList<UnitObj> *units = team->GetUnitObjects((*t)->GetNameCrc());

    // Tell them all to check their status
    if (units)
    {
      for (NList<UnitObj>::Iterator i(units); *i; i++)
      {
        // Get the wall object
        WallObj *wall = Promote::Object<WallObjType, WallObj>(*i);

        ASSERT(wall)

        // Call the wall
        wall->UpdatePowerStatus(ignorePower);
      }
    }
  }
}


//
// Toggle
//
// If the given unit is a wall, turn it on or off
//
Bool WallObjType::Toggle(UnitObj *unit, Bool on)
{
  ASSERT(unit)

  // Is this unit a wall
  if (WallObj *wall = Promote::Object<WallObjType, WallObj>(unit))
  {
    if (on)
    {
      wall->ActivateIdleLinks(TRUE);
    }
    else
    {
      wall->BreakLinks();
    }

    return (TRUE);
  }

  return (FALSE);
}



///////////////////////////////////////////////////////////////////////////////
//
// Class WallObj - Instance class for above type
//


//
// ExplainResult
//
// Returns a message explaining the given test result
//
const char * WallObj::ExplainResult(TestResult r)
{
  switch (r)
  {
    case TR_SUCCESS: 
      return ("The wall operation was successful");

    case TR_SAMEWALL: 
      return ("Trying to link to the same wall");

    case TR_TYPEFAIL: 
      return ("The wall types are not configured to link to each other");

    case TR_NOTALLY: 
      return ("Connecting walls must be on allied teams");

    case TR_UNAVAILABLE:
      return ("One of the walls is not available for linking");

    case TR_OFFMAP: 
      return ("A cell in the link is off the map");

    case TR_CLAIMED_S:
      return ("A cell in the link is claimed by an immovable or unknown object");
    
    case TR_CLAIMED_M:
      return ("A cell in the link is claimed by a mobile unit");

    case TR_FOOTINDEX:
      return ("A cell in the link has a footprint on it");

    case TR_DEVIATION:
      return ("The deviation of the terrain along the link exceeds requirements");

    case TR_NOTALINK:
      return ("The angle between the walls is not a link");

    case TR_LINKUSEDSRC:
      return ("The link is already used on the source");

    case TR_LINKUSEDDST:
      return ("The link is already used on the destination");

    case TR_DISTANCE:
      return ("The distance between the walls is too great");
  }

  return ("Unknown test result!");
}


//
// FXCallback
//
// Manages the display of links between walls
//
Bool WallObj::FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context)
{
  // Get the wall object
  if (WallObj *wall = Promote::Object<WallObjType, WallObj>(mapObj))
  {
    ASSERT(context)

    // Get the link information
    Link *link = static_cast<Link *>(context);

    // Is the link still active
    if (link->Active())
    {
      Vector src, dst;

      // We're going to update the cbd
      cbd.particle.valid = TRUE;
     
      // Get the beam source position
      wall->GetBeamPosition(src, link->responsible);

      // Get the beam destination position
      (*link)->GetBeamPosition(dst, link->responsible);

      // Set the particle matrix
      cbd.particle.matrix.ClearData();
      cbd.particle.matrix.Set(src);

      // Set the particle length
      cbd.particle.length = dst - src;

      return (FALSE);
    }
  }

  return (TRUE);
}


//
// LinkToDelta
//
// Returns the deltas for the given link
//
const Point<S32> & WallObj::LinkToDelta(S32 link)
{
  ASSERT(link >= 0 && link < MAX_LINKS)

  static const Point<S32> table[MAX_LINKS] =
  {
    Point<S32>( 0, +1), 
    Point<S32>(+1, +1), 
    Point<S32>(+1,  0), 
    Point<S32>(+1, -1), 
    Point<S32>( 0, -1), 
    Point<S32>(-1, -1), 
    Point<S32>(-1,  0), 
    Point<S32>(-1, +1)
  };

  return (table[link]);
}


//
// DeltaToLink
//
// Returns the link index for the given delta
//
S32 WallObj::DeltaToLink(const Point<S32> &delta)
{
  ASSERT(delta.x || delta.z)
  ASSERT(delta.x >= -1 && delta.x <= 1 && delta.z >= -1 && delta.z <= 1)

  // Link indexes for each possible [dx+1][dz+1] (-1 is invalid)
  static S32 table[3][3] =
  {
     5,  6,  7,
     4, -1,  0,
     3,  2,  1
  };

  return (table[delta.x + 1][delta.z + 1]);
}


//
// LinkOpposite
//
// Returns the link that is opposite to the given one
//
S32 WallObj::LinkOpposite(S32 link)
{
  return ((link + (MAX_LINKS / 2)) & (MAX_LINKS - 1));
}


//
// MaxLinkRange
//
// Returns the range for the given link
//
S32 WallObj::MaxLinkRange(S32 link)
{
  return 
  (
    (link & 1) ? WallType()->GetRangeDiagonal() : WallType()->GetRangeStraight()
  );
}


//
// GetLink
//
// Returns the given link data
//
WallObj::Link & WallObj::GetLink(S32 link)
{
  ASSERT(link >= 0 && link < MAX_LINKS)

  return (links[link]);
}


//
// LinkAvailable
//
// Is the given link available
//
Bool WallObj::LinkAvailable(S32 link)
{
  return (!GetLink(link).Active());
}


//
// AvailableCell
//
// Is the given cell available for construction
//
WallObj::TestResult WallObj::AvailableCell(const Point<S32> &p)
{
  // Is the cell on the map
  if (WorldCtrl::CellOnMap(p.x, p.z))
  {
    // Is the cell available to claim
    if (Claim::ProbeCell(p.x, p.z))
    {
      // Get the map cell at this location
      TerrainData::Cell &cell = TerrainData::GetCell(p.x, p.z);

      // Can not build a wall over a footprint because of surface type changes
      return (FootPrint::AvailableInstanceIndex(cell.footIndex) ? TR_SUCCESS : TR_FOOTINDEX);
    }
    else
    {
      Point<S32> g;

      // Convert cell to grain
      WorldCtrl::CellToFirstGrain(p.x, p.z, g.x, g.z);

      // Iterate each grain in the cell
      for (S32 z = g.z; z <= g.z + 1; ++z)
      {
        for (S32 x = g.x; x <= g.x + 1; ++x)
        {
          // Has something claimed this grain
          if (!Claim::Probe(x, z, x, z, Claim::LAYER_LOWER))
          {
            // Try and find the owner
            if (UnitObj *unit = Claim::GetOwner(x, z, Claim::LAYER_LOWER))
            {
              // Can not ignore immovable units
              if (!unit->CanEverMove())
              {
                // An immovable owner
                return (TR_CLAIMED_S);
              }
            }
            else
            {
              // An unknown owner
              return (TR_CLAIMED_S);
            }
          }
        }
      }

      // Cell is claimed by another unit
      return (TR_CLAIMED_M);
    }
  }
  else
  {
    // Cell is off the map
    return (TR_OFFMAP);
  }
}


//
// Height
//
// Returns the height at the given cell
//
F32 WallObj::Height(const Point<S32> &p)
{
  ASSERT(WorldCtrl::CellOnMap(p.x, p.z))

  // Just use find floor for now
  return
  (
    TerrainData::FindFloorWithWater
    (
      WorldCtrl::CellToMetresX(p.x), WorldCtrl::CellToMetresZ(p.z)
    )
  );
}


//
// CheckDeviation
//
// Check the height deviations for the given link
//
Bool WallObj::CheckDeviation(const Point<S32> &src, S32 link, S32 distance)
{
  ASSERT(WorldCtrl::CellOnMap(src.x, src.z))
  ASSERT(link >= 0 && link < MAX_LINKS)
  ASSERT(distance > 0)

  Point<S32> delta(LinkToDelta(link));
  Point<S32> dst(src + (delta * distance));

  // Is the destination on the map
  if (WorldCtrl::CellOnMap(dst.x, dst.z))
  {
    LinkIterator iterator(src, link, distance);
    Point<S32> p;

    // Get the source height
    F32 srcHeight = Height(src) + WallType()->GetBeamOffset();

    // Get the difference between source and destination heights
    F32 inc = ((Height(dst) + WallType()->GetBeamOffset()) - srcHeight) / F32(distance);

    // Step over each point
    for (S32 count = 1; iterator.Next(p); count++)
    {
      // Is this an invalid deviation
      if (!WallType()->ValidDeviation((srcHeight + (inc * F32(count))) - Height(p)))
      {
        return (FALSE);
      }
    }

    // Acceptable link position
    return (TRUE);
  }

  // Link not allowed
  return (FALSE);
}


//
// TestLinkLine
//
// Test each cell in the given line
//
WallObj::TestResult WallObj::TestLinkLine(const Point<S32> &src, S32 link, S32 distance)
{
  LinkIterator iterator(src, link, distance);
  Point<S32> p;

  // Check each cell in the link
  while (iterator.Next(p))
  {
    // Does this cell block our wall
    TestResult r = AvailableCell(p);

    if (r != TR_SUCCESS)
    {
      return (r);
    }
  }

  // Check the deviation
  if (!CheckDeviation(src, link, distance))
  {
    return (TR_DEVIATION);
  }

  // No problems found
  return (TR_SUCCESS);
}



//
// BreakLink
//
// Break the given active link
//
void WallObj::BreakLink(S32 link, Bool initial, Bool deactivate)
{
  // Get the link data
  Link &l = GetLink(link);

  ASSERT(l.Active())

  // Is this wall responsible for the link
  if (l.responsible)
  {
    Point<S32> p(GetOrigin());
    LinkIterator iterator(p, link, l.distance);

    // Release each cell
    while (iterator.Next(p))
    {
      // Change the surface back to the original
      TerrainData::RestoreSurfaceType(p.x, p.z);
    }

    // Release all the claim blocks
    claimInfo.Release(link + 1);
  }

  // Tell the other wall to break its link
  if (initial)
  {
    l->BreakLink(LinkOpposite(link), FALSE, deactivate);
  }

  // If requested deactivation
  if (deactivate)
  {
    l.deactivated = TRUE;
  }
  else
  {
    l.Reset();
  }
}


//
// SufficientPower
//
// Is there sufficient power to maintain this wall
//
Bool WallObj::SufficientPower()
{
  return (GetEfficiency() >= 0.5F);
}


//
// GetBeamPosition
//
// Get the position of the given beam
//
void WallObj::GetBeamPosition(Vector &pos, Bool primary)
{
  // Does this wall have a specific origin point
  if (WallType()->GetOriginHardPoint().Valid())
  {
    // Get the position of the hardpoint
    pos = Position(WallType()->GetOriginHardPoint());
  }
  else
  {
    // Use the origin
    pos = Position();
  }

  // Add in the beam offset
  pos.y += WallType()->GetBeamOffset(primary);
}


//
// Constructor
//
WallObj::WallObj(WallObjType *objType, U32 id) : UnitObj(objType, id), claimInfo(NULL)
{
}


//
// Destructor
//
WallObj::~WallObj()
{
}


//
// PreDelete
//
// Pre deletion cleanup
//
void WallObj::PreDelete()
{
  // Ensure all existing links are broken
  BreakLinks();

  // Call parent scope last
  UnitObj::PreDelete();
}


//
// LoadState
//
// Load a state configuration scope
//
void WallObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  UnitObj::LoadState(fScope);

  // Get the config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Load the links
  for (S32 i = 0; i < MAX_LINKS; i++)
  {
    char name[64];
    Utils::Sprintf(name, 64, "Link%d", i);
    StdLoad::TypeReaper(fScope, name, GetLink(i));
  }
}


//
// SaveState
//
// Save a state configuration scope
//
void WallObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  UnitObj::SaveState(fScope);

  // Create specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  // Save the links
  for (S32 i = 0; i < MAX_LINKS; i++)
  {
    // Get the link
    Link &l = GetLink(i);

    // Only save active links we're responsible for
    if (l.Alive() && l.responsible)
    {
      char name[64];
      Utils::Sprintf(name, 64, "Link%d", i);
      StdSave::TypeReaper(fScope, name, GetLink(i));
    }
  }
}


//
// PostLoad
//
// Called after all objects are loaded
//
void WallObj::PostLoad()
{
  // Call parent scope first
  UnitObj::PostLoad();

  // Resolve the links
  for (S32 i = 0; i < MAX_LINKS; i++)
  {
    // Get this link
    Link &l = GetLink(i);

    // Attempt to resolve
    Resolver::Object<WallObj, WallObjType>(l);

    // Setup as deactivated (will be activated at power-on)
    if (l.Alive())
    {
      l.deactivated = TRUE;
      l.responsible = TRUE;
    }
  }
}


//
// GetOrigin
//
// Get the origin cell for this wall
//
Point<S32> WallObj::GetOrigin()
{
  // Does this wall have a specific origin point
  if (WallType()->GetOriginHardPoint().Valid())
  {
    // Get the position of the hardpoint
    Vector v(Position(WallType()->GetOriginHardPoint()));
    Point<F32> pos(v.x, v.z);

    // Clamp it onto the map
    WorldCtrl::ClampMetreMapPoint(pos);

    // Return the cell position
    return (Point<S32>(WorldCtrl::MetresToCellX(pos.x), WorldCtrl::MetresToCellZ(pos.z)));
  }
  else
  {
    // Return centre of object
    return (Point<S32>(GetCellX(), GetCellZ()));
  }
}


//
// FindLink
//
// Returns TRUE if there is a link to the given wall
//
Bool WallObj::FindLink(WallObj *target, S32 *linkPtr)
{
  for (S32 i = 0; i < MAX_LINKS; i++)
  {
    // Get this link
    Link &l = GetLink(i);

    // Is it linked to the target
    if (l.Active() && l == target)
    {
      // Setup the link
      if (linkPtr)
      {
        *linkPtr = i;
      }

      // A link was found
      return (TRUE);
    }
  }

  // No link found
  return (FALSE);
}


//
// AvailableForLinking
//
// Is the wall available for linking
//
Bool WallObj::AvailableForLinking(Bool checkPower)
{
  // Is there enough power
  if (checkPower && !SufficientPower())
  {
    return (FALSE);
  }

  // Ensure wall is zipped
  if (!GetFootInstance())
  {
    return (FALSE);
  }

  // If on a team, ensure it's active
  if (GetTeam() && !IsActivelyOnTeam())
  {
    return (FALSE);
  }
  
  return (TRUE);
}


//
// TestLink
//
// Test if we can link to the given wall
//
WallObj::TestResult WallObj::TestLink(WallObj *target, LinkIterator *iteratorPtr, Bool checkPower)
{
  ASSERT(target)

  // Fail if it's the same wall
  if (target == this)
  {
    return (TR_SAMEWALL);
  }

  // Fail if the types can not link
  if (!WallType()->CanLinkWith(target->WallType()))
  {
    return (TR_TYPEFAIL);
  }

  // If either wall has a team
  if (GetTeam() || target->GetTeam())
  {
    // They must be allied
    if (!Team::TestRelation(GetTeam(), target->GetTeam(), Relation::ALLY))
    {
      return (TR_NOTALLY);
    }
  }

  // Fail if either wall is not available for linking
  if (!AvailableForLinking(checkPower) || !target->AvailableForLinking(checkPower))
  {
    return (TR_UNAVAILABLE);
  }

  // Get our cell location
  Point<S32> src(GetOrigin());

  // Get the target's cell location
  Point<S32> dst(target->GetOrigin());

  // Calculate the raw deltas
  Point<S32> delta(dst - src);

  // And the absolute values
  Point<S32> deltaAbs(abs(delta.x), abs(delta.z));

  // Is this a valid link direction
  if ((delta.x || delta.z) && (!delta.x || !delta.z || (deltaAbs.x == deltaAbs.z)))
  {
    // What is the link distance
    S32 distance = deltaAbs.x ? deltaAbs.x : deltaAbs.z;

    ASSERT(distance > 0)

    // Convert to minimal deltas
    delta.x = Clamp<S32>(-1, delta.x, 1);
    delta.z = Clamp<S32>(-1, delta.z, 1);

    // Get the link index
    S32 link = DeltaToLink(delta);

    // Is the link on this object available
    if (LinkAvailable(link))
    {
      // Is target in range
      if (distance <= MaxLinkRange(link))
      {
        // Is the target link available
        if (target->LinkAvailable(LinkOpposite(link)))
        {
          // Test the cells
          TestResult r = TestLinkLine(src, link, distance);

          // Can we link
          if (r == TR_SUCCESS)
          {
            // Setup the iterator
            if (iteratorPtr)
            {
              iteratorPtr->Set(src, link, distance);
            }
          }

          // Return the result
          return (r);
        }
        else
        {
          // The link is used on the destination
          return (TR_LINKUSEDDST);
        }
      }
      else
      {
        // Distance is too great
        return (TR_DISTANCE);
      }
    }
    else
    {
      // The link is used on the source
      return (TR_LINKUSEDSRC);
    }
  }
  else
  {
    // This angle is not a link
    return (TR_NOTALINK);
  }
}


//
// FormLink
//
// Attempt to form a link with the given wall
//
WallObj::TestResult WallObj::FormLink(WallObj *target, Bool checkPower)
{
  LinkIterator iterator;

  // Test if we can link to the given target
  TestResult r = TestLink(target, &iterator, checkPower);

  // Were we successful
  if (r == TR_SUCCESS)
  {
    // Get the link index
    S32 link = iterator.link;

    ASSERT(link >= 0 && link < MAX_LINKS)
    ASSERT(LinkAvailable(link))
    ASSERT(target->LinkAvailable(LinkOpposite(link)))

    // Setup the local link
    GetLink(link).Set(target, iterator.distance, TRUE);

    // Setup the remote link
    target->GetLink(LinkOpposite(link)).Set(this, iterator.distance, FALSE);

    // Setup each cell in the wall
    Point<S32> p, g;

    while (iterator.Next(p))
    { 
      // Convert cell to grain
      WorldCtrl::CellToFirstGrain(p.x, p.z, g.x, g.z);

      // Claim the entire cell
      claimInfo.Claim(g.x, g.z, g.x + 1, g.z + 1, link + 1);

      // Get the map cell at this location
      TerrainData::Cell &cell = TerrainData::GetCell(p.x, p.z);

      // Set the surface type
      cell.surface = WallType()->GetSurface();
    }

    // "Wall::Link"
    StartGenericFX(0xD063DAD7, FXCallBack, FALSE, NULL, &GetLink(link));

    // "Wall::Link"
    target->StartGenericFX
    (
      0xD063DAD7, FXCallBack, FALSE, NULL, &target->GetLink(LinkOpposite(link))
    );
    
    return (TR_SUCCESS);
  }
  else
  {
    return (r);
  }
}


//
// ToggleLink
//
// Attempt to toggle the link to the target
//
WallObj::TestResult WallObj::ToggleLink(WallObj *target, Bool checkPower, Bool deactivate)
{
  S32 link;

  // Are we linked to the given target
  if (FindLink(target, &link))
  {
    // Break the link
    BreakLink(link, TRUE, deactivate);

    // Always succeeds
    return (TR_SUCCESS);
  }
  else
  {
    // Attempt to form a link
    return (FormLink(target, checkPower));
  }
}


//
// SetupEvacuator
//
// Add each cell in the wall link to the given evacuator
//
Bool WallObj::SetupEvacuator(WallObj *target, UnitEvacuate &evacuate)
{
  ASSERT(target)

  // Get our cell location
  Point<S32> src(GetOrigin());

  // Get the target's cell location
  Point<S32> dst(target->GetOrigin());

  // Calculate the raw deltas
  Point<S32> delta(dst - src);

  // And the absolute values
  Point<S32> deltaAbs(abs(delta.x), abs(delta.z));

  // Is this a valid link direction
  if ((delta.x || delta.z) && (!delta.x || !delta.z || (deltaAbs.x == deltaAbs.z)))
  {
    // What is the link distance
    S32 distance = deltaAbs.x ? deltaAbs.x : deltaAbs.z;

    // Convert to minimal deltas
    delta.x = Clamp<S32>(-1, delta.x, 1);
    delta.z = Clamp<S32>(-1, delta.z, 1);

    // Get the link index
    S32 link = DeltaToLink(delta);

    // Setup the link iterator
    LinkIterator iterator(src, link, distance);
    Point<S32> p;

    // Step over each point
    while (iterator.Next(p))
    {
      evacuate.Register(p, Claim::LAYER_LOWER);
    }

    return (TRUE);
  }

  return (FALSE);
}


//
// ActivateIdleLinks
//
// Activate any deactivated links (if 'all', includes non-responsible)
//
void WallObj::ActivateIdleLinks(Bool all)
{
  for (S32 i = 0; i < MAX_LINKS; i++)
  {
    if (GetLink(i).Deactivated() && (all || GetLink(i).responsible))
    {
      if (FormLink(GetLink(i), FALSE) != TR_SUCCESS)
      {
        GetLink(i).Reset();
      }
    }
  }
}


//
// UpdatePowerStatus
//
// Check to see if the wall needs to activate/deactivate
//
void WallObj::UpdatePowerStatus(Bool ignorePower)
{
  // Is power supply acceptable
  if (ignorePower || SufficientPower())
  {
    // Bring online
    ActivateIdleLinks(FALSE);
  }
  else
  {
    // Take offline
    BreakLinks();
  }
}


//
// BreakLinks
//
// Break all current links
//
void WallObj::BreakLinks(Bool deactivate)
{
  for (S32 i = 0; i < MAX_LINKS; i++)
  {
    // Is it linked to the target
    if (GetLink(i).Active())
    {
      BreakLink(i, TRUE, deactivate);
    }
  }
}


//
// SetupAutoLink
//
// Setup the idle task to link with nearby walls
//
void WallObj::SetupAutoLink()
{
  // Do we have a wall idle task
  if (Tasks::WallIdle *idle = TaskCtrl::PromoteIdle<Tasks::WallIdle>(this))
  {
    WallObjList list;

    // Get the list of nearby walls
    WallType()->GetNearbyWalls(GetTeam(), Position(), list, this);

    // Setup the idle task
    idle->SetLinkTargets(list);

    // Clean up the list
    list.Clear();
  }
}


//
// LinkVisible
//
// Should the given link be displayed for the given deltas
//
Bool WallObj::LinkVisible(S32 link, Point<S32> d)
{
  // Is this link available
  if (LinkAvailable(link))
  {
    // Grab the absolute values
    Point<S32> da(abs(d.x), abs(d.z));

    // Is this point on a link-line
    if ((d.x || d.z) && (!d.x || !d.z || (da.x == da.z)))
    {
      // Is it within the range
      if (Max<S32>(da.x, da.z) <= MaxLinkRange(link))
      {
        // Is the point on this link
        if (Point<S32>(Clamp<S32>(-1, d.x, 1), Clamp<S32>(-1, d.z, 1)) == LinkToDelta(link))
        {
          return (TRUE);
        }
      }
    }
  }

  // Link not visible
  return (FALSE); 
}


//
// DisplayLinks
//
// Display the links on this wall that are visible from the given point
//
void WallObj::DisplayLinks(WallObjType *, const Vector &cursor)
{
  ASSERT(WorldCtrl::MetreOnMap(cursor.x, cursor.z))

  // Get the cell locations
  Point<S32> src(GetOrigin());
  Point<S32> cursorCell(WorldCtrl::MetresToCellX(cursor.x), WorldCtrl::MetresToCellZ(cursor.z));

  // Calculate the raw deltas
  Point<S32> delta(cursorCell - src);

  // Set the line starting location
  Vector start;
  GetBeamPosition(start);

  // Setup display colors
  Color colorY(0.0F, 1.0F, 0.0F, 0.2F);
  Color colorN(1.0F, 0.0F, 0.0F, 0.2F);

  // Iterate the links
  for (S32 i = 0; i < MAX_LINKS; i++)
  {
    // Is this link visible
    if (LinkVisible(i, delta))
    {
      // Check the link to each cell along this link
      for (S32 distance = 1; distance <= MaxLinkRange(i); distance++)
      {
        // Get the destination cell
        Point<S32> dst(src + (LinkToDelta(i) * distance));

        // Is it on the map
        if (WorldCtrl::CellOnMap(dst.x, dst.z))
        {
          // Can we link to this cell
          TestResult r = TestLinkLine(src, i, distance);

          // Render the cell
          Area<S32> rect;
          rect.SetSize(dst.x, dst.z, 1, 1);
          Terrain::RenderCellRect(rect, (r == TR_SUCCESS || r == TR_CLAIMED_M) ? colorY : colorN, TRUE);

          // Draw the line if this is the cursor position
          if ((r == TR_SUCCESS) && (dst == cursorCell))
          {
            // Set the line ending location
            Vector end;
            end.x = WorldCtrl::CellToMetresX(dst.x);
            end.z = WorldCtrl::CellToMetresZ(dst.z);
            end.y = TerrainData::FindFloorWithWater(end.x, end.z) + WallType()->GetBeamOffset();

            // Display the line
            Render::FatLine(start, end, Color(0.0F, 1.0F, 0.0F, 0.8F), 0.5F);
          }
        }
      }
    }
  }
}

