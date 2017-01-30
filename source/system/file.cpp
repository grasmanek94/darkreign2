///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File Class
// 1-DEC-1997
//

//
// Notes:
//
// There are two distinct interfaces to the majority of these functions
// Most functions can take either DIR\FILE combinations or just PATH
// Hence the functions which implement DIR\FILE will make use of the PATH methods in all cases
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
// 
#include "file.h"
#include "filemap.h"

#pragma warning(push)
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <direct.h>
#include <shellapi.h>
#pragma warning(pop)


// Temp for debugging
#include "filesys.h"


//#define LOG_FILE(x) LOG_DIAG(x)
#define LOG_FILE(x)



///////////////////////////////////////////////////////////////////////////////
//
// File Implementations
//

// Internal data
static Bool alignmentSetup = FALSE;
static U32 virtualMemAlignShift = 0;


//
// Align an offset to the operating system's required alignment
//
static U32 AlignMapPointer(U32 i)
{
  if (!alignmentSetup)
  {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    // Calculate bit shift amount
    for (U32 n = sysInfo.dwAllocationGranularity; n >>= 1;)
    {
      virtualMemAlignShift++;
    }
    alignmentSetup = TRUE;
  }

  // Truncate down to the specified alignment
  i >>= virtualMemAlignShift;
  i <<= virtualMemAlignShift;

  return (i);
}


//
// File: (constructor)
//
File::File()
: handle(NULL),
  mapHandle(NULL),
  mapCount(0)
{
}


//
// ~File: (destructor)
//
File::~File()
{
  // Close the file
  Close();
}


//
// Open: (method)
//
Bool File::Open(const char *dir, const char *fname, U32 _mode)
{
  FilePath  path;

  Dir::PathMake(path, dir, fname);
  return (Open(path.str, _mode));
}


//
// Open: (method)
//
Bool File::Open(const char *path, U32 _mode)
{
  ASSERT(!IsOpen())

  U32 accessMode = 0;
  U32 createMode = 0;
  U32 shareMode = FILE_SHARE_READ;

  #ifdef CHECKMAPPINGS

    fileName = path;

  #endif

  // Save mode
  mode = _mode;

  // READ / WRITE ACCESS ?
  if ((mode & READ) && (mode & WRITE))
  {
    accessMode = GENERIC_READ | GENERIC_WRITE;
  }
  else

  if (mode & READ)
  {
    accessMode = GENERIC_READ;
  }
  else

  if (mode & WRITE)
  {
    accessMode = GENERIC_WRITE;
  }

  // APPEND ?
  if (mode & APPEND)
  {
    accessMode = GENERIC_READ | GENERIC_WRITE;
  }

  // Create mode
  if (mode & CREATE)
  {
    createMode = CREATE_ALWAYS;
  }
  else 
    
  if (mode & APPEND)
  {
    createMode = OPEN_ALWAYS;
  }
  else

  {
    createMode = OPEN_EXISTING;
  }

  // Open the file
  handle = CreateFile(path, accessMode, shareMode, NULL, createMode, 0, NULL);

  // Was the file created successfully ?
  if (handle == INVALID_HANDLE_VALUE)
  {
    handle = NULL;
    LOG_ERR(("Error occured opening %s", path))
    return (FALSE);
  }

  return (TRUE);
}


//
// Close: (method)
//
void File::Close()
{
  if (IsOpen())
  {
    if (mapCount)
    {
      FileSys::DataFile::LogOpen();

      #ifdef CHECKMAPPINGS

        LOG_ERR(("Open Mappings for %s : %d", fileName.str, openMappings.GetCount()))

        for (BinTree<FileName>::Iterator i(&openMappings); *i; i++)
        {
          LOG_ERR((" - [%s]", (*i)->str))
        }
        openMappings.DisposeAll();

        ERR_FATAL(("File [%s] closed with open mappings [%d]", fileName.str, mapCount));

      #else

        ERR_FATAL(("File closed with open mappings [%d]", mapCount));

      #endif
    }

    // Close file mapping
    if (mapHandle)
    {
      CloseHandle(mapHandle);
      mapHandle = NULL;
    }

    // Close the file
    CloseHandle(handle);
  }

  handle = NULL;
}


