///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Pack file management code
//
// 08-JAN-1998
//

#include <string.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include "std.h"
#include "packtool.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class PackTool - management of pack files
//


//
// PackTool::workBuf
//
// used as a buffer for reading and writing data
//
U8 PackTool::workBuf[PACKTOOL_WORKBUFSIZE];


// 
// PackTool::Display
//
// Display a string using supplied routine
//
void CDECL PackTool::Display(const char *format, ...)
{
  // do we have a custom routine 
  if (customDisplay)
  {
    va_list args;
    char fmtBuf[1024];
    va_start(args, format);
    vsprintf(fmtBuf, format, args);
    va_end(args);
    customDisplay(fmtBuf);
  }
}


//
// PackTool::NameFromPath
//
// returns a pointer to the file name part of a path
//
const char* PackTool::NameFromPath(const char *path)
{
  const char *ptr = strrchr(path, '\\');
  return (ptr ? ptr + 1 : path);
}


//
// PackTool::WriteData
//
// write data to the pack file, and update 'writePos'
//
void PackTool::WriteData(const void *data, U32 size)
{
  ASSERT(mode == WRITING);

  // write the data
  if (fwrite(data, 1, size, packFile) != size)
  {
    Display("Failed writing %d bytes to pack file", size);
    throw ("Unable to continue");
  }

  // update current write position
  writePos += size;
}


//
// PackTool::WritePackedString
//
// writes a packed string to the pack file
//
void PackTool::WritePackedString(const char *str)
{
  U32 checkLen = strlen(str);

  if (checkLen > U8_MAX)
  {
    Display("String '%s' exceeds maximum length");
    throw ("Unable to continue");
  }

  U8 len = (U8) checkLen;
  WriteData(&len, sizeof(U8));
  WriteData(str, len);
}


//
// PackTool::WritePackInfo
//
// write out the pack info header
//
void PackTool::WritePackInfo()
{
  ASSERT(mode == WRITING);

  // go to start of file
  fseek(packFile, 0, SEEK_SET);
  
  // write out pack file info
  WriteData(&packInfo, sizeof(packInfo));
}


//
// PackTool::WriteFileInfo
//
// write out info for each known file
//
void PackTool::WriteFileInfo()
{
  ASSERT(mode == WRITING);

  // save start location
  packInfo.fileOffset = writePos;

  // write each item
  for (List<FileInfo>::Iterator i(&fileInfo); *i; i++)
  {
    // must conform to file format
    WriteData(&(*i)->dirIndex, sizeof(U32));
    WritePackedString((*i)->fName);
    WriteData(&(*i)->offset, sizeof(U32));
    WriteData(&(*i)->size, sizeof(U32));
  }
}


//
// PackTool::WriteDirInfo
//
// write out info for each known directory
//
void PackTool::WriteDirInfo()
{
  List<DirInfo>::Iterator i(&dirInfo);

  // save write position
  packInfo.dirOffset = writePos;

  // write each dir
  for (!i; *i; i++)
  {
    WritePackedString((*i)->dir);
  }
}


//
// PackTool::GetSubDirIndex
//
// returns the index for 'sDir'
//
U32 PackTool::GetSubDirIndex(const char *sDir)
{
  ASSERT(mode == WRITING);
  ASSERT(sDir);

  // see if already registered
  for (List<DirInfo>::Iterator i(&dirInfo); *i; i++)
  {
    if (!strcmp((*i)->dir, sDir))
    {
      return (i.GetPos() + 1);
    }
  }
  
  // check length of directory
  if (strlen(sDir) > U32_MAX)
  {
    Display("Sub-directory '%s' exceeds max length (%d)", sDir, U32_MAX);
    throw ("Unable to continue");
  }

  // check overflow
  if (packInfo.dirCount == U32_MAX)
  {
    Display("Too many sub-dirs (max %d)", U32_MAX);
    throw ("Unable to continue");
  }

  // increment count
  packInfo.dirCount++;

  // new entry
  DirInfo *info = new DirInfo;

  // copy directory
  strcpy(info->dir, sDir);

  // add to list
  dirInfo.Append(info);

  // return index
  return (packInfo.dirCount);
}


