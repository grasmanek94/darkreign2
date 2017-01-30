///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// File System Types
//
// 11-JUN-1997
//

#ifndef __FILETYPES_H
#define __FILETYPES_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// The maximum size of a file name identifer
#define MAX_FILEIDENT       128

// The maximum size of the full path of a file name
#define MAX_PATHIDENT       270

// The maximum size of a file system identifier
#define MAX_FILESYSIDENT     32


// File Identifier - for file names
typedef StrCrc<MAX_FILEIDENT> FileIdent;
typedef StrBuf<MAX_FILEIDENT> FileString;

// Path Identifier - for full path of a file name
typedef StrCrc<MAX_PATHIDENT> PathIdent;
typedef StrBuf<MAX_PATHIDENT> PathString;

// General File System Identifier (stream id's etc)
typedef StrCrc<MAX_FILESYSIDENT> FileSysIdent;
typedef StrBuf<MAX_FILESYSIDENT> FileSysString;


#endif