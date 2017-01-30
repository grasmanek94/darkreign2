/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        V   V  IIIII  FFFFF  FFFFF                           %
%                        V   V    I    F      F                               %
%                        V   V    I    FFF    FFF                             %
%                         V V     I    F      F                               %
%                          V    IIIII  F      F                               %
%                                                                             %
%                                                                             %
%                    Read/Write ImageMagick Image Format.                     %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright (C) 2000 ImageMagick Studio, a non-profit organization dedicated %
%  to making software imaging solutions freely available.                     %
%                                                                             %
%  Permission is hereby granted, free of charge, to any person obtaining a    %
%  copy of this software and associated documentation files ("ImageMagick"),  %
%  to deal in ImageMagick without restriction, including without limitation   %
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   %
%  and/or sell copies of ImageMagick, and to permit persons to whom the       %
%  ImageMagick is furnished to do so, subject to the following conditions:    %
%                                                                             %
%  The above copyright notice and this permission notice shall be included in %
%  all copies or substantial portions of ImageMagick.                         %
%                                                                             %
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s V I F F                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsVIFF returns True if the image format type, identified by the
%  magick string, is VIFF.
%
%  The format of the ReadVIFFImage method is:
%
%      unsigned int IsVIFF(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsVIFF returns True if the image format type is VIFF.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsVIFF(const unsigned char *magick,
  const unsigned int length)
{
  if (length < 2)
    return(False);
  if (strncmp((char *) magick,"\253\1",2) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d V I F F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadVIFFImage reads a Khoros Visualization image file and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadVIFFImage method is:
%
%      Image *ReadVIFFImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image: Method ReadVIFFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o filename: Specifies the name of the image to read.
%
%
*/
Export Image *ReadVIFFImage(const ImageInfo *image_info)
{
#define VFF_CM_genericRGB  15
#define VFF_CM_ntscRGB  1
#define VFF_CM_NONE  0
#define VFF_DEP_DECORDER  0x4
#define VFF_DEP_NSORDER  0x8
#define VFF_DES_RAW  0
#define VFF_LOC_IMPLICIT  1
#define VFF_MAPTYP_NONE  0
#define VFF_MAPTYP_1_BYTE  1
#define VFF_MAPTYP_2_BYTE  2
#define VFF_MAPTYP_4_BYTE  4
#define VFF_MAPTYP_FLOAT  5
#define VFF_MAPTYP_DOUBLE  7
#define VFF_MS_NONE  0
#define VFF_MS_ONEPERBAND  1
#define VFF_MS_SHARED  3
#define VFF_TYP_BIT  0
#define VFF_TYP_1_BYTE  1
#define VFF_TYP_2_BYTE  2
#define VFF_TYP_4_BYTE  4
#define VFF_TYP_FLOAT  5
#define VFF_TYP_DOUBLE  9

  typedef struct _ViffHeader
  {
    unsigned char
      identifier,
      file_type,
      release,
      version,
      machine_dependency,
      reserve[3];

    char
      comment[512];

    unsigned long
      rows,
      columns,
      subrows;

    long
      x_offset,
      y_offset;

    float
      x_bits_per_pixel,
      y_bits_per_pixel;

    unsigned long
      location_type,
      location_dimension,
      number_of_images,
      number_data_bands,
      data_storage_type,
      data_encode_scheme,
      map_scheme,
      map_storage_type,
      map_rows,
      map_columns,
      map_subrows,
      map_enable,
      maps_per_cycle,
      color_space_model;
  } ViffHeader;

  double
    min_value,
    scale_factor,
    value;

  Image
    *image;

  int
    bit,
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    buffer[7],
    *viff_pixels;

  unsigned int
    bytes_per_pixel,
    status;

  unsigned long
    max_packets;

  ViffHeader
    viff_header;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Open image file.
  */
  status=OpenBlob(image_info,image,ReadBinaryType);
  if (status == False)
    ReaderExit(FileOpenWarning,"Unable to open file",image);
  /*
    Read VIFF header (1024 bytes).
  */
  status=ReadBlob(image,1,(char *) &viff_header.identifier);
  do
  {
    /*
      Verify VIFF identifier.
    */
    if ((status == False) || ((unsigned char) viff_header.identifier != 0xab))
      ReaderExit(CorruptImageWarning,"Not a VIFF raster",image);
    /*
      Initialize VIFF image.
    */
    (void) ReadBlob(image,7,(char *) buffer);
    viff_header.file_type=buffer[0];
    viff_header.release=buffer[1];
    viff_header.version=buffer[2];
    viff_header.machine_dependency=buffer[3];
    (void) ReadBlob(image,512,(char *) viff_header.comment);
    viff_header.comment[511]='\0';
    if (Extent(viff_header.comment) > 4)
      (void) CloneString(&image->comments,viff_header.comment);
    if ((viff_header.machine_dependency == VFF_DEP_DECORDER) ||
        (viff_header.machine_dependency == VFF_DEP_NSORDER))
      {
        viff_header.rows=LSBFirstReadLong(image);
        viff_header.columns=LSBFirstReadLong(image);
        viff_header.subrows=LSBFirstReadLong(image);
        viff_header.x_offset=LSBFirstReadLong(image);
        viff_header.y_offset=LSBFirstReadLong(image);
        viff_header.x_bits_per_pixel=(float) LSBFirstReadLong(image);
        viff_header.y_bits_per_pixel=(float) LSBFirstReadLong(image);
        viff_header.location_type=LSBFirstReadLong(image);
        viff_header.location_dimension=LSBFirstReadLong(image);
        viff_header.number_of_images=LSBFirstReadLong(image);
        viff_header.number_data_bands=LSBFirstReadLong(image);
        viff_header.data_storage_type=LSBFirstReadLong(image);
        viff_header.data_encode_scheme=LSBFirstReadLong(image);
        viff_header.map_scheme=LSBFirstReadLong(image);
        viff_header.map_storage_type=LSBFirstReadLong(image);
        viff_header.map_rows=LSBFirstReadLong(image);
        viff_header.map_columns=LSBFirstReadLong(image);
        viff_header.map_subrows=LSBFirstReadLong(image);
        viff_header.map_enable=LSBFirstReadLong(image);
        viff_header.maps_per_cycle=LSBFirstReadLong(image);
        viff_header.color_space_model=LSBFirstReadLong(image);
      }
    else
      {
        viff_header.rows=MSBFirstReadLong(image);
        viff_header.columns=MSBFirstReadLong(image);
        viff_header.subrows=MSBFirstReadLong(image);
        viff_header.x_offset=MSBFirstReadLong(image);
        viff_header.y_offset=MSBFirstReadLong(image);
        viff_header.x_bits_per_pixel=(float) MSBFirstReadLong(image);
        viff_header.y_bits_per_pixel=(float) MSBFirstReadLong(image);
        viff_header.location_type=MSBFirstReadLong(image);
        viff_header.location_dimension=MSBFirstReadLong(image);
        viff_header.number_of_images=MSBFirstReadLong(image);
        viff_header.number_data_bands=MSBFirstReadLong(image);
        viff_header.data_storage_type=MSBFirstReadLong(image);
        viff_header.data_encode_scheme=MSBFirstReadLong(image);
        viff_header.map_scheme=MSBFirstReadLong(image);
        viff_header.map_storage_type=MSBFirstReadLong(image);
        viff_header.map_rows=MSBFirstReadLong(image);
        viff_header.map_columns=MSBFirstReadLong(image);
        viff_header.map_subrows=MSBFirstReadLong(image);
        viff_header.map_enable=MSBFirstReadLong(image);
        viff_header.maps_per_cycle=MSBFirstReadLong(image);
        viff_header.color_space_model=MSBFirstReadLong(image);
      }
    for (i=0; i < 420; i++)
      (void) ReadByte(image);
    image->columns=(unsigned int) viff_header.rows;
    image->rows=(unsigned int) viff_header.columns;
    image->depth=viff_header.x_bits_per_pixel <= 8 ? 8 : QuantumDepth;
    /*
      Verify that we can read this VIFF image.
    */
    if ((viff_header.columns*viff_header.rows) == 0)
      ReaderExit(CorruptImageWarning,
        "Image column or row size is not supported",image);
    if ((viff_header.data_storage_type != VFF_TYP_BIT) &&
        (viff_header.data_storage_type != VFF_TYP_1_BYTE) &&
        (viff_header.data_storage_type != VFF_TYP_2_BYTE) &&
        (viff_header.data_storage_type != VFF_TYP_4_BYTE) &&
        (viff_header.data_storage_type != VFF_TYP_FLOAT) &&
        (viff_header.data_storage_type != VFF_TYP_DOUBLE))
      ReaderExit(CorruptImageWarning,"Data storage type is not supported",
        image);
    if (viff_header.data_encode_scheme != VFF_DES_RAW)
      ReaderExit(CorruptImageWarning,"Data encoding scheme is not supported",
        image);
    if ((viff_header.map_storage_type != VFF_MAPTYP_NONE) &&
        (viff_header.map_storage_type != VFF_MAPTYP_1_BYTE) &&
        (viff_header.map_storage_type != VFF_MAPTYP_2_BYTE) &&
        (viff_header.map_storage_type != VFF_MAPTYP_4_BYTE) &&
        (viff_header.map_storage_type != VFF_MAPTYP_FLOAT) &&
        (viff_header.map_storage_type != VFF_MAPTYP_DOUBLE))
      ReaderExit(CorruptImageWarning,"Map storage type is not supported",image);
    if ((viff_header.color_space_model != VFF_CM_NONE) &&
        (viff_header.color_space_model != VFF_CM_ntscRGB) &&
        (viff_header.color_space_model != VFF_CM_genericRGB))
      ReaderExit(CorruptImageWarning,"Colorspace model is not supported",image);
    if (viff_header.location_type != VFF_LOC_IMPLICIT)
      ReaderExit(CorruptImageWarning,"Location type is not supported",image);
    if (viff_header.number_of_images != 1)
      ReaderExit(CorruptImageWarning,"Number of images is not supported",image);
    if (viff_header.map_rows == 0)
      viff_header.map_scheme=VFF_MS_NONE;
    switch (viff_header.map_scheme)
    {
      case VFF_MS_NONE:
      {
        if (viff_header.number_data_bands < 3)
          {
            /*
              Create linear color ramp.
            */
            if (viff_header.data_storage_type == VFF_TYP_BIT)
              image->colors=2;
            else
              image->colors=1 << (viff_header.number_data_bands*QuantumDepth);
            image->colormap=(PixelPacket *)
              AllocateMemory(image->colors*sizeof(PixelPacket));
            if (image->colormap == (PixelPacket *) NULL)
              ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
            for (i=0; i < (int) image->colors; i++)
            {
              image->colormap[i].red=(MaxRGB*i)/(image->colors-1);
              image->colormap[i].green=(MaxRGB*i)/(image->colors-1);
              image->colormap[i].blue=(MaxRGB*i)/(image->colors-1);
            }
          }
        break;
      }
      case VFF_MS_ONEPERBAND:
      case VFF_MS_SHARED:
      {
        unsigned char
          *viff_colormap;

        /*
          Allocate VIFF colormap.
        */
        switch (viff_header.map_storage_type)
        {
          case VFF_MAPTYP_1_BYTE: bytes_per_pixel=1; break;
          case VFF_MAPTYP_2_BYTE: bytes_per_pixel=2; break;
          case VFF_MAPTYP_4_BYTE: bytes_per_pixel=4; break;
          case VFF_MAPTYP_FLOAT: bytes_per_pixel=4; break;
          case VFF_MAPTYP_DOUBLE: bytes_per_pixel=8; break;
          default: bytes_per_pixel=1; break;
        }
        image->colors=(unsigned int) viff_header.map_columns;
        image->colormap=(PixelPacket *)
          AllocateMemory(image->colors*sizeof(PixelPacket));
        viff_colormap=(unsigned char *) AllocateMemory(bytes_per_pixel*
          image->colors*viff_header.map_rows*sizeof(unsigned char));
        if ((image->colormap == (PixelPacket *) NULL) ||
            (viff_colormap == (unsigned char *) NULL))
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        /*
          Read VIFF raster colormap.
        */
        (void) ReadBlob(image,bytes_per_pixel*
          image->colors*viff_header.map_rows,(char *) viff_colormap);
        if ((viff_header.machine_dependency == VFF_DEP_DECORDER) ||
            (viff_header.machine_dependency == VFF_DEP_NSORDER))
          switch (viff_header.map_storage_type)
          {
            case VFF_MAPTYP_2_BYTE:
            {
              MSBFirstOrderShort((char *) viff_colormap,(unsigned int)
                (bytes_per_pixel*image->colors*viff_header.map_rows));
              break;
            }
            case VFF_MAPTYP_4_BYTE:
            case VFF_MAPTYP_FLOAT:
            {
              MSBFirstOrderLong((char *) viff_colormap,(unsigned int)
                (bytes_per_pixel*image->colors*viff_header.map_rows));
              break;
            }
            default: break;
          }
        for (i=0; i < (int) (viff_header.map_rows*image->colors); i++)
        {
          switch (viff_header.map_storage_type)
          {
            case VFF_MAPTYP_2_BYTE: value=((short *) viff_colormap)[i]; break;
            case VFF_MAPTYP_4_BYTE: value=((int *) viff_colormap)[i]; break;
            case VFF_MAPTYP_FLOAT: value=((float *) viff_colormap)[i]; break;
            case VFF_MAPTYP_DOUBLE: value=((double *) viff_colormap)[i]; break;
            default: value=viff_colormap[i]; break;
          }
          if (i < (int) image->colors)
            {
              image->colormap[i].red=UpScale((unsigned int) value);
              image->colormap[i].green=UpScale((unsigned int) value);
              image->colormap[i].blue=UpScale((unsigned int) value);
            }
          else
            if (i < (int) (2*image->colors))
              image->colormap[i % image->colors].green=
                UpScale((unsigned int) value);
            else
              if (i < (int) (3*image->colors))
                image->colormap[i % image->colors].blue=
                  UpScale((unsigned int) value);
        }
        FreeMemory(viff_colormap);
        break;
      }
      default:
        ReaderExit(CorruptImageWarning,"Colormap type is not supported",image);
    }
    /*
      Allocate VIFF pixels.
    */
    switch (viff_header.data_storage_type)
    {
      case VFF_TYP_2_BYTE: bytes_per_pixel=2; break;
      case VFF_TYP_4_BYTE: bytes_per_pixel=4; break;
      case VFF_TYP_FLOAT: bytes_per_pixel=4; break;
      case VFF_TYP_DOUBLE: bytes_per_pixel=8; break;
      default: bytes_per_pixel=1; break;
    }
    if (viff_header.data_storage_type == VFF_TYP_BIT)
      max_packets=((viff_header.columns+7) >> 3)*viff_header.rows;
    else
      max_packets=
        viff_header.columns*viff_header.rows*viff_header.number_data_bands;
    viff_pixels=(unsigned char *)
      AllocateMemory(bytes_per_pixel*max_packets*sizeof(Quantum));
    if (viff_pixels == (unsigned char *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    (void) ReadBlob(image,bytes_per_pixel*max_packets,(char *) viff_pixels);
    if ((viff_header.machine_dependency == VFF_DEP_DECORDER) ||
        (viff_header.machine_dependency == VFF_DEP_NSORDER))
      switch (viff_header.data_storage_type)
      {
        case VFF_TYP_2_BYTE:
        {
          MSBFirstOrderShort((char *) viff_pixels,(unsigned int)
            (bytes_per_pixel*max_packets));
          break;
        }
        case VFF_TYP_4_BYTE:
        case VFF_TYP_FLOAT:
        {
          MSBFirstOrderLong((char *) viff_pixels,(unsigned int)
            (bytes_per_pixel*max_packets));
          break;
        }
        default: break;
      }
    min_value=0.0;
    scale_factor=1.0;
    if (viff_header.map_scheme == VFF_MS_NONE)
      {
        double
          max_value;

        /*
          Determine scale factor.
        */
        switch (viff_header.data_storage_type)
        {
          case VFF_TYP_2_BYTE: value=((short *) viff_pixels)[0]; break;
          case VFF_TYP_4_BYTE: value=((int *) viff_pixels)[0]; break;
          case VFF_TYP_FLOAT: value=((float *) viff_pixels)[0]; break;
          case VFF_TYP_DOUBLE: value=((double *) viff_pixels)[0]; break;
          default: value=viff_pixels[0]; break;
        }
        max_value=value;
        min_value=value;
        for (i=0; i < (int) max_packets; i++)
        {
          switch (viff_header.data_storage_type)
          {
            case VFF_TYP_2_BYTE: value=((short *) viff_pixels)[i]; break;
            case VFF_TYP_4_BYTE: value=((int *) viff_pixels)[i]; break;
            case VFF_TYP_FLOAT: value=((float *) viff_pixels)[i]; break;
            case VFF_TYP_DOUBLE: value=((double *) viff_pixels)[i]; break;
            default: value=viff_pixels[i]; break;
          }
          if (value > max_value)
            max_value=value;
          else
            if (value < min_value)
              min_value=value;
        }
        if ((min_value == 0) && (max_value == 0))
          scale_factor=0;
        else
          if (min_value == max_value)
            {
              scale_factor=(double) MaxRGB/min_value;
              min_value=0;
            }
          else
            scale_factor=(double) MaxRGB/(max_value-min_value);
        if ((max_value-min_value) > 1.0)
          {
            min_value=0.0;
            scale_factor=1.0;
          }
      }
    /*
      Convert pixels to Quantum size.
    */
    p=(unsigned char *) viff_pixels;
    for (i=0; i < (int) max_packets; i++)
    {
      switch (viff_header.data_storage_type)
      {
        case VFF_TYP_2_BYTE: value=((short *) viff_pixels)[i]; break;
        case VFF_TYP_4_BYTE: value=((int *) viff_pixels)[i]; break;
        case VFF_TYP_FLOAT: value=((float *) viff_pixels)[i]; break;
        case VFF_TYP_DOUBLE: value=((double *) viff_pixels)[i]; break;
        default: value=viff_pixels[i]; break;
      }
      if (viff_header.map_scheme == VFF_MS_NONE)
        {
          value=(value-min_value)*scale_factor;
          if (value > MaxRGB)
            value=MaxRGB;
          else
            if (value < 0)
              value=0;
        }
      *p=(Quantum) value;
      p++;
    }
    /*
      Initialize image structure.
    */
    image->matte=(viff_header.number_data_bands == 4);
    image->class=
      (viff_header.number_data_bands < 3 ? PseudoClass : DirectClass);
    image->columns=(unsigned int) viff_header.rows;
    image->rows=(unsigned int) viff_header.columns;
    if (image_info->ping)
      {
        CloseBlob(image);
        return(image);
      }
    /*
      Convert VIFF raster image to pixel packets.
    */
    p=(unsigned char *) viff_pixels;
    if (viff_header.data_storage_type == VFF_TYP_BIT)
      {
        unsigned int
          polarity;

        /*
          Convert bitmap scanline to runlength-encoded color packets.
        */
        polarity=0;
        if (image->colors >= 2)
          polarity=Intensity(image->colormap[0]) <
            Intensity(image->colormap[1]);
        for (y=0; y < (int) image->rows; y++)
        {
          if (!SetPixelCache(image,0,y,image->columns,1))
            break;
          for (x=0; x < (int) (image->columns-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
              image->indexes[x+bit]=
                ((*p) & (0x01 << bit) ? (int) !polarity : (int) polarity);
            p++;
          }
          if ((image->columns % 8) != 0)
            {
              for (bit=0; bit < (int) (image->columns % 8); bit++)
                image->indexes[x+bit]=
                  ((*p) & (0x01 << bit) ? (int) !polarity : (int) polarity);
              p++;
            }
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,y,image->rows);
        }
      }
    else
      if (image->class == PseudoClass)
        for (y=0; y < (int) image->rows; y++)
        {
          if (!SetPixelCache(image,0,y,image->columns,1))
            break;
          for (x=0; x < (int) image->columns; x++)
            image->indexes[x]=(*p++);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,y,image->rows);
        }
      else
        {
          unsigned long
            offset;

          /*
            Convert DirectColor scanline to runlength-encoded color packets.
          */
          offset=image->columns*image->rows;
          for (y=0; y < (int) image->rows; y++)
          {
            q=SetPixelCache(image,0,y,image->columns,1);
            if (q == (PixelPacket *) NULL)
              break;
            for (x=0; x < (int) image->columns; x++)
            {
              q->red=UpScale(*p);
              q->green=UpScale(*(p+offset));
              q->blue=UpScale(*(p+offset*2));
              if (image->colors != 0)
                {
                  q->red=image->colormap[q->red].red;
                  q->green=image->colormap[q->green].green;
                  q->blue=image->colormap[q->blue].blue;
                }
              q->opacity=(Quantum) (image->matte ? (*(p+offset*3)) : 0);
              p++;
              q++;
            }
            if (!SyncPixelCache(image))
              break;
            if (image->previous == (Image *) NULL)
              if (QuantumTick(y,image->rows))
                ProgressMonitor(LoadImageText,y,image->rows);
          }
        }
    FreeMemory(viff_pixels);
    if (image->class == PseudoClass)
      SyncImage(image);
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    status=ReadBlob(image,1,(char *) &viff_header.identifier);
    if ((status == True) && (viff_header.identifier == 0xab))
      {
        /*
          Allocate next image structure.
        */
        AllocateNextImage(image_info,image);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        image=image->next;
        ProgressMonitor(LoadImagesText,TellBlob(image),image->filesize);
      }
  } while ((status == True) && (viff_header.identifier == 0xab));
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e V I F F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteVIFFImage writes an image to a file in the VIFF image format.
%
%  The format of the WriteVIFFImage method is:
%
%      unsigned int WriteVIFFImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteVIFFImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteVIFFImage(const ImageInfo *image_info,Image *image)
{
#define VFF_CM_genericRGB  15
#define VFF_CM_NONE  0
#define VFF_DEP_IEEEORDER  0x2
#define VFF_DES_RAW  0
#define VFF_LOC_IMPLICIT  1
#define VFF_MAPTYP_NONE  0
#define VFF_MAPTYP_1_BYTE  1
#define VFF_MS_NONE  0
#define VFF_MS_ONEPERBAND  1
#define VFF_TYP_BIT  0
#define VFF_TYP_1_BYTE  1

  typedef struct _ViffHeader
  {
    char
      identifier,
      file_type,
      release,
      version,
      machine_dependency,
      reserve[3],
      comment[512];

    unsigned long
      rows,
      columns,
      subrows;

    long
      x_offset,
      y_offset;

    unsigned int
      x_bits_per_pixel,
      y_bits_per_pixel;

    unsigned long
      location_type,
      location_dimension,
      number_of_images,
      number_data_bands,
      data_storage_type,
      data_encode_scheme,
      map_scheme,
      map_storage_type,
      map_rows,
      map_columns,
      map_subrows,
      map_enable,
      maps_per_cycle,
      color_space_model;
  } ViffHeader;

  int
    y;

  register int
    i,
    x;

  register PixelPacket
    *p;

  register unsigned char
    *q;

  unsigned char
    buffer[8],
    *viff_pixels;

  unsigned int
    scene,
    status;

  unsigned long
    packets;

  ViffHeader
    viff_header;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  scene=0;
  do
  {
    /*
      Initialize VIFF image structure.
    */
    TransformRGBImage(image,RGBColorspace);
    viff_header.identifier=(char) 0xab;
    viff_header.file_type=1;
    viff_header.release=1;
    viff_header.version=3;
    viff_header.machine_dependency=VFF_DEP_IEEEORDER;  /* IEEE byte ordering */
    *viff_header.comment='\0';
    if (image->comments != (char *) NULL)
      {
        (void) strncpy(viff_header.comment,image->comments,
          Min(Extent(image->comments),511));
        viff_header.comment[Min(Extent(image->comments),511)]='\0';
      }
    viff_header.rows=image->columns;
    viff_header.columns=image->rows;
    viff_header.subrows=0;
    viff_header.x_offset=(~0);
    viff_header.y_offset=(~0);
    viff_header.x_bits_per_pixel=0;
    viff_header.y_bits_per_pixel=0;
    viff_header.location_type=VFF_LOC_IMPLICIT;
    viff_header.location_dimension=0;
    viff_header.number_of_images=1;
    viff_header.data_encode_scheme=VFF_DES_RAW;
    viff_header.map_scheme=VFF_MS_NONE;
    viff_header.map_storage_type=VFF_MAPTYP_NONE;
    viff_header.map_rows=0;
    viff_header.map_columns=0;
    viff_header.map_subrows=0;
    viff_header.map_enable=1;  /* no colormap */
    viff_header.maps_per_cycle=0;
    if (!IsPseudoClass(image) && !IsGrayImage(image))
      {
        /*
          Full color VIFF raster.
        */
        viff_header.number_data_bands=image->matte ? 4 : 3;
        viff_header.color_space_model=VFF_CM_genericRGB;
        viff_header.data_storage_type=VFF_TYP_1_BYTE;
        packets=image->columns*image->rows*viff_header.number_data_bands;
      }
    else
      {
        viff_header.number_data_bands=1;
        viff_header.color_space_model=VFF_CM_NONE;
        viff_header.data_storage_type=VFF_TYP_1_BYTE;
        packets=image->columns*image->rows;
        if (!IsGrayImage(image))
          {
            /*
              Colormapped VIFF raster.
            */
            viff_header.map_scheme=VFF_MS_ONEPERBAND;
            viff_header.map_storage_type=VFF_MAPTYP_1_BYTE;
            viff_header.map_rows=3;
            viff_header.map_columns=image->colors;
          }
        else
          if (image->colors == 2)
            {
              /*
                Monochrome VIFF raster.
              */
              viff_header.data_storage_type=VFF_TYP_BIT;
              packets=((image->columns+7) >> 3)*image->rows;
            }
      }
    /*
      Write VIFF image header (pad to 1024 bytes).
    */
    buffer[0]=viff_header.identifier;
    buffer[1]=viff_header.file_type;
    buffer[2]=viff_header.release;
    buffer[3]=viff_header.version;
    buffer[4]=viff_header.machine_dependency;
    buffer[5]=viff_header.reserve[0];
    buffer[6]=viff_header.reserve[1];
    buffer[7]=viff_header.reserve[2];
    (void) WriteBlob(image,8,(char *) buffer);
    (void) WriteBlob(image,512,(char *) viff_header.comment);
    MSBFirstWriteLong(image,viff_header.rows);
    MSBFirstWriteLong(image,viff_header.columns);
    MSBFirstWriteLong(image,viff_header.subrows);
    MSBFirstWriteLong(image,(unsigned long) viff_header.x_offset);
    MSBFirstWriteLong(image,(unsigned long) viff_header.y_offset);
    viff_header.x_bits_per_pixel=(63 << 24) | (128 << 16);
    MSBFirstWriteLong(image,(unsigned long) viff_header.x_bits_per_pixel);
    viff_header.y_bits_per_pixel=(63 << 24) | (128 << 16);
    MSBFirstWriteLong(image,(unsigned long) viff_header.y_bits_per_pixel);
    MSBFirstWriteLong(image,viff_header.location_type);
    MSBFirstWriteLong(image,viff_header.location_dimension);
    MSBFirstWriteLong(image,viff_header.number_of_images);
    MSBFirstWriteLong(image,viff_header.number_data_bands);
    MSBFirstWriteLong(image,viff_header.data_storage_type);
    MSBFirstWriteLong(image,viff_header.data_encode_scheme);
    MSBFirstWriteLong(image,viff_header.map_scheme);
    MSBFirstWriteLong(image,viff_header.map_storage_type);
    MSBFirstWriteLong(image,viff_header.map_rows);
    MSBFirstWriteLong(image,viff_header.map_columns);
    MSBFirstWriteLong(image,viff_header.map_subrows);
    MSBFirstWriteLong(image,viff_header.map_enable);
    MSBFirstWriteLong(image,viff_header.maps_per_cycle);
    MSBFirstWriteLong(image,viff_header.color_space_model);
    for (i=0; i < 420; i++)
      (void) WriteByte(image,'\0');
    /*
      Convert MIFF to VIFF raster pixels.
    */
    viff_pixels=(unsigned char *) AllocateMemory(packets*sizeof(unsigned char));
    if (viff_pixels == (unsigned char *) NULL)
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    q=viff_pixels;
    if (!IsPseudoClass(image) && !IsGrayImage(image))
      {
        unsigned long
          offset;

        /*
          Convert DirectClass packet to VIFF RGB pixel.
        */
        offset=image->columns*image->rows;
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            *q=DownScale(p->red);
            *(q+offset)=DownScale(p->green);
            *(q+offset*2)=DownScale(p->blue);
            if (image->matte)
              *(q+offset*3)=DownScale(p->opacity);
            p++;
            q++;
          }
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
        }
      }
    else
      if (!IsGrayImage(image))
        {
          unsigned char
            *viff_colormap;

          /*
            Dump colormap to file.
          */
          viff_colormap=(unsigned char *)
            AllocateMemory(image->colors*3*sizeof(unsigned char));
          if (viff_colormap == (unsigned char *) NULL)
            WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
          q=viff_colormap;
          for (i=0; i < (int) image->colors; i++)
            *q++=DownScale(image->colormap[i].red);
          for (i=0; i < (int) image->colors; i++)
            *q++=DownScale(image->colormap[i].green);
          for (i=0; i < (int) image->colors; i++)
            *q++=DownScale(image->colormap[i].blue);
          (void) WriteBlob(image,3*image->colors,(char *) viff_colormap);
          FreeMemory(viff_colormap);
          /*
            Convert PseudoClass packet to VIFF colormapped pixels.
          */
          q=viff_pixels;
          for (y=0; y < (int) image->rows; y++)
          {
            if (!GetPixelCache(image,0,y,image->columns,1))
              break;
            for (x=0; x < (int) image->columns; x++)
              *q++=image->indexes[x];
            if (image->previous == (Image *) NULL)
              if (QuantumTick(y,image->rows))
                ProgressMonitor(SaveImageText,y,image->rows);
          }
        }
      else
        if (image->colors == 2)
          {
            int
              x,
              y;

            register unsigned char
              bit,
              byte,
              polarity;

            /*
              Convert PseudoClass image to a VIFF monochrome image.
            */
            polarity=Intensity(image->colormap[0]) > (MaxRGB >> 1);
            if (image->colors == 2)
              polarity=
                Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
            for (y=0; y < (int) image->rows; y++)
            {
              if (!GetPixelCache(image,0,y,image->columns,1))
                break;
              bit=0;
              byte=0;
              for (x=0; x < (int) image->columns; x++)
              {
                byte>>=1;
                if (image->indexes[x] == polarity)
                  byte|=0x80;
                bit++;
                if (bit == 8)
                  {
                    *q++=byte;
                    bit=0;
                    byte=0;
                  }
              }
              if (bit != 0)
                *q++=byte >> (8-bit);
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  ProgressMonitor(SaveImageText,y,image->rows);
            }
          }
        else
          {
            /*
              Convert PseudoClass packet to VIFF grayscale pixel.
            */
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                *q++=Intensity(*p);
                p++;
              }
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  ProgressMonitor(SaveImageText,y,image->rows);
            }
          }
    (void) WriteBlob(image,packets,(char *) viff_pixels);
    FreeMemory(viff_pixels);
    if (image->next == (Image *) NULL)
      break;
    image=GetNextImage(image);
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  CloseBlob(image);
  return(True);
}
