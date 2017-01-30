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
#include "multiplayer_controls_report.h"
#include "multiplayer_network_private.h"
#include "iface.h"
#include "styxnet_std.h"


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
    // Class Report
    //


    //
    // Constructor
    //
    Report::Report(IControl *parent)
    : ICWindow(parent)
    {
    }


    //
    // Destructor
    //
    Report::~Report()
    {
    }


    //
    // Redraw self
    //
    void Report::DrawSelf(PaintInfo &pi)
    {
      VarInteger recvPackets("$.recvPackets", this);
      VarFloat recvPacketsPerSecond("$.recvPacketsPerSecond", this);
      VarInteger recvBytes("$.recvBytes", this);
      VarFloat recvBytesPerSecond("$.recvBytesPerSecond", this);
      VarFloat recvBytesPerPacket("$.recvBytesPerPacket", this);
      VarFloat recvRate("$.recvRate", this);
      VarFloat recvRateSmooth("$.recvRateSmooth", this);
      VarFloat recvRateMax("$.recvRateMax", this);
      VarFloat recvRateMin("$.recvRateMin", this);

      VarInteger sentPackets("$.sentPackets", this);
      VarFloat sentPacketsPerSecond("$.sentPacketsPerSecond", this);
      VarInteger sentBytes("$.sentBytes", this);
      VarFloat sentBytesPerSecond("$.sentBytesPerSecond", this);
      VarFloat sentBytesPerPacket("$.sentBytesPerPacket", this);
      VarFloat sentRate("$.sentRate", this);
      VarFloat sentRateSmooth("$.sentRateSmooth", this);
      VarFloat sentRateMax("$.sentRateMax", this);
      VarFloat sentRateMin("$.sentRateMin", this);

      VarInteger pingSmooth("$.pingSmooth", this);
      VarInteger pingSmoothDev("$.pingSmoothDev", this);
      VarInteger pingAvg("$.pingAvg", this);
      VarInteger pingMax("$.pingMax", this);
      VarInteger pingMin("$.pingMin", this);
      VarInteger hops("$.hops", this);
      VarFloat loss("$.loss", this);

      if (Network::client)
      {
        const Win32::Socket::Statistics &stats = Network::client->GetStatistics();
        const Win32::Socket::Statistics::Flow &flowRecv = stats.GetRecvFlow();
        const Win32::Socket::Statistics::Flow &flowSent = stats.GetSentFlow();

        U32 time = Clock::Time::Ms() - stats.GetTimeOpened();

        recvPackets = flowRecv.packets;
        recvPacketsPerSecond = time ? F32(flowRecv.packets) * 1000.0f / F32(time) : 0.0f;
        recvBytes = flowRecv.bytes;
        recvBytesPerSecond = time ? F32(flowRecv.bytes)  * 1000.0f / F32(time) : 0.0f;
        recvBytesPerPacket = flowRecv.packets ? F32(flowRecv.bytes) / F32(flowRecv.packets) : 0.0f;
        recvRate = flowRecv.rate * 1000.0f;
        recvRateSmooth = flowRecv.smoothRate * 1000.0f;
        recvRateMax = flowRecv.maxRate * 1000.0f;
        recvRateMin = flowRecv.minRate * 1000.0f;

        sentPackets = flowSent.packets;
        sentPacketsPerSecond = time ? F32(flowSent.packets) * 1000.0f / F32(time) : 0.0f;
        sentBytes = flowSent.bytes;
        sentBytesPerSecond = time ? F32(flowSent.bytes)  * 1000.0f / F32(time) : 0.0f;
        sentBytesPerPacket = flowSent.packets ? F32(flowSent.bytes) / F32(flowSent.packets) : 0.0f;
        sentRate = flowSent.rate * 1000.0f;
        sentRateSmooth = flowSent.smoothRate * 1000.0f;
        sentRateMax = flowSent.maxRate * 1000.0f;
        sentRateMin = flowSent.minRate * 1000.0f;
      }
      else
      {
        recvPackets = 0;
        recvPacketsPerSecond = 0.0f;
        recvBytes = 0;
        recvBytesPerSecond = 0.0f;
        recvBytesPerPacket = 0.0f;
        recvRate = 0.0f;
        recvRateSmooth = 0.0f;
        recvRateMax = 0.0f;
        recvRateMin = 0.0f;

        sentPackets = 0;
        sentPacketsPerSecond = 0.0f;
        sentBytes = 0;
        sentBytesPerSecond = 0.0f;
        sentBytesPerPacket = 0.0f;
        sentRate = 0.0f;
        sentRateSmooth = 0.0f;
        sentRateMax = 0.0f;
        sentRateMin = 0.0f;
      }


      if (Network::client)
      {
        const StyxNet::Std::Data::UserConnection *userConnection;
        if 
        (
          Network::HaveCurrentPlayer() && 
          (userConnection = Network::client->GetUserConnection(Network::GetCurrentPlayer().GetId())) != NULL
        )
        {
          pingSmooth = userConnection->pingSmooth;
          pingSmoothDev = userConnection->pingSmoothDev;
          pingMax = userConnection->pingMax;
          pingMin = userConnection->pingMin;
          pingAvg = userConnection->num ? userConnection->pingSum / userConnection->num : 0;
          hops = userConnection->hopsSmooth;
          loss = userConnection->num ? userConnection->lost * 100.0f / userConnection->num : 0.0f;
        }
        else
        {
          pingSmooth = 0;
          pingSmoothDev = 0;
          pingMax = 0;
          pingMin = 0;
          pingAvg = 0;
          hops = 0;
          loss = 0.0f;
        }
      }

      ICWindow::DrawSelf(pi);
    }
  }

}
