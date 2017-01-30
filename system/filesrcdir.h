///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Management
//
// 16-DEC-1997
//

#ifndef __FILESRCDIR_H
#define __FILESRCDIR_H


//
// Includes
//
#include "filesys_private.h"


namespace FileSys
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FileSrcDir - Direct access file source
  //
  class FileSrcDir : public FileSrc
  {
  protected:

    // Directory to load files from
    PathIdent dirId;

    // Index of known files
    BinTree<void> index;

  protected:

    // For accessing files from this source type
    class DataFileDir : public DataFile
    {
    protected:

      // File object
      File file;

      // Memory pointer
      void *memoryPtr;

      // File source containing this file
      Reaper<FileSrcDir> source;

    public:

      // Constructor and destructor
      DataFileDir(FileSrcDir *source, const char *name);
      ~DataFileDir();

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

    // For quick finds on files from this source type
    class FastFindDir : public FastFind
    {
    protected:

      // Required method
      DataFile* Open();

      // File source that this file is within
      FileSrcDir *source;

    public:

      FastFindDir(const char *name, U32 size, ResourceStream *stream, FileSrcDir *source) : 
        FastFind(name, size, stream), 
        source(source)
      {
      }
    };

  public:
  
    // Constructor and destructor
    FileSrcDir(const char *dir);
    ~FileSrcDir();

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