///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team
//
// 24-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "team.h"
#include "console.h"
#include "main.h"
#include "player.h"
#include "stdload.h"
#include "strategic.h"
#include "strategic_object.h"
#include "gameobjctrl.h"
#include "resolver.h"
#include "unitobj.h"
#include "worldctrl.h"
#include "iface.h"
#include "iclistbox.h"
#include "icstatic.h"
#include "sight.h"
#include "gametime.h"
#include "viewer.h"
#include "trailobj.h"
#include "wallobj.h"
#include "common.h"
#include "savegame.h"
#include "varfile.h"
#include "difficulty.h"
#include "sides.h"
#include "sideplacement.h"
#include "user.h"
#include "unitlimits.h"
#include "message.h"
#include "spyobj.h"
#include "sync.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define CON_TEAM(x) CONSOLE(GetConsoleId(0xEDF0E1CF), x ) // "Team"


///////////////////////////////////////////////////////////////////////////////
//
// Logging
//
LOGDEF(Team, "Team")


///////////////////////////////////////////////////////////////////////////////
//
// Internal Data
//
Bool Team::initialized = FALSE;
Team *Team::displayTeam;
NBinTree<Team> Team::teamsByName(&Team::node);
Team *Team::teamsById[Game::MAX_TEAMS];

MonoBufDef(Team::monoBuffer);
const char *Team::varScope = "team.dyndata";


///////////////////////////////////////////////////////////////////////////////
//
// Class Team::EndGame
//
const char *Team::EndGame::resultName[] = { "Win", "Lose", "No Result" };

const char *Team::EndGame::resultKey[] = { "#game.stats.win", "#game.stats.lose", "#game.stats.noresult" };


///////////////////////////////////////////////////////////////////////////////
//
// Struct TeamSubstitution 
//
struct TeamSubstitution : public VarSys::Substitution
{

  // Constructor
  TeamSubstitution() : VarSys::Substitution(TEAM_VARCHAR) { }

  // Expansion
  const char *Expand(const char *name, void *context)
  {
    Team *team;

    switch (*name)
    {
      case TEAM_VARCHAR:
        // Use the display team
        team = Team::GetDisplayTeam();
        if (!team)
        {
          LOG_DIAG(("Expanding '%s': No Display Team", name))
          return (name);
        }
        break;

      case '\0':
        // Use the context team
        if (context)
        {
          team = reinterpret_cast<Team *>(context);
        }
        else
        {
          LOG_DIAG(("Expanding '%s': Expected context for team substitution", name))
          return (name);
        }
        break;

      default:
        team = Team::Name2Team(name);
        if (!team)
        {
          LOG_DIAG(("Expanding '%s': Could not find team", name));
          return (name);
        }
        break;
    }

    return (team->DynVarName());
  }

};

static TeamSubstitution substitution;


//
// EndGame
//
Team::EndGame::EndGame() :
  result(NR),
  constructed("Constructed"),
  killsSelf("SelfKills"),
  killsMisc("MiscKills"),
  losses("Losses"),
  resources("Resources")
{
  for (U32 r = 0; r < Relation::MAX; r++)
  {
    // Build the name from the relation
    char name[256];
    Utils::Strcpy(name, Relation((Relation::Type) r).GetName());
    Utils::Strcat(name, "Kills");

    kills[r] = new Stats::Category(name);
  }
}


//
// ~EndGame
//
Team::EndGame::~EndGame()
{
  for (U32 r = 0; r < Relation::MAX; r++)
  {
    delete kills[r];
  }
}


//
// SaveState
//
void Team::EndGame::SaveState(FScope *scope)
{
  StdSave::TypeU32(scope, "Result", result);
  
  constructed.SaveState(scope->AddFunction("Constructed"));

  for (U32 i = 0; i < Relation::MAX; i++)
  {
    FScope *sScope = StdSave::TypeU32(scope, "Kills", i);
    kills[i]->SaveState(sScope);
  }

  killsSelf.SaveState(scope->AddFunction("KillsSelf"));
  killsMisc.SaveState(scope->AddFunction("KillsMisc"));
  losses.SaveState(scope->AddFunction("Losses"));
  resources.SaveState(scope->AddFunction("Resources"));
}


//
// LoadState
//
void Team::EndGame::LoadState(FScope *scope)
{
  FScope *sScope;

  while ((sScope = scope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0x36AB92D6: // "Result"
        result = Result(StdLoad::TypeU32(sScope));
        break;

      case 0xCB107CBF: // "Constructed"
        constructed.LoadState(sScope);
        break;

      case 0x3E2459B1: // "Kills"
      {
        U32 i = StdLoad::TypeU32(sScope);

        if (i < U32(Relation::MAX))
        {
          kills[i]->LoadState(sScope);
        }
        break;
      }

      case 0x6F7A2782: // "KillsSelf"
        killsSelf.LoadState(sScope);
        break;

      case 0x12DFED83: // "KillsMisc"
        killsMisc.LoadState(sScope);
        break;

      case 0x926CEDB4: // "Losses"
        losses.LoadState(sScope);
        break;

      case 0x8C34B766: // "Resources"
        resources.LoadState(sScope);
        break;
    }
  }
}



///////////////////////////////////////////////////////////////////////////////
//
// Class Team
//


//
// Team : Initalizing version
//
Team::Team(GameIdent name, U32 id) : 
  id(id),             // Set the Team ID
  name(name),         // Set the name of the team
  startYaw(0.0F)
{ 
  ASSERT(initialized)

  // Setup the team
  Setup();

  // Add this team to the neutral lists of all pre-existing teams
  // and add those teams to our list of neutral teams
  for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
  {
    if ((*t) != this)
    {
      (*t)->SetRelationPriv(id, Relation::NEUTRAL);
      SetRelationPriv((*t)->GetId(), Relation::NEUTRAL);
    }
  }

  // Since this is the initializing version then we need to set some user
  // friendly defaults.  The initializing version can only be used after
  // the types are loaded.  So... check to see if there is another team
  // which has been set as client.  If not make this team the client team.
  // Either way select the first AI type available and set it as the default
  // AI.
  defaultClient = TRUE;
  for (!t; *t; t++)
  {
    if (((*t) != this) && (*t)->defaultClient)
    {
      defaultClient = FALSE;
    }
  }

  /*
  NBinTree<Strategic::Type>::Iterator i(Strategic::GetTypes());
  if (!(*i))
  { 
    ERR_CONFIG(("There are no default strategic AI's to choose from"))
  }
  defaultAI = (*i)->GetName();
  */
} 


//
// Team : Loading version
//
Team::Team(FScope *fScope)
{
  ASSERT(initialized)
  ASSERT(fScope)

  // Get name of team
  name = fScope->NextArgString();

  // Get id of team
  id = fScope->NextArgInteger();

  // Ensure that this team is not already used
  ASSERT(!teamsById[id])

  // Setup the team
  Setup();

  // Load the configuration
  FScope *sScope;
  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xC908780F: // "Color"
      {
        S32 red, green, blue;
        red = StdLoad::TypeU32(sScope);
        green = StdLoad::TypeU32(sScope);
        blue = StdLoad::TypeU32(sScope);

        color = Color(red, green, blue);
        break;
      }

      case 0x0DDAA584: // "DefaultClient"
        defaultClient = StdLoad::TypeU32(sScope);
        break;

      case 0xCE0973A7: // "AvailablePlay"
        availablePlay = StdLoad::TypeU32(sScope);
        break;

      case 0xEB129ED9: // "HasStats"
        hasStats = StdLoad::TypeU32(sScope);
        break;

      case 0x2BE1F579: // "PermanentRadar"
        permanentRadar = StdLoad::TypeU32(sScope);
        break;

      case 0x8B6891D7: // "RequireAI"
        requireAI = StdLoad::TypeU32(sScope);
        break;

      case 0x171A345B: // "Difficulty"
        difficulty = StdLoad::TypeF32(sScope);
        break;

      case 0x14C55DB9: // "UnitLimit"
        unitLimit = StdLoad::TypeU32(sScope);
        break;

      case 0x49E0C44D: // "Side"
        side = StdLoad::TypeString(sScope);
        break;

      case 0x54C55E75: // "SideFixed"
        sideFixed = StdLoad::TypeU32(sScope);
        break;

      case 0xE6DED68D: // "EndGame"
        endGame.LoadState(sScope);
        break;

      case 0x29986528: // "ConstructList"
        StdLoad::TypeReaperList(sScope, constructList);
        break;

      case 0x4A0ED712: // "DisplayObjectives"
        displayObjectives.LoadState(sScope);
        break;

      case 0x19315046: // "GivingSightTo"
      {
        U32 i = StdLoad::TypeU32(sScope);
        
        if (i < Game::MAX_TEAMS)
        {
          givingSightTo[i] = U8(StdLoad::TypeU32(sScope));
        }
        break;
      }

      case 0xA42AD0B3: // "Relations"
      {
        FScope *ssScope;
        while ((ssScope = sScope->NextFunction()) != NULL)
        {
          switch (ssScope->NameCrc())
          {
            case 0x14899588: // "With"
            {
              const char *team = ssScope->NextArgString();
              Relation *relation = new Relation((ssScope->NextArgString()));
              relationsByTeamName.Add(Crc::CalcStr(team), relation);
              break;
            }

            default:
              LOG_WARN(("Unexpected Command '%s'", ssScope->NameStr()))
              break;
          }
        }
        break;
      }

      case 0xB69043F0: // "DynData"
        VarFile::Load(sScope, DynVarName());
        break;

      case 0x6E4C83C0: // "Objectives"
        StdLoad::TypeReaperList(sScope, currentObjectives);
        break;

      case 0x39E49422: // "StartRegion"
        StdLoad::TypeReaper(sScope, startRegion);
        break;

      case 0xB19098CC: // "StartPoint"
        StdLoad::TypePoint(sScope, startPoint);
        break;

      case 0xA026B5E4: // "StartYaw"
        startYaw = StdLoad::TypeF32(sScope);
        break;

      case 0x95A076ED: // "ResourceStore"
        resourceStore = StdLoad::TypeU32(sScope);
        ReportResource(resourceStore, "resource.initial");
        break;

      case 0x8DB86B0C: // "Personality"
        personality = StdLoad::TypeString(sScope);
        break;

      case 0x1E14EE29: // "PainCurrentCluster"
        painCurrentCluster = StdLoad::TypeU32(sScope);
        break;

      case 0xA748C922: // "UniqueScriptId"
        uniqueScriptId = StdLoad::TypeU32(sScope);
        break;

      case 0x593A8B7B: // "FilteringEnabled"
        filtering = StdLoad::TypeU32(sScope);
        break;

      case 0xE4C4B902: // "FilteredTypes"
      {
        // Support old style save
        if (filtering)
        {
          StdLoad::TypeReaperListObjType(sScope, filteredTypes);
        }
        break;
      }

      case 0xAA515A08: // "Power"
        power.LoadState(sScope);
        break;
    }
  }
}


