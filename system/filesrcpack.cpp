///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Management
//
// 16-DEC-1997
//

#include "filesrcpack.h"
#include "str.h"


namespace FileSys
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DataFilePack - Reads a file from a pack
  //

  //
  // ClipRemaining
  //
  // Returns the number of bytes left in the file, or 
  // 'request', whichever is smaller.
  //
  U32 FileSrcPack::DataFilePack::ClipRemaining(U32 request)
  {
    ASSERT(readPos <= info->size);

    // Return smallest value
    if (info->size - readPos < request)
    {
      return (info->size - readPos);
    }
    else
    {
      return (request);
    }
  }


  //
  // Constructor
  //
  FileSrcPack::DataFilePack::DataFilePack(const char *nameIn, FileSrcPack *source, FileInfo *info) 
  : DataFile(nameIn),
    source(source),
    info(info),
    readPos(0)
  {
    // Map portion of file
    memoryPtr = (U8 *)source->packFile.MapMemory(name.str, File::MAP_READ, info->offset, info->size, &alignedPtr);
  }


  //
  // Destructor
  //
  FileSrcPack::DataFilePack::~DataFilePack()
  {
    // Unmap portion of file using the aligned memory pointer
    source->packFile.UnmapMemory(alignedPtr);
  }


  //
  // Size
  //
  // Returns the file size in bytes
  //
  U32 FileSrcPack::DataFilePack::Size()
  {
    return (info->size);
  }


  //
  // Read
  //
  // Read 'size' bytes into 'dst'.  returns bytes actually read.
  //
  U32 FileSrcPack::DataFilePack::Read(void *dst, U32 size)
  {
    ASSERT(memoryPtr)

    // How many bytes are left
    U32 read = ClipRemaining(size);

    // Only actually read if any bytes remaining
    if (read > 0)
    {
      memcpy(dst, memoryPtr + readPos, size);
      readPos += read;
    }

    return (read);
  }


  //
  // Seek
  //
  // Jumps to the position 'offset' bytes from the start of the file.
  // returns TRUE if 'offset' is a valid position within the file
  // (offset < file size), othewise returns FALSE
  //
  Bool FileSrcPack::DataFilePack::Seek(U32 offset)
  {
    if (offset < info->size)
    {
      readPos = offset;
      return (TRUE);
    }
    
    return (FALSE);
  }


  //
  // FilePos
  //
  // Returns the current file position in bytes
  //
  U32 FileSrcPack::DataFilePack::FilePos()
  {
    return (readPos);
  }


  //
  // GetMemoryPtr
  //
  // Return the memory mapped address of the file
  //
  void *FileSrcPack::DataFilePack::GetMemoryPtr()
  {
    ASSERT(memoryPtr)
    return (memoryPtr);
  }


  //
  // Path
  //
  // Returns the physical path that represents this file
  //
  const char * FileSrcPack::DataFilePack::Path()
  {
    return (source->Path());
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FastFindPack - Fast find class for this source
  //

  //
  // Open
  //
  // Open this file 
  //
  DataFile* FileSrcPack::FastFindPack::Open()
  { 
    return (new DataFilePack(name.str, source, fileInfo));
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FileSrcPack - Pack file source
  //

  //
  // ReadData
  //
  // Reads data from the pack file at the current read pos
  //
  void FileSrcPack::ReadData(void *dest, U32 size)
  {
    ASSERT(packFile.IsOpen());

    if (packFile.Read(dest, size, 1) != size)
    {
      ERR_FATAL(("Error reading %d bytes from pack file '%s'", size, pathId.str));
    }
  }


  //
  // Constructor
  //
  FileSrcPack::FileSrcPack(const char *path) : FileSrc()
  {
    // Take a copy of the file name
    pathId = path;

    // Check that the file exists
    if (!File::Exists(path))
    {
      return;
    }

    // Open pack file
    if (!packFile.Open(path, File::READ))
    {
      LOG_ERR(("Open failed after file (%s) was found to exist", path));
      return;
    }

    // Read header
    ReadData(&packInfo, sizeof(packInfo));

    // Check format id
    if (packInfo.packId != PACK_ID)
    {
      LOG_WARN(("'%s' is not a valid pack file", path));
      return;
    }

    // Check version
    switch (packInfo.verId)
    {
      // All supported versions
      case PACK_VER001:
        break;

      // Unknown version
      default :
        LOG_WARN(("Unsupported pack version '%s' (0x%08x)", path, packInfo.verId));
        return;
    }
 
    // Seek to start of file info block
    if (!packFile.Seek(File::SET, packInfo.fileOffset))
    {
      LOG_WARN(("Error seeking within pack (%s)", path));
      return;
    }

    // For reading packed file names
    PackStr name;
    U8 packedLen;

    // Read each item
    for (U32 i = 0; i < packInfo.fileCount; i++)
    {
      // Allocate new info structure
      FileInfo *info = new FileInfo;

      // Read and ignore dir index
      ReadData(&packedLen, sizeof(U32));

      // Read file name and store crc
      ReadData(&packedLen, sizeof(U8));
      ReadData(name, packedLen);
      name[packedLen] = '\0';

      // Read file offset and size
      ReadData(&info->offset, sizeof(U32));
      ReadData(&info->size, sizeof(U32));

      // Add to known list
      infoTree.Add(Crc::CalcStr(name), info);
    }

    // We can now be used
    setup = TRUE;

    ASSERT(packInfo.fileCount == infoTree.GetCount());
  }


  //
  // Destructor
  //
  FileSrcPack::~FileSrcPack()
  {
    packFile.Close();
    infoTree.DisposeAll();
  }


  //
  // BuildIndex
  //
  // The index for this source can not be refreshed
  //
  void FileSrcPack::BuildIndex()
  {
  }


  //
  // Exists
  //
  // True if file 'crc' exists in this source
  //
  Bool FileSrcPack::Exists(U32 crc)
  {
    ASSERT(IsSetup());
    return (infoTree.Exists(crc));
  }


  //
  // GetFastFind
  //
  // Gets a fast find object from this source
  //
  FastFind* FileSrcPack::GetFastFind(const char *name, ResourceStream *stream)
  { 
    ASSERT(IsSetup());

    // See if file is stored in pack
    FileInfo *info = infoTree.Find(Crc::CalcStr(name));

    // Setup fast find
    return (info ? new FastFindPack(name, info->size, stream, this, info) : NULL);
  }


  //
  // Open
  //
  // Open a file from this source (null if not found)
  // 
  DataFile* FileSrcPack::Open(const char *name)
  {
    ASSERT(IsSetup());
  
    // Get info for this file
    FileInfo *info = infoTree.Find(Crc::CalcStr(name));

    // Open the file
    return (info ? new DataFilePack(name, this, info) : NULL);
  };


  //
  // Path
  //
  // Returns the physical path that represents this source
  //
  const char * FileSrcPack::Path()
  {
    static PathIdent path;

    // Copy this source's pack name
    path = pathId;

    // Look for a directory slash
    char *ptr = Utils::Strrchr(path.str, '\\');

    if (ptr)
    {
      *ptr = '\0';
      return (path.str);
    }

    // Pack exists in the current directory
    return (".");      
  }


  //
  // LogSource
  //
  // For debugging, logs source info
  //
  void FileSrcPack::LogSource(U32 indent)
  {
    ASSERT(IsSetup());

    // Generate the indent
    String iStr;
    iStr.Fill(indent, ' ');

    LOG_DIAG(("%sPACK %s", *iStr, pathId.str));
  }
}