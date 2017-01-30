/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         W   W  BBBB   M   M  PPPP                           %
%                         W   W  B   B  MM MM  P   P                          %
%                         W W W  BBBB   M M M  PPPP                           %
%                         WW WW  B   B  M   M  P                              %
%                         W   W  BBBB   M   M  P                              %
%                                                                             %
%                                                                             %
%                    Read/Write ImageMagick Image Format.                     %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               January 2000                                  %
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
%   R e a d W B M P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadWBMPImage reads a WBMP (level 0) image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  ReadWBMPImage was contributed by Milan Votava <votava@mageo.cz>.
%
%  The format of the ReadWBMPImage method is:
%
%      Image *ReadWBMPImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadWBMPImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/

static unsigned int WBMPReadInteger(Image *image,unsigned int *value)
{
  int
    byte;

  *value=0;
  do
  {
    byte=ReadByte(image);
    if (byte == EOF)
      return(False);
    *value<<=7;
    *value|=(unsigned int) (byte & 0x7f);
  } while (byte & 0x80);
  return(True);
}

Export Image *ReadWBMPImage(const ImageInfo *image_info)
{
  Image
    *image;

  int
    y,
    byte;

  register int
    i,
    x;

  unsigned char
    bit;

  unsigned int
    status;

  unsigned short
    header;

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
  if (!ReadBlob(image,2,(char *) &header)) 
    ReaderExit(CorruptImageWarning,"Not a WBMP image file",image);
  if (header)
    ReaderExit(CorruptImageWarning,"Only WBMP level 0 files supported",image);
  /*
    Determine width and height
  */
  if (WBMPReadInteger(image,&image->columns) == False) 
    ReaderExit(CorruptImageWarning,"Corrupt WBMP image",image);
  if (WBMPReadInteger(image,&image->rows) == False) 
    ReaderExit(CorruptImageWarning,"Corrupt WBMP image",image);
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
        {
          byte=ReadByte(image);
          if (byte == EOF)
            ReaderExit(CorruptImageWarning,"Corrupt WBMP image",image);
        }
      image->indexes[x]=(byte & (0x01 << (7-bit))) ? 1 : 0;
      bit++;
      if (bit == 8)
        bit=0;
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
%   W r i t e W B M P I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteWBMPImage writes an image to a file in the Wireless Bitmap
%  (level 0) image format.
%
%  WriteWBMPImage was contributed by Milan Votava <votava@mageo.cz>.
%
%  The format of the WriteWBMPImage method is:
%
%      unsigned int WriteWBMPImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteWBMPImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/

static void WBMPWriteInteger(Image *image,const unsigned int value)
{
  int
    bits,
    flag,
    n;

  register int
    i;

  unsigned char
    buffer[5],
    octet;

  n=1;
  bits=28;
  flag=False;
  for(i=4; i >= 0; i--)
  {
    octet=((value >> bits) & 0x7f);
    if (!flag && octet)
      {
        flag=True;
        n=i+1;
      }
    buffer[4-i]=octet | (i && (flag || octet))*(0x01 << 7);
    bits-=7;
  }
  (void) WriteBlob(image,n,(char *) buffer+5-n);
}

Export unsigned int WriteWBMPImage(const ImageInfo *image_info,Image *image)
{
  int
    y;

  register int
    x;

  register PixelPacket
    *p;

  unsigned char
    bit,
    byte,
    polarity;

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
    polarity=Intensity(image->colormap[0]) < Intensity(image->colormap[1]);
  MSBFirstWriteShort(image,0);
  WBMPWriteInteger(image,image->columns);
  WBMPWriteInteger(image,image->rows);
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    bit=0;
    byte=0;
    for (x=0; x < (int) image->columns; x++)
    {
      if (image->indexes[x] == polarity)
        byte|=0x1 << (7-bit);
      bit++;
      if (bit == 8)
        {
          (void) WriteByte(image,byte);
          bit=0;
          byte=0;
        }
    }
    if (bit != 0)
      (void) WriteByte(image,byte);
    if (QuantumTick(y,image->rows))
      ProgressMonitor(SaveImageText,y,image->rows);
  }
  CloseBlob(image);
  return(True);
}
