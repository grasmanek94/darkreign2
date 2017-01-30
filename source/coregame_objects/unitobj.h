///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//

#ifndef __UNITOBJ_H
#define __UNITOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobj.h"
#include "restoreobjdec.h"
#include "squadobjdec.h"
#include "trailobjdec.h"
#include "worldctrl.h"
#include "pathsearch.h"
#include "sight_map.h"
#include "prereq.h"
#include "tactical.h"
#include "gametime.h"
#include "power.h"
#include "ai_map.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class Team;
class Relation;
namespace Movement
{
  class Driver;
}
namespace Weapon
{
  class Type;
  class Object;
}
class Target;
class TransportObj;


/////////////////////////////////////////////////////////////////////////////
//
// Namespace UnitObjNotify - Notification CRC's
//
namespace UnitObjNotify
{
  // Can this unit use a rally point
  const U32 CanUseRallyPoint = 0x949735A5; // "UnitObj::CanUseRallyPoint"

  // Set the current rally point
  const U32 SetRallyPoint = 0x4BC34BEB; // "UnitObj::SetRallyPoint"

  // Get the current rally point
  const U32 GetRallyPoint = 0x52DAF57C; // "UnitObj::GetRallyPoint"
}


///////////////////////////////////////////////////////////////////////////////
//
// Constants
//
const U32 MaxWeaponNodes = 2;
const U32 MaxFirePointNodes = 4;


///////////////////////////////////////////////////////////////////////////////
//
// Class UnitObjType
//
class UnitObjType : public MapObjType
{
  PROMOTE_LINK(UnitObjType, MapObjType, 0x0F244098); // "UnitObjType"

private:

  // A single response event
  struct ResponseEvent
  {
    // Name of the event
    GameIdent name;

    // Number of variations found
    U32 count;

    ResponseEvent(const char *name, U32 count) : name(name), count(count) {}
  };

  // List of all known response events
  List<ResponseEvent> responseEvents;


  // Weapon name
  GameIdent weaponName;

  // This object's weapon
  Weapon::Type *weapon;

  // All of this stuff is here because weapon use the units 
  // mesh and this information is bound to that mesh.

  // Nodes used by the weapon
  NodeIdent weaponNodes[MaxWeaponNodes];

  // Firepoint nodes
  NodeIdent firePointNodes[MaxFirePointNodes];

  // Firing node
  NodeIdent *fireNode;

  // Vector to the barrel
  Vector barrel;

  // Cluster bleed map for the weapon
  AI::Map::BleedMap *weaponBleedMap;

  // Cluster bleed map for the weapon
  AI::Map::BleedMap *selfDestructBleedMap;

  // Current seeing range in metres (defaults to daytime seeing range)
  InstanceModifierType seeingRange;

  // Seeing range during the day
  U32 seeingRangeDay;

  // Seeing range during the night
  U32 seeingRangeNight;

  // Seeing height, in metres
  F32 seeingHeight;

  // Disruption range
  F32 disruptionRange;
  F32 disruptionRange2;

  // Cluster bleed map for the disruption
  AI::Map::BleedMap *disruptionBleedMap;

  // Top speed of the vehicle
  InstanceModifierType maxSpeed;
  F32 maxSpeedInv;

  // Linear acceleration m/s/s
  F32 linearAccel;
  F32 linearAccel2inv;

  // Maximum angular speed rad/s
  F32 turnSpeed;

  // Angular acceleration rad/s/s
  F32 turnAccel;
  F32 turnAccel2inv;

  // Altitude to at lower and upper levels
  F32 altitude[Claim::LAYER_COUNT];

