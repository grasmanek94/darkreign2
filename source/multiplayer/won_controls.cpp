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
#include "won_controls.h"
#include "won_controls_roomlist.h"
#include "won_controls_gamelist.h"
#include "won_controls_playerlist.h"
#include "won_controls_gameinfo.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Won
//
namespace Won
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

      IFace::RegisterControlClass("Won::RoomList", Babel);
      IFace::RegisterControlClass("Won::GameList", Babel);
      IFace::RegisterControlClass("Won::PlayerList", Babel);
      IFace::RegisterControlClass("Won::GameInfo", Babel);

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      IFace::UnregisterControlClass("Won::RoomList");
      IFace::UnregisterControlClass("Won::GameList");
      IFace::UnregisterControlClass("Won::PlayerList");
      IFace::UnregisterControlClass("Won::GameInfo");

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
        case 0xE47F67C8: // "Won::RoomList"
          ctrl = new RoomList(parent);
          break;

        case 0xDBF27997: // "Won::GameList"
          ctrl = new GameList(parent);
          break;

        case 0x0B53AD48: // "Won::PlayerList"
          ctrl = new PlayerList(parent);
          break;

        case 0x9E5783C7: // "Won::GameInfo"
          ctrl = new GameInfo(parent);
          break;
      }

      return (ctrl);
    }
  }
}
