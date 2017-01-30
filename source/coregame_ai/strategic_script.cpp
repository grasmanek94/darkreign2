/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Script
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_private.h"
#include "strategic_script.h"
#include "strategic_script_state.h"
#include "strategic_script_manager.h"
#include "strategic_script_recruiter.h"
#include "strategic_object.h"
#include "strategic_transport.h"
#include "orders_squad.h"
#include "orders_game.h"
#include "console.h"
#include "gametime.h"
#include "operation.h"
#include "resolver.h"
#include "varfile.h"
#include "weapon.h"


/////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

#ifdef DEVELOPMENT
  #define CON_TEAM(x) CONSOLE(GetObject().GetTeam()->GetConsoleId(0x0E39AE9E), x ) // "Strategic::Script"
#else
  #define CON_TEAM(x)
#endif

#define SCRIPT_VARCHAR '%'


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{
/*
  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Script::RadioReceiver
  //
  struct Script::RadioReceiver
  {
    // Script which it belongs to
    Script &script;
    
    // Transition to perform when handled
    State::Transition *transition;

    // Script node
    NBinTree<RadioReceiver>::Node nodeScript;

    // Constructor
    RadioReceiver(Script &script, State::Transition *transition)
    : script(script),
      transition(transition)
    {
    }

    // Destructor
    ~RadioReceiver()
    {
      delete transition;
    }

  };
*/

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Script::Substitution 
  //
  struct Script::Substitution : public VarSys::Substitution
  {
    // Constructor
    Substitution() : VarSys::Substitution(SCRIPT_VARCHAR) { }

    // Expansion
    const char *Expand(const char *name, void *context)
    {
      Script *script = NULL;

      switch (*name)
      {
        case '\0':
          // Use the context script
          if (context)
          {
            script = reinterpret_cast<Script *>(context);
          }
          else
          {
            LOG_DIAG(("Expanding '%s': Expected context for team substitution", name))
            return (name);
          }
          break;

        default:

          // Does the name contain a SCRIPT_VARCHAR
          char *ptr = Utils::Strchr(name, SCRIPT_VARCHAR);

          if (context && !ptr)
          {
            // This is another script within the same AI
            script = reinterpret_cast<Script *>(context);
            script = script->GetManager().FindScript(name);
            if (!script)
            {
              LOG_DIAG(("Expanding '%s': Could not find script", name))
            }
          }
          else
          {
            if (ptr)
            {
              ASSERT(ptr - name < MAX_GAMEIDENT)
              char teamName[MAX_GAMEIDENT];

              // The first part is the team name of the AI
              Utils::Strmcpy(teamName, name, ptr - name);

              // Find the team which has this name
              Team *team = Team::Name2Team(teamName);

              if (team)
              {
                // Get the Strategic AI of this team
                Object *object = team->GetStrategicObject();

                if (object)
                {
                  // The second part is the script name
                  script = object->GetScriptManager().FindScript(name);
                  if (!script)
                  {
                    LOG_DIAG(("Expanding '%s': Could not find script '%s' in team '%s'", name, ptr, teamName))
                  }
                }
                else
                {
                  LOG_DIAG(("Expanding '%s': Team '%s' has no AI", name, teamName))
                }
              }
              else
              {
                LOG_DIAG(("Expanding '%s': Could not find team '%s'", name, teamName))
              }

              /*
              script = Script::Name2Script(name);
              if (!script)
              {
                LOG_DIAG(("Expanding '%s': Could not find script", name));
                return (name);
              }
              */
            }
            else
            {
              LOG_DIAG(("Expanding '%s': Expected '%c' to delimit team and script", name, SCRIPT_VARCHAR))
            }
          }
          break;
      }

      return (script->DynVarName());
    }

  };


  //
  // Internal Data
  //
  static Script::Substitution substitution;
  static const char *varScope = "ai.strategic.dyndata";


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script
  //

  
  //
  // Script::Script
  //
  Script::Script(Manager &manager, const char *name, const char *configName, FScope *fScope, U32 weighting, U32 priority) 
  : name(name),
    configName(configName),
    manager(manager),
    weighting(weighting),
    priority(priority),
    states(&State::nodeScript),
//    installed(NULL),
//    receivers(&RadioReceiver::nodeScript),
    reserved(FALSE),
    keep(TRUE),
    locationValid(FALSE),
    recruiter(NULL),
    transports(&Transport::nodeScript),
    threat(NULL),
    defense(NULL)
  {
    RegisterConstruction(dTrack);

    initialState = "";

    CON_TEAM(("%5.1f %s: Created", GameTime::SimTotalTime(), GetName()))
//    LOG_AI(("%5.1f %s: Created", GameTime::SimTotalTime(), GetName()))

    Configure(fScope);

    // Add the installed state (this is only used for installing conditions!)
//    installed = new State(*this, "Installed");

    // Add the CreateSquad state
    currentState = new State(*this, "CreateSquad", State::Action::Create(*this, NULL));
    newState = TRUE;

    // Throw it in the state tree so it will be cleaned up with the rest
    states.Add(0xB45F2AE3, currentState); // "CreateSquad"
  }


  //
  // Script::~Script
  //
  // Destructor
  //
  Script::~Script()
  {
    CON_TEAM(("%5.1f %s: Destroyed", GameTime::SimTotalTime(), GetName()))
//    LOG_AI(("%5.1f %s: Destroyed", GameTime::SimTotalTime(), GetName()))

    // If we have a squad, order its destruction
    if (squad.Alive())
    {
      Orders::Squad::Destroy::Generate(manager.GetObject(), squad.Id());
    }

    // Release any transports we had
    ReleaseTransports();

    // Clear out the stack
    stack.PopAll();

/*
    // Purge installed conditions
    installed->PurgeConditions();

    // Delete the installed state
    delete installed;
*/

    // Delete all of the states
    states.DisposeAll();

    // Delete all the handlers
    //receivers.DisposeAll();

    // Delete threat/defense
    if (threat)
    {
      delete threat;
    }
    if (defense)
    {
      delete defense;
    }

    // Remove all the dynamic vars
    VarSys::DeleteItem(DynVarName());

    RegisterDestruction(dTrack);
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Script::SaveState(FScope *scope)
  {
    StdSave::TypeReaper(scope, "Squad", squad);
    
    for (Stack<State>::Iterator i(stack); *i; i++)
    {
      StdSave::TypeU32(scope, "StatesItem", Crc::CalcStr((*i)->GetName()));
    }

    if (currentState)
    {
      StdSave::TypeU32(scope, "CurrentState", Crc::CalcStr(currentState->GetName()));
    }

    StdSave::TypeU32(scope, "NewState", newState);
    StdSave::TypeU32(scope, "Reserved", reserved);
    StdSave::TypeU32(scope, "Keep", keep);

    if (locationValid)
    {
      StdSave::TypeVector(scope, "Location", location);
    }

    StdSave::TypeU32(scope, "Weighting", weighting);
    StdSave::TypeU32(scope, "Priority", priority);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Script::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xAF55CD8F: // "Squad"
          StdLoad::TypeReaper(sScope, squad);
          Resolver::Object<SquadObj, SquadObjType>(squad);
          break;

        case 0x23BD7B34: // "StatesItem"
        {
          if (State *state = states.Find(StdLoad::TypeU32(sScope)))
          {
            stack.PushToBottom(state);
          }
          break;
        }

        case 0xFE325917: // "CurrentState"
          currentState = states.Find(StdLoad::TypeU32(sScope));
          break;

        case 0x35BC5703: // "NewState"
          newState = StdLoad::TypeU32(sScope);
          break;

        case 0x65D5985D: // "Reserved"
          reserved = StdLoad::TypeU32(sScope);
          break;

        case 0xC774D607: // "Keep"
          keep = StdLoad::TypeU32(sScope);
          break;

        case 0x693D5359: // "Location"
          StdLoad::TypeVector(sScope, location);
          locationValid = TRUE;
          break;

        case 0xA83FA04A: // "Weighting"
          weighting = StdLoad::TypeU32(sScope);
          break;

        case 0xFFF34C6F: // "Priority"
          priority = StdLoad::TypeU32(sScope);
          break;

      }
    }
  }


  //
  // Configure script from a scope
  //
  void Script::Configure(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x631D77FE: // "InitialState"
          initialState = StdLoad::TypeString(sScope);
          break;

        case 0x96880780: // "State"
        {
          const char *name = StdLoad::TypeString(sScope);
          states.Add(Crc::CalcStr(name), new State(*this, name, sScope));
          break;
        }

        case 0x921C808B: // "CreateVarInteger"
        {
          const char *name = StdLoad::TypeString(sScope);
          S32 dVal = S32(StdLoad::TypeU32(sScope, U32(0)));
          S32 minVal = S32(StdLoad::TypeU32(sScope, U32(S32_MIN)));
          S32 maxVal = S32(StdLoad::TypeU32(sScope, U32(S32_MAX)));

          if (*name != SCRIPT_VARCHAR)
          {
            sScope->ScopeError("Only dynamic team vars can be created");
          }

          VarSys::CreateInteger(name, dVal, VarSys::DEFAULT, NULL, this)->SetIntegerRange(minVal, maxVal);
          break;
        }

        case 0xAFF1375D: // "CreateVarFloat"
        {
          const char *name = StdLoad::TypeString(sScope);
          F32 dVal = StdLoad::TypeF32(sScope, 0.0F);
          F32 minVal = StdLoad::TypeF32(sScope, F32_MIN);
          F32 maxVal = StdLoad::TypeF32(sScope, F32_MAX);

          if (*name != SCRIPT_VARCHAR)
          {
            sScope->ScopeError("Only dynamic script vars can be created");
          }

          VarSys::CreateFloat(name, dVal, VarSys::DEFAULT, NULL, this)->SetFloatRange(minVal, maxVal);
          break;
        }

        case 0xA2F8DAA2: // "CreateVarString"
        {
          const char *name = StdLoad::TypeString(sScope);
          const char *dVal = StdLoad::TypeStringD(sScope, "");

          if (*name != SCRIPT_VARCHAR)
          {
            sScope->ScopeError("Only dynamic team vars can be created");
          }

          VarSys::CreateString(name, dVal, VarSys::DEFAULT, NULL, this);
          break;
        }

        case 0xC39EE127: // "Op"
          Operation::Function(sScope, this);
          break;

        default:
          sScope->ScopeError("Unknown function '%s' in script '%s'", sScope->NameStr(), name);
      }
    }
  }


  //
  // Script::SetSquad
  //
  // Set the squad which this script uses
  //
  void Script::SetSquad(SquadObj *squadIn)
  {
    squad = squadIn;
    GotoState(initialState);
  }


  //
  // Script::SetFlag
  //
  // Set flag
  //
  void Script::SetFlag(const GameIdent &flag, Bool value)
  {
    switch (flag.crc)
    {
      case 0x95292449: // "Reserverd"
        reserved = value;
        break;

      case 0xC774D607: // "Keep"
        keep = value;
        break;

      default:
        ERR_CONFIG(("Unknown flag '%s'", flag.str))
    }
  }


  //
  // Process
  //
  // Process the script
  //
  void Script::Process()
  {
    if (newState)
    {
      // Clear new state flag
      newState = FALSE;

      // Enter the new state
      currentState->Enter();
    }
    else
    {
//      installed->Process();
      currentState->Process();
    }
  }


  //
  // Notify
  //
  // Notify the script
  //
  void Script::Notify(U32 message, U32 data)
  {
    currentState->Notify(message, data);
  }


  //
  // GetSquad
  //
  // Get the squad associated with this script
  //
  SquadObj * Script::GetSquad(Bool required)
  {
    if (required)
    {
      if (squad.Alive())
      {
        return (squad);
      }
      else
      {
        ERR_FATAL(("Squad is dead in script '%s'", GetName()))
      }
    }
    else
    {
      if (squad.Alive())
      {
        return (squad);
      }
      else
      {
        return (NULL);
      }
    }
  }


  //
  // GetObject
  //
  // Get the strategic object who owns this script
  //
  Object & Script::GetObject()
  {
    return (manager.GetObject());
  }


  //
  // End
  //
  // End the script
  //
  void Script::End()
  {
    // Notify the manager that this script has ended
    manager.NotifyScriptEnded(this);
  }


  //
  // GotoState
  //
  // Goto the given state
  //
  void Script::GotoState(const GameIdent &state)
  {
    CON_TEAM(("%5.1f %s: Entering State '%s'", GameTime::SimTotalTime(), GetName(), state.str))
    LOG_AI(("%5.1f %s: Entering State '%s'", GameTime::SimTotalTime(), GetName(), state.str))

    currentState = states.Find(state.crc);
    newState = TRUE;

    if (!currentState)
    {
      ERR_CONFIG(("Could not find state '%s' in script '%s'", state.str, GetName()))
    }
  }


  //
  // CallState
  //
  // Call the given state
  //
  void Script::CallState(const GameIdent &state)
  {
    // Push the current state onto the stack
    stack.Push(currentState);

    // Goto the given state
    GotoState(state);
  }

 
  //
  // ReturnState
  //
  // Return from the current state
  //
  void Script::ReturnState()
  {
    // Pop the current state from the stack
    currentState = stack.Pop();
    newState = TRUE;

    // Make sure there's a state
    if (!currentState)
    {
      ERR_CONFIG(("Callstack underflow in script '%s'", GetName()))
    }
  }