//
// ~Team
//
Team::~Team()
{
  ASSERT(initialized)

  if (unitObjects.GetCount())
  {
    ERR_FATAL(("%d objects left in Team::unitObjects at destruction", unitObjects.GetCount()));
  }

  if (unitObjectsByType.GetCount())
  {
    ERR_FATAL(("%d objects left in Team::unitObjectsByType at destruction", unitObjectsByType.GetCount()));
  }

  // Delete all trails belonging to this team
  TrailObj::Delete(this);

  // Clear current objectives
  currentObjectives.Clear();

  // Free the team id
  teamsById[id] = NULL;

  // Remove from the team tree
  teamsByName.Unlink(this);

  // Clear the filtering flag
  filtering = FALSE;

  // If we haven't postloaded then resolve the types before clearing
  if (!postLoaded)
  {
    // Post load the filtered types
    Resolver::TypeList<UnitObjType, UnitObjTypeListNode>(filteredTypes);
  }
  filteredTypes.Clear();

  // Deassociate any players who are associated with this team
  for (U32 i = 0; i < Game::MAX_PLAYERS; i++)
  {
    Player *player = Player::Id2Player(i);

    if (player && (this == player->GetTeam()))
    {
//      LOG_DEV(("Team '%s' /-/ Player '%s'", name.str, player->GetName()))
      player->ClearTeam();
    }
  }
  DeterminePlayers();

  // If this team is the display team then clear the display team
  if (this == displayTeam)
  {
    displayTeam = NULL;
  }

  // Unlink all of the relation lists
  for (i = 0; i < Relation::MAX; i++)
  {
    group[i].UnlinkAll();
  }
  groupEnemyHuman.UnlinkAll();

  // This only needs to be cleaned up if the mission is NOT postloaded
  relationsByTeamName.DisposeAll();

  // Empty construction list
  constructList.Clear();

  // Remove this team from all of the other teams relation groups
  for (U32 id = 0; id < Game::MAX_TEAMS; id++)
  {
    if (teamsById[id])
    {
      for (i = 0; i < Relation::MAX; i++)
      {
        teamsById[id]->group[i].Unlink(this);
      }
      teamsById[id]->groupEnemyHuman.Unlink(this);
    }
  }

  // Delete the dynamic data scope
  VarSys::DeleteItem(DynVarName());
}


//
// Team::Setup
//
void Team::Setup()
{
  // Clear the post load flag
  postLoaded = FALSE;

  // Set the team in the display objectives
  displayObjectives.SetTeam(this);

  // Add the team to the id array
  teamsById[id] = this;

  // Add the team to the tree
  teamsByName.Add(name.crc, this); 

  // Generate the console prefix
  consoleId = 0xEDF0E1CF; // "Team"
  consoleId ^= Crc::CalcStr(name.str);

  // Initialize the map object list
  unitObjects.SetNodeMember(&UnitObj::teamNode);

  // No command cost
  totalCommandCost = 0;

  // No unit members
  totalUnitMembers = 0;

  // Clear property counts
  propertyRadar = 0;
  propertyCloaking = 0;

  // Set the default team color
  color = Color(0l, 0l, 0l);

  // Set the default start location
  startPoint = Point<F32>((id + 1) * 0.1f, (id + 1) * 0.1f);

  // Reset the power levels
  powerLevel = 0;
  powerLevelUnfiltered = 0;

  // Reset start yaw
  startYaw = 0.0F;

  // Set the defult flags
  defaultClient = FALSE;
  availablePlay = TRUE;
  hasStats = TRUE;
  permanentRadar = FALSE;
  requireAI = FALSE;
  sideFixed = TRUE;

  // Setup some default values
  difficulty = Difficulty::GetDefaultSetting().GetValue();
  unitLimit = UnitLimits::GetLimit();
  side = Sides::GetSide().GetName().str;

  // Set the default personality name
  personality = "None";

  // By default teams are NOT controlled by strategic AI
  strategicObject = NULL;

  // Pain reduction
  painCurrentCluster = 0;

  // Reset script id
  uniqueScriptId = 0;

  // Set the resource storage and capacity
  resourceStore = 0;

  // Turn filtering off by default
  filtering = FALSE;

  // Initialize the Relations
  for (U32 i = 0; i < Game::MAX_TEAMS; i++)
  {
    relations[i] = (i == id) ? Relation::ALLY : Relation::NEUTRAL;

    // Not giving line of sight to anyone
    givingSightTo[i] = 0;

    // Dirty all relations
    Game::TeamSet(relationChanged, i);
  }

  // Clear all teams by relation arrays
  for (U32 r = 0; r < Relation::MAX; r++)
  {
    teamsByRelation[r] = 0;
  }

  // Set the id of the team radio
  radio.SetTeam(id);

  // Create vars
  VarSys::CreateString("@.engine.name", GetName(), VarSys::NOEDIT, NULL, this);
}


//
// Team::CurrentObjective
//
Objective::Object * Team::CurrentObjective(const char *obj)
{
  ASSERT(initialized)

  U32 objCrc = Crc::CalcStr(obj);

  for (Objective::ObjList::Iterator i(&currentObjectives); *i; i++)
  {
    if ((*i)->Alive())
    {
      if (objCrc == (**i)->GetType()->GetNameCrc())
      {
        return (**i);
      }
    }
  }

  return (NULL);
}


//
// Team::NewObjective
//
void Team::NewObjective(const char *obj, Bool required)
{
  ASSERT(initialized)

  // Check first to see that we don't already have an objective of this type
  Objective::Object *objective = CurrentObjective(obj);

  if (objective)
  {
    LOG_DIAG(("Duplicate Objective '%s' for '%s'", obj, GetName()));
  }
  else
  {
    // Ask the objective system for a new objective
    objective = Objective::Create(obj);

    if (objective)
    {
      #ifdef DEVELOPMENT
        CON_TEAM(("%5.1f New Objective '%s'", GameTime::SimTotalTime(), objective->GetType()->GetName()))
        LOG_DIAG(("%s: New Objective '%s'", GetName(), objective->GetType()->GetName()))
      #endif

      // Post load the objective
      objective->PostLoad();

      // Add the objectives to the current objective list
      currentObjectives.Append(objective);
    }
    else
    {
      if (required)
      {
        ERR_CONFIG(("Unknown Objective '%s' for team '%s'", obj, GetName())) 
      }
    }
  }
}


//
// Team::ObjectiveAchieved
//
void Team::ObjectiveAchieved(Objective::Object *objective)
{
  ASSERT(initialized)
  ASSERT(objective)

  #ifdef DEVELOPMENT
    CON_TEAM(("%5.1f Objective '%s' achieved", GameTime::SimTotalTime(), objective->GetType()->GetName()))
    LOG_DIAG(("%s: Objective '%s' achieved", GetName(), objective->GetType()->GetName()))
  #endif

  // Mark the objective for deletion
  GameObjCtrl::MarkForDeletion(objective);

  // Remove from the current objectives
  currentObjectives.Remove(objective);

  // Execute the action associated with this objective
  objective->Act(this);
}


