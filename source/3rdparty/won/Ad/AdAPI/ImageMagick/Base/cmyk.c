/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                         CCCC  M   M  Y   Y  K   K                           %
%                        C      MM MM   Y Y   K  K                            %
%                        C      M M M    Y    KKK                             %
%                        C      M   M    Y    K  K                            %
%                         CCCC  M   M    Y    K   K                           %
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
%   R e a d C Y M K I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadCMYKImage reads an image of raw cyan, yellow, magenta, and black
%  bytes and returns it.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the ReadCMYKImage method is:
%
%      Image *ReadCMYKImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadCMYKImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadCMYKImage(const ImageInfo *image_info)
{
  Image
    *image;

  int
    count,
    y;

  register int
    i,
    x;

  unsigned char
    *scanline;

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
  packet_size=image->depth > 8 ? 8 : 4;
  scanline=(unsigned char *)
    AllocateMemory(packet_size*image->tile_info.width*sizeof(unsigned char));
  if (scanline == (unsigned char *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  if (image_info->subrange != 0)
    while (image->scene < image_info->subimage)
    {
      /*
        Skip to next image.
      */
      image->scene++;
      for (y=0; y < (int) image->rows; y++)
        (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
    }
  x=packet_size*image->tile_info.x;
  do
  {
    /*
      Convert raster image to pixel packets.
    */
    image->colorspace=CMYKColorspace;
    switch (image_info->interlace)
    {
      case NoInterlace:
      default:
      {
        /*
          No interlacing:  CMYKCMYKCMYKCMYKCMYKCMYK...
        */
        for (y=0; y < image->tile_info.y; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        for (y=0; y < (int) image->rows; y++)
        {
          if ((y > 0) || (image->previous == (Image *) NULL))
            (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
          if (!SetPixelCache(image,0,y,image->columns,1))
            break;
          (void) ReadPixelCache(image,CMYKQuantum,scanline+x);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,y,image->rows);
        }
        count=image->tile_info.height-image->rows-image->tile_info.y;
        for (y=0; y < count; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        break;
      }
      case LineInterlace:
      {
        /*
          Line interlacing:  CCC...MMM...YYY...KKK...CCC...MMM...YYY...KKK...
        */
        packet_size=image->depth > 8 ? 2 : 1;
        for (y=0; y < image->tile_info.y; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        for (y=0; y < (int) image->rows; y++)
        {
          if ((y > 0) || (image->previous == (Image *) NULL))
            (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
          if (!SetPixelCache(image,0,y,image->columns,1))
            break;
          (void) ReadPixelCache(image,CyanQuantum,scanline+x);
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
          (void) ReadPixelCache(image,YellowQuantum,scanline+x);
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
          (void) ReadPixelCache(image,MagentaQuantum,scanline+x);
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
          (void) ReadPixelCache(image,BlackQuantum,scanline+x);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,y,image->rows);
        }
        count=image->tile_info.height-image->rows-image->tile_info.y;
        for (y=0; y < count; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        break;
      }
      case PlaneInterlace:
      case PartitionInterlace:
      {
        unsigned int
          span;

        /*
          Plane interlacing:  CCCCCC...MMMMMM...YYYYYY...KKKKKK...
        */
        if (image_info->interlace == PartitionInterlace)
          {
            AppendImageFormat("C",image->filename);
            status=OpenBlob(image_info,image,ReadBinaryType);
            if (status == False)
              ReaderExit(FileOpenWarning,"Unable to open file",image);
          }
        packet_size=image->depth > 8 ? 2 : 1;
        for (y=0; y < image->tile_info.y; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        i=0;
        span=image->rows*(image->matte ? 4 : 3);
        for (y=0; y < (int) image->rows; y++)
        {
          if ((y > 0) || (image->previous == (Image *) NULL))
            (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
          if (!SetPixelCache(image,0,y,image->columns,1))
            break;
          (void) ReadPixelCache(image,CyanQuantum,scanline+x);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(i,span))
              ProgressMonitor(LoadImageText,i,span);
          i++;
        }
        count=image->tile_info.height-image->rows-image->tile_info.y;
        for (y=0; y < count; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        if (image_info->interlace == PartitionInterlace)
          {
            CloseBlob(image);
            AppendImageFormat("Y",image->filename);
            status=OpenBlob(image_info,image,ReadBinaryType);
            if (status == False)
              ReaderExit(FileOpenWarning,"Unable to open file",image);
          }
        for (y=0; y < image->tile_info.y; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        for (y=0; y < (int) image->rows; y++)
        {
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) ReadPixelCache(image,YellowQuantum,scanline+x);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(i,span))
              ProgressMonitor(LoadImageText,i,span);
          i++;
        }
        count=image->tile_info.height-image->rows-image->tile_info.y;
        for (y=0; y < count; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        if (image_info->interlace == PartitionInterlace)
          {
            CloseBlob(image);
            AppendImageFormat("M",image->filename);
            status=OpenBlob(image_info,image,ReadBinaryType);
            if (status == False)
              ReaderExit(FileOpenWarning,"Unable to open file",image);
          }
        for (y=0; y < image->tile_info.y; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        for (y=0; y < (int) image->rows; y++)
        {
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) ReadPixelCache(image,MagentaQuantum,scanline+x);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(i,span))
              ProgressMonitor(LoadImageText,i,span);
          i++;
        }
        count=image->tile_info.height-image->rows-image->tile_info.y;
        for (y=0; y < count; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        if (image_info->interlace == PartitionInterlace)
          {
            CloseBlob(image);
            AppendImageFormat("K",image->filename);
            status=OpenBlob(image_info,image,ReadBinaryType);
            if (status == False)
              ReaderExit(FileOpenWarning,"Unable to open file",image);
          }
        for (y=0; y < image->tile_info.y; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        for (y=0; y < (int) image->rows; y++)
        {
          (void) ReadBlob(image,packet_size*image->tile_info.width,
            scanline);
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) ReadPixelCache(image,BlackQuantum,scanline+x);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(i,span))
              ProgressMonitor(LoadImageText,i,span);
          i++;
        }
        count=image->tile_info.height-image->rows-image->tile_info.y;
        for (y=0; y < count; y++)
          (void) ReadBlob(image,packet_size*image->tile_info.width,scanline);
        if (image_info->interlace == PartitionInterlace)
          (void) strcpy(image->filename,image_info->filename);
        break;
      }
    }
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    count=ReadBlob(image,packet_size*image->tile_info.width,scanline);
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
%   W r i t e C M Y K I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteCMYKImage writes an image to a file in cyan, yellow, magenta,
%  and black rasterfile format.
%
%  The format of the WriteCMYKImage method is:
%
%      unsigned int WriteCMYKImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteCMYKImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteCMYKImage(const ImageInfo *image_info,Image *image)
{
  int
    y;

  unsigned char
    *pixels;

  unsigned int
    packet_size,
    scene,
    status;

  /*
    Allocate memory for pixels.
  */
  packet_size=image->depth > 8 ? 8 : 4;
  pixels=(unsigned char *)
    AllocateMemory(packet_size*image->columns*sizeof(unsigned char));
  if (pixels == (unsigned char *) NULL)
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  if (image_info->interlace != PartitionInterlace)
    {
      /*
        Open output image file.
      */
      status=OpenBlob(image_info,image,WriteBinaryType);
      if (status == False)
        WriterExit(FileOpenWarning,"Unable to open file",image);
    }
  scene=0;
  do
  {
    /*
      Convert MIFF to CMYK raster pixels.
    */
    if (image->colorspace != CMYKColorspace)
      RGBTransformImage(image,CMYKColorspace);
    switch (image_info->interlace)
    {
      case NoInterlace:
      default:
      {
        /*
          No interlacing:  CMYKCMYKCMYKCMYKCMYKCMYK...
        */
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) WritePixelCache(image,CMYKQuantum,pixels);
          (void) WriteBlob(image,packet_size*image->columns,pixels);
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
        }
        break;
      }
      case LineInterlace:
      {
        /*
          Line interlacing:  CCC...MMM...YYY...KKK...CCC...MMM...YYY...KKK...
        */
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) WritePixelCache(image,CyanQuantum,pixels);
          (void) WriteBlob(image,image->columns,pixels);
          (void) WritePixelCache(image,YellowQuantum,pixels);
          (void) WriteBlob(image,image->columns,pixels);
          (void) WritePixelCache(image,MagentaQuantum,pixels);
          (void) WriteBlob(image,image->columns,pixels);
          (void) WritePixelCache(image,BlackQuantum,pixels);
          (void) WriteBlob(image,image->columns,pixels);
          if (QuantumTick(y,image->rows))
            ProgressMonitor(SaveImageText,y,image->rows);
        }
        break;
      }
      case PlaneInterlace:
      case PartitionInterlace:
      {
        /*
          Plane interlacing:  CCCCCC...MMMMMM...YYYYYY...KKKKKK...
        */
        if (image_info->interlace == PartitionInterlace)
          {
            AppendImageFormat("C",image->filename);
            status=OpenBlob(image_info,image,WriteBinaryType);
            if (status == False)
              WriterExit(FileOpenWarning,"Unable to open file",image);
          }
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) WritePixelCache(image,CyanQuantum,pixels);
          (void) WriteBlob(image,image->columns,pixels);
        }
        if (image_info->interlace == PartitionInterlace)
          {
            CloseBlob(image);
            AppendImageFormat("M",image->filename);
            status=OpenBlob(image_info,image,WriteBinaryType);
            if (status == False)
              WriterExit(FileOpenWarning,"Unable to open file",image);
          }
        ProgressMonitor(SaveImageText,100,400);
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) WritePixelCache(image,YellowQuantum,pixels);
          (void) WriteBlob(image,image->columns,pixels);
        }
        if (image_info->interlace == PartitionInterlace)
          {
            CloseBlob(image);
            AppendImageFormat("Y",image->filename);
            status=OpenBlob(image_info,image,WriteBinaryType);
            if (status == False)
              WriterExit(FileOpenWarning,"Unable to open file",image);
          }
        ProgressMonitor(SaveImageText,200,400);
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) WritePixelCache(image,MagentaQuantum,pixels);
          (void) WriteBlob(image,image->columns,pixels);
        }
        if (image->matte)
          {
            ProgressMonitor(SaveImageText,300,400);
            if (image_info->interlace == PartitionInterlace)
              {
                CloseBlob(image);
                AppendImageFormat("K",image->filename);
                status=OpenBlob(image_info,image,WriteBinaryType);
                if (status == False)
                  WriterExit(FileOpenWarning,"Unable to open file",image);
              }
            for (y=0; y < (int) image->rows; y++)
            {
              if (!GetPixelCache(image,0,y,image->columns,1))
                break;
              (void) WritePixelCache(image,BlackQuantum,pixels);
              (void) WriteBlob(image,image->columns,pixels);
            }
          }
        if (image_info->interlace == PartitionInterlace)
          (void) strcpy(image->filename,image_info->filename);
        ProgressMonitor(SaveImageText,400,400);
        break;
      }
    }
    if (image->next == (Image *) NULL)
      break;
    image=GetNextImage(image);
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
  } while (image_info->adjoin);
  FreeMemory(pixels);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  CloseBlob(image);
  return(True);
}
