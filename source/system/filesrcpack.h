///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Management
//
// 16-DEC-1997
//

#ifndef __FILESRCPACK_H
#define __FILESRCPACK_H


//
// Includes
//
#include "filesys_private.h"
#include "packdef.h"


namespace FileSys
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FileSrcPack - Packed data file source
  //
  class FileSrcPack : public FileSrc
  {
  protected:

    // Pack file
    File packFile;

    // Name of pack file
    PathIdent pathId;

    // Info for this pack file
    PackInfo packInfo;

    // Info for each stored file
    struct FileInfo
    {
      // Offset of this file into the pack file
      U32 offset;

      // Size of the file inside the pack
      U32 size;
    };

    // File info list
    BinTree<FileInfo> infoTree;

  protected:

    // For accessing files from this source
    class DataFilePack : public DataFile
    {
    protected:

      // Pointer to the pack file source
      Reaper<FileSrcPack> source;

      // Info about file data within pack file
      FileInfo *info;

      // Memory mapped pointer to this file entry
      U8 *memoryPtr;

      // Aligned memory map pointer (for internal use)
      void *alignedPtr;

      // Current relative read position
      U32 readPos;

    protected:

      // Ensure pack file seek is where we want it
      void AdjustSeek();

      // Clip 'request' to remaining bytes
      U32 ClipRemaining(U32 request);

    public:

      // Constructor
      DataFilePack(const char *name, FileSrcPack *source, FileInfo *info);

      // Destructor
      ~DataFilePack();

      // Required overriding methods
      U32 Size();
      U32 Read(void *dst, U32 size);
      Bool Seek(U32 offset);
      U32 FilePos();
      void *GetMemoryPtr();
      const char * Path();
    };

    // Needs to access pack data
    friend class DataFilePack;

    // For quick finds on files from this source type
    class FastFindPack : public FastFind
    {
    protected:

      // File source that this file is within
      FileSrcPack *source;

      // Info for file
      FileInfo *fileInfo;

      // Required method
      DataFile* Open();

    public:

      FastFindPack
      (
        const char *name, U32 size, ResourceStream *stream, 
        FileSrcPack *source, FileInfo *info
      ) 
      : FastFind(name, size, stream), source(source), fileInfo(info)
      {
      }
    };

    // Private pack reading methods
    void ReadData(void *dest, U32 size);

  public:

    // Constructor and destructor
    FileSrcPack(const char *path);
    ~FileSrcPack();

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