//
// Team::ObjectiveAbandoned
//
void Team::ObjectiveAbandoned(Objective::Object *objective)
{
  ASSERT(initialized)

  if (!objective)
  {
    ERR_CONFIG(("Attempt to abandon an objective for team '%s' when the team did not currently have that objective", GetName()))
  }

  #ifdef DEVELOPMENT
    CON_TEAM(("%5.1f Objective '%s' abandoned", GameTime::SimTotalTime(), objective->GetType()->GetName()))
    LOG_DIAG(("%s: Objective '%s' abandoned", GetName(), objective->GetType()->GetName()))
  #endif

  // Mark the objective for deletion
  GameObjCtrl::MarkForDeletion(objective);
}


//
// Team::ClearObjectives
//
// Clear objectives
//
void Team::ClearObjectives()
{
  for (Objective::ObjList::Iterator i(&currentObjectives); *i; i++)
  {
    if ((*i)->Alive())
    {
      GameObjCtrl::MarkForDeletion(**i);
    }
  }

  currentObjectives.Clear();
}


//
// Team::PreLoad
//
void Team::PreLoad()
{
  // Resolve relations
  for (BinTree<Relation>::Iterator r(&relationsByTeamName); *r; r++)
  {
    // Find the team with the given name
    Team *team = teamsByName.Find(r.GetKey());

    if (team)
    {
      SetRelationPriv(team->GetId(), *(*r));
    }
  }

  // Don't need these anymore
  relationsByTeamName.DisposeAll();
}


//
// Team::PostLoad
//
void Team::PostLoad()
{
  // Set the post load flag
  postLoaded = TRUE;

  // Ensure mods are setup for this team
  User::SetupAddons(this);

  // Setup colors
  IFace::SetConsoleColor(GetConsoleId(0xEDF0E1CF), IFace::GetConsoleColor(0xEDF0E1CF)); // "Team"
  IFace::SetConsoleColor(GetConsoleId(0x0E39AE9E), IFace::GetConsoleColor(0x0E39AE9E)); // "Strategic::Script"
  IFace::SetConsoleColor(GetConsoleId(0x29853B05), IFace::GetConsoleColor(0x29853B05)); // "Strategic::Orderer"
  IFace::SetConsoleColor(GetConsoleId(0x622EF512), IFace::GetConsoleColor(0x622EF512)); // "Strategic"

  power.PostLoad();

  // Reset the power filter
  power.ResetFilter();
  powerLevel = power.GetLevel(power.GetAvailable());
  powerLevelUnfiltered = power.GetLevel(power.GetAvailableUnfiltered());

  // Post load the filtered types
  Resolver::TypeList<UnitObjType, UnitObjTypeListNode>(filteredTypes);

  // Post load the construction list
  Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(constructList);

  // Post Load this teams current objectives
  Resolver::ObjList<Objective::Object, Objective::Type, Objective::ListNode>(currentObjectives);

  // Display the list of objectives
  #ifdef DEVELOPMENT
    for (Objective::ObjList::Iterator o(&currentObjectives); *o; o++)
    {
      CON_TEAM(("%5.1f Initial Objective '%s'", GameTime::SimTotalTime(), (**o)->GetType()->GetName()))
      LOG_DIAG(("%s: Initial Objective '%s'", GetName(), (**o)->GetType()->GetName()))
    }
  #endif

  // Post Load the start location for this team
  Resolver::Object<RegionObj, RegionObjType>(startRegion);

  // If there is no player associated with this team then we look at the defaults
  DeterminePlayers();
  if (!HavePlayer())
  {
    if (defaultClient)
    {
      // No client player exists currently, create one
      // NOTE: The name of the player will be replaced with the name
      //       of the currently logged in user and will not be "Client"
      Player *player = new Player(User::GetName());
  
      // Set the players team to this and then give it to the client
      player->SetTeam(this);
      Player::SetCurrentPlayer(player);
    }
  }

  // Create an AI player for this team if we don't have a player at all or if we 
  if (!strategicObject && (!HavePlayer() || requireAI))
  {
    Strategic::Object *object = Strategic::Create(HavePlayer() ? 0 : personality.crc);
    object->SetTeam(this);
  }

  DeterminePlayers();

  // Check the status of all walls
  WallObjType::UpdatePowerStatus(this, Common::InStudio());
}


//
// Team::Save
//
void Team::Save(FScope *fScope)
{
  ASSERT(initialized)
  ASSERT(fScope)

  // Create the function
  FScope *sScope = fScope->AddFunction("CreateTeam");

  // Argument is team name and team id
  sScope->AddArgString(name.str);
  sScope->AddArgInteger(id);

  // Save the color
  FScope *ssScope;
  ssScope = sScope->AddFunction("Color");
  ssScope->AddArgInteger(color.R());
  ssScope->AddArgInteger(color.G());
  ssScope->AddArgInteger(color.B());

  // Save the relations
  ssScope = sScope->AddFunction("Relations");

  for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
  {
    FScope *sssScope = ssScope->AddFunction("With");
    sssScope->AddArgString((*t)->GetName());
    sssScope->AddArgString(relations[(*t)->GetId()].GetName());
  }

  // Save some stuff
  StdSave::TypeU32(sScope, "DefaultClient", defaultClient);
  StdSave::TypeU32(sScope, "AvailablePlay", availablePlay);
  StdSave::TypeU32(sScope, "HasStats", hasStats);
  StdSave::TypeU32(sScope, "PermanentRadar", permanentRadar);
  StdSave::TypeU32(sScope, "RequireAI", requireAI);
  StdSave::TypeString(sScope, "Side", side.str);
  StdSave::TypeU32(sScope, "SideFixed", sideFixed);
  StdSave::TypeReaperList(sScope, "Objectives", currentObjectives);

  // Save the start location
  StdSave::TypeReaper(sScope, "StartRegion", startRegion);
  StdSave::TypePoint<F32>(sScope, "StartPoint", startPoint);
  StdSave::TypeF32(sScope, "StartYaw", startYaw);

  // Save the personality
  StdSave::TypeString(sScope, "Personality", personality.str);

  // Pain reduction
  StdSave::TypeU32(sScope, "PainCurrentCluster", painCurrentCluster);

  // Script id
  StdSave::TypeU32(sScope, "UniqueScriptId", uniqueScriptId);

  // Save the filtered types
  if (filtering)
  {
    // Have to support old style missions :(
    StdSave::TypeU32(sScope, "FilteringEnabled", filtering);
    StdSave::TypeReaperListObjType(sScope, "FilteredTypes", filteredTypes);
  }

  // Save the resource store
  StdSave::TypeU32(sScope, "ResourceStore", resourceStore);

  // Save game data
  if (SaveGame::SaveActive())
  {
    // Save the difficulty setting
    StdSave::TypeF32(sScope, "Difficulty", difficulty);

    // Save the unit limit
    StdSave::TypeU32(sScope, "UnitLimit", unitLimit);

    // Find the dynamic data scope
    if (VarSys::VarScope *varScope = VarSys::FindVarScope(DynVarName()))
    {  
      VarFile::Save(sScope->AddFunction("DynData"), varScope);
    }

    // Save the stats
    endGame.SaveState(sScope->AddFunction("EndGame"));

    // Save the list of objects under construction
    StdSave::TypeReaperList(sScope, "ConstructList", constructList);

    // Save the display objectives
    displayObjectives.SaveState(sScope->AddFunction("DisplayObjectives"));

    // Save which teams we're giving sight too
    for (U32 i = 0; i < Game::MAX_TEAMS; i++)
    {
      if (givingSightTo[i])
      {
        StdSave::TypeU32(sScope, "GivingSightTo", i)->AddArgInteger(S32(givingSightTo[i]));
      }
    }

    // Save power leaks
    power.SaveState(sScope->AddFunction("Power"));
  }
}


//
// Team::ProcessSight
//
void Team::ProcessSight(Bool force)
{
  ASSERT(initialized)

  // Is it our turn to do scheduled processing?
  if (Bool distribute = (force) || ((GameTime::SimCycle() & (Game::MAX_TEAMS-1)) == GetId()))
  {
    U8 *dirtyClust = Sight::GetDirtyClusterMap(this);

    // Round x clusters up to the nearest 8 for the bit array
    U32 mapClustXon8 = ((WorldCtrl::ClusterMapX() + 7) & (~7)) >> 3;

    for (U32 z = 0, index = 0; z < WorldCtrl::ClusterMapZ(); z++)
    {
      for (U32 x = 0; x < mapClustXon8; x++, index++)
      {
        for (U8 byte = dirtyClust[index], byteIndex = 0; byte; byte >>= 1, byteIndex++)
        {
          if (byte & 1)
          {
            // We have a dirty cluster, update the visibility of units/resources in the cluster
            MapCluster * cluster = WorldCtrl::GetCluster((x << 3) + byteIndex, z);

            // Iterate over the units in this cluster and update the seeing bitfields
            for (NList<UnitObj>::Iterator u(&cluster->unitList); *u; u++)
            {
              UnitObj *unit = *u;

              if (unit->GetVisible(this))
              {
                if (!unit->TestCanSee(GetId()))
                {
                  unit->SetCanSee(GetId());
                }
              }
              else
              {
                if (unit->TestCanSee(GetId()))
                {
                  unit->ClearCanSee(GetId());
                }
              }
            }

            // Iterate over the resources in this cluster and update the seeing bitfields
            for (NList<ResourceObj>::Iterator r(&cluster->resourceList); *r; r++)
            {
              ResourceObj *resource = *r;

              if (resource->GetVisible(this))
              {
                if (!resource->TestCanSee(GetId()))
                {
                  resource->SetCanSee(GetId());
                }
              }
              else
              {
                if (resource->TestCanSee(GetId()))
                {
                  resource->ClearCanSee(GetId());
                }
              }
            }
          }        
        }

        // Undirty the 8 clusters
        dirtyClust[index] = 0;
      }
    }
  }
}


