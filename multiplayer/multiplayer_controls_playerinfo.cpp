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
#include "multiplayer_private.h"
#include "multiplayer_data.h"
#include "multiplayer_host.h"
#include "multiplayer_controls_playerinfo.h"
#include "multiplayer_controls_playerlist.h"
#include "multiplayer_network.h"
#include "multiplayer_network_private.h"
#include "multiplayer_pingdisplay.h"
#include "iface.h"
#include "styxnet_std.h"
#include "stdload.h"


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
    // Class PlayerInfo
    //


    //
    // Constructor
    //
    PlayerInfo::PlayerInfo(IControl *parent)
    : ICWindow(parent)
    {
      // Ping graph rectangle
      pingGraph.p0.x = 5;
      pingGraph.p0.y = 5;
      pingGraph.p1.x = 50;
      pingGraph.p1.y = 50;
    }


    //
    // Destructor
    //
    PlayerInfo::~PlayerInfo()
    {
    }


    //
    // Activate
    //
    Bool PlayerInfo::Activate()
    {
      if (ICWindow::Activate())
      {
        networkId = 0;
        if (context.Alive())
        {
          if (PlayerList::PlayerItem *item = IFace::Promote<PlayerList::PlayerItem>(context))
          {
            networkId = item->GetNetworkId();
          }
        }
        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Setup
    //
    void PlayerInfo::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0x1AB18CDB: // "PingGraph"
          StdLoad::TypeArea<S32>(fScope, pingGraph);
          break;

        default:
          ICWindow::Setup(fScope);
          break;
      }
    }


    //
    // Redraw self
    //
    void PlayerInfo::DrawSelf(PaintInfo &pi)
    {
      ICWindow::DrawSelf(pi);

      VarString playerName("$.playerName", this);

      VarInteger pingSmooth("$.pingSmooth", this);
      VarInteger pingSmoothDev("$.pingSmoothDev", this);
      VarInteger pingMax("$.pingMax", this);
      VarInteger pingMin("$.pingMin", this);
      VarInteger pingAvg("$.pingAvg", this);
      VarInteger hops("$.hops", this);
      VarFloat loss("$.loss", this);

      // Set the defaults
      playerName = "-";
      pingSmooth = 0;
      pingSmoothDev = 0;
      pingMax = 0;
      pingMin = 0;
      pingAvg = 0;
      hops = 0;
      loss = 0.0f;

      {
        Network::Player *player = Network::GetPlayers().Find(networkId);
        if (player)
        {
          playerName = player->GetName();
        }
      }

      {
        if (Network::client)
        {
          const StyxNet::Std::Data::UserConnection *userConnection = Network::client->GetUserConnection(networkId);
          if (userConnection)
          {
            pingSmooth = userConnection->pingSmooth;
            pingSmoothDev = userConnection->pingSmoothDev;
            pingMax = userConnection->pingMax;
            pingMin = userConnection->pingMin;
            pingAvg = userConnection->num ? userConnection->pingSum / userConnection->num : 0;
            hops = userConnection->hopsSmooth;
            loss = userConnection->num ? userConnection->lost * 100.0f / userConnection->num : 0.0f;

            PingDisplay::Draw
            (
              StyxNet::Std::Data::maxPings,
              userConnection->pings,
              ClipRect
              (
                pi.client.p0.x + pingGraph.p0.x,
                pi.client.p0.y + pingGraph.p0.y,
                pi.client.p0.x + pingGraph.p1.x,
                pi.client.p0.y + pingGraph.p1.y
              ),
              pi.alphaScale
            );
          }
        }
      }
    }
  }

}