  // Bit flags
  U32 isSelectable      : 1,  // Object can be selected
      canFireIndirect   : 1,  // Object can fire indirectly
      canEverMove       : 1,  // Can this object EVER move
      canEverFire       : 1,  // Can this object EVER fire
      canAutoTarget     : 1,  // Can this unit automatically find targets
      needsBoardManager : 1,  // Does this object need a board manager
      antiClandestine   : 1,  // Does this object remove clandestine status
      disruptor         : 1,  // Does this object disrupt remote functionality
      isFacility        : 1,  // Has UnitConstructor as an idle task
      canBeInfiltrated  : 1,  // Can this object be infiltrated by a spy
      isFiltered        : 1;  // Can't be built cause its filtered

  // Prereqs
  Prereq prereqs;


  // Constructor of this unit
  Reaper<UnitObjType> constructorType;

  // Does the constructor get swallowed and returned
  Bool constructorConsume;

  // Construction time in seconds
  F32 constructionTime;

  // Initial hitpoints when being constructed
  S32 constructionHitPoints;

  // List of types this type is capable of constructing
  UnitObjTypeList constructionWares;

  // A unit that gets created when this object is built in the game
  Reaper<UnitObjType> companionType;

  // Default tactical modifier
  GameIdent defaultBehavior;


  // Amount of resource this object needs to be ordered
  U32 resourceCost;

  // Number of command points this unit uses
  U32 commandCost;

  // Amount of resource this object can transport
  U32 resourceTransport;

  // Rate at which resource is loaded
  U32 resourceLoad;

  // Rate at which resource is unloaded
  U32 resourceUnload;


  // Power
  Power::Unit power;

  // Efficiency
  Power::Efficiency efficiency;


  // The number of seconds this unit takes to recycle
  F32 recycleTime;

  // Percentage of resource cost refunded when recycled
  F32 recyclePercentage;


  // Time it takes to power down (in seconds)
  F32 powerDownTime;

  // Time it takes to power up (in seconds)
  F32 powerUpTime;

  struct UpgradeInfo
  {
    Reaper<UnitObjType> type;
    NodeIdent point;
    U32 index;
  }; 
  
  // Upgrade information
  List<UpgradeInfo> upgrades;

  // Type that this unit is an upgrade for
  UnitObjTypePtr upgradeFor;

public:

  // List nodes
  NList<UnitObjType>::Node sightChangeNode;

  // Constructor
  UnitObjType(const char *typeName, FScope *fScope);

  // Destructor
  ~UnitObjType();

  // Are instances of this type on the primitive list by default
  Bool DefaultPrimitiveProcessing()
  {
    return (TRUE);
  }

  // Called after all types are loaded
  void PostLoad();

  // Returns static buffer containing the file name of the given response event
  const char * ResponseName(const char *asset, const char *event, U32 variation);

  // Load a single unit response event
  Bool LoadResponseEvent(const char *event);

  // Initialized type specific resources
  Bool InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);

  // Get the treat this unit has to the given armour class
  U32 GetThreat(U32 ac);

  // Get the weapon threat
  U32 GetWeaponThreat(U32 ac);

  // Get the self destruct threat
  U32 GetSelfDestructThreat(U32 ac);

  // Get the type that constructs this object, or NULL
  UnitObjType * GetConstructorType();

  // Get the type that accompanies this object, or NULL
  UnitObjType * GetCompanionType();

  // Update seing range
  Bool UpdateSeeingRange(F32 val);

  // Get the responses for this event, or NULL if none
  ResponseEvent * GetResponse(U32 event);

  // Returns the name of a random response, or NULL if none available
  const char * RandomResponse(U32 event);

  // Create a unit at the closest available location, or return NULL
  UnitObj * SpawnClosest(const Vector &pos, Team *team = NULL, Bool construct = FALSE, F32 orientation = 0.0F);

  // Create a unit at the closest available linked location, or return NULL
  UnitObj * SpawnLinked(const Vector &pos, Team *team = NULL, Bool construct = FALSE, F32 orientation = 0.0F);

  // Create a unit at the given location
  UnitObj & Spawn(const Vector &pos, Team *team = NULL, Bool construct = FALSE, F32 orientation = 0.0F);
  UnitObj & Spawn(const Matrix &mat, Team *team = NULL, Bool construct = FALSE);

  // Create a unit inside the hold of a transport (team is automatically set by the transport)
  UnitObj & Spawn(TransportObj *transport);

  // Create a unit from the given facility
  UnitObj & Spawn(UnitObj *subject, Bool move = TRUE);

  // Get the closest position this type can move on
  Bool FindClosestPos(const Vector &src, Vector &dst, U32 range = 10);

  // Get the closest linked location
  Bool FindLinkedPos(const Vector &src, Vector &dst, U32 range = 10);