//
// PackTool::ProcessItem
//
// processes a search item
//
// info - the file structure returned from the _find functions
// rDir - root directory being processed (eg. 'c:\temp')
// sDir - sub directory from root (eg. 'subdir' or 'subdir1\subdir2') or null
// mask - file mask to add (eg. '*.jpg')
// subs - should we add sub dirs
//
void PackTool::ProcessItem
(
  _finddata_t *info,
  const char *rDir,
  const char *sDir,
  const char *mask, 
  Bool subs
)
{
  ASSERT(info);
  ASSERT(rDir);
  ASSERT(mask);

  char buf[_MAX_PATH];
  
  // filter out current and previous dirs
  if (*info->name != '.')
  {
    // check item type
    if (info->attrib & _A_SUBDIR)
    {
      // do we want sub-dirs
      if (subs)
      {
        // add on this sub-dir
        sprintf(buf, "%s%s%s", sDir?sDir:"", sDir?"\\":"", info->name);

        // add a directory
        ProcessDir(rDir, buf, mask, TRUE);
      }
    }
    else
    {
      // make full name
      sprintf(buf, "%s\\%s%s%s", rDir, sDir?sDir:"", sDir?"\\":"", info->name);

      // add a file
      AddFile(buf, sDir);
    }
  }
}


//
// PackTool::ProcessDir
//
// processes a directory
//
// rDir - root directory being processed (eg. 'c:\temp')
// sDir - sub directory from root (eg. 'subdir' or 'subdir1\subdir2') or null
// mask - file mask to add (eg. '*.jpg')
// subs - should we add sub dirs
//
void PackTool::ProcessDir(const char *rDir, const char *sDir, const char *mask, Bool subs)
{
  ASSERT(rDir);
  ASSERT(mask);

  char dirMask[_MAX_PATH];
  _finddata_t info;
  long handle;

  // add directory and mask
  sprintf(dirMask, "%s\\%s%s%s", rDir, sDir?sDir:"", sDir?"\\":"", mask);

  // find first file
  if ((handle = _findfirst(dirMask, &info)) != -1L)
  {
    do
    {
      ProcessItem(&info, rDir, sDir, mask, subs);
    }
    while (_findnext(handle, &info) == 0);

    _findclose(handle);
  }
}


//
// PackTool::ReadData
//
// read data from the current pack file
//
void PackTool::ReadData(void *dest, U32 size)
{
  ASSERT(mode == READING);

  if (fread(dest, 1, size, packFile) != size)
  {
    Display("Failed reading %d bytes from pack file", size);
    throw ("Unable to continue");
  }
}


//
// PackTool::ReadPackedString
//
// reads a packed string from the pack file
//
void PackTool::ReadPackedString(PackStr dest)
{
  U8 len;
  ReadData(&len, sizeof(U8));
  ReadData(dest, len);
  dest[len] = '\0';
}


//
// PackTool::ReadFileInfo
//
// read info for each known file
//
void PackTool::ReadFileInfo()
{
  ASSERT(mode == READING);
  
  // ensure list is clear
  fileInfo.DisposeAll();
  
  // go to start of file info block
  fseek(packFile, packInfo.fileOffset, SEEK_SET);

  // read each item
  for (U32 i =0; i < packInfo.fileCount; i++)
  {
    FileInfo *info = new FileInfo;
    ReadData(&info->dirIndex, sizeof(U32));
    ReadPackedString(info->fName);
    ReadData(&info->offset, sizeof(U32));
    ReadData(&info->size, sizeof(U32));
    fileInfo.Append(info);
  }

  ASSERT(packInfo.fileCount == fileInfo.GetCount());
}


