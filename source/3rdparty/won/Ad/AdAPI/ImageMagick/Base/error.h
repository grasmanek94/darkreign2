/*
  ImageMagick Error Methods.
*/
#ifndef _ERROR_H
#define _ERROR_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Error define definitions.
*/
#if defined(sun) && !defined(SVR4)
#if !defined(strerror)
#define strerror(n) \
  (((n) >= 0 && (n) < sys_nerr) ? sys_errlist[n] : "unknown error")

extern char
  *sys_errlist[];

extern int
  sys_nerr;
#endif
#endif

/*
  Error typedef declarations.
*/
typedef void
  (*ErrorHandler)(const ErrorType,const char *,const char *);

typedef void
  (*WarningHandler)(const WarningType,const char *,const char *);

/*
  Error declarations.
*/
extern Export ErrorHandler
  SetErrorHandler(ErrorHandler);

extern Export void
  MagickError(const ErrorType,const char *,const char *),
  MagickWarning(const WarningType,const char *,const char *);

extern Export WarningHandler
  SetWarningHandler(WarningHandler);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
