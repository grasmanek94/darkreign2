/*
  ImageMagick Image Methods.
*/
#ifndef _IMAGE_H
#define _IMAGE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(QuantumLeap)
/*
  Color quantum is [0..65535].
*/
#define DownScale(quantum)  (((unsigned long) (quantum)) >> 8)
#define HexColorFormat "#%04x%04x%04x"
#define MaxRGB  65535L
#define QuantumDepth  16
#define UpScale(quantum)  (257*((unsigned long) (quantum)))
#define XDownScale(color)  ((unsigned long) (color))
#define XUpScale(color)  ((unsigned long) (color))

typedef unsigned short Quantum;
#else
/*
  Color quantum is [0..255].
*/
#define DownScale(quantum)  ((unsigned long) (quantum))
#define HexColorFormat "#%02x%02x%02x"
#define MaxRGB  255L
#define QuantumDepth  8
#define UpScale(quantum)  ((unsigned long) (quantum))
#define XDownScale(color)  (((unsigned long) (color)) >> 8)
#define XUpScale(color)  (257*((unsigned long) (color)))

typedef unsigned char Quantum;
#endif
#define Opaque  MaxRGB
#define Transparent  0

/*
  3D effects.
*/
#define AccentuateModulate  UpScale(80)
#define HighlightModulate  UpScale(125)
#define ShadowModulate  UpScale(135)
#define DepthModulate  UpScale(185)
#define TroughModulate  UpScale(110)

/*
  Typedef declarations.
*/
typedef struct _BlobInfo
{
  unsigned int
    mapped;

  char
    *data;

  off_t
    offset;

  size_t
    length,
    extent,
    quantum;
} BlobInfo;

typedef void* Cache;

typedef struct _ColorlistInfo
{
  char
    *name;

  unsigned char
    red,
    green,
    blue;
} ColorlistInfo;

typedef struct _FrameInfo
{
  int
    x,
    y;

  unsigned int
    width,
    height;

  int
    inner_bevel,
    outer_bevel;
} FrameInfo;

typedef unsigned short IndexPacket;

typedef struct _PixelPacket
{
#if defined(WORDS_BIGENDIAN)
  Quantum
    red,
    green,
    blue,
    opacity;
#else
#if defined(WIN32)
  Quantum
    blue,
    green,
    red,
    opacity;
#else
  Quantum
    opacity,
    red,
    green,
    blue;
#endif
#endif
} PixelPacket;

typedef struct _ImageInfo
{
  /*
    Blob member.
  */
  BlobInfo
    blob_info;

  /*
    File and image dimension members.
  */
  FILE
    *file;

  char
    filename[MaxTextExtent],
    magick[MaxTextExtent],
    unique[MaxTextExtent],
    zero[MaxTextExtent];

  unsigned int
    temporary,
    adjoin,
    subimage,
    subrange,
    depth,
    ping;

  char
    *size,
    *tile,
    *page;

  InterlaceType
    interlace;

  ResolutionType
    units;

  /*
    Compression members.
  */
  CompressionType
    compression;

  unsigned int
    quality;

  /*
    Annotation members.
  */
  char
    *server_name,
    *box,
    *font,
    *pen,
    *texture,
    *density;

  double
    pointsize;

  unsigned int
    linewidth,
    antialias;

  int
    fuzz;

  PixelPacket
    background_color,
    border_color,
    matte_color;

  /*
    Color reduction members.
  */
  unsigned int
    dither,
    monochrome;

  ColorspaceType
    colorspace;

  /*
    Animation members.
  */
  char
    *dispose,
    *delay,
    *iterations;

  unsigned int
    decode_all_MNG_objects,
    coalesce_frames,
    insert_backdrops;

  /*
    Miscellaneous members.
  */
  unsigned int
    verbose;

  PreviewType
    preview_type;

  char
    *view;

  long
    group;
} ImageInfo;

