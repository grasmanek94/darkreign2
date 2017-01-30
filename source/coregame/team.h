///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team
//
// 24-JUL-1998
//

#ifndef __TEAM_H
#define __TEAM_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gameconstants.h"
#include "filesys.h"
#include "objective.h"
#include "varsys.h"
#include "utiltypes.h"
#include "relation.h"
#include "stats.h"
#include "radio.h"
#include "power.h"
#include "regionobjdec.h"
#include "displayobjective.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class Player;
namespace Strategic
{
  class Object;
}


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define TEAM_VARCHAR  '@'


///////////////////////////////////////////////////////////////////////////////
//
// Class Team
//
class Team
{
public:

  struct EndGame
  {
    static const char *resultName[];

    static const char *resultKey[];

    // Did we win, lose or no result
    enum Result
    { 
      WIN, 
      LOSE, 
      NR 
    } result;

    // Number of units constructed
    Stats::Category constructed;

    // Number of kills by type killed per relation
    Stats::Category *kills[Relation::MAX];

    // Number of self kills
    Stats::Category killsSelf;

    // Number of kills to non aligned map objects
    Stats::Category killsMisc;

    // Number of losses
    Stats::Category losses;

    // Number of resources collected by resource type
    Stats::Category resources;

  public:

    // Constructor
    EndGame();

    // Destructor
    ~EndGame();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

  };

private:

  LOGDEC


  //
  // General
  //

  // Team identifier
  U32 id;

  // Console identifier
  U32 consoleId;

  // Name of the team
  GameIdent name;

  // Has the team been postloaded
  Bool postLoaded;


  //
  // Unit Objects
  //

  // Units owned by this team
  NList<UnitObj> unitObjects;

  // Units owned by this team by type
  BinTree<NList<UnitObj> > unitObjectsByType;

  // Number of command points on team
  U32 totalCommandCost;

  // Number of units that have their team set to this
  U32 totalUnitMembers;

  // Number of units with radar ability
  U32 propertyRadar;

  // Number of units with cloaking ability
  U32 propertyCloaking;

  // List of objects registered for construction
  UnitObjList constructList;
  

  //
  // Objectives
  //

  // The current objectives of this team
  Objective::ObjList currentObjectives;

  // Set of display objectives of this team
  DisplayObjective::Set displayObjectives;


  //
  // Color
  //

  // Color of this team
  Color color;


  //
  // Relations
  //

  // Relationships this team has with other teams
  Relation relations[Game::MAX_TEAMS];
  
  // Relationships this team has with other teams (only used during loading)
  BinTree<Relation> relationsByTeamName;

  // Team groupings by relation
  List<Team> group[Relation::MAX];

  // Team groupsings for ENEMY HUMAN
  List<Team> groupEnemyHuman;

  // Bitfield of teams by relation
  Game::TeamBitfield teamsByRelation[Relation::MAX];

  // Bitfield of relations that have changed with other teams
  Game::TeamBitfield relationChanged;

  // Teams we are providing line of sight to
  U8 givingSightTo[Game::MAX_TEAMS];


  //
  // Start Location
  //

  // Region to start
  RegionObjPtr startRegion;

  // Initial yaw (radians)
  F32 startYaw;

  // Start location
  Point<F32> startPoint;


  //
  // AI
  //

  // Is the default type Client ?
  Bool defaultClient;

  // Is the team available for play
  Bool availablePlay;

  // Does this team have stats
  Bool hasStats;

  // Does this team always have radar
  Bool permanentRadar;

  // Does this team require an AI player
  Bool requireAI;
  
  // The difficulty level of this team
  F32 difficulty;

  // The unit limit for this team
  U32 unitLimit;

  // The default side for this team
  GameIdent side;

  // Is the side fixed
  Bool sideFixed;

  // Name of the AI personality this team defaults to
  GameIdent personality;

  // Pointer to strategic AI controlling this team  
  Strategic::Object *strategicObject;

  // Pain reduction
  U32 painCurrentCluster;

  // Current unique script identifier
  U32 uniqueScriptId;

  //
  // Players
  //

  // List of players which control this team
  List<Player> players;


  //
  // EndGame
  // 
  EndGame endGame;


