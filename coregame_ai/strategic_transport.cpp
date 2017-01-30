/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Transport
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_transport.h"
#include "strategic_script.h"
#include "strategic_base.h"
#include "strategic_object.h"
#include "orders_game.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Transport
  //


  //
  // Constructor
  //
  Transport::Transport(TransportObj *transport, Transport::Manager &manager)
  : Reaper<TransportObj>(transport),
    manager(manager),
    script(NULL),
    flag(FALSE)
  {
    location = transport->Origin();
  }


  //
  // Destructor
  //
  Transport::~Transport()
  {
    if (script)
    {
      // Remove from the script
      script->RemoveTransport(*this);
      script = NULL;
    }
  }

  
  //
  // SaveState
  //
  // Save state information
  //
  void Transport::SaveState(FScope *scope)
  {
    StdSave::TypeVector(scope, "Location", location);

    if (script)
    {
      StdSave::TypeU32(scope, "Script", script->GetNameCrc());
    }

    StdSave::TypeU32(scope, "Flag", flag);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Transport::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x693D5359: // "Location"
          StdLoad::TypeVector(sScope, location);
          break;

        case 0x8810AE3C: // "Script"
          script = manager.GetObject().GetScriptManager().FindScript(StdLoad::TypeU32(sScope));

          if (script)
          {
            AssignToSquad(script);
          }
          break;

        case 0x8669FADC: // "Flag"
          flag = StdLoad::TypeU32(sScope);
          break;
      }
    }
  }


  //
  // Return this transport to its
  //
  void Transport::Return()
  {
    // Select the transport
    Orders::Game::ClearSelected::Generate(manager.GetObject());
    Orders::Game::AddSelected::Generate(manager.GetObject(), *this);

    // Move it to the base
    Orders::Game::Move::Generate(manager.GetObject(), location, FALSE, Orders::FLUSH);
  }

  
  //
  // Assign this base to a squad
  //
  void Transport::AssignToSquad(Script *squad)
  {
    ASSERT(squad)
    script = squad;

    // Assign the transport to the script
    script->AssignTransport(*this);

    // Tell the manager to activate us
    manager.ActivateTransport(*this);
  }


  //
  // Remove this base from a squad
  //
  void Transport::RemoveFromSquad(Script *squad)
  {
    ASSERT(squad)
    ASSERT(script == squad)

    // Remove the transport from the script
    script->RemoveTransport(*this);

    // Tell the manager to deactivate us
    manager.DeactivateTransport(*this);

    // Clear the script
    script = NULL;
  }

}

