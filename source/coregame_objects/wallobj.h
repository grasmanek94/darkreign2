///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 06-JAN-1999
//

#ifndef __WALLOBJ_H
#define __WALLOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "unitobj.h"
#include "wallobjdec.h"
#include "propertylist.h"
#include "claim.h"
#include "unitevacuate.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//


///////////////////////////////////////////////////////////////////////////////
//
// Class WallObjType - De La Uber Wall
//
class WallObjType : public UnitObjType
{
  PROMOTE_LINK(WallObjType, UnitObjType, 0x2F983F29); // "WallObjType"

protected:

  // List of all wall types
  static NList<WallObjType> types;

  // The type list node
  NList<WallObjType>::Node node;

  // The maximum range of each link
  S32 rangeStraight;
  S32 rangeDiagonal;

  // The beam offsets
  F32 beamOffsetPrimary;
  F32 beamOffsetSecondary;

  // The maximum deviations from the connecting line
  F32 deviationMin;
  F32 deviationMax;

  // The surface to place
  U8 surface;

  // What should this wall link with
  PropertyList<16> properties;

  // Origin cell hardpoint
  NodeIdent originHardPoint;

public:

  // Constructor and destructor
  WallObjType(const char *typeName, FScope *fScope);
  ~WallObjType();

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // Get walls within link distance of the given position
  void GetNearbyWalls(Team *team, const Vector &p, WallObjList &list, WallObj *filter = NULL);

  // Display the links for all walls within range of the given point
  void DisplayAvailableLinks(const Vector &cursor);

  // Check to see if any walls need to be powered up or down
  static void UpdatePowerStatus(Team *team, Bool ignorePower = FALSE);

  // If the given unit is a wall, turn it on or off
  static Bool Toggle(UnitObj *unit, Bool on);

  S32 GetRangeStraight()
  {
    return (rangeStraight);
  }

  S32 GetRangeDiagonal()
  {
    return (rangeDiagonal);
  }

  F32 GetBeamOffset(Bool primary = TRUE)
  {
    return (primary ? beamOffsetPrimary : beamOffsetSecondary);
  }

  U8 GetSurface()
  {
    return (surface);
  }

  // True if the given deviation is acceptable
  Bool ValidDeviation(F32 d)
  {
    return (d >= deviationMin && d <= deviationMax);
  }

  // True if the given type is ever allowed to link with this one
  Bool CanLinkWith(WallObjType *type)
  {
    return (!properties.GetCount() || properties.Test(type));
  }

