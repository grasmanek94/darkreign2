///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
//


#ifndef __MULTIPLAYER_DOWNLOAD_H
#define __MULTIPLAYER_DOWNLOAD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "file.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Download
  //
  namespace Download
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Type definitions
    //
    typedef StrBuf<256> HostName;


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Context
    //
    struct Context
    {
      // Download type
      U32 type;

      // Download handle
      U32 handle;

      // Name of the host to download from
      HostName name;

      // Resolved name of the host to download from
      HostName host;

      // Port on the host to download from
      U16 port;

      // Has the transfer been aborted
      Bool aborted;

      // Path on host to download from
      FilePath path;

      // Name of the file to download
      FileName file;

      // Size of the file
      U32 size;

      // Amount transfered
      U32 transferred;

      // Constructor
      Context()
      : type(0),
        handle(0)
      {
        Reset();
      }

      // Reset
      void Reset()
      {
        size = 0;
        transferred = 0;
        aborted = FALSE;
      }

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Extra
    //
    struct Extra
    {
      NList<Extra>::Node node;
      
      GameIdent name;
      GameIdent author;
      U32 size;
      FilePath file;

      HostName sourceHost;
      U32 sourcePort;
      FilePath sourcePath;

      // Constructor
      Extra(FScope *fScope);
    };


    // Initialization and Shutdown
    void Init();
    void Done();

    // Abort
    void Abort();

    // Download the latest update file
    void GetUpdates();

    // Download the message of the day
    void GetMotd();

    // Check the version
    Bool CheckVersion();

    // Get the list of extras
    const NList<Extra> & GetExtras();

    // Abort download
    void AbortDownload();

    // Get the download context
    const Context & GetDownloadContext();

    // Message
    void Message(U32 message, void *data);

  }
}


#endif