//
// IsOpen: (method)
//
Bool File::IsOpen()
{
  return ((handle == NULL) ? FALSE : TRUE);
}


//
// File::IsMapped
//
Bool File::IsMapped()
{
  return ((mapHandle == NULL) ? FALSE : TRUE);
}


//
// Read: (method)
//
// Returns the number of bytes read
// Note that this is not the number of objects if size > 1
//
U32 File::Read(void *buffer, U32 count, U32 size)
{
  ASSERT(IsOpen())
  //ASSERT(!IsMapped())

  U32 total = count * size;
  U32 actual;

  if (!ReadFile(handle, buffer, total, &actual, NULL))
  {
    if (!(mode & NOERR) && (actual != total))
    {
      ReadError(total);
    }
  }

  return (actual);
}


//
// WriteU8: (method)
//
Bool File::WriteU8(U8 data)
{
  if (Write(&data, sizeof (U8)) == sizeof (U8))
  {
    return TRUE;
  }

  if (!(mode & NOERR))
  {
    WriteError(sizeof(data));
  }

  return FALSE;
}


//
// WriteU16: (method)
// 
Bool File::WriteU16(U16 data)
{
  if (Write(&data, sizeof (U16)) == sizeof (U16))
  {
    return TRUE;
  }

  if (!(mode & NOERR))
  {
    WriteError(sizeof(data));
  }

  return FALSE;
}


//
// WriteU32: (method)
//
Bool File::WriteU32(U32 data)
{
  if (Write(&data, sizeof (U32)) == sizeof (U32))
  {
    return TRUE;
  }
  
  if (!(mode & NOERR))
  {
    WriteError(sizeof(data));
  }

  return FALSE;
}


//
// Write a string
//
Bool CDECL File::WriteString(const char *format, ...)
{
  char buff[512];
  va_list args;
  va_start(args, format);
  vsprintf(buff, format, args);
  va_end(args);

  U32 length = Utils::Strlen(buff);

  if (length)
  {
    if (Write(buff, length) == length)
    {
      return (TRUE);
    }

    if (!(mode & NOERR))
    {
      WriteError(length);
    }
  }

  return (FALSE);
}


//
// Write: (method)
//
U32 File::Write(const void *src, U32 count, U32 size)
{
  ASSERT(IsOpen())
  ASSERT(!IsMapped())

  U32 total = count * size;
  U32 actual;

  if (mode & APPEND)
  {
    Seek(File::Seek::END, 0);
  }

  if (!WriteFile(handle, src, total, &actual, NULL))
  {
    if (!(mode & NOERR) && (actual != total))
    {
      WriteError(total);
    }
  }

  if (mode & FLUSH)
  {
    Flush();
  }

  return (actual);
}


//
// Seek: (method)
//
Bool File::Seek(U32 pos, S32 offset)
{
  ASSERT(IsOpen())
  ASSERT(pos == SET || pos == CUR || pos == END)

  if (SetFilePointer(handle, offset, NULL, pos) == 0xFFFFFFFF)
  {
    return (FALSE);
  }

  return (TRUE);
}


//
// Tell: (method)
//
U32 File::Tell()
{
  ASSERT(IsOpen())

  return (SetFilePointer(handle, 0, NULL, FILE_CURRENT));
}


//
// Flush: (method)
//
Bool File::Flush()
{
  ASSERT(IsOpen())
  ASSERT(!IsMapped())

  if (!FlushFileBuffers(handle))
  {
    return (FALSE);
  }

  return (TRUE);
}


//
// GetSize: (method)
//
U32 File::GetSize()
{
  ASSERT(IsOpen())

  return (GetFileSize(handle, NULL));
}


