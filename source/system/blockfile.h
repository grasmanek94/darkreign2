///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Simple file access using string 'keys' to access data 'blocks'
//
// 13-FEB-1998
//

#ifndef __BLOCKFILE_H
#define __BLOCKFILE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "filesys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Format BLOCK_VER001 Description
//
// If the following blocks were added to a file :
// 
//   'Map Header'    - 100 bytes
//   'Map Data'      - 500 bytes
//    
// It would look like this :
// 
// ----
// 0000 BlockHeader
//   0000 Block Identifier BLOCK_ID defined below (U32)
//   0004 Format Version BLOCK_VER001 defined below (U32)
//   0008 Number of blocks in file '2' (U32)
//   0012 Unused data for future expansion [undefined] (32 bytes)
// 0044 BlockInfo
//   0044 Crc Key for "Map Header" '0x93AE4CE0' (U32)
//   0048 Size of data '100' (U32)
// 0052 Data for 'Map Header' (100 bytes)
// 0152 BlockInfo
//   0152 Crc Key for "Map Data" '0xAE9C47AC' (U32)
//   0156 Size of data '500' (U32)
// 0160 Data (100 bytes)
/// ----
//
// Notes  :
//  - there should be absolutely NO duplicate keys in a single block file
//  - a single 'block index' was not used so appending blocks is easier
//



///////////////////////////////////////////////////////////////////////////////
//
// Block Format Definitions
//

// Identifies a block file
#define BLOCK_ID       (*((U32*)"DOCB"))

// Version 1 id
#define BLOCK_VER001   0x001

// Current version
#define BLOCK_VER      BLOCK_VER001


#pragma pack(push, 1)

// Block header (written at start of file)
struct BlockHeader
{
  // Identifies a block file
  U32 blockId;
  
  // Identifies a block file// the format version
  U32 verId;

  // Number of blocks in file
  U32 blockCount;
  
  // For future expansion
  U8 notUsed[32];
};

// Block info (written at start of each block)
struct BlockInfo
{
  U32 key;
  U32 size;
};

#pragma pack(pop)


///////////////////////////////////////////////////////////////////////////////
//
// Class BlockFile - Allows creation, appending and reading of block files.
//

class BlockFile
{
public:

  // Mode passed to open method
  enum OpenMode { CREATE, APPEND, READ };

private:

  // Created when file opened for reading
  struct IndexEntry
  {
    BlockInfo info;
    U32 dataPos;
  };

  // Current file mode
  enum FileMode 
  { 
    FM_CLOSED, 
    FM_CREATE, 
    FM_APPEND, 
    FM_READ 
  } fileMode;

  // Current block mode
  enum BlockMode 
  { 
    BM_CLOSED, 
    BM_WRITE, 
    BM_READ 
  } blockMode;

  // Index entry for currently open block
  IndexEntry *cEntry;

  // Name of currently open file
  PathIdent nameId;
  
  // File header
  BlockHeader fileHeader;

  // File handle for writing
  File wFile;

  // File handle for reading
  FileSys::DataFile *rFile;

  // Memory mapped pointer
  U8 *mapPointer;

  // File position
  U32 filePos;

  // Tree of block entries
  BinTree<IndexEntry> index;

  // Set when an error occurs
  char lastError[256];
 
private:

  // Initialise data
  void Initialize();

  // Returns entry for 'key', or null if not found
  IndexEntry* GetIndexEntry(U32 key);

  // Create a new index entry
  IndexEntry* NewIndexEntry(U32 key, U32 size);

  // Seek to a position in the file, and update 'filePos'
  void WriteSeekTo(U32 pos);
  void ReadSeekTo(U32 pos);

  // Write data to the block file, and update 'filePos'
  void WriteData(const void *data, U32 size);

  // Read data into 'dest'. and update read position
  void ReadData(void *dest, U32 size);

  // Writes the block header to the currently open file
  void WriteBlockHeader();

public:

  // Constructor and destructor
  BlockFile();
  ~BlockFile();

  // Open a blockfile for reading, writing, or appending
  Bool Open(const char *name, OpenMode mode, Bool fatal = TRUE);

  // Close a new or open file, FALSE if error
  void Close(); 

  // Does 'key' exist in this file
  Bool FindKey(U32 key);

  // Returns the size of block 'key' which must exist
  U32 SizeOfBlock(U32 key);

  // Open a particular block (read/write depends on mode)
  Bool OpenBlock(U32 key, Bool fatal = TRUE, U32 *size = NULL);
  void CloseBlock();

  // Read/Write incrementally (read returns bytes read)
  U32 ReadFromBlock(void *dest, U32 size, Bool fatal = TRUE);
  void WriteToBlock(const void *data, U32 size);

  // Read/Write an entire block
  Bool ReadBlock(U32 key, void *dest, U32 size, Bool fatal = TRUE);
  Bool WriteBlock(U32 key, const void *data, U32 size, Bool fatal = TRUE);

  // Allocate block (use delete on return) and load 'key', setting 'size' if not NULL
  void* ReadBlockAlloc(U32 key, U32 *size, Bool fatal = TRUE);

  // Returns the memory mapped pointer of the current block
  U8 *GetBlockPtr();

  // Return the last error string
  const char* LastError() 
  { 
    return lastError; 
  }


  // Inline versions of above methods using string keys
  Bool FindKey(const char *keyStr)
  {
    return (FindKey(Crc::CalcStr(keyStr)));
  }
  Bool OpenBlock(const char *keyStr, Bool fatal = TRUE, U32 *size = NULL)
  {
    return (OpenBlock(Crc::CalcStr(keyStr), fatal, size));
  }
  Bool ReadBlock(const char *keyStr, void *dest, U32 size, Bool fatal = TRUE)
  {
    return (ReadBlock(Crc::CalcStr(keyStr), dest, size, fatal));
  }
  Bool WriteBlock(const char *keyStr, const void *data, U32 size, Bool fatal = TRUE)
  {
    return (WriteBlock(Crc::CalcStr(keyStr), data, size, fatal));
  }
  void* ReadBlockAlloc(const char *keyStr, U32 *size, Bool fatal = TRUE)
  {
    return (ReadBlockAlloc(Crc::CalcStr(keyStr), size, fatal));
  } 
};

  
#endif