///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client side systems
//
// 11-AUG-1998
//

#ifndef __CLIENT_PRIV_H
#define __CLIENT_PRIV_H



///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client.h"
#include "mapobjctrl.h"
#include "orders_game.h"
#include "console.h"
#include "main.h"
#include "perfstats.h"
#include "pathsearch.h"
#include "common_gamewindow.h"
#include "unitobjdec.h"
#include "footprint.h"
#include "terraindata.h"
#include "offmapobj.h"
#include "unitobjinfo.h"
#include "iface_util.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Client - Controls client side interface
//

namespace Client
{
  // Forward declarations
  class Construction;
  class UnitContext;
  class Facility;

  // Enumeration of all 'forced' client modes
  enum ClientMode
  {
    CM_NONE,
    CM_MOVE,
    CM_ATTACK,
    CM_MOVEATTACK,
    CM_ATTACKNOMOVE,
    CM_TURN,
    CM_JUMPSCROLL,
    CM_FORMATION,
    CM_SETRESTORE,
    CM_UNLOADCARGO,
    CM_SETRALLY,
    CM_CONSTRUCT,
    CM_GUARD,
    CM_PLAYERMARKER,

    // Maximum value
    CM_MAX
  };

  // Enumeration of all primary client events
  enum PrimaryEvent
  {
    PE_NONE,
    PE_SELECT,
    PE_MOVE,
    PE_NOMOVE,
    PE_ATTACK,
    PE_NOATTACK,
    PE_MOVEATTACK,
    PE_ATTACKNOMOVE,
    PE_TURN,
    PE_COLLECT,
    PE_STORE,
    PE_JUMPSCROLL,
    PE_CONSTRUCT,
    PE_NOCONSTRUCT,
    PE_GUARD,
    PE_NOGUARD,
    PE_FORMATION,
    PE_SETRESTORE,
    PE_NOSETRESTORE,
    PE_USEOFFMAP,
    PE_NOUSEOFFMAP,
    PE_RESTOREMOBILE,
    PE_RESTORESTATIC,
    PE_BOARD,
    PE_TRANSPORT,
    PE_UNLOADCARGO,
    PE_NOUNLOADCARGO,
    PE_SETRALLY,
    PE_NOSETRALLY,
    PE_TRAILSELECT,
    PE_TRAILPOINT,
    PE_TRAILNOPOINT,
    PE_TRAILAPPLY,
    PE_WALLACTIVATE,
    PE_WALLNOACTIVATE,
    PE_WALLDEACTIVATE,
    PE_MORPH,
    PE_NOMORPH,
    PE_INFILTRATE,
    PE_NOINFILTRATE,
    PE_PLAYERMARKER,
    PE_NOPLAYERMARKER,

    // Maximum value
    PE_MAX
  };

  // Enumeration of all capture state event types
  enum CaptureEvent
  {
    CSE_INIT,
    CSE_MOUSEMOVE,
    CSE_DONE,
    CSE_ABORT,

    // Maximum value
    CSE_MAX
  };

  // Enumeration of group selection modes
  enum SelectMode
  {
    SM_INACTIVE,
    SM_PREDRAW,
    SM_POSTDRAW
  };

  // Capture state handler function
  typedef void (CaptureCallBack)(CaptureEvent event);

  // Information setup each event processing cycle
  struct CycleInfo
  {
    // Current mouse position in screen pixels (always updated)
    Point<S32> mouse;

    // Number of objects in the last display list (diagnostics only)
    U32 displayListCount;

    // Valid if a single unit object is selected
    UnitObjPtr oneUnit;

    // Game window mouse is over (if NULL, all info below is invalid)
    Reaper<Common::GameWindow> gameWnd;

    // The primary event type for this cycle
    PrimaryEvent pEvent;

    // Info for the object currently under the mouse OR 'mapObj' is NULL
    struct ObjectInfo
    { 
      MapObjPtr map; 
      UnitObjPtr unit;
    } o;

    // Terrain position the mouse is over OR 'cell' is NULL
    struct TerrainInfo
    { 
      Cell *cell;
      Vector pos; 
      TerrainData::Cell *dataCell;
      U32 cellX, cellZ;
      Bool seen, visible;
    } t;
  };

  // Client interface controls
  struct InterfaceControls
  {
    // The construction context menus
    Reaper<Construction> construction;

    // The context menus
    Reaper<UnitContext> context;

    // The orders menu
    Reaper<IControl> orders;

    // The facility shortcut bar
    Reaper<Facility> facility;

    // The trails menu
    IControlPtr trails;
  };


  // Private system-wide data structure
  struct SystemData
  {
    // Should events be processed
    Bool processing;

    // Event processing cycle counter and last update
    U32 cycleCount, lastUpdate;

