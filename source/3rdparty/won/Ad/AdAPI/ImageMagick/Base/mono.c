/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        M   M   OOO   N   N   OOO                            %
%                        MM MM  O   O  NN  N  O   O                           %
%                        M M M  O   O  N N N  O   O                           %
%                        M   M  O   O  N  NN  O   O                           %
%                        M   M   OOO   N   N   OOO                            %
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
%   R e a d M O N O I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadMONOImage reads an image of raw bites in LSB order and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadMONOImage method is:
%
%      Image *ReadMONOImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadMONOImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadMONOImage(const ImageInfo *image_info)
{
  Image
    *image;

  int
    y;

  register int
    i,
    x;

  unsigned char
    bit,
    byte;

  unsigned int
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
  for (i=0; i < image->offset; i++)
    (void) ReadByte(image);
  /*
    Initialize image structure.
  */
  image->class=PseudoClass;
  image->colors=2;
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
  /*
    Convert bi-level image to pixel packets.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    if (!SetPixelCache(image,0,y,image->columns,1))
      break;
    bit=0;
    byte=0;
    for (x=0; x < (int) image->columns; x++)
    {
      if (bit == 0)
        byte=ReadByte(image);
      image->indexes[x]=(byte & 0x01) ? 0 : 1;
      bit++;
      if (bit == 8)
        bit=0;
      byte>>=1;
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(LoadImageText,y,image->rows);
  }
  SyncImage(image);
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M O N O I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteMONOImage writes an image of raw bits in LSB order to a file.
%
%  The format of the WriteMONOImage method is:
%
%      unsigned int WriteMONOImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteMONOImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteMONOImage(const ImageInfo *image_info,Image *image)
{
  int
    y;

  register unsigned char
    bit,
    byte,
    polarity;

  register int
    x;

  unsigned int
    status;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  TransformRGBImage(image,RGBColorspace);
  /*
    Convert image to a bi-level image.
  */
  if (!IsMonochromeImage(image))
    {
      QuantizeInfo
        quantize_info;

      GetQuantizeInfo(&quantize_info);
      quantize_info.number_colors=2;
      quantize_info.dither=image_info->dither;
      quantize_info.colorspace=GRAYColorspace;
      (void) QuantizeImage(&quantize_info,image);
    }
  polarity=Intensity(image->colormap[0]) > (MaxRGB >> 1);
  if (image->colors == 2)
    polarity=Intensity(image->colormap[0]) > Intensity(image->colormap[1]);
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
          (void) WriteByte(image,byte);
          bit=0;
          byte=0;
        }
    }
    if (bit != 0)
      (void) WriteByte(image,byte >> (8-bit));
    if (QuantumTick(y,image->rows))
      ProgressMonitor(SaveImageText,y,image->rows);
  }
  CloseBlob(image);
  return(True);
}
