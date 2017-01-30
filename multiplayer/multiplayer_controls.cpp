///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_controls.h"
#include "multiplayer_controls_sessions.h"
#include "multiplayer_controls_addressbook.h"
#include "multiplayer_controls_chatedit.h"
#include "multiplayer_controls_playerlist.h"
#include "multiplayer_controls_playerinfo.h"
#include "multiplayer_controls_hostconfig.h"
#include "multiplayer_controls_synclist.h"
#include "multiplayer_controls_mission.h"
#include "multiplayer_controls_color.h"
#include "multiplayer_controls_context.h"
#include "multiplayer_controls_transferlist.h"
#include "multiplayer_controls_propertylist.h"
#include "multiplayer_controls_localpings.h"
#include "multiplayer_controls_earth.h"
#include "multiplayer_controls_report.h"
#include "multiplayer_controls_options.h"
#include "multiplayer_controls_download.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //
    Bool initialized = FALSE;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    IControl *Babel(U32 classId, IControl *parent, U32 flags);


    //
    // Initialization
    //
    void Init()
    {
      ASSERT(!initialized)

      IFace::RegisterControlClass("MultiPlayer::Sessions", Babel);
      IFace::RegisterControlClass("MultiPlayer::AddressBook", Babel);
      IFace::RegisterControlClass("MultiPlayer::ChatEdit", Babel);
      IFace::RegisterControlClass("MultiPlayer::PlayerList", Babel);
      IFace::RegisterControlClass("MultiPlayer::PlayerList::AllyWithButton", Babel);
      IFace::RegisterControlClass("MultiPlayer::PlayerInfo", Babel);
      IFace::RegisterControlClass("MultiPlayer::HostConfig", Babel);
      IFace::RegisterControlClass("MultiPlayer::SyncList", Babel);
      IFace::RegisterControlClass("MultiPlayer::Mission", Babel);
      IFace::RegisterControlClass("MultiPlayer::Color", Babel);
      IFace::RegisterControlClass("MultiPlayer::TeamPlayerOptions", Babel);
      IFace::RegisterControlClass("MultiPlayer::TransferList", Babel);
      IFace::RegisterControlClass("MultiPlayer::PropertyList", Babel);
      IFace::RegisterControlClass("MultiPlayer::LocalPings", Babel);
      IFace::RegisterControlClass("MultiPlayer::Earth", Babel);
      IFace::RegisterControlClass("MultiPlayer::Report", Babel);
      IFace::RegisterControlClass("MultiPlayer::Options", Babel);
      IFace::RegisterControlClass("MultiPlayer::Download", Babel);

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      IFace::UnregisterControlClass("MultiPlayer::Sessions");
      IFace::UnregisterControlClass("MultiPlayer::AddressBook");
      IFace::UnregisterControlClass("MultiPlayer::ChatEdit");
      IFace::UnregisterControlClass("MultiPlayer::PlayerList");
      IFace::UnregisterControlClass("MultiPlayer::PlayerList::AllyWithButton");
      IFace::UnregisterControlClass("MultiPlayer::PlayerInfo");
      IFace::UnregisterControlClass("MultiPlayer::HostConfig");
      IFace::UnregisterControlClass("MultiPlayer::SyncList");
      IFace::UnregisterControlClass("MultiPlayer::Mission");
      IFace::UnregisterControlClass("MultiPlayer::Color");
      IFace::UnregisterControlClass("MultiPlayer::TeamPlayerOptions");
      IFace::UnregisterControlClass("MultiPlayer::TransferList");
      IFace::UnregisterControlClass("MultiPlayer::PropertyList");
      IFace::UnregisterControlClass("MultiPlayer::LocalPings");
      IFace::UnregisterControlClass("MultiPlayer::Earth");
      IFace::UnregisterControlClass("MultiPlayer::Report");
      IFace::UnregisterControlClass("MultiPlayer::Options");
      IFace::UnregisterControlClass("MultiPlayer::Download");

      initialized = FALSE;
    }


    //
    // Babel
    //
    IControl * Babel(U32 classId, IControl *parent, U32)
    {
      ASSERT(initialized)

      IControl *ctrl = NULL;

      // Map class id to basic class type
      switch (classId)
      {
        case 0xAABE7B4D: // "MultiPlayer::Sessions"
          ctrl = new Sessions(parent);
          break;

        case 0x5C9A1947: // "MultiPlayer::AddressBook"
          ctrl = new AddressBook(parent);
          break;

        case 0xD95DE290: // "MultiPlayer::ChatEdit"
          ctrl = new ChatEdit(parent);
          break;

        case 0xC137FFE9: // "MultiPlayer::PlayerList"
          ctrl = new PlayerList(parent);
          break;

        case 0x2F3BA796: // "MultiPlayer::PlayerList::AllyWithButton"
          ctrl = new PlayerList::AllyWithButton(parent);
          break;

        case 0x849205B9: // "MultiPlayer::PlayerInfo"
          ctrl = new PlayerInfo(parent);
          break;

        case 0xC9424004: // "MultiPlayer::HostConfig"
          ctrl = new HostConfig(parent);
          break;

        case 0x2A1A8866: // "MultiPlayer::SyncList"
          ctrl = new SyncList(parent);
          break;

        case 0x0D2987ED: // "MultiPlayer::Mission"
          ctrl = new Mission(parent);
          break;

        case 0xD9BD6D1F: // "MultiPlayer::Color"
          ctrl = new Color(parent);
          break;

        case 0x9BA70E5D: // "MultiPlayer::TeamPlayerOptions"
          ctrl = new TeamPlayerOptions(parent);
          break;

        case 0xE5C45EFE: // "MultiPlayer::TransferList"
          ctrl = new TransferList(parent);
          break;

        case 0xC89CCC7A: // "MultiPlayer::PropertyList"
          ctrl = new PropertyList(parent);
          break;

        case 0x7C6FDFAE: // "MultiPlayer::LocalPings"
          ctrl = new LocalPings(parent);
          break;

        case 0xCEA3103E: // "MultiPlayer::Earth"
          ctrl = new Earth(parent);
          break;

        case 0xC0E97969: // "MultiPlayer::Report"
          ctrl = new Report(parent);
          break;

        case 0x05BF3A49: // "MultiPlayer::Options"
          ctrl = new Options(parent);
          break;

        case 0xE47EC488: // "MultiPlayer::Download"
          ctrl = new Download(parent);
          break;
      }

      return (ctrl);
    }

  }

}
