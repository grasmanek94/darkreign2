/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Script
// 25-MAR-1999
//


#ifndef __STRATEGIC_SCRIPT_H
#define __STRATEGIC_SCRIPT_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "squadobj.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //
  class Transport;
  class Base;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script
  //
  class Script
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Manager;
    class State;
    class Recruiter;
    //struct RadioReceiver;
    struct Substitution;

  private:

    // Name of the script
    GameIdent name;

    // Name of the script config
    GameIdent configName;

    // Reaper to the squad this script is controlling
    SquadObjPtr squad;

    // Script manager which is managing this script
    Manager &manager;

    // Manager Node
    NBinTree<Script>::Node nodeManager;
    NBinTree<Script>::Node nodeManagerSquad;

    // States in the script
    NBinTree<State> states;

  /*
    // State which contains installed conditions
    State *installed;
  */

    // Stack
    Stack<State> stack;

    // Initial state name
    GameIdent initialState;

    // Current state
    State *currentState;

    // Flags
    U32 newState : 1,
        reserved : 1,
        keep     : 1;

    // Weighting
    U32 weighting;

    // Priority
    U32 priority;

  /*
    // Team Radio Recivers
    NBinTree<RadioReceiver> receivers;
  */

    // Location of the last received message
    Vector location;
    Bool locationValid;

    // Recruiter
    Recruiter *recruiter;

    // Transports which are under the control of this squad
    NList<Transport> transports;


    // Threat break down by armour class for the squad
    U32 *threat;

    // Defense break down by armour class for the squad
    U32 *defense;

  public:

    // Death track info
    DTrack::Info dTrack;

  public:

    // Constructor and destructor
    Script(Manager &manager, const char *name, const char *configName, FScope *fScope, U32 weighting, U32 priority);
    ~Script();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Configure script from a scope
    void Configure(FScope *fScope);

    // Set the squad which this script uses
    void SetSquad(SquadObj *squad);

    // Set flag
    void SetFlag(const GameIdent &flag, Bool value);

    // Process the script
    void Process();

    // Notify
    void Notify(U32 message, U32 data = 0);

    // Get the squad associated with this script
    SquadObj * GetSquad(Bool required = TRUE);

    // Get the strategic object who owns this script
    Object & GetObject();

    // End the script
    void End();

    // Goto the given state
    void GotoState(const GameIdent &state);

    // Call the given state
    void CallState(const GameIdent &state);

    // Return from the current state
    void ReturnState();

  /*
    // Get the state which has the installed conditions
    State & GetInstallState();

    // Install a radio receiver
    void InstallRadioReceiver(const GameIdent &ident, const GameIdent &event, FScope *trans);

    // Uninstall a radio receiver
    void UnInstallRadioReceiver(const GameIdent &ident);

    // Purge radio receivers
    void PurgeRadioReceivers();
  */

    // Build the dyn var name for a script
    const char * DynVarName();

    // Add a unit to the script
    void AddUnit(UnitObj *unit);

    // Assign a transport to this squad
    void AssignTransport(Transport &transport);

    // Remove a transport from this squad
    void RemoveTransport(Transport &transport);

    // Send transport back to base
    void SendTransports();

    // Send transport back to base
    void SendTransports(const Vector &location);

    // Release transports back to the transport manager
    void ReleaseTransports();


    // Get defense/threat composition (by armour class / weapon type)
    Bool GetThreatDefense(U32 *&threat, U32 *&defense, U32 &totalThreat, U32 &totalDefense);

  public:

    // Notififcation that an asset reshuffle is pending
    void NotifyAsset();

  public:

    // Get the name of the script
    const char * GetName()
    {
      return (name.str);
    }

    // Get the config name of the script
    const char * GetConfigName()
    {
      return (configName.str);
    }

    // Get the crc of the script name
    U32 GetNameCrc()
    {
      return (name.crc);
    }

    // Get the game ident of the script
    const GameIdent & GetIdent()
    {
      return (name);
    }

    // Get the script manager in control of this script
    Manager & GetManager()
    {
      return (manager);
    }

    // Is the script in resered mode ?
    Bool IsReserved()
    {
      return (reserved);
    }

    // Get the weighting of the script
    U32 GetWeighting()
    {
      return (weighting);
    }

    // Get the priority of the script
    U32 GetPriority()
    {
      return (priority);
    }

    // Set the weighting of the script
    void SetWeighting(U32 value)
    {
      weighting = value;
    }

    // Set the priority of the script
    void SetPriority(U32 value)
    {
      priority = value;
    }

    // Get the location of the last handled message
    Bool GetLocation(Vector &vector)
    {
      if (locationValid)
      {
        vector = location;
        return (TRUE);
      }
      return (FALSE);
    }

    // Set this scripts recruiter
    void SetRecruiter(Recruiter *newRecruiter)
    {
      recruiter = newRecruiter;
    }

    // Get the recruiter
    Recruiter * GetRecruiter()
    {
      return (recruiter);
    }

    // Get the transports
    const NList<Transport> & GetTransports()
    {
      return (transports);
    }

    // Is this scripts squad alive ?
    Bool IsSquadAlive()
    {
      return (squad.Alive());
    }

    // Get the current state of the script
    State * GetCurrentState()
    {
      return (currentState);
    }

  public:

    // Initialization
    static void Init();

    // Shutdown
    static void Done();

    // Save and load data
    static void Save(FScope *scope);
    static void Load(FScope *scope);

    // Receiver Handler
    static void ReceiverHandler(U32 id, void *context, const Vector &location, Bool valid);

  public:

    // Friends
    friend class Manager;

  };

}

#endif