//
// PackTool::ReadDirInfo
//
// read info for each known directory
//
void PackTool::ReadDirInfo()
{
  ASSERT(mode == READING);

  // ensure list is clear
  dirInfo.DisposeAll();
  
  // go to start of dir info block
  fseek(packFile, packInfo.dirOffset, SEEK_SET);

  // read each item
  for (U32 i =0; i < packInfo.dirCount; i++)
  {
    DirInfo *info = new DirInfo;
    ReadPackedString(info->dir);
    dirInfo.Append(info);
  }

  ASSERT(packInfo.dirCount == dirInfo.GetCount());
}


//
// PackTool::GetFileInfo
//
// returns the info for 'fName', or null if not found
//
PackTool::FileInfo* PackTool::GetFileInfo(const char *fName)
{
  ASSERT(mode == READING);

  List<FileInfo>::Iterator i(&fileInfo);

  // check each item
  for (!i; *i; i++)
  {
    // is this the file we're after
    if (!strcmp(fName, (*i)->fName))
    {
      return (*i);
    }
  }

  // didn't find it
  return (0);
}


//
// PackTool::GetDirInfo
//
// returns the info for 'index' (must be in range)
//
PackTool::DirInfo* PackTool::GetDirInfo(U32 index)
{
  ASSERT(mode == READING);
  ASSERT(index > 0);
  ASSERT(index <= packInfo.dirCount);

  // seek to correct position
  List<DirInfo>::Iterator i(&dirInfo);

  i.GoTo(index - 1);
  
  ASSERT(*i);

  // return the info
  return (*i);
}


//
// PackTool::CreateFullDir
//
// creates each directory within 'path'
//
void PackTool::CreateFullDir(const char *path)
{
  char buf[_MAX_PATH];
  const char *ptr, *walk = path;

  do 
  {
    // have we found a '\' character
    if ((ptr = strchr(walk, '\\')) != 0)
    {
      // copy portion infront of that char
      const char *s = path;
      for (char *d = buf; s != ptr; s++, d++) 
      { 
        *d = *s; 
      }
      *d = '\0';
      walk = ptr + 1;
    }
    else
    {
      // take entire directory
      strcpy(buf, path);
    }

    // does it exist already
    if (_access(buf, 0) == -1)
    {
      // create it
      if (_mkdir(buf))
      {
        Display("Unable to create directory '%s'", buf);
        throw ("Unable to continue"); 
      }
    }
  }
  while (ptr);
}


//
// PackTool::ExtractFile  
//
// extract 'info' to 'dir'
//
Bool PackTool::ExtractFile(FileInfo *info, const char *destDir)
{ 
  ASSERT(mode == READING);
  ASSERT(info);
  ASSERT(destDir);

  U32 block, remain;
  FILE *destFile;
  char fullDest[_MAX_PATH];

  // create full destination path
  strcpy(fullDest, destDir);
  
  // is this file within a sub-dir
  if (info->dirIndex > 0)
  {
    DirInfo *dirInfo = GetDirInfo(info->dirIndex);
    strcat(fullDest, "\\");
    strcat(fullDest, dirInfo->dir);
  }

  // make sure destination directory exists
  if (_access(fullDest, 0) == -1)
  {
    // create it
    CreateFullDir(fullDest);
  }

  // add on the file name
  strcat(fullDest, "\\");
  strcat(fullDest, info->fName);

  // open destination file
  if ((destFile = fopen(fullDest, "wb")) == 0)
  {
    Display("Unable to open '%s' for writing", fullDest);
    return (FALSE);
  }

  // seek to start of entry
  fseek(packFile, info->offset, SEEK_SET);

  // initial bytes remaining
  remain = info->size;

  while (remain)
  {
    // how much should we read
    block = (remain > sizeof(workBuf)) ? sizeof(workBuf) : remain;

    // read it
    ReadData(workBuf, block);
    
    // write it to the dest file
    if (fwrite(workBuf, 1, block, destFile) != block)
    {
      Display("Failed writing %d bytes to '%s'", fullDest);
      throw ("Unable to continue");
    }

    // reduce remaining bytes
    remain -= block;
  }

  // close the dest file
  fclose(destFile);

  //Display("  Extracted '%s'", fullDest);

  return (TRUE);  
}