public:

  // Find an enemy anti-clandestine unit that can see the given location
  static UnitObj * FindAntiClandestine(Team *team, const Vector &location);

  // Find an enemy disruptor unit that can see the given location
  static UnitObj * FindDisruptor(Team *team, const Vector &location);

public:

  // Does the constructor get swallowed and returned
  Bool GetConstructorConsume()
  {
    return (constructorConsume);
  }

  // Get construction time
  F32 GetConstructionTime()
  {
    return (constructionTime);
  }

  // Initial hitpoints when being constructed
  S32 GetConstructionHitPoints()
  {
    return (constructionHitPoints);
  }

  // Get the construction wares
  const UnitObjTypeList & GetConstructionWares()
  {
    return (constructionWares);
  }

  // Return the maximum seeing range of this unit
  U32 GetMaxSeeingRange() const
  {
    return (Max(seeingRangeDay, seeingRangeNight));
  }

  // Return the seeing range
  U32 GetSeeingRange() const
  {
    return (U32(seeingRange.GetInteger()));
  }

  // Return seeing height
  F32 GetSeeingHeight() const
  {
    ASSERT(resourcesInitialized)
    return (seeingHeight);
  }

  // Return the top speed
  F32 GetMaxSpeed() const
  {
    return (maxSpeed.GetFPoint());
  }

  // Return Linear acceleration
  F32 LinearAcceleration()
  {
    return (linearAccel);
  }

  // Return 1 / 2 * linear acceleration
  F32 LinearAccel2Inv()
  {
    return (linearAccel2inv);
  }

  // Return maximum angular speed
  F32 TurnSpeed()
  {
    return (turnSpeed);
  }

  // Return Angular acceleration
  F32 TurnAcceleration()
  {
    return (turnAccel);
  }

  // Return 1 / 2 * Angular acceleration
  F32 TurnAccel2Inv()
  {
    return (turnAccel2inv);
  }

  // Return hovering/flying height
  F32 GetAltitude(Claim::LayerId layer)
  {
    return (altitude[layer]);
  }

  // TRUE if this object can be selected
  Bool IsSelectable()
  {
    return (isSelectable);
  }

  // TRUE if this object can fire indirectly
  Bool CanFireIndirect()
  {
    return (canFireIndirect);
  }

  // TRUE if thie object can move
  Bool CanEverMove()
  {
    return (canEverMove);
  }

  // TRUE if thie object can fire
  Bool CanEverFire()
  {
    return (canEverFire);
  }

  // Can this unit automatically find targets
  Bool CanAutoTarget()
  {
    return (canAutoTarget);
  }

  // Does this object remove clandestine status
  Bool GetAntiClandestine()
  {
    return (antiClandestine);
  }

  // Does this unit disrupt remote functionality
  Bool GetDisruptor()
  {
    return (disruptor);
  }

  // What is the disruption range of this unit
  F32 GetDisruptionRange()
  {
    return (disruptionRange);
  }

  // What is the square of the disruption range of this unit
  F32 GetDisruptionRange2()
  {
    return (disruptionRange2);
  }

  // Does this unit have the UnitConstructor task as idle
  Bool GetIsFacility()
  {
    return (isFacility);
  }

  // Can this object be infiltrated by a spy
  Bool CanBeInfiltrated()
  {
    return (canBeInfiltrated);
  }

  // Set the filtering for this type
  void SetFiltered(Bool flag)
  {
    isFiltered = flag;
  }

  // Is this type filtered
  Bool IsFiltered()
  {
    return (isFiltered);
  }

  // Get the prereq for this type
  const Prereq &GetPrereqs()
  {
    return (prereqs);
  }

  // Get the power
  const Power::Unit & GetPower()
  {
    return (power);
  }

  // Get the efficiency
  const Power::Efficiency & GetEfficiency()
  {
    return (efficiency);
  }

  // Get the resource cost
  U32 GetResourceCost()
  {
    return (resourceCost);
  }

  // Get the command cost
  U32 GetCommandCost()
  {
    return (commandCost);
  }

  // Get the resource transport
  U32 GetResourceTransport()
  {
    return (resourceTransport);
  }

  // Get the resource load rate
  U32 GetResourceLoad()
  {
    return (resourceLoad);
  }

  // Get the resource unload rate
  U32 GetResourceUnload()
  {
    return (resourceUnload);
  }

  // The number of seconds this unit takes to recycle
  F32 GetRecycleTime()
  {
    return (recycleTime);
  }

  // Percentage of resource cost refunded when recycled
  F32 GetRecyclePercentage()
  {
    return (recyclePercentage);
  }

  // The number of seconds it takes this unit to power down
  F32 GetPowerDownTime()
  {
    return (powerDownTime);
  }

  // The number of seconds it takes this unit to power up
  F32 GetPowerUpTime()
  {
    return (powerUpTime);
  }

  // Can this unit be boarded?
  Bool CanBoard()
  {
    return (needsBoardManager);
  }

  // Instance modifiers
  const InstanceModifierType &MaxSpeedIM()
  {
    return (maxSpeed);
  }

  const InstanceModifierType &SeeingRangeIM()
  {
    return (seeingRange);
  }

  // Get upgrade for
  UnitObjType * GetUpgradeFor()
  {
    return (upgradeFor.Alive() ? upgradeFor.GetData() : NULL);
  }

  // Get weapon
  Weapon::Type * GetWeapon()
  {
    return (weapon);
  }

  // Friends of UnitObjType
  friend UnitObj;
  friend Prereq;
  friend Weapon::Type;
  friend Weapon::Object;
};