//
// Team::Process
//
void Team::Process()
{
  ASSERT(initialized)

  // Some heavy duty syncing
  SYNC
  (
    id << 
    totalCommandCost << 
    totalUnitMembers << 
    propertyRadar << 
    propertyCloaking << 
    resourceStore <<
    constructList.GetCount() <<
    filtering
  )

  // Test the current objective list
  Objective::ObjList::Iterator i(&currentObjectives);
  Objective::ListNode *node;

  // Objectives will have a tendancy of modifying the current objective list
  // Adding objectives and abandoning objectives.  For this reason, objectives
  // should not be set up in such a way as to go off on the same game cycle if
  // they modify each other
  while ((node = i++) != NULL)
  {
    if (node->Alive())
    {
      if ((*node)->Test(this))
      {
        ObjectiveAchieved(*node);
      }
    }
    else
    {
      currentObjectives.Dispose(node);
    }
  }

  // Update power filters
  power.Process();

  // See if the unfiltered power level changed
  U32 powerLevelNewUnfiltered = power.GetLevel(power.GetAvailableUnfiltered());
  if (powerLevelUnfiltered != powerLevelNewUnfiltered)
  {
    powerLevelUnfiltered = powerLevelNewUnfiltered;

    // Using the name of the power level build the name of the game message
    char event[MAX_GAMEIDENT + 8] = "Power::";
    Utils::Strcat(event, Power::GetLevelName(powerLevelUnfiltered));
    GetRadio().Trigger(Crc::CalcStr(event), NULL);
  }

  // See if the filtered power level changed
  U32 powerLevelNew = power.GetLevel(power.GetAvailable());
  if (powerLevel != powerLevelNew)
  {
    powerLevel = powerLevelNew;

    // Check the status of all walls
    WallObjType::UpdatePowerStatus(this, Common::InStudio());
  }

  // Check if relations with any other team changed
  if (relationChanged)
  {
    Game::TeamBitfield bitMask = relationChanged;

    // Clear array of team bit fields by relation
    for (U32 r = 0; r < Relation::MAX; r++)
    {
      teamsByRelation[r] = 0;
    }

    for (U32 t = 0; bitMask; ++t, bitMask >>= 1)
    {
      if (bitMask & 1)
      {
        Team *other = Id2Team(t);

        if (other)
        {
          RelationsChanged(other);
        }
      }
    }

    // Rebuild the teams by relation array
    for (t = 0; t < Game::MAX_TEAMS; t++)
    {
      if (Id2Team(t))
      {
        // Set the bit in the appropriate array by relation
        Game::TeamSet(teamsByRelation[GetRelation(t)], t);
      }
    }

    relationChanged = 0;
  }

  // Perform pain reduction on the cluster map

  // We want to reduce the pain on the map at 10% per 2 minutes 
  // (10% per 120 cycles) so that after 20 minutes all pain 
  // would be disipated

  // To acheive this goal we must visit every cluster once every 120 
  // cycles.  We calculate the number of clusters we should have covered 
  // at this time and then cover them.

  // Setup the number of clusters to process each cycle
  U32 numClusters = Clamp<U32>(1, WorldCtrl::ClusterCount() / 120, 100);
  while (numClusters--)
  {
    // Reduce pain in this cluster
    WorldCtrl::GetCluster(painCurrentCluster++)->ai.ReducePain(GetId(), 25);

    // Have we wrapped yet ?
    if (painCurrentCluster == WorldCtrl::ClusterCount())
    {
      painCurrentCluster = 0;
    }
  }

}


//
// Team::DeterminePlayers
//
void Team::DeterminePlayers()
{
  players.UnlinkAll();

  for (U32 i = 0; i < Game::MAX_PLAYERS; i++)
  {
    Player *player = Player::Id2Player(i);
    if (player && (this == player->GetTeam()))
    {
      players.Append(player);
    }
  }
}


//
// Team::HavePlayer
//
Bool Team::HavePlayer()
{
  DeterminePlayers();
  return (players.GetCount() ? TRUE : FALSE);
}


//
// Team::GetPlayers
//
const List<Player> & Team::GetPlayers()
{
  return (players);
}


//
// GetTeamLeader
//
// Get the player considered as the team leader
//
Player * Team::GetTeamLeader()
{
  Player *leader = NULL;

  for (List<Player>::Iterator i(&GetPlayers()); *i; i++)
  {
    if (!leader || ((*i)->GetType() != Player::AI))
    {
      leader = *i;
    }
  }

  return (leader);
}


//
// IsAI
//
// Is this an AI team
//
Bool Team::IsAI()
{
  // Make sure the player list is up to date
  DeterminePlayers();

  for (List<Player>::Iterator i(&GetPlayers()); *i; i++)
  {
    if ((*i)->GetType() != Player::AI)
    {
      return (FALSE);
    }
  }

  return (TRUE);
}


//
// IsHuman
//
// Is this an human team
//
Bool Team::IsHuman()
{
  // Make sure the player list is up to date
  DeterminePlayers();

  for (List<Player>::Iterator i(&GetPlayers()); *i; i++)
  {
    if ((*i)->GetType() == Player::AI)
    {
      return (FALSE);
    }
  }

  return (TRUE);
}


//
// Team::MoveToStart
//
void Team::MoveToStart()
{
  F32 x, z;

  if (startRegion.Alive())
  {
    // Middle of region
    x = startRegion->GetMidPoint().x;
    z = startRegion->GetMidPoint().z;
  }
  else
  {
    // Middle of map
    x = WorldCtrl::MetreMapX() * 0.5F;
    z = WorldCtrl::MetreMapZ() * 0.5F;
  }

  // Build quaternion and vector
  Quaternion quat;
  Vector pos;

  Viewer::GetCurrent()->GenerateLookAt(x, z, pos, quat, startYaw * DEG2RAD, Viewer::GetDefaultHeight());
  Viewer::GetCurrent()->Set(pos, quat);
}


//
// AddMember
//
// Add the given unit as a MEMBER of this team
//
void Team::AddMember(UnitObj *obj)
{
  ASSERT(obj)

  // Increment the number of unit members
  totalUnitMembers++;

  // Add the command cost
  totalCommandCost += obj->UnitType()->GetCommandCost();
}


//
// RemoveMember
//
// Remove the given unit as a MEMBER of this team
// 
void Team::RemoveMember(UnitObj *obj)
{
  ASSERT(obj)
  ASSERT(totalUnitMembers)
  ASSERT(totalCommandCost >= obj->UnitType()->GetCommandCost())

  // Decrement the number of unit members
  totalUnitMembers--;

  // Remove the command cost
  totalCommandCost -= obj->UnitType()->GetCommandCost();
}


//
// Team::AddToTeam
//
void Team::AddToTeam(UnitObj *obj)
{
  ASSERT(obj)
  
  // Ensure the given unit belongs to this team
  if (obj->GetTeam() != this)
  {
    ERR_FATAL(("Attempt to add [%s] to [%s] but team is [%s]", obj->TypeName(), GetName(), obj->GetTeam() ? obj->GetTeam()->GetName() : "None"));
  }

  // Ensure the given unit is not already actively on a team
  if (obj->teamNode.InUse())
  {
    MapObj *p = obj->GetParent();

    LOG_DIAG(("Current Team [%s]", obj->GetTeam() ? obj->GetTeam()->GetName() : "None"));
    LOG_DIAG(("Parent [%s][%d]", p ? p->TypeName() : "None", p ? p->Id() : 0));
    LOG_DIAG(("Children [%d]", obj->GetAttachments().GetCount()));

    for (NList<MapObj>::Iterator i(&obj->GetAttachments()); *i; i++)
    {
      if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*i))
      {
        LOG_DIAG((" - Child [%s][%d]", unit->TypeName(), unit->Id()));
      }
    } 

    ERR_FATAL(("Adding [%s][%d] to [%s] when already on team", obj->TypeName(), obj->Id(), GetName()));
  }

  // Add to all units list
  unitObjects.Append(obj);

  // Check for properties
  if (obj->HasProperty(0x49E109AB)) // "Provide::Radar"
  {
    propertyRadar++;
  }

  if (obj->HasProperty(0xA28E1761)) // "Provide::Cloaking"
  {
    propertyCloaking++;
  }

  // Find/Create type specific list and add to it
  NList<UnitObj> *list = unitObjectsByType.Find(obj->GameType()->GetNameCrc());
  if (!list)
  {
    list = new NList<UnitObj>(&UnitObj::teamTypeNode);
    unitObjectsByType.Add(obj->GameType()->GetNameCrc(), list);
  }
  list->Append(obj);


  // Add this units power
  power.AddUnit(obj->UnitType()->GetPower());

  // If this team is under the control of a strategic AI then give the unit to the strategic AI
  if (strategicObject)
  {
    strategicObject->GiveUnit(obj);
  }

  // Notify the player
  NotifyPlayer(obj, 0x6D9FC394); // "Team::AddUnit"

  // Add all attachments to the team
  for (NList<MapObj>::Iterator i(&obj->GetAttachments()); *i; i++)
  {
    if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*i))
    {
      // Only add if on this team, but not actively (ignore parasites)
      if ((unit->GetTeam() == this) && !unit->teamNode.InUse())
      {
        AddToTeam(unit);
      }
    }
  } 
}


