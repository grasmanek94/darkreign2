/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            X   X  W   W  DDDD                               %
%                             X X   W   W  D   D                              %
%                              X    W   W  D   D                              %
%                             X X   W W W  D   D                              %
%                            X   X   W W   DDDD                               %
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
#if defined(HasX11)
#include "xwindows.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s X W D                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsXWD returns True if the image format type, identified by the
%  magick string, is XWD.
%
%  The format of the ReadXWDImage method is:
%
%      unsigned int IsXWD(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsXWD returns True if the image format type is XWD.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsXWD(const unsigned char *magick,const unsigned int length)
{
  if (length < 8)
    return(False);
  if ((magick[1] == 0x00) && (magick[2] == 0x00))
    if ((magick[5] == 0x00) && (magick[6] == 0x00))
      if ((magick[4] == 0x07) || (magick[7] == 0x07))
        return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d X W D I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadXWDImage reads an X Window System window dump image file and
%  returns it.  It allocates the memory necessary for the new Image structure
%  and returns a pointer to the new image.
%
%  The format of the ReadXWDImage method is:
%
%      Image *ReadXWDImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadXWDImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadXWDImage(const ImageInfo *image_info)
{
  Image
    *image;

  IndexPacket
    index;

  int
    status,
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  register unsigned long
    pixel;

  unsigned long
    lsb_first,
    length;

  XColor
    *colors;

  XImage
    *ximage;

  XWDFileHeader
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
  /*
     Read in header information.
  */
  status=ReadBlob(image,sz_XWDheader,(char *) &header);
  if (status == False)
    ReaderExit(CorruptImageWarning,"Unable to read dump file header",image);
  image->columns=header.pixmap_width;
  image->rows=header.pixmap_height;
  image->depth=8;
  /*
    Ensure the header byte-order is most-significant byte first.
  */
  lsb_first=1;
  if (*(char *) &lsb_first)
    MSBFirstOrderLong((char *) &header,sz_XWDheader);
  /*
    Check to see if the dump file is in the proper format.
  */
  if (header.file_version != XWD_FILE_VERSION)
    ReaderExit(CorruptImageWarning,"XWD file format version mismatch",image);
  if (header.header_size < sz_XWDheader)
    ReaderExit(CorruptImageWarning,"XWD header size is too small",image);
  length=header.header_size-sz_XWDheader;
  image->comments=(char *) AllocateMemory((length+1)*sizeof(char));
  if (image->comments == (char *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  status=ReadBlob(image,length,(char *) image->comments);
  image->comments[length]='\0';
  if (status == False)
    ReaderExit(CorruptImageWarning,"Unable to read window name from dump file",
      image);
  /*
    Initialize the X image.
  */
  ximage=(XImage *) AllocateMemory(sizeof(XImage));
  if (ximage == (XImage *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  ximage->depth=header.pixmap_depth;
  ximage->format=header.pixmap_format;
  ximage->xoffset=header.xoffset;
  ximage->data=(char *) NULL;
  ximage->width=header.pixmap_width;
  ximage->height=header.pixmap_height;
  ximage->bitmap_pad=header.bitmap_pad;
  ximage->bytes_per_line=header.bytes_per_line;
  ximage->byte_order=header.byte_order;
  ximage->bitmap_unit=header.bitmap_unit;
  ximage->bitmap_bit_order=header.bitmap_bit_order;
  ximage->bits_per_pixel=header.bits_per_pixel;
  ximage->red_mask=header.red_mask;
  ximage->green_mask=header.green_mask;
  ximage->blue_mask=header.blue_mask;
  status=XInitImage(ximage);
  if (status == False)
    ReaderExit(CorruptImageWarning,"Invalid XWD header",image);
  /*
    Read colormap.
  */
  colors=(XColor *) NULL;
  if (header.ncolors != 0)
    {
      XWDColor
        color;

      colors=(XColor *)
        AllocateMemory((unsigned int) header.ncolors*sizeof(XColor));
      if (colors == (XColor *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (i=0; i < (int) header.ncolors; i++)
      {
        status=ReadBlob(image,sz_XWDColor,(char *) &color);
        if (status == False)
          ReaderExit(CorruptImageWarning,
            "Unable to read color map from dump file",image);
        colors[i].pixel=color.pixel;
        colors[i].red=color.red;
        colors[i].green=color.green;
        colors[i].blue=color.blue;
        colors[i].flags=color.flags;
      }
      /*
        Ensure the header byte-order is most-significant byte first.
      */
      lsb_first=1;
      if (*(char *) &lsb_first)
        for (i=0; i < (int) header.ncolors; i++)
        {
          MSBFirstOrderLong((char *) &colors[i].pixel,sizeof(unsigned long));
          MSBFirstOrderShort((char *) &colors[i].red,3*sizeof(unsigned short));
        }
    }
  /*
    Allocate the pixel buffer.
  */
  if (ximage->format == ZPixmap)
    length=ximage->bytes_per_line*ximage->height;
  else
    length=ximage->bytes_per_line*ximage->height*ximage->depth;
  ximage->data=(char *) AllocateMemory(length*sizeof(unsigned char));
  if (ximage->data == (char *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  status=ReadBlob(image,length,ximage->data);
  if (status == False)
    ReaderExit(CorruptImageWarning,"Unable to read dump pixmap",image);
  /*
    Convert image to MIFF format.
  */
  image->columns=ximage->width;
  image->rows=ximage->height;
  if ((colors == (XColor *) NULL) || (ximage->red_mask != 0) ||
      (ximage->green_mask != 0) || (ximage->blue_mask != 0))
    image->class=DirectClass;
  else
    image->class=PseudoClass;
  image->colors=header.ncolors;
  if (image_info->ping)
    {
      if (header.ncolors != 0)
        FreeMemory(colors);
      CloseBlob(image);
      return(image);
    }
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      register unsigned long
        color;

      unsigned long
        blue_mask,
        blue_shift,
        green_mask,
        green_shift,
        red_mask,
        red_shift;

      /*
        Determine shift and mask for red, green, and blue.
      */
      red_mask=ximage->red_mask;
      red_shift=0;
      while ((red_mask & 0x01) == 0)
      {
        red_mask>>=1;
        red_shift++;
      }
      green_mask=ximage->green_mask;
      green_shift=0;
      while ((green_mask & 0x01) == 0)
      {
        green_mask>>=1;
        green_shift++;
      }
      blue_mask=ximage->blue_mask;
      blue_shift=0;
      while ((blue_mask & 0x01) == 0)
      {
        blue_mask>>=1;
        blue_shift++;
      }
      /*
        Convert X image to DirectClass packets.
      */
      if (image->colors != 0)
        for (y=0; y < (int) image->rows; y++)
        {
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            pixel=XGetPixel(ximage,x,y);
            index=(unsigned short) ((pixel >> red_shift) & red_mask);
            q->red=XDownScale(colors[index].red);
            index=(unsigned short) ((pixel >> green_shift) & green_mask);
            q->green=XDownScale(colors[index].green);
            index=(unsigned short) ((pixel >> blue_shift) & blue_mask);
            q->blue=XDownScale(colors[index].blue);
            q++;
          }
          if (!SyncPixelCache(image))
            break;
          if (QuantumTick(y,image->rows))
            ProgressMonitor(LoadImageText,y,image->rows);
        }
      else
        for (y=0; y < (int) image->rows; y++)
        {
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            pixel=XGetPixel(ximage,x,y);
            color=(pixel >> red_shift) & red_mask;
            q->red=XDownScale((color*65535L)/red_mask);
            color=(pixel >> green_shift) & green_mask;
            q->green=XDownScale((color*65535L)/green_mask);
            color=(pixel >> blue_shift) & blue_mask;
            q->blue=XDownScale((color*65535L)/blue_mask);
            q++;
          }
          if (!SyncPixelCache(image))
            break;
          if (QuantumTick(y,image->rows))
            ProgressMonitor(LoadImageText,y,image->rows);
        }
      break;
    }
    case PseudoClass:
    {
      /*
        Convert X image to PseudoClass packets.
      */
      image->colormap=(PixelPacket *)
        AllocateMemory(image->colors*sizeof(PixelPacket));
      if (image->colormap == (PixelPacket *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (i=0; i < (int) image->colors; i++)
      {
        image->colormap[i].red=XDownScale(colors[i].red);
        image->colormap[i].green=XDownScale(colors[i].green);
        image->colormap[i].blue=XDownScale(colors[i].blue);
      }
      for (y=0; y < (int) image->rows; y++)
      {
        q=SetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          index=XGetPixel(ximage,x,y);
          image->indexes[x]=index;
          if (index >= image->colors)
            ReaderExit(CorruptImageWarning,"invalid colormap index",image);
          *q++=image->colormap[index];
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(LoadImageText,y,image->rows);
      }
      break;
    }
  }
  /*
    Free image and colormap.
  */
  if (header.ncolors != 0)
    FreeMemory(colors);
  FreeMemory(ximage->data);
  FreeMemory(ximage);
  CloseBlob(image);
  return(image);
}
#else
Export Image *ReadXWDImage(const ImageInfo *image_info)
{
  MagickWarning(MissingDelegateWarning,"X11 library is not available",
    image_info->filename);
  return((Image *) NULL);
}
#endif

#if defined(HasX11)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e X W D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteXWDImage writes an image to a file in X window dump
%  rasterfile format.
%
%  The format of the WriteXWDImage method is:
%
%      unsigned int WriteXWDImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteXWDImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteXWDImage(const ImageInfo *image_info,Image *image)
{
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
    *pixels;

  unsigned int
    bits_per_pixel,
    bytes_per_line,
    scanline_pad,
    status;

  unsigned long
    lsb_first;

  XWDFileHeader
    xwd_header;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  TransformRGBImage(image,RGBColorspace);
  /*
    Initialize XWD file header.
  */
  xwd_header.header_size=sz_XWDheader+Extent(image->filename)+1;
  xwd_header.file_version=(CARD32) XWD_FILE_VERSION;
  xwd_header.pixmap_format=(CARD32) ZPixmap;
  xwd_header.pixmap_depth=(CARD32) (image->class == DirectClass ? 24 : 8);
  xwd_header.pixmap_width=(CARD32) image->columns;
  xwd_header.pixmap_height=(CARD32) image->rows;
  xwd_header.xoffset=(CARD32) 0;
  xwd_header.byte_order=(CARD32) MSBFirst;
  xwd_header.bitmap_unit=(CARD32) (image->class == DirectClass ? 32 : 8);
  xwd_header.bitmap_bit_order=(CARD32) MSBFirst;
  xwd_header.bitmap_pad=(CARD32) (image->class == DirectClass ? 32 : 8);
  bits_per_pixel=(image->class == DirectClass ? 24 : 8);
  xwd_header.bits_per_pixel=(CARD32) bits_per_pixel;
  bytes_per_line=(CARD32) ((((xwd_header.bits_per_pixel*
    xwd_header.pixmap_width)+((xwd_header.bitmap_pad)-1))/
    (xwd_header.bitmap_pad))*((xwd_header.bitmap_pad) >> 3));
  xwd_header.bytes_per_line=(CARD32) bytes_per_line;
  xwd_header.visual_class=(CARD32)
    (image->class == DirectClass ? DirectColor : PseudoColor);
  xwd_header.red_mask=(CARD32) (image->class == DirectClass ? 0xff0000 : 0);
  xwd_header.green_mask=(CARD32) (image->class == DirectClass ? 0xff00 : 0);
  xwd_header.blue_mask=(CARD32) (image->class == DirectClass ? 0xff : 0);
  xwd_header.bits_per_rgb=(CARD32) (image->class == DirectClass ? 24 : 8);
  xwd_header.colormap_entries=(CARD32)
    (image->class == DirectClass ? 256 : image->colors);
  xwd_header.ncolors=(image->class == DirectClass ? 0 : image->colors);
  xwd_header.window_width=(CARD32) image->columns;
  xwd_header.window_height=(CARD32) image->rows;
  xwd_header.window_x=0;
  xwd_header.window_y=0;
  xwd_header.window_bdrwidth=(CARD32) 0;
  /*
    Write XWD header.
  */
  lsb_first=1;
  if (*(char *) &lsb_first)
    MSBFirstOrderLong((char *) &xwd_header,sizeof(xwd_header));
  (void) WriteBlob(image,sz_XWDheader,(char *) &xwd_header);
  (void) WriteBlob(image,Extent(image->filename)+1,(char *) image->filename);
  if (image->class == PseudoClass)
    {
      XColor
        *colors;

      XWDColor
        color;

      /*
        Dump colormap to file.
      */
      colors=(XColor *) AllocateMemory(image->colors*sizeof(XColor));
      if (colors == (XColor *) NULL)
        WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (i=0; i < (int) image->colors; i++)
      {
        colors[i].pixel=i;
        colors[i].red=XUpScale(image->colormap[i].red);
        colors[i].green=XUpScale(image->colormap[i].green);
        colors[i].blue=XUpScale(image->colormap[i].blue);
        colors[i].flags=DoRed | DoGreen | DoBlue;
        colors[i].pad=0;
        if (*(char *) &lsb_first)
          {
            MSBFirstOrderLong((char *) &colors[i].pixel,sizeof(long));
            MSBFirstOrderShort((char *) &colors[i].red,3*sizeof(short));
          }
      }
      for (i=0; i < (int) image->colors; i++)
      {
        color.pixel=(unsigned long) colors[i].pixel;
        color.red=colors[i].red;
        color.green=colors[i].green;
        color.blue=colors[i].blue;
        color.flags=colors[i].flags;
        (void) WriteBlob(image,sz_XWDColor,(char *) &color);
      }
      FreeMemory(colors);
    }
  /*
    Allocate memory for pixels.
  */
  pixels=(unsigned char *)
    AllocateMemory(image->columns*sizeof(PixelPacket));
  if (pixels == (unsigned char *) NULL)
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  /*
    Convert MIFF to XWD raster pixels.
  */
  scanline_pad=(unsigned int)
    (bytes_per_line-((image->columns*bits_per_pixel) >> 3));
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    q=pixels;
    for (x=0; x < (int) image->columns; x++)
    {
      if (image->class == PseudoClass)
        *q++=image->indexes[x];
      else
        {
          *q++=DownScale(p->red);
          *q++=DownScale(p->green);
          *q++=DownScale(p->blue);
        }
      p++;
    }
    for (x=0; x < (int) scanline_pad; x++)
      *q++=0;
    (void) WriteBlob(image,q-pixels,(char *) pixels);
    if (image->previous == (Image *) NULL)
      if (QuantumTick(y,image->rows))
        ProgressMonitor(SaveImageText,y,image->rows);
  }
  FreeMemory(pixels);
  CloseBlob(image);
  return(True);
}
#else
Export unsigned int WriteXWDImage(const ImageInfo *image_info,Image *image)
{
  MagickWarning(MissingDelegateWarning,"X11 library is not available",
    image->filename);
  return(False);
}
#endif
