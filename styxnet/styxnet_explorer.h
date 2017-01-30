////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_EXPLORER_H
#define __STYXNET_EXPLORER_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "styxnet_session.h"
#include "win32_thread.h"
#include "win32_event.h"
#include "win32_socket.h"
#include "win32_mutex.h"
#include "queue.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Explorer
  //
  class Explorer
  {
  public:

    class Session;

  private:

    // Explorer Thread
    Win32::Thread thread;

    // Event to stop the explorer
    Win32::EventIndex eventQuit;

    // Socket for exploring
    Win32::Socket socket;

    // Event handle for the above socket
    Win32::EventIndex event;

    // The sessions
    // Each session is keyed by the address then the name
    NList<Session> sessions;

    // Session mutex
    Win32::Mutex sessionMutex;

  public:

    // Constructor
    Explorer();

    // Destructor
    ~Explorer();

    // Explore the local network
    void Broadcast(U16 port);

    // Explore a specific address
    void Unicast(const Win32::Socket::Address &address);

    // Remove sessions older than the given time
    void RemoveSessions(U32 lifeTime);

    // Get a COPY of the current set of sessions
    void CopySessions(NList<Session> &sessions);

  private:

    // Thread procedure
    static U32 STDCALL ThreadProc(void *);

  };


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Explorer::Session
  // 
  class Explorer::Session : public StyxNet::Session
  {
  private:

    // List node
    NList<Session>::Node nodeExplorer;

  public:

    // Initializing constructor
    Session(const SessionName &name, U16 flags, U16 version, U32 numUsers, U32 maxUsers, const Win32::Socket::Address &address, U32 lastUpdateTime);

    // Copy constructor
    Session(const Session &session);

    // Time when we last received an update from this session
    U32 lastUpdateTime;

    friend Explorer;

  };

}

#endif