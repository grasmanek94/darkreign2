///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client/Editor Common Systems
//
// 19-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common.h"
#include "common_mapwindow.h"
#include "common_messagewindow.h"
#include "common_typelist.h"
#include "common_stats.h"
#include "common_calendar.h"
#include "common_cursorpos.h"
#include "common_teamlistbox.h"
#include "common_prereqtree.h"
#include "common_prereqlist.h"
#include "common_typepreview.h"
#include "console.h"
#include "iface.h"
#include "main.h"
#include "resolver.h"
#include "viewer.h"
#include "user.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Common - Code used in both the game and the studio
//
namespace Common
{
  // System initialized flags
  static Bool initialized = FALSE;
  static Bool initializedSim = FALSE;

  // Full Preview
  VarInteger fullPreview;

  // User data key
  static GameIdent userDataKey("Common::Settings");


  //
  // LoadFromUser
  //
  static void LoadFromUser()
  {
    // Read saved settings from user
    if (FScope *fScope = User::GetConfigScope(userDataKey.crc))
    {
      /*
      while (FScope *sScope = fScope->NextFunction())
      {
        switch (sScope->NameCrc())
        {
          case 0x427B3EBE: // "FullPreview"
            fullPreview = StdLoad::TypeU32(sScope);
            break;
        }
      }
      */
    }
  }


  //
  // SaveToUser
  //
  static void SaveToUser()
  {
    // Save user settings
    FScope fScope(NULL, userDataKey.str);

    User::SetConfigScope(userDataKey.crc, &fScope);
  }


  //
  // CmdHandler
  //
  // Handles var system events
  //
  static void CmdHandler(U32 pathCrc)
  {
    ASSERT(initialized);

    switch (pathCrc)
    {
      case 0x5CAD41C5: // "common.debug.lookat"
      {
        S32 id;

        if (Console::GetArgInteger(1, id))
        {
          MapObj *mapObj = Resolver::Object<MapObj, MapObjType>(id);

          if (mapObj && mapObj->OnMap())
          {
            Viewer::GetCurrent()->LookAt(mapObj->Position().x, mapObj->Position().z);
          }
        }
        break;
      }
    }
  }


  //
  // CreateHandler
  //
  // Handles interface control creation
  //
  static IControl * CreateHandler(U32 crc, IControl *parent, U32)
  {
    IControl *ctrl = NULL;

    switch (crc)
    {
      case 0xB57D2BB8: // "MapWindow"
        ctrl = new MapWindow(parent);
        break;

      case 0x1974C5DE: // "MessageWindow"
        ctrl = new MessageWindow(parent);
        break;

      case 0x58BCAC6A: // "TypeList"
        ctrl = new TypeList(parent);
        break;

      case 0xC01F9142: // "Stats"
        ctrl = new Stats(parent);
        break;

      case 0xFB6A0289: // "Calendar"
        ctrl = new Calendar(parent);
        break;

      case 0xA715E223: // "TeamListBox"
        ctrl = new TeamListBox(parent);
        break;

      case 0x422BC5FC: // "PlayerListBox"
        ctrl = new PlayerListBox(parent);
        break;

      case 0xBD5C79C8: // "SideListBox"
        ctrl = new SideListBox(parent);
        break;

      case 0x0D719CE9: // "RuleSetListBox"
        ctrl = new RuleSetListBox(parent);
        break;

      case 0x3B994011: // "PersonalityListBox"
        ctrl = new PersonalityListBox(parent);
        break;

      case 0xE1FC3FC1: // "WorldListBox"
        ctrl = new WorldListBox(parent);
        break;

      case 0xF8E8F42E: // "TerrainCursorPos"
        ctrl = new TerrainCursorPos(parent);
        break;

      case 0xF210E7EF: // "PrereqList"
        ctrl = new PrereqList(parent);
        break;

      case 0x509FE9F5: // "PrereqTree"
        ctrl = new PrereqTree(parent);
        break;

      case 0xD74EB6C3: // "TypePreview"
        ctrl = new TypePreview(parent);
        break;
    }

    return (ctrl);
  }


