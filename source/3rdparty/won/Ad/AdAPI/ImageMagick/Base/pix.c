/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   IIIII  X   X                              %
%                            P   P    I     X X                               %
%                            PPPP     I      X                                %
%                            P        I     X X                               %
%                            P      IIIII  X   X                              %
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
%   R e a d P I X I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadPIXImage reads a Alias/Wavefront RLE image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPIXImage method is:
%
%      Image *ReadPIXImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPIXImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadPIXImage(const ImageInfo *image_info)
{
  Image
    *image;

  IndexPacket
    index;

  int
    y;

  Quantum
    blue,
    green,
    red;

  register int
    x;

  register PixelPacket
    *q;

  unsigned int
    bits_per_pixel,
    status;

  unsigned long
    height,
    length,
    width;

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
    Read PIX image.
  */
  width=MSBFirstReadShort(image);
  height=MSBFirstReadShort(image);
  (void) MSBFirstReadShort(image);  /* x-offset */
  (void) MSBFirstReadShort(image);  /* y-offset */
  bits_per_pixel=MSBFirstReadShort(image);
  if ((width == (unsigned long) ~0) || (height == (unsigned long) ~0) ||
      ((bits_per_pixel != 8) && (bits_per_pixel != 24)))
    ReaderExit(CorruptImageWarning,"Not a PIX image file",image);
  do
  {
    /*
      Initialize image structure.
    */
    image->columns=width;
    image->rows=height;
    if (bits_per_pixel == 8)
      {
        register int
          i;

        /*
          Create linear colormap.
        */
        image->class=PseudoClass;
        image->colors=MaxRGB+1;
        image->colormap=(PixelPacket *)
          AllocateMemory(image->colors*sizeof(PixelPacket));
        if (image->colormap == (PixelPacket *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        for (i=0; i < (int) image->colors; i++)
        {
          image->colormap[i].red=(Quantum) UpScale(i);
          image->colormap[i].green=(Quantum) UpScale(i);
          image->colormap[i].blue=(Quantum) UpScale(i);
        }
      }
    if (image_info->ping)
      {
        CloseBlob(image);
        return(image);
      }
    /*
      Convert PIX raster image to pixel packets.
    */
    red=0;
    green=0;
    blue=0;
    index=0;
    length=0;
    for (y=0; y < (int) image->rows; y++)
    {
      q=SetPixelCache(image,0,y,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) image->columns; x++)
      {
        if (length == 0)
          {
            length=ReadByte(image);
            if (bits_per_pixel == 8)
              index=UpScale(ReadByte(image));
            else
              {
                blue=UpScale(ReadByte(image));
                green=UpScale(ReadByte(image));
                red=UpScale(ReadByte(image));
              }
          }
        q->blue=blue;
        q->green=green;
        q->red=red;
        image->indexes[x]=index;
        length--;
        q++;
      }
      if (!SyncPixelCache(image))
        break;
      if (image->previous == (Image *) NULL)
        ProgressMonitor(LoadImageText,y,image->rows);
    }
    if (image->class == PseudoClass)
      SyncImage(image);
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    width=MSBFirstReadLong(image);
    height=MSBFirstReadLong(image);
    (void) MSBFirstReadShort(image);
    (void) MSBFirstReadShort(image);
    bits_per_pixel=MSBFirstReadShort(image);
    status=(width != (unsigned long) ~0) && (height == (unsigned long) ~0) &&
      ((bits_per_pixel == 8) || (bits_per_pixel == 24));
    if (status == True)
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
  } while (status == True);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseBlob(image);
  return(image);
}