  //
  // Power
  //

  Power::Team power;
  U32 powerLevel;
  U32 powerLevelUnfiltered;


  //
  // Resource
  //

  // Total resource stored
  U32 resourceStore;


  //
  // Type Filtering
  //

  // Is type filtering enabled
  Bool filtering;

  // What types are allowed ?
  UnitObjTypeList filteredTypes;

  //
  // Radio
  //
  Radio::EventQueue radio;


  //
  // Misc
  //


  // Tree node
  NBinTree<Team>::Node node;


  //
  // Static Data
  //


  // Team System Initialized ?
  static Bool initialized;

  // The current team used for display purposes only!
  static Team *displayTeam;

  // Tree containing all of the current teams by name
  static NBinTree<Team> teamsByName;

  // Array of teams by id
  static Team *teamsById[];

  // Scope where team vars are stored
  static const char *varScope;

  // Mono buffer for displaying info about all teams
  MonoBufDefStatic(monoBuffer);

public:

  // Team
  Team(GameIdent name, U32 id);

  // Team
  Team(FScope *fScope);

  // ~Team
  ~Team();

  // Setup
  void Setup();

  // CurrentObjective
  Objective::Object * CurrentObjective(const char *obj);

  // NewObjective
  void NewObjective(const char *obj, Bool required = TRUE);

  // ObjectiveAchieved
  void ObjectiveAchieved(Objective::Object *obj);

  // ObjectiveAbandoned
  void ObjectiveAbandoned(Objective::Object *obj);

  // Clear objectives
  void ClearObjectives();

  // PreLoad
  void PreLoad();

  // PostLoad
  void PostLoad();

  // Save
  void Save(FScope *fScope);

  // DeterminePlayers()
  void DeterminePlayers();

  // HavePlayer
  Bool HavePlayer();

  // Get Players
  const List<Player> & GetPlayers();

  // Get the player considered as the team leader
  Player * GetTeamLeader();

  // Is this an AI team
  Bool IsAI();
  
  // Is this a Human team
  Bool IsHuman();

  // Move to this teams start location
  void MoveToStart();

  // Add and remove the given unit as a MEMBER of this team
  void AddMember(UnitObj *obj);
  void RemoveMember(UnitObj *obj);

  // Add and remove the given unit from the ACTIVE team list
  void AddToTeam(UnitObj *obj);
  void RemoveFromTeam(UnitObj *obj);

  // Report a construction
  void ReportConstruction(MapObj *mapObj);

  // Report a kill
  void ReportKill(MapObj *mapObj);

  // Report a loss
  void ReportLoss(UnitObj *unitObj);

  // Report a resource
  void ReportResource(U32 amount, const char *reason);

  // Clear a resource
  void ClearResource(const char *reason);

  // Increase the amount of resource stored
  void AddResourceStore(U32 mod);

  // Decrease the amount of resource stored
  Bool RemoveResourceStore(U32 mod);

  // Set the amont of resource stored
  void SetResourceStore(U32 resource);

  // Notify the player(s) which control this team
  Bool NotifyPlayer(GameObj *from, U32 message, U32 param1 = 0, U32 param2 = 0);

  // Build the dyn var name for a team
  const char * DynVarName();

  // Is the given type allowed by the filter ?
  Bool AllowedType(UnitObjType *type);

  // Is radar currently available for this team
  Bool RadarAvailable();

  // Is cloaking currently available for this team
  Bool CloakingAvailable();

  // Returns the unit limit balance
  S32 GetUnitLimitBalance();

  // Returns TRUE if the unit limit has been reached
  Bool UnitLimitReached(UnitObjType *type = NULL, UnitObjType *remove = NULL);

  // Register that a unit has started/stopped construction for this team
  void RegisterConstructionBegin(UnitObj *unit);
  void RegisterConstructionEnd(UnitObj *unit);

  // Test if a unit type is being constructed for this team
  Bool IsConstructingType(UnitObjType *type);

  // Add a reference to provide team with line of sight
  void GiveSightTo(U32 teamId);

  // Remove a reference to provide team with line of sight
  void UngiveSightTo(U32 teamId);

