///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sync String System
//
// 28-JUL-1998
//

#ifndef __SYNC_H
#define __SYNC_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#pragma warning(push, 3)
#include <strstream>
#pragma warning(pop)
using std::ostrstream;
using std::endl;
using std::ends;
using std::ios;

#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define SYNC(a)                   \
{                                 \
  Sync::Set(__FILE__, __LINE__);  \
  Sync::stream << a << ends;      \
  Sync::SyncEnd();                \
}

#define NONSYNC(a)                \
{                                 \
  Sync::Set(__FILE__, __LINE__);  \
  Sync::stream << a << ends;      \
  Sync::NonSyncEnd();             \
}

#define SYNCCALLER Sync::Caller();
#define SYNCDUMP   Sync::DumpCallStack(__FILE__, __LINE__);


#ifdef SYNC_BRUTAL_ACTIVE
  #define SYNC_BRUTAL(x) SYNC(x)
#else
  #define SYNC_BRUTAL(x)
#endif


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Sync
//
namespace Sync
{

  // Stream which syncs are contructed on
  extern ostrstream stream; 

  // Init: Initialize Sync String system
  void Init();

  // Done: Shutdown Sync String system
  void Done();

  // Reset: Reset Sync String system
  void Reset();

  // Set: Set the file and line
  void Set(const char *file, U32 line);

  // End: End of a sync string line
  void SyncEnd();

  // End: End of a non sync string line
  void NonSyncEnd();

  // Save: Save the current CRC
  void Save();

  // GetSavedCrc: Return the saved CRC
  U32 GetSavedCrc();

  // GetSavedCycle: Return the saved CYCLE
  U32 GetSavedCycle();

  // IndexFirst: Return the index of the first Crc
  U32 IndexFirst();

  // IndexNext: Return the index of the next Crc
  U32 IndexNext();

  // Caller: Report the calling function
  void Caller();

  // DumpCallStack
  void DumpCallStack(const char *_file, U32 _line);

  // Test: Compare the given CRC with an older CRC
  Bool Test(U32 cycle, U32 crc, const GameIdent &name);

  // SyncObjects: Sync current object states
  void SyncObjects(Bool units);

  // Dump: Dumps all of the previous syncs to the LOG file
  void Dump();

  // DumpType
  template <class TYPE> const char * DumpType(const TYPE &type)
  {
    static char buf[sizeof(TYPE) * 2 + 1];
    Utils::CharEncode(buf, (U8*)(&type), sizeof(TYPE), sizeof(TYPE) * 2 + 1);
    return (buf);
  }
};

#endif