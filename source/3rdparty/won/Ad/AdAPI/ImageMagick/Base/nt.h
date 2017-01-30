/*
  Windows NT Utility Methods for ImageMagick.
*/
#ifndef _NT_H
#define _NT_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include <winuser.h>
#include <wingdi.h>
#include <io.h>
#include <errno.h>

/*
  Define declarations.
*/
#define PROT_READ  1
#define PROT_WRITE  2
#define PROT_READWRITE  3
#define MAP_SHARED  1
#define MAP_PRIVATE  2
/*
  Typedef declarations.
*/
#if !defined(XS_VERSION)
typedef struct _DIR
{
  HANDLE
    hSearch;
 
  WIN32_FIND_DATA
    Win32FindData;
} DIR;

struct dirent
{
  char
     d_name[2048];
 
  int
    d_namlen;
};
#endif

/*
  NT utilities routines.
*/
extern __declspec(dllexport) char
  *SetClientName(const char *);

extern __declspec(dllexport) int
  Exit(int),
  ImageFormatConflict(const char *),
  IsWindows95(),
  NTSystemCommand(const char *);

#if !defined(XS_VERSION)
extern __declspec(dllexport) DIR
  *opendir(char *);
 
extern __declspec(dllexport) double
  NTElapsedTime(void),
  NTUserTime(void);

extern __declspec(dllexport) int
  munmap(void *,size_t),
  NTTemporaryFilename(char *);

extern __declspec(dllexport) long
  telldir(DIR *);

extern __declspec(dllexport) struct dirent
  *readdir(DIR *);
 
extern __declspec(dllexport) void
  closedir(DIR *),
  *mmap(char *,size_t,int,int,int,off_t),
  NTErrorHandler(const ErrorType,const char *,const char *),
  NTWarningHandler(const WarningType,const char *,const char *),
  seekdir(DIR *,long);
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