  // Test if we are giving Line of sight to a team
  Bool GivingSightTo(U32 teamId)
  {
    ASSERT(teamId < Game::MAX_TEAMS)
    return (givingSightTo[teamId]);
  }

  // Give units to a team (from us)
  U32 GiveUnits(Team *to, const UnitObjList &units, UnitObjPtr &first);

  // Give money to a team (from us)
  U32 GiveMoney(Team *to, U32 amount);

  // Set the start region
  void SetStartRegion(RegionObj *region);

  // Set the starting yaw
  void SetStartYaw(F32 yaw);

  // Get this teams relationship with the given unit
  Relation GetUnitRelation(UnitObj *unit);

  // Return true if the given relation matches
  Bool TestUnitRelation(UnitObj *unit, Relation relation);

  // Return true if this team satisfies enemy human relationship
  Bool TestEnemyHumanRelation(U32 teamId);

  // Return color for the relationship with the given team
  Color GetRelationColor(Team *team);

public:

  // GetName
  const char *GetName() const
  {
    return (name.str);
  }

  // GetNameCrc
  U32 GetNameCrc() const
  {
    return (name.crc);
  }

  // GetId
  U32 GetId() const
  {
    return (id);
  }

  // GetColor
  Color GetColor() const
  {
    return (color);
  }

  // GetDimColor
  Color GetDimColor() const
  {
    Color c = color;
    c.r >>= 1;
    c.g >>= 1;
    c.b >>= 1;
    c.a = 128;
    return (c);
  }

  // SetColor
  void SetColor(const Color &c);

  // GetUnitObjects
  const NList<UnitObj> & GetUnitObjects()
  {
    return (unitObjects);
  }

  // GetUnitObjects
  const NList<UnitObj> * GetUnitObjects(U32 type)
  {
    return (unitObjectsByType.Find(type));
  }

  // GetCurrentObjectives
  const Objective::ObjList & GetCurrentObjectives()
  {
    return (currentObjectives);
  }

  // GetDisplayObjectives
  DisplayObjective::Set & GetDisplayObjectives()
  {
    return (displayObjectives);
  }

  // Return true if the given relation matches
  Bool TestRelation(U32 teamId, Relation relation)
  {
    ASSERT(teamId < Game::MAX_TEAMS)
    ASSERT(teamsById[teamId])
    ASSERT(relation < Relation::MAX)

    return (relations[teamId] == relation);
  }

  // AgreedRelation
  Bool AgreedRelation(U32 teamId, Relation relation)
  {
    ASSERT(teamId < Game::MAX_TEAMS)
    ASSERT(teamsById[teamId])
    ASSERT(relation < Relation::MAX)

    return 
    (
      (relations[teamId] == relation) 
      && 
      Id2Team(teamId)->relations[id] == relation
    );
  }

  // SetRelation
  void SetRelation(U32 teamId, Relation relation)
  {
    ASSERT(teamId < Game::MAX_TEAMS)
    ASSERT(teamsById[teamId])
    ASSERT(relation < Relation::MAX)

    ClearRelationPriv(teamId);
    SetRelationPriv(teamId, relation);
  }

  // GetRelation
  Relation GetRelation(U32 teamId)
  {
    ASSERT(teamId < Game::MAX_TEAMS)
    ASSERT(teamsById[teamId])

    return (relations[teamId]);
  }

  // GetTeamsByRelation
  Game::TeamBitfield GetTeamsByRelation(Relation relation)
  {
    ASSERT(relation < Relation::MAX)
    return (teamsByRelation[relation]);
  }

  // NumRelated
  Bool NumRelated(Relation relation)
  {
    ASSERT(relation < Relation::MAX)
    return (group[relation].GetCount());
  }

  // RelatedTeams
  const List<Team> & RelatedTeams(Relation relation)
  {
    if (relation == Relation::ENEMYHUMAN)
    {
      return (groupEnemyHuman);
    }
    else
    {
      ASSERT(relation < Relation::MAX)
      return (group[relation]);
    }
  }

  // Get the start location
  const Point<F32> & GetStartPoint()
  {
    return (startPoint);
  }

  // Get the start region
  RegionObj * GetStartRegion()
  {
    return (startRegion.GetPointer());
  }

  // Get the start yaw
  F32 GetStartYaw()
  {
    return (startYaw);
  }