  //
  // Init
  //
  // Initialize this sub-system
  //
  void Init()
  {
    ASSERT(!initialized);

    IFace::RegisterControlClass("MapWindow", CreateHandler);
    IFace::RegisterControlClass("MessageWindow", CreateHandler);
    IFace::RegisterControlClass("TypeList", CreateHandler);
    IFace::RegisterControlClass("Stats", CreateHandler);
    IFace::RegisterControlClass("Calendar", CreateHandler);
    IFace::RegisterControlClass("TeamListBox", CreateHandler);
    IFace::RegisterControlClass("PlayerListBox", CreateHandler);
    IFace::RegisterControlClass("SideListBox", CreateHandler);
    IFace::RegisterControlClass("PersonalityListBox", CreateHandler);
    IFace::RegisterControlClass("RuleSetListBox", CreateHandler);
    IFace::RegisterControlClass("WorldListBox", CreateHandler);
    IFace::RegisterControlClass("TerrainCursorPos", CreateHandler);
    IFace::RegisterControlClass("PrereqTree", CreateHandler);
    IFace::RegisterControlClass("PrereqList", CreateHandler);
    IFace::RegisterControlClass("TypePreview", CreateHandler);

    // Register command handlers
    VarSys::RegisterHandler("common", CmdHandler);
    VarSys::RegisterHandler("common.input", CmdHandler);
    VarSys::RegisterHandler("common.debug", CmdHandler);

    VarSys::CreateInteger("common.fullpreview", FALSE, VarSys::DEFAULT, &fullPreview);

    VarSys::CreateCmd("common.debug.lookat");

    VarSys::CreateInteger("common.debug.pathsearch", FALSE, VarSys::DEFAULT, &Debug::data.pathSearch);
    VarSys::CreateInteger("common.debug.movement",   FALSE, VarSys::DEFAULT, &Debug::data.movement);
    VarSys::CreateInteger("common.debug.weapons",    FALSE, VarSys::DEFAULT, &Debug::data.weapons);
    VarSys::CreateInteger("common.debug.claiming",   FALSE, VarSys::DEFAULT, &Debug::data.claiming);
    VarSys::CreateInteger("common.debug.target",     FALSE, VarSys::DEFAULT, &Debug::data.target);
    VarSys::CreateInteger("common.debug.sight",      FALSE, VarSys::DEFAULT, &Debug::data.sight);
    VarSys::CreateInteger("common.debug.hud",        FALSE, VarSys::DEFAULT, &Debug::data.hud);
    VarSys::CreateInteger("common.debug.footdeform", FALSE, VarSys::DEFAULT, &Debug::data.footDeform);
    VarSys::CreateInteger("common.debug.thumpmutex", FALSE, VarSys::DEFAULT, &Debug::data.thumpMutex);

    // Initialize sub-systems
    Cycle::Init();
    Display::Init();
    Input::Init();

    // Load user settings
    LoadFromUser();

    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown this sub-system
  //
  void Done()
  {
    ASSERT(initialized);

    // Shutdown sub-systems
    Display::Done();
    Cycle::Done();

    // Delete the scope
    VarSys::DeleteItem("common.input");
    VarSys::DeleteItem("common");

    IFace::UnregisterControlClass("MapWindow");
    IFace::UnregisterControlClass("MessageWindow");
    IFace::UnregisterControlClass("TypeList");
    IFace::UnregisterControlClass("Stats");
    IFace::UnregisterControlClass("Calendar");
    IFace::UnregisterControlClass("TeamListBox");
    IFace::UnregisterControlClass("PlayerListBox");
    IFace::UnregisterControlClass("SideListBox");
    IFace::UnregisterControlClass("PersonalityListBox");
    IFace::UnregisterControlClass("RuleSetListBox");
    IFace::UnregisterControlClass("WorldListBox");
    IFace::UnregisterControlClass("TerrainCursorPos");
    IFace::UnregisterControlClass("PrereqTree");
    IFace::UnregisterControlClass("PrereqList");
    IFace::UnregisterControlClass("TypePreview");

    initialized = FALSE;
  }    


  //
  // InitSim
  //
  // Initialize simulation systems
  //
  void InitSim()
  {
    ASSERT(!initializedSim);

    // Initialize the sub-systems
    Display::InitSim();

    Cycle::InitSim();

    initializedSim = TRUE;
  }


  //
  // DoneSim
  //
  // Shutdown simulation systems
  //
  void DoneSim()
  {
    ASSERT(initializedSim);

    // Shutdown the sub-systems
    Cycle::DoneSim();
    Display::DoneSim();

    // This is here for the studio only!
    Display::Done();
    Cycle::Done();

    initializedSim = FALSE;
  }    


  //
  // InStudio
  //
  // Are we in the studio
  //
  Bool InStudio()
  {
    return (Main::runCodes.GetCurrentCrc() == 0xCB860660 ? TRUE : FALSE); // "Studio"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Detail - render detail adjustments
  //
  namespace Detail
  {
    VarInteger shadows;
    VarInteger lights;
    VarInteger lightMulti;

    VarFloat   terrMrmThresh;
    VarFloat   terrMrmDistFactor;
    VarFloat   terrMrmDist;
    VarInteger terrMrm;
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Input - Common input functions
  //
  namespace Input
  {
    // Number of key modifier bindings
    const U32 MAX_MOD_KEYS = 3;

    // Key modifier bindings
    static VarInteger modKeys[MAX_MOD_KEYS];


    //
    // GetModifierKey
    //
    // Is a modifier key down?
    //
    Bool GetModifierKey(U32 i)
    {
      ASSERT(i > 0 && i <= MAX_MOD_KEYS)
      return (*modKeys[i-1] ? TRUE : FALSE);
    }


    //
    // Create bindings
    //
    void Init()
    {
      char name[80];

      for (U32 i = 0; i < MAX_MOD_KEYS; i++)
      {
        Utils::Sprintf(name, 80, "common.input.mod%d", i+1);
        VarSys::CreateInteger(name, 0, VarSys::DEFAULT, &modKeys[i]);
      }
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Debug
  //
  namespace Debug
  {
    Data data;
  }
}