  // Origin cell hardpoint
  NodeIdent & GetOriginHardPoint()
  {
    return (originHardPoint);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class WallObj - Instance class for above type
//

class WallObj : public UnitObj
{
public:

  enum TestResult
  {
    TR_SUCCESS,       // The operation was successful
    TR_SAMEWALL,      // Trying to link to the same wall
    TR_TYPEFAIL,      // These types can not link
    TR_NOTALLY,       // Walls must be allied
    TR_UNAVAILABLE,   // One of the walls is not available for linking
    TR_OFFMAP,        // A cell in the link is off the map
    TR_CLAIMED_S,     // A cell in the link is claimed by an immovable or unknown object
    TR_CLAIMED_M,     // A cell in the link is claimed by a mobile unit
    TR_FOOTINDEX,     // A cell in the link has a footprint on it
    TR_DEVIATION,     // The deviation of the link exceeds requirements
    TR_NOTALINK,      // The angle between the walls is not a link
    TR_LINKUSEDSRC,   // The link is already used on the source
    TR_LINKUSEDDST,   // The link is already used on the destination
    TR_DISTANCE,      // The distance between the walls is too great
  };

  // Returns a message explaining the given test result
  static const char * ExplainResult(TestResult r);

protected:

  // Manages the display of links between walls
  static Bool FXCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

  // Returns the deltas for the given link
  static const Point<S32> & LinkToDelta(S32 link);

  // Returns the link index for the given delta
  static S32 DeltaToLink(const Point<S32> &delta);

  // Returns the link that is opposite to the given one
  static S32 LinkOpposite(S32 link);

protected:

  // Maximum number of links from a wall
  enum { MAX_LINKS = 8 };

  // An iterator for each map cell in an actual link
  struct LinkIterator
  {
    // The current position
    Point<S32> pos;

    // The link being iterated
    S32 link;

    // The remaining distance to the destination
    S32 distance;

    // Constructor
    LinkIterator()
    {
      distance = 0;
    }

    // Constructor
    LinkIterator(const Point<S32> &src, S32 _link, S32 _distance)
    {
      Set(src, _link, _distance);
    }

    // Setup the iterator
    void Set(const Point<S32> &src, S32 _link, S32 _distance)
    {
      pos = src;
      link = _link;
      distance = _distance;
    }

    // Returns TRUE if p was setup
    Bool Next(Point<S32> &p)
    {
      // Are there any non-destination cells
      if (--distance > 0)
      {
        p = pos += LinkToDelta(link);
        return (TRUE);
      }
      return (FALSE);
    }
  };

  // Friends of the wall
  friend struct LinkIterator;

  // The data for a single link 
  struct Link : public WallObjPtr
  {
    // The distance to the target
    S32 distance;

    // Is the link owner responsible for the link
    Bool responsible;

    // Is this link deactivated
    Bool deactivated;

    // Constructor
    Link()
    {
      Reset();
    }

    // Set this link
    void Set(WallObj *target = NULL, S32 _distance = 0, Bool _responsible = FALSE)
    {
      Setup(target);
      distance = _distance;
      responsible = _responsible;
      deactivated = FALSE;
    }

    // Reset this link
    void Reset()
    {
      Set();
    }

    Bool Deactivated()
    {
      return (Alive() && deactivated);
    }

    // Is this link active
    Bool Active()
    {
      return (Alive() && !deactivated);
    }
  };

  // The data for each possible link
  Link links[MAX_LINKS];

  // Claiming data
  Claim::Manager claimInfo;

protected:

  // Returns the range for the given link
  S32 MaxLinkRange(S32 link);

  // Returns the given link data
  Link & GetLink(S32 link);

  // Get the given link available
  Bool LinkAvailable(S32 link);

  // Is the given cell available for construction
  TestResult AvailableCell(const Point<S32> &p);

  // Returns the height at the given cell
  F32 Height(const Point<S32> &p);

  // Check the height deviations for the given link
  Bool CheckDeviation(const Point<S32> &src, S32 link, S32 distance);

  // Test each cell in the given line
  TestResult TestLinkLine(const Point<S32> &src, S32 link, S32 distance);

  // Break the given active link
  void BreakLink(S32 link, Bool initial, Bool deactivate = TRUE);

  // Should the given link be displayed for the given deltas
  Bool LinkVisible(S32 link, Point<S32> d);

  // Is there sufficient power to maintain this wall
  Bool SufficientPower();

  // Get the position of the given beam
  void GetBeamPosition(Vector &pos, Bool primary = TRUE);

public:

  // Constructor and destructor
  WallObj(WallObjType *objType, U32 id);
  ~WallObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Get the origin cell for this wall
  Point<S32> GetOrigin();

  // Returns TRUE if there is a link to the given wall
  Bool FindLink(WallObj *target, S32 *linkPtr = NULL);

  // Is the wall available for linking
  Bool AvailableForLinking(Bool checkPower = TRUE);

  // Test if we can link to the given wall
  TestResult TestLink(WallObj *target, LinkIterator *iteratorPtr = NULL, Bool checkPower = TRUE);

  // Attempt to form a link with the given wall
  TestResult FormLink(WallObj *target, Bool checkPower = TRUE);

  // Attempt to toggle the link to the target
  TestResult ToggleLink(WallObj *target, Bool checkPower = TRUE, Bool deactivate = TRUE);

  // Add each cell in the wall link to the given evacuator
  Bool SetupEvacuator(WallObj *target, UnitEvacuate &evacuate);

  // Activate any deactivated links (if 'all', includes non-responsible)
  void ActivateIdleLinks(Bool all = FALSE);

  // Check to see if the wall needs to activate/deactivate
  void UpdatePowerStatus(Bool ignorePower = FALSE);

  // Break all current links
  void BreakLinks(Bool deactivate = TRUE);

public:

  // Setup the idle task to link with nearby walls
  void SetupAutoLink();

  // Display the links on this wall that are visible from the given point
  void DisplayLinks(WallObjType *linkType, const Vector &cursor);

public:

  // Get pointer to type
  WallObjType * WallType()
  {
    // This is a safe cast
    return ((WallObjType *)type);
  }
};

#endif