  // SetStrategicObject
  void SetStrategicObject(Strategic::Object *object)
  {
    strategicObject = object;
  }

  // GetStrategicObject
  Strategic::Object * GetStrategicObject()
  {
    return (strategicObject);
  }

  // SetPersonality
  void SetPersonality(const char *name)
  {
    personality = name;
  }

  // GetPersonality
  const char * GetPersonality()
  {
    return (personality.str);
  }

  // Get the next unique script id
  U32 GetNextScriptId()
  {
    return (uniqueScriptId++);
  }

  // Report the result of the game
  void ReportResult(EndGame::Result result)
  {
    endGame.result = result;
  }

  // GetEndGame
  const EndGame & GetEndGame()
  {
    return (endGame);
  }

  // Get the team power
  const Power::Team & GetPower()
  {
    return (power);
  }
        
  // Get the total resource stored
  U32 GetResourceStore()
  {
    return (resourceStore);
  }

  // Is type filtering enabled 
  Bool GetTypeFiltering()
  {
    return (filtering);
  }

  // Set type filtering
  void SetTypeFiltering(Bool enable)
  {
    filtering = enable;
  }

  // Get the list of filtered types
  const UnitObjTypeList & GetFilteredTypes()
  {
    return (filteredTypes);
  }

  // Add the list of filtered types
  void AddFilteredTypes(const UnitObjTypeList &types)
  {
    for (UnitObjTypeList::Iterator t(&types); *t; t++)
    {
      if ((*t)->Alive())
      {
        filteredTypes.Append(**t);
      }
    }
  }

  // Set the list of filtered types
  void SetFilteredTypes(const UnitObjTypeList &types)
  {
    filteredTypes.Clear();
    AddFilteredTypes(types);
  }

  // Add a power leak
  void AddPowerLeak(UnitObj *facility, UnitObj *thief, U32 amount, U32 rate)
  {
    power.AddLeak(facility, thief, amount, rate);
  }

  // Remove a power leak
  void RemovePowerLeak(UnitObj *thief)
  {
    power.RemoveLeak(thief);
  }

  // Get the radio event queue of this team
  Radio::EventQueue & GetRadio()
  {
    return (radio);
  }

  // Get console ID for the given type
  U32 Team::GetConsoleId(U32 type)
  {
    return (consoleId ^ type);
  }

  // Notification that relations changed with another team
  void RelationsChanged(Team *team);

  // Is this the default client
  Bool IsDefaultClient()
  {
    return (defaultClient);
  }
  
  // Is this team available for play
  Bool IsAvailablePlay()
  {
    return (availablePlay);
  }

  // Is this 
  Bool HasStats()
  {
    return (hasStats);
  }

  // Does this team have permanent radar
  Bool GetPermanentRadar()
  {
    return (permanentRadar);
  }

  // Does this team require AI
  Bool GetRequireAI()
  {
    return (requireAI);
  }

  // The difficulty level of this team
  F32 GetDifficulty()
  {
    return (difficulty);
  }

  // Returns the current command cost total
  S32 GetTotalCommandCost()
  {
    return (totalCommandCost);
  }
  
  // Returns the number of unit members
  S32 GetTotalUnitMembers()
  {
    return (totalUnitMembers);
  }

  // Get the unit limit for this team
  U32 GetUnitLimit()
  {
    return (unitLimit);
  }

  // Returns the name of the default side for this team
  const char * GetSide()
  {
    return (side.str);
  }

  // Is the side fixed
  Bool GetSideFixed()
  {
    return (sideFixed);
  }

  // Set the default client
  void SetDefaultClient(Bool client)
  {
    if (client)
    {
      for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
      {
        (*t)->defaultClient = FALSE;
      }
      defaultClient = TRUE;
    }
    else
    {
      defaultClient = FALSE;
    }
  }

  // Set this team for available play
  void SetAvailablePlay(Bool available)
  {
    availablePlay = available;
  }

  // Set this has stats
  void SetHasStats(Bool stats)
  {
    hasStats = stats;
  }

  // Set the permanent radar flag
  void SetPermanentRadar(Bool flag)
  {
    permanentRadar = flag;
  }

