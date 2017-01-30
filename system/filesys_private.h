///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Managment
//
// 16-DEC-1997
//

#ifndef __FILESYS_PRIVATE_H
#define __FILESYS_PRIVATE_H


//
// Includes
//
#include "filesys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FileSys - Manages all file resource streams
//

namespace FileSys
{
  // Base class for all FileSrc's
  class FileSrc
  {
  protected :

    // Is this source setup
    Bool setup;

  public:

    // Death track info
    DTrack::Info dTrack;

    // List node
    NList<FileSrc>::Node node;

  public:

    // Constructor and destructor
    FileSrc();
    virtual ~FileSrc();

    // Builds the current file index for this source
    virtual void BuildIndex() = 0;

    // True if file 'crc' exists in this source
    virtual Bool Exists(U32 crc) = 0;

    // Get a fast find object for a file
    virtual FastFind* GetFastFind(const char *name, ResourceStream *stream) = 0;

    // Open a file from this source (null if not found)
    virtual DataFile* Open(const char *name) = 0;

    // Returns the physical path that represents this source
    virtual const char * Path() = 0;

    // Is this source successfully setup
    virtual Bool IsSetup()
    {
      return (setup);
    }

    // Used to prevent recursive stream references
    virtual Bool HasStreamReference(U32)
    {
      return (FALSE);
    }

    // For debugging, log source info
    virtual void LogSource(U32 indent) = 0;
  };


  // A single resource stream
  class ResourceStream
  {
  public :

    // Death track info
    DTrack::Info dTrack;

    // List node
    NList<ResourceStream>::Node node;

    // Stream identifier
    FileSysIdent streamId;

    // If true, stream can never be deleted once created
    Bool readOnly;

    // List of each file source in this stream
    NList<FileSrc> sources;

  public:

    // Constructor and destructor
    ResourceStream(const char *stream);
    ~ResourceStream();

    // Used to prevent recursive stream references
    Bool HasStreamReference(U32 crc);

    // Builds the current file index for this source
    void BuildIndex();

    // True if file 'crc' exists in this stream
    Bool Exists(U32 crc);

    // Get a fast find object for a file
    FastFind* GetFastFind(const char *name);

    // Open a file from this stream using a file name
    DataFile* Open(const char *name);

    // For debugging, logs all current sources
    void LogAllSources(U32 indent);
  };

  // Reaper to a file source
  typedef Reaper<FileSrc> FileSrcPtr;

  // For directory substitution
  struct KeyDirPair
  {
    FileSysIdent keyId;
    PathIdent dirId;
  };
 
  // Used in the stream source
  ResourceStream * FindStream(U32 crc);
};


#endif