/*
  //
  // GetInstallState
  //
  // Get the state which has the installed conditions
  //
  Script::State & Script::GetInstallState()
  {
    ASSERT(installed)
    return (*installed);
  }


  //
  // InstallRadioReceiver
  //
  // Install a Radio Receiver
  //
  void Script::InstallRadioReceiver(const GameIdent &ident, const GameIdent &event, FScope *transition)
  {
    // Create the handler data
    RadioReceiver *receiver = new RadioReceiver(*this, State::Transition::Create(transition));

    // Save the handler in the list
    receivers.Add(ident.crc, receiver);

    // Install the receiver into the team radio system
    ASSERT(GetObject().GetTeam())
    GetObject().GetTeam()->GetRadio().InstallReceiver(event.crc, ReceiverHandler, receiver);
  }


  //
  // UnInstallRadioReceiver
  //
  // UnInstall a Radio Receiver
  //
  void Script::UnInstallRadioReceiver(const GameIdent &ident)
  {
    // Find the receiver with this ident
    RadioReceiver *receiver = receivers.Find(ident.crc);

    if (receiver)
    {
      GetObject().GetTeam()->GetRadio().UnInstallReceiver(receiver);
      receivers.Dispose(receiver);
    }
  }


  //
  // PurgeRadioReceivers
  //
  // Purge all radio receivers
  //
  void Script::PurgeRadioReceivers()
  {
    NBinTree<RadioReceiver>::Iterator r(&receivers);
    RadioReceiver *receiver;

    while ((receiver = r++) != NULL)
    {
      GetObject().GetTeam()->GetRadio().UnInstallReceiver(receiver);
      receivers.Dispose(receiver);
    }
  }
*/

  //
  // Build the dyn var name for a script
  //
  const char * Script::DynVarName()
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
    p = GetObject().GetTeam()->GetName();
    while (*p)
    {
      *s++ = *p++;
    }
    *s++ = VARSYS_SCOPEDELIM;

    // Copy the script name
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
  // Add a unit to the script
  //
  void Script::AddUnit(UnitObj *unit)
  {
    ASSERT(unit)

    if (GetSquad())
    {
      Orders::Game::ClearSelected::Generate(GetObject());
      Orders::Game::AddSelected::Generate(GetObject(), unit);

      // If the squad has a location, select the unit and tell it to move to where the squad is
      Vector location;
      if (GetSquad()->GetLocation(location))
      {
        Orders::Game::Move::Generate(GetObject(), location, FALSE, Orders::FLUSH);
      }

      // Add the unit to the squad
      Orders::Squad::AddSelected::Generate(GetObject(), GetSquad()->Id());
    }
    else
    {
      LOG_WARN(("Adding a unit [%d] '%s' to a script '%s' which has no squad!", unit->TypeName(), unit->Id(), GetName()))
    }
  }


  //
  // Assign a transport to this squad
  //
  void Script::AssignTransport(Transport &transport)
  {
    // Add the transport to the transports list
    transports.Append(&transport);
  }


  //
  // Remove a transport from this squad
  //
  void Script::RemoveTransport(Transport &transport)
  {
    // Remove the transport from the transports list
    transports.Unlink(&transport);
  }


  //
  // Send trasports back to base
  //
  void Script::SendTransports()
  {
    for (NList<Transport>::Iterator t(&transports); *t; t++)
    {
      // Tell the transport to return
      (*t)->Return();
    }
  }


  //
  // Send trasports back to base
  //
  void Script::SendTransports(const Vector &location)
  {
    for (NList<Transport>::Iterator t(&transports); *t; t++)
    {
      // Clear the transport's flag
      (*t)->ClearFlag();

      // Set the location to return to
      (*t)->SetLocation(location);

      // Tell the transport to return
      (*t)->Return();
    }
  }


  //
  // Release transports back to the transport manager
  //
  void Script::ReleaseTransports()
  {
    NList<Transport>::Iterator t(&transports);
    Transport *transport;
    
    while ((transport = t++) != NULL)
    {
      // Remove the transport fromt this squad
      transport->RemoveFromSquad(this);
    }
  }


  //
  // Get defense/threat composition (by armour class / weapon type)
  //
  Bool Script::GetThreatDefense(U32 *&t, U32 *&d, U32 &tt, U32 &td)
  {
    if (!squad.Alive())
    {
      return (FALSE);
    }

    if (!threat)
    {
      threat = new U32[ArmourClass::NumClasses()];
      defense = new U32[ArmourClass::NumClasses()];
    }

    t = threat;
    d = defense;

    tt = 0;
    td = 0;

    Utils::Memset(threat, 0x00, sizeof (U32) * ArmourClass::NumClasses());
    Utils::Memset(defense, 0x00, sizeof (U32) * ArmourClass::NumClasses());

    // Iterate the units and add their threats and defense
    for (SquadObj::UnitList::Iterator u(&squad->GetList()); *u; ++u)
    {
      if ((*u)->Alive())
      {
        UnitObj *unit = **u;

        // Add the defense of this unit
        defense[unit->UnitType()->GetArmourClass()] += unit->GetHitPoints();
        td += unit->GetHitPoints();

        Weapon::Object *weapon = unit->GetWeapon();

        if (weapon)
        {
          // Add the threat of this unit
          for (U32 a = 0; a < ArmourClass::NumClasses(); a++)
          {
            threat[a] += weapon->GetType().GetThreat(a);
            tt += weapon->GetType().GetThreat(a);
          }
        }
      }
    }
    return (TRUE);
  }


  //
  // NotifyAsset
  //
  // Notififcation that an asset reshuffle is pending
  //
  void Script::NotifyAsset()
  {
    // Do we want to keep the assets we've got ?
    if (keep)
    {
      Object &object = GetObject();

      if (recruiter)
      {
        // Submit a request 
        recruiter->Execute(*this, manager.GetRecruitId());
      }
      else
      {
        // Submit a request to keep what we got ... (only if we've got units)
        if (GetSquad() && GetSquad()->GetList().GetCount())
        {
          object.GetAssetManager().SubmitRequest(*new Asset::Request::Squad(this, 0, GetSquad()), weighting, priority);
        }
      }
    }
  }


  //
  // Initialization
  //
  void Script::Init()
  {
    // Register the var substitution for teams
    VarSys::RegisterSubstitution(substitution);
  }


  //
  // Shutdown
  //
  void Script::Done()
  {
    // Unregister the var substitution for teams
    VarSys::UnregisterSubstitution(substitution);
  }


  //
  // Save
  //
  // Save data
  //
  void Script::Save(FScope *scope)
  {
    // Find the dynamic data scope
    if (VarSys::VarScope *v = VarSys::FindVarScope(varScope))
    {  
      VarFile::Save(scope->AddFunction("DynData"), v);
    }   
  }


  //
  // Load
  //
  // Load data
  //
  void Script::Load(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xB69043F0: // "DynData"
          VarFile::Load(sScope, varScope);
          break;
      }
    }
  }

/*
  //
  // Event handler
  //
  void Script::ReceiverHandler(U32, void *context, const Vector &vector, Bool valid)
  {
    RadioReceiver *receiver = static_cast<RadioReceiver *>(context);

    // Save the location
    receiver->script.location = vector;
    receiver->script.locationValid = valid;

    // Perform the transition instituted by the radio receiver
    receiver->transition->Perform(receiver->script);
  }
*/

}