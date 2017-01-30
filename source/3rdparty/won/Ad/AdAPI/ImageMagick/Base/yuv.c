/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            Y   Y  U   U  V   V                              %
%                             Y Y   U   U  V   V                              %
%                              Y    U   U  V   V                              %
%                              Y    U   U   V V                               %
%                              Y     UUU     V                                %
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
%   R e a d Y U V I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadYUVImage reads an image with digital YUV (CCIR 601 4:1:1) bytes
%  and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadYUVImage method is:
%
%      Image *ReadYUVImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadYUVImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadYUVImage(const ImageInfo *image_info)
{
  Image
    *chroma_image,
    *image,
    *zoom_image;

  int
    count,
    y;

  register int
    i,
    x;

  register PixelPacket
    *q,
    *r;

  register unsigned char
    *p;

  unsigned char
    *scanline;

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
  image->depth=8;
  if (image_info->interlace != PartitionInterlace)
    {
      /*
        Open image file.
      */
      status=OpenBlob(image_info,image,ReadBinaryType);
      if (status == False)
        ReaderExit(FileOpenWarning,"Unable to open file",image);
      for (i=0; i < image->offset; i++)
        (void) ReadByte(image);
    }
  /*
    Allocate memory for a scanline.
  */
  scanline=(unsigned char *)
    AllocateMemory(image->columns*sizeof(unsigned char));
  if (scanline == (unsigned char *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  do
  {
    /*
      Convert raster image to pixel packets.
    */
    if (image_info->interlace == PartitionInterlace)
      {
        AppendImageFormat("Y",image->filename);
        status=OpenBlob(image_info,image,ReadBinaryType);
        if (status == False)
          ReaderExit(FileOpenWarning,"Unable to open file",image);
      }
    for (y=0; y < (int) image->rows; y++)
    {
      if ((y > 0) || (image->previous == (Image *) NULL))
        (void) ReadBlob(image,image->columns,scanline);
      p=scanline;
      q=SetPixelCache(image,0,y,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) image->columns; x++)
      {
        q->red=UpScale(*p++);
        q->green=0;
        q->blue=0;
        q++;
      }
      if (!SyncPixelCache(image))
        break;
      if (image->previous == (Image *) NULL)
        ProgressMonitor(LoadImageText,y,image->rows);
    }
    if (image_info->interlace == PartitionInterlace)
      {
        CloseBlob(image);
        AppendImageFormat("U",image->filename);
        status=OpenBlob(image_info,image,ReadBinaryType);
        if (status == False)
          ReaderExit(FileOpenWarning,"Unable to open file",image);
      }
    chroma_image=CloneImage(image,image->columns/2,image->rows/2,True);
    if (chroma_image == (Image *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    for (y=0; y < (int) chroma_image->rows; y++)
    {
      (void) ReadBlob(image,chroma_image->columns,scanline);
      p=scanline;
      q=SetPixelCache(chroma_image,0,y,chroma_image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) chroma_image->columns; x++)
      {
        q->red=0;
        q->green=UpScale(*p++);
        q->blue=0;
        q++;
      }
      if (!SyncPixelCache(chroma_image))
        break;
    }
    if (image_info->interlace == PartitionInterlace)
      {
        CloseBlob(image);
        AppendImageFormat("V",image->filename);
        status=OpenBlob(image_info,image,ReadBinaryType);
        if (status == False)
          ReaderExit(FileOpenWarning,"Unable to open file",image);
      }
    for (y=0; y < (int) chroma_image->rows; y++)
    {
      (void) ReadBlob(image,chroma_image->columns,scanline);
      p=scanline;
      q=GetPixelCache(chroma_image,0,y,chroma_image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) chroma_image->columns; x++)
      {
        q->blue=UpScale(*p++);
        q++;
      }
      if (!SyncPixelCache(chroma_image))
        break;
    }
    /*
      Scale image.
    */
    chroma_image->orphan=True;
    zoom_image=SampleImage(chroma_image,image->columns,image->rows);
    DestroyImage(chroma_image);
    if (zoom_image == (Image *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    for (y=0; y < (int) image->rows; y++)
    {
      q=GetPixelCache(image,0,y,image->columns,1);
      r=GetPixelCache(zoom_image,0,y,zoom_image->columns,1);
      if ((q == (PixelPacket *) NULL) || (r == (PixelPacket *) NULL))
        break;
      for (x=0; x < (int) image->columns; x++)
      {
        q->green=r->green;
        q->blue=r->blue;
        r++;
        q++;
      }
      if (!SyncPixelCache(image))
        break;
    }
    DestroyImage(zoom_image);
    TransformRGBImage(image,YCbCrColorspace);
    if (image_info->interlace == PartitionInterlace)
      (void) strcpy(image->filename,image_info->filename);
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    count=ReadBlob(image,image->columns,(char *) scanline);
    if (count > 0)
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
  } while (count > 0);
  FreeMemory(scanline);
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
%   W r i t e Y U V I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteYUVImage writes an image to a file in the digital YUV
%  (CCIR 601 4:1:1) format.
%
%  The format of the WriteYUVImage method is:
%
%      unsigned int WriteYUVImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteYUVImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteYUVImage(const ImageInfo *image_info,Image *image)
{
  Image
    *chroma_image,
    *yuv_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *p;

  unsigned int
    height,
    scene,
    status,
    width;

  if (image_info->interlace != PartitionInterlace)
    {
      /*
        Open output image file.
      */
      status=OpenBlob(image_info,image,WriteBinaryType);
      if (status == False)
        WriterExit(FileOpenWarning,"Unable to open file",image);
    }
  if (image_info->interlace == PartitionInterlace)
    {
      AppendImageFormat("Y",image->filename);
      status=OpenBlob(image_info,image,WriteBinaryType);
      if (status == False)
        WriterExit(FileOpenWarning,"Unable to open file",image);
    }
  scene=0;
  do
  {
    /*
      Sample image to an even width and height.
    */
    TransformRGBImage(image,RGBColorspace);
    width=image->columns+(image->columns & 0x01);
    height=image->rows+(image->rows & 0x01);
    image->orphan=True;
    yuv_image=SampleImage(image,width,height);
    if (yuv_image == (Image *) NULL)
      WriterExit(ResourceLimitWarning,"Unable to zoom image",image);
    RGBTransformImage(yuv_image,YCbCrColorspace);
    /*
      Initialize Y channel.
    */
    for (y=0; y < (int) yuv_image->rows; y++)
    {
      p=GetPixelCache(yuv_image,0,y,yuv_image->columns,1);
      if (p == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) yuv_image->columns; x++)
      {
        (void) WriteByte(image,DownScale(p->red));
        p++;
      }
      if (image->previous == (Image *) NULL)
        if (QuantumTick(y,image->rows))
          ProgressMonitor(SaveImageText,y,image->rows);
    }
    DestroyImage(yuv_image);
    /*
      Downsample image.
    */
    image->orphan=True;
    chroma_image=SampleImage(image,width/2,height/2);
    if (chroma_image == (Image *) NULL)
      WriterExit(ResourceLimitWarning,"Unable to zoom image",image);
    RGBTransformImage(chroma_image,YCbCrColorspace);
    /*
      Initialize U channel.
    */
    if (image_info->interlace == PartitionInterlace)
      {
        CloseBlob(image);
        AppendImageFormat("U",image->filename);
        status=OpenBlob(image_info,image,WriteBinaryType);
        if (status == False)
          WriterExit(FileOpenWarning,"Unable to open file",image);
      }
    for (y=0; y < (int) chroma_image->rows; y++)
    {
      p=GetPixelCache(chroma_image,0,y,chroma_image->columns,1);
      if (p == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) chroma_image->columns; x++)
      {
        (void) WriteByte(image,DownScale(p->green));
        p++;
      }
    }
    /*
      Initialize V channel.
    */
    if (image_info->interlace == PartitionInterlace)
      {
        CloseBlob(image);
        AppendImageFormat("V",image->filename);
        status=OpenBlob(image_info,image,WriteBinaryType);
        if (status == False)
          WriterExit(FileOpenWarning,"Unable to open file",image);
      }
    for (y=0; y < (int) chroma_image->rows; y++)
    {
      p=GetPixelCache(chroma_image,0,y,chroma_image->columns,1);
      if (p == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) chroma_image->columns; x++)
      {
	(void) WriteByte(image,DownScale(p->blue));
        p++;
      }
    }
    DestroyImage(chroma_image);
    if (image_info->interlace == PartitionInterlace)
      (void) strcpy(image->filename,image_info->filename);
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
