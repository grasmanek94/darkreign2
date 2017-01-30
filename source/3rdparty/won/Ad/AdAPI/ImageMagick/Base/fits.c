/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        FFFFF  IIIII  TTTTT  SSSSS                           %
%                        F        I      T    SS                              %
%                        FFF      I      T     SSS                            %
%                        F        I      T       SS                           %
%                        F      IIIII    T    SSSSS                           %
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
%   I s F I T S                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsFITS returns True if the image format type, identified by the
%  magick string, is FITS.
%
%  The format of the ReadFITSImage method is:
%
%      unsigned int IsFITS(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsFITS returns True if the image format type is FITS.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsFITS(const unsigned char *magick,
  const unsigned int length)
{
  if (length < 6)
    return(False);
  if (strncmp((char *) magick,"IT0",3) == 0)
    return(True);
  if (strncmp((char *) magick,"SIMPLE",6) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d F I T S I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadFITSImage reads a FITS image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadFITSImage method is:
%
%      Image *ReadFITSImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image: Method ReadFITSImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o filename: Specifies the name of the image to read.
%
%
*/
Export Image *ReadFITSImage(const ImageInfo *image_info)
{
  typedef struct _FITSHeader
  {
    unsigned int
      simple;

    int
      bits_per_pixel;

    unsigned int
      columns,
      rows,
      number_axes,
      number_scenes;

    double
      min_data,
      max_data,
      zero,
      scale;
  } FITSHeader;

  char
    long_quantum[8],
    keyword[MaxTextExtent],
    value[MaxTextExtent];

  double
    pixel,
    scale,
    scaled_pixel;

  FITSHeader
    fits_header;

  Image
    *image;

  IndexPacket
    index;

  int
    c,
    j,
    packet_size,
    y;

  long
    count,
    quantum;

  register int
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *fits_pixels;

  unsigned int
    scene,
    status,
    value_expected;

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
    Initialize image header.
  */
  fits_header.simple=False;
  fits_header.bits_per_pixel=8;
  fits_header.columns=1;
  fits_header.rows=1;
  fits_header.number_scenes=1;
  fits_header.min_data=0.0;
  fits_header.max_data=0.0;
  fits_header.zero=0.0;
  fits_header.scale=1.0;
  /*
    Decode image header.
  */
  c=ReadByte(image);
  count=1;
  if (c == EOF)
    {
      DestroyImage(image);
      return((Image *) NULL);
    }
  for ( ; ; )
  {
    if (!isalnum(c))
      {
        c=ReadByte(image);
        count++;
      }
    else
      {
        register char
          *p;

        /*
          Determine a keyword and its value.
        */
        p=keyword;
        do
        {
          if ((p-keyword) < (MaxTextExtent-1))
            *p++=(char) c;
          c=ReadByte(image);
          count++;
        } while (isalnum(c) || (c == '_'));
        *p='\0';
        if (Latin1Compare(keyword,"END") == 0)
          break;
        value_expected=False;
        while (isspace(c) || (c == '='))
        {
          if (c == '=')
            value_expected=True;
          c=ReadByte(image);
          count++;
        }
        if (value_expected == False)
          continue;
        p=value;
        while (isalnum(c) || (c == '-') || (c == '+') || (c == '.'))
        {
          if ((p-value) < (MaxTextExtent-1))
            *p++=(char) c;
          c=ReadByte(image);
          count++;
        }
        *p='\0';
        /*
          Assign a value to the specified keyword.
        */
        if (Latin1Compare(keyword,"SIMPLE") == 0)
          fits_header.simple=(*value == 'T') || (*value == 't');
        if (Latin1Compare(keyword,"BITPIX") == 0)
          fits_header.bits_per_pixel=(unsigned int) atoi(value);
        if (Latin1Compare(keyword,"NAXIS") == 0)
          fits_header.number_axes=(unsigned int) atoi(value);
        if (Latin1Compare(keyword,"NAXIS1") == 0)
          fits_header.columns=(unsigned int) atoi(value);
        if (Latin1Compare(keyword,"NAXIS2") == 0)
          fits_header.rows=(unsigned int) atoi(value);
        if (Latin1Compare(keyword,"NAXIS3") == 0)
          fits_header.number_scenes=(unsigned int) atoi(value);
        if (Latin1Compare(keyword,"DATAMAX") == 0)
          fits_header.max_data=atof(value);
        if (Latin1Compare(keyword,"DATAMIN") == 0)
          fits_header.min_data=atof(value);
        if (Latin1Compare(keyword,"BZERO") == 0)
          fits_header.zero=atof(value);
        if (Latin1Compare(keyword,"BSCALE") == 0)
          fits_header.scale=atof(value);
      }
    while (isspace(c))
    {
      c=ReadByte(image);
      count++;
    }
  }
  while (count > 2880)
    count-=2880;
  for ( ; count < 2880; count++)
    (void) ReadByte(image);
  /*
    Verify that required image information is defined.
  */
  if ((!fits_header.simple) || (fits_header.number_axes < 1) ||
      (fits_header.number_axes > 4) ||
      (fits_header.columns*fits_header.rows) == 0)
    ReaderExit(CorruptImageWarning,"image type not supported",image);
  for (scene=0; scene < fits_header.number_scenes; scene++)
  {
    /*
      Create linear colormap.
    */
    image->columns=fits_header.columns;
    image->rows=fits_header.rows;
    image->depth=fits_header.bits_per_pixel <= 8 ? 8 : QuantumDepth;
    image->class=PseudoClass;
    image->colors=MaxRGB+1;
    image->scene=scene;
    if (image_info->ping)
      {
        CloseBlob(image);
        return(image);
      }
    image->colormap=(PixelPacket *)
      AllocateMemory(image->colors*sizeof(PixelPacket));
    if (image->colormap == (PixelPacket *) NULL)
      ReaderExit(FileOpenWarning,"Unable to open file",image);
    for (i=0; i < (int) image->colors; i++)
    {
      image->colormap[i].red=((unsigned long) (MaxRGB*i)/(image->colors-1));
      image->colormap[i].green=((unsigned long) (MaxRGB*i)/(image->colors-1));
      image->colormap[i].blue=((unsigned long) (MaxRGB*i)/(image->colors-1));
    }
    /*
      Initialize image structure.
    */
    packet_size=fits_header.bits_per_pixel/8;
    if (packet_size < 0)
      packet_size=(-packet_size);
    fits_pixels=(unsigned char *) AllocateMemory(
      packet_size*image->columns*image->rows*sizeof(unsigned char));
    if (fits_pixels == (unsigned char *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    /*
      Convert FITS pixels to pixel packets.
    */
    status=ReadBlob(image,packet_size*image->columns*image->rows,fits_pixels);
    if (status == False)
      MagickWarning(CorruptImageWarning,"Insufficient image data in file",
        image->filename);
    if ((fits_header.min_data == 0.0) && (fits_header.max_data == 0.0))
      {
        /*
          Determine minimum and maximum intensity.
        */
        p=fits_pixels;
        long_quantum[0]=(*p);
        quantum=(*p++);
        for (j=0; j < (packet_size-1); j++)
        {
          long_quantum[j+1]=(*p);
          quantum=(quantum << 8) | (*p++);
        }
        pixel=(double) quantum;
        if (fits_header.bits_per_pixel == 16)
          if (pixel > 32767)
            pixel-=65536;
        if (fits_header.bits_per_pixel == -32)
          pixel=(double) (*((float *) &quantum));
        if (fits_header.bits_per_pixel == -64)
          pixel=(double) (*((double *) long_quantum));
        fits_header.min_data=pixel*fits_header.scale+fits_header.zero;
        fits_header.max_data=pixel*fits_header.scale+fits_header.zero;
        for (i=1; i < (int) (image->columns*image->rows); i++)
        {
          long_quantum[0]=(*p);
          quantum=(*p++);
          for (j=0; j < (packet_size-1); j++)
          {
            long_quantum[j+1]=(*p);
            quantum=(quantum << 8) | (*p++);
          }
          pixel=(double) quantum;
          if (fits_header.bits_per_pixel == 16)
            if (pixel > 32767)
              pixel-=65536;
          if (fits_header.bits_per_pixel == -32)
            pixel=(double) (*((float *) &quantum));
          if (fits_header.bits_per_pixel == -64)
            pixel=(double) (*((double *) long_quantum));
          scaled_pixel=pixel*fits_header.scale+fits_header.zero;
          if (scaled_pixel < fits_header.min_data)
            fits_header.min_data=scaled_pixel;
          if (scaled_pixel > fits_header.max_data)
            fits_header.max_data=scaled_pixel;
        }
      }
    /*
      Convert FITS pixels to pixel packets.
    */
    scale=1.0;
    if ((fits_header.max_data-fits_header.min_data) <= 1.0)
      scale=MaxRGB/(fits_header.max_data-fits_header.min_data);
    p=fits_pixels;
    for (y=image->rows-1; y >= 0; y--)
    {
      q=SetPixelCache(image,0,y,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) image->columns; x++)
      {
        long_quantum[0]=(*p);
        quantum=(*p++);
        for (j=0; j < (packet_size-1); j++)
        {
          long_quantum[j+1]=(*p);
          quantum=(quantum << 8) | (*p++);
        }
        pixel=(double) quantum;
        if (fits_header.bits_per_pixel == 16)
          if (pixel > 32767)
            pixel-=65536;
        if (fits_header.bits_per_pixel == -32)
          pixel=(double) (*((float *) &quantum));
        if (fits_header.bits_per_pixel == -64)
          pixel=(double) (*((double *) long_quantum));
        scaled_pixel=scale*
          (pixel*fits_header.scale-fits_header.min_data-fits_header.zero);
        if (scaled_pixel < 0)
          scaled_pixel=0;
        else
          if (scaled_pixel > MaxRGB)
            scaled_pixel=MaxRGB;
        index=(IndexPacket) (scaled_pixel+0.5);
        image->indexes[x]=index;
        *q++=image->colormap[index];
      }
      if (!SyncPixelCache(image))
        break;
      if (QuantumTick(y,image->rows))
        ProgressMonitor(LoadImageText,y,image->rows);
    }
    FreeMemory(fits_pixels);
    /*
      Proceed to next image.
    */
    if (EOFBlob(image))
      break;
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    if (scene < (fits_header.number_scenes-1))
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
  }
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
%   W r i t e F I T S I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteFITSImage writes a Flexible Image Transport System image to a
%  file as gray scale intensities [0..255].
%
%  The format of the WriteFITSImage method is:
%
%      unsigned int WriteFITSImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteFITSImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteFITSImage(const ImageInfo *image_info,Image *image)
{
  char
    buffer[81],
    *fits_header;

  int
    y;

  register int
    i;

  register PixelPacket
    *p;

  unsigned char
    *pixels;

  unsigned int
    packet_size,
    status;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  TransformRGBImage(image,RGBColorspace);
  /*
    Allocate image memory.
  */
  packet_size=image->depth > 8 ? 2 : 1;
  fits_header=(char *) AllocateMemory(2880*sizeof(unsigned char));
  pixels=(unsigned char *)
    AllocateMemory(packet_size*image->columns*sizeof(unsigned char));
  if ((fits_header == (char *) NULL) || (pixels == (unsigned char *) NULL))
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  /*
    Initialize image header.
  */
  for (i=0; i < 2880; i++)
    fits_header[i]=' ';
  (void) strcpy(buffer,"SIMPLE  =                    T");
  (void) strncpy(fits_header+0,buffer,Extent(buffer));
  FormatString(buffer,"BITPIX  =                    %d",image->depth);
  (void) strncpy(fits_header+80,buffer,Extent(buffer));
  (void) strcpy(buffer,"NAXIS   =                    2");
  (void) strncpy(fits_header+160,buffer,Extent(buffer));
  FormatString(buffer,"NAXIS1  =           %10u",image->columns);
  (void) strncpy(fits_header+240,buffer,Extent(buffer));
  FormatString(buffer,"NAXIS2  =           %10u",image->rows);
  (void) strncpy(fits_header+320,buffer,Extent(buffer));
  FormatString(buffer,"DATAMIN =           %10u",0);
  (void) strncpy(fits_header+400,buffer,Extent(buffer));
  FormatString(buffer,"DATAMAX =           %10u",MaxRGB);
  (void) strncpy(fits_header+480,buffer,Extent(buffer));
  (void) strcpy(buffer,"HISTORY Created by ImageMagick.");
  (void) strncpy(fits_header+560,buffer,Extent(buffer));
  (void) strcpy(buffer,"END");
  (void) strncpy(fits_header+640,buffer,Extent(buffer));
  (void) WriteBlob(image,2880,(char *) fits_header);
  FreeMemory(fits_header);
  /*
    Convert image to fits scale PseudoColor class.
  */
  for (y=image->rows-1; y >= 0; y--)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    (void) WritePixelCache(image,GrayQuantum,pixels);
    (void) WriteBlob(image,packet_size*image->columns,pixels);
    if (QuantumTick(image->rows-y-1,image->rows))
      ProgressMonitor(SaveImageText,image->rows-y-1,image->rows);
  }
  FreeMemory(pixels);
  CloseBlob(image);
  return(True);
}
