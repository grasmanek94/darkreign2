///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Management
//
// 16-DEC-1997
//

#ifndef __FILESYS_H
#define __FILESYS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ptree.h"
#include "filetypes.h"
#include "dtrack.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FileSys - The file stream system
//
namespace FileSys
{
  // Forward declaration
  class ResourceStream;

  // Reaper to a resource stream
  typedef Reaper<ResourceStream> ResourceStreamPtr;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DataFile - Abstract interface to a data file
  //
  class DataFile
  {
  protected:

    // List of all existing data files
    static NList<DataFile> list;

    // List node
    NList<DataFile>::Node node;

    // Name of file opened
    FileIdent name;

  public:

    // Log all open data files
    static void LogOpen();

  public:

    // Constructor
    DataFile(const char *name);
    virtual ~DataFile();

    // Size in bytes of the file
    virtual U32 Size() = 0;

    // Read data into 'dst', returning bytes actually read
    virtual U32 Read(void *dst, U32 size) = 0;

    // Seek to a position in the file
    virtual Bool Seek(U32 offset) = 0;

    // Current file position in bytes
    virtual U32 FilePos() = 0;

    // Return the memory mapped address of the file
    virtual void *GetMemoryPtr() = 0;

    // Returns the physical path that represents this file
    virtual const char * Path() = 0;

    // Crc of the file data
    U32 DataCrc(U32 crc = 0);

    // Returns a unique identifier for this file
    U32 UniqueId();

    // Returns the name of this file
    const char * Name()
    {
      return (name.str);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FastFind - Abstract interface to a fast find for a data file
  //
  class FastFind
  {
  protected:

    // Name of this file
    FileIdent name;

    // Size of the file
    U32 size;
   
    // Stream that this file is within
    ResourceStreamPtr stream;

  public:

    // Constructor
    FastFind(const char *name, U32 size, ResourceStream *stream);

    // Returns the name of this file
    const char * Name()
    {
      return (name.str);
    }

    // Returns the size of this file
    U32 Size()
    {
      return (size);
    }

    // True if the stream this points to is still alive
    Bool StreamAlive();

    // Open this file
    virtual DataFile* Open() = 0;
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DataFileDirect - Direct access to a file without a source
  //
  class DataFileDirect : public DataFile
  {
  protected:

    // File object
    File file;

    // Memory pointer
    void *memoryPtr;

    // Path in which file exists
    PathString path;

  public:

    // Constructor and destructor
    DataFileDirect(const char *path, const char *name);
    ~DataFileDirect();

    // Was the file successfully opened
    Bool IsOpen();

    // Required overriding methods
    U32 Size();
    U32 Read(void *dst, U32 size);
    Bool Seek(U32 offset);
    U32 FilePos();
    void *GetMemoryPtr();
    const char * Path();
  };

  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Initialize/shutdown file system
  void Init();
  void Done();

  // Get the configured pack file extension (e.g. ".zwp")
  const char * GetPackExtension();

  // Get the wildcard of the pack extension (e.g. "*.zwp")
  const char * GetPackWildCard();

  // Register (or change) a key/directory substitution pair
  void RegisterDirSub(const char *key, const char *dir);

  // Processes 'path' for subs, placing result in and returning 'dest'
  char * Sub(char *dest, const char *path);

  // Does a sub on 'path', but returns pointer to static string
  const char * SubStatic(const char *path);

  // Returns the path for sub 'key', which MUST exist (fatal error if not)
  char * GetSub(const char *key);


  // Add a directory source to an existing resource stream
  Bool AddSrcDir(const char *stream, const char *dir);

  // Add a pack file to 'stream'
  Bool AddSrcPack(const char *stream, const char *path);

  // Add a sub-stream (can be done before 'addStream' exists)
  Bool AddSrcStream(const char *stream, const char *addStream);


  // Same as AddSrcDir, but recursive
  Bool AddSrcDirRecurse(const char *stream, const char *dir);

  // Attempts to add a directory or a pack
  Bool AddResource(const char *stream, const char *dir, const char *name, Bool required = TRUE);


  // Get the active stream (NULL if none)
  const char *GetActiveStream();

  // Set active resource stream (NULL to clear)
  Bool SetActiveStream(const char *stream);

  // Makes 'stream' read-only (ie. can never be deleted)
  Bool SetStreamReadOnly(const char *stream);


  // Delete 'stream' (true if was actually deleted)
  Bool DeleteStream(const char *stream);
  
  // Delete all streams except read-only ones (true if any stream is deleted)
  Bool DeleteAllStreams();


  // Build file indexes (optionally for a specific stream)
  Bool BuildIndexes(const char *stream = NULL);

  // Check if a file exists in current stream
  Bool Exists(const char *name);

  // Get a fast find object for a file
  FastFind * GetFastFind(const char *name);

  // Open a file directly using a directory and name
  DataFile * Open(const char *dir, const char *name);

  // Open a file, optionally including a directory
  DataFile * Open(const char *path);

  // Open a file using a fast find class
  DataFile * Open(FastFind *fastFind);

  // Close and delete a DataFile pointer
  void Close(DataFile *file);

  
  // Configuration processing 
  void ProcessConfigScope(FScope *fScope);
  
  // For debugging, logs all current sources
  void LogAllSources();
}


#endif