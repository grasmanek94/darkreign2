///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Action System
//
// 11-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "action.h"
#include "console.h"
#include "team.h"
#include "tagobj.h"
#include "regionobj.h"
#include "terrain.h"
#include "stdload.h"
#include "gameobjctrl.h"
#include "mapobjctrl.h"
#include "worldctrl.h"
#include "promote.h"
#include "unitobj.h"
#include "message.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"
#include "viewer.h"
#include "sound.h"
#include "strategic_object.h"
#include "operation.h"
#include "resolver.h"
#include "offmapobj.h"
#include "wallobj.h"
#include "tasks_unitrecycle.h"
#include "tasks_unitanimate.h"
#include "client.h"
#include "param.h"
#include "iface.h"
#include "missions.h"
#include "client_debriefing.h"
#include "comparison.h"
#include "switch.h"
#include "missionvar.h"
#include "fx.h"
#include "multiplayer_data.h"
#include "multiplayer.h"
#include "iface_messagebox.h"
#include "babel.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Action
//
namespace Action
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void CreateVarInteger(Team *team, FScope *fScope);
  static void CreateVarFloat(Team *team, FScope *fScope);
  static void CreateVarString(Team *team, FScope *fScope);
  static void DeleteVar(Team *team, FScope *fScope);
  static void Cmd(Team *team, FScope *fScope);
  static void TeamCmd(Team *team, FScope *fScope);
  static void Op(Team *team, FScope *fScope);
  static void NewObjective(Team *team, FScope *fScope);
  static void NewRandomObjective(Team *team, FScope *fScope);
  static void CurrentObjectiveAbandoned(Team *team, FScope *fScope);
  static void ObjectiveAbandoned(Team *team, FScope *fScope);
  static void Win(Team *team, FScope *fScope);
  static void Lose(Team *team, FScope *fScope);
  static void Next(Team *team, FScope *fScope);
  static void Eliminate(Team *team, FScope *fScope);
  static void Stop(Team *team, FScope *fScope);
  static void SpawnObjects(Team *team, FScope *fScope);
  static void UseOffMapObject(Team *team, FScope *fScope);
  static void SetTeam(Team *team, FScope *fScope);
  static void SetTactical(Team *team, FScope *fScope);
  static void SelfDestruct(Team *team, FScope *fScope);
  static void Delete(Team *team, FScope *fScope);
  static void TriggerFX(Team *team, FScope *fScope);
  static void SetUnitLimit(Team *team, FScope *fScope);
  static void WallOperation(Team *team, FScope *fScope);
  static void Recycle(Team *team, FScope *fScope);
  static void GameMessage(Team *team, FScope *fScope);
  static void RegionMessage(Team *team, FScope *fScope);
  static void TagMessage(Team *team, FScope *fScope);
  static void TriggerTeamRadio(Team *team, FScope *fScope);

  static void DisplayObjective(Team *team, FScope *fScope);

  static void Cineractive(Team *team, FScope *fScope);

  static void ExecuteScript(Team *team, FScope *fScope);
  static void KillScript(Team *team, FScope *fScope);
  static void SetScriptWeighting(Team *team, FScope *fScope);
  static void SetScriptPriority(Team *team, FScope *fScope);

  static void AddBase(Team *team, FScope *fScope);
  static void RemoveBase(Team *team, FScope *fScope);
  static void SetBaseState(Team *team, FScope *fScope);
  static void SetBaseWeighting(Team *team, FScope *fScope);
  static void SetBasePriority(Team *team, FScope *fScope);
  static void SetBaseOrdererWeighting(Team *team, FScope *fScope);
  static void SetBaseOrdererPriority(Team *team, FScope *fScope);
  static void AssignBaseConstructors(Team *team, FScope *fScope);
  static void AssignBaseUnits(Team *team, FScope *fScope);
  static void AutoDisruptBase(Team *team, FScope *fScope);
  static void SetBombardierPreferences(Team *team, FScope *fScope);
  static void AddIntel(Team *team, FScope *fScope);
  static void RemoveIntel(Team *team, FScope *fScope);

  static void HighlightConstruction(Team *team, FScope *fScope);
  static void UnhighlightConstruction(Team *team, FScope *fScope);
  static void Animate(Team *team, FScope *fScope);


  //
  // Execute the action
  //
  void Execute(Team *team, FScope *fScope)
  {
    ASSERT(team)
    ASSERT(fScope)

    fScope->InitIterators();
    FScope *sScope;

    // Process each Action
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x70DFC843: // "If"
        {
          FSCOPE_DIRTYALL(sScope)

          if (Comparison::Function(sScope, team))
          {
            sScope->InitIterators();
            Execute(team, sScope);
          }
          else
          {
            if (sScope->ParentFunction())
            {
              FScope *nScope = sScope->ParentFunction()->PeekFunction();
              if (nScope && nScope->NameCrc() == 0x5F3F76C0) // "Else"
              {
                Execute(team, nScope);
              }
            }
          }
          break;
        }

        case 0x5F3F76C0: // "Else"
          // Ignore elses on their own
          break;

        case 0x4BED1273: // "Switch"
        {
          FSCOPE_DIRTYALL(sScope)

          // Get the name of the var to test on
          Switch::Value &value = Switch::GetValue(sScope, team);

          while (FScope *ssScope = sScope->NextFunction())
          {
            if (ssScope->NameCrc() == 0x97AF68BF) // "Case"
            {
              if (Switch::CompareValue(value, ssScope, team))
              {
                Execute(team, ssScope);
                break;
              }
            }
            else if (ssScope->NameCrc() == 0x8F651465) // "Default"
            {
              Execute(team, ssScope);
            }
          }

          Switch::DisposeValue(value);
          break;
        }

        case 0x921C808B: // "CreateVarInteger"
          CreateVarInteger(team, sScope);
          break;

        case 0xAFF1375D: // "CreateVarFloat"
          CreateVarFloat(team, sScope);
          break;

        case 0xA2F8DAA2: // "CreateVarString"
          CreateVarString(team, sScope);
          break;

        case 0xDB80CCEB: // "DeleteVar"
          DeleteVar(team, sScope);
          break;

        case 0xC4FD8F50: // "Cmd"
          Cmd(team, sScope);
          break;

        case 0x90BA257C: // "TeamCmd"
          TeamCmd(team, sScope);
          break;

        case 0xC39EE127: // "Op"
          Op(team, sScope);
          break;

        case 0x6E8D4F78: // "NewObjective"
          NewObjective(team, sScope);
          break;

        case 0x53B6C4D8: // "NewRandomObjective"
          NewRandomObjective(team, sScope);
          break;

        case 0xC41038E7: // "CurrentObjectiveAbandoned"
          CurrentObjectiveAbandoned(team, sScope);
          break;

        case 0x50600A61: // "ObjectiveAbandoned"
          ObjectiveAbandoned(team, sScope);
          break;

        case 0xB4BB0243: // "Win"
          Win(team, sScope);
          break;

        case 0x78D181F4: // "Lose"
          Lose(team, sScope);
          break;

        case 0x79CC4E5D: // "Next"
          Next(team, sScope);
          break;

        case 0xF6C261CD: // "Eliminate"
          Eliminate(team, sScope);
          break;

        case 0xFF62DA04: // "Stop"
          Stop(team, sScope);
          break;

        case 0x60D3C700: // "SpawnObjects"
          SpawnObjects(team, sScope);
          break;

        case 0x4FFA4788: // "UseOffMapObject"
          UseOffMapObject(team, sScope);
          break;

        case 0xA9119C46: // "SetTeam"
          SetTeam(team, sScope);
          break;

        case 0xC115BCE6: // "SetTactical"
          SetTactical(team, sScope);
          break;

        case 0x3E6F6C6C: // "SelfDestruct"
          SelfDestruct(team, sScope);
          break;

        case 0x0725EAC5: // "Delete"
          Delete(team, sScope);
          break;

        case 0xEFC00805: // "TriggerFX"
          TriggerFX(team, sScope);
          break;

        case 0xF19CF58F: // "SetUnitLimit"
          SetUnitLimit(team, sScope);
          break;

        case 0x7E479B37: // "WallOperation"
          WallOperation(team, sScope);
          break;

        case 0xF8AB3A7A: // "Recycle"
          Recycle(team, sScope);
          break;

        case 0x8C217C61: // "GameMessage"
          GameMessage(team, sScope);
          break;

        case 0xF2D032B3: // "RegionMessage"
          RegionMessage(team, sScope);
          break;

        case 0xAA3E093E: // "TagMessage"
          TagMessage(team, sScope);
          break;

        case 0xA1D44A0D: // "TriggerTeamRadio"
          TriggerTeamRadio(team, sScope);
          break;

        case 0xF296346A: // "DisplayObjective"
          DisplayObjective(team, sScope);
          break;

        case 0x20C2A163: // "Cineractive"
          Cineractive(team, sScope);
          break;

        case 0x7285C93A: // "ExecuteScript"
          ExecuteScript(team, sScope);
          break;

        case 0x37F335B0: // "KillScript"
          KillScript(team, sScope);
          break;

        case 0x63406872: // "SetScriptWeighting"
          SetScriptWeighting(team, sScope);
          break;

        case 0x757EB591: // "SetScriptPriority"
          SetScriptPriority(team, sScope);
          break;

        case 0xEF05AB1B: // "AddBase"
          AddBase(team, sScope);
          break;

        case 0x483747F8: // "RemoveBase"
          RemoveBase(team, sScope);
          break;

        case 0x1111A664: // "SetBaseState"
          SetBaseState(team, sScope);
          break;

        case 0x4A015FB4: // "SetBaseWeighting"
          SetBaseWeighting(team, sScope);
          break;

        case 0xCDCCC2C6: // "SetBasePriority"
          SetBasePriority(team, sScope);
          break;

        case 0xC2E5FFB6: // "SetBaseOrdererWeighting"
          SetBaseOrdererWeighting(team, sScope);
          break;

        case 0x9A22F69D: // "SetBaseOrdererPriority"
          SetBaseOrdererPriority(team, sScope);
          break;

        case 0x85B430EF: // "AssignBaseConstructors"
          AssignBaseConstructors(team, sScope);
          break;

        case 0x4E19070A: // "AutoDisruptBase"
          AutoDisruptBase(team, sScope);
          break;

        case 0x978F281B: // "AssignBaseBuildings" // To support old configs
          LOG_WARN(("AssignBaseBuildings should now be AssignBaseUnits"))

        case 0xB6AE6E61: // "AssignBaseUnits"
          AssignBaseUnits(team, sScope);
          break;

        case 0x8F6096A1: // "SetBombardierPreferences"
          SetBombardierPreferences(team, sScope);
          break;

        case 0xAA8E1319: // "AddIntel"
          AddIntel(team, sScope);
          break;

        case 0x770AF612: // "RemoveIntel"
          RemoveIntel(team, sScope);
          break;

        case 0x95592DD9: // "HighlightConstruction"
          HighlightConstruction(team, sScope);
          break;

        case 0xE0ABA3AB: // "UnhighlightConstruction"
          UnhighlightConstruction(team, sScope);
          FSCOPE_DIRTY(sScope)
          break;

        case 0xFAA22138: // "Animate"
          Animate(team, sScope);
          break;

        default:
          sScope->ScopeError("Unknown Action");
          break;
      }
    }

    FSCOPE_CHECK(fScope)
  }


  //
  // CreateVarInteger
  //
  void CreateVarInteger(Team *team, FScope *fScope)
  {
    const char *name = StdLoad::TypeString(fScope);
    S32 dVal = S32(StdLoad::TypeU32(fScope, U32(0)));
    S32 minVal = S32(StdLoad::TypeU32(fScope, U32(S32_MIN)));
    S32 maxVal = S32(StdLoad::TypeU32(fScope, U32(S32_MAX)));

    if (*name != TEAM_VARCHAR && *name != MISSION_VARCHAR)
    {
      fScope->ScopeError("Only dynamic team vars can be created");
    }

    VarSys::CreateInteger(name, dVal, VarSys::DEFAULT, NULL, team)->SetIntegerRange(minVal, maxVal);
  }


  //
  // CreateVarFloat
  //
  void CreateVarFloat(Team *team, FScope *fScope)
  {
    const char *name = StdLoad::TypeString(fScope);
    F32 dVal = StdLoad::TypeF32(fScope, 0.0F);
    F32 minVal = StdLoad::TypeF32(fScope, F32_MIN);
    F32 maxVal = StdLoad::TypeF32(fScope, F32_MAX);

    if (*name != TEAM_VARCHAR && *name != MISSION_VARCHAR)
    {
      fScope->ScopeError("Only dynamic team vars can be created");
    }

    VarSys::CreateFloat(name, dVal, VarSys::DEFAULT, NULL, team)->SetFloatRange(minVal, maxVal);
  }


  //
  // CreateVarString
  //
  void CreateVarString(Team *team, FScope *fScope)
  {
    const char *name = StdLoad::TypeString(fScope);
    const char *dVal = StdLoad::TypeStringD(fScope, "");

    if (*name != TEAM_VARCHAR && *name != MISSION_VARCHAR)
    {
      fScope->ScopeError("Only dynamic team vars can be created");
    }

    VarSys::CreateString(name, dVal, VarSys::DEFAULT, NULL, team);
  }


  //
  // DeleteVar
  //
  void DeleteVar(Team *team, FScope *fScope)
  {
    const char *name = StdLoad::TypeString(fScope);

    if (*name != TEAM_VARCHAR && *name != MISSION_VARCHAR)
    {
      fScope->ScopeError("Only dynamic team vars can be deleted");
    }
    
    VarSys::DeleteItem(name, team);
  }


  //
  // Cmd
  //
  void Cmd(Team *, FScope *fScope)
  {
    Console::ProcessCmd(fScope->NextArgString());
  }


  //
  // TeamCmd
  //
  void TeamCmd(Team *team, FScope *fScope)
  {
    Console::ProcessCmd(StdLoad::TypeString(fScope), team);
  }


  //
  // Op
  //
  void Op(Team *team, FScope *fScope)
  {
    Operation::Function(fScope, team);
  }


  //
  // NewObjective
  //
  void NewObjective(Team *team, FScope *fScope)
  {
    Param::String name(fScope, team);
    team->NewObjective(name, StdLoad::TypeU32(fScope, TRUE));
  }


  //
  // NewRandomObjective
  //
  void NewRandomObjective(Team *team, FScope *fScope)
  {
    GameIdentListWeighted list;

    list.Load(fScope);
    team->NewObjective(list.RandomSync().str);
  }


  //
  // CurrentObjectiveAbandoned
  //
  void CurrentObjectiveAbandoned(Team *team, FScope *fScope)
  {
    Param::String name(fScope, team);
    team->ObjectiveAbandoned(team->CurrentObjective(name));
  }


  //
  // ObjectiveAbandoned
  //
  void ObjectiveAbandoned(Team *team, FScope *fScope)
  {
    Param::String name(fScope, team);

    // Test to see if this is a current objective
    if (Objective::Object *objective = team->CurrentObjective(name))
    {
      team->ObjectiveAbandoned(objective);
    }
  }


  //
  // CheckNoResult
  //
  // Returns TRUE if no teams have a result
  //
  static Bool CheckNoResult()
  {
    for (U32 i = 0; i < Game::MAX_TEAMS; i++)
    {
      if (Team *t = Team::Id2Team(i))
      {
        if (t->GetEndGame().result != Team::EndGame::NR)
        {
          LOG_DIAG((" [%s] already has result [%s]", t->GetName(), Team::EndGame::resultName[t->GetEndGame().result]));
          return (FALSE);
        }
      }
    }

    return (TRUE);
  }


  //
  // SetTeamResult
  //
  // Set the given results
  //
  static void SetTeamResult(Team *team, Team::EndGame::Result r1, Team::EndGame::Result r2)
  {
    // Set the results for each team
    for (U32 i = 0; i < Game::MAX_TEAMS; i++)
    {
      if (Team *t = Team::Id2Team(i))
      {
        t->ReportResult(team->TestRelation(i, Relation::ALLY) ? r1 : r2);

        LOG_DIAG((" [%s] [%s]", t->GetName(), Team::EndGame::resultName[t->GetEndGame().result]));
      }
    }
  }

  //
  // TriggerDebriefing
  //
  // Initiate the debriefing sequence using the given control
  //
  static void TriggerDebriefing(const char *control)
  {
    LOG_DIAG(("Initiating non-sync debriefing [%s]", control));

    // Find the debriefing control
    if (IControl *ctrl = IFace::Find<IControl>(control))
    {
      // Kill all game windows
      IFace::PurgeNonSystem(); 

      // Bring up the debriefing
      IFace::Activate(ctrl);

      // Notify it once
      IFace::SendNotify(ctrl, NULL, Client::DebriefingResultNotify::Init);
    }
    else
    {
      LOG_DIAG((" - control not found!"));
    }
  }


  //
  // TriggerDebriefing
  //
  // Start debriefing sequence for this user
  //
  static void TriggerUserDebriefing()
  {
    // Does this user get the win screen
    if (Team::GetDisplayTeam() && Team::GetDisplayTeam()->GetEndGame().result == Team::EndGame::WIN)
    {
      TriggerDebriefing("Client::Debriefing::Win");
    }
    else
    {
      TriggerDebriefing("Client::Debriefing::Lose");
    }

    // Pause the game
    if (!GameTime::Paused())
    {
      GameTime::Pause(FALSE);
    }
  }


  //
  // Win
  //
  void Win(Team *team, FScope *fScope)
  {
    fScope;
    FSCOPE_DIRTY(fScope)

    LOG_DIAG(("Team [%s] has WON", team->GetName()))

    if (CheckNoResult())
    {
      // Set the team results
      SetTeamResult(team, Team::EndGame::WIN, Team::EndGame::LOSE);

      // Start the debriefing
      TriggerUserDebriefing();
    }
  }


  //
  // Lose
  //
  void Lose(Team *team, FScope *fScope)
  {
    fScope;
    FSCOPE_DIRTY(fScope)

    LOG_DIAG(("Team [%s] has LOST", team->GetName()))

    if (CheckNoResult())
    {
      // Set the team results
      SetTeamResult(team, Team::EndGame::LOSE, Team::EndGame::WIN);

      // Start the debriefing
      TriggerUserDebriefing();
    }
  }

  
  //
  // Next
  //
  void Next(Team *team, FScope *fScope)
  {
    fScope;
    FSCOPE_DIRTY(fScope)

    if (CheckNoResult())
    {
      LOG_DIAG(("Team [%s] NEXT", team->GetName()))

      // Give a win to all teams
      SetTeamResult(team, Team::EndGame::WIN, Team::EndGame::WIN);

      // Are we in multiplayer
      if (MultiPlayer::Data::Online())
      {
        Missions::ReturnToShell();
      }
      else
      {
        // Proceed automatically to next mission
        Missions::NextMission(TRUE);
      }
    }
  }

  
  //
  // Lose
  //
  void Eliminate(Team *team, FScope *fScope)
  {
    fScope;
    FSCOPE_DIRTY(fScope)

    // Ignore if this isn't the display team
    if (Team::GetDisplayTeam() == team)
    {
      // Do not eliminate if the host
      if (MultiPlayer::Data::Online() && MultiPlayer::IsHost())
      {
        IFace::MsgBox
        (
          TRANSLATE(("#multiplayer.eliminated.title")),
          TRANSLATE(("#multiplayer.eliminated.message")),
          0,
          new MBEvent("Ok", TRANSLATE(("#standard.buttons.ok")))
        );
      }
      else

      // Ignore if team already has a result
      if (team->GetEndGame().result == Team::EndGame::NR)
      {
        LOG_DIAG(("Team [%s] has been ELIMINATED", team->GetName()))

        // Disconnect from a network game
        if (MultiPlayer::Data::Online())
        {
          Console::ProcessCmd("multiplayer.abort");
        }

        // Set this team to lose
        team->ReportResult(Team::EndGame::LOSE);

        // Start the debriefing
        TriggerUserDebriefing();
      }
    }
  }


  //
  // Stop
  //
  void Stop(Team *team, FScope *fScope)
  {
    Param::Ident tagName("Tag", fScope, "", team);

    // Is a tag name supplied
    if (tagName.GetCrc())
    {
      // Self destruct every unit in the tag
      if (TagObj *tag = TagObj::FindTag(tagName.GetCrc()))
      {
        for (MapObjList::Iterator o(&tag->list); *o; o++)
        {
          if ((*o)->Alive())
          {
            // If this is a unit tell it to stop
            UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(**o);

            if (unit)
            {
              unit->Stop();
            }
          }
        }
      }
      else
      {
        fScope->ScopeError("Stop could not find tag [%s]", tagName.GetStr());
      }
    }
    else
    {
      // Stop every unit on the team
      for (NList<GameObj>::Iterator u(&GameObjCtrl::listAll); *u; ++u)
      {
        UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*u);

        if (unit && unit->GetTeam() == team)
        {
          unit->Stop();
        }
      }
    }
  }


  //
  // SpawnObjects
  //
  void SpawnObjects(Team *team, FScope *fScope)
  {
    const char *formationName = StdLoad::TypeString(fScope, "Formation");
    F32 direction = (90.0f - StdLoad::TypeF32(fScope, "Direction", Range<F32>(-180.0f, 180.0f))) * DEG2RAD;

    Param::Ident regionName("Region", fScope, team);
    Param::Ident tagName("Tag", fScope, "", team);

    Bool random = StdLoad::TypeU32(fScope, "Random", FALSE, Range<U32>::flag);
    Bool construct = StdLoad::TypeU32(fScope, "Construct", FALSE, Range<U32>::flag);
    UnitObjTypeList types;

    FScope *ssScope;
    while ((ssScope = fScope->NextFunction()) != NULL)
    {
      if (ssScope->NameCrc() == 0xB95A11FF) // "AddType"
      {
        UnitObjTypePtr type;
        StdLoad::TypeReaperObjType(ssScope, type);
        U32 amount = StdLoad::TypeU32(ssScope, 1, Range<U32>::cardinal);

        Resolver::Type(type, TRUE);
        
        while (amount--)
        {
          types.Append(type);
        }
      }
    }

    // Resolve the formation
    Formation::Slots *formation = Formation::Find(Crc::CalcStr(formationName));

    if (!formation)
    {
      fScope->ScopeError("Couldn't resolve formation '%s'", formationName);
    }

    // Resolve the region
    RegionObj *region = RegionObj::FindRegion(regionName);

    if (!region)
    {
      fScope->ScopeError("Couldn't resolve region '%s'", regionName);
    }

    // Resolve the tag
    TagObj *tag = NULL;
    
    if (tagName.GetCrc())
    {
      tag = TagObj::FindTag(tagName.GetCrc());
      if (!tag)
      {
        tag = TagObj::CreateTag(tagName);
      }
    }

    List<::Formation::Slot>::Iterator slot(&formation->slots);

    while (types.GetCount())
    {
      // Is there a slot for this unit ?
      if (*slot)
      {
        // Calculate the absolute direction to the slot
        F32 dir = direction + (*slot)->direction;
        VectorDir::FixU(dir);

        // Calculate the absolute orientation of the slot
        F32 orient = direction + (*slot)->orientation;
        VectorDir::FixU(orient);

        // Determine the slot offset
        Vector offset;
        offset.x = (F32) cos(dir);
        offset.y = 0.0f;
        offset.z = (F32) sin(dir);
        offset *= (*slot)->distance;
        offset.x += region->GetMidPoint().x;
        offset.z += region->GetMidPoint().z;
        offset.y = TerrainData::FindFloor(offset.x, offset.z);

        UnitObjType *type;
        UnitObjTypeListNode *node;

        if (random)
        {
          node = types[Random::sync.Integer(types.GetCount())];
        }
        else
        {
          node = types.GetHead();
        }

        // Rip the type out of the node
        type = *node;

        // Dispose of the node
        types.Dispose(node);

        ASSERT(type)

        UnitObj &unitObj = type->Spawn(offset, team, construct);

        unitObj.SetSimTarget(Quaternion(orient, Matrix::I.Up()));

        if (tag)
        {
          tag->list.Append(&unitObj);
        }

        // Increment the slot
        slot++;
      }
      else
      {
        // Ran out of slots
        LOG_WARN(("Ran out of slots in formation '%s' whilst spawning in units", formationName))
        types.Clear();
        break;
      }
    }
  }


  //
  // UseOffMapObject
  //
  void UseOffMapObject(Team *team, FScope *fScope)
  {
    // Load the region

    Param::Ident regionName("Region", fScope, team);
    RegionObj *region = RegionObj::FindRegion(regionName);
    if (!region)
    {
      fScope->ScopeError("Couldn't resolve region '%s'", regionName.GetStr());
    }

    // Load the type
    UnitObjTypePtr type;
    StdLoad::TypeReaperObjType(fScope, "Type", type);
    Resolver::Type(type);
    if (!type.Alive())
    {
      fScope->ScopeError("Could not resolve type");
    }
    if (!Promote::Type<OffMapObjType>(type))
    {
      fScope->ScopeError("Type supplied is not an offmap type");
    }

    // Get the position
    Vector pos;
    pos.x = region->GetMidPoint().x;
    pos.z = region->GetMidPoint().z;
    pos.y = TerrainData::FindFloor(pos.x, pos.z);

    // Create the object
    MapObj *obj = MapObjCtrl::ObjectNew(type);

    // Promote to an offmap object
    OffMapObj *offMap = Promote::Object<OffMapObjType, OffMapObj>(obj);
    ASSERT(offMap)
    
    // Set the object's team
    offMap->SetTeam(team);

    //LOG_DIAG(("Spawning air strike '%s' [%f, %f, %f]", type->GetName(), pos.x, pos.y, pos.z))

    // Execute the offmap object
    offMap->Execute(0x63417A92, pos); // "Trigger::Positional"
  }


  //
  // SetTeam
  //
  void SetTeam(Team *team, FScope *fScope)
  {
    Param::Ident teamName("Team", fScope, team);
    Param::Ident tagName("Tag", fScope, team);

    // Resolve Team
    Team *nteam = team;
    if (teamName)
    {
      nteam = Team::Name2Team(teamName);
      if (!nteam)
      {
        LOG_WARN(("SetTeam: Could not resolve team '%s'", teamName.GetStr()))
        return;
      }
    }

    // Resolve Tag (if one was given)
    TagObj *tag;
    if ((tag = TagObj::FindTag(tagName.GetCrc())) == NULL)
    {
      LOG_WARN(("SetTeam: Could not find tag '%s'", tagName.GetStr()))
      return;
    }

    // Change the team of the objects in the tag
    for (MapObjList::Iterator o(&tag->list); *o; o++)
    {
      if ((*o)->Alive())
      {
        UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(**o);
        if (unitObj)
        {
          // Change the team of this unit
          unitObj->SetTeam(nteam);
        }
      }
    }
  }


  //
  // SetTactcical
  //
  void SetTactical(Team *team, FScope *fScope)
  {
    Param::Ident tagName("Tag", fScope, team);
    Tactical::ModifierSettings modifierSettings(fScope);

    // Resolve Tag
    TagObj *tag;
    if ((tag = TagObj::FindTag(tagName.GetCrc())) == NULL)
    {
      LOG_WARN(("SetTactical: Could not find tag '%s'", tagName.GetStr()))
      return;
    }

    // Change the tactical modifiers of the objects in the tag
    for (MapObjList::Iterator o(&tag->list); *o; o++)
    {
      if ((*o)->Alive())
      {
        UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(**o);
        if (unitObj)
        {
          // Set the tactical modifiers of this unit
          unitObj->settings = modifierSettings;
        }
      }
    }
  }


  //
  // SelfDestruct
  //
  void SelfDestruct(Team *team, FScope *fScope)
  {
    Param::Ident tagName("Tag", fScope, "", team);

    // Is a tag name supplied
    if (tagName.GetCrc())
    {
      // Self destruct every unit in the tag
      if (TagObj *tag = TagObj::FindTag(tagName.GetCrc()))
      {
        for (MapObjList::Iterator o(&tag->list); *o; o++)
        {
          if ((*o)->Alive())
          {
            (**o)->SelfDestruct(TRUE);
          }
        }
      }
      else
      {
        fScope->ScopeError("SelfDestruct could not find tag [%s]", tagName.GetStr());
      }
    }
    else
    {
      // Self destruct every unit on the team
      for (NList<GameObj>::Iterator u(&GameObjCtrl::listAll); *u; ++u)
      {
        UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*u);

        if (unit && unit->GetTeam() == team)
        {
          unit->SelfDestruct(TRUE);
        }
      }
    }
  }


  //
  // Delete
  //
  void Delete(Team *team, FScope *fScope)
  {
    Param::Ident tagName("Tag", fScope, "", team);

    // Is a tag name supplied
    if (tagName.GetCrc())
    {
      // Delete every unit in the tag
      if (TagObj *tag = TagObj::FindTag(tagName.GetCrc()))
      {
        for (MapObjList::Iterator o(&tag->list); *o; o++)
        {
          if ((*o)->Alive())
          {
            GameObjCtrl::MarkForDeletion(**o);
          }
        }
      }
      else
      {
        fScope->ScopeError("Delete could not find tag [%s]", tagName.GetStr());
      }
    }
    else
    {
      // Delete every unit on the team
      for (NList<GameObj>::Iterator u(&GameObjCtrl::listAll); *u; ++u)
      {
        UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*u);

        if (unit && unit->GetTeam() == team)
        {
          unit->MarkForDeletion();
        }
      }
    }
  }


  //
  // TriggerFX
  //
  void TriggerFX(Team *team, FScope *fScope)
  {
    // Load the tag name
    Param::Ident tagName("Tag", fScope, "", team);

    // Is a tag name supplied
    if (tagName.GetCrc())
    {
      // Find the tag
      if (TagObj *tag = TagObj::FindTag(tagName.GetCrc()))
      {
        // Load the effect name
        Param::Ident effect("Effect", fScope, "", team);

        // Find the referenced FX type
        if (FX::Type *fx = FX::Find(effect.GetStr()))
        {
          // Trigger FX on every unit in the tag
          for (MapObjList::Iterator o(&tag->list); *o; o++)
          {
            if ((*o)->Alive())
            {
              fx->Generate(**o);
            }
          }
        }
        else
        {
          fScope->ScopeError("TriggerFX: Unknown FX type [%s]", effect.GetStr());
        }
      }
      else
      {
        fScope->ScopeError("TriggerFX: Could not find tag [%s]", tagName.GetStr());
      }
    }
    else
    {
      fScope->ScopeError("TriggerFX: No tag specified");
    }
  }

  
  //
  // SetUnitLimit
  //
  void SetUnitLimit(Team *team, FScope *fScope)
  {
    team->SetUnitLimit(StdLoad::TypeU32(fScope));
  }


  //
  // WallOperation
  //
  void WallOperation(Team *team, FScope *fScope)
  {
    // Get the tag to toggle
    Param::Ident tagName("Tag", fScope, "", team);

    // What operation are we conducting
    const char *operation = StdLoad::TypeString(fScope, "Operation");
    
    // Find the tag
    if (TagObj *tag = TagObj::FindTag(tagName.GetCrc()))
    {
      // Get the crc of the operation
      U32 crc = Crc::CalcStr(operation);

      // Step through each object in the tag
      for (MapObjList::Iterator o(&tag->list); *o; o++)
      {
        // Is it alive
        if (MapObj *object = (*o)->GetPointer())
        {
          // Promote to a wall
          if (WallObj *wall = Promote::Object<WallObjType, WallObj>(object))
          {
            switch (crc)
            {
              case 0x7619BFA6: // "Deactivate"
                wall->BreakLinks();
                break;

              case 0x01E5156C: // "Activate"
                wall->ActivateIdleLinks(TRUE);
                break;

              case 0x876B6C1F: // "AutoConnect"
                wall->SetupAutoLink();
                break;

              default:
                fScope->ScopeError("WallOperation: unknown operation [%s]", operation);
            }
          }
        }
      }
    }
    else
    {
      fScope->ScopeError("WallOperation could not find tag [%s]", tagName.GetStr());
    }
  }


  //
  // Recycle
  //
  void Recycle(Team *team, FScope *)
  {
    // Iterate all of the units on this team, and the ones which can be recycled, get recycled
    for (NList<GameObj>::Iterator u(&GameObjCtrl::listAll); *u; ++u)
    {
      UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*u);

      if (unit && unit->GetTeam() == team)
      {
        unit->RecycleNow();
      }
    }
  }


  //
  // GameMessage
  //
  void GameMessage(Team *team, FScope *fScope)
  {
    Param::Ident message("Message", fScope, team);
    Message::TriggerGameMessage(message.GetCrc());
  }


  //
  // RegionMessage
  //
  void RegionMessage(Team *team, FScope *fScope)
  {
    Param::Ident message("Message", fScope, team);
    Param::Ident regionName("Region", fScope, team);

    // Resolve the region
    RegionObj *region = RegionObj::FindRegion(regionName);
    if (!region)
    {
      LOG_WARN(("Warp: Region '%s' doesn't exist", regionName.GetStr()))
      return;
    }

    // Get the mid point of the region
    Vector location;
    location.x = region->GetMidPoint().x;
    location.z = region->GetMidPoint().y;
    location.y = TerrainData::FindFloor(location.x, location.z); 

    Message::TriggerLocationMessage(message.GetCrc(), location);
  }


  //
  // TagMessage
  //
  void TagMessage(Team *team, FScope *fScope)
  {
    Param::Ident message("Message", fScope, team);
    Param::Ident tagName("Tag", fScope, team);

    // Resolve Tag
    TagObj *tag = TagObj::FindTag(tagName.GetCrc());
    if (!tag)
    {
      LOG_WARN(("SetTeam: Could not find tag '%s'", tagName.GetStr()))
      return;
    }

    // Get the midpoint of the tag
    Vector location;
    if (tag->GetLocation(location))
    {
      Message::TriggerLocationMessage(Crc::CalcStr(message), location);
    }
  }


  //
  // TriggerTeamRadio
  //
  void TriggerTeamRadio(Team *team, FScope *fScope)
  {
    GameIdent event = StdLoad::TypeString(fScope);


    Param::Ident tagName("Tag", fScope, "", team);
    Param::Ident typeName("Type", fScope, "", team);
    Param::Ident regionName("Region", fScope, "", team);

    if (!typeName.GetCrc() && !regionName.GetCrc() && !tagName.GetCrc())
    {
      team->GetRadio().Trigger(event.crc, Radio::Event());
      return;
    }

    if (typeName.GetCrc())
    {
      // Find the newest unit of this type
      const NList<UnitObj> *types = team->GetUnitObjects(typeName.GetCrc());

      if (types)
      {
        // Get the last item in the list
        UnitObj *unit = types->GetTail();

        ASSERT(unit)
        team->GetRadio().Trigger(event.crc, Radio::Event(unit));
      }
      return;
    }

    if (regionName.GetCrc())
    {
      // Find this region
      RegionObj *region = RegionObj::FindRegion(regionName);
      if (region)
      {
        Vector l;
        l.x = region->GetMidPoint().x;
        l.z = region->GetMidPoint().z;
        l.y = TerrainData::FindFloor(l.x, l.z);
        team->GetRadio().Trigger(event.crc, Radio::Event(l));
      }
      else
      {
        fScope->ScopeError("Could not find region '%s'", regionName.GetStr());
      }
      return;
    }

    if (tagName.GetCrc())
    {
      // Find the tag
      TagObj *tag = TagObj::FindTag(tagName.GetCrc());
      if (tag)
      {
        Vector l;
        if (tag->GetLocation(l))
        {
          team->GetRadio().Trigger(event.crc, Radio::Event(l));
        }
      }
      else
      {
        fScope->ScopeError("Could not find tag '%s'", tagName.GetStr());
      }
    }
  }


  //
  // DisplayObjective
  //
  void DisplayObjective(Team *team, FScope *fScope)
  {
    DisplayObjective::Set &set = team->GetDisplayObjectives();

    GameIdent action = StdLoad::TypeString(fScope);
    GameIdent item = StdLoad::TypeString(fScope);

    // If this team is the display team, manipulate the display objectives display
    IControl *ctrl = (team == Team::GetDisplayTeam()) ? IFace::Find<IControl>("DisplayObjectives") : NULL;

    switch (action.crc)
    {
      case 0x9F1D54D0: // "Add"
        set.AddItem(item, fScope);

        if (ctrl)
        {
          ctrl->Activate();
        }
        break;

      case 0x9A7F366C: // "Remove"
        set.RemoveItem(item);
        break;

      case 0xB69A8789: // "Complete"
        set.CompleteItem(item);
        break;

      case 0xC135A21A: // "Incomplete"
        set.IncompleteItem(item);
        break;

      case 0xABDF9818: // "Abandon"
        set.AbandonItem(item);
        break;

      case 0xB5DE3B44: // "Dump"
        set.DumpItem(item);
        break;

      default:
        fScope->ScopeError("Unknown DisplayObjective action '%s'", action.str);
    }

    // If no objectives left, deactivate the control
    if (ctrl && !set.GetItems().GetCount())
    {
      ctrl->Deactivate();
    }
  }


  //
  // Cineractive
  //
  void Cineractive(Team *team, FScope *fScope)
  {
    Viewer::Action::Execute(team, fScope);
    FSCOPE_DIRTYALL(fScope)
  }


  //
  // ExecuteScript
  //
  void ExecuteScript(Team *team, FScope *fScope)
  {
    // Is this team being controlled by a strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      char nameBuffer[64];

      const char *name = StdLoad::TypeString(fScope);
      if (Crc::CalcStr(name) == 0x58DBEF19) // "Unique"
      {
        Utils::Sprintf(nameBuffer, 64, "Unique[%08x]", team->GetNextScriptId());
        name = nameBuffer;
      }
      else if (Utils::Strlen(name) > 0 && name[Utils::Strlen(name) - 1] == '*')
      {
        char temp[64];
        Utils::Strmcpy(temp, name, Utils::Strlen(name));
        Utils::Sprintf(nameBuffer, 64, "%s[%08x]", temp, team->GetNextScriptId());
        name = nameBuffer;
      }

      const char *config = StdLoad::TypeString(fScope);
      U32 weighting = StdLoad::TypeU32(fScope, 100, Range<U32>(1, U16_MAX));
      U32 priority = StdLoad::TypeU32(fScope, 100, Range<U32>(0, U16_MAX));
      object->GetScriptManager().ExecuteScript(name, config, weighting, priority).Configure(fScope);
    }
  }


  //
  // KillScript
  //
  void KillScript(Team *team, FScope *fScope)
  {
    // Is this team being controlled by a strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      object->GetScriptManager().KillScript(StdLoad::TypeString(fScope));
    }
  }


  //
  // SetScriptWeighting
  //
  void SetScriptWeighting(Team *team, FScope *fScope)
  {
    // Is this team being controlled by a strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      U32 weighting = StdLoad::TypeU32(fScope, Range<U32>(1, U16_MAX));
      object->GetScriptManager().SetScriptWeighting(name, weighting);
    }
  }


  //
  // SetScriptPriority
  //
  void SetScriptPriority(Team *team, FScope *fScope)
  {
    // Is this team being controlled by a strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      U32 priority = StdLoad::TypeU32(fScope, Range<U32>(0, U16_MAX));
      object->GetScriptManager().SetScriptPriority(name, priority);
    }
  }


  //
  // AddBase
  //
  void AddBase(Team *team, FScope *fScope)
  {
    // Is this team being controlled by a strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      const char *config = StdLoad::TypeString(fScope);
      U32 weighting = StdLoad::TypeU32(fScope, 100, Range<U32>(1, U16_MAX));
      U32 priority = StdLoad::TypeU32(fScope, 100, Range<U32>(0, U16_MAX));

      const char *regionName = StdLoad::TypeString(fScope, "Region", NULL);

      RegionObj *region;

      if (regionName)
      {
        region = RegionObj::FindRegion(regionName);
        if (!region)
        {
          fScope->ScopeError("Could not find region '%s'", regionName);
        }
      }
      else
      {
        region = team->GetStartRegion();
        if (!region)
        {
          fScope->ScopeError("Team '%s' doesn't have a start region", team->GetName());
        }
      }

      Point<F32> location = region->GetMidPoint();

      // Get the vector of the centre of the map
      Point<F32> centre;
      centre.x = (WorldCtrl::MetreMapXMin() + WorldCtrl::MetreMapXMax()) * 0.5f;
      centre.z = (WorldCtrl::MetreMapZMin() + WorldCtrl::MetreMapZMax()) * 0.5f;
      centre -= location;
      centre.Normalize();

      F32 orientation = (F32) atan2(centre.z, centre.x);
      orientation = StdLoad::TypeCompassAngle(fScope, "Orientation", ANGLE2COMPASS(orientation));

      object->GetBaseManager().AddBase(name, config, weighting, priority, location, orientation);
    }
  }


  //
  // RemoveBase
  //
  void RemoveBase(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      object->GetBaseManager().RemoveBase(name);
    }
  }


  //
  // SetBaseState
  //
  void SetBaseState(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      const char *state = StdLoad::TypeString(fScope);
      object->GetBaseManager().SetBaseState(name, state);
    }
  }


  //
  // SetBaseWeighting
  //
  void SetBaseWeighting(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      U32 weighting = StdLoad::TypeU32(fScope, Range<U32>(1, U16_MAX));
      object->GetBaseManager().SetBaseWeighting(name, weighting);
    }
  }


  //
  // SetBasePriority
  //
  void SetBasePriority(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      U32 priority = StdLoad::TypeU32(fScope, Range<U32>(0, U16_MAX));
      object->GetBaseManager().SetBasePriority(name, priority);
    }
  }


  //
  // SetBaseOrdererWeighting
  //
  void SetBaseOrdererWeighting(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      const char *orderer = StdLoad::TypeString(fScope);
      U32 weighting = StdLoad::TypeU32(fScope, Range<U32>(1, U16_MAX));
      object->GetBaseManager().SetBaseOrdererWeighting(name, orderer, weighting);
    }
  }


  //
  // SetBaseOrdererPriority
  //
  void SetBaseOrdererPriority(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      const char *orderer = StdLoad::TypeString(fScope);
      U32 priority = StdLoad::TypeU32(fScope, Range<U32>(0, U16_MAX));
      object->GetBaseManager().SetBaseOrdererPriority(name, orderer, priority);
    }
  }


  //
  // AssignBaseConstructor
  //
  void AssignBaseConstructors(Team *team, FScope *fScope)
  {
    // Assign constructors to a base

    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      const char *tag = StdLoad::TypeString(fScope, "Tag", NULL);
      object->GetBaseManager().AssignBaseConstructors(name, tag);
    }
  }


  //
  // AssignBaseUnits
  //
  void AssignBaseUnits(Team *team, FScope *fScope)
  {
    // Assign units to a base

    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      const char *tag = StdLoad::TypeString(fScope, "Tag", NULL);
      object->GetBaseManager().AssignBaseUnits(name, tag);
    }
  }


  //
  // AutoDisruptBase
  //
  void AutoDisruptBase(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      const char *name = StdLoad::TypeString(fScope);
      object->GetBaseManager().AutoDisrupt(name, StdLoad::TypeString(fScope));
    }
  }


  //
  // SetBombardierPreferences
  //
  void SetBombardierPreferences(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      object->GetBombardierManager().SetPreferences(StdLoad::TypeString(fScope));
    }
  }


  //
  // AddIntel
  //
  void AddIntel(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      GameIdent intelName = StdLoad::TypeString(fScope);
      GameIdent configName = StdLoad::TypeString(fScope);
      object->GetIntelManager().AddIntel(intelName, configName);
    }
  }


  //
  // RemoveIntel
  //
  void RemoveIntel(Team *team, FScope *fScope)
  {
    // Is this team being controlled by strategic ai ?
    Strategic::Object *object = team->GetStrategicObject();

    if (object)
    {
      GameIdent intelName = StdLoad::TypeString(fScope);
      object->GetIntelManager().RemoveIntel(intelName);
    }
  }


  //
  // HighlightConstruction
  //
  void HighlightConstruction(Team *team, FScope *fScope)
  {
    if (team == Team::GetDisplayTeam())
    {
      const char *s = StdLoad::TypeString(fScope);
      UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(s);

      if (type)
      {
        Client::SetHiliteConstructType(type);
      }
      else
      {
        LOG_ERR(("HighlightConstruction type not found [%s]", s))
      }
    }
  }


  //
  // UnhighlightConstruction
  //
  void UnhighlightConstruction(Team *team, FScope *)
  {
    if (team == Team::GetDisplayTeam())
    {
      Client::SetHiliteConstructType(NULL);
    }
  }


  //
  // Animate
  //
  void Animate(Team *, FScope *fScope)
  {
    const char *name = StdLoad::TypeString(fScope, "Tag");

    // Apply animation to all units in tag
    TagObj *tag = TagObj::FindTag(name);

    // Load cycle id
    U32 cycleId = StdLoad::TypeStringCrc(fScope, "Cycle");

    if (tag)
    {
      for (MapObjList::Iterator i(&tag->list); *i; i++)
      {
        UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(**i);

        if (unit)
        {
          // Attempt to flush current tasks
          if (unit->FlushTasks(Tasks::UnitAnimate::GetConfigBlockingPriority()))
          {
            // Create the recycle task
            unit->PrependTask(new Tasks::UnitAnimate(unit, cycleId));
          }
        }
      }
    }
    else
    {
      LOG_ERR(("Animate: Tag not found [%s]", name))
    }
  }
}
