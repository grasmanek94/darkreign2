///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Win32 File
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "logging.h"
#include "win32_file.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Win32
//
namespace Win32
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class File
  //

  //
  // File::File
  //
  // Constructor
  //
  File::File()
  : handle(INVALID_HANDLE_VALUE)
  {
  }


  //
  // File::~File
  //
  // Destructor
  //
  File::~File()
  {
    // Close the file
    Close();
  }


  //
  // File::Open
  //
  // Open a file using a directory/filename
  //
  Bool File::Open(const char *dir, const char *fname, U32 mode)
  {
    FilePath path;
    Dir::PathMake(path, dir, fname);
    return (Open(path.str, mode));
  }


  //
  // File::Open
  //
  // Open a file using a path
  //
  Bool File::Open(const char *path, U32 modeIn)
  {
    ASSERT(!IsOpen())

    // Save the mode
    mode = modeIn;

    U32 access = 0;
    U32 share = 0;
    U32 create = 0;
    U32 attrib = 0;

    // Translate mode into attributes

    access |= mode & MODE_READ ? GENERIC_READ : 0;
    access |= mode & MODE_WRITE ? GENERIC_WRITE : 0;

    share |= mode & MODE_SHARE_READ ? FILE_SHARE_READ : 0;
    share |= mode & MODE_SHARE_WRITE ? FILE_SHARE_WRITE : 0;

    switch (mode & MODE_CREATE_MASK)
    {
      case MODE_CREATE_NEW:
        create = CREATE_NEW;
        break;

      case MODE_CREATE_ALWAYS:
        create = CREATE_ALWAYS;
        break;

      case MODE_OPEN_EXISTING:
        create = OPEN_EXISTING;
        break;

      case MODE_OPEN_ALWAYS:
        create = OPEN_ALWAYS;
        break;

      case MODE_TRUNCATE_EXISTING:
        create = TRUNCATE_EXISTING;
        break;

      default:
        LERR("Unknown create/open mode")
    }

    attrib = FILE_ATTRIBUTE_NORMAL;
    attrib |= mode & MODE_ATTRIB_ARCHIVE ? FILE_ATTRIBUTE_ARCHIVE : 0;
    attrib |= mode & MODE_ATTRIB_HIDDEN ? FILE_ATTRIBUTE_HIDDEN : 0;
    attrib |= mode & MODE_ATTRIB_READONLY ? FILE_ATTRIBUTE_READONLY : 0;
    attrib |= mode & MODE_ATTRIB_SYSTEM ? FILE_ATTRIBUTE_SYSTEM : 0;

    handle = CreateFile(path, access, share, NULL, create, attrib, NULL);

    // Did an error occur
    if (handle == INVALID_HANDLE_VALUE)
    {
      Error("Could not open file");
      return (FALSE);
    }
    else
    {
      return (TRUE);
    }
  }


  //
  // File::Close
  //
  // Close an open file
  // 
  Bool File::Close()
  {
    if (IsOpen() && !CloseHandle(handle))
    {
    //  LOG_WARN("Attempt to close invalid file handle")
      return (FALSE);
    }
    else
    {
      handle = NULL;
      return (TRUE);
    }
  }


  //
  // File::IsOpen
  //
  // Test to see if a file is open
  //
  Bool File::IsOpen()
  {
    return ((handle == INVALID_HANDLE_VALUE) ? FALSE : TRUE);
  }


  //
  // File::IsEof
  //
  // Test to see if a file is at its end point
  //
  Bool File::IsEof()
  {
    ASSERT(IsOpen())

    return (TRUE);
  }


  //
  // File::Read
  //
  // Returns the number of bytes read
  // Note that this is not the number of objects if size > 1
  //
  U32 File::Read(void *buffer, U32 count, U32 size)
  {
    ASSERT(IsOpen())

    U32 total = count * size;
    U32 read;

    if (ReadFile(handle, buffer, total, &read, NULL))
    {
      return (read);
    }
    else
    {
      Error("Could not read from file");
      return (0);
    }
  }



  //
  // File::Write
  //
  // Returns the number of bytes written
  //
  U32 File::Write(const void *src, U32 count, U32 size)
  {
    ASSERT(IsOpen())

    U32 total = count * size;
    U32 written;

    if (!WriteFile(handle, src, total, &written, NULL))
    {
      return (written);
    }
    else
    {
      Error("Could not write to file");
      return (0);
    }

    return (written);
  }


  //
  // File::Seek
  //
  // Seek to a position in the file
  //
  Bool File::Seek(U32 pos, S32 offset)
  {
    ASSERT(IsOpen())

    // Convert method
    U32 method;

    switch (pos)
    {
      case SET:
        method = FILE_BEGIN;
        break;

      case CUR:
        method = FILE_CURRENT;
        break;

      case END:
        method = FILE_END;
        break;

      default:
        LERR("Invalid origin specified")
        return (FALSE);
        break;
    }

    if (SetFilePointer(handle, offset, NULL, method) == 0xFFFFFFFF)
    {
      return (FALSE);
    }

    return (TRUE);
  }


  //
  // File::Tell
  //
  // Tell the current position in the file
  //
  U32 File::Tell()
  {
    ASSERT(IsOpen())

    U32 offset = SetFilePointer(handle, 0, NULL, FILE_CURRENT);

    if (offset == 0xFFFFFFF)
    {
      return (0);
    }
    else
    {
      return (offset);
    }
  }


  //
  // File::Flush
  //
  // Flush the file
  //
  Bool File::Flush()
  {
    ASSERT(IsOpen())
  
    if (FlushFileBuffers(handle))
    {
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // File::GetSize
  //
  // Get the size of the file
  //
  U32 File::GetSize()
  {
    ASSERT(IsOpen())
    U32 size = GetFileSize(handle, NULL);
    return (size);
  }


  //
  // File::Error
  //
  // Error reporting
  //
  void File::Error(const char *message)
  {
    if (mode & MODE_NOERROR)
    {
      LERR(message << " :" << Debug::LastError())
    }
    else
    {
      LDIAG(message << " :" << Debug::LastError())
    }
  }


  //
  // File::Exists
  //
  Bool File::Exists(const char *dir, const char *fname)
  {
    FilePath  path;

    Dir::PathMake(path, dir, fname);
    return (Exists(path.str));
  }


  //
  // File::Exists
  //
  Bool File::Exists(const char *path)
  {
    File file;
    Bool exists = file.Open(path, MODE_READ | MODE_OPEN_EXISTING);
    file.Close();
    return (exists);
  }


  //
  // File::Unlink
  //
  Bool File::Unlink(const char *dir, const char *fname)
  {
    FilePath  path;

    Dir::PathMake(path, dir, fname);
    return (Unlink(path.str));
  }


  //
  // File::Unlink
  //
  Bool File::Unlink(const char *path)
  {
    return (DeleteFile(path));
  }


  //
  // File::Rename
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
  // File::Rename
  //
  Bool File::Rename(const char *path, const char *newpath)
  {
    path;
    newpath;
    LERR("Uninplemented!")
    return (TRUE);
  }


  //
  // File::GetAttrib
  //
  Bool File::GetAttrib(const char *dir, const char *fname, U32 &attrib)
  {
    FilePath  path;

    Dir::PathMake(path, dir, fname);
    return (GetAttrib(path.str, attrib));
  }


  //
  // File::GetAttrib
  //
  Bool File::GetAttrib(const char *path, U32 &attrib)
  {
    U32 att;
  
    att = GetFileAttributes(path);

    if (att == (U32) -1)
    {
      return (FALSE);
    }

    attrib = 0;

    if (att & FILE_ATTRIBUTE_ARCHIVE)
    {
      attrib |= MODE_ATTRIB_ARCHIVE;
    }
    
    if (att & FILE_ATTRIBUTE_DIRECTORY)
    {
      attrib |= MODE_ATTRIB_SUBDIR;
    }
  
    if (att & FILE_ATTRIBUTE_HIDDEN)
    {
      attrib |= MODE_ATTRIB_HIDDEN;
    }
  
    if (att & FILE_ATTRIBUTE_READONLY)
    {
      attrib |= MODE_ATTRIB_READONLY;
    }

    if (att & FILE_ATTRIBUTE_SYSTEM)
    {
      attrib |= MODE_ATTRIB_SYSTEM;
    }

    return (TRUE);
  }


  //
  // File::SetAttrib
  //
  Bool File::SetAttrib(const char *dir, const char *fname, U32 attrib)
  {
    FilePath  path;

    Dir::PathMake(path, dir, fname);
    return (SetAttrib(path.str, attrib));
  }


  //
  // File::SetAttrib
  //
  Bool File::SetAttrib(const char *path, U32 attrib)
  {
    U32 att;

    att= FILE_ATTRIBUTE_NORMAL;
  
    if (attrib & MODE_ATTRIB_READONLY)
    {
      att |= FILE_ATTRIBUTE_READONLY;
    }

    if (attrib & MODE_ATTRIB_HIDDEN)
    {
      att |= FILE_ATTRIBUTE_HIDDEN;
    }

    if (attrib & MODE_ATTRIB_SYSTEM)
    {
      att |= FILE_ATTRIBUTE_SYSTEM;
    }

    if (attrib & MODE_ATTRIB_SUBDIR)
    {
      att |= FILE_ATTRIBUTE_DIRECTORY;
    }

    if (attrib & MODE_ATTRIB_ARCHIVE)
    {
      att |= FILE_ATTRIBUTE_ARCHIVE;
    }

    return (SetFileAttributes(path, att) ? TRUE : FALSE);
  }


  //
  // File::Crc
  //
  U32 File::Crc(const char *dir, const char *fname)
  {
    FilePath  path;

    Dir::PathMake(path, dir, fname);
    return (Crc(path.str));
  }


  //
  // File::Crc
  //
  U32 File::Crc(const char *path)
  {
    const U32 buffsize = 65536;
    U8 * buffer;
    U32 count;
    U32 crc;
    File f;

    // Can we open the file ?
    if (!f.Open(path, MODE_READ))
    {
      LERR(("Could not open file to collect CRC"))
      return (0);
    }

    // Allocate buffer and initialise CRC
    buffer = new U8[buffsize];
    crc = 0;

    // Read into buffer
    count = f.Read(buffer, buffsize);
    while (count == buffsize)
    {
      // Whilst buffer is full keep reading it
      crc = Crc::Calc(buffer, count, crc);
      count = f.Read(buffer, buffsize);
    }

    // Calculate CRC of remaining data
    crc = Crc::Calc(buffer, count, crc);

    // Delete buffer
    delete buffer;

    // Close file
    f.Close();
  
    // Return CRC
    return (crc);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class File::Mapping
  //


  //
  // Constructor
  //
  File::Mapping::Mapping()
  : handle(NULL),
    ptr(NULL)
  {
  }


  //
  // Destructor
  //
  File::Mapping::~Mapping()
  {
    Cleanup();
  }
  

  //
  // Setup the mapping from the given file
  //
  Bool File::Mapping::Setup(File &file, U32 offset, U32 length)
  {
    // Get the size of the file (assume that its is less than 4GB)
    U32 size = GetFileSize(file.handle, NULL);

    // If length is greater than size and we're writting, change size
    if (file.mode & MODE_WRITE && length > size)
    {
      size = length;
    }

    // If length wasn't specified, assume its the entire file
    if (!length)
    {
      length = size;
    }

    // Create the file mapping
    U32 f = 0;
    if (file.mode & MODE_WRITE)
    {
      f = PAGE_READWRITE;
    }
    else
    {
      f = PAGE_READONLY;
    }
    handle = CreateFileMapping(file.handle, NULL, f, 0, size, NULL);

    // Could the file be mapped
    if (handle == NULL)
    {
      LWARN("Could not create file mapping: " << Debug::LastError())
      return (FALSE);
    }

    // Get a pointer to the mapping
    f = 0;
    if (file.mode & MODE_WRITE)
    {
      f = FILE_MAP_WRITE;
    }
    else
    {
      f = FILE_MAP_READ;
    }
    ptr = (U8 *) MapViewOfFile(handle, f, 0, offset, length);

    // Could we get a view of the mapping ?
    if (ptr == NULL)
    {
      LWARN("Could not create a view of the mapping: " << Debug::LastError())
      Cleanup();
      return (FALSE);
    }

    return (TRUE);
  }


  //
  // Cleanup the mapping
  //
  void File::Mapping::Cleanup()
  {
    if (ptr)
    {
      UnmapViewOfFile(ptr);
      ptr = NULL;
    }

    if (handle)
    {
      CloseHandle(handle);
      handle = NULL;
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Dir
  //
  namespace Dir
  {

    //
    // PathAbsolute
    //
    Bool PathAbsolute(FilePath &absolute, const char *relative)
    {
      absolute;
      relative;

      LERR("Uninplemented!")
  //    return (GetFullPathName(relative, PATHNAME_MAX, absolute, NULL) ? TRUE : FALSE);
      return (FALSE);
    }


    //
    // PathExpand
    //
    Bool PathExpand(const char *path, FileDrive &drive, FileDir &dir, FileName &fname, FileExt &ext)
    {
      _splitpath(path, drive.str, dir.str, fname.str, ext.str);
      return (TRUE);
    }


    //
    // PathMake
    //
    Bool PathMake(FilePath &path, const char *drive, const char *dir, const char *fname, const char *ext)
    {
      _makepath(path.str, drive, dir, fname, ext);
      return (TRUE);
    }


    //
    // PathMake
    //
    Bool PathMake(FilePath &path, const char *dir, const char *fname)
    {
      _makepath(path.str, NULL, dir, fname, NULL);
      return (TRUE);
    }

  }

}
