///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File Class
// 1-DEC-1997
//


#ifndef __FILE_H
#define __FILE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include <io.h>


// HACKS
#undef FILENAME_MAX


//
// Definitions
//
#define DRIVENAME_MAX       3
#define DIRNAME_MAX         256
#define PATHNAME_MAX        260
#define FILENAME_MAX        260
#define EXTNAME_MAX         256

// Open file map checking
//#define CHECKMAPPINGS

//
// Type Defintions
//

// Drive Name
typedef StrBuf<DRIVENAME_MAX> FileDrive;

// Directory Name
typedef StrBuf<DIRNAME_MAX>   FileDir;

// Path Name (filename)
typedef StrBuf<PATHNAME_MAX>  FilePath;

// File Name
typedef StrBuf<FILENAME_MAX>  FileName;

// File Extension
typedef StrBuf<EXTNAME_MAX>   FileExt;


///////////////////////////////////////////////////////////////////////////////
//
// File Class
//
class File
{
private:

  // Handle for current file
  HANDLE handle;

  // Memory mapped handle
  HANDLE mapHandle;

  // Number of file mapping views
  U32 mapCount;

  // Mode of current file
  U32 mode;

  #ifdef CHECKMAPPINGS

    FileName fileName;

    // Open mappings
    BinTree<FileName> openMappings;

  #endif

public:

  enum Attrib
  {
    // Normal, no file restrictions
    NORMAL =   0x0000,

    // File is READ ONLY
    READONLY = 0x0001,

    // File is HIDDEN
    HIDDEN =   0x0002,

    // File is a SYSTEM file
    SYSTEM =   0x0004,
    
    // File is a SUB DIRECTORY
    SUBDIR =   0x0010,

    // File has ARCIVE flag set
    ARCHIVE =  0x0020,

    // All files
    ALLFILE =  0x002F,

    // All files and directories
    ALL =      0x003F 
  };

  // Open modes
  enum Mode
  {
    // Allow reading of the file
    READ =     0x0001,

    // Allow writting of the file
    WRITE =    0x0002,

    // Move position to end of file before every write
    APPEND =   0x0004,

    // Create new file or delete old one
    CREATE =   0x0008,

    // Flush data after every write
    FLUSH =    0x0020,

    // Do not throw an error if a read or write fails
    NOERR =    0x0040,
  };

  // Seek Modes
  enum Seek
  {
    // Seek from start of file
    SET = FILE_BEGIN,

    // Seek from current location in file
    CUR = FILE_CURRENT,

    // Seek from end of file
    END = FILE_END 
  };

  enum
  {
    // Map file for readonly
    MAP_READ = 0x01,

    // Map file for write access
    MAP_WRITE = 0x02
  };

public:

  // Contructor
  File();

  // Destructor
  ~File();

  // Open Methods
  Bool Open(const char *dir, const char *filename, U32 mode);
  Bool Open(const char *path, U32 mode);

  // Close Methods
  void Close();

  // Is the file open
  Bool IsOpen();

  // Is the file mapped into memory
  Bool IsMapped();

  // Read Methods
  U32 Read(void *dst, U32 count, U32 size = 1);

  // Write Methods
  U32 Write(const void *src, U32 count, U32 size = 1);

  // Write 8-bit data
  Bool WriteU8(U8 data);

  // Write 16-bit data
  Bool WriteU16(U16 data);

  // Write 32-bit data
  Bool WriteU32(U32 data);

  // Write a string
  Bool CDECL WriteString(const char *format, ...);

  // Set the current location in the file
  Bool Seek(U32 pos, S32 offset);

  // Return current file location
  U32 Tell();

  // Flush the write buffer
  Bool Flush();

  // Get the size of the currently open file
  U32 GetSize();

  // Map all or part of file into memory, return a pointer to the memory
  void *MapMemory(const char *name, U32 flags = MAP_READ, U32 offset = 0, U32 length = 0, void **alignedPointer = NULL);

