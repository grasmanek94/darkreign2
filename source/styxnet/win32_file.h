///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1998
// Matthew Versluys
//
// Win32 File
//


#ifndef __WIN32_FILE_H
#define __WIN32_FILE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "win32.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define DRIVENAME_MAX 3
#define DIRNAME_MAX   256
#define PATHNAME_MAX  260
#define FILENAME_MAX  260
#define EXTNAME_MAX   256


///////////////////////////////////////////////////////////////////////////////
//
// Type Defintions
//
typedef StrBuf<DRIVENAME_MAX> FileDrive;  // Drive Name
typedef StrBuf<DIRNAME_MAX>   FileDir;    // Directory Name
typedef StrBuf<PATHNAME_MAX>  FilePath;   // Path Name (filename)
typedef StrBuf<FILENAME_MAX>  FileName;   // File Name
typedef StrBuf<EXTNAME_MAX>   FileExt;    // File Extension


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
  class File
  {
  private:

    // Handle for the current file
    HANDLE handle;

    // Mode
    U32 mode;

  public:

    class Mapping;

    enum Mode
    {
      MODE_READ              = 0x0001, // Read
      MODE_WRITE             = 0x0002, // Write
      MODE_SHARE_READ        = 0x0004, // Allow others to read
      MODE_SHARE_WRITE       = 0x0008, // Allow others to write

      MODE_CREATE_MASK       = 0x0070, // Create mask (don't use this)

      MODE_CREATE_NEW        = 0x0010, // Create a new file (fail if it exists)
      MODE_CREATE_ALWAYS     = 0x0020, // Create a new file (delete if it exits)
      MODE_OPEN_EXISTING     = 0x0030, // Open an existing file (fail if it doesn't exist)
      MODE_OPEN_ALWAYS       = 0x0040, // Open a file (create if it doesn't exit)
      MODE_TRUNCATE_EXISTING = 0x0050, // Open a file and truncate it (fail if it exists)

      MODE_ATTRIB_ARCHIVE    = 0x0100, // Archive attribute
      MODE_ATTRIB_HIDDEN     = 0x0200, // Hidden attribute
      MODE_ATTRIB_READONLY   = 0x0400, // Read-Only attribute
      MODE_ATTRIB_SYSTEM     = 0x0800, // System attribute
      MODE_ATTRIB_SUBDIR     = 0x1000, // Sub Directory

      MODE_NOERROR           = 0x8000  // No Error reporting
    };

    enum Seek
    {
      SET,  // Seek from start of file
      CUR,  // Seek from current location in file
      END   // Seek from end of file
    };

  public:

    // Constructor and Destructor
    File();
    ~File();

    // Open Methods
    Bool Open(const char *dir, const char *file, U32 mode);
    Bool Open(const char *path, U32 mode);

    // Close Methods
    Bool Close();

    // Queries

    // Is the file open
    Bool IsOpen();

    // Are we at the end of the file ?
    Bool IsEof();

    // Read Methods

    // Read via Template
    template <class DATA> Bool Read(DATA &data)
    {
      return ((Read(&data, sizeof (DATA)) == sizeof (DATA)) ? TRUE : FALSE);
    }

    // Generic Read
    U32 Read(void *dst, U32 count, U32 size = 1);

    // Write Methods

    // Write via Template
    template <class DATA> Bool Write(DATA &data)
    {
      return ((Write(&data, sizeof (DATA)) == sizeof (DATA)) ? TRUE : FALSE);
    }

    // Generic Write
    U32 Write(const void *src, U32 count, U32 size = 1);

    // Location Methods

    // Set the current location in the file
    Bool Seek(U32 pos, S32 offset);
  
    // Return current file location
    U32 Tell();

    // Misc Methods

    // Flush the write buffer
    Bool Flush();

    // Get the size of the currently open file
    U32 GetSize();

    // Error reporting
    void Error(const char *message);

  public:

    // Does a file exist ?
    static Bool Exists(const char *dir, const char *filename);
    static Bool Exists(const char *path);

    // Unlink (delete) a file
    static Bool Unlink(const char *dir, const char *filename);
    static Bool Unlink(const char *path);

    // Rename a file
    static Bool Rename(const char *dir, const char *filename, const char *newname);
    static Bool Rename(const char *path, const char *newpath);

    // Get the Attributes of a file
    static Bool GetAttrib(const char *dir, const char *filename, U32 &attrib);
    static Bool GetAttrib(const char *path, U32 &attrib);

    // Set the Attributes of a file
    static Bool SetAttrib(const char *dir, const char *filename, U32 attrib);
    static Bool SetAttrib(const char *path, U32 attrib);

    // Cyclic Redundancy Check
    static U32 Crc(const char *dir, const char *filename);
    static U32 Crc(const char *path);

  public:

    friend class Mapping;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class File::Mapping
  //
  class File::Mapping
  {
  private:

    HANDLE handle;
    U8 *ptr;

  public:

    // Constructor and destructor
    Mapping();
    ~Mapping();
    
    // Setup the mapping from the given file
    Bool Setup(File &file, U32 offset = 0, U32 length = 0);

    // Cleanup the mapping
    void Cleanup();

  public:

    U8 * GetPointer()
    {
      return (ptr);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Dir
  //
  namespace Dir
  {

    // Directory String Manipulation
    Bool PathAbsolute(FilePath &absolute, const char *relative);
    Bool PathExpand(const char *path, FileDrive &drive, FileDir &dir, FileName &name, FileExt &ext);
    Bool PathMake(FilePath &path, const char *drive, const char *dir, const char *name, const char *ext);
    Bool PathMake(FilePath &path, const char *dir, const char *name);

  }

}

#endif