    // Current forced client mode
    ClientMode clientMode;

    // Current group selection mode
    SelectMode selectMode;

    // Current capture state callback
    CaptureCallBack *captureHandler;

    // Current capture code (which mouse button has capture)
    S16 captureCode;

    // Starting position of a mouse click
    Point<S32> mouseStart;

    // Area of selection box
    Area<S32> mouseRect;

    // Current list of selected objects
    UnitObjList sList;

    // Information about each selected object
    UnitObjInfo *sListInfo;

    // List of all rigs on the display team
    UnitObjList rigs;

    // Currently selected squad
    SquadObjPtr squad;

    // Current region being painted
    RegionObjPtr paintRegion;

    // Processing cycle information
    CycleInfo cInfo;

    // Var pointers
    VarInteger trackObject;
    VarInteger trackDistance;
    VarInteger squadReset;
    VarInteger giveAmount;
    VarInteger sundialDir;
    VarInteger fullSquadHud;

    // Unit type for a rig to construct
    UnitObjTypePtr constructType;

    // Unit type that is highlighted on construct menu
    UnitObjTypePtr hiliteConstructType;

    // Data for footprint placement
    WorldCtrl::CompassDir dir;
    FootPrint::Placement placeFoot;
    Matrix placeLocation;

    // Data for formation placement
    U32 formation;

    // Alpha scale for pulsating client thingies
    IFace::PulsingValue clientAlpha;

    // Client interface controls
    InterfaceControls controls;
  };

  // Private system-wide data instance
  extern SystemData data;

  // Returns the first selected unit
  UnitObj * FirstSelected();

  // Command sub-system
  namespace Cmd
  {
    void Init();
    void Done();
  }

  // Interface controls sub-system
  namespace Controls
  {
    void Init();
    void Done();
  }

  // Groups system
  namespace Group
  {
    // Group operations
    enum Operation
    {
      O_NEWGROUP,
      O_NEWGROUP_REMOVE,
      O_SELECTGROUP,
      O_SELECTGROUP_SCROLL,
      O_CLEAR,

      // Maximum value
      O_MAX
    };

    // Perform a group operation
    Bool DoOperation(U32 group, Operation operation);

    // Initialize and shutdown system
    void Init();
    void Done();

    // Save and load groups
    void Save(FScope *scope);
    void Load(FScope *scope);
  }

  // Trail system
  namespace Trail
  {
    // Is the system currently active
    Bool Active();

    // Returns the primary event
    PrimaryEvent GetPrimaryEvent();

    // Triggers the given event
    Bool TriggerEvent(U32 event, U32 value = 0);

    // Start trail construction
    void StartConstruction(TrailObj::Mode m);

    // Display trails
    void Render();

    // Initialize and shutdown system
    void Init();
    void Done();
  }

  // Display sub-system
  namespace Display
  {
    // Multiplayer markers
    struct PlayerMarker
    {
      // World cell position
      S32 cx, cz;

      // Allowed to see this (ally team)?
      U32 canSee : 1;

      // World metre position, setup in RenderPlayerMarker
      Vector pos;

      // Player this marker belongs to
      const Player *player;

      NBinTree<PlayerMarker>::Node node;
    };
    extern NBinTree<PlayerMarker> markers;

    // Selection box skin
    extern TextureSkin *selectionSkin;

    void Init();
    void Done();
  }

  // Event sub-system
  namespace Events
  {
    void Init();
    void Done();

    // Update the previous selected list
    void UpdatePreviousSelected(Bool notify);

    // Select a single unit, using all modifier keys
    void SelectUnit(UnitObj *unit);

    // Set the current selection using the given list
    void SelectList(const UnitObjList &list, Bool scroll);

    // Set the current selection using the given squad
    void SelectSquad(SquadObj *squad, Bool scroll);

    // Called when the user selects units
    void UnitsSelected();

    // Send the order to change the list of selected units if they have changed
    void UpdateSelectedLists();

    // Trigger the given response event
    void TriggerResponse(const UnitObjList &list, U32 event);

    // Handler for discrete events
    Bool HandleDiscreteEvent(void *context, U32 crc, U32 param1, U32 param2);

    // Sets the current forced client mode, or reverts to none
    void TriggerClientMode(ClientMode mode);

    // Find 'name' and call above method, FALSE if not found
    Bool TriggerClientMode(const char *name);

    // Modify tactical setting of selected squad/units
    void ModifyTacticalSetting(U8 modifier, U8 setting);

    // Game window event handler
    Bool FASTCALL GameWindowHandler(Event &e, Common::GameWindow *gameWindow);
  }

  // Debug sub-system
  namespace Debug
  {
    void Init();
    void Render();
    void Done();
  }
}

#endif