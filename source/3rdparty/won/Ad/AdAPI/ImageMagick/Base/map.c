/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            M   M   AAA   PPPP                               %
%                            MM MM  A   A  P   P                              %
%                            M M M  AAAAA  PPPP                               %
%                            M   M  A   A  P                                  %
%                            M   M  A   A  P                                  %
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
%   R e a d M A P I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadMAPImage reads an image of raw RGB colormap and colormap index
%  bytes and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadMAPImage method is:
%
%      Image *ReadMAPImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadMAPImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadMAPImage(const ImageInfo *image_info)
{
  Image
    *image;

  IndexPacket
    index;

  int
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *colormap,
    *pixels;

  unsigned int
    packet_size,
    status;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  if ((image->columns == 0) || (image->rows == 0))
    ReaderExit(OptionWarning,"Must specify image size",image);
  /*
    Open image file.
  */
  status=OpenBlob(image_info,image,ReadBinaryType);
  if (status == False)
    ReaderExit(FileOpenWarning,"Unable to open file",image);
  /*
    Initialize image structure.
  */
  image->class=PseudoClass;
  image->colors=image->offset ? image->offset : 256;
  packet_size=image->depth > 8 ? 2 : 1;
  pixels=(unsigned char *)
    AllocateMemory(packet_size*image->columns*sizeof(unsigned char));
  packet_size=image->colors > 256 ? 6 : 3;
  colormap=(unsigned char *)
    AllocateMemory(packet_size*image->colors*sizeof(unsigned char));
  image->colormap=(PixelPacket *)
    AllocateMemory(image->colors*sizeof(PixelPacket));
  if ((pixels == (unsigned char *) NULL) ||
      (colormap == (unsigned char *) NULL) ||
      (image->colormap == (PixelPacket *) NULL))
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  /*
    Read image colormap.
  */
  (void) ReadBlob(image,packet_size*image->colors,(char *) colormap);
  p=colormap;
  if (image->colors <= 256)
    for (i=0; i < (int) image->colors; i++)
    {
      image->colormap[i].red=UpScale(*p++);
      image->colormap[i].green=UpScale(*p++);
      image->colormap[i].blue=UpScale(*p++);
    }
  else
    for (i=0; i < (int) image->colors; i++)
    {
      image->colormap[i].red=(*p++ << 8);
      image->colormap[i].red|=(*p++);
      image->colormap[i].green=(*p++ << 8);
      image->colormap[i].green|=(*p++);
      image->colormap[i].blue=(*p++ << 8);
      image->colormap[i].blue|=(*p++);
    }
  FreeMemory(colormap);
  /*
    Read image pixels.
  */
  packet_size=image->depth > 8 ? 2 : 1;
  for (y=0; y < (int) image->rows; y++)
  {
    p=pixels;
    q=SetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    (void) ReadBlob(image,packet_size*image->columns,(char *) pixels);
    for (x=0; x < (int) image->columns; x++)
    {
      index=(*p++);
      if (image->colors > 256)
        index=(index << 8)+(*p++);
      if (index >= image->colors)
        ReaderExit(CorruptImageWarning,"invalid colormap index",image);
      image->indexes[x]=index;
      *q++=image->colormap[index];
    }
    if (!SyncPixelCache(image))
      break;
  }
  FreeMemory(pixels);
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M A P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteMAPImage writes an image to a file as red, green, and blue
%  colormap bytes followed by the colormap indexes.
%
%  The format of the WriteMAPImage method is:
%
%      unsigned int WriteMAPImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteMAPImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteMAPImage(const ImageInfo *image_info,Image *image)
{
  int
    y;

  register int
    i,
    x;

  register unsigned char
    *q;

  unsigned char
    *colormap,
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
    Allocate colormap.
  */
  if (!IsPseudoClass(image))
    {
      QuantizeInfo
        quantize_info;

      /*
        Demote DirectClass to PseudoClass.
      */
      GetQuantizeInfo(&quantize_info);
      quantize_info.number_colors=MaxColormapSize;
      quantize_info.dither=image_info->dither;
      (void) QuantizeImage(&quantize_info,image);
    }
  packet_size=image->depth > 8 ? 2 : 1;
  pixels=(unsigned char *)
    AllocateMemory(image->columns*packet_size*sizeof(unsigned char));
  packet_size=image->colors > 256 ? 6 : 3;
  colormap=(unsigned char *)
    AllocateMemory(packet_size*image->colors*sizeof(unsigned char));
  if ((pixels == (unsigned char *) NULL) ||
      (colormap == (unsigned char *) NULL))
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  /*
    Write colormap to file.
  */
  q=colormap;
  if (image->colors <= 256)
    for (i=0; i < (int) image->colors; i++)
    {
      *q++=image->colormap[i].red;
      *q++=image->colormap[i].green;
      *q++=image->colormap[i].blue;
    }
  else
    for (i=0; i < (int) image->colors; i++)
    {
      *q++=image->colormap[i].red >> 8;
      *q++=image->colormap[i].red & 0xff;
      *q++=image->colormap[i].green >> 8;
      *q++=image->colormap[i].green & 0xff;
      *q++=image->colormap[i].blue >> 8;
      *q++=image->colormap[i].blue & 0xff;
    }
  (void) WriteBlob(image,packet_size*image->colors,(char *) colormap);
  FreeMemory(colormap);
  /*
    Write image pixels to file.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    if (!GetPixelCache(image,0,y,image->columns,1))
      break;
    q=pixels;
    for (x=0; x < (int) image->columns; x++)
    {
      if (image->colors > 256)
        *q++=image->indexes[x] >> 8;
      *q++=image->indexes[x];
    }
    status=WriteBlob(image,q-pixels,(char *) pixels);
  }
  FreeMemory(pixels);
  CloseBlob(image);
  return(status);
}