//
// File::MapMemory
//
// Map all or part of file into memory, return a pointer to the memory
//
void *File::MapMemory(const char *mapFileName, U32 flags, U32 offset, U32 length, void **realPointer)
{
  ASSERT(IsOpen())

  // Create a file mapping if it does not exist yet
  if (mapHandle == NULL)
  {
    ASSERT(mapCount == 0)

    // Create the file mapping
    U32 fileMapFlag = (flags & MAP_WRITE) ? PAGE_READWRITE : PAGE_READONLY;
    U32 size = GetSize();

    mapHandle = CreateFileMapping(handle, NULL, fileMapFlag, 0, size, NULL);

    // Could the file be mapped
    if (mapHandle == NULL)
    {
      #ifdef CHECKMAPPINGS

        ERR_FATAL(("Could not create file mapping [%s] : %s", fileName.str, Debug::LastError()))

      #else

        ERR_FATAL(("Could not create file mapping : %s", Debug::LastError()))

      #endif
    }

    LOG_FILE(("CreateFileMapping: 0x%.8X %d %d", handle, fileMapFlag, size))
  }

  // Map the specified part of this file
  U32 mapViewFlag = (flags & MAP_WRITE) ? FILE_MAP_WRITE : FILE_MAP_READ;

  // Need to align the pointer to the system's aligning boundary
  U32 newOfs = AlignMapPointer(offset);
  U32 change = offset - newOfs;
  U32 newLen = length + change;

  ASSERT(newOfs <= offset)

  U8 *ptr = (U8 *)MapViewOfFile(mapHandle, mapViewFlag, 0, newOfs, newLen);

  // Could we get a view of the mapping ?
  if (ptr == NULL)
  {
    #ifdef CHECKMAPPINGS

      ERR_FATAL(("Could not create a view of the mapping [%s] : %s [pos=%d(%d),size=%d(%d)]", fileName.str, Debug::LastError(), offset, newOfs, length, newLen))

    #else

      ERR_MESSAGE(("The system requires more memory or swap file space.  Check to see that your hard drive is not full, or adjust your virtual memory settings.\n\nCould not create a view of a file mapping : %s", Debug::LastError()));
      
      //ERR_FATAL(("Could not create a view of the mapping : %s [pos=%d(%d),size=%d(%d)]", Debug::LastError(), offset, newOfs, length, newLen))

    #endif
  }

  // Increment mapping reference count
  mapCount++;

  #ifdef CHECKMAPPINGS

    FilePath *prevMap;
    
    if ((prevMap = openMappings.Find(U32(ptr))) != NULL)
    {
      ERR_FATAL(("%s: mapping address [0x%.8X] for %s already in use by [%s]", fileName.str, ptr, mapFileName, prevMap->str))
    }

    openMappings.Add(U32(ptr), new FileName(mapFileName));

  #else

    mapFileName;

  #endif

  LOG_FILE(("MapViewOfFile: 0x%.8X [ref=%d] %d [pos=%d(%d),size=%d(%d)]", handle, mapCount, mapViewFlag, offset, newOfs, length, newLen))

  // Setup actual pointer
  if (realPointer)
  {
    *realPointer = ptr;
  }

  // Return pointer to start of data
  return (ptr + change);
}


//
// File::UnmapMemory
//
// Close a previously opened memory map
//
void File::UnmapMemory(void *data)
{
  ASSERT(IsOpen())
  ASSERT(data)
  ASSERT(mapCount > 0)

  // Unmap just this view of the file
  if (!UnmapViewOfFile(data))
  {
    #ifdef CHECKMAPPINGS

      ERR_FATAL(("Could not unmap view of file [%s] : %s [data=0x%.8X]", fileName.str, Debug::LastError(), data))

    #else

      ERR_FATAL(("Could not unmap view of file : %s [data=0x%.8X]", Debug::LastError(), data))

    #endif
  }

  // Decrement mapping reference count;
  mapCount--;

  #ifdef CHECKMAPPINGS

    if (!openMappings.Find(U32(data)))
    {
      ERR_FATAL(("%s: mapping address 0x%.8X not found", fileName.str, data))
    }

    openMappings.Dispose(U32(data));

  #endif

  LOG_FILE(("UnmapMemory: 0x%.8X [ref=%d]", handle, mapCount))
}