///////////////////////////////////////////////////////////////////////////////
//
// Class UnitObj - Instance class for above type
//

class UnitObj : public MapObj
{
public:

  // State flags
  enum UnitFlag
  { 
    FLAG_CLANDESTINE  = 0x01,     // Object will be ignored by enemies
    FLAG_INVISIBLE    = 0x02,     // Not displayed for enemies
    FLAG_PARASITE     = 0x04,     // A parasite is attached to this object
    FLAG_UPDATELOS    = 0x08,     // Does the line of sight need to be updated
    FLAG_CARGO        = 0x10,     // Is this unit inside a transporter
    FLAG_SELFDESTRUCT = 0x20,     // Does this unit wish to self destruct

    // Flags that don't need to be saved
    FLAG_NONSAVING   = FLAG_UPDATELOS,
  };

private: 

  // Current state flags
  U8 flags;

  // Team who owns this mapobject
  Team *team;

  // Amount of resource currently transported
  U32 resourceTransport;

  // Current altitude
  F32 altitude;

  // Teams which can see this unit
  Game::TeamBitfield teamsCanSee;

  // Teams which have seen this unit
  Game::TeamBitfield teamsHaveSeen;

  // Teams which have a spy inside this unit
  Game::TeamBitfield spyingTeams;

  // Instance modified values
  InstanceModifier maxSpeed;
  InstanceModifier seeingRange;

  // Squad this unit belongs to
  SquadObjPtr squad;

  // Primary restore facility
  RestoreObjPtr primaryRestore;

  // Reapers to each possible upgrade
  UnitObjPtr *upgrades;

  // The weapon
  Weapon::Object *weapon;

protected:

