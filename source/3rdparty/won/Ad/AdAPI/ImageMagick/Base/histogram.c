/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%       H   H  IIIII  SSSSS  TTTTT   OOO    GGGG  RRRR    AAA   M   M         %
%       H   H    I    SS       T    O   O  G      R   R  A   A  MM MM         %
%       HHHHH    I     SSS     T    O   O  G  GG  RRRR   AAAAA  M M M         %
%       H   H    I       SS    T    O   O  G   G  R R    A   A  M   M         %
%       H   H  IIIII  SSSSS    T     OOO    GGG   R  R   A   A  M   M         %
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
%   R e a d H I S T O G R A M I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadHISTOGRAMImage reads a HISTOGRAM image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadHISTOGRAMImage method is:
%
%      Image *ReadHISTOGRAMImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadHISTOGRAMImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadHISTOGRAMImage(const ImageInfo *image_info)
{
  Image
    *image;

  image=ReadMIFFImage(image_info);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e H I S T O G R A M I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteHISTOGRAMImage writes an image to a file in HISTOGRAM format.
%  The image shows a histogram of the color (or gray) values in the image.  The
%  image consists of three overlaid histograms:  a red one for the red channel,
%  a green one for the green channel, and a blue one for the blue channel.  The
%  image comment contains a list of unique pixel values and the number of times
%  each occurs in the image.
%
%  This method is strongly based on a similar one written by
%  muquit@warm.semcor.com which in turn is based on ppmhistmap of netpbm.
%
%  The format of the WriteHISTOGRAMImage method is:
%
%      unsigned int WriteHISTOGRAMImage(const ImageInfo *image_info,
%        Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteHISTOGRAMImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteHISTOGRAMImage(const ImageInfo *image_info,
  Image *image)
{
#define HistogramDensity  "256x200"

  char
    filename[MaxTextExtent];

  double
    scale;

  FILE
    *file;

  Image
    *histogram_image;

  int
    *blue,
    *green,
    maximum,
    *red,
    sans_offset,
    y;

  register PixelPacket
    *p,
    *q;

  register int
    x;

  unsigned int
    height,
    status,
    width;

  /*
    Allocate histogram image.
  */
  width=image->columns;
  height=image->rows;
  if (image_info->density != (char *) NULL)
    (void) ParseGeometry(image_info->density,&sans_offset,&sans_offset,
      &width,&height);
  else
    (void) ParseGeometry(HistogramDensity,&sans_offset,&sans_offset,
      &width,&height);
  histogram_image=CloneImage(image,width,height,True);
  if (histogram_image == (Image *) NULL)
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  histogram_image->class=DirectClass;
  /*
    Allocate histogram count arrays.
  */
  red=(int *) AllocateMemory (histogram_image->columns*sizeof(int));
  green=(int *) AllocateMemory (histogram_image->columns*sizeof(int));
  blue=(int *) AllocateMemory (histogram_image->columns*sizeof(int));
  if ((red == (int *) NULL) || (green == (int *) NULL) ||
      (blue == (int *) NULL))
    {
      DestroyImage(histogram_image);
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    }
  /*
    Initialize histogram count arrays.
  */
  for (x=0; x < (int) histogram_image->columns; x++)
  {
    red[x]=0;
    green[x]=0;
    blue[x]=0;
  }
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      red[DownScale(p->red)]++;
      green[DownScale(p->green)]++;
      blue[DownScale(p->blue)]++;
      p++;
    }
  }
  maximum=0;
  for (x=0; x < (int) histogram_image->columns; x++)
  {
    if (maximum < red[x])
      maximum=red[x];
    if (maximum < green[x])
      maximum=green[x];
    if (maximum < blue[x])
      maximum=blue[x];
  }
  for (x=0; x < (int) histogram_image->columns; x++)
  {
    if (red[x] > maximum)
      red[x]=maximum;
    if (green[x] > maximum)
      green[x]=maximum;
    if (blue[x] > maximum)
      blue[x]=maximum;
  }
  scale=(double) histogram_image->rows/maximum;
  /*
    Initialize histogram image.
  */
  (void) QueryColorDatabase("black",&histogram_image->background_color);
  SetImage(histogram_image);
  for (x=0; x < (int) histogram_image->columns; x++)
  {
    q=GetPixelCache(histogram_image,x,0,1,histogram_image->rows);
    if (q == (PixelPacket *) NULL)
      break;
    y=histogram_image->rows-(int) (scale*red[x]);
    p=q+y;
    for ( ; y < (int) histogram_image->rows; y++)
    {
      p->red=MaxRGB;
      p++;
    }
    y=histogram_image->rows-(int) (scale*green[x]);
    p=q+y;
    for ( ; y < (int) histogram_image->rows; y++)
    {
      p->green=MaxRGB;
      p++;
    }
    y=histogram_image->rows-(int) (scale*blue[x]);
    p=q+y;
    for ( ; y < (int) histogram_image->rows; y++)
    {
      p->blue=MaxRGB;
      p++;
    }
    if (!SyncPixelCache(histogram_image))
      break;
    if (QuantumTick(x,histogram_image->columns))
      ProgressMonitor(SaveImageText,x,histogram_image->columns);
  }
  /*
    Free memory resources.
  */
  FreeMemory ((char *) blue);
  FreeMemory ((char *) green);
  FreeMemory ((char *) red);
  TemporaryFilename(filename);
  file=fopen(filename,WriteBinaryType);
  if (file != (FILE *) NULL)
    {
      char
        command[MaxTextExtent];

      /*
        Add a histogram as an image comment.
      */
      if (image->comments != (char *) NULL)
        (void) fprintf(file,"%s\n",image->comments);
      (void) GetNumberColors(image,file);
      (void) fclose(file);
      FormatString(command,"@%.1024s",filename);
      CommentImage(histogram_image,command);
      (void) remove(filename);
    }
  /*
    Write HISTOGRAM image as MIFF.
  */
  status=WriteMIFFImage(image_info,histogram_image);
  DestroyImage(histogram_image);
  return(status);
}