//
// Team::RemoveFromTeam
//
void Team::RemoveFromTeam(UnitObj *obj)
{
  ASSERT(obj)

  if (!obj->teamNode.InUse())
  {
    return;
  }

  // Remove from all units list
  unitObjects.Unlink(obj);

  // Check for properties
  if (obj->HasProperty(0x49E109AB)) // "Provide::Radar"
  {
    ASSERT(propertyRadar)  
    propertyRadar--;
  }
  if (obj->HasProperty(0xA28E1761)) // "Provide::Cloaking"
  {
    ASSERT(propertyCloaking)  
    propertyCloaking--;
  }

  // Remove from type specific list
  if (NList<UnitObj> *list = unitObjectsByType.Find(obj->GameType()->GetNameCrc()))
  {
    list->Unlink(obj);

    // If the list is empty can it
    if (!list->GetCount())
    {
      unitObjectsByType.Dispose(obj->GameType()->GetNameCrc());
    }
  }
  else
  {
    ERR_FATAL(("Unable to find type list for [%s][%d]", obj->TypeName(), obj->Id()));
  }

  // Remove this units power
  power.RemoveUnit(obj->UnitType()->GetPower());

  // Remove all attachments from the team
  for (NList<MapObj>::Iterator i(&obj->GetAttachments()); *i; i++)
  {
    if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*i))
    {
      // Only remove if on this team
      if ((unit->GetTeam() == this) && unit->teamNode.InUse())
      {
        RemoveFromTeam(unit);
      }
    }
  } 
}


//
// Team::ReportConstruction
//
void Team::ReportConstruction(MapObj *mapObj)
{
  if (!mapObj->MapType()->HasProperty(0xBE0A35C9)) // "Team::NoStats"
  {
    const char *typeName = mapObj->TypeName();
    endGame.constructed.Sample(typeName);
  }
}


//
// Team::ReportKill
//
void Team::ReportKill(MapObj *mapObj)
{
  const char *typeName = mapObj->TypeName();
  UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(mapObj);
  Team *team;

  if (!mapObj->MapType()->HasProperty(0xBE0A35C9)) // "Team::NoStats"
  {
    if (unitObj && (team = unitObj->GetTeam()) != NULL)
    {
      // Did we kill one of our own ?
      if (team == this)
      {
        endGame.killsSelf.Sample(typeName);
      }
      else
      {
        // What's our relation to the given team ?
        endGame.kills[GetRelation(team->GetId())]->Sample(typeName);
      }
    }
    else
    {
      // We probably killed something poor and defenseless
      endGame.killsMisc.Sample(typeName);
    }
  }
}


//
// Team::ReportLoss
//
void Team::ReportLoss(UnitObj *unitObj)
{
  if (!unitObj->MapType()->HasProperty(0xBE0A35C9)) // "Team::NoStats"
  {
    endGame.losses.Sample(unitObj->TypeName());
  }
}


//
// Team::ReportResource
//
void Team::ReportResource(U32 amount, const char *reason)
{
  endGame.resources.Sample(reason, amount);
}


//
// Team::ClearResource
//
void Team::ClearResource(const char *reason)
{
  endGame.resources.Clear(reason);
}


//
// Team::AddResourceStore
//
// Increase the amount of resource stored
// Amount returned is overflow (if any)
//
void Team::AddResourceStore(U32 mod)
{
  ASSERT(mod >= 0)

  if (IsAI())
  {
    mod = Utils::FtoLNearest(GetDifficulty() * F32(mod));
  }

  resourceStore += mod;

  // Notify the player that there was an increase in resource
  NotifyPlayer(NULL, 0xD45DC3A0); // "Team::AddResource"

  // Generate a game message
  if (mod && (GetDisplayTeam() == this))
  {
    // "Team::AddResourceStore"
    Message::TriggerGameMessage(0x947E3558);
  }
}


//
// Team::RemoveResourceStore
//
// Decrease the amount of resource stored
//
Bool Team::RemoveResourceStore(U32 mod)
{
  ASSERT(mod >= 0)

  if (mod > resourceStore)
  {
    return (FALSE);
  }

  resourceStore -= mod;

  // Generate a game message
  if (GetDisplayTeam() == this)
  {
    // "Team::RemoveResourceStore"
    Message::TriggerGameMessage(0x10DC9518);
  }

  return (TRUE);
}


//
// Set the amont of resource stored
//
void Team::SetResourceStore(U32 resource)
{
  resourceStore = resource;
  ClearResource("resource.initial");
  ReportResource(resourceStore, "resource.initial");
}


//
// Notify the player(s) which control this team
//
Bool Team::NotifyPlayer(GameObj *from, U32 message, U32 param1, U32 param2)
{
  Bool succeeded = FALSE;

  for (List<Player>::Iterator p(&players); *p; p++)
  {
    if ((*p)->Notify(from, message, param1, param2))
    {
      succeeded = TRUE;
    }
  }

  return (succeeded);
}


//
// Build the dyn var name for a team
//
const char * Team::DynVarName()
{
  static char buf[VARSYS_MAXVARPATH];
  const char *p;
  char *s = buf;

  // Copy scope name
  p = varScope;
  while (*p)
  {
    *s++ = *p++;
  }
  *s++ = VARSYS_SCOPEDELIM;

  // Copy the team name
  p = name.str;
  while (*p)
  {
    *s++ = *p++;
  }

  // Null terminate
  *s = '\0';

  return (buf);
}


//
// Is the given type allowed by the filter ?
//
Bool Team::AllowedType(UnitObjType *type)
{
  // Is this type globally filtered?
  if (type->IsFiltered())
  {
    return (FALSE);
  }

  // Are we filtering ?
  if (filtering)
  {
    // We are, is this an allowed type?
    return (filteredTypes.Exists(type));
  }
  else
  {
    return (TRUE);
  }
}


//
// RadarAvailable
//
// Is radar currently available for this team
//
Bool Team::RadarAvailable()
{
  return (permanentRadar || (propertyRadar && GetPower().GetLevel(power.GetAvailable()) + 1 == Power::NumLevels));
}


//
// CloakingAvailable
//
// Is cloaking currently available for this team
//
Bool Team::CloakingAvailable()
{
  return (propertyCloaking ? TRUE : FALSE);
}


//
// GetUnitLimitBalance
//
// Returns the unit limit balance
//
S32 Team::GetUnitLimitBalance()
{
  return (GetUnitLimit() - GetTotalCommandCost());
}


//
// UnitLimitReached
//
// Returns TRUE if the unit limit has been reached
//
Bool Team::UnitLimitReached(UnitObjType *type, UnitObjType *remove)
{
  // Get the number of command points available
  S32 available = GetUnitLimitBalance();

  // Add in the cost of the constructor if it will be consumed
  if (remove && type && type->GetConstructorConsume())
  {
    available += remove->GetCommandCost();
  }

  // Limited if available is less than required
  return (available < (type ? S32(type->GetCommandCost()) : 1));
}


//
// RegisterConstructionBegin
//
// Register that a unit has started construction for this team
//
void Team::RegisterConstructionBegin(UnitObj *unit)
{
  constructList.AppendNoDup(unit);
}


//
// RegisterConstructionEnd
//
// Register that a unit has stopped construction for this team
//
void Team::RegisterConstructionEnd(UnitObj *unit)
{
  UnitObjListNode *node = constructList.Find(unit->Id());

  if (node)
  {
    constructList.Dispose(node);
  }
}


//
// IsConstructing
//
// Test if a unit type is being constructed for this team
//
Bool Team::IsConstructingType(UnitObjType *type)
{
  for (UnitObjList::Iterator i(&constructList); *i; i++)
  {
    if (UnitObj *unit = (*i)->GetPointer())
    {
      if ((type == unit->UnitType()) && !unit->IsActivelyOnTeam())
      {
        // Matching unit type, and finished constructing
        return (TRUE);
      }
    }
  }

  return (FALSE);
}


//
// GiveSightTo
//
// Add a reference to provide team with line of sight
//
void Team::GiveSightTo(U32 teamId)
{
  ASSERT(teamId < Game::MAX_TEAMS)

  givingSightTo[teamId]++;

  // Force sight update
  Game::TeamSet(relationChanged, teamId);
}