  // Close a previously opened memory map
  void UnmapMemory(void *data);

  // Make Unique file name
  static Bool Unique(const char *tplate, FileName &filename);

  // Does a file exist ?
  static Bool Exists(const char *dir, const char *filename);
  static Bool Exists(const char *path);

  // Unlink (delete) a file
  static Bool Unlink(const char *dir, const char *filename);
  static Bool Unlink(const char *path);

  // Rename a file
  static Bool Rename(const char *dir, const char *filename, const char *newname);
  static Bool Rename(const char *path, const char *newpath);

  // Copy a file
  static Bool Copy(const char *src, const char *dst, Bool overwrite = FALSE, Bool progress = FALSE);

  // Remove a file or a directory (recurse, no confirmation)
  static Bool Remove(const char *path);

  // Get the Attributes of a file
  static Bool GetAttrib(const char *dir, const char *filename, U32 &attrib);
  static Bool GetAttrib(const char *path, U32 &attrib);

  // Set the Attributes of a file
  static Bool SetAttrib(const char *dir, const char *filename, U32 attrib);
  static Bool SetAttrib(const char *path, U32 attrib);

  // Cyclic Redundancy Check
  static U32 Crc(const char *dir, const char *filename, U32 crc = 0);
  static U32 Crc(const char *path, U32 crc = 0);

  // Error reporting
  void ReadError(U32 bytes);
  void WriteError(U32 bytes);

};


///////////////////////////////////////////////////////////////////////////////
//
// Directory Class
//
class Dir
{
private:

public:

  // Used when scanning directories
  class Find
  {
  private:

    // Contains the requested attributes
    U32 findattrib;

    // Handle used during file search
    int handle;

  public:

    // Data from the find functions
    _finddata_t finddata;

    friend class Dir;
  };

  // Directory String Manipulation
  static Bool PathAbsolute(FilePath &absolute, const char *relative);
  static void PathExpand(const char *path, FileDrive &drive, FileDir &dir, FileName &name, FileExt &ext);
  static void PathMake(FilePath &path, const char *drive, const char *dir, const char *name, const char *ext);
  static void PathMake(FilePath &path, const char *dir, const char *name);

  // Current Directory
  static Bool GetCurrent(const int drive, FileDir &dir);
  static Bool GetCurrent(FileDir &dir);
  static Bool SetCurrent(const char *dir);

  // Directory Listing
  static Bool FindFirst(Find &find, const char *dir, const char *fname, U32 attrib = File::Attrib::NORMAL);
  static Bool FindFirst(Find &find, const char *fname, U32 attrib = File::Attrib::NORMAL);
  static Bool FindNext(Find &find);
  static Bool FindClose(Find &find);

  static Bool FindPath( char *dst, U32 dstlen, const char *path);

  // Directory Manipulation
  static Bool Make(const char *dir);
  static Bool MakeFull(const char *dir);
  static Bool Remove(const char *dir);

  // Crc a directory
  static U32 Crc(const char *dir);

};


///////////////////////////////////////////////////////////////////////////////
//
// Drive Class
//
class Drive
{
public:

  // Drive Type
  enum Type
  {
    // Drive Type is unknown
    UNKNOWN,

    // Drive does not exist
    NONEXIST,

    // Drive is removable
    REMOVABLE,

    // Drive is fixed
    FIXED,

    // Drive is networked
    NETWORK,

    // Drive is a CD-ROM
    CDROM,

    // Drive is a RAM DISK
    RAMDISK
  };

  // Find a drive with the given type and volume (zero based)
  static Bool FindDriveByVolume(Type type, const char *label, int &d);

  // Get the type of the given drive (zero based)
  static Type GetType(const int drive);

  // Get the current drive index (zero based)
  static int GetCurrent();

  // Set the current working drive (zero based)
  static Bool SetCurrent(const int drive);
};

#endif