typedef struct _MontageInfo
{
  char
    filename[MaxTextExtent],
    *geometry,
    *tile,
    *title,
    *frame,
    *texture,
    *pen,
    *font;

  PixelPacket
    background_color,
    border_color,
    matte_color;

  double
    pointsize;

  unsigned int
    border_width,
    gravity,
    shadow;

  CompositeOperator
    compose;
} MontageInfo;

typedef struct _PointInfo
{
  double
    x,
    y,
    z;
} PointInfo;

typedef struct _ProfileInfo
{
  unsigned int
    length;

  unsigned char
    *info;
} ProfileInfo;

typedef struct _RectangleInfo
{
  unsigned int
    width,
    height;

  int
    x,
    y;
} RectangleInfo;

typedef struct _SegmentInfo
{
  double
    x1,
    y1,
    x2,
    y2;
} SegmentInfo;

typedef struct _Timer
{
  double
    start,
    stop,
    total;
} Timer;

typedef struct _TimerInfo
{
  Timer
    user,
    elapsed;

  TimerState
    state;
} TimerInfo;

typedef struct _ChromaticityInfo
{
  PointInfo
    red_primary,
    green_primary,
    blue_primary,
    white_point;
} ChromaticityInfo;

typedef struct _Image
{
  int
    exempt,
    status,
    temporary;

  char
    filename[MaxTextExtent];

  FILE
    *file;

  size_t
    filesize;

  int
    pipe;

  char
    magick[MaxTextExtent],
    *comments,
    *label;

  ClassType
#if defined(__cplusplus) || defined(c_plusplus)
    c_class;
#else
    class;
#endif

  unsigned int
    matte;

  CompressionType
    compression;

  unsigned int
    columns,
    rows,
    depth;

  int
    offset;

  RectangleInfo
    tile_info;

  InterlaceType
    interlace;

  unsigned int
    scene;

  char
    *montage,
    *directory;

  PixelPacket
    *colormap;

  unsigned int
    colors;

  ColorspaceType
    colorspace;

  RenderingIntent
    rendering_intent;

  double
    gamma;

  ChromaticityInfo
    chromaticity;

  ProfileInfo
    color_profile,
    iptc_profile;

  ResolutionType
    units;

  double
    x_resolution,
    y_resolution;

  char
    *signature;

  PixelPacket
    *pixels;

  unsigned short
    *indexes;

  PixelPacket
    background_color,
    border_color,
    matte_color;

  char
    *geometry;

  RectangleInfo
    page_info;

  unsigned int
    dispose,
    delay,
    iterations;

  int
    fuzz;

  FilterType
    filter;

  double
    blur;

  unsigned long
    total_colors;

  unsigned int
    mean_error_per_pixel;

  double
    normalized_mean_error,
    normalized_maximum_error;

  char
    magick_filename[MaxTextExtent];

  unsigned int
    magick_columns,
    magick_rows;

  int
    restart_animation_here,
    tainted;

  TimerInfo
    timer_info;

  unsigned int
    orphan;

  BlobInfo
    blob_info;

  Cache
    cache;

  RectangleInfo
    cache_info;

  struct _Image
    *previous,
    *list,
    *next;
} Image;

typedef struct _AnnotateInfo
{
  ImageInfo
    *image_info;

  unsigned int
    gravity;

  char
    *geometry,
    *text,
    *primitive,
    *font_name;

  double
    degrees;

  Image
    *tile;

  RectangleInfo
    bounds;
} AnnotateInfo;

typedef struct _MagickInfo
{
  char
    *tag;

  Image
    *(*decoder)(const ImageInfo *);

  unsigned int
    (*encoder)(const ImageInfo *,Image *),
    (*magick)(const unsigned char *,const unsigned int),
    adjoin,
    blob_support,
    raw;

  char
    *description;

  void
    *data;

  struct _MagickInfo
    *previous,
    *next;
} MagickInfo;

/*
  Image const declarations.
*/
extern const char
  *Alphabet,
  *BackgroundColor,
  *BorderColor,
  *DefaultPointSize,
  *DefaultTileFrame,
  *DefaultTileGeometry,
  *DefaultTileLabel,
  *ForegroundColor,
  *MatteColor,
  *LoadImageText,
  *LoadImagesText,
  *PSDensityGeometry,
  *PSPageGeometry,
  *ReadBinaryType,
  *ReadBinaryUnbufferedType,
  *SaveImageText,
  *SaveImagesText,
  *WriteBinaryType;

