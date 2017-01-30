/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              SSSSS  TTTTT  EEEEE   GGGG   AAA   N   N   OOO                 %
%              SS       T    E      G      A   A  NN  N  O   O                %
%               SSS     T    EEE    G  GG  AAAAA  N N N  O   O                %
%                 SS    T    E      G   G  A   A  N  NN  O   O                %
%              SSSSS    T    EEEEE   GGG   A   A  N   N   OOO                 %
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
%   R e a d S T E G A N O I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadSTEGANOImage reads a steganographic image hidden within another
%  image type.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadSTEGANOImage method is:
%
%      Image *ReadSTEGANOImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadSTEGANOImage returns a pointer to the image
%      after reading.  A null image is returned if there is a memory shortage
%      of if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadSTEGANOImage(const ImageInfo *image_info)
{
#define UnembedBit(byte) \
{ \
  if (!GetPixelCache(image,j % image->columns,j/image->columns,1,1)) \
    break; \
  (*image->indexes)|=((byte) & 0x01) << shift; \
  (void) SyncPixelCache(image); \
  j++; \
  if (j == (image->columns*image->rows)) \
    { \
      j=0; \
      shift--; \
      if (shift < 0) \
        break; \
    } \
}

  ImageInfo
    *local_info;

  int
    j,
    shift,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p;

  Image
    *cloned_image,
    *image,
    *stegano_image;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if ((image->columns == 0) || (image->rows == 0))
    ReaderExit(OptionWarning,"Must specify image size",image);
  /*
    Initialize Image structure.
  */
  local_info=CloneImageInfo(image_info);
  if (local_info == (ImageInfo *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  *local_info->magick='\0';
  stegano_image=ReadImage(local_info);
  DestroyImageInfo(local_info);
  if (stegano_image == (Image *) NULL)
    return((Image *) NULL);
  cloned_image=CloneImage(stegano_image,image->columns,image->rows,True);
  DestroyImage(image);
  if (cloned_image == (Image *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",stegano_image);
  image=cloned_image;
  image->class=PseudoClass;
  image->colors=1 << QuantumDepth;
  image->colormap=(PixelPacket *)
    AllocateMemory(image->colors*sizeof(PixelPacket));
  if (image->colormap == (PixelPacket *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  for (i=0; i < (int) image->colors; i++)
  {
    image->colormap[i].red=((unsigned long) (MaxRGB*i)/(image->colors-1));
    image->colormap[i].green=((unsigned long) (MaxRGB*i)/(image->colors-1));
    image->colormap[i].blue=((unsigned long) (MaxRGB*i)/(image->colors-1));
  }
  SetImage(image);
  /*
    Grab embedded watermark.
  */
  i=image->offset;
  j=0;
  shift=image->depth-1;
  for (y=0; y < (int) stegano_image->rows; y++)
  {
    for (x=0; x < (int) stegano_image->columns; x++)
    {
      if (i == (stegano_image->columns*stegano_image->rows))
        i=0;
      p=GetPixelCache(stegano_image,i % stegano_image->columns,
        i/stegano_image->columns,1,1);
      if (p == (PixelPacket *) NULL)
        break;
      if (stegano_image->class == PseudoClass)
        UnembedBit(*stegano_image->indexes)
      else
        {
          UnembedBit(p->red);
          UnembedBit(p->green);
          UnembedBit(p->blue);
        }
      i++;
    }
    if (shift < 0)
      break;
    if (QuantumTick(y,stegano_image->rows))
      ProgressMonitor(LoadImageText,y,stegano_image->rows);
  }
  SyncImage(image);
  DestroyImage(stegano_image);
  return(image);
}