  // Pathsearching object
  PathSearch::Finder pathFinder;

  // Path following object
  Movement::Driver *driver;

  // Boarding manager
  Movement::BoardManager *boardManager;

  // Unit this one is linked to
  UnitObjPtr linkedUnit;

public:

  // Line of sight object
  Sight::Map *sightMap;

  // Tactical AI settings
  Tactical::ModifierSettings settings;

  // If not elapsed the unit will target any object
  GameTime::Timer blindTarget;

  // List Nodes
  NList<UnitObj>::Node unitNode;
  NList<UnitObj>::Node teamNode;
  NList<UnitObj>::Node teamTypeNode;

  // Constructor and destructor
  UnitObj(UnitObjType *objType, U32 id);
  ~UnitObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Send an event
  Bool SendEvent(const Task::Event &event, Bool idle = FALSE);

  // Tell unit to move to a location/direction
  void Move(Movement::Handle &handle, const Vector *dst, const Vector *dir, Bool passUnit, Movement::RequestData *req = NULL);

  // Tell unit to move onto the given trail
  void Move(Movement::Handle &handle, TrailObj *trail, Bool passUnit, Movement::RequestData *req = NULL);

  // Tell unit to move into an object with an ENTRY hardpoint
  void MoveOnto(Movement::Handle &handle, UnitObj *obj, Bool passUnit, Movement::RequestData *req = NULL);

  // Get a random position that will cause a unit to move away from its current location
  void GetMoveAwayPosition(Vector &dest, F32 distance);

  // Make this unit move away
  void MoveAwayDirect(Movement::Handle &handle, F32 distance = 40.0F);

  // Make this unit move away using the move task (FALSE if unable)
  Bool MoveAwayTask(F32 distance = 40.0F);

  // Order the unit to stop moving
  Bool Stop();

  // AddToMapHook
  void AddToMapHook();

  // RemoveFromMapHook
  void RemoveFromMapHook();

  // Capture/Release cell hooks when the object's centre point moves cells
  void CaptureCellHooks(Bool capture);

  // Per-cycle processing
  void ProcessCycle();

  // Capture and release map hooks
  void CaptureMapHooks(Bool capture);

  // Set the team this object belongs to
  void SetTeam(Team *t, Bool modifyTeamList = TRUE);

  // Find cover from enemies and the optional unit
  Bool FindCover(Bool search, UnitObj *avoid = NULL);

  // Find the given source of danger
  Bool FindDanger(UnitObj *sourceUnit = NULL);

  // Is it time for this unit to respond to danger
  Bool ReadyToRespondToDanger();

  // Give this unit the chance to respond to danger
  Bool DangerResponse(UnitObj *sourceUnit = NULL, UnitObj *buddy = NULL);

  // Give all friendly nearby units the chance to respond to danger
  void DangerResponseAlert(UnitObj *sourceUnit = NULL);

  // Modify the number of hitpoints an object has
  void ModifyHitPoints(S32 mod, UnitObj *sourceUnit = NULL, Team *sourceTeam = NULL, const Vector *direction = NULL);

  // Modify the armour an object has
  void ModifyArmour(S32 mod);

  // Get the amount of resource transported
  U32 GetResourceTransport();

  // Add the amount of resource transported
  U32 AddResourceTransport(U32 amount);

  // Remove the amount of resource transported
  U32 RemoveResourceTransport(U32 amount);

  // Clear the amount of resource transported
  void ClearResourceTransport();

  // This unit can now be seen by the given team
  void SetCanSee(U32 team);

  // This unit cannot be seen by the given team
  void ClearCanSee(U32 team);

  // Can this unit be seen by a team of the given relation
  Bool SeenByRelation(Relation r);

  // Is this an AI controlled unit
  Bool IsAI();

  // Get the squad this unit belongs to
  SquadObj * GetSquad();

  // Clear the squad this unit belongs to
  void ClearSquad();

  // Set the squad this unit belongs to
  void SetSquad(SquadObj *squad);

