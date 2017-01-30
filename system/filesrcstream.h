///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Management
//
// 16-DEC-1997
//

#ifndef __FILESRCSTREAM_H
#define __FILESRCSTREAM_H

//
// Includes
//
#include "filesys_private.h"


namespace FileSys
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FileSrcStream - Allows streams to search other streams
  //
  class FileSrcStream : public FileSrc
  {
  protected:

    // Name of the stream we're pointing at
    FileSysIdent targetName;

    // Target reaper
    ResourceStreamPtr target;

  public:
  
    // Constructor
    FileSrcStream(const char *targetName);

    // True if targetPtr is alive (will attempt to setup if not)
    Bool HaveTarget();

    // True if this stream refers to 'crc'
    Bool HasStreamReference(U32 crc);

    // Required overriding methods
    void BuildIndex();
    Bool Exists(U32 crc);
    FastFind* GetFastFind(const char *name, ResourceStream *stream);
    DataFile* Open(const char *name);
    const char * Path();
    void LogSource(U32 indent);
  };
}

#endif