//
// Unique: (method)
//
Bool File::Unique(const char *tplate, FileName &fname)
{
  char temp[FILENAME_MAX];
  
  Utils::Strmcpy(temp, tplate, FILENAME_MAX);
  if (_mktemp(temp) == NULL)
  {
    return (FALSE);
  }
  else
  {
    fname = temp;
    return (TRUE);
  }
}


//
// Exists: (method)
//
Bool File::Exists(const char *dir, const char *fname)
{
  FilePath  path;

  Dir::PathMake(path, dir, fname);
  return (Exists(path.str));
}


//
// Exists: (method)
//
Bool File::Exists(const char *path)
{
  return (_access(path, 0) == -1 ? FALSE : TRUE);
}


//
// Unlink: (method)
//
Bool File::Unlink(const char *dir, const char *fname)
{
  FilePath  path;

  Dir::PathMake(path, dir, fname);
  return (Unlink(path.str));
}


//
// Unlink: (method)
//
Bool File::Unlink(const char *path)
{
  return (_unlink(path) == -1 ? FALSE : TRUE);
}


//
// Rename: (method)
//
Bool File::Rename(const char *dir, const char *fname, const char *newname)
{
  FilePath path;
  FilePath newpath;

  Dir::PathMake(path, dir, fname);
  Dir::PathMake(newpath, dir, newname);
  return (Rename(path.str, newpath.str));
}


//
// Rename: (method)
//
Bool File::Rename(const char *path, const char *newpath)
{
  return (rename(path, newpath) == -1 ? FALSE : TRUE);
}


//
// Copy: (method)
//
Bool File::Copy(const char *src, const char *dst, Bool overwrite, Bool progress)
{
  // Setup the operation data
  SHFILEOPSTRUCT data;
  data.hwnd = NULL;
  data.wFunc = FO_COPY;
  data.pFrom = src;
  data.pTo = dst;
  data.fFlags = 
    FILEOP_FLAGS
    (
      (overwrite ? 0 : FOF_NOCONFIRMATION) | 
      FOF_NOERRORUI | 
      FOF_SILENT | 
      FOF_RENAMEONCOLLISION | 
      (progress ? FOF_SIMPLEPROGRESS : FOF_SILENT)
    );
  data.fAnyOperationsAborted = FALSE;
  data.hNameMappings = NULL;
  data.lpszProgressTitle = NULL;

  return (!SHFileOperation(&data));
}


//
// Remove: (method)
//
Bool File::Remove(const char *path)
{
  // Get length of the path
  U32 len = Utils::Strlen(path);

  // Ensure the buffer is big enough
  if (len < PATHNAME_MAX - 1)
  {
    // Copy to the buffer
    char buf[PATHNAME_MAX];
    Utils::Memcpy(buf, path, len + 1);

    // Add second null termination
    buf[len + 1] = '\0';

    // Setup the operation data
    SHFILEOPSTRUCT data;
    data.hwnd = NULL;
    data.wFunc = FO_DELETE;
    data.pFrom = buf;
    data.pTo = NULL;
    data.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    data.fAnyOperationsAborted = FALSE;
    data.hNameMappings = NULL;
    data.lpszProgressTitle = NULL;

    return (!SHFileOperation(&data));
  }

  return (FALSE);
}


//
// GetAttrib: (method)
//
Bool File::GetAttrib(const char *dir, const char *fname, U32 &attrib)
{
  FilePath  path;

  Dir::PathMake(path, dir, fname);
  return (GetAttrib(path.str, attrib));
}


