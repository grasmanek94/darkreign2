/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            SSSSS   CCCC  TTTTT                              %
%                            SS     C        T                                %
%                             SSS   C        T                                %
%                               SS  C        T                                %
%                            SSSSS   CCCC    T                                %
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
%   I s S C T                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsSCT returns True if the image format type, identified by the
%  magick string, is SCT.
%
%  The format of the ReadSCTImage method is:
%
%      unsigned int IsSCT(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsSCT returns True if the image format type is SCT.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsSCT(const unsigned char *magick,const unsigned int length)
{
  if (length < 2)
    return(False);
  if (strncmp((char *) magick,"CT",2) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S C T I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadSCTImage reads a Scitex image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadSCTImage method is:
%
%      Image *ReadSCTImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadSCTImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadSCTImage(const ImageInfo *image_info)
{
  char
    buffer[768],
    magick[2];

  Image
    *image;

  int
    y;

  register int
    x;

  register PixelPacket
    *q;

  unsigned int
    status;

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
    Read control block.
  */
  (void) ReadBlob(image,80,(char *) buffer);
  (void) ReadBlob(image,2,(char *) magick);
  if ((strncmp((char *) magick,"CT",2) != 0) &&
      (strncmp((char *) magick,"LW",2) != 0) &&
      (strncmp((char *) magick,"BM",2) != 0) &&
      (strncmp((char *) magick,"PG",2) != 0) &&
      (strncmp((char *) magick,"TX",2) != 0))
    ReaderExit(CorruptImageWarning,"Not a SCT image file",image);
  if ((strncmp((char *) magick,"LW",2) == 0) ||
      (strncmp((char *) magick,"BM",2) == 0) ||
      (strncmp((char *) magick,"PG",2) == 0) ||
      (strncmp((char *) magick,"TX",2) == 0))
    ReaderExit(CorruptImageWarning,"only Continuous Tone Picture supported",
      image);
  (void) ReadBlob(image,174,(char *) buffer);
  (void) ReadBlob(image,768,(char *) buffer);
  /*
    Read paramter block.
  */
  (void) ReadBlob(image,32,(char *) buffer);
  (void) ReadBlob(image,14,(char *) buffer);
  image->rows=atoi(buffer);
  (void) ReadBlob(image,14,(char *) buffer);
  image->columns=atoi(buffer);
  (void) ReadBlob(image,196,(char *) buffer);
  (void) ReadBlob(image,768,(char *) buffer);
  if (image_info->ping)
    {
      CloseBlob(image);
      return(image);
    }
  /*
    Convert SCT raster image to pixel packets.
  */
  image->colorspace=CMYKColorspace;
  for (y=0; y < (int) image->rows; y++)
  {
    q=SetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      q->red=MaxRGB-UpScale(ReadByte(image));
      q++;
    }
    if ((image->columns % 2) != 0)
      (void) ReadByte(image);  /* pad */
    q=GetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      q->green=MaxRGB-UpScale(ReadByte(image));
      q++;
    }
    if ((image->columns % 2) != 0)
      (void) ReadByte(image);  /* pad */
    q=GetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      q->blue=MaxRGB-UpScale(ReadByte(image));
      q++;
    }
    if ((image->columns % 2) != 0)
      (void) ReadByte(image);  /* pad */
    q=GetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      q->opacity=MaxRGB-UpScale(ReadByte(image));
      q++;
    }
    if (!SyncPixelCache(image))
      break;
    if ((image->columns % 2) != 0)
      (void) ReadByte(image);  /* pad */
    if (QuantumTick(y,image->rows))
      ProgressMonitor(LoadImageText,y,image->rows);
  }
  return(image);
}
