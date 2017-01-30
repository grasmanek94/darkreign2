/*
  ImageMagick defines.
*/
#ifndef _DEFINES_H
#define _DEFINES_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Review these platform specific definitions.
*/
#if defined(sgi)
#if !defined(RGBColorDatabase)
#define RGBColorDatabase  "/usr/lib/X11/rgb.txt"
#endif
#if !defined(ApplicationDefaults)
#define ApplicationDefaults  "/usr/lib/X11/app-defaults/"
#endif
#endif

#if !defined(vms) && !defined(macintosh) && !defined(WIN32)
#if !defined(ApplicationDefaults)
#define ApplicationDefaults  "/usr/X11R6/lib/X11/app-defaults/"
#endif
#if !defined(DelegatePath)
#define DelegatePath  "/usr/local/share/ImageMagick/"
#endif
#define DirectorySeparator  "/"
#define DirectoryListSeparator  ':'
#define EditorOptions  " -title \"Edit Image Comment\" -e vi"
#define Exit  exit
#define IsBasenameSeparator(c)  ((c) == '/')
#define IsGlob(text) \
  ((strchr(text,'*') != (char *) NULL) || \
   (strchr(text,'?') != (char *) NULL) || \
   (strchr(text,'{') != (char *) NULL) || \
   (strchr(text,'}') != (char *) NULL) || \
   (strchr(text,'[') != (char *) NULL) || \
   (strchr(text,']') != (char *) NULL))
#define PreferencesDefaults  "~/."
#define ProcessPendingEvents(text)
#define ReadCommandlLine(argc,argv)
#if !defined(RGBColorDatabase)
#define RGBColorDatabase  "/usr/X11R6/lib/X11/rgb.txt"
#endif
#define SetNotifyHandlers
#define TemporaryTemplate  "magick"
#else

#if defined(vms)
#define ApplicationDefaults  "decw$system_defaults:"
#define DelegatePath  "sys$login:"
#define DirectorySeparator  ""
#define DirectoryListSeparator  ';'
#define EditorOptions  ""
#define Exit  exit
#define IsBasenameSeparator(c)  ((c) == ']')
#define IsGlob(text) \
  ((strchr(text,'*') != (char *) NULL) || \
   (strchr(text,'?') != (char *) NULL) || \
   (strchr(text,'{') != (char *) NULL) || \
   (strchr(text,'}') != (char *) NULL))
#define j1(x)  x
#define PreferencesDefaults  "decw$user_defaults:"
#define ProcessPendingEvents(text)
#define ReadCommandlLine(argc,argv)
#if !defined(RGBColorDatabase)
#define RGBColorDatabase  "sys$common:[sysmgr]decw$rgb.dat"
#endif
#define SetNotifyHandlers
#endif

#if defined(macintosh)
#define ApplicationDefaults  "/usr/lib/X11/app-defaults/"
#define DelegatePath  ""
#define DirectorySeparator  ":"
#define DirectoryListSeparator  ';'
#define EditorOptions ""
#define IsBasenameSeparator(c)  ((c) == ':')
#define IsGlob(text) \
  ((strchr(text,'*') != (char *) NULL) || \
   (strchr(text,'?') != (char *) NULL) || \
   (strchr(text,'{') != (char *) NULL) || \
   (strchr(text,'}') != (char *) NULL) || \
   (strchr(text,'[') != (char *) NULL) || \
   (strchr(text,']') != (char *) NULL))
#define j1(x)  x
#define PreferencesDefaults  "~/."
#define ReadCommandlLine(argc,argv)  argc=ccommand(argv); puts(MagickVersion);
#if !defined(RGBColorDatabase)
#define RGBColorDatabase  "../xlib/lib/X11/rgb.txt"
#endif
#define SetNotifyHandlers \
  SetErrorHandler(MACErrorHandler); \
  SetWarningHandler(MACWarningHandler)
#endif

