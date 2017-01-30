////////////////////////////////////////////////////////////////////////////////
//
// Network Ping
//
// Copyright 1999-2000
// Matthew Versluys
//


#ifndef __NETWORK_HTTP_H
#define __NETWORK_HTTP_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "network_url.h"
#include "win32_socket.h"


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace Network
//
namespace Network
{

  ////////////////////////////////////////////////////////////////////////////////
  //
  // Class Http
  //
  class Http
  {
  private:

    // Socket used for connecting the the http server
    Win32::Socket socket;

  public:

    // Open a connection to the given URL
    Bool Open(const Url &url, U32 &offset, U32 &size);

    // Close a the connection
    Bool Close();

    // Read from the HTTP connection
    Bool Read(U8 *buf, U32 size);

  };

}

#endif

