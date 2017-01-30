////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_SESSION_H
#define __STYXNET_SESSION_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32_socket.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Struct SessionData
  //
  struct SessionData
  {
    // Name of the session
    SessionName name;

    // Session flags
    U16 flags;

    // Version
    U16 version;

    // Current number of users
    U32 numUsers;

    // Maximum number of users
    U32 maxUsers;

    // Default constructor
    SessionData()
    : flags(0),
      numUsers(0),
      maxUsers(0)
    {
    }

    // Initializing constructor
    SessionData(const SessionName &name, U16 flags, U16 version, U32 numUsers, U32 maxUsers)
    : name(name),
      flags(flags),
      version(version),
      numUsers(numUsers),
      maxUsers(maxUsers)
    {
    }

    // Copy constructor
    SessionData(const SessionData &sessionData)
    : name(sessionData.name),
      flags(sessionData.flags),
      version(sessionData.version),
      numUsers(sessionData.numUsers),
      maxUsers(sessionData.maxUsers)
    {
    }

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Struct Session
  //
  struct Session : public SessionData
  {
    // Address of the server which is hosting the session
    Win32::Socket::Address address;

    // Default constructor
    Session()
    {
    }

    // Initializing constructor
    Session(const SessionName &name, U16 flags, U16 version, U32 numUsers, U32 maxUsers, const Win32::Socket::Address &address)
    : SessionData(name, flags, version, numUsers, maxUsers),
      address(address)
    {
    }

    // Initializing constructor
    Session(const SessionData &sessionData, const Win32::Socket::Address &address)
    : SessionData(sessionData),
      address(address)
    {
    }

    // Copy constructor
    Session(const Session &session)
    : SessionData(session),
      address(session.address)
    {
    }

  };

}

#endif