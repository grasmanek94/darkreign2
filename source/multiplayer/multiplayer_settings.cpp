///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_settings.h"
#include "multiplayer_network_private.h"
#include "multiplayer_cmd_private.h"
#include "utiltypes.h"
#include "user.h"
#include "stdload.h"
#include "woniface.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Settings
  //
  namespace Settings
  {

    // Config key
    static GameIdent settings("MultiPlayer::Settings");

    // Initialized flag
    static Bool initialized = FALSE;

    static U32 firewallStatus;
    static U32 localAddress;
    static U16 port;
    static F32 latitude;
    static F32 longitude;
    static StrBuf<256> proxy;


    //
    // Initialization
    //
    void Init()
    {
      ASSERT(!initialized)

      // Set the defaults
      firewallStatus = FirewallStatus::AutoDetect;
      localAddress = 0;
      port = 0;
      latitude = 34.01001f;
      longitude = -118.471708f;
      proxy = "";

      initialized = TRUE;
    }


    //
    // Shutdown
    //
    void Done()
    {
      ASSERT(initialized)

      initialized = FALSE;
    }


    //
    // Load settings from user
    //
    void LoadFromUser()
    {
      FScope *fScope = User::GetConfigScope(settings.crc);
      if (fScope)
      {
        firewallStatus = StdLoad::TypeU32(fScope, "Firewall");
        localAddress = StdLoad::TypeU32(fScope, "LocalAddress");
        port = U16(StdLoad::TypeU32(fScope, "Port", Range<U32>(0, U16_MAX)));
        latitude = StdLoad::TypeF32(fScope, "Latitude");
        longitude = StdLoad::TypeF32(fScope, "Longitude");
        proxy = StdLoad::TypeString(fScope, "Proxy");
      }
    }


    //
    // Save settings to user
    //
    void SaveToUser()
    {
      FScope fScope(NULL, settings.str);

      StdSave::TypeU32(&fScope, "Firewall", firewallStatus);
      StdSave::TypeU32(&fScope, "LocalAddress", localAddress);
      StdSave::TypeU32(&fScope, "Port", port);
      StdSave::TypeF32(&fScope, "Latitude", latitude);
      StdSave::TypeF32(&fScope, "Longitude", longitude);
      StdSave::TypeString(&fScope, "Proxy", proxy.str);

      User::SetConfigScope(settings.crc, &fScope);
    }
    

    //
    // Get various settings
    //
    U32 GetConfigFirewallStatus()
    {
      return (firewallStatus);
    }
    U32 GetFirewallStatus()
    {
      // If we're in WON
      if (Cmd::inWon)
      {
        switch (firewallStatus)
        {
          case FirewallStatus::AutoDetect:
            switch (WonIface::GetFirewallStatus())
            {
              default:
              case WonIface::Firewall::Unchecked:
              case WonIface::Firewall::Checking:
                return (FirewallStatus::Unknown);

              case WonIface::Firewall::Behind:
                return (FirewallStatus::BehindFirewall);

              case WonIface::Firewall::None:
                return (FirewallStatus::NotBehindFirewall);
            }
            break;

          default:
            return (firewallStatus);
        }
      }
      else
      {
        return (FirewallStatus::NotBehindFirewall);
      }
    }
    U32 GetLocalAddress()
    {
      return (localAddress);
    }
    U32 GetConfigPort()
    {
      return (port);
    }
    U16 GetPort()
    {
      return (U16(port ? port : StyxNet::defaultPort));
    }
    F32 GetLatitude()
    {
      return (latitude);
    }
    F32 GetLongitude()
    {
      return (longitude);
    }
    const char * GetProxy()
    {
      return (proxy.str);
    }

    // Set various settings
    void SetFirewallStatus(U32 status)
    {
      firewallStatus = status;
    }
    void SetLocalAddress(U32 address)
    {
      localAddress = address;
    }
    void SetPort(U32 p)
    {
      if (p)
      {
        port = U16(Clamp<U32>(1024, p, 65535));
      }
      else
      {
        port = 0;
      }
    }
    void SetLatitude(F32 l)
    {
      latitude = l;
    }
    void SetLongitude(F32 l)
    {
      longitude = l;
    }
    void SetProxy(const char *p)
    {
      proxy = p;
    }

  }

}