extern const ColorlistInfo
  XPMColorlist[235],
  XColorlist[757];

/*
  Image utilities methods.
*/
extern Export AnnotateInfo
  *CloneAnnotateInfo(const ImageInfo *,const AnnotateInfo *);

extern Export Image
  *AddNoiseImage(Image *,const NoiseType),
  *AllocateImage(const ImageInfo *),
  *AppendImages(Image *,const unsigned int),
  *AverageImages(Image *),
  *BlurImage(Image *,const double),
  *BorderImage(Image *,const RectangleInfo *),
  *ChopImage(Image *,const RectangleInfo *),
  *CloneImage(Image *,const unsigned int,const unsigned int,const unsigned int),
  *CoalesceImages(Image *),
  *ColorizeImage(Image *,const char *,const char *),
  *CreateImage(const unsigned int,const unsigned int,const char *,
    const StorageType,const void *),
  *CropImage(Image *,const RectangleInfo *),
  *DeconstructImages(Image *),
  *DespeckleImage(Image *),
  *EdgeImage(Image *,const double),
  *EmbossImage(Image *),
  *EnhanceImage(Image *),
  *FlipImage(Image *),
  *FlopImage(Image *),
  *FrameImage(Image *,const FrameInfo *),
  *GetNextImage(Image *),
  *ImplodeImage(Image *,const double),
  **ListToGroupImage(const Image *,unsigned int *),
  *MagnifyImage(Image *),
  *MedianFilterImage(Image *,const unsigned int),
  *MinifyImage(Image *),
  *MontageImages(const Image *,const MontageInfo *),
  *MorphImages(Image *,const unsigned int),
  *MosaicImages(Image *),
  *OilPaintImage(Image *,const unsigned int),
  *PingImage(const ImageInfo *),
  *Read8BIMImage(const ImageInfo *image_info),
  *ReadAVSImage(const ImageInfo *image_info),
  *ReadBMPImage(const ImageInfo *image_info),
  *ReadCMYKImage(const ImageInfo *image_info),
  *ReadDCMImage(const ImageInfo *image_info),
  *ReadDPSImage(const ImageInfo *image_info),
  *ReadFAXImage(const ImageInfo *image_info),
  *ReadFITSImage(const ImageInfo *image_info),
  *ReadFPXImage(const ImageInfo *image_info),
  *ReadGIFImage(const ImageInfo *image_info),
  *ReadGRADATIONImage(const ImageInfo *image_info),
  *ReadGRAYImage(const ImageInfo *image_info),
  *ReadHDFImage(const ImageInfo *image_info),
  *ReadHISTOGRAMImage(const ImageInfo *image_info),
  *ReadICCImage(const ImageInfo *image_info),
  *ReadICONImage(const ImageInfo *image_info),
  *ReadImage(ImageInfo *),
  *ReadImages(ImageInfo *),
  *ReadIPTCImage(const ImageInfo *image_info),
  *ReadJBIGImage(const ImageInfo *image_info),
  *ReadJPEGImage(const ImageInfo *image_info),
  *ReadLABELImage(const ImageInfo *image_info),
  *ReadLOGOImage(const ImageInfo *image_info),
  *ReadMAPImage(const ImageInfo *image_info),
  *ReadMIFFImage(const ImageInfo *image_info),
  *ReadMONOImage(const ImageInfo *image_info),
  *ReadMTVImage(const ImageInfo *image_info),
  *ReadNULLImage(const ImageInfo *image_info),
  *ReadPCDImage(const ImageInfo *image_info),
  *ReadPCLImage(const ImageInfo *image_info),
  *ReadPCXImage(const ImageInfo *image_info),
  *ReadPDFImage(const ImageInfo *image_info),
  *ReadPICTImage(const ImageInfo *image_info),
  *ReadPIXImage(const ImageInfo *image_info),
  *ReadPLASMAImage(const ImageInfo *image_info),
  *ReadPNGImage(const ImageInfo *image_info),
  *ReadPNMImage(const ImageInfo *image_info),
  *ReadPSDImage(const ImageInfo *image_info),
  *ReadPSImage(const ImageInfo *image_info),
  *ReadPWPImage(const ImageInfo *image_info),
  *ReadRGBImage(const ImageInfo *image_info),
  *ReadRLAImage(const ImageInfo *image_info),
  *ReadRLEImage(const ImageInfo *image_info),
  *ReadSCTImage(const ImageInfo *image_info),
  *ReadSFWImage(const ImageInfo *image_info),
  *ReadSGIImage(const ImageInfo *image_info),
  *ReadSTEGANOImage(const ImageInfo *image_info),
  *ReadSUNImage(const ImageInfo *image_info),
  *ReadTGAImage(const ImageInfo *image_info),
  *ReadTIFFImage(const ImageInfo *image_info),
  *ReadTILEImage(const ImageInfo *image_info),
  *ReadTIMImage(const ImageInfo *image_info),
  *ReadTTFImage(const ImageInfo *image_info),
  *ReadTXTImage(const ImageInfo *image_info),
  *ReadUILImage(const ImageInfo *image_info),
  *ReadUYVYImage(const ImageInfo *image_info),
  *ReadVICARImage(const ImageInfo *image_info),
  *ReadVIDImage(const ImageInfo *image_info),
  *ReadVIFFImage(const ImageInfo *image_info),
  *ReadWBMPImage(const ImageInfo *image_info),
  *ReadXImage(const ImageInfo *image_info),
  *ReadXBMImage(const ImageInfo *image_info),
  *ReadXCImage(const ImageInfo *image_info),
  *ReadXPMImage(const ImageInfo *image_info),
  *ReadXWDImage(const ImageInfo *image_info),
  *ReadYUVImage(const ImageInfo *image_info),
  *ReduceNoiseImage(Image *),
  *RollImage(Image *,const int,const int),
  *RotateImage(Image *,const double),
  *SampleImage(Image *,const unsigned int,const unsigned int),
  *ScaleImage(Image *,const unsigned int,const unsigned int),
  *ShadeImage(Image *,const unsigned int,double,double),
  *SharpenImage(Image *,const double),
  *ShearImage(Image *,const double,const double),
  *SpreadImage(Image *,const unsigned int),
  *SteganoImage(Image *,Image *),
  *StereoImage(Image *,Image *),
  *SwirlImage(Image *,double),
  *WaveImage(Image *,const double,const double),
  *ZoomImage(Image *,const unsigned int,const unsigned int);

