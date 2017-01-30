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
#include "multiplayer_controls_options.h"
#include "multiplayer_settings.h"
#include "winsock2.h"
  

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
    // Class Options
    //

 
    //
    // Constructor
    //
    Options::Options(IControl *parent)
    : ICWindow(parent)
    {
    }


    //
    // Destructor
    //
    Options::~Options()
    {
    }


    //
    // Activate
    //
    Bool Options::Activate()
    {
      if (ICWindow::Activate())
      {
        Upload();
        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }


    //
    // Deactivate
    //
    Bool Options::Deactivate()
    {
      if (ICWindow::Deactivate())
      {
        Download();
        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }


    //
    // Upload
    //
    void Options::Upload()
    {
      VarString firewall("$.firewall", this);
      VarString localAddress("$.localAddress", this);
      VarInteger port("$.port", this);
      VarString proxy("$.proxy", this);

      Settings::LoadFromUser();

      switch (Settings::GetConfigFirewallStatus())
      {
        case Settings::FirewallStatus::AutoDetect:
          firewall = "FirewallStatus::AutoDetect";
          break;

        case Settings::FirewallStatus::BehindFirewall:
          firewall = "FirewallStatus::BehindFirewall";
          break;

        case Settings::FirewallStatus::NotBehindFirewall:
          firewall = "FirewallStatus::NotBehindFirewall";
          break;
      }

      if (Settings::GetLocalAddress())
      {
        in_addr addr;
        addr.s_addr = Settings::GetLocalAddress();
        localAddress = inet_ntoa(addr);
      }
      else
      {
        localAddress = "";
      }
      port = Settings::GetConfigPort();
      proxy = Settings::GetProxy();
    }


    //
    // Download
    //
    void Options::Download()
    {
      VarString firewall("$.firewall", this);
      VarString localAddress("$.localAddress", this);
      VarInteger port("$.port", this);
      VarString proxy("$.proxy", this);

      Settings::SetFirewallStatus(Crc::CalcStr(firewall));
      Settings::SetLocalAddress(inet_addr(*localAddress));
      Settings::SetPort(port);
      Settings::SetProxy(proxy);

      Settings::SaveToUser();
    }

  }

}