//
// PackTool::PackTool
//
// constructor
//
PackTool::PackTool(DisplayCallBack *display)
{
  mode = CLOSED;
  customDisplay = display;
}


//
// PackTool::~PackTool
//
// destructor
//
PackTool::~PackTool()
{
  // close any currently open file
  Close();
}


//
// PackTool::Create
//
// create a new pack file
//
Bool PackTool::Create(const char *fName)
{
  // close any currently open file
  Close();  

  ASSERT(mode == CLOSED);
    
  // create file
  if ((packFile = fopen(fName, "wb")) == 0)
  {
    return (FALSE);
  }

  Display("Creating : %s", fName);

  // Ensure header is cleared completely
  memset(&packInfo, 0, sizeof(packInfo));

  // set default info
  packInfo.packId = PACK_ID;
  packInfo.verId = PACK_VER;
  packInfo.dirCount = 0;
  packInfo.fileCount = 0;

  // reset write position
  writePos = 0;

  // copy name
  packPath = strdup(fName);

  // get file name portion
  packName = NameFromPath(fName);
  packNameCrc = Crc::CalcStr(packName);

  // set new mode
  mode = WRITING;

  // write the header (will be overwritten at the end)
  WritePackInfo();

  // success
  return (TRUE);  
}


//
// PackTool::AddFile
//
// add a file to current pack file
//
Bool PackTool::AddFile(const char *fName, const char *sDir)
{
  ASSERT(mode == WRITING);
  ASSERT(fName);

  U32 readSize = 0, readCount;
  FILE *readFile;
  const char *name;
  U32 dirIndex = 0;

  // do we have a sub-dir
  if (sDir)
  {
    dirIndex = GetSubDirIndex(sDir);
  }
  
  // check length of file name
  if (strlen(fName) > U8_MAX)
  {
    Display("File name '%s' exceeds max length (%d)", fName, U8_MAX);
    throw ("Unable to continue");
  }

  // get file name
  name = NameFromPath(fName);

  // get crc of the file name
  U32 crc = Crc::CalcStr(name);

  // not allowed to add destination file
  if (crc == packNameCrc)
  {
    return (FALSE);
  }
  
  // ignore stupid source safe file "vssver.scc" (HACK!) 
  if (crc == 0xA396920A)
  {
    return (FALSE);
  }
  
  // spew at duplicate file names
  if (crcTree.Exists(crc))
  {
    Display("Dup: %s\\%s", sDir ? sDir : ".", name);
    return (FALSE);
  }

  // open source file
  if ((readFile = fopen(fName, "rb")) == 0)
  {
    Display("Unable to open: %s\\%s", sDir ? sDir : ".", name);
    return (FALSE);
  }

  // create new entry
  FileInfo *pEntry = new FileInfo;

  // setup entry info
  pEntry->dirIndex = dirIndex;
  strcpy(pEntry->fName, name);
  strlwr(pEntry->fName);
  pEntry->offset = writePos;

  // add to list
  fileInfo.Append(pEntry);

  // add to crc search tree
  crcTree.Add(Crc::CalcStr(pEntry->fName));

  // increase count
  packInfo.fileCount++;

  // copy source file into pack file
  do 
  {
    if ((readCount = fread(workBuf, 1, sizeof(workBuf), readFile)) > 0)
    {
      WriteData(workBuf, readCount);
      readSize += readCount;
    }
  } 
  while (readCount);

  // close the source file
  fclose(readFile);

  // set size of entry
  pEntry->size = readSize;

  // success
  return (TRUE);  
}

 
//
// PackTool::AddDir
//
// add a directory to the current pack file
//
// dir  - directory to add (eg. 'c:\temp')
// mask - file mask to add (eg. '*.jpg')
// subs - should we add sub dirs
//
Bool PackTool::AddDir(const char *dir, const char *mask, Bool subs)
{
  ASSERT(dir);
  ASSERT(mask);

  // call private function
  ProcessDir(dir, 0, mask, subs);  

  return (TRUE);
}


