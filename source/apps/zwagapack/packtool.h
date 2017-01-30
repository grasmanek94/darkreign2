///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Pack file management code
//
// 08-JAN-1998
//

#ifndef __PACKTOOL_H
#define __PACKTOOL_H


//
// Includes
//
#include <stdio.h>
#include <io.h>
#include "std.h"
#include "packdef.h"


///////////////////////////////////////////////////////////////////////////////
//
// Pack Tool Definitions
//

// Bytes of memory used for copy buffer
//
#define PACKTOOL_WORKBUFSIZE    262144


///////////////////////////////////////////////////////////////////////////////
//
// Class PackTool - management of pack files
//

class PackTool
{
public:
  
  // Display callback
  typedef void (DisplayCallBack)(const char *);

private:

  // Copy buffer
  static U8 workBuf[PACKTOOL_WORKBUFSIZE];

  // Current operational mode
  enum { CLOSED, WRITING, READING } mode;

  // Pack file name
  char *packPath;

  // Pointer to file name component of 'packPath'
  const char *packName;
  U32 packNameCrc;

  // Pack file handle
  FILE *packFile;

  // Info for current pack file
  PackInfo packInfo;

  // Info for each stored file
  struct FileInfo
  {
    U32 dirIndex;
    PackStr fName;
    U32 offset;
    U32 size;
  };

  // Info for each stored directory
  struct DirInfo
  {
    PackStr dir;
  };

  // Custom draw function
  DisplayCallBack *customDisplay;
  
private:

  // Display a string using custom routine
  void CDECL Display(const char *format, ...);

  // Private writing methods
  void WriteData(const void *data, U32 size);
  void WritePackedString(const char *str);
  void WritePackInfo();
  void WriteFileInfo();
  void WriteDirInfo();
  U32 GetSubDirIndex(const char *sDir);
  void ProcessItem(_finddata_t *, const char *, const char *, const char *, Bool);
  void ProcessDir(const char *rDir, const char *sDir, const char *mask, Bool subs);

  // Private reading methods
  void ReadData(void *dest, U32 size);
  void ReadPackedString(PackStr dest);
  void ReadFileInfo();
  void ReadDirInfo();
  FileInfo* GetFileInfo(const char *fName);
  DirInfo* GetDirInfo(U32 index);
  void CreateFullDir(const char *path);
  Bool ExtractFile(FileInfo *info, const char *destDir);

  // Current write position
  U32 writePos;

  // All known files
  List<FileInfo> fileInfo;

  // All known directories
  List<DirInfo> dirInfo;

  // Tree of all file crc's
  BinTree<void> crcTree;

public:
  
  // Constructor and destructor
  PackTool(DisplayCallBack *display = NULL);
  ~PackTool();

  // Returns pointer to file name part of path
  static const char* NameFromPath(const char *path);

  // Functions for creating a new pack file
  Bool Create(const char *fName);
  Bool AddFile(const char *fName, const char *sDir = 0);
  Bool AddDir(const char *dir, const char *mask, Bool subs = TRUE);

  // Functions for opening an existing pack file
  Bool Open(const char *fName);
  void ListContents();
  Bool ExtractFile(const char *fName, const char *destDir);
  void ExtractAll(const char *destDir);
  
  // Closes both new and existing pack files
  void Close();
};

#endif