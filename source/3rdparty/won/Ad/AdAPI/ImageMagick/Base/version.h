/*
  ImageMagick version and copyright.
*/
#ifndef _VERSION_H
#define _VERSION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define MagickCopyright  "Copyright (C) 2000 ImageMagick Studio"
#define MagickLibVersion  0x0511
#if defined(QuantumLeap)
#define MagickVersion  \
  "@(#)ImageMagick 5.1.1 00/02/01 Q:16 cristy@mystic.es.dupont.com"
#else
#define MagickVersion  \
  "@(#)ImageMagick 5.1.1 00/02/01 Q:8 cristy@mystic.es.dupont.com"
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
