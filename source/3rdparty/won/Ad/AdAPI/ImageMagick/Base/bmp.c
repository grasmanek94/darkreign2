/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            BBBB   M   M  PPPP                               %
%                            B   B  MM MM  P   P                              %
%                            BBBB   M M M  PPPP                               %
%                            B   B  M   M  P                                  %
%                            BBBB   M   M  P                                  %
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
%   D e c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DecodeImage unpacks the packed image pixels into runlength-encoded
%  pixel packets.
%
%  The format of the DecodeImage method is:
%
%      unsigned int DecodeImage(Image *image,const unsigned int compression,
%        const unsigned int number_columns,const unsigned int number_rows,
%        unsigned char *pixels)
%
%  A description of each parameter follows:
%
%    o status:  Method DecodeImage returns True if all the pixels are
%      uncompressed without error, otherwise False.
%
%    o image: The address of a structure of type Image.
%
%    o compression:  A value of 1 means the compressed pixels are runlength
%      encoded for a 256-color bitmap.  A value of 2 means a 16-color bitmap.
%
%    o number_columns:  An integer value that is the number of columns or
%      width in pixels of your source image.
%
%    o number_rows:  An integer value that is the number of rows or
%      heigth in pixels of your source image.
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the decoding process.
%
%
*/
static unsigned int DecodeImage(Image *image,const unsigned int compression,
  const unsigned int number_columns,const unsigned int number_rows,
  unsigned char *pixels)
{
  int
    byte,
    count,
    y;

  register int
    i,
    x;

  register unsigned char
    *q;

  assert(image != (Image *) NULL);
  assert(pixels != (unsigned char *) NULL);
  for (i=0; i < (int) (number_columns*number_rows); i++)
    pixels[i]=0;
  byte=0;
  x=0;
  q=pixels;
  for (y=0; y < (int) number_rows; )
  {
    count=ReadByte(image);
    if (count == EOF)
      break;
    if (count != 0)
      {
        /*
          Encoded mode.
        */
        byte=ReadByte(image);
        for (i=0; i < count; i++)
        {
          if (compression == 1)
            *q++=(unsigned char) byte;
          else
            *q++=(i & 0x01) ? (byte & 0x0f) : ((byte >> 4) & 0x0f);
          x++;
        }
      }
    else
      {
        /*
          Escape mode.
        */
        count=ReadByte(image);
        if (count == 0x01)
          return(True);
        switch (count)
        {
          case 0x00:
          {
            /*
              End of line.
            */
            x=0;
            y++;
            q=pixels+y*number_columns;
            break;
          }
          case 0x02:
          {
            /*
              Delta mode.
            */
            x+=ReadByte(image);
            y+=ReadByte(image);
            q=pixels+y*number_columns+x;
            break;
          }
          default:
          {
            /*
              Absolute mode.
            */
            for (i=0; i < count; i++)
            {
              if (compression == 1)
                *q++=ReadByte(image);
              else
                {
                  if ((i & 0x01) == 0)
                    byte=ReadByte(image);
                  *q++=(i & 0x01) ? (byte & 0x0f) : ((byte >> 4) & 0x0f);
                }
              x++;
            }
            /*
              Read pad byte.
            */
            if (compression == 1)
              {
                if (count & 0x01)
                  (void) ReadByte(image);
              }
            else
              if (((count & 0x03) == 1) || ((count & 0x03) == 2))
                (void) ReadByte(image);
            break;
          }
        }
      }
    if (QuantumTick(y,number_rows))
      ProgressMonitor(LoadImageText,y,number_rows);
  }
  (void) ReadByte(image);  /* end of line */
  (void) ReadByte(image);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method EncodeImage compresses pixels using a runlength encoded format.
%
%  The format of the EncodeImage method is:
%
%      status=EncodeImage(pixels,number_columns,number_rows,
%        compressed_pixels)
%
%  A description of each parameter follows:
%
%    o status:  Method EncodeImage returns the number of bytes in the
%      runlength encoded compress_pixels array.
%
%    o pixels:  The address of a byte (8 bits) array of pixel data created by
%      the compression process.
%
%    o number_columns:  An integer value that is the number of columns or
%      width in pixels of your source image.
%
%    o number_rows:  An integer value that is the number of rows or
%      heigth in pixels of your source image.
%
%    o compressed_pixels:  The address of a byte (8 bits) array of compressed
%      pixel data.
%
%
*/
static unsigned int EncodeImage(const unsigned char *pixels,
  const unsigned int number_columns,const unsigned int number_rows,
  unsigned char *compressed_pixels)
{
  int
    y;

  register const unsigned char
    *p;

  register int
    i,
    x;

  register unsigned char
    *q;

  unsigned int
    bytes_per_line;

  /*
    Runlength encode pixels.
  */
  assert(pixels != (unsigned char *) NULL);
  assert(compressed_pixels != (unsigned char *) NULL);
  p=pixels;
  q=compressed_pixels;
  i=0;
  bytes_per_line=number_columns+(number_columns % 2 ? 1 : 0);
  for (y=0; y < (int) number_rows; y++)
  {
    for (x=0; x < (int) bytes_per_line; x+=i)
    {
      /*
        Determine runlength.
      */
      for (i=1; ((x+i) < (int) bytes_per_line); i++)
        if ((*(p+i) != *p) || (i == 255))
          break;
      *q++=(unsigned char) i;
      *q++=(*p);
      p+=i;
    }
    /*
      End of line.
    */
    *q++=0x00;
    *q++=0x00;
    if (QuantumTick(y,number_rows))
      ProgressMonitor(SaveImageText,y,number_rows);
  }
  /*
    End of bitmap.
  */
  *q++=0;
  *q++=0x01;
  return((unsigned int) (q-compressed_pixels));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s B M P                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsBMP returns True if the image format type, identified by the
%  magick string, is BMP.
%
%  The format of the ReadBMPImage method is:
%
%      unsigned int IsBMP(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsBMP returns True if the image format type is BMP.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsBMP(const unsigned char *magick,const unsigned int length)
{
  if (length < 2)
    return(False);
  if (strncmp((char *) magick,"BM",2) == 0)
    return(True);
  if (strncmp((char *) magick,"IC",2) == 0)
    return(True);
  if (strncmp((char *) magick,"PI",2) == 0)
    return(True);
  if (strncmp((char *) magick,"CI",2) == 0)
    return(True);
  if (strncmp((char *) magick,"CP",2) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d B M P I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadBMPImage reads a Microsoft Windows bitmap image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadBMPImage method is:
%
%      image=ReadBMPImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadBMPImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadBMPImage(const ImageInfo *image_info)
{
  typedef struct _BMPHeader
  {
    unsigned long
      file_size;

    unsigned short
      reserved[2];

    unsigned long
      offset_bits,
      size;

    long
      width,
      height;

    unsigned short
      planes,
      bits_per_pixel;

    unsigned long
      compression,
      image_size,
      x_pixels,
      y_pixels,
      number_colors,
      colors_important;

    unsigned short
      red_mask,
      green_mask,
      blue_mask,
      alpha_mask;

    long
      colorspace;

    PointInfo
      red_primary,
      green_primary,
      blue_primary,
      gamma_scale;
  } BMPHeader;

  BMPHeader
    bmp_header;

  Image
    *image;

  IndexPacket
    index;

  int
    bit,
    y;

  long
    start_position;

  register int
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *bmp_pixels,
    magick[12];

  unsigned int
    bytes_per_line,
    image_size,
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
    Determine if this is a BMP file.
  */
  status=ReadBlob(image,2,(char *) magick);
  do
  {
    /*
      Verify BMP identifier.
    */
    start_position=TellBlob(image)-2;
    if ((status == False) || (strncmp((char *) magick,"BM",2) != 0))
      ReaderExit(CorruptImageWarning,"Not a BMP image file",image);
    bmp_header.file_size=LSBFirstReadLong(image);
    bmp_header.reserved[0]=LSBFirstReadShort(image);
    bmp_header.reserved[1]=LSBFirstReadShort(image);
    bmp_header.offset_bits=LSBFirstReadLong(image);
    bmp_header.size=LSBFirstReadLong(image);
    if (image->filesize)
      if ((int) (bmp_header.file_size-bmp_header.size) > image->filesize)
        ReaderExit(CorruptImageWarning,"Not a BMP image file",image);
    if (bmp_header.size == 12)
      {
        /*
          OS/2 BMP image file.
        */
        bmp_header.width=LSBFirstReadShort(image);
        bmp_header.height=LSBFirstReadShort(image);
        bmp_header.planes=LSBFirstReadShort(image);
        bmp_header.bits_per_pixel=LSBFirstReadShort(image);
        bmp_header.x_pixels=0;
        bmp_header.y_pixels=0;
        bmp_header.number_colors=0;
        bmp_header.compression=0;
        bmp_header.image_size=0;
      }
    else
      {
        /*
          Microsoft Windows BMP image file.
        */
        bmp_header.width=LSBFirstReadLong(image);
        bmp_header.height=LSBFirstReadLong(image);
        bmp_header.planes=LSBFirstReadShort(image);
        bmp_header.bits_per_pixel=LSBFirstReadShort(image);
        bmp_header.compression=LSBFirstReadLong(image);
        bmp_header.image_size=LSBFirstReadLong(image);
        bmp_header.x_pixels=LSBFirstReadLong(image);
        bmp_header.y_pixels=LSBFirstReadLong(image);
        bmp_header.number_colors=LSBFirstReadLong(image);
        bmp_header.colors_important=LSBFirstReadLong(image);
        for (i=0; i < ((int) bmp_header.size-40); i++)
          (void) ReadByte(image);
        if ((bmp_header.compression == 3) &&
            ((bmp_header.bits_per_pixel == 16) ||
             (bmp_header.bits_per_pixel == 32)))
          {
            bmp_header.red_mask=LSBFirstReadShort(image);
            bmp_header.green_mask=LSBFirstReadShort(image);
            bmp_header.blue_mask=LSBFirstReadShort(image);
            if (bmp_header.size > 40)
              {
                /*
                  Read color management information.
                */
                bmp_header.alpha_mask=LSBFirstReadShort(image);
                bmp_header.colorspace=LSBFirstReadLong(image);
                bmp_header.red_primary.x=LSBFirstReadLong(image);
                bmp_header.red_primary.y=LSBFirstReadLong(image);
                bmp_header.red_primary.z=LSBFirstReadLong(image);
                bmp_header.green_primary.x=LSBFirstReadLong(image);
                bmp_header.green_primary.y=LSBFirstReadLong(image);
                bmp_header.green_primary.z=LSBFirstReadLong(image);
                bmp_header.blue_primary.x=LSBFirstReadLong(image);
                bmp_header.blue_primary.y=LSBFirstReadLong(image);
                bmp_header.blue_primary.z=LSBFirstReadLong(image);
                bmp_header.gamma_scale.x=LSBFirstReadShort(image);
                bmp_header.gamma_scale.y=LSBFirstReadShort(image);
                bmp_header.gamma_scale.z=LSBFirstReadShort(image);
              }
          }
      }
    image->matte=bmp_header.bits_per_pixel == 32;
    image->columns=(unsigned int) bmp_header.width;
    image->rows=(unsigned int) AbsoluteValue(bmp_header.height);
    image->depth=8;
    if ((bmp_header.number_colors != 0) || (bmp_header.bits_per_pixel < 16))
      {
        image->class=PseudoClass;
        image->colors=(unsigned int) bmp_header.number_colors;
        if (image->colors == 0)
          image->colors=1 << bmp_header.bits_per_pixel;
      }
    if (image_info->ping)
      {
        CloseBlob(image);
        return(image);
      }
    if (image->class == PseudoClass)
      {
        unsigned char
          *bmp_colormap;

        unsigned int
          packet_size;

        /*
          Allocate image colormap.
        */
        image->colormap=(PixelPacket *)
          AllocateMemory(image->colors*sizeof(PixelPacket));
        if (image->colormap == (PixelPacket *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        /*
          Read BMP raster colormap.
        */
        bmp_colormap=(unsigned char *)
          AllocateMemory(4*image->colors*sizeof(unsigned char));
        if (bmp_colormap == (unsigned char *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        packet_size=4;
        if (bmp_header.size == 12)
          packet_size=3;
        (void) ReadBlob(image,packet_size*image->colors,
          (char *) bmp_colormap);
        p=bmp_colormap;
        for (i=0; i < (int) image->colors; i++)
        {
          image->colormap[i].blue=UpScale(*p++);
          image->colormap[i].green=UpScale(*p++);
          image->colormap[i].red=UpScale(*p++);
          if (bmp_header.size != 12)
            p++;
        }
        FreeMemory(bmp_colormap);
      }
    while (TellBlob(image) < (int) (start_position+bmp_header.offset_bits))
      (void) ReadByte(image);
    /*
      Read image data.
    */
    if (bmp_header.compression == 2)
      bmp_header.bits_per_pixel<<=1;
    bytes_per_line=((image->columns*bmp_header.bits_per_pixel+31)/32)*4;
    image_size=bytes_per_line*image->rows;
    bmp_pixels=(unsigned char *)
      AllocateMemory(image_size*sizeof(unsigned char));
    if (bmp_pixels == (unsigned char *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    if ((bmp_header.compression == 0) || (bmp_header.compression == 3))
      (void) ReadBlob(image,image_size,(char *) bmp_pixels);
    else
      {
        /*
          Convert run-length encoded raster pixels.
        */
        status=DecodeImage(image,(unsigned int) bmp_header.compression,
          (unsigned int) bmp_header.width,image->rows,bmp_pixels);
        if (status == False)
          ReaderExit(CorruptImageWarning,"runlength decoding failed",image);
      }
    /*
      Initialize image structure.
    */
    image->units=PixelsPerCentimeterResolution;
    image->x_resolution=bmp_header.x_pixels/100.0;
    image->y_resolution=bmp_header.y_pixels/100.0;
    /*
      Convert BMP raster image to pixel packets.
    */
    switch (bmp_header.bits_per_pixel)
    {
      case 1:
      {
        /*
          Convert bitmap scanline to runlength-encoded color packets.
        */
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < ((int) image->columns-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
            {
              index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
              image->indexes[x+bit]=index;
              *q++=image->colormap[index];
            }
            p++;
          }
          if ((image->columns % 8) != 0)
            {
              for (bit=0; bit < (int) (image->columns % 8); bit++)
              {
                index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
                image->indexes[x+bit]=index;
                *q++=image->colormap[index];
              }
              p++;
            }
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);
        }
        break;
      }
      case 4:
      {
        /*
          Convert PseudoColor scanline to runlength-encoded color packets.
        */
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < ((int) image->columns-1); x+=2)
          {
            index=(*p >> 4) & 0xf;
            image->indexes[x]=index;
            *q++=image->colormap[index];
            index=(*p) & 0xf;
            image->indexes[x+1]=index;
            *q++=image->colormap[index];
            p++;
          }
          if ((image->columns % 2) != 0)
            {
              index=(*p >> 4) & 0xf;
              image->indexes[x]=index;
              *q++=image->colormap[index];
              p++;
            }
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);
        }
        break;
      }
      case 8:
      {
        /*
          Convert PseudoColor scanline to runlength-encoded color packets.
        */
        if ((bmp_header.compression == 1) || (bmp_header.compression == 2))
          bytes_per_line=image->columns;
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            index=(*p++);
            image->indexes[x]=index;
            *q++=image->colormap[index];
          }
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);
        }
        break;
      }
      case 16:
      {
        unsigned char
          h,
          l;

        /*
          Convert PseudoColor scanline to runlength-encoded color packets.
        */
        image->class=DirectClass;
        if (bmp_header.compression == 1)
          bytes_per_line=image->columns << 1;
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            h=(*p++);
            l=(*p++);
            q->red=(MaxRGB*((int) (l & 0x7c) >> 2))/31;
            q->green=
              (MaxRGB*(((int) (l & 0x03) << 3)+((int) (h & 0xe0) >> 5)))/31;
            q->blue=(MaxRGB*((int) (h & 0x1f)))/31;
            q++;
          }
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);
        }
        break;
      }
      case 24:
      case 32:
      {
        /*
          Convert DirectColor scanline to runlength-encoded color packets.
        */
        for (y=image->rows-1; y >= 0; y--)
        {
          p=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            q->blue=UpScale(*p++);
            q->green=UpScale(*p++);
            q->red=UpScale(*p++);
            if (image->matte)
              q->opacity=Opaque-UpScale(*p++);
            q++;
          }
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);
        }
        break;
      }
      default:
        ReaderExit(CorruptImageWarning,"Not a BMP image file",image);
    }
    FreeMemory(bmp_pixels);
    if (bmp_header.height < 0)
      {
        Image
          *flipped_image;

        /*
          Correct image orientation.
        */
        flipped_image=FlipImage(image);
        if (flipped_image != (Image *) NULL)
          {
            DestroyImage(image);
            image=flipped_image;
          }
      }
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    *magick='\0';
    (void) ReadBlob(image,2,(char *) magick);
    if (strncmp((char *) magick,"BM",2) == 0)
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
  } while (strncmp((char *) magick,"BM",2) == 0);
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
%   W r i t e B M P I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteBMPImage writes an image in Microsoft Windows bitmap encoded
%  image format.
%
%  The format of the WriteBMPImage method is:
%
%      unsigned int WriteBMPImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteBMPImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteBMPImage(const ImageInfo *image_info,Image *image)
{
  typedef struct _BMPHeader
  {
    unsigned long
      file_size;

    unsigned short
      reserved[2];

    unsigned long
      offset_bits,
      size,
      width,
      height;

    unsigned short
      planes,
      bit_count;

    unsigned long
      compression,
      image_size,
      x_pixels,
      y_pixels,
      number_colors,
      colors_important;
  } BMPHeader;

  BMPHeader
    bmp_header;

  int
    y;

  register int
    i,
    x;

  register PixelPacket
    *p;

  register unsigned char
    *q;

  unsigned char
    *bmp_data,
    *bmp_pixels;

  unsigned int
    bytes_per_line,
    scene,
    status;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  scene=0;
  do
  {
    /*
      Initialize BMP raster file header.
    */
    TransformRGBImage(image,RGBColorspace);
    bmp_header.file_size=14+40;
    bmp_header.offset_bits=14+40;
    if ((Latin1Compare(image_info->magick,"BMP24") == 0) ||
        (!IsPseudoClass(image) && !IsGrayImage(image)))
      {
        /*
          Full color BMP raster.
        */
        image->class=DirectClass;
        bmp_header.number_colors=0;
        bmp_header.bit_count=image->matte ? 32 : 24;
        bytes_per_line=4*((image->columns*bmp_header.bit_count+31)/32);
      }
    else
      {
        /*
          Colormapped BMP raster.
        */
        bmp_header.bit_count=8;
        bytes_per_line=image->columns+(image->columns % 2 ? 1 : 0);
        if (IsMonochromeImage(image))
          {
            bmp_header.bit_count=1;
            bytes_per_line=4*((image->columns*bmp_header.bit_count+31)/32);
          }
        bmp_header.file_size+=4*(1 << bmp_header.bit_count);
        bmp_header.offset_bits+=4*(1 << bmp_header.bit_count);
        bmp_header.number_colors=1 << bmp_header.bit_count;
      }
    bmp_header.reserved[0]=0;
    bmp_header.reserved[1]=0;
    bmp_header.size=40;
    bmp_header.width=image->columns;
    bmp_header.height=image->rows;
    bmp_header.planes=1;
    bmp_header.compression=0;
    bmp_header.image_size=bytes_per_line*image->rows;
    bmp_header.file_size+=bmp_header.image_size;
    bmp_header.x_pixels=75*39;
    bmp_header.y_pixels=75*39;
    if (image->units == PixelsPerInchResolution)
      {
        bmp_header.x_pixels=(unsigned long) (100.0*image->x_resolution/2.54);
        bmp_header.y_pixels=(unsigned long) (100.0*image->y_resolution/2.54);
      }
    if (image->units == PixelsPerCentimeterResolution)
      {
        bmp_header.x_pixels=(unsigned long) (100.0*image->x_resolution);
        bmp_header.y_pixels=(unsigned long) (100.0*image->y_resolution);
      }
    bmp_header.colors_important=bmp_header.number_colors;
    /*
      Convert MIFF to BMP raster pixels.
    */
    bmp_pixels=(unsigned char *)
      AllocateMemory(bmp_header.image_size*sizeof(unsigned char));
    if (bmp_pixels == (unsigned char *) NULL)
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    switch (bmp_header.bit_count)
    {
      case 1:
      {
        register unsigned char
          bit,
          byte,
          polarity;

        /*
          Convert PseudoClass image to a BMP monochrome image.
        */
        polarity=Intensity(image->colormap[0]) > (MaxRGB >> 1);
        if (image->colors == 2)
          polarity=
            Intensity(image->colormap[1]) > Intensity(image->colormap[0]);
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          q=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          bit=0;
          byte=0;
          for (x=0; x < (int) image->columns; x++)
          {
            byte<<=1;
            if (image->indexes[x] == polarity)
              byte|=0x01;
            bit++;
            if (bit == 8)
              {
                *q++=byte;
                bit=0;
                byte=0;
              }
             p++;
           }
         if (bit != 0)
           *q++=byte << (8-bit);
         if (image->previous == (Image *) NULL)
           if (QuantumTick(y,image->rows))
             ProgressMonitor(SaveImageText,y,image->rows);
        }
        break;
      }
      case 8:
      {
        /*
          Convert PseudoClass packet to BMP pixel.
        */
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          q=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          for (x=0; x < (int) image->columns; x++)
          {
            *q++=image->indexes[x];
            p++;
          }
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
        }
        break;
      }
      case 24:
      case 32:
      {
        /*
          Convert DirectClass packet to BMP RGB pixel.
        */
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          q=bmp_pixels+(image->rows-y-1)*bytes_per_line;
          for (x=0; x < (int) image->columns; x++)
          {
            *q++=DownScale(p->blue);
            *q++=DownScale(p->green);
            *q++=DownScale(p->red);
            if (image->matte)
              *q++=Opaque-DownScale(p->opacity);
            p++;
          }
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
        }
        break;
      }
    }
    if (bmp_header.bit_count == 8)
      if (image_info->compression != NoCompression)
        {
          unsigned int
            packets;

          /*
            Convert run-length encoded raster pixels.
          */
          packets=(unsigned int)
            ((bytes_per_line*(bmp_header.height+2)+1) << 1);
          bmp_data=(unsigned char *)
            AllocateMemory(packets*sizeof(unsigned char));
          if (bmp_pixels == (unsigned char *) NULL)
            {
              FreeMemory(bmp_pixels);
              WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
            }
          bmp_header.file_size-=bmp_header.image_size;
          bmp_header.image_size=
            EncodeImage(bmp_pixels,image->columns,image->rows,bmp_data);
          bmp_header.file_size+=bmp_header.image_size;
          FreeMemory(bmp_pixels);
          bmp_pixels=bmp_data;
          bmp_header.compression=1;
        }
    /*
      Write BMP header.
    */
    (void) WriteBlob(image,2,"BM");
    (void) LSBFirstWriteLong(image,bmp_header.file_size);
    (void) LSBFirstWriteShort(image,bmp_header.reserved[0]);
    (void) LSBFirstWriteShort(image,bmp_header.reserved[1]);
    (void) LSBFirstWriteLong(image,bmp_header.offset_bits);
    (void) LSBFirstWriteLong(image,bmp_header.size);
    (void) LSBFirstWriteLong(image,bmp_header.width);
    (void) LSBFirstWriteLong(image,bmp_header.height);
    (void) LSBFirstWriteShort(image,bmp_header.planes);
    (void) LSBFirstWriteShort(image,bmp_header.bit_count);
    (void) LSBFirstWriteLong(image,bmp_header.compression);
    (void) LSBFirstWriteLong(image,bmp_header.image_size);
    (void) LSBFirstWriteLong(image,bmp_header.x_pixels);
    (void) LSBFirstWriteLong(image,bmp_header.y_pixels);
    (void) LSBFirstWriteLong(image,bmp_header.number_colors);
    (void) LSBFirstWriteLong(image,bmp_header.colors_important);
    if (image->class == PseudoClass)
      {
        unsigned char
          *bmp_colormap;

        /*
          Dump colormap to file.
        */
        bmp_colormap=(unsigned char *)
          AllocateMemory(4*(1 << bmp_header.bit_count)*sizeof(unsigned char));
        if (bmp_colormap == (unsigned char *) NULL)
          WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
        q=bmp_colormap;
        for (i=0; i < (int) image->colors; i++)
        {
          *q++=DownScale(image->colormap[i].blue);
          *q++=DownScale(image->colormap[i].green);
          *q++=DownScale(image->colormap[i].red);
          *q++=(Quantum) 0x0;
        }
        for ( ; i < (int) (1 << bmp_header.bit_count); i++)
        {
          *q++=(Quantum) 0x0;
          *q++=(Quantum) 0x0;
          *q++=(Quantum) 0x0;
          *q++=(Quantum) 0x0;
        }
        (void) WriteBlob(image,4*(1 << bmp_header.bit_count),
          (char *) bmp_colormap);
        FreeMemory(bmp_colormap);
      }
    (void) WriteBlob(image,bmp_header.image_size,(char *) bmp_pixels);
    FreeMemory(bmp_pixels);
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