//
// GetAttrib: (method)
//
Bool File::GetAttrib(const char *path, U32 &attrib)
{
  U32 att;
  
  att = GetFileAttributes(path);

  if (att == (U32) -1)
  {
    return (FALSE);
  }

  attrib = NORMAL;

  if (att & FILE_ATTRIBUTE_ARCHIVE)
  {
    attrib |= ARCHIVE;
  }
    
  if (att & FILE_ATTRIBUTE_COMPRESSED)
  {
    // Ignored at this time
  }
  
  if (att & FILE_ATTRIBUTE_DIRECTORY)
  {
    attrib |= SUBDIR;
  }
  
  if (att & FILE_ATTRIBUTE_HIDDEN)
  {
    attrib |= HIDDEN;
  }
  
  if (att & FILE_ATTRIBUTE_NORMAL)
  {
    attrib |= NORMAL;
  }

  if (att & FILE_ATTRIBUTE_OFFLINE)
  {
    // Ignored at this time
  }

  if (att & FILE_ATTRIBUTE_READONLY)
  {
    attrib |= READONLY;
  }

  if (att & FILE_ATTRIBUTE_SYSTEM)
  {
    attrib |= SYSTEM;
  }

  if (att & FILE_ATTRIBUTE_TEMPORARY)
  {
    // Ignored at this time
  }

  return (TRUE);
}


//
// SetAttrib: (method)
//
Bool File::SetAttrib(const char *dir, const char *fname, U32 attrib)
{
  FilePath path;

  Dir::PathMake(path, dir, fname);
  return (SetAttrib(path.str, attrib));
}


//
// SetAttrib: (method)
//
Bool File::SetAttrib(const char *path, U32 attrib)
{
  U32 att;

  att = 0;

  if (attrib & NORMAL)
  {
    att |= FILE_ATTRIBUTE_NORMAL;
  }

  if (attrib & READONLY)
  {
    att |= FILE_ATTRIBUTE_READONLY;
  }

  if (attrib & HIDDEN)
  {
    att |= FILE_ATTRIBUTE_HIDDEN;
  }

  if (attrib & SYSTEM)
  {
    att |= FILE_ATTRIBUTE_SYSTEM;
  }

  if (attrib & SUBDIR)
  {
    att |= FILE_ATTRIBUTE_DIRECTORY;
  }

  if (attrib & ARCHIVE)
  {
    att |= FILE_ATTRIBUTE_ARCHIVE;
  }

  return (SetFileAttributes(path, att) ? TRUE : FALSE);
}


//
// Crc: (method)
//
U32 File::Crc(const char *dir, const char *fname, U32 crc)
{
  FilePath  path;

  Dir::PathMake(path, dir, fname);
  return (Crc(path.str), crc);
}


//
// Crc: (method)
//
U32 File::Crc(const char *path, U32 crc)
{
  FileMap fileMap(path, FileMap::READ);
  crc = Crc::Calc(fileMap.GetPtr(), fileMap.GetSize(), crc);
  return (crc);
}


//
// File::ReadError
//
void File::ReadError(U32 bytes)
{
  #ifdef CHECKMAPPINGS

    ERR_FATAL(("Error reading %u bytes from %s", bytes, fileName.str));

  #else

    ERR_FATAL(("Error reading %u bytes", bytes));

  #endif
}


//
// File::WriteError
//
void File::WriteError(U32 bytes)
{
  #ifdef CHECKMAPPINGS

    ERR_FATAL(("Error writing %u bytes from %s", bytes, fileName.str));

  #else

    ERR_FATAL(("Error writing %u bytes", bytes));

  #endif
}


///////////////////////////////////////////////////////////////////////////////
//
// Dir Implementations
//

//
// PathAbsolute: (method)
//
Bool Dir::PathAbsolute(FilePath &absolute, const char *relative)
{
  char temp_abs[PATHNAME_MAX];
  char *p = _fullpath(temp_abs, relative, PATHNAME_MAX);
  absolute = temp_abs;
  return ((p == NULL) ? FALSE : TRUE);
}


