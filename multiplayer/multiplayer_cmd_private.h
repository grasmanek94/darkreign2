///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Commands
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CMD_PRIVATE_H
#define __MULTIPLAYER_CMD_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"
#include "multiplayer_cmd.h"
#include "multiplayer_network_private.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Cmd
  //
  namespace Cmd
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct JoinInformation
    //
    struct JoinInformation
    {
      Win32::Socket::Address address;
      SessionName session;
      U32 flags;
      U32 password;

      JoinInformation(const Win32::Socket::Address &address, const SessionName &session, U32 flags)
      : address(address),
        session(session),
        flags(flags),
        password(0)
      {
      }
    };

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct HostInformation
    //
    struct HostInformation
    {
      Win32::Socket::Address address;
      Ident session;
      U32 password;
      U32 maxUsers;

      HostInformation(const Win32::Socket::Address &address, const Ident &session, U32 password, U32 maxUsers)
      : address(address),
        session(session),
        password(password),
        maxUsers(maxUsers)
      {
      }
    };

    // Interface vars
    extern VarInteger isHost;
    extern VarInteger online;
    extern VarInteger inWon;
    extern VarInteger isLobby;
    extern VarInteger isLocked;
    extern VarInteger fillInterface;
    extern Bool updatingLock;

    extern JoinInformation *joinInfo;
    extern HostInformation *hostInfo;

  }
}


#endif