#if defined(WIN32)
#define ApplicationDefaults  "c:\\ImageMagick\\"
#define DelegatePath  "c:\\ImageMagick\\"
#define DirectorySeparator  "\\"
#define DirectoryListSeparator  ';'
#define EditorOptions ""
#undef isatty
#define isatty(filedes)  1
#define IsBasenameSeparator(c)  (((c) == '/') || ((c) == '\\'))
#define IsGlob(text) \
  ((strchr(text,'*') != (char *) NULL) || \
   (strchr(text,'?') != (char *) NULL) || \
   (strchr(text,'{') != (char *) NULL) || \
   (strchr(text,'}') != (char *) NULL) || \
   (strchr(text,'[') != (char *) NULL) || \
   (strchr(text,']') != (char *) NULL))
#define j1(x)  x
#define PreferencesDefaults  "~/."
#define ProcessPendingEvents(text)
#define ReadCommandlLine(argc,argv)
#if !defined(RGBColorDatabase)
#define RGBColorDatabase  "c:\\ImageMagick\\rgb.txt"
#endif
#define SetNotifyHandlers \
  SetErrorHandler(NTErrorHandler); \
  SetWarningHandler(NTWarningHandler)
#undef sleep
#define sleep(seconds)  Sleep(seconds*1000)
#define HAVE_TIFFCONF_H
#endif

#endif

/*
  Define declarations.
*/
#define AbsoluteValue(x)  ((x) < 0 ? -(x) : (x))
#define CloseImage(image)  CloseBlob(image)
#define ColorMatch(color,target,distance) \
  (((distance) == 0) ? \
   (((color).red == (target).red) && \
    ((color).green == (target).green) && \
    ((color).blue == (target).blue)) : \
   ((unsigned long) ((((int) (color).red-(int) (target).red)* \
      ((int) (color).red-(int) (target).red))+ \
     (((int) (color).green-(int) (target).green)* \
      ((int) (color).green-(int) (target).green))+ \
     (((int) (color).blue-(int) (target).blue)* \
      ((int) (color).blue-(int) (target).blue))) <= \
      (unsigned long) (distance*distance)))
#define Extent(string)  ((int) strlen(string))
#define False  0
#define DegreesToRadians(x) ((x)*M_PI/180.0)
#define Intensity(color)  ((unsigned long) \
  ((0.299*(color).red+0.587*(color).green+0.1140000000000001*(color).blue)))
#define IsFaxImage(color)  \
  (IsMonochromeImage(image) && ((image)->columns <= 2560))
#define IsGray(color)  \
  (((color).red == (color).green) && ((color).green == (color).blue))
#define Max(x,y)  (((x) > (y)) ? (x) : (y))
#define Min(x,y)  (((x) < (y)) ? (x) : (y))
#if !defined(M_PI)
#define M_PI  3.14159265358979323846
#endif
#define OpenImage(image_info,image,type)  OpenBlob(image,image,type)
#define QuantumTick(i,span) \
  (((~((span)-i-1) & ((span)-i-2))+1) == ((span)-i-1))
#define RadiansToDegrees(x) (180.0*(x)/M_PI)
#define ReaderExit(warning,message,image) \
{ \
  MagickWarning(warning,message,image->filename); \
  DestroyImages(image); \
  return((Image *) NULL); \
}
#define RenderPostscriptText  "  Rendering postscript...  "
#define Swap(x,y) ((x)^=(y), (y)^=(x), (x)^=(y))
#define True  1
#define WriterExit(error,message,image) \
{ \
  MagickWarning(error,message,image->filename); \
  if (image_info->adjoin) \
    while (image->previous != (Image *) NULL) \
      image=image->previous; \
  CloseBlob(image); \
  return(False); \
}

/*
  Define system symbols if not already defined.
*/
#if !defined(STDIN_FILENO)
#define STDIN_FILENO  0x00
#endif

#if !defined(O_BINARY)
#define O_BINARY  0x00
#endif

#if !defined(MAP_FAILED)
#define MAP_FAILED      ((void *) -1)
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
