///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Management
//
// 16-DEC-1997
//

#include "filesys.h"
#include "filesrcstream.h"
#include "str.h"


namespace FileSys
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FileSrcStream - Allows streams to search other streams
  //

  //
  // Constructor
  //
  FileSrcStream::FileSrcStream(const char *targetName) : FileSrc(), targetName(targetName)
  {
    // We are now setup
    setup = TRUE;
  }

  
  //
  // HaveTarget
  //
  // True if target is alive (will attempt to setup if not)  
  //
  Bool FileSrcStream::HaveTarget()
  {
    // Is the target already alive
    if (target.Alive())
    {
      return (TRUE);
    }

    // Try and find the target
    ResourceStream *s = FindStream(targetName.crc);

    // Setup our reaper
    target = s;

    // Return TRUE if we found the target
    return (s ? TRUE : FALSE);
  }


  //
  // HasStreamReference
  //
  // Used to prevent recursive stream references - true if
  // this source points to stream 'crc', or if the stream
  // it points to in-turn points to stream 'crc'.
  //
  Bool FileSrcStream::HasStreamReference(U32 crc)
  {
    if (HaveTarget())
    {
      // Is this the one we're looking for
      if (target->streamId.crc == crc)
      {
        return (TRUE);
      }

      // Dig deeper
      return (target->HasStreamReference(crc));
    }

    return (FALSE);
  }


  //
  // BuildIndex
  //
  // Tell stream to rebuild all index's
  //
  void FileSrcStream::BuildIndex()
  {
    if (HaveTarget())
    {
      target->BuildIndex();
    }
  }


  //
  // Exists
  //
  // True if file 'crc' exists in this source
  //
  Bool FileSrcStream::Exists(U32 crc)
  {
    return (HaveTarget() ? target->Exists(crc) : FALSE);
  }


  //
  // GetFastFind
  //
  // Gets a fast find object from this source
  //
  FastFind* FileSrcStream::GetFastFind(const char *name, ResourceStream *)
  {
    return (HaveTarget() ? target->GetFastFind(name) : NULL);
  }


  //
  // Open
  //
  // Open a file from this source (null if not found)
  // 
  DataFile* FileSrcStream::Open(const char *name)
  {
    return (HaveTarget() ? target->Open(name) : NULL);
  }


  //
  // Path
  //
  // Returns the physical path that represents this source
  //
  const char * FileSrcStream::Path()
  {
    // Should never be called, but return current dir just incase
    return (".");
  }


  //
  // LogSource
  //
  // For debugging, logs source info
  //
  void FileSrcStream::LogSource(U32 indent)
  {
    ASSERT(IsSetup());

    // Generate the indent
    String iStr;
    iStr.Fill(indent, ' ');

    if (HaveTarget())
    {
      target->LogAllSources(indent);
    }
    else
    {
      LOG_DIAG(("%sDEAD STREAM", *iStr)); 
    }
  }
}