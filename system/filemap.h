///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File Mapping
// 23-MAR-1999
//


#ifndef __FILEMAP_H
#define __FILEMAP_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// Class FileMap
//
class FileMap
{
private:

  HANDLE fileHandle;
  HANDLE mapHandle;
  U32 size;
  U8 *ptr;

public:

  enum Flags
  {
    READ = 0x01,
    WRITE = 0x02
  };

  // Constructor and destructor
  FileMap(const char *name, U32 flags, U32 offset = 0, U32 length = 0);
  ~FileMap();

  // Get the size of the file
  U32 GetSize();

  // Get the pointer to the mapping
  U8 * GetPtr();

};


#endif