  // Send a notification to the squad if any
  void PostSquadEvent(const Task::Event &event, Bool idle = FALSE);

  // Can this unit ever damage the given target
  Bool CanEverDamage(const Target &target);

  // Can this unit ever damage the given map object
  Bool CanEverDamage(MapObj *target);

  // Can this unit damage the target right now
  Bool CanDamageNow(const Target &target);

  // Can this unit damage the given map object right now
  Bool CanDamageNow(MapObj *target);

  // Is this unit force attacking the given object
  Bool ForceAttacking(MapObj *target);

  // Is this unit force attacking the given target
  Bool ForceAttacking(const Target &target);

  // Suggest a target
  Bool SuggestTarget(const Target &target, Bool potShot = FALSE, Bool notify = TRUE);

  // Get the maximum ammunition this unit has
  U32 GetMaximumAmmunition();

  // Get the current ammunition this unit has
  U32 GetAmmunition();

  // Reload ammunition
  void ReloadAmmunition(F32 pct);

  // Get the primary restore facility
  RestoreObj * GetPrimaryRestore();

  // Set the primary restore facility
  void SetPrimaryRestore(RestoreObj *target);

  // If required, get an available restore facility
  RestoreObj * GetRestoreFacility();

  // Go to a restoration facility
  void RestoreNow();

  // Same as above, but posts an event
  void PostRestoreNowEvent();

  // Notify the player
  Bool NotifyPlayer(U32 message, U32 param1 = 0, U32 param2 = 0);

  // Start blind target timer
  void StartBlindTarget(F32 time);

  // Get the current driver destination, if there is one
  Bool GetCurrentDestination(Vector &v);

  // Can this unit currently recycle
  Bool CanRecycleNow();

  // Recycle this unit now
  Bool RecycleNow();

  // Upgrade this unit, FALSE if unable
  UnitObj * UpgradeNow(Bool immediate = FALSE);

  // Get the type of the next upgrade
  UnitObjType * GetNextUpgrade();

  // Can this unit currently upgrade
  Bool CanUpgradeNow();

  // Get the reaper for the given upgrade slot
  UnitObjPtr & GetUpgrade(U32 i);

  // Returns the total resource value of this unit (includes upgrades)
  U32 GetResourceValue();

  // Returns percentage construction is complete
  F32 GetConstructionPercentage();

  // Returns the recycle times, including upgrades & partial construction
  void GetRecycleTimes(F32 &total, F32 &current);

  // Returns TRUE if this object can ever construct the given type
  Bool CanConstructEver(UnitObjType *t);

  // Returns TRUE if this object can currently construct the given type
  Bool CanConstructNow(UnitObjType *t);

  // Returns TRUE if both the above methods succeed
  Bool CanConstruct(UnitObjType *t);

  // Is it an Upgrade and I have don't it ?
  Bool MissingUpgrade(UnitObjType *t);

  // Is this object under construction
  Bool UnderConstruction();

  // Get the current efficiency of this object
  F32 GetEfficiency();

  // Get the highest upgrade, or this unit if none
  UnitObj & GetUpgradedUnit();

  // Can this unit use a rally point
  Bool CanUseRallyPoint();

  // Set the current rally point
  Bool SetRallyPoint(const Point<U32> &p);

  // Get the current rally point
  Bool GetRallyPoint(Point<U32> &p);

  // Set animation on this object and all upgrades
  void SetAnimationUpgrades(U32 nameCrc, Bool blend = TRUE);

  // Set tactical settings
  void SetTacticalModifierSetting(U8 modifier, U8 index);

  // Does this object have a spy inside it from this team?
  Bool HasSpyInside(U32 team);

  // Set spy inside this building state for this team
  void SetSpyInside(U32 team, Bool state);

  // Used to temporarily restrict/restore the line of sight (construction, powerup/down)
  void ModifySight(Bool restrict, Bool instant);

