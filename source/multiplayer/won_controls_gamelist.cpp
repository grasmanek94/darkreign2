///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Won
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "won_controls_gamelist.h"
#include "stdload.h"
#include "font.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_priv.h"
#include "network_ping.h"
#include "multiplayer_pingdisplay.h"


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
    // Definitions
    //
    static const U32 pingInterval = 5000;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    static void PingCallback(const Win32::Socket::Address &address, U32 rtt, U32 hopCount, void *context);


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class GameList
    //

    //
    // Constructor
    //
    GameList::GameList(IControl *parent)
    : ICListBox(parent),
      offsetName(20),
      widthName(80),
      offsetHost(100),
      widthHost(80),
      offsetUsers(160),
      offsetLocked(0),
      offsetPing(200),
      widthPing(20)
    {
      // Setup the poll interval
      pollInterval = 1000;
      nextPollTime = pollInterval;
    }


    //
    // Setup
    //
    void GameList::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0xD6ABAE33: // "OffsetName"
          offsetName = StdLoad::TypeU32(fScope);
          break;

        case 0x4F76FDA5: // "WidthName"
          widthName = StdLoad::TypeU32(fScope);
          break;

        case 0x2B4DE4E5: // "OffsetHost"
          offsetHost = StdLoad::TypeU32(fScope);
          break;

        case 0xB290B773: // "WidthHost"
          widthHost = StdLoad::TypeU32(fScope);
          break;

        case 0x87CA6B98: // "OffsetUsers"
          offsetUsers = StdLoad::TypeU32(fScope);
          break;

        case 0x026FBE4B: // "OffsetLocked"
          offsetLocked = StdLoad::TypeU32(fScope);
          break;

        case 0x6D167FC3: // "OffsetPing"
          offsetPing = StdLoad::TypeU32(fScope);
          break;

        case 0xF4CB2C55: // "WidthPing"
          widthPing = StdLoad::TypeU32(fScope);
          break;

        case 0x82ACDEED: // "IconLocked"
          IFace::FScopeToTextureInfo(fScope, iconLocked);
          break;

        default:
          ICListBox::Setup(fScope);
          break;
      }
    }


    //
    // Get the selected game
    //
    GameList::Item * GameList::GetSelectedGame()
    {
      IControl *ctrl = GetSelectedItem();

      if (ctrl)
      {
        Item *item = IFace::Promote<Item>(ctrl, TRUE);
        return (item);
      }
      return (NULL);
    }


    //
    // Ping all games
    //
    void GameList::PingAll()
    {
      // Check all of the items to see if they have the property specified
      for (NList<IControl>::Iterator c(&GetItems()); *c; c++)
      {
        Item *item = IFace::Promote<Item>((*c), TRUE);
        item->Ping();
      }
    }


    //
    // Poll
    //
    void GameList::Poll()
    {
      GameList::Item *item = GetSelectedGame();

      if (item)
      {
        item->Ping();
      }
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class GameList::Item
    //


    //
    // Constructor
    //
    GameList::Item::Item(GameList &gameList, const WonIface::Game &game, IControl *parent)
    : gameList(gameList),
      ICStatic(parent),
      ping(Clock::Time::Ms()),
      old(FALSE)
    { 
      host = Utils::Unicode2Ansi(game.host.str);
      Utils::Memcpy(&session, game.data, sizeof (StyxNet::Session));

      controlStyle &= ~STYLE_INERT;

      SetColorGroup(IFace::data.cgListItem);
      SetTipText(Utils::Ansi2Unicode(session.name.str), TRUE);
    }


    //
    // Redraw self
    //
    void GameList::Item::DrawSelf(PaintInfo &pi)
    {
      U32 savedState = controlState;
      controlState &= ~STATE_DISABLED;
      DrawCtrlBackground(pi, GetTexture());
      controlState = savedState;

      DrawCtrlFrame(pi);

      // Does session have a password ?
      if (session.flags & StyxNet::SessionFlags::Locked)
      {
        SetEnabled(FALSE);
      }
      else
      {
        SetEnabled(TRUE);
      }

      if (pi.font)
      {
        const CH *ch;
        U32 yoffs = (pi.client.Height() - pi.font->Height()) / 2;

        // Does session have a password ?
        if (session.flags & StyxNet::SessionFlags::Password)
        {
          TextureInfo *icon = &gameList.iconLocked;
          S32 midY = (pi.client.p0.y + pi.client.p1.y) >> 1;

          if (icon->texture)
          {
            // Add launch ready icon
            IFace::RenderRectangle
            (
              ClipRect(
                pi.client.p0.x + gameList.offsetLocked,
                midY - (icon->pixels.Height() >> 1),
                pi.client.p0.x + gameList.offsetLocked + icon->pixels.Width(),
                midY - (icon->pixels.Height() >> 1) + icon->pixels.Height()),
              Color(1.0f, 1.0f, 1.0f), 
              icon,
              pi.alphaScale
            );
          }
        }

        // Add Game name
        ClipRect nameClip
        (
          pi.client.p0.x + gameList.offsetName,
          pi.client.p0.y,
          pi.client.p0.x + gameList.offsetName + gameList.widthName,
          pi.client.p1.y
        );

        ch = Utils::Ansi2Unicode(session.name.str);
        pi.font->Draw(
          pi.client.p0.x + gameList.offsetName,
          pi.client.p0.y + yoffs, 
          ch, 
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &nameClip
        );

        // Add Game host
        ClipRect hostClip
        (
          pi.client.p0.x + gameList.offsetHost,
          pi.client.p0.y,
          pi.client.p0.x + gameList.offsetHost + gameList.widthHost,
          pi.client.p1.y
        );

        ch = Utils::Ansi2Unicode(host.str);
        pi.font->Draw(
          pi.client.p0.x + gameList.offsetHost,
          pi.client.p0.y + yoffs, 
          ch, 
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &hostClip
        );

        // Add the number of players in the game
        CH buff[20];
        Utils::Sprintf(buff, 20, L"%d/%d", session.numUsers, session.maxUsers);
        ch = buff;
        pi.font->Draw(
          pi.client.p0.x + gameList.offsetUsers,
          pi.client.p0.y + yoffs, 
          ch, 
          Utils::Strlen(ch),
          pi.colors->fg[ColorIndex()],
          &pi.client
        );

        // Draw the ping graph
        MultiPlayer::PingDisplay::Draw
        (
          StyxNet::Std::Data::maxPings,
          connection.pings,
          ClipRect
          (
            pi.client.p0.x + gameList.offsetPing,
            pi.client.p0.y + 1,
            pi.client.p0.x + gameList.offsetPing + gameList.widthPing,
            pi.client.p1.y - 2
          ),
          pi.alphaScale
        );
      }
    }


    //
    // Ping
    //
    void GameList::Item::Ping()
    {
      // Has enough time expired ?
      U32 time = Clock::Time::Ms();
      if (time - ping > pingInterval)
      {
        // Mark time of ping
        ping = time;

        // Send off a ping
        Network::Ping::Send(session.address, PingCallback, (void *) dTrack.id);
      }
    }


    //
    // PingCallback
    //
    void PingCallback(const Win32::Socket::Address &, U32 rtt, U32 hopCount, void *context)
    {
      U32 id = (U32) context;

      IControl *ctrl = IControl::FindById(id);

      if (ctrl)
      {
        GameList::Item *item = IFace::Promote<GameList::Item>(ctrl, TRUE);
        item->GetConnection().ProcessPing(U16(rtt), U8(hopCount));
      }
    }
  }
}
