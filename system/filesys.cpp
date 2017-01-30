///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Management
//
// 16-DEC-1997
//


//
// Includes
//
#include "filesrcdir.h"
#include "filesrcpack.h"
#include "filesrcstream.h"
#include "str.h"


namespace FileSys
{
  // Static Data
  static Bool sysInit = FALSE;
  static ResourceStreamPtr cStream;
  static NList<ResourceStream> streams(&ResourceStream::node);
  static List<FileSys::KeyDirPair> subPairs;
  static DTrack *dTracker;
  static FileString packExtension(".zwp");
  static FileString packWildCard("*.zwp");


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DataFile - A virtual file interface
  //

  //
  // List of all existing data files
  //
  NList<DataFile> DataFile::list(&DataFile::node);


  //
  // Constructor
  //
  DataFile::DataFile(const char *name) : name(name)
  {
    // Add to list
    list.Append(this);
  }

  
  //
  // Destructor
  //
  DataFile::~DataFile()
  {
    // Remove from list
    list.Unlink(this);
  }


  //
  // LogOpen
  //
  // Log all open data files
  //
  void DataFile::LogOpen()
  {
    LOG_DIAG(("DataFile: open files [%d]", list.GetCount()));

    for (NList<DataFile>::Iterator i(&list); *i; ++i)
    {
      LOG_DIAG((" [%s]", (*i)->Name()));
    }
  }


  //
  // DataCrc
  //
  // Crc of the file data
  //
  U32 DataFile::DataCrc(U32 crc)
  {
    return (Crc::Calc(GetMemoryPtr(), Size(), crc));
  }


