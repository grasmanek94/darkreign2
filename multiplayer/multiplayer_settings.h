///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_OPTIONS_H
#define __MULTIPLAYER_OPTIONS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


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

    namespace FirewallStatus
    {
      const U32 AutoDetect        = 0x6CDD4973; // "FirewallStatus::AutoDetect"
      const U32 BehindFirewall    = 0xBC8DB0CD; // "FirewallStatus::BehindFirewall"
      const U32 NotBehindFirewall = 0xACD98AD4; // "FirewallStatus::NotBehindFirewall"
      const U32 Unknown           = 0x4E86E405; // "FirewallStatus::Unknown"
    }


    // Initialization and Shutdown
    void Init();
    void Done();

    // Load options from user
    void LoadFromUser();

    // Save options to user
    void SaveToUser();
    
    // Get various options
    U32 GetConfigFirewallStatus();
    U32 GetFirewallStatus();
    U32 GetLocalAddress();
    U32 GetConfigPort();
    U16 GetPort();
    F32 GetLatitude();
    F32 GetLongitude();
    const char * GetProxy();

    // Set various options
    void SetFirewallStatus(U32 status);
    void SetLocalAddress(U32 address);
    void SetPort(U32 port);
    void SetLatitude(F32 latitude);
    void SetLongitude(F32 longitude);
    void SetProxy(const char *proxy);

  }

}


#endif