//
// PackTool::Open
//
// open a pack file for reading
//
Bool PackTool::Open(const char *fName)
{
  // close any currently open file
  Close();  

  ASSERT(mode == CLOSED);

  // open pack file
  if ((packFile = fopen(fName, "rb")) == 0)
  {
    return (FALSE);
  }

  // copy name
  packPath = strdup(fName);

  // get file name portion
  packName = NameFromPath(fName);
  packNameCrc = Crc::CalcStr(packName);

  // enter reading mode
  mode = READING;

  // read pack info header
  ReadData(&packInfo, sizeof(packInfo));

  // check format
  if (packInfo.packId != PACK_ID)
  {
    Display("'%s' is not a valid pack file (id = 0x%08x)", fName);
    throw ("Unable to continue");
  }

  // check version
  switch (packInfo.verId)
  {
    // all supported versions
    case PACK_VER001:
      break;

    // unknown version
    default :
      Display("'%s' is not a supported version (0x%08x)", fName, packInfo.verId);
      throw ("Unable to continue");
  }

  // read file info
  ReadFileInfo();

  // read directory info
  ReadDirInfo();

  Display("Opened : %s [%d files]", fName, packInfo.fileCount);

  // success
  return (TRUE);  
}


//
// PackTool::ListContents
//
// List contents of pack file
//
void PackTool::ListContents()
{
  ASSERT(mode == READING);

  for (List<FileInfo>::Iterator i(&fileInfo); *i; ++i)
  {
    FileInfo &info = **i;

    if (info.dirIndex)
    {
      Display("%s\\%s", GetDirInfo(info.dirIndex)->dir, info.fName);
    }
    else
    {
      Display("%s", info.fName);
    }
  }
}


//
// PackTool::ExtractFile
//
// extract 'fName' to 'destDir'
//
Bool PackTool::ExtractFile(const char *fName, const char *destDir)
{
  ASSERT(mode == READING);
  ASSERT(fName);
  ASSERT(destDir);

  FileInfo *info = GetFileInfo(fName);

  if (info)
  {
    ExtractFile(info, destDir);
    return (TRUE);
  }

  return (FALSE);
}


//
// PackTool::ExtractAll
//
// extract all files to 'dir'
//
void PackTool::ExtractAll(const char *destDir)
{
  ASSERT(mode == READING);
  ASSERT(destDir);

  for (List<FileInfo>::Iterator i(&fileInfo); *i; ++i)
  {
    if (!ExtractFile(*i, destDir))
    {
      Display("Unable to extract '%s' to '%s'", (*i)->fName, destDir);
    }
  }
}


//
// PackTool::Close
//
// close an open pack file
//
void PackTool::Close()
{
  switch (mode)
  {
    // already closed
    case CLOSED:
      break;

    // write pack file information
    case WRITING:
 
      // write out each entry's info
      WriteFileInfo();

      // write out each registered directory
      WriteDirInfo();

      // return to start and write header
      WritePackInfo();

      // close file
      fclose(packFile);
      packFile = 0;   

      // free name
      free(packPath);

      break;

    // reading an existing pack file
    case READING:

      // close file
      fclose(packFile);
      packFile = 0;   

      // free name
      free(packPath);

      break;

    // should never happen
    default :
      throw ("Missing case");
  }

  // always clear lists
  fileInfo.DisposeAll();
  dirInfo.DisposeAll();
  crcTree.DisposeAll();

  // reset mode
  mode = CLOSED;
}