  // Can this unit see the given object
  Bool GetCanSee(MapObj *object);

  // Display specialized unit information
  void RenderSpecialized();

  // Dump information about the object
  ostream & Info(ostream &o);

protected:

  // Heuristics for finding cells
  Bool DefenseAndEnemyCantSee(U32 &val, U32 x, U32 z, void *context);

  // Structure for use in the below heuristic
  struct AwayFromPositionData
  {
    Vector origin;
    F32 angle;
  };

  // Maximize for moving away from a position
  Bool AwayFromPosition(U32 &val, U32 x, U32 z, void *context);

  // Engine FX Callback
  static Bool EngineCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

  // Blind Target FX Callback
  static Bool BlindCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

public:

  // Tell the units in the given list to 'scatter'
  static void Scatter(const UnitObjList &list);

public:

  // Can this object ever move?
  Bool CanEverMove()
  {
    return (UnitType()->canEverMove);
  }

  // Can this object ever fire?
  Bool CanEverFire()
  {
    return (UnitType()->canEverFire);
  }

  // Return the value of a flag
  Bool GetFlag(UnitFlag f)
  {
    return (flags & f);
  }

  // Set the value of a flag
  void SetFlag(UnitFlag f, Bool value)
  {
    flags = U8((value) ? (flags | f) : (flags & ~f));
  }

  // Get this units weapon
  Weapon::Object * GetWeapon()
  {
    return (weapon);
  }

  // Is this object actively on a team
  Bool IsActivelyOnTeam()
  {
    return (teamNode.InUse());
  }

  // Get the team this object is on
  Team *GetTeam()
  {
    return team;
  }

  // Get the team this object is actively on
  Team *GetActiveTeam()
  {
    return (IsActivelyOnTeam() ? GetTeam() : NULL);
  }

  // Get the path searching object
  PathSearch::Finder &GetPathFinder()
  {
    return (pathFinder);
  }

  // Get the path follower object
  Movement::Driver *GetDriver()
  {
    ASSERT(driver)
    return (driver);
  }

  // Get the board manager object
  Movement::BoardManager *GetBoardManager()
  {
    ASSERT(boardManager)
    return (boardManager);
  }

  // Get pointer to type
  UnitObjType* UnitType()
  {
    // This is a safe cast
    return ((UnitObjType*) type);
  };

  // Test to see if a team can see this unit
  Bool TestCanSee(U32 id)
  {
    return (Game::TeamTest(teamsCanSee, id));
  }

  // Test to see if a team has seen this unit
  Bool TestHaveSeen(U32 id)
  {
    return (Game::TeamTest(teamsHaveSeen, id));
  }

  // Get teams that can see this object
  Game::TeamBitfield GetTeamsCanSee()
  {
    return (teamsCanSee);
  }

  // Get teams that have spies in this building
  Game::TeamBitfield GetSpyingTeams()
  {
    return (spyingTeams);
  }

  // Get the current altitude
  F32 GetAltitude()
  {
    return (altitude);
  }

  // Set the altitude
  void SetAltitude(F32 f);

  // Return the top speed
  F32 GetMaxSpeed() const
  {
    return (maxSpeed.GetFPoint());
  }

  // Return the seeing range
  U32 GetSeeingRange() const 
  {
    return U32(seeingRange.GetInteger());
  }

  // Instance modifiers
  InstanceModifier &MaxSpeedIM()
  {
    return (maxSpeed);
  }

  InstanceModifier &SeeingRangeIM()
  {
    return (seeingRange);
  }

  // Get the unit this one is linked to, or NULL
  UnitObj * GetLinkedUnit()
  {
    return (linkedUnit.GetPointer());
  }

  // Set the unit this one is linked to
  void SetLinkedUnit(UnitObj *unit)
  {
    linkedUnit = unit;
  }

  //
  // Debugging
  //
  #ifdef DEVELOPMENT

  void RenderDebug();
  
  #endif
};

#endif