//
// UngiveSightTo
//
// Remove a reference to provide team with line of sight
//
void Team::UngiveSightTo(U32 teamId)
{
  ASSERT(teamId < Game::MAX_TEAMS)
  ASSERT(givingSightTo[teamId] > 0)

  givingSightTo[teamId]--;

  // Force sight update
  Game::TeamSet(relationChanged, teamId);
}


//
// GiveUnits
//
// Give units to a team (from us)
//
U32 Team::GiveUnits(Team *to, const UnitObjList &units, UnitObjPtr &first)
{
  ASSERT(to)

  U32 count = 0;

  for (UnitObjList::Iterator i(&units); *i; i++)
  {
    UnitObj *u = **i;

    // Check that we own the unit
    if (u && u->GetTeam() && (u->GetTeam()->GetId() == GetId()))
    {
      // Can't give if unit exceeds the destination team's command points remaining
      if (to->GetUnitLimitBalance() >= S32(u->UnitType()->GetCommandCost()))
      {
        // Can't give if unit is busy
        if (!u->Blocked())
        {
          // Stop active task
          u->Stop();

          // Remove from squad
          if (u->GetSquad())
          {
            u->ClearSquad();
          }

          // Give to the team
          u->SetTeam(to);
          count++;

          // Setup reaper to first given unit
          if (first.Dead())
          {
            first = u;
          }
        }
      }
    }
  }

  return (count);
}


//
// GiveMoney
//
// Give money to a team (from us)
//
U32 Team::GiveMoney(Team *to, U32 amount)
{
  ASSERT(to)

  if ((amount = Clamp<U32>(0, amount, GetResourceStore())) != 0)
  {
    RemoveResourceStore(amount);
    ReportResource(amount, "resource.gifts.given");
    to->AddResourceStore(amount);
    to->ReportResource(amount, "resource.gifts.received");
  }

  return (amount);
}


//
// SetStartRegion
//
// Set the start region
//
void Team::SetStartRegion(RegionObj *region)
{
  if (region)
  {
    startRegion = region;
    startPoint.x = Clamp(0.0f, region->GetMidPoint().x / WorldCtrl::MetreMapX(), 1.0f);
    startPoint.y = Clamp(0.0f, 1.0f - (region->GetMidPoint().z / WorldCtrl::MetreMapZ()), 1.0f);
  }
  else
  {
    startRegion = NULL;
    startPoint = Point<F32>((id + 1) * 0.1f, (id + 1) * 0.1f);
  }
}


//
// SetStartYaw
//
// Set the start yaw
//
void Team::SetStartYaw(F32 yaw)
{
  startYaw = yaw;
}


//
// GetUnitRelation
//
// Get this teams relationship with the given unit
//
Relation Team::GetUnitRelation(UnitObj *unit)
{
  ASSERT(unit)

  // Get the actual relation
  Relation r = unit->GetTeam() ? GetRelation(unit->GetTeam()->GetId()) : Relation::NEUTRAL;

  // Is this a clandestine enemy
  if ((r == Relation::ENEMY) && unit->GetFlag(UnitObj::FLAG_CLANDESTINE))
  {
    // Is this a spy
    if (SpyObj *spy = Promote::Object<SpyObjType, SpyObj>(unit))
    {
      // Is it morphed
      if (Team *team = spy->GetMorphTeam())
      {
        // Set the relation as we see it
        r = GetRelation(team->GetId());

        // But do not allow enemies to see us as an ally
        if (r == Relation::ALLY)
        {
          r = Relation::NEUTRAL;
        }
      }
    }
    else
    {
      // Make it appear as a neutral unit
      r = Relation::NEUTRAL;
    }
  }

  return (r);
}


//
// TestUnitRelation
//
// Return true if the given relation matches
//
Bool Team::TestUnitRelation(UnitObj *unit, Relation relation)
{
  ASSERT(unit)
  ASSERT(relation < Relation::MAX)

  return (GetUnitRelation(unit) == relation);
}



//
// Return true if this team satisfies enemy human relationship
//
Bool Team::TestEnemyHumanRelation(U32 teamId)
{
  ASSERT(teamId < Game::MAX_TEAMS)
  ASSERT(teamsById[teamId])

  // Firstly, we have to be enemies
  if (TestRelation(teamId, Relation::ENEMY))
  {
    // Are we AI ?
    if (IsAI())
    {
      // Are they AI ?
      if (teamsById[teamId]->IsAI())
      {
        // AIs don't pass the enemy human test
        return (FALSE);
      }
      else
      {
        // If we're AI and there not, they pass
        return (TRUE);
      }
    }
    else
    {
      // Humans are enemies with everyone
      return (TRUE);
    }
  }
  else
  {
    // We're friends
    return (FALSE);
  }
}


//
// GetRelationColor
//
// Return color for the relationship with the given team
//
Color Team::GetRelationColor(Team *team)
{
  switch (Team::GetRelation(this, team))
  {
    case Relation::ALLY:
      return (Color(0.0F, 1.0F, 0.0F));
    
    case Relation::ENEMY:
      return (Color(1.0F, 0.0F, 0.0F));

    default:
      return (Color(0.0F, 1.0F, 1.0F));
  }
}


//
// Team::SetRelationPriv
//
void Team::SetRelationPriv(U32 teamId, Relation relation)
{
  ASSERT(relation < Relation::MAX)
  ASSERT(teamId < Game::MAX_TEAMS)
  ASSERT(teamsById[teamId])

  // Set the relation
  relations[teamId] = relation;

  // Add the team to the list
  group[relation].Append(teamsById[teamId]);

  // If we're setting ENEMY check the enemy human test
  if (relation == Relation::ENEMY && TestEnemyHumanRelation(teamId))
  {
    groupEnemyHuman.Append(teamsById[teamId]);
  }

  // Update changed relation
  Game::TeamSet(relationChanged, teamId);

  // Update inverse relation
  Game::TeamSet(teamsById[teamId]->relationChanged, id);
}


//
// Team::ClearRelationPriv
//
void Team::ClearRelationPriv(U32 teamId)
{
  ASSERT(relations[teamId] < Relation::MAX)

  // Remove from the list
  group[relations[teamId]].Unlink(teamsById[teamId]);

  // If we're clearing ENEMY check the enemy human test
  if (relations[teamId] == Relation::ENEMY && TestEnemyHumanRelation(teamId))
  {
    groupEnemyHuman.Unlink(teamsById[teamId]);
  }
}


//
// Team::RelationsChanged
//
// Notification that relations changed with another team
//
void Team::RelationsChanged(Team *team)
{
  ASSERT(team)

  //LOG_DIAG(("[%s] Relations with team [%s] changed", GetName(), team->GetName()));

  // Rescan line of sight for both teams
  // NOTE: RescanAll will not rescan a team more than once in a game cycle
  Sight::RescanAll(this);
  Sight::RescanAll(team);
}


//
// Team::Init
//
void Team::Init()
{
  ASSERT(!initialized)

  // Register the var substitution for teams
  VarSys::RegisterSubstitution(substitution);

  // Initialized team radio
  Radio::Init();

  // Clear the current display team
  displayTeam = NULL;

  // Create mono buffers
  MonoBufCreate("Teams", &monoBuffer);

  // Write the titles onto the mono
  MonoBufWrite(monoBuffer, 0, 0, "Id Name                 Relations A N E #Obj   Threat  Defense  Storage    Power", Mono::BRIGHT);
//                                0  01234567890123456789 01234567  0 0 0 0000 01234567 01234567 01234567 01234567

  // Ensure used Ids are clear
  memset(teamsById, 0x00, sizeof (teamsById));

  // Define team commands
  VarSys::RegisterHandler("team", CmdHandler);
  VarSys::RegisterHandler("team.config", CmdHandler);
  VarSys::RegisterHandler("team.mono", CmdHandler);
  VarSys::RegisterHandler("team.objectives", CmdHandler);
  VarSys::RegisterHandler("team.displayobjectives", CmdHandler);

#ifdef DEVELOPMENT

  // Some team commands
  VarSys::CreateCmd("team.list");

#endif

  // Relation comands
  VarSys::CreateCmd("team.setrelation");

  // Objective commands
  VarSys::CreateCmd("team.objectives.current");
  VarSys::CreateCmd("team.objectives.list");
  VarSys::CreateCmd("team.objectives.new");
  VarSys::CreateCmd("team.objectives.complete");
  VarSys::CreateCmd("team.objectives.abandon");

  // Display objectives
  VarSys::CreateCmd("team.displayobjectives.list");
  VarSys::CreateCmd("team.displayobjectives.remove");
  VarSys::CreateCmd("team.displayobjectives.complete");
  VarSys::CreateCmd("team.displayobjectives.abandon");

  // Unit commands
  VarSys::CreateCmd("team.listunits");


  // Set the initialized flag
  initialized = TRUE;
}