//
// PathExpand: (method)
//
void Dir::PathExpand(const char *path, FileDrive &drive, FileDir &dir, FileName &fname, FileExt &ext)
{
  char temp_drive[DRIVENAME_MAX];
  char temp_dir[DIRNAME_MAX];
  char temp_fname[FILENAME_MAX];
  char temp_ext[EXTNAME_MAX];

  _splitpath(path, temp_drive, temp_dir, temp_fname, temp_ext);
  drive = temp_drive;
  dir   = temp_dir;
  fname = temp_fname;
  ext   = temp_ext;
}


//
// PathMake: (method)
//
void Dir::PathMake(FilePath &path, const char *drive, const char *dir, const char *fname, const char *ext)
{
  char temp_path[PATHNAME_MAX];

  _makepath(temp_path, drive, dir, fname, ext);
  path = temp_path;
}


//
// PathMake: (method)
//
void Dir::PathMake(FilePath &path, const char *dir, const char *fname)
{
  char temp_path[PATHNAME_MAX];

  _makepath(temp_path, NULL, dir, fname, NULL);
  path = temp_path;
}


//
// GetCurrent: (method)
//
// Always return a pathname without trailing slash, so C:\ becomes C:
//
Bool Dir::GetCurrent(const int drive, FileDir &dir)
{
  char temp_dir[DIRNAME_MAX] = "";
  char *r = _getdcwd(drive, temp_dir, DIRNAME_MAX);

  // Trim trailing slash
  U32 len = Utils::Strlen(temp_dir);
  if (temp_dir[len-1] == '/' || temp_dir[len-1] == '\\')
  {
    temp_dir[len-1] = 0;
  }

  dir = temp_dir;
  return ((r == NULL) ? FALSE : TRUE);
}


//
// GetCurrent: (method)
//
Bool Dir::GetCurrent(FileDir &dir)
{
  char temp_dir[DIRNAME_MAX] = "";
  char *r = _getcwd(temp_dir, DIRNAME_MAX);

  // Trim trailing slash
  U32 len = Utils::Strlen(temp_dir);
  if (temp_dir[len-1] == '/' || temp_dir[len-1] == '\\')
  {
    temp_dir[len-1] = 0;
  }
  
  dir = temp_dir;
  return ((r == NULL) ? FALSE : TRUE);
}


//
// SetCurrent: (method)
//
Bool Dir::SetCurrent(const char *dir)
{
  return (_chdir(dir) == -1 ? FALSE : TRUE);
}


//
// FindFirst: (method)
//
Bool Dir::FindFirst(Find &find, const char *dir, const char *fname, U32 attrib)
{
  FilePath  path;

  PathMake(path, dir, fname);
  return (FindFirst(find, path.str, attrib));
}


//
// FindFirst: (method)
//
Bool Dir::FindFirst(Find &find, const char *path, U32 attrib)
{
  find.findattrib = attrib;
  find.handle = _findfirst(path, &find.finddata);

  // No files, or some other error
  if (find.handle == -1)
  {
    return (FALSE);
  }

  // Does this file have ALL of the requested attributes
  if ((find.findattrib & find.finddata.attrib) == find.findattrib)
  {
    // Found one
    return (TRUE);
  }
  else
  {
    // Continue search
    return (FindNext(find));
  }
}


//
// FindNext: (method)
//
Bool Dir::FindNext(Find &find)
{
  for (;;)
  {  
    // No files, or some other error
    if (_findnext(find.handle, &find.finddata) == -1)
    {
      return (FALSE);
    }

    // Does this file have ALL of the requested attributes
    if ((find.findattrib & find.finddata.attrib) == find.findattrib)
    {
      // Found one
      return (TRUE);
    }
  } 
}


//
// FindClose: (method)
//
Bool Dir::FindClose(Find &find)
{
  _findclose(find.handle);
  return (TRUE);
}

