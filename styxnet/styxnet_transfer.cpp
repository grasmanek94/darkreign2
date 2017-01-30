////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet_transfer.h"
#include "file.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Transfer
  //

  const U32 BlockSize = 1024;
  const U32 RateFilter = 224;
  const U32 RateFilterInv = (256 - RateFilter);


  //
  // Transfer::Transfer
  //
  // Constructor
  //
  Transfer::Transfer()
  : fileData(NULL),
    state(TransferState::Idle),
    remaining(0),
    rate(0),
    startTime(0)
  {
  }


  //
  // Transfer::~Transfer
  //
  // Destructor
  //
  Transfer::~Transfer()
  {
    // Tell the threat to quit
    eventQuit.Signal();

    // Wait for the threat to terminate
    thread.Stop();

    // Make sure the socket is closed
    socket.Close();

    // Cleanup
    Cleanup();
  }


  //
  // Send
  //
  Bool Transfer::Send(const char *path, U16 &port, U32 &size)
  {
    // Attempt to open the file
    if (!file.Open(path, Win32::File::MODE_READ | Win32::File::MODE_OPEN_EXISTING | Win32::File::MODE_SHARE_READ))
    {
      LWARN("Could not open file: " << path)
      return (FALSE);
    }

    size = file.GetSize();
    remaining = size;

    // Attempt to create a file mapping
    if (!fileMapping.Setup(file))
    {
      LWARN("Could not map file: " << path);
      file.Close();
      return (FALSE);
    }

    fileData = fileMapping.GetPointer();

    // Bind the sending socket to any adress/port
    socket.Bind(Win32::Socket::Address(ADDR_ANY, 0));

    // Get the port number the address was bound to
    Win32::Socket::Address address;
    socket.GetLocalAddress(address);
    port = address.GetPort();

    // Listen on the port for the receiver
    socket.Listen(1);
    socket.EventSelect(event, FD_ACCEPT);

    // Change states
    stateCrit.Enter();
    if (state != TransferState::Aborted)
    {
      state = TransferState::Listening;
    }
    stateCrit.Exit();

    // Start the thread
    thread.Start(Thread, this);

    // Make it above normal
    thread.SetPriority(Win32::Thread::ABOVE_NORMAL);

    return (TRUE);
  }


  //
  // Recv
  //
  Bool Transfer::Recv(const char *path, const Win32::Socket::Address &address, U32 size)
  {
    // Make sure that the destination path exists
    FileDrive dstDrive;
    FileDir dstDir; 
    FileName dstName;
    FileExt dstExt;

    Dir::PathExpand(path, dstDrive, dstDir, dstName, dstExt);
    Dir::MakeFull(dstDir.str);

    // Store full destination filename
    destName = path;

    // Make sure the working directory exists
    FilePath workPath = "downloads\\temp";
    Dir::MakeFull(workPath.str);

    // Create file with same name in working directory
    Utils::MakePath(tempName.str, PATHNAME_MAX, workPath.str, dstName.str, dstExt.str);

    LDIAG("Downloading to " << tempName.str)

    // Delete the temp file if it exists
    File::Unlink(tempName.str);

    // Attempt to open the file
    if (!file.Open(tempName.str, Win32::File::MODE_WRITE | Win32::File::MODE_READ | Win32::File::MODE_CREATE_NEW))
    {
      LWARN("Could not open file: " << tempName.str)
      return (FALSE);
    }

    // Create a file mapping
    if (!fileMapping.Setup(file, 0, size))
    {
      LWARN("Could not map file: " << tempName.str)
      return (FALSE);
    }

    fileData = fileMapping.GetPointer();
    remaining = size;

    // Associate events with our socket
    socket.EventSelect(event, FD_CONNECT | FD_READ | FD_CLOSE);

    // Bind the socket to any local address
    socket.Bind(Win32::Socket::Address(ADDR_ANY, 0));

    // Initiate the connection to the address specifiec
    socket.Connect(address);

    // Start the thread
    thread.Start(Thread, this);

    return (TRUE);
  }

  
  //
  // Transfer::Cleanup
  //
  void Transfer::Cleanup()
  {
    // Cleanup any filemapping
    fileMapping.Cleanup();

    // Make sure the file is closed if one is open
    file.Close();
  }


  //
  // Transfer::Thread
  //
  // Thread procedure
  //
  U32 STDCALL Transfer::Thread(void *context)
  {
    Transfer *transfer = static_cast<Transfer *>(context);

    Bool quit = FALSE;

    Win32::EventIndex::List<2> events;
    events.AddEvent(transfer->eventQuit, NULL);
    events.AddEvent(transfer->event, transfer);

    U32 nextTime = Clock::Time::Ms() + 1000;
    U32 transferRemaining = transfer->remaining;

    while (!quit)
    {
      S32 remaining = nextTime - Clock::Time::Ms();

      if (remaining > 0 && events.Wait(context, FALSE, remaining))
      {
        if (context)
        {
          Win32::Socket::NetworkEvents networkEvents;
          transfer->socket.EnumEvents(transfer->event, networkEvents);

          if (networkEvents.GetEvents() & FD_ACCEPT)
          {
            // Connection has been established by the receiver
            Win32::Socket::Address address;

            // Create a new user which uses this socket
            Win32::Socket connected = transfer->socket.Accept(address);

            // Now that we're connected, we don't need the listening socket anymore, close it
            transfer->socket.Close();

            // Take the connected socket and setup events for it
            transfer->socket = connected;
            transfer->socket.EventSelect(transfer->event, FD_WRITE | FD_CLOSE);

            LDIAG("Sender: Connection from " << address)

            U32 maxSize;
            U32 maxSizeSize = sizeof (U32);
            transfer->socket.GetSockOpt(SOL_SOCKET, SO_MAX_MSG_SIZE, reinterpret_cast<char *>(&maxSize), maxSizeSize);
            LDIAG("Maximum transfer size is " << maxSize)

            transfer->startTime = Clock::Time::Ms();
            transfer->stateCrit.Enter();
            if (transfer->state != TransferState::Aborted)
            {
              transfer->state = TransferState::Transferring;
            }
            transfer->stateCrit.Exit();
          }

          if (networkEvents.GetEvents() & FD_CONNECT)
          {
            // Connection has been established !
            transfer->startTime = Clock::Time::Ms();
            transfer->stateCrit.Enter();
            if (transfer->state != TransferState::Aborted)
            {
              transfer->state = TransferState::Transferring;
            }
            transfer->stateCrit.Exit();

            LDIAG("Connected")
          }

          if (networkEvents.GetEvents() & FD_WRITE)
          {
            // The socket is allowing us to write to it, stuff it till it chokes
            for (;;)
            {
              U32 amount = Min(transfer->remaining, BlockSize);
              if (amount)
              {
                U32 transferred = transfer->socket.Send(transfer->fileData, amount);

                if (transferred)
                {
                  // Something is in the pipe, modify the file pointer and the remaining
                  transfer->fileData += transferred;
                  transfer->remaining -= transferred;
                }
                else
                {
                  // It choked ... 
                  break;
                }
              }
              else
              {
                LDIAG("Send complete")

                // We're finished sending!
                transfer->socket.Close();

                // Cleanup
                transfer->Cleanup();

                transfer->stateCrit.Enter();
                if (transfer->state != TransferState::Aborted)
                {
                  transfer->state = TransferState::Completed;
                }
                transfer->stateCrit.Exit();
                quit = TRUE;
                break;
              }
            }
          }

          if (networkEvents.GetEvents() & FD_READ)
          {
            // Incomming data, read until it is exhausted
            for (;;)
            {
              U32 amount = Min(transfer->remaining, BlockSize);
              if (amount)
              {
                U32 transferred = transfer->socket.Recv(transfer->fileData, amount);

                if (transferred)
                {
                  // Something is in the pipe, modify the file pointer and the remaining
                  transfer->fileData += transferred;
                  transfer->remaining -= transferred;
                }
                else
                {
                  // There was no more data
                  break;
                }
              }
              else
              {
                // We received data and we weren't expecting it !
                break;
              }
            }
          }

          if (networkEvents.GetEvents() & FD_CLOSE)
          {
            // The socket has been closed
            if (transfer->remaining)
            {
              // Cleanup
              transfer->Cleanup();

              // The transfer wasn't completed!
              transfer->stateCrit.Enter();
              if (transfer->state != TransferState::Aborted)
              {
                transfer->state = TransferState::Error;
              }
              transfer->stateCrit.Exit();
            }
            else
            {
              // Cleanup
              transfer->Cleanup();

              // If recieving, move the file
              if ((transfer->tempName.str[0] != '\0') && (transfer->destName.str[0] != '\0'))
              {
                rename(transfer->tempName.str, transfer->destName.str);
              }

              // The transfer was completed
              transfer->stateCrit.Enter();
              if (transfer->state != TransferState::Aborted)
              {
                transfer->state = TransferState::Completed;
              }
              transfer->stateCrit.Exit();
            }
            quit = TRUE;
          }
        }
        else
        {
          // Quit event
          quit = TRUE;
        }
      }
      else
      {
        // The wait failed or timed out ... lets assumed it timed out
        nextTime += 1000;

        // Work out how much was transferred in the last second
        S32 amount = transferRemaining - transfer->remaining;
        if (transfer->rate)
        {
          U32 rate = ((transfer->rate * RateFilter) + (amount * RateFilterInv)) >> 8;
          if (rate == U32(amount))
          {
            transfer->rate = amount;
          }
          else
          {
            transfer->rate = rate;
          }
        }
        else
        {
          transfer->rate = amount;
        }

        transferRemaining = transfer->remaining;
      }
    }

    return (TRUE);
  }
}