//
// Team::Done
//
void Team::Done()
{
  ASSERT(initialized)

  // Delete Teams
  for (int i = 0; i < Game::MAX_TEAMS; i++)
  {
    if (teamsById[i])
    {
      delete teamsById[i];
      teamsById[i] = NULL;
    }
  }

  // Delete team scope
  VarSys::DeleteItem("team");

  // Delete mono buffer
  MonoBufDestroy(&monoBuffer);

  // Shutdown team radio
  Radio::Done();

  // Unregister the var substitution for teams
  VarSys::UnregisterSubstitution(substitution);

  // Clear the initialied flag
  initialized = FALSE;
}


//
// Team::CmdHandler
//
void Team::CmdHandler(U32 pathCrc)
{
  static Team * curTeam;

  ASSERT(initialized)

  switch (pathCrc)
  {
    case 0xD07A7672: // "team.list"
    {
      NBinTree<Team>::Iterator i(&teamsByName);

      CON_DIAG(("Current Teams", (*i)->GetName))
      for (!i; *i; i++)
      {
        CON_DIAG((" - %s", (*i)->GetName()))
      }
      break;
    }

    case 0xD317A0F3: // "team.setrelation"
    {
      char *teamName;
      char *teamWithName;
      char *relation;

      if (
        !Console::GetArgString(1, teamName) || 
        !Console::GetArgString(2, teamWithName) ||
        !Console::GetArgString(3, relation))
      {
        CON_ERR(("team.setrelation teamname teamwith { ALLY, NEUTRAL, ENEMY }"))
      }
      else
      {
        Team *team = Name2Team(teamName);

        if (team)
        {
          Team *teamWith = Name2Team(teamWithName);

          if (teamWith)
          {
            SYNC("SetRelation")

            switch (Crc::CalcStr(relation))
            {
              case 0xF701DCEA: // "ALLY"
                team->SetRelation(teamWith->GetId(), Relation::ALLY);
                break;

              case 0x1FDC3051: // "NEUTRAL"
                team->SetRelation(teamWith->GetId(), Relation::NEUTRAL);
                break;

              case 0x232F33F0: // "ENEMY"
                team->SetRelation(teamWith->GetId(), Relation::ENEMY);
                break;

              default:
                CON_ERR(("Unknown relation type '%s'", relation))
                break;
            }
          }
          else
          {
            CON_ERR(("Unknown team '%s'", teamWithName))
          }
        }
        else
        {
          CON_ERR(("Unknown team '%s'", teamWithName))
        }
      }
      break;
    }

    case 0xFEDF0DB1: // "team.objectives.list"
    {
      CON_DIAG(("All possible objectives"))
      for (NBinTree<Objective::Type>::Iterator o(&Objective::GetTypes()); *o; o++)
      {
        CON_DIAG((" - %s", (*o)->GetName()))
      }
      break;
    }

    case 0xB7EE2847: // "team.objectives.current"
    {
      char *name;
      if (!Console::GetArgString(1, name))
      {
        CON_ERR(("team.objectives.current teamname"))
      }
      else
      {
        Team *team = Name2Team(name);
        if (team)
        {
          Objective::ObjList::Iterator i(&team->currentObjectives);

          CON_DIAG(("Current Objectives for team '%s'", name))
          for (!i; *i; i++)
          {
            CON_DIAG((" - %s", (**i)->GetType()->GetName()))
          }
        }
        else
        {
          CON_ERR(("Could not find team '%s'", name))
        }
      }
      break;
    }

    case 0x378674C4: // "team.objectives.new"
    {
      char *name;
      char *obj;
      if (!Console::GetArgString(1, name) || !Console::GetArgString(2, obj))
      {
        CON_ERR(("team.objectives.new teamname objective"))
      }
      else
      {
        Team *team = Name2Team(name);

        if (team)
        {
          if (Objective::CheckType(obj))
          {
            // Attempt to create an objective of that type
            team->NewObjective(obj);
          }
          else
          {
            S32 flag;
            if (!Console::GetArgInteger(3, flag))
            {
              // Objective name is invalid
              CON_ERR(("No objective named '%s'", obj))
            }
          }
        }
        else
        {
          CON_ERR(("Could not find team '%s'", name))
        }
      }
      break;
    }

    case 0xD9CBF44D: // "team.objectives.complete"
    {
      char *name;
      char *obj;
      if (!Console::GetArgString(1, name) || !Console::GetArgString(2, obj))
      {
        CON_ERR(("team.objectives.complete teamname objective"))
      }
      else
      {
        Team *team = Name2Team(name);
        if (team)
        {
          if (team->CurrentObjective(obj))
          {
            team->ObjectiveAchieved(team->CurrentObjective(obj));
          }
          else
          {
            CON_ERR(("Could not find objective '%s'", obj))
          }
        }
        else
        {
          CON_ERR(("Could not find team '%s'", name))
        }
      }
      break;
    }

    case 0x444D2FF0: // "team.objectives.abandon"
    {
      char *name;
      char *obj;
      if (!Console::GetArgString(1, name) || !Console::GetArgString(2, obj))
      {
        CON_ERR(("team.objectives.abandon teamname objective"))
      }
      else
      {
        Team *team = Name2Team(name);
        if (team)
        {
          if (team->CurrentObjective(obj))
          {
            team->ObjectiveAbandoned(team->CurrentObjective(obj));
          }
          else
          {
            CON_ERR(("Could not find objective '%s'", obj))
          }
        }
        else
        {
          CON_ERR(("Could not find team '%s'", name))
        }
      }
      break;
    }

    case 0xAF8C39AF: // "team.displayobjectives.list"
    {
      char *name;
      if (!Console::GetArgString(1, name))
      {
        CON_ERR(("team.displayobjectives.list teamname"))
      }
      else
      {
        Team *team = Name2Team(name);
        if (team)
        {
          //
          DisplayObjective::Set::Iterator iter;

          team->displayObjectives.ResetIterator(iter);

          for (;;)
          {
            const CH *text;
            DisplayObjective::Item::State state;
            GameIdent ident;

            if (team->displayObjectives.Iterate(iter, ident, text, state))
            {
              CON_DIAG(("%s : %d", ident.str, state))
              CON_DIAG((text))
            }
            else
            {
              break;
            }
          }
        }
        else
        {
          CON_ERR(("Could not find team '%s'", name))
        }
      }
      break;
    }

    case 0x2196626B: // "team.displayobjectives.remove"
    {
      char *name;
      char *obj;
      if (!Console::GetArgString(1, name) || !Console::GetArgString(2, obj))
      {
        CON_ERR(("team.displayobjectives.remove teamname displayobjective"))
      }
      else
      {
        Team *team = Name2Team(name);
        if (team)
        {
          team->displayObjectives.RemoveItem(obj);
        }
        else
        {
          CON_ERR(("Could not find team '%s'", name))
        }
      }
      break;
    }

    case 0x4F2038D3: // "team.displayobjectives.complete"
    {
      char *name;
      char *obj;
      if (!Console::GetArgString(1, name) || !Console::GetArgString(2, obj))
      {
        CON_ERR(("team.displayobjectives.complete teamname displayobjective"))
      }
      else
      {
        Team *team = Name2Team(name);
        if (team)
        {
          team->displayObjectives.CompleteItem(obj);
        }
        else
        {
          CON_ERR(("Could not find team '%s'", name))
        }
      }
      break;
    }

    case 0xD4FED907: // "team.displayobjectives.abandon"
    {
      char *name;
      char *obj;
      if (!Console::GetArgString(1, name) || !Console::GetArgString(2, obj))
      {
        CON_ERR(("team.displayobjectives.abandon teamname displayobjective"))
      }
      else
      {
        Team *team = Name2Team(name);
        if (team)
        {
          team->displayObjectives.AbandonItem(obj);
        }
        else
        {
          CON_ERR(("Could not find team '%s'", name))
        }
      }
      break;
    }

    case 0x4A63A99D: // "team.listunits"
    {
      char *name;
      char *type;

      if (!Console::GetArgString(1, name))
      {
        CON_ERR(("team.listunits teamname [type]"))
      }
      else
      {
        Team *team = Name2Team(name);
        if (!team)
        {
          CON_ERR(("Could not find team '%s'", name))
          break;
        }

        if (Console::GetArgString(2, type))
        {
          // Print out the units of this type
          const NList<UnitObj> *units = team->GetUnitObjects(Crc::CalcStr(type));

          if (units)
          {
            CON_DIAG(("Units for '%s' of type '%s'", name, type))

            for (NList<UnitObj>::Iterator u(units); *u; u++)
            {
              CON_DIAG((" - [%d]", (*u)->Id()))
            }
          }
          else
          {
            CON_DIAG(("No units of type '%s' for team '%s'", type, name))
          }
        }
        else
        {
          const NList<UnitObj> &units = team->GetUnitObjects();

          for (NList<UnitObj>::Iterator u(&units); *u; u++)
          {
            CON_DIAG((" - %s [%d]", (*u)->TypeName(), (*u)->Id()))
          }
        }
      }

      break;
    }
  }
}


//
// Team::NewId
//
// Finds an unused Id
//
U32 Team::NewId()
{
  ASSERT(initialized)

  for (int i = 0; i < Game::MAX_TEAMS; i++)
  {
    if (!teamsById[i])
    {
      return (i);
    }
  }

  ERR_FATAL(("Ran out of team id's!"))
}