extern Export ImageInfo
  *CloneImageInfo(const ImageInfo *);

extern Export ImageType
  GetImageType(Image *);

extern Export int
  ParseGeometry(const char *,int *,int *,unsigned int *,unsigned int *),
  ParseImageGeometry(const char *,int *,int *,unsigned int *,unsigned int *);

extern Export MagickInfo
  *GetMagickInfo(const char *),
  *RegisterMagickInfo(MagickInfo *),
  *SetMagickInfo(const char *);

extern Export unsigned int
  AnimateImages(const ImageInfo *image_info,Image *image),
  DisplayImages(const ImageInfo *image_info,Image *image),
  GetNumberScenes(const Image *),
  Is8BIM(const unsigned char *,const unsigned int),
  IsBMP(const unsigned char *,const unsigned int),
  IsDCM(const unsigned char *,const unsigned int),
  IsDCX(const unsigned char *,const unsigned int),
  IsEPT(const unsigned char *,const unsigned int),
  IsFAX(const unsigned char *,const unsigned int),
  IsFITS(const unsigned char *,const unsigned int),
  IsGIF(const unsigned char *,const unsigned int),
  IsGeometry(const char *),
  IsGrayImage(Image *),
  IsMatteImage(Image *),
  IsHDF(const unsigned char *,const unsigned int),
  IsHTML(const unsigned char *,const unsigned int),
  IsIPTC(const unsigned char *,const unsigned int),
  IsJPEG(const unsigned char *,const unsigned int),
  IsMIFF(const unsigned char *,const unsigned int),
  IsMNG(const unsigned char *,const unsigned int),
  IsMonochromeImage(Image *),
  IsPCD(const unsigned char *,const unsigned int),
  IsPCL(const unsigned char *,const unsigned int),
  IsPCX(const unsigned char *,const unsigned int),
  IsPDF(const unsigned char *,const unsigned int),
  IsPNG(const unsigned char *,const unsigned int),
  IsPNM(const unsigned char *,const unsigned int),
  IsPS(const unsigned char *,const unsigned int),
  IsPSD(const unsigned char *,const unsigned int),
  IsPWP(const unsigned char *,const unsigned int),
  IsPseudoClass(Image *),
  IsRLE(const unsigned char *,const unsigned int),
  IsSCT(const unsigned char *,const unsigned int),
  IsSFW(const unsigned char *,const unsigned int),
  IsSGI(const unsigned char *,const unsigned int),
  IsSUN(const unsigned char *,const unsigned int),
  IsSubimage(const char *,const unsigned int),
  IsTIFF(const unsigned char *,const unsigned int),
  IsTainted(const Image *),
  IsVICAR(const unsigned char *,const unsigned int),
  IsVIFF(const unsigned char *,const unsigned int),
  IsXBM(const unsigned char *,const unsigned int),
  IsXPM(const unsigned char *,const unsigned int),
  IsXWD(const unsigned char *,const unsigned int),
  PlasmaImage(Image *,const SegmentInfo *,int,int),
  QueryColorDatabase(const char *,PixelPacket *),
  UnregisterMagickInfo(const char *),
  Write8BIMImage(const ImageInfo *image_info,Image *image),
  WriteAVSImage(const ImageInfo *image_info,Image *image),
  WriteBMPImage(const ImageInfo *image_info,Image *image),
  WriteCMYKImage(const ImageInfo *image_info,Image *image),
  WriteEPTImage(const ImageInfo *image_info,Image *image),
  WriteFAXImage(const ImageInfo *image_info,Image *image),
  WriteFITSImage(const ImageInfo *image_info,Image *image),
  WriteFPXImage(const ImageInfo *image_info,Image *image),
  WriteGIFImage(const ImageInfo *image_info,Image *image),
  WriteGRAYImage(const ImageInfo *image_info,Image *image),
  WriteHDFImage(const ImageInfo *image_info,Image *image),
  WriteHISTOGRAMImage(const ImageInfo *image_info, Image *image),
  WriteHTMLImage(const ImageInfo *image_info,Image *image),
  WriteICCImage(const ImageInfo *image_info,Image *image),
  WriteIPTCImage(const ImageInfo *image_info,Image *image),
  WriteImage(const ImageInfo *,Image *),
  WriteJBIGImage(const ImageInfo *image_info,Image *image),
  WriteJPEGImage(const ImageInfo *image_info,Image *image),
  WriteLOGOImage(const ImageInfo *image_info,Image *image),
  WriteMAPImage(const ImageInfo *image_info,Image *image),
  WriteMATTEImage(const ImageInfo *image_info,Image *image),
  WriteMIFFImage(const ImageInfo *image_info,Image *image),
  WriteMONOImage(const ImageInfo *image_info,Image *image),
  WriteMTVImage(const ImageInfo *image_info,Image *image),
  WriteNTImage(const ImageInfo *,Image *),
  WritePCDImage(const ImageInfo *image_info,Image *image),
  WritePCLImage(const ImageInfo *image_info,Image *image),
  WritePCXImage(const ImageInfo *image_info,Image *image),
  WritePDFImage(const ImageInfo *image_info,Image *image),
  WritePICTImage(const ImageInfo *image_info,Image *image),
  WritePNGImage(const ImageInfo *image_info,Image *image),
  WritePNMImage(const ImageInfo *image_info,Image *image),
  WritePREVIEWImage(const ImageInfo *image_info,Image *image),
  WritePS2Image(const ImageInfo *image_info,Image *image),
  WritePS3Image(const ImageInfo *image_info,Image *image),
  WritePSDImage(const ImageInfo *image_info,Image *image),
  WritePSImage(const ImageInfo *image_info,Image *image),
  WriteRGBImage(const ImageInfo *image_info,Image *image),
  WriteSGIImage(const ImageInfo *image_info,Image *image),
  WriteSUNImage(const ImageInfo *image_info,Image *image),
  WriteTGAImage(const ImageInfo *image_info,Image *image),
  WriteTIFFImage(const ImageInfo *image_info,Image *image),
  WriteTXTImage(const ImageInfo *image_info,Image *image),
  WriteUILImage(const ImageInfo *image_info,Image *image),
  WriteUYVYImage(const ImageInfo *image_info,Image *image),
  WriteVICARImage(const ImageInfo *image_info,Image *image),
  WriteVIFFImage(const ImageInfo *image_info,Image *image),
  WriteWBMPImage(const ImageInfo *image_info,Image *image),
  WriteXBMImage(const ImageInfo *image_info,Image *image),
  WriteXImage(const ImageInfo *image_info,Image *image),
  WriteXPMImage(const ImageInfo *image_info,Image *image),
  WriteXWDImage(const ImageInfo *image_info,Image *image),
  WriteYUVImage(const ImageInfo *image_info,Image *image);

