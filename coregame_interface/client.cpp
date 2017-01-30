///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client side systems
//
// 11-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "client_private.h"
#include "varsys.h"
#include "input.h"
#include "game.h"
#include "orders_game.h"
#include "team.h"
#include "client_clustermap.h"
#include "particlesystem.h"
#include "viewer.h"
#include "iface.h"
#include "common.h"
#include "common_gamewindow.h"
#include "common_mapwindow.h"
#include "mapobj.h"
#include "gameobjctrl.h"
#include "client_construction.h"
#include "client_unitcontext.h"
#include "client_facility.h"
#include "client_squadcontrol.h"
#include "taskctrl.h"
#include "tasks_unitconstructor.h"
#include "multiplayer_data.h"
#include "resolver.h"
#include "message.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Client - Controls client side interface
//

namespace Client
{
  // Has the system been initialized
  static Bool initialized = FALSE;

  // System-wide private data
  SystemData data;

  // GUI event hook
  static void GUIHook(U32 id, UINT msg, WPARAM wParam, LPARAM lParam);

  
  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized)

    // System now initialized
    initialized = TRUE;

    // Initialize sub-systems
    Cmd::Init();
    Controls::Init();

    #ifdef DEVELOPMENT
      Debug::Init();
    #endif

    // Initilalise camera system
    Viewer::Init();

    // Initialize common simulation
    Common::InitSim();

    // Make sure the display team is setup
    if (!Team::GetDisplayTeam())
    {
      ERR_FATAL(("Cannot play a mission which has no Team!"))
    }

    // turn off specularity
    Vid::renderState.status.specular = FALSE;
    Vid::SetRenderState();

    // Register game window event handler
    Common::GameWindow::RegisterHandler(Events::GameWindowHandler);

    // Create the main game window
    Common::GameWindow *ctrl = new Common::GameWindow(IFace::RootWindow());
    ctrl->SetName("MainWindow");
    ctrl->SetGeometry("ParentWidth", "ParentHeight", NULL);
    ctrl->SetZPos(0xFFFFFFFF);
    ctrl->Activate(); 

    // If we have a player then move to this player's team's start location
    if (Team::GetDisplayTeam())
    {
      Team::GetDisplayTeam()->MoveToStart();
    }

    // Initialize sub-systems
    Events::Init();
    Display::Init();
    Group::Init();
    Trail::Init();
    HUD::Init();

    // Initialise map window to inform it of the client's team
    Common::MapWindow::Init(&data.clientAlpha);

    // Register GUI hooks
    Main::RegisterGUIHook("Activate", GUIHook);
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized)

    // Unregister GUI hooks
    Main::UnregisterGUIHook("Activate", GUIHook);

    // Shutdown sub-systems
    HUD::Done();
    Trail::Done();
    Group::Done();
    Display::Done();
    Events::Done();

    // Clean up cluster maps
    ClusterMap::DeleteMaps();

    // Shutdown camera system
    Viewer::Done();

    // Clear game window event handler
    Common::MapWindow::Done();
    Common::GameWindow::RegisterHandler(NULL);

    // Shutdown common systems
    Common::DoneSim();

    // Shutdown sub-systems
    #ifdef DEVELOPMENT
      Debug::Done();
    #endif

    Controls::Done();
    Cmd::Done();

    // System now shutdown
    initialized = FALSE;
  }


  //
  // Initialized
  //
  // Is the system initialized
  //
  Bool Initialized()
  {
    return (initialized);
  }


  //
  // Prepare
  //
  // Called after configuration files are processed
  //
  void Prepare()
  {
    ASSERT(initialized)

    // Setup control reapers
    data.controls.construction = IFace::Find<Construction>("Client::Construction");
    data.controls.context = IFace::Find<UnitContext>("Client::Context");
    data.controls.facility = IFace::Find<Facility>("Client::Facility"); 
    data.controls.orders = IFace::Find<IControl>("Client::Orders"); 
    data.controls.trails = IFace::Find<IControl>("Client::Trails"); 

    // Notify client of all current objects
    for (NList<GameObj>::Iterator i(&GameObjCtrl::listAll); *i; i++)
    {
      Notify(*i, 0x0A43EBAB); // "Game::Exists"
    }
  }


  //
  // Save
  //
  // Save state
  //
  void Save(FScope *scope)
  {
    ASSERT(initialized)

    // Save the selected object list
    StdSave::TypeReaperList(scope, "SelectedList", data.sList);

    // Save the relative mouse position
    Point<F32> p
    (
      F32(Input::MousePos().x) / F32(IFace::ScreenWidth()), 
      F32(Input::MousePos().y) / F32(IFace::ScreenHeight())
    );

    StdSave::TypePoint(scope, "MousePosition", p);

    SquadControl::Save(scope->AddFunction("SquadControls"));

    Group::Save(scope->AddFunction("Group"));
  }


  //
  // Load
  //
  // Load state
  //
  void Load(FScope *scope)
  {
    ASSERT(initialized)

    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xA620A474: // "SelectedList"
        {
          // Load the selected object list
          UnitObjList list;
          StdLoad::TypeReaperList(sScope, list);
          Resolver::ObjList<UnitObj, UnitObjType, UnitObjListNode>(list);
          Events::SelectList(list, FALSE);
          list.Clear();
          break;
        }

        case 0x8E0E412B: // "MousePosition"
        {
          Point<F32> p;
          StdLoad::TypePoint(sScope, p);
          Input::SetMousePos(S32(p.x * IFace::ScreenWidth()), S32(p.y * IFace::ScreenHeight()));
          break;
        }

        case 0xA88CEBFF: // "SquadControls"
          SquadControl::Load(sScope);
          break;

        case 0x769A103E: // "Group"
          Group::Load(sScope);
          break;
      }
    }
  }


  //
  // GetPlayer
  //
  // Returns a reference to the current player
  //
  Player & GetPlayer()
  {  
    if (!Player::GetCurrentPlayer())
    {
      ERR_FATAL(("No player was created, mission probably needs a team!"));
    }

    return (*Player::GetCurrentPlayer());
  }


  //
  // FirstSelected
  //
  // Get the first selected object, or NULL (DEBUGGING ONLY!)
  //
  UnitObj * FirstSelected()
  {
    ASSERT(initialized);
    data.sList.PurgeDead();
    return (data.sList.GetFirst());
  }

  
  //
  // MouseOver
  //
  // Get the object the mouse is over, or NULL (DEBUGGING ONLY!)
  //
  MapObj * MouseOver()
  {
    ASSERT(initialized);
    
    if (data.cInfo.gameWnd.Alive() && data.cInfo.o.map.Alive())
    {
      return (data.cInfo.o.map);
    }

    return (NULL);
  }

  //
  // DataObj
  //
  // ??? Get the object the mouse is over, or NULL (DEBUGGING ONLY!)
  //
  MapObj * DataObj()
  {
    ASSERT(initialized);
    
    return (data.cInfo.o.map.Alive() ? data.cInfo.o.map.GetData() : NULL);
  }


  //
  // Notify
  //  
  // Notify the client of a game event (TRUE if accepted)
  //
  Bool Notify(GameObj *from, U32 message, U32 param1, U32 param2)
  {
    if (Initialized())
    {
      switch (message)
      {
        case 0x0A43EBAB: // "Game::Exists"
        case 0x0DD0BE24: // "Unit::SetTeam"
        {
          ASSERT(from)

          // Promote to a unit
          if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(from))
          {
            // Is it a unit on the display team
            if (unit->GetTeam() == Team::GetDisplayTeam())
            {
              // Does this unit want to be added to the facility bar
              if (data.controls.facility.Alive() && unit->HasProperty(0xBC346FB5)) // "Client::FacilityBar"
              {
                data.controls.facility->Add(unit);
              }

              // Is it a constructor
              if (unit->HasProperty(0xDCDE71CD)) // "Ability::Construction"
              {
                // Is it a rig
                if (!TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(unit))
                {
                  data.rigs.AppendNoDup(unit);
                }
              }
            }
          }

          return (TRUE);
        }

        case 0x8AA808B7: // "Squad::Created"
        case 0x2EC81EFB: // "Squad::UnitsAdded"
          return (SquadControl::Notify(from, message, param1, param2));
      }
    }

    // Event not accepted
    return (FALSE);
  }


  //
  // SetHiliteConstructType
  //
  // Set highlighted construction type
  //
  void SetHiliteConstructType(UnitObjType *type)
  {
    data.hiliteConstructType = type;
  }


  //
  // GUI event hook
  //
  static void GUIHook(U32 id, UINT, WPARAM wParam, LPARAM)
  {
    switch (id)
    {
      case 0x01E5156C: // "Activate"
      {
        if (wParam)
        {
          // Catch game time up to real time after the window is reactivated
          GameTime::Readjust();
        }
        break;
      }
    }
  }
}