//
// Team::Configure
//
// Configure all of the teams from a config file
//
void Team::Configure(FScope *fScope)
{
  ASSERT(initialized)

  FScope *sScope;

  // Process each function
  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xB884B9E8: // "CreateTeam"
        new Team(sScope);
        break;

      default:
        LOG_ERR(("Unknown command '%s'", sScope->NameStr()))
        break;
    }
  }
}


//
// Team::Purge
//
// Purge all teams which don't have players
//
void Team::Purge()
{
  ASSERT(initialized)

  NBinTree<Team>::Iterator t(&teamsByName);
  Team *team;

  while ((team = t++) != NULL)
  {
    if (!team->HavePlayer() && team->IsAvailablePlay())
    {
      #ifdef DEVELOPMENT
        LOG_DIAG(("Purging team %s [%d]", team->GetName(), team->GetId()))
      #endif
      delete team;
    }
  }
}


//
// Team::PreLoadAll
//
void Team::PreLoadAll()
{
  ASSERT(initialized)

  for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
  {
    // Preload each team
    (*t)->PreLoad();
  }

  /*
  // Clear relation flags
  for (!t; *t; t++)
  {
    // Clear relations changed bitfield
    (*t)->relationChanged = 0;
  }
  */
}


//
// Team::PostLoadAll
//
void Team::PostLoadAll()
{
  ASSERT(initialized)

  for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
  {
    // Postload each team
    (*t)->PostLoad();
  }
}


//
// CreateSideBasedUnits
//
// If required, create side based units for all teams
//
void Team::CreateSideBasedUnits()
{
  // Iterate the teams
  for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
  {
    // Ignore if fixed side
    if (!(*t)->GetSideFixed())
    {
      SidePlacement::Place(*t);
    }
  }
}


//
// ApplyDifficultyLevels
//
// If required, apply difficulty levels to enemy teams
//
void Team::ApplyDifficultyLevels()
{
  // Do we need to do anything
  if (Difficulty::GetApplyEnemy() && displayTeam)
  {
    // Get the current difficulty level
    F32 level = Difficulty::GetCurrentSetting().GetValue();

    // Iterate the teams
    for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
    {
      // Do we need to change this team
      if (TestRelation(*t, displayTeam, Relation::ENEMY))
      {
        (*t)->SetDifficulty(level);
      }
    }
  }

  // Clear the apply flag
  Difficulty::SetApplyEnemy(FALSE);
}


//
// Team::SaveAll
//
// Save all teams to a config file
//
void Team::SaveAll(FScope *fScope)
{
  ASSERT(initialized)
  ASSERT(fScope)

  for (NBinTree<Team>::Iterator i(&teamsByName); *i; i++)
  {
    (*i)->Save(fScope);
  }
}


//
// Team::ProcessAll
//
// Perform team processing for all teams
//
void Team::ProcessAll(Bool force)
{
  ASSERT(initialized)
 
  // Process all sight first
  for (NBinTree<Team>::Iterator t(&teamsByName); *t; t++)
  {
    (*t)->ProcessSight(force);
  }

  // Process the rest
  for (!t; *t; t++)
  {
    (*t)->Process();
  }

  UpdateMono();
}


//
// TestRelation
//
// Test a unit against the current display team (FALSE if no display team)
//
Bool Team::TestDisplayRelation(UnitObj *unit, Relation relation)
{
  ASSERT(unit)
  ASSERT(initialized)

  return (displayTeam ? displayTeam->TestUnitRelation(unit, relation) : FALSE);
}


//
// TestRelation
//
// Test relation of 2 teams, where either could be NULL
//
Bool Team::TestRelation(Team *team1, Team *team2, Relation relation)
{
  if (team1 && team2)
  {
    // Use the standard relation function
    return (team1->TestRelation(team2->GetId(), relation));
  }
  else
  {
    // When either team is NULL, assume NEUTRAL
    return (relation == Relation::NEUTRAL);
  }
}


//
// TestUnitRelation
//
// Test relation of unit with a team, team could be NULL
//
Bool Team::TestUnitRelation(UnitObj *unit, Team *team, Relation relation)
{
  if (team)
  {
    return (team->TestUnitRelation(unit, relation));
  }
  else
  {
    return (relation == Relation::NEUTRAL);
  }
}


//
// GetRelation
//
// Get relation between two teams, where either could be NULL
//
Relation Team::GetRelation(Team *team1, Team *team2)
{
  if (team1 && team2)
  {
    return (team1->GetRelation(team2->GetId()));
  }
  else
  {
    return Relation::NEUTRAL;
  }
}



//
// AllowedTypeByAny
//
// Is the given unit type available for any team
//
Bool Team::AllowedTypeByAny(UnitObjType *type)
{
  ASSERT(initialized)

  // Check each team
  for (int i = 0; i < Game::MAX_TEAMS; ++i)
  {
    // Is this a valid team
    if (Team *team = teamsById[i])
    {
      // Is the type allowed by this team
      if (team->AllowedType(type))
      {
        return (TRUE);
      }
    }
  }

  return (FALSE);
}


//
// Player had departed
//
void Team::PlayerDeparted(Player *player)
{
  ASSERT(player)

  // This player has departed
  player->Departed();

  // Were they on a team (I hope so, but you never can tell)
  Team *team = player->GetTeam();
  if (team)
  {
    Bool anyone = FALSE;

    // Are there any non AI players remaining on this team ?
    for (List<Player>::Iterator p(&team->players); *p; ++p)
    {
      if ((*p)->GetType() != Player::AI && !(*p)->HasDeparted())
      {
        anyone = TRUE;
        break;
      }
    }

    if (!anyone)
    {
      // Blow it all up!
      for (NList<GameObj>::Iterator g(&GameObjCtrl::listAll); *g; ++g)
      {
        UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*g);

        if (unit && unit->GetTeam() == team)
        {
          unit->SelfDestruct(TRUE);
        }
      }
    }
  }
}


//
// Team::UpdateMono
//
// Update the mono display
//
void Team::UpdateMono()
{
  #ifndef MONO_DISABLED

  // Firstly update the panel which shows general information about all teams
  char buff[90];
  char *s = buff;
  U32 n;

  // Update the mono display
  for (U32 t = 0; t < Game::MAX_TEAMS; t++)
  {
    Team *team = teamsById[t];

    if (team)
    {
      // Move pointer back to start of buffer
      s = buff;

      // Add the ID      
      Utils::StrFmtDec(s, 2, team->GetId());
      *(s += 2)++ = ' ';

      // Add the Team Name
      const char *p = team->GetName();
      n = 0;
      while (n++ < 20)
      {
        *s++ = *p ? *p++ : ' ';
      }
      *s++ = ' ';

      // Add the Relations
      for (U32 i = 0; i < Game::MAX_TEAMS; i++)
      {
        if (i == t)
        {
          *s++ = ' ';
        }
        else
        {
          if (teamsById[i])
          {
            *s++ = team->relations[i].GetChar();
          }
          else
          {
            *s++ = (U8) 250;
          }
        }
      }
      *s++ = ' ';
      *s++ = ' ';

      // Add the Number of Allied teams
      Utils::StrFmtDec(s, 1, team->NumRelated(Relation::ALLY));
      *(s += 1)++ = ' ';

      // Add the Number of Neutral teams
      Utils::StrFmtDec(s, 1, team->NumRelated(Relation::NEUTRAL));
      *(s += 1)++ = ' ';

      // Add the Number of Enemy teams
      Utils::StrFmtDec(s, 1, team->NumRelated(Relation::ENEMY));
      *(s += 1)++ = ' ';

      // Add the Number of objects
      Utils::StrFmtDec(s, 4, team->GetUnitObjects().GetCount());
      *(s += 4)++ = ' ';

      // Add the Total Threat
      Utils::StrFmtDec(s, 8, AI::Map::GetTotalThreat(team->GetId()));
      *(s += 8)++ = ' ';

      // Add the Total Defense
      Utils::StrFmtDec(s, 8, AI::Map::GetTotalDefense(team->GetId()));
      *(s += 8)++ = ' ';

      // Add the Resource Storage
      Utils::StrFmtDec(s, 8, team->resourceStore);
      *(s += 8)++ = ' ';

      // Add the Power level
      Utils::StrFmtDec(s, 8, team->power.GetAvailable());
      *(s += 8)++ = ' ';

      // Null terminate
      *s = 0;

      // Write it to the mono
      MonoBufWrite(monoBuffer, t + 1, 0, buff, Mono::NORMAL);
    }
    else
    {
      // Clear the row
      MonoBufFillRow(monoBuffer, t + 1, ' ', Mono::NORMAL);
    }
  }

  #endif
}

//
// Team::SetColor
//
// update color
//
void Team::SetColor(const Color &c)
{
  ASSERT(initialized)

  color = c;

  // All of the units on this team get their color assigned
  NList<UnitObj>::Iterator u(&unitObjects);
  UnitObj *unitObj;
  while ((unitObj = u++) != NULL)
  {
    unitObj->Mesh().SetTeamColor( color);
  }
}