extern Export void
  AllocateNextImage(const ImageInfo *,Image *),
  AnnotateImage(Image *,const AnnotateInfo *),
  ColorFloodfillImage(Image *,const PixelPacket *,Image *,const int x,
    const int y,const PaintMethod),
  CommentImage(Image *,const char *),
  CompositeImage(Image *,const CompositeOperator,Image *,const int,const int),
  CompressColormap(Image *),
  ContrastImage(Image *,const unsigned int),
  CycleColormapImage(Image *,const int),
  DescribeImage(Image *,FILE *,const unsigned int),
  DestroyAnnotateInfo(AnnotateInfo *),
  DestroyImage(Image *),
  DestroyImageInfo(ImageInfo *),
  DestroyImages(Image *),
  DestroyMagickInfo(),
  DestroyMontageInfo(MontageInfo *),
  DrawImage(Image *,const AnnotateInfo *),
  EqualizeImage(Image *),
  GammaImage(Image *,const char *),
  GetAnnotateInfo(const ImageInfo *,AnnotateInfo *),
  GetImageInfo(ImageInfo *),
  GetMontageInfo(MontageInfo *),
  GetPageInfo(RectangleInfo *),
  GetPixels(Image *,const int,const int,const unsigned int,const unsigned int,
    const char *,const StorageType,void *),
  GetPixelPacket(PixelPacket *),
  LabelImage(Image *,const char *),
  LayerImage(Image *,const LayerType),
  ListMagickInfo(FILE *),
  MatteFloodfillImage(Image *,const PixelPacket *,const unsigned int,
    const int x,const int y,const PaintMethod),
  MatteImage(Image *,Quantum),
  ModulateImage(Image *,const char *),
  MogrifyImage(const ImageInfo *,const int,char **,Image **),
  MogrifyImages(const ImageInfo *,const int,char **,Image **),
  NegateImage(Image *,const unsigned int),
  NormalizeImage(Image *),
  OpaqueImage(Image *,const char *,const char *),
  RaiseImage(Image *,const RectangleInfo *,const int),
  RGBTransformImage(Image *,const ColorspaceType),
  SetImage(Image *),
  SetImageInfo(ImageInfo *,const unsigned int),
  SignatureImage(Image *),
  SolarizeImage(Image *,const double),
  SortColormapByIntensity(Image *),
  SyncImage(Image *),
  TextureImage(Image *,Image *),
  ThresholdImage(Image *,const double),
  TransformHSL(const Quantum,const Quantum,const Quantum,double *,double *,
    double *),
  TransformImage(Image **,const char *,const char *),
  TransformRGBImage(Image *,const ColorspaceType),
  TransparentImage(Image *,const char *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
