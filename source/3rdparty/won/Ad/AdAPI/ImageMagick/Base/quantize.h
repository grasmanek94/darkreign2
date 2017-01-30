/*
  Methods to Reduce the Number of Unique Colors in an Image.
*/
#ifndef _QUANTIZE_H
#define _QUANTIZE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*
  Define declarations.
*/
#define MaxTreeDepth  8
#define NodesInAList  1536

/*
  Typedef declarations.
*/
typedef struct _QuantizeInfo
{
  unsigned int
    number_colors,
    tree_depth,
    dither;

  ColorspaceType
    colorspace;

  unsigned int
    measure_error;
} QuantizeInfo;

/*
  Constant declarations.
*/
extern const long
  MaxColormapSize;

/*
  Quantization utilities methods.
*/
extern Export QuantizeInfo
  *CloneQuantizeInfo(const QuantizeInfo *);

extern Export unsigned int
  MapImage(Image *,Image *,const unsigned int),
  MapImages(Image *,Image *,const unsigned int),
  QuantizationError(Image *),
  QuantizeImage(const QuantizeInfo *,Image *),
  QuantizeImages(const QuantizeInfo *,Image *),
  QueryColorName(const PixelPacket *,char *),
  SegmentImage(Image *,const ColorspaceType,const unsigned int,const double,
    const double);

extern Export unsigned long
  GetNumberColors(Image *,FILE *);

extern Export void
  DestroyQuantizeInfo(QuantizeInfo *),
  GetQuantizeInfo(QuantizeInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
