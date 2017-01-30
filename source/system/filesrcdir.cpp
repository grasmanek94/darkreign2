///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Resource Stream Management
//
// 16-DEC-1997
//


#include "filesrcdir.h"
#include "str.h"


namespace FileSys
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DataFileDir - Direct access data file
  //


  //
  // Constructor
  //  
  FileSrcDir::DataFileDir::DataFileDir(FileSrcDir *source, const char *nameIn) :
    DataFile(nameIn),
    source(source),
    memoryPtr(NULL)
  {
    file.Open(source->Path(), name.str, File::READ);
  }


  //
  // Destructor
  //
  FileSrcDir::DataFileDir::~DataFileDir()
  {
    // Unmap memory if needed
    if (memoryPtr)
    {
      file.UnmapMemory(memoryPtr);
      memoryPtr = NULL;
    }

    file.Close();
  }

  
  //
  // IsOpen
  //
  // Was the file successfully opened
  //  
  Bool FileSrcDir::DataFileDir::IsOpen()
  {
    return (file.IsOpen());
  }


  //
  // Size
  //
  // Returns the file size in bytes
  //
  U32 FileSrcDir::DataFileDir::Size()
  {
    return (file.GetSize());
  }


  //
  // Read
  //
  // Read 'size' bytes into 'dst'.  returns bytes actually read.
  //
  U32 FileSrcDir::DataFileDir::Read(void *dst, U32 size)
  {
    return (file.Read(dst, 1, size));
  }


  //
  // Seek
  //
  // Jumps to the position 'offset' bytes from the start of the file.
  // Returns TRUE if valid position, FALSE otherwise
  //
  Bool FileSrcDir::DataFileDir::Seek(U32 offset)
  {
    return (file.Seek(File::SET, offset));
  }


  //
  // FilePos
  //
  // Returns the current file position in bytes
  //
  U32 FileSrcDir::DataFileDir::FilePos()
  {
    return (file.Tell());
  }


  //
  // GetMemoryPtr
  //
  // Return the memory mapped address of the file
  //
  void *FileSrcDir::DataFileDir::GetMemoryPtr()
  {
    if (memoryPtr == NULL)
    {
      memoryPtr = file.MapMemory(name.str);
    }
    return (memoryPtr);
  }


  //
  // Path
  //
  // Returns the physical path that represents this file
  //
  const char * FileSrcDir::DataFileDir::Path()
  {
    return (source.Alive() ? source->Path() : "");
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FastFindDir - Fast find for this source
  //

  //
  // Open
  //
  // Open this file
  //
  DataFile* FileSrcDir::FastFindDir::Open()
  {
    return (source->Open(name.str));
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FileSrcDir - Directory file source
  //

  //
  // Constructor
  //
  FileSrcDir::FileSrcDir(const char *dir) : FileSrc()
  {
    // Does the target directory exist
    if (File::Exists(dir))
    {
      // Set local info
      dirId = dir;

      // Build the file index
      BuildIndex();

      // Now we are setup
      setup = TRUE;
    }
  }


  //
  // Destructor
  //
  FileSrcDir::~FileSrcDir()
  {
    index.DisposeAll();
  }


  //
  // BuildIndex
  //
  // Builds the current file index for this source
  //
  void FileSrcDir::BuildIndex()
  {
    Dir::Find find;

    // Clear current index
    index.DisposeAll();

    // Build index of all files in this dir
    if (Dir::FindFirst(find, dirId.str, "*.*"))
    {
      do
      {
        // Exclude directories
        if (!(find.finddata.attrib & File::Attrib::SUBDIR))
        {
          // Add the crc of the filename to the index
          index.Add(Crc::CalcStr(find.finddata.name));
        }
      } 
      while (Dir::FindNext(find));
    }  

    // Finish find operation
    Dir::FindClose(find);
  }


  //
  // Exists
  //
  // True if file 'crc' exists in this source
  //
  Bool FileSrcDir::Exists(U32 crc)
  {
    ASSERT(IsSetup());
    return (index.Exists(crc));
  }


  //
  // GetFastFind
  //
  // Gets a fast find object from this source
  //
  FastFind* FileSrcDir::GetFastFind(const char *name, ResourceStream *stream)
  {
    ASSERT(IsSetup());

    FastFindDir *fastFind = NULL;
    File file;

    // Does the file exist
    if (Exists(Crc::CalcStr(name)))
    {
      // Open it
      if (file.Open(dirId.str, name, File::READ))
      {   
        // Allocate new fast find
        fastFind = new FastFindDir(name, file.GetSize(), stream, this);

        // Close the file
        file.Close();
      }
      else
      {
        LOG_ERR(("Expected to open %s in %s - May have been deleted after dir scan!", name, dirId.str));
      }
    }

    // may or may not have found file
    return (fastFind);
  }


  //
  // Open
  //
  // Open a file from this source (null if not found)
  // 
  DataFile * FileSrcDir::Open(const char *name)
  {
    ASSERT(IsSetup());

    DataFileDir *dFile = NULL;

    // Does the file exist
    if (Exists(Crc::CalcStr(name)))
    {
      // Allocate a new directory specific data file
      dFile = new DataFileDir(this, name);

      // Was the file successfully opened
      if (!dFile->IsOpen())
      {
        delete dFile;
        dFile = NULL;
        LOG_ERR(("Expected to open %s in %s - May have been deleted after dir scan!", name, dirId.str));
      }
    }

    // May or may not have found file
    return (dFile);
  };


  //
  // Path
  //
  // Returns the physical path that represents this source
  //
  const char * FileSrcDir::Path()
  {
    return (dirId.str);
  }


  //
  // LogSource
  //
  // For debugging, logs source info
  //
  void FileSrcDir::LogSource(U32 indent)
  {
    ASSERT(IsSetup());

    // Generate the indent
    String iStr;
    iStr.Fill(indent, ' ');

    LOG_DIAG(("%sDIR %s (%d files)", *iStr, dirId.str, index.GetCount()));
  }
}