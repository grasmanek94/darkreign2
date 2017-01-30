////////////////////////////////////////////////////////////////////////////////
//
// StyxNet
//
// Copyright 1999-2000
// Matthew Versluys
//

#ifndef __STYXNET_PRIVATE_H
#define __STYXNET_PRIVATE_H


////////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "styxnet.h"
#include "logging.h"
#include "styxnet_event.h"
#include "styxnet_std.h"


////////////////////////////////////////////////////////////////////////////////
//
// Warnings
//
#pragma warning(disable: 4200)


////////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define CAST(type, var, value) type var = reinterpret_cast<type>(value);


////////////////////////////////////////////////////////////////////////////////
//
// NameSpace StyxNet
//
namespace StyxNet
{

  // Buffer size
  const U32 serverBufferSize = 8192;

  // Incoming buffer size
  const U32 clientBufferSize = 8192;

  // Maximum sync data at once
  const U32 maxSyncDataSize = 1024;

  // Default update interval
  const U32 defaultUpdateInterval = 500;

  // Event queue size
  const U32 eventQueueSize = 256;


  // Add client
  void AddClient();

  // Remove client
  void RemoveClient();

  // Add server
  void AddServer();

  // Remove server
  void RemoveServer();

}


#endif