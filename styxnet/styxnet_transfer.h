////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_TRANSFER_H
#define __STYXNET_TRANSFER_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_private.h"
#include "win32_thread.h"
#include "win32_event.h"
#include "win32_socket.h"
#include "win32_file.h"
#include "win32_critsec.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace TransferState
  //
  namespace TransferState
  {
    const U32 Idle          = 0x793360CC; // "Idle"
    const U32 Connecting    = 0x3EDDF46D; // "Connecting"
    const U32 Listening     = 0x981A17D1; // "Listening"
    const U32 Transferring  = 0x90AE3BDE; // "Transferring"
    const U32 Completed     = 0x3369B9CC; // "Completed"
    const U32 Error         = 0xC21C10D7; // "Error"
    const U32 Aborted       = 0x3269E9D0; // "Aborted"
  }


  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Transfer
  //
  class Transfer
  {
  private:

    // Transfer Thread
    Win32::Thread thread;

    // Event to stop the transfer
    Win32::EventIndex eventQuit;

    // Socket for transferring
    Win32::Socket socket;

    // Event handle for the above socket
    Win32::EventIndex event;

    // The file we're transferring
    Win32::File file;

    // File mapping of the file we're transferring
    Win32::File::Mapping fileMapping;

    // File name of temporary file (for writing)
    FilePath tempName;

    // File name of destination file (for writing)
    FilePath destName;

    // State change critical section
    Win32::CritSec stateCrit;

    // File data pointer
    U8 *fileData;

    // Current state
    U32 state;

    // Amount remaining to be transferred
    U32 remaining;

    // Transfer rate (bytes/sec)
    U32 rate;

    // Time at which the transfer started (ms)
    U32 startTime;

  public:

    // Constructor
    Transfer();

    // Destructor
    ~Transfer();

    // Send
    Bool Send(const char *path, U16 &port, U32 &size);

    // Recv
    Bool Recv(const char *path, const Win32::Socket::Address &address, U32 size);

  public:

    // Progress
    void Progress(U32 &state, U32 &remaining, U32 &rate)
    {
      state = this->state;
      remaining = this->remaining;
      rate = this->rate;
    }

    // Get the start time
    U32 GetStartTime()
    {
      return (startTime);
    }

    // Abort
    void Abort()
    {
      stateCrit.Enter();
      state = TransferState::Aborted;
      stateCrit.Exit();
    }

  private:

    // Cleanup
    void Cleanup();

    // Thread procedure
    static U32 STDCALL Thread(void *);

  };

}

#endif