  // Set the require AI flag
  void SetRequireAI(Bool flag)
  {
    requireAI = flag;
  }

  // Set the difficulty level of this team
  void SetDifficulty(F32 d)
  {
    difficulty = d;
  }

  // Set the unit limit for this team
  void SetUnitLimit(U32 l)
  {
    unitLimit = l;
  }

  // Set the default side for this team
  void SetSide(const char *name)
  {
    side = name;
  }

  // Set whether the side is fixed
  void SetSideFixed(Bool flag)
  {
    sideFixed = flag;
  }

private:

  // SetRelation
  void SetRelationPriv(U32 teamId, Relation alliance);

  // ClearRelation
  void ClearRelationPriv(U32 teamId);

  // Process sight updates
  void ProcessSight(Bool force);

  // Process
  void Process();

public:

  // Init : Initialize team system
  static void Init();

  // Done : Shutdown team system
  static void Done();

  // CmdHandler : Console Command handler
  static void CmdHandler(U32 pathCrc);

  // NewId: Returns an unused Id
  static U32 NewId();

  // Configure: Configure all of the teams from a config file
  static void Configure(FScope *fScope);

  // Purge : Purge all teams which don't have players
  static void Purge();

  // PreLoadAll: Allows pre loading processing to occur
  static void PreLoadAll();

  // PostLoadAll: Allows post loading processing to occur
  static void PostLoadAll();

  // If required, create side based units for all teams
  static void CreateSideBasedUnits();

  // If required, apply difficulty levels to enemy teams
  static void ApplyDifficultyLevels();

  // SaveAll: Save all teams to a config file
  static void SaveAll(FScope *fScope);
  
  // ProcessAll : Perform team processing for all teams
  static void ProcessAll(Bool force = FALSE);

  // Test a unit against the current display team (FALSE if no display team)
  static Bool TestDisplayRelation(UnitObj *unit, Relation relation);

  // Test relation of 2 teams, where either could be NULL
  static Bool TestRelation(Team *team1, Team *team2, Relation relation);

  // Test relation of unit with a team, where team could be NULL
  static Bool TestUnitRelation(UnitObj *unit, Team *team, Relation relation);

  // Get relation between two teams, where either could be NULL
  static Relation GetRelation(Team *team1, Team *team2);

  // Is the given unit type available for any team
  static Bool AllowedTypeByAny(UnitObjType *type);

  // Player had departed
  static void PlayerDeparted(Player *player);

private:

  // LoadObjectives
  static void LoadObjectives(const char *fileName);

  // UpdateMono : Update the mono display
  static void UpdateMono();

public:

  // GetDisplayTeam : Get the current display team, or NULL
  static Team * GetDisplayTeam()
  {
    ASSERT(initialized)
    return (displayTeam);
  }

  // SetDisplayTeam : Set the current display team
  static void SetDisplayTeam(Team *team)
  {
    ASSERT(initialized)
    ASSERT(team)
    displayTeam = team;
  }

  // ClearDisplayTeam : Clear the current display team
  static void ClearDisplayTeam()
  {
    ASSERT(displayTeam)
    displayTeam = NULL;
  }

  // Name2Team: Converts a team name into a team pointer
  static Team * Name2Team(const char *name)
  {
    return (teamsByName.Find(Crc::CalcStr(name)));
  }

  // NameCrc2Team: Converts a team name crc into a team pointer
  static Team * NameCrc2Team(U32 crc)
  {
    return (teamsByName.Find(crc));
  }

  // Id2Team: Converts a team Id to a team pointer
  static Team * Id2Team(U32 id)
  {
    return (teamsById[id]);
  }

  // Id2TeamName : Converts a team Id into its name
  static const char *Id2TeamName(U32 id)
  {
    ASSERT(teamsById[id])
    return (teamsById[id]->GetName());
  }

  // NumTeams: Return the number of teams which are available
  static U32 NumTeams()
  {
    return (teamsByName.GetCount());
  }

  // NumTeamsAvailable: Return the number of teams which are available
  static U32 NumAvailableTeams()
  {
    U32 num = 0;
    for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
    {
      if ((*t)->availablePlay)
      {
        num++;
      }
    }
    return (num);
  }
};

#endif