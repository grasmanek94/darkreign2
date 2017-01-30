/*
  ImageMagick Application Programming Interface declarations.

  Api.h depends on a number of ANSI C headers as follows:

      #include <stdio.h>
      #include <time.h>
      #include <sys/type.h>
      #include <magick/api.h>

*/

#ifndef _MAGICK_API_H
#define _MAGICK_API_H

#if defined(__cplusplus) || defined(c_plusplus)
#define class  c_class
#endif

#if defined(WIN32) || defined(__CYGWIN__)
//#define Export  __declspec(dllexport)
#define Export
#if defined(_VISUALC_)
#pragma warning(disable : 4018)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)
#endif
#else
# define Export
#endif

#define MaxTextExtent  1664

#if !defined(vms) && !defined(macintosh) && !defined(WIN32)
#if defined(HAVE_CONFIG_H)
#include "magick/config.h"
#endif
#include "magick/classify.h"
#include "magick/image.h"
#include "magick/gems.h"
#include "magick/quantize.h"
#include "magick/error.h"
#include "magick/monitor.h"
#include "magick/compress.h"
#include "magick/utility.h"
#include "magick/blob.h"
#include "magick/cache.h"
#include "magick/cache_io.h"
#include "magick/memory.h"
#include "magick/delegates.h"
#include "magick/timer.h"
#include "magick/version.h"
#else
#include "classify.h"
#include "image.h"
#include "gems.h"
#include "quantize.h"
#include "error.h"
#include "monitor.h"
#include "compress.h"
#include "utility.h"
#include "blob.h"
#include "cache.h"
#include "cache_io.h"
#include "memory.h"
#include "delegates.h"
#include "timer.h"
#include "version.h"
#endif

#endif
