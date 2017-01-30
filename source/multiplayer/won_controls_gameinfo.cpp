///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Won Stuff
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "won_controls_gameinfo.h"
#include "iface.h"
#include "styxnet_std.h"
#include "stdload.h"
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
    // Class GameInfo
    //


    //
    // Constructor
    //
    GameInfo::GameInfo(IControl *parent)
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
    GameInfo::~GameInfo()
    {
    }


    //
    // Activate
    //
    Bool GameInfo::Activate()
    {
      if (ICWindow::Activate())
      {
        itemPtr = NULL;

        if (context.Alive())
        {
          if (GameList::Item *item = IFace::Promote<GameList::Item>(context))
          {
            itemPtr = item;
            itemNameCrc = item->NameCrc();
            containerPtr = item->Parent();
          }
        }
        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Setup
    //
    void GameInfo::Setup(FScope *fScope)
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
    void GameInfo::DrawSelf(PaintInfo &pi)
    {
      ICWindow::DrawSelf(pi);

      VarString gameName("$.gameName", this);

      VarInteger pingSmooth("$.pingSmooth", this);
      VarInteger pingSmoothDev("$.pingSmoothDev", this);
      VarInteger pingMax("$.pingMax", this);
      VarInteger pingMin("$.pingMin", this);
      VarInteger pingAvg("$.pingAvg", this);
      VarInteger hops("$.hops", this);
      VarFloat loss("$.loss", this);

      // Set the defaults
      gameName = "-";
      pingSmooth = 0;
      pingSmoothDev = 0;
      pingMax = 0;
      pingMin = 0;
      pingAvg = 0;
      hops = 0;
      loss = 0.0f;

      if (!itemPtr.Alive())
      {
        if (containerPtr.Alive())
        {
          if (IControl *ctrl = containerPtr->Find(itemNameCrc))
          {
            itemPtr = IFace::Promote<GameList::Item>(ctrl);
          }
        }
      }

      if (itemPtr.Alive())
      {
        gameName = itemPtr->GetSession().name.str;

        const StyxNet::Std::Data::UserConnection &connection = itemPtr->GetConnection();

        pingSmooth = connection.pingSmooth;
        pingSmoothDev = connection.pingSmoothDev;
        pingMax = connection.pingMax;
        pingMin = connection.pingMin;
        pingAvg = connection.num ? connection.pingSum / connection.num : 0;
        hops = connection.hopsSmooth;
        loss = connection.num ? connection.lost * 100.0f / connection.num : 0.0f;

        MultiPlayer::PingDisplay::Draw
        (
          StyxNet::Std::Data::maxPings,
          connection.pings,
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