  //
  // UniqueId
  //
  // Returns a unique identifier for this file
  //
  U32 DataFile::UniqueId()
  {
    return (Crc::CalcStr(Name(), Crc::CalcStr(Path())));
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FastFind - Interface to a fast find for a virtual data file
  //

  //
  // Constructor
  //
  FastFind::FastFind(const char *name, U32 size, ResourceStream *stream) : 
    name(name), size(size), stream(stream)
  { 
  }


  //
  // StreamAlive
  //
  // True if the stream this points to is still alive
  //
  Bool FastFind::StreamAlive()
  {
    return (stream.Alive());
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FileSrc - A single file source (directory/pack/stream)
  //

  //
  // Constructor
  //
  FileSrc::FileSrc()
  {
    ASSERT(sysInit);

    setup = FALSE;
    dTracker->RegisterConstruction(dTrack);
  }


  //
  // Destructor
  //
  FileSrc::~FileSrc()
  {
    dTracker->RegisterDestruction(dTrack);
  }


  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ResourceStream - A single resource stream
  //

  //
  // Constructor 
  //
  ResourceStream::ResourceStream(const char *stream) : sources(&FileSrc::node)
  {
    streamId = stream;
    readOnly = FALSE;
    dTracker->RegisterConstruction(dTrack);
  }


  //
  // Destructor
  //
  ResourceStream::~ResourceStream()
  {
    sources.DisposeAll();
    dTracker->RegisterDestruction(dTrack);
  }


  //
  // HasStreamReference
  //
  // Used to prevent recursive stream references - TRUE if
  // this stream has a file source that points to 'crc'
  //
  Bool ResourceStream::HasStreamReference(U32 crc)
  {
    // Check each file source
    for (NList<FileSrc>::Iterator i(&sources); *i; i++)
    {
      // Does this file source reference 'crc'
      if ((*i)->HasStreamReference(crc))
      {
        // Bingo
        return (TRUE);
      }
    }

    // No reference within this stream
    return (FALSE);
  }


  //
  // BuildIndex
  //
  // Builds the current file index for this source
  //
  void ResourceStream::BuildIndex()
  {
    // Call each file source
    for (NList<FileSrc>::Iterator i(&sources); *i; (i++)->BuildIndex());
  }


  //
  // Exists
  //
  // True if file 'crc' exists in this stream
  //
  Bool ResourceStream::Exists(U32 crc)
  { 
    // Check each file source
    for (NList<FileSrc>::Iterator i(&sources); *i; i++)
    {   
      if ((*i)->Exists(crc))
      {
        return (TRUE);
      }
    }
  
    // Not found
    return (FALSE);
  }


  //
  // GetFastFind
  //
  // Get a fast find object for a file
  //
  FastFind * ResourceStream::GetFastFind(const char *name)
  {   
    FastFind *fastFind = NULL;

    // Check each file source
    for (NList<FileSrc>::Iterator i(&sources); *i && !fastFind; i++)
    {   
      fastFind = (*i)->GetFastFind(name, this);
    }

    return (fastFind);
  }


  //
  // Open
  //
  // Open a file from this stream using a file name
  //
  DataFile * ResourceStream::Open(const char *name)
  {    
    DataFile *file = NULL;

    // Check each file source
    for (NList<FileSrc>::Iterator i(&sources); *i && !file; i++)
    {
      file = (*i)->Open(name);
    }

    return (file);
  }


  //
  // LogAllSources
  //
  // For debugging, logs all current sources
  //
  void ResourceStream::LogAllSources(U32 indent)
  {
    // Generate the indent
    String iStr;
    iStr.Fill(indent, ' ');

    LOG_DIAG(("%sSTREAM %s", *iStr, streamId.str)); 
  
    // log each source
    for (NList<FileSrc>::Iterator i(&sources); *i; i++)
    {
      (*i)->LogSource(indent + 2);
    }
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DataFileDirect - Direct access to a file without a source
  //

  //
  // Constructor
  //  
  DataFileDirect::DataFileDirect(const char *path, const char *nameIn) :
    DataFile(nameIn),
    path(path),
    memoryPtr(NULL)
  {
    file.Open(path, name.str, File::READ);
  }


  //
  // Destructor
  //
  DataFileDirect::~DataFileDirect()
  {
    if (memoryPtr)
    {
      file.UnmapMemory(memoryPtr);
      memoryPtr = NULL;
    }

    file.Close();
  }

  
  //
  // IsOpen
  //
  // Was the file successfully opened
  //  
  Bool DataFileDirect::IsOpen()
  {
    return (file.IsOpen());
  }


  //
  // Size
  //
  // Returns the file size in bytes
  //
  U32 DataFileDirect::Size()
  {
    return (file.GetSize());
  }


  //
  // Read
  //
  // Read 'size' bytes into 'dst'.  returns bytes actually read.
  //
  U32 DataFileDirect::Read(void *dst, U32 size)
  {
    return (file.Read(dst, 1, size));
  }


  //
  // Seek
  //
  // Jumps to the position 'offset' bytes from the start of the file.
  // Returns TRUE if valid position, FALSE otherwise
  //
  Bool DataFileDirect::Seek(U32 offset)
  {
    return (file.Seek(File::SET, offset));
  }


  //
  // FilePos
  //
  // Returns the current file position in bytes
  //
  U32 DataFileDirect::FilePos()
  {
    return (file.Tell());
  }


  //
  // GetMemoryPtr
  //
  // Return the memory mapped address of the file
  //
  void *DataFileDirect::GetMemoryPtr()
  {
    if (memoryPtr == NULL)
    {
      memoryPtr = file.MapMemory(name.str);
    }
    return (memoryPtr);
  }

  
  //
  // Path
  //
  // Returns the physical path that represents this file
  //
  const char * DataFileDirect::Path()
  {
    return (path.str);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  //
  // FindStream
  //
  // Returns stream 'crc' or null if not found
  //
  ResourceStream * FindStream(U32 crc)
  {    
    // Check each existing stream
    for (NList<ResourceStream>::Iterator i(&streams); *i; i++)
    {
      // Is this the one we're after
      if ((*i)->streamId.crc == crc)
      {
        return (*i);
      }
    }

    return (NULL);
  }


  //
  // GetStream
  //
  // Finds or creates 'stream', never returns NULL
  //
  static ResourceStream * GetStream(const char *stream)
  {
    // Does the stream already exist
    ResourceStream *s = FindStream(Crc::CalcStr(stream));
  
    // If not, create a new one
    if (!s)
    {
      // Allocate
      s = new ResourceStream(stream); 

      // Add to resource stream list
      streams.Append(s);
    }

    // Always successful
    return (s);
  }


  //
  // GetDirSub
  //
  // Returns a pointer to the sub pair 'key' or null
  //
  static KeyDirPair * GetDirSub(const char *key)
  {
    // Get crc of key
    U32 crc = Crc::CalcStr(key);

    // Find sub
    for (List<KeyDirPair>::Iterator i(&subPairs); *i; i++)
    {
      // Is this the one we're looking for
      if ((*i)->keyId.crc == crc)
      {
        return (*i);
      }
    } 

    // Didn't find the pair
    return (NULL);
  }


  //
  // Subst
  //
  // Does one key substitution on 'str'
  //
  static Bool Subst(char *dest, const char *str, KeyDirPair *pair)
  {
    ASSERT(dest);
    ASSERT(str);
    ASSERT(pair);

    char *p;

    // Does 'str' contain the substitution key
    if ((p = Utils::Strstr(str, pair->keyId.str)) == 0)
    {
      return (FALSE);
    }

    char *bp = dest;
    const char *s = str;

    // Work out how many chars in front of sub key
    U32 c = (U32) (p - s);

    ASSERT(c >= 0);

    // Copy over initial data
    Utils::Strncpy(bp, s, c);
    bp += c;
    s += c;

    // Copy over key sub chars
    for (char *sp = pair->dirId.str; *sp;)
    {
      *bp++ = *sp++;
    }

    // Move source past the key
    s += Utils::Strlen(pair->keyId.str);

    // Copy over remaining 
    while (*s)
    {
      *bp++ = *s++;
    }

    // Add terminating null
    *bp = '\0';

    return (TRUE);
  }

  //
  // ProcessRegisterDirSub
  //
  // Parse a RegisterDirSub("key", "dir"); function
  //
  static void ProcessRegisterDirSub(FScope *fScope)
  {
    ASSERT(fScope);

    // Must get first because of param evaluation order
    FileSysIdent keyId = fScope->NextArgString();

    // Register or change the key/dir pair
    RegisterDirSub(keyId.str, fScope->NextArgString());
  }


  //
  // ProcessConfigureStream
  //
  // Process a ConfigureStream("stream") {...} function
  //  
  static void ProcessConfigureStream(FScope *fScope)
  {
    ASSERT(fScope); 
    FScope *sScope; 
 
    // Get name of stream to configure
    FileSysIdent streamId = fScope->NextArgString();

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != 0)
    {
      switch (sScope->NameCrc())
      {
        case 0x9EE0321C: // "AddDir"
          AddSrcDir(streamId.str, SubStatic(sScope->NextArgString()));
          break;

        case 0x88D49578: // "AddPack"
          AddSrcPack(streamId.str, SubStatic(sScope->NextArgString()));
          break;

        case 0xA8C3233B: // "AddStream"
          AddSrcStream(streamId.str, sScope->NextArgString());
          break;

        case 0xAE33BA8A: // "AddDirRecurse"
          AddSrcDirRecurse(streamId.str, SubStatic(sScope->NextArgString()));
          break;

        case 0xAB492463: // "AddResource"
        {
          // Get the directory that holds the resource
          PathString path(SubStatic(sScope->NextArgString()));

          // Add the resource
          AddResource(streamId.str, path.str, sScope->NextArgString());
          break;
        }

        case 0x2FBD944F: // "AddResourceOptional"
        {
          // Get the directory that holds the resource
          PathString path(SubStatic(sScope->NextArgString()));

          // Add the resource
          AddResource(streamId.str, path.str, sScope->NextArgString(), FALSE);
          break;
        }

        // ignore anything else
        default:
          LOG_WARN(("Ignoring unexpected function '%s'", sScope->NameStr()));
          break;
      }
    }
  }


  //
  // ProcessSetActiveStream
  //
  // Process a SetActiveStream("stream"); function.
  //
  static void ProcessSetActiveStream(FScope *fScope)
  {
    ASSERT(fScope); 

    // Set the currently active stream
    SetActiveStream(fScope->NextArgString());
  }


  //
  // ProcessSetStreamReadOnly
  //
  // Process a SetStreamReadOnly("stream"); function.
  //
  static void ProcessSetStreamReadOnly(FScope *fScope)
  {
    ASSERT(fScope); 

    // Change stream to be read only
    SetStreamReadOnly(fScope->NextArgString());
  }


  //
  // ProcessDeleteStream
  //
  // Process a DeleteStream("stream"); function.
  //
  static void ProcessDeleteStream(FScope *fScope)
  {
    ASSERT(fScope); 

    // Delete specified stream
    DeleteStream(fScope->NextArgString());
  }


  //
  // ProcessDeleteAllStreams
  //
  // Process a DeleteAllStreams(); function.
  //
  static void ProcessDeleteAllStreams()
  {
    // Delete all current streams
    DeleteAllStreams();
  }

  
  //
  // ProcessPackExtension
  //
  // Process a PackExtension(".ext") function
  //
  static void ProcessPackExtension(FScope *fScope)
  {
    packExtension = fScope->NextArgString();
    Utils::Sprintf(packWildCard.str, packWildCard.GetSize(), "*%s", packExtension.str);
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!sysInit);

    // Allocate a death tracker
    dTracker = new DTrack("FileSys", 64);

    // Clear current stream
    cStream = NULL;

    sysInit = TRUE;
  }


  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(sysInit);
  
    DataFile::LogOpen();

    // Delete list items
    streams.DisposeAll();
    subPairs.DisposeAll();

    // Delete the death tracker
    delete dTracker;

    sysInit = FALSE;
  }


  //
  // GetPackExtension
  //
  // Get the configured pack file extension (e.g. "*.zwp")
  //
  const char * GetPackExtension()
  {
    return (packExtension.str);
  }

  
  //
  // GetPackWildCard
  //
  // Get the wildcard of the pack extension (e.g. "*.zwp")
  //
  const char * GetPackWildCard()
  {
    return (packWildCard.str);
  }


  //
  // Sub
  //
  // Processes 'path' for subs, placing result in and returning 'dest'
  //
  char * Sub(char *dest, const char *path)
  {
    ASSERT(path);
    ASSERT(dest);

    List<KeyDirPair>::Iterator i(&subPairs);
    Bool didSub = FALSE;

    // Buffers for substitutions
    PathIdent dBuf, sBuf;
    const char *sPtr = path;

    // Support multiple subs on single string
    for (!i; *i; i++)
    {
      // Process subs on source, put result in dest
      if (Subst(dBuf.str, sPtr, *i))
      {
        // Signal that we've done a substitution
        didSub = TRUE;

        // Copy result into source buffer
        sBuf = dBuf;

        // And point to it
        sPtr = sBuf.str;
      }
    } 

    // Copy final string into destination
    Utils::Strcpy(dest, didSub ? dBuf.str : path);

    // Return the string
    return (dest);
  }

  
  //
  // SubStatic
  //
  // Does a sub on 'path', but returns pointer to static string (ie. you must take copy)
  //
  const char * SubStatic(const char *path)
  {
    static PathString subPath;
    return (Sub(subPath.str, path));  
  }


  //
  // GetSub
  //
  // Returns the path for sub 'key', which MUST exist (fatal error if not)
  //
  char * GetSub(const char *key)
  {
    ASSERT(key);

    // Search for sub pair
    KeyDirPair *pair = GetDirSub(key);

    // Did we find it
    if (!pair)
    {
      ERR_FATAL(("Attempt to use GetSub when key does not exist (%s)", key));
    } 

    // Success 
    return (pair->dirId.str);
  }
  

  //
  // RegisterDirSub
  //
  // Register (or change) a key/directory substitution pair
  //
  void RegisterDirSub(const char *key, const char *dir)
  {
    KeyDirPair *pair = GetDirSub(key);

    // Do a dir sub on the source dir
    dir = SubStatic(dir);

    // Does this key already exist
    if (pair)
    {
      // Change the directory it points to
      pair->dirId = dir;
    }
    else
    {
      // Create a new pair
      pair = new KeyDirPair;
      pair->keyId = key;
      pair->dirId = dir;

      // Add to list
      subPairs.Append(pair);
    }
  }


  //
  // AddSrcDir
  //
  // Add a directory source to an existing resource stream
  // False if directory doesn't exist
  //
  Bool AddSrcDir(const char *stream, const char *dir)
  {
    // Create directory specific file source instance
    FileSrcDir *newSrc = new FileSrcDir(dir);

    // If not setup, delete the source
    if (!newSrc->IsSetup())
    {  
      delete newSrc;
      return (FALSE);
    }

    // Add the source
    GetStream(stream)->sources.Append(newSrc);

    // Success
    return (TRUE);
  }


  //
  // AddSrcPack
  //
  // Add a pack file to 'stream'
  // False if pack file doesn't exist
  //
  Bool AddSrcPack(const char *stream, const char *path)
  {
    // Create pack specific file source instance
    FileSrcPack *newSrc = new FileSrcPack(path);

    // If unable to setup for this pack, delete the source
    if (!newSrc->IsSetup())
    {  
      delete newSrc;
      return (FALSE);
    }

    // Add the source
    GetStream(stream)->sources.Append(newSrc);

    // Success
    return (TRUE); 
  }


  //
  // AddSrcStream
  //
  // Add a sub-stream (can be done before 'addStream' exists)
  // False on error
  //
  Bool AddSrcStream(const char *stream, const char *addStream)
  { 
    // Get the stream we're trying to point to
    ResourceStream *s = FindStream(Crc::CalcStr(addStream));

    // See if it refers back to the stream we're adding to
    if (s && s->HasStreamReference(Crc::CalcStr(stream)))
    {  
      LOG_WARN(("Recursive stream reference ignored! (\"%s\")", addStream));
      return (FALSE);
    }

    // Add the source
    GetStream(stream)->sources.Append(new FileSrcStream(addStream));

    return (TRUE);
  }


  //
  // AddSrcDirRecurse
  //
  // Same as AddSrcDir, but recursive
  //
  Bool AddSrcDirRecurse(const char *stream, const char *dir)
  {
    Dir::Find find;

    // Add this directory
    if (!AddSrcDir(stream, dir))
    {
      return (FALSE);
    }

    // Add all sub-dirs
    if (Dir::FindFirst(find, dir, "*.*", File::Attrib::SUBDIR))
    {
      do
      {
        // Exclude previous and current dirs
        if (*find.finddata.name != '.')
        {
          // Make full path to this sub-dir
          FilePath subDir;
          Dir::PathMake(subDir, dir, find.finddata.name);

          // Add this sub-dir and all below it
          if (!AddSrcDirRecurse(stream, subDir.str))
          {
            LOG_WARN(("Unable to add sub-dir '%s' to stream '%s'", find.finddata.name, stream));
          }
        }
      } 
      while (Dir::FindNext(find));
    }  

    // Finish find operation
    Dir::FindClose(find);

    // Found directory
    return (TRUE);
  }

  
  //
  // AddResource
  //
  // Attempts to add a directory or a pack
  //
  Bool AddResource(const char *stream, const char *dir, const char *name, Bool required)
  {
    PathString path;

    // Try and add the directory
    Utils::MakePath(path.str, path.GetSize(), dir, name, NULL);
    Bool success1 = AddSrcDirRecurse(stream, path.str);

    // Try and add the pack
    Utils::MakePath(path.str, path.GetSize(), dir, name, packExtension.str);
    Bool success2 = FileSys::AddSrcPack(stream, path.str);

    // Did both sources fail, and the resource is required
    if (!success1 && !success2 && required)
    {
      ERR_FATAL(("Unable to add required resource [%s][%s]", dir, name));
    }

    return (TRUE);
  }

  
  //
  // GetActiveStream
  //
  // Get's the currently active stream
  //
  const char * GetActiveStream()
  {
    return (cStream.Alive() ? cStream->streamId.str : NULL);
  }


  //
  // SetActiveStream
  //
  // Set active resource stream, clearing if not found
  // False if not found
  //
  Bool SetActiveStream(const char *stream)
  {
    if (stream)
    {
      cStream = FindStream(Crc::CalcStr(stream));
      return (cStream.Alive());
    }
    else
    {
      cStream = NULL;
      return (TRUE);
    }
  }


  //
  // SetStreamReadOnly
  //
  // Makes 'stream' read-only (ie. can never be deleted)
  // False if not found
  //
  Bool SetStreamReadOnly(const char *stream)
  {
    ResourceStream *s = FindStream(Crc::CalcStr(stream));

    // If we found the stream, set read-only flag
    if (s)
    {
      s->readOnly = TRUE;
      return (TRUE);
    }

    // Didn't find the stream
    return (FALSE);
  }


  //
  // DeleteStream
  //
  // Delete 'stream' (true if was actually deleted)
  //
  Bool DeleteStream(const char *stream)
  {
    // Get existing stream
    ResourceStream *s = FindStream(Crc::CalcStr(stream));

    // Did we find it and not read-only
    if (s && !s->readOnly)
    {
      // Are we currently pointing to it
      if (s == cStream)
      {
        cStream = NULL;
      }

      // Delete it
      streams.Dispose(s);

      return (TRUE);
    }

    // didn't delete it
    return (FALSE);
  }


  //
  // DeleteAllStreams
  //
  // Delete all streams except read-only ones. 
  // true if any stream is deleted.
  //
  Bool DeleteAllStreams()
  {
    NList<ResourceStream>::Iterator i(&streams);
    Bool deleted = FALSE; 
    ResourceStream *s;
  
    // Check each existing stream
    while ((s = i++) != NULL)
    {
      // Ignore if read-only flag is set
      if (!s->readOnly)
      {
        // Clear current stream if being deleted
        if (s == cStream)
        {
          cStream = NULL;
        }

        // Delete it
        streams.Dispose(s);

        // Flag that we've deleted at least one
        deleted = TRUE;
      }
    }

    return (deleted);
  }


  //
  // Exists
  //
  // Check if a file exists in current stream
  //
  Bool Exists(const char *name)
  {
    // Does the name contain a directory
    if (Utils::Strchr(name, '\\'))
    {
      return (File::Exists(name));
    }

    // Do we have a current stream
    if (cStream.Alive())
    {
      // Search the stream for the file
      return (cStream->Exists(Crc::CalcStr(name)));
    }

    // No active stream
    return (FALSE);
  }


  //
  // BuildIndexes
  //
  // Build file indexes (optionally for a specific stream)
  //
  Bool BuildIndexes(const char *stream)
  {
    // Get target stream
    ResourceStream *s = NULL;

    if (stream)
    {    
      s = FindStream(Crc::CalcStr(stream));
    }
    else

    if (cStream.Alive())
    {
      s = cStream;
    }    

    if (s)
    {
      s->BuildIndex();
      return (TRUE);
    }

    return (FALSE);
  }


  //
  // GetFastFind
  //
  // Get a fast find object for a file
  //
  FastFind * GetFastFind(const char *name)
  {
    // Do we have a current stream
    if (cStream.Alive())
    {
      // Search the stream for the file
      return (cStream->GetFastFind(name));
    }

    // No active stream
    return (NULL);   
  }


  //
  // Open
  //
  // Open a file directly using a directory and name
  //
  DataFile * Open(const char *dir, const char *name)
  {
    // Does the file exist
    if (File::Exists(dir, name))
    {
      // Allocate a new direct data file
      DataFileDirect *d = new DataFileDirect(dir, name);

      // Was the file successfully opened
      if (!d->IsOpen())
      {
        delete d;
        LOG_ERR(("Expected to directly open %s in %s!", name, dir));
      }
      else
      {
        return (d);
      }
    }

    // Failed
    return (NULL);
  }


  //
  // Open
  //
  // Open a file, optionally including a directory
  //
  DataFile * Open(const char *path)
  {
    // Does the name contain a directory
    if (const char *slash = Utils::Strrchr(path, '\\'))
    {
      PathString dir;

      // Copy the path
      Utils::Strmcpy(dir.str, path, ++slash - path);

      // Open the file
      return (Open(dir.str, slash));
    }

    // Do we have a current stream
    if (cStream.Alive())
    {
      // Search the stream for the file
      return (cStream->Open(path));
    }
  
    return (NULL);
  }


  //
  // Open
  //
  // Open a file using a fast find class
  //
  DataFile * Open(FastFind *fastFind)
  {
    ASSERT(fastFind);  

    // Does stream still exist
    if (fastFind->StreamAlive())
    {
      // Open the file
      return (fastFind->Open());
    }

    LOG_WARN(("FastFind failed (%s) : stream deleted!", fastFind->Name()));
    return (NULL);
  }


  //
  // Close
  //
  // Close and delete the DataFile pointed to by 'file'
  //
  void Close(DataFile *file)
  {
    ASSERT(file);

    // Just delete the file
    delete file;
  }


  //
  // ProcessConfigScope
  //
  // Parse 'fScope' for file system setup functions
  //
  void ProcessConfigScope(FScope *fScope)
  {
    ASSERT(fScope);
  
    // Pointer to sub-scopes
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != 0)
    {
      switch (sScope->NameCrc())
      {
        case 0x2B450588: // "SetSub"
          ProcessRegisterDirSub(sScope);
          break;

        case 0x7583EA7D: // "ConfigureStream"
          ProcessConfigureStream(sScope);
          break;

        case 0xB54D014C: // "SetActiveStream"
          ProcessSetActiveStream(sScope);
          break;

        case 0x6993308D: // "SetStreamReadOnly"
          ProcessSetStreamReadOnly(sScope);
          break;

        case 0x3A79CCC8: // "DeleteStream"
          ProcessDeleteStream(sScope);
          break;

        case 0xAF7D752D: // "DeleteAllStreams"
          ProcessDeleteAllStreams();
          break;

        case 0x16D4B4B4: // "PackExtension"
          ProcessPackExtension(sScope);
          break;

        // Warn if unknown
        default:
          LOG_WARN(("Ignoring unexpected function '%s'", sScope->NameStr()));
          break;
      }
    }
  }


  //
  // LogAllSources
  //
  // For debugging, logs all current sources
  //
  void LogAllSources()
  {
    LOG_DIAG(("Logging FileSys Stream Data"));

    // Log all them streams
    for (NList<ResourceStream>::Iterator i(&streams); *i; i++)
    {
      if (cStream.Alive() && (*i == cStream))
      {
        LOG_DIAG(("Current Stream"))
      }

      (*i)->LogAllSources(0);
    }
  }
}