//
// FindPath: (method)
// check subdirs for 'path' and return the full path to it in 'dst'
//
Bool Dir::FindPath( char *dst, U32 dstlen, const char *path)
{
  Dir::Find find;

  // Add each file
  char buffer[257];
  buffer[256] = 0x00;
  Utils::Strncpy( buffer, path, 256);

  // get pointer to just the path part of 'path'
  static char *nopath = ".";
  char *pathonly = nopath;

  U32 pathlen = 1;
  // get pointer to just the filename part of 'path'
  char *filename = Utils::ChopPath( buffer);
  U32 filelen = Utils::Strlen( filename);

  if (filename != buffer)
  {
    pathlen = filename - buffer - 1;
    pathonly = buffer;
    *(filename - 1) = '\0';
  }

  // check this path for filename
  if (Dir::FindFirst(find, pathonly, filename))
  {
    // Finish find operation
    Dir::FindClose(find);

    Utils::Strmcpy(dst, path, dstlen);
    return TRUE;
  }  
  // Finish find operation
  Dir::FindClose(find);

  // check subdirs for filename
  if (Dir::FindFirst(find, pathonly, "*."))
  {
    do
    {
      U32 len = strlen( find.finddata.name);
      if (!((len == 1 || len == 2) && *find.finddata.name == '.') && len + pathlen + 1 + filelen + 1 < PATHNAME_MAX)
      {
        char subbuffer[PATHNAME_MAX + 1];
        strcpy( subbuffer, pathonly);
        strcpy( subbuffer + pathlen, "\\");
        strcpy( subbuffer + pathlen + 1, find.finddata.name);
        strcpy( subbuffer + pathlen + 1 + len, "\\");
        strcpy( subbuffer + pathlen + 1 + len + 1, filename);

        if (FindPath( dst, dstlen, subbuffer))
        {
          Dir::FindClose(find);
          return TRUE;
        }
      }
    } 
    while (Dir::FindNext(find));

    // Finish find operation
    Dir::FindClose(find);
  }  
  return FALSE;
}

//
// Make: (method)
//
Bool Dir::Make(const char *dir)
{
  if (_mkdir(dir))
  {
    switch (errno)
    {
      case EACCES:
        LOG_WARN(("Path already exists (%s)", dir))
        return (FALSE);
        break;

      case ENOENT:
        LOG_WARN(("Path not found (%s)", dir))
        return (FALSE);
        break;
    }
  }

  return (TRUE);
}


//
// Dir::MakeFull
//
// Creates each directory within 'path', FALSE on error
//
Bool Dir::MakeFull(const char *dir)
{
  char buf[PATHNAME_MAX];
  const char *ptr, *walk = dir;

  do 
  {
    // have we found a '\' character
    if ((ptr = Utils::Strchr(walk, '\\')) != 0)
    {
      // copy portion infront of that char
      const char *s = dir;
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
      Utils::Strcpy(buf, dir);
    }

    // does it exist already
    if (!File::Exists(buf))
    {
      // create it
      if (!Make(buf))
      {
        return (FALSE);
      }
    }
  }
  while (ptr);

  return (TRUE);
}


//
// Remove: (method)
//
Bool Dir::Remove(const char *dir)
{
  if (_rmdir(dir))
  {
    switch (errno)
    {
      case ENOTEMPTY:
        LOG_ERR(("Given path is not a directory,\ndirectory is not empty\nor directory is either current working directory or root directory."))
        return (FALSE);
        break;

      case ENOENT:
        LOG_ERR(("Path is invalid."))
        return (FALSE);
        break;
    }
  }
  return (TRUE);
}


//
// Crc a directory
//

struct DirFile
{
  FilePath path;
  Bool dir;
  NList<DirFile>::Node node;

  DirFile(const char *path, Bool dir)
  : path(path), 
    dir(dir)
  {
  }

};

