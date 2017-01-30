///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Won
//

#ifndef __WON_CONTROLS_GAMELIST
#define __WON_CONTROLS_GAMELIST


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "icstatic.h"
#include "woniface.h"
#include "multiplayer_network_private.h"


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
    // Class GameList
    //
    class GameList : public ICListBox
    {
      PROMOTE_LINK(GameList, ICListBox, 0xDBF27997) // "Won::GameList"

    public:
      
      class Item;

    protected:

      // Offsets
      U32 offsetName;
      U32 widthName;
      U32 offsetHost;
      U32 widthHost;
      U32 offsetUsers;
      U32 offsetLocked;
      U32 offsetPing;
      U32 widthPing;

      // Locked icon
      TextureInfo iconLocked;

    public:

      // Constructor
      GameList(IControl *parent);

      // Setup
      void Setup(FScope *fScope);

      // Get the selected game
      Item * GetSelectedGame();

      // Ping all games
      void PingAll();

      // Poll func
      void Poll();

    public:

      friend Item;

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class GameList::Item
    //
    class GameList::Item : public ICStatic
    {
      PROMOTE_LINK(Game, ICStatic, 0x1E5416FD) // "Won::Game"

    protected:

      // Offsets are in here
      GameList &gameList;

      // Host of the game
      Ident host;

      // The last time we pinged this game
      U32 ping;

      // The game
      StyxNet::Session session;

      // The pings to the session
      StyxNet::Std::Data::UserConnection connection;

    public:

      // Marked as old
      Bool old;

    public:

      // Constructor
      Item(GameList &gameList, const WonIface::Game &game, IControl *parent);

      // Redraw self
      void DrawSelf(PaintInfo &pi);

      // Ping
      void Ping();

    public:

      // Get the session
      StyxNet::Session & GetSession()
      {
        return (session);
      }

      // Get the connection information
      StyxNet::Std::Data::UserConnection & GetConnection()
      {
        return (connection);
      }

      friend GameList;

    };

  }

}

#endif