U32 Dir::Crc(const char *dir)
{
  LOGFMTOFF

  LOG_DIAG(("Crcing Directory %s", dir))

  U32 crc = 0;

  NList<DirFile> files(&DirFile::node);
  files.Append(new DirFile(dir, TRUE));

  // Iterate through the files and generate the CRC

  // If a directory is encountered then enumerate the files 
  // within the directory and append them to the list

  DirFile *file;

  while (files.GetCount())
  {
    NList<DirFile>::Iterator f(&files);
    while ((file = f++) != NULL)
    {
      if (file->dir)
      {
        FilePath path = file->path.str;
        Utils::Strcat(path.str, "\\*.*");

        // Use a tree to guarentee the files are sorted
        BinTree<DirFile> dir;

        // Its a directory, get the files within the directory
        Find find;
        if (FindFirst(find, path.str))
        {
          do
          {
            // If the last character is a '.' then ignore
            if (find.finddata.name[Utils::Strlen(find.finddata.name) - 1] != '.')
            {
              FilePath path = file->path.str;
              Utils::Strcat(path.str, "\\");
              Utils::Strcat(path.str, find.finddata.name);

              dir.Add(
                Crc::CalcStr(find.finddata.name), 
                new DirFile(path.str, find.finddata.attrib & _A_SUBDIR));
            }
          }
          while (FindNext(find));
        }
        FindClose(find);

        // Append the new files using the tree order
        for (BinTree<DirFile>::Iterator d(&dir); *d; d++)
        {
          files.Append(*d);
        }

        // Kill the tree
        dir.UnlinkAll();
      }
      else
      {
        // Its a file, get its CRC
        crc = File::Crc(file->path.str, crc);

        LOG_DIAG(("[%08X] %s", crc, file->path.str))
      }
      files.Dispose(file);
    }
  }

  LOG_DIAG(("[%08X]", crc))

  LOGFMTON

  return (crc);
}


///////////////////////////////////////////////////////////////////////////////
//
// Drive Implementations
//


//
// FindDriveByVolume
//
// Find a drive with the given type and volume (zero based)
//
Bool Drive::FindDriveByVolume(Type type, const char *label, int &d)
{
  // Get the logical drives
  U32 logical = GetLogicalDrives();

  // Generate root dir string
  char root[4] = " :\\";

  // Step through each drive letter
	for (U32 drive = 0; drive < 26; ++drive)
	{
    // Does this system have this drive
		if (logical & (1 << drive))
		{
      // Are we interested in this type
      if (GetType(drive) == type)
      {
        char volume[64];
        char systemName[64];
        U32 maxComponentLen;
        U32 flags;

        // Setup the root dir
        root[0] = char(drive + 'A');

        // Get the volume name
        if (GetVolumeInformation(root, volume, 64, NULL, &maxComponentLen, &flags, systemName, 64))
        {
          // Is it a match
          if (!Utils::Stricmp(volume, label))
          {
            // Set the drive index
            d = drive;

            // Success
            return (TRUE);
          }
        }
        else
        {
          LOG_DIAG(("Unable to get volume information for drive [%s]", root));
        }
      }
		}
	}

  return (FALSE); 
}


//
// GetType: (method)
//
// Get the type of the given drive (zero based)
//
Drive::Type Drive::GetType(const int drive)
{
  U32  type;
  char drivestr[4];

  drivestr[0] = (char) (drive + 'A');
  drivestr[1] = ':';
  drivestr[2] = '\\';
  drivestr[3] = '\0';
  
  type = GetDriveType(drivestr);

  switch (type)
  {
    case 0:
      return (UNKNOWN);
      break;

    case 1:
      return (NONEXIST);
      break;

    case DRIVE_REMOVABLE:
      return (REMOVABLE);
      break;

    case DRIVE_FIXED:
      return (FIXED);
      break;

    case DRIVE_REMOTE:
      return (NETWORK);
      break;

    case DRIVE_CDROM:
      return (CDROM);
      break;

    case DRIVE_RAMDISK:
      return (RAMDISK);
      break;

    default:
      LOG_WARN(("GetDriveType returned unknown value %d", type))
      return (UNKNOWN);
      break;
  }
}


//
// GetCurrent: (method)
//
// Get the current drive index (zero based)
//
int Drive::GetCurrent()
{
  return (_getdrive() - 1);
}


//
// SetCurrent: (method)
//
// Set the current working drive (zero based)
//
Bool Drive::SetCurrent(const int drive)
{
  return (_chdrive(drive + 1) ? FALSE : TRUE);
}
