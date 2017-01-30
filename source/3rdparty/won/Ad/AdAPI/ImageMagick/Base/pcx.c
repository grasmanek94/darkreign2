/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP    CCCC  X   X                              %
%                            P   P  C       X X                               %
%                            PPPP   C        X                                %
%                            P      C       X X                               %
%                            P       CCCC  X   X                              %
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
%   I s D C X                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsDCX returns True if the image format type, identified by the
%  magick string, is DCX.
%
%  The format of the ReadDCXImage method is:
%
%      unsigned int IsDCX(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsDCX returns True if the image format type is DCX.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsDCX(const unsigned char *magick,const unsigned int length)
{
  if (length < 4)
    return(False);
  if (strncmp((char *) magick,"\261\150\336\72",4) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P C X                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsPCX returns True if the image format type, identified by the
%  magick string, is PCX.
%
%  The format of the ReadPCXImage method is:
%
%      unsigned int IsPCX(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsPCX returns True if the image format type is PCX.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsPCX(const unsigned char *magick,const unsigned int length)
{
  if (length < 2)
    return(False);
  if (strncmp((char *) magick,"\12\2",2) == 0)
    return(True);
  if (strncmp((char *) magick,"\12\5",2) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P C X I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadPCXImage reads a ZSoft IBM PC Paintbrush file and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadPCXImage method is:
%
%      Image *ReadPCXImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPCXImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadPCXImage(const ImageInfo *image_info)
{
  typedef struct _PCXHeader
  {
    unsigned char
      identifier,
      version,
      encoding,
      bits_per_pixel;

    short int
      left,
      top,
      right,
      bottom,
      horizontal_resolution,
      vertical_resolution;

    unsigned char
      reserved,
      planes;

    short int
      bytes_per_line,
      palette_info;

    unsigned char
      colormap_signature;
  } PCXHeader;

  PCXHeader
    pcx_header;

  Image
    *image;

  int
    bits,
    count,
    id,
    mask,
    pcx_packets,
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p,
    *r;

  unsigned char
    packet,
    *pcx_colormap,
    *pcx_pixels,
    *scanline;

  unsigned int
    status;

  unsigned long
    *page_table;

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
    Determine if this is a PCX file.
  */
  page_table=(unsigned long *) NULL;
  if (Latin1Compare(image_info->magick,"DCX") == 0)
    {
      unsigned long
        magic;

      /*
        Read the DCX page table.
      */
      magic=LSBFirstReadLong(image);
      if (magic != 987654321)
        ReaderExit(CorruptImageWarning,"Not a DCX image file",image);
      page_table=(unsigned long *) AllocateMemory(1024*sizeof(unsigned long));
      if (page_table == (unsigned long *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (id=0; id < 1024; id++)
      {
        page_table[id]=LSBFirstReadLong(image);
        if (page_table[id] == 0)
          break;
      }
    }
  if (page_table != (unsigned long *) NULL)
    (void) SeekBlob(image,page_table[0],SEEK_SET);
  status=ReadBlob(image,1,(char *) &pcx_header.identifier);
  for (id=1; id < 1024; id++)
  {
    /*
      Verify PCX identifier.
    */
    pcx_header.version=ReadByte(image);
    if ((status == False) || (pcx_header.identifier != 0x0a) ||
        ((pcx_header.version != 2) && (pcx_header.version != 3) &&
         (pcx_header.version != 5)))
      ReaderExit(CorruptImageWarning,"Not a PCX image file",image);
    pcx_header.encoding=ReadByte(image);
    pcx_header.bits_per_pixel=ReadByte(image);
    pcx_header.left=LSBFirstReadShort(image);
    pcx_header.top=LSBFirstReadShort(image);
    pcx_header.right=LSBFirstReadShort(image);
    pcx_header.bottom=LSBFirstReadShort(image);
    pcx_header.horizontal_resolution=LSBFirstReadShort(image);
    pcx_header.vertical_resolution=LSBFirstReadShort(image);
    /*
      Read PCX raster colormap.
    */
    image->columns=(pcx_header.right-pcx_header.left)+1;
    image->rows=(pcx_header.bottom-pcx_header.top)+1;
    image->depth=pcx_header.bits_per_pixel <= 8 ? 8 : QuantumDepth;
    image->units=PixelsPerInchResolution;
    image->x_resolution=pcx_header.horizontal_resolution;
    image->y_resolution=pcx_header.vertical_resolution;
    image->colors=16;
    pcx_colormap=(unsigned char *) AllocateMemory(3*256*sizeof(unsigned char));
    if (pcx_colormap == (unsigned char *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    (void) ReadBlob(image,3*image->colors,(char *) pcx_colormap);
    pcx_header.reserved=ReadByte(image);
    pcx_header.planes=ReadByte(image);
    if ((pcx_header.bits_per_pixel != 8) || (pcx_header.planes == 1))
      {
        image->class=PseudoClass;
        if ((pcx_header.version == 3) || (pcx_header.version == 5))
          image->colors=1 << (pcx_header.bits_per_pixel*pcx_header.planes);
      }
    if (image_info->ping)
      {
        FreeMemory(pcx_colormap);
        if (page_table != (unsigned long *) NULL)
          FreeMemory(page_table);
        CloseBlob(image);
        return(image);
      }
    image->colormap=(PixelPacket *) AllocateMemory(256*sizeof(PixelPacket));
    if (image->colormap == (PixelPacket *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    p=pcx_colormap;
    for (i=0; i < (int) image->colors; i++)
    {
      image->colormap[i].red=UpScale(*p++);
      image->colormap[i].green=UpScale(*p++);
      image->colormap[i].blue=UpScale(*p++);
    }
    if (pcx_header.version == 3)
      for (i=0; i < (int) image->colors; i++)
      {
        image->colormap[i].red=(Quantum) ((long) (MaxRGB*i)/(image->colors-1));
        image->colormap[i].green=(Quantum)
          ((long) (MaxRGB*i)/(image->colors-1));
        image->colormap[i].blue=(Quantum) ((long) (MaxRGB*i)/(image->colors-1));
      }
    pcx_header.bytes_per_line=LSBFirstReadShort(image);
    pcx_header.palette_info=LSBFirstReadShort(image);
    for (i=0; i < 58; i++)
      (void) ReadByte(image);
    /*
      Read image data.
    */
    pcx_packets=image->rows*pcx_header.bytes_per_line*pcx_header.planes;
    pcx_pixels=(unsigned char *)
      AllocateMemory(pcx_packets*sizeof(unsigned char));
    scanline=(unsigned char *)
      AllocateMemory(image->columns*pcx_header.planes*sizeof(unsigned char));
    if ((pcx_pixels == (unsigned char *) NULL) ||
        (scanline == (unsigned char *) NULL))
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    /*
      Uncompress image data.
    */
    p=pcx_pixels;
    while (pcx_packets > 0)
    {
      packet=ReadByte(image);
      if ((packet & 0xc0) != 0xc0)
        {
          *p++=packet;
          pcx_packets--;
          continue;
        }
      count=packet & 0x3f;
      for (packet=ReadByte(image); count > 0; count--)
      {
        *p++=packet;
        pcx_packets--;
        if (pcx_packets == 0)
          break;
      }
    }
    if (image->class == DirectClass)
      image->matte=pcx_header.planes > 3;
    else
      if (pcx_header.version == 5)
        {
          /*
            Initialize image colormap.
          */
          if (image->colors > 256)
            ReaderExit(CorruptImageWarning,"PCX colormap exceeded 256 colors",
              image);
          if ((pcx_header.bits_per_pixel*pcx_header.planes) == 1)
            {
              /*
                Monochrome colormap.
              */
              image->colormap[0].red=0;
              image->colormap[0].green=0;
              image->colormap[0].blue=0;
              image->colormap[1].red=MaxRGB;
              image->colormap[1].green=MaxRGB;
              image->colormap[1].blue=MaxRGB;
            }
          else
            if (image->colors > 16)
              {
                /*
                  256 color images have their color map at the end of the file.
                */
                pcx_header.colormap_signature=ReadByte(image);
                (void) ReadBlob(image,3*image->colors,(char *) pcx_colormap);
                p=pcx_colormap;
                for (i=0; i < (int) image->colors; i++)
                {
                  image->colormap[i].red=UpScale(*p++);
                  image->colormap[i].green=UpScale(*p++);
                  image->colormap[i].blue=UpScale(*p++);
                }
            }
          FreeMemory(pcx_colormap);
        }
    /*
      Convert PCX raster image to pixel packets.
    */
    for (y=0; y < (int) image->rows; y++)
    {
      p=pcx_pixels+(y*pcx_header.bytes_per_line*pcx_header.planes);
      q=SetPixelCache(image,0,y,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      r=scanline;
      if (image->class == DirectClass)
        for (i=0; i < (int) pcx_header.planes; i++)
        {
          r=scanline+i;
          for (x=0; x < pcx_header.bytes_per_line; x++)
          {
            switch (i)
            {
              case 0:
              {
                *r=UpScale(*p++);
                break;
              }
              case 1:
              {
                *r=UpScale(*p++);
                break;
              }
              case 2:
              {
                *r=UpScale(*p++);
                break;
              }
              case 3:
              default:
              {
                *r=UpScale(*p++);
                break;
              }
            }
            r+=pcx_header.planes;
          }
        }
      else
        if (pcx_header.planes > 1)
          {
            for (x=0; x < (int) image->columns; x++)
              *r++=0;
            for (i=0; i < (int) pcx_header.planes; i++)
            {
              r=scanline;
              for (x=0; x < pcx_header.bytes_per_line; x++)
              {
                 bits=(*p++);
                 for (mask=0x80; mask != 0; mask>>=1)
                 {
                   if (bits & mask)
                     *r|=1 << i;
                   r++;
                 }
               }
            }
          }
        else
          switch (pcx_header.bits_per_pixel)
          {
            case 1:
            {
              register int
                bit;

              for (x=0; x < ((int) image->columns-7); x+=8)
              {
                for (bit=7; bit >= 0; bit--)
                  *r++=((*p) & (0x01 << bit) ? 0x01 : 0x00);
                p++;
              }
              if ((image->columns % 8) != 0)
                {
                  for (bit=7; bit >= (int) (8-(image->columns % 8)); bit--)
                    *r++=((*p) & (0x01 << bit) ? 0x01 : 0x00);
                  p++;
                }
              break;
            }
            case 2:
            {
              for (x=0; x < ((int) image->columns-3); x+=4)
              {
                *r++=(*p >> 6) & 0x3;
                *r++=(*p >> 4) & 0x3;
                *r++=(*p >> 2) & 0x3;
                *r++=(*p) & 0x3;
                p++;
              }
              if ((image->columns % 4) != 0)
                {
                  for (i=3; i >= (int) (4-(image->columns % 4)); i--)
                    *r++=(*p >> (i*2)) & 0x03;
                  p++;
                }
              break;
            }
            case 4:
            {
              for (x=0; x < ((int) image->columns-1); x+=2)
              {
                *r++=(*p >> 4) & 0xf;
                *r++=(*p) & 0xf;
                p++;
              }
              if ((image->columns % 2) != 0)
                *r++=(*p++ >> 4) & 0xf;
              break;
            }
            case 8:
            {
              (void) memcpy(r,p,image->columns*sizeof(unsigned char));
              break;
            }
            default:
              break;
          }
      /*
        Transfer image scanline.
      */
      r=scanline;
      for (x=0; x < (int) image->columns; x++)
      {
        if (image->class == PseudoClass)
          image->indexes[x]=(*r++);
        else
          {
            q->red=UpScale(*r++);
            q->green=UpScale(*r++);
            q->blue=UpScale(*r++);
            if (image->matte)
              q->opacity=UpScale(*r++);
          }
        q++;
      }
      if (!SyncPixelCache(image))
        break;
      if (image->previous == (Image *) NULL)
        if (QuantumTick(y,image->rows))
          ProgressMonitor(LoadImageText,y,image->rows);
    }
    if (image->class == PseudoClass)
      SyncImage(image);
    FreeMemory(scanline);
    FreeMemory(pcx_pixels);
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    if (page_table == (unsigned long *) NULL)
      break;
    if (page_table[id] == 0)
      break;
    (void) SeekBlob(image,page_table[id],SEEK_SET);
    status=ReadBlob(image,1,(char *) &pcx_header.identifier);
    if ((status == True) && (pcx_header.identifier == 0x0a))
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
  }
  if (page_table != (unsigned long *) NULL)
    FreeMemory(page_table);
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
%   W r i t e P C X I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WritePCXImage writes an image in the ZSoft IBM PC Paintbrush file
%  format.
%
%  The format of the WritePCXImage method is:
%
%      unsigned int WritePCXImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WritePCXImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WritePCXImage(const ImageInfo *image_info,Image *image)
{
  typedef struct _PCXHeader
  {
    unsigned char
      identifier,
      version,
      encoding,
      bits_per_pixel;

    short int
      left,
      top,
      right,
      bottom,
      horizontal_resolution,
      vertical_resolution;

    unsigned char
      reserved,
      planes;

    short int
      bytes_per_line,
      palette_info;

    unsigned char
      colormap_signature;
  } PCXHeader;

  int
    y;

  PCXHeader
    pcx_header;

  register int
    i,
    x;

  register PixelPacket
    *p;

  register unsigned char
    *q;

  unsigned char
    count,
    packet,
    *pcx_colormap,
    *pcx_pixels,
    previous;

  unsigned int
    packets,
    scene,
    status;

  unsigned long
    *page_table;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  TransformRGBImage(image,RGBColorspace);
  page_table=(unsigned long *) NULL;
  if (image_info->adjoin)
    {
      /*
        Write the DCX page table.
      */
      LSBFirstWriteLong(image,0x3ADE68B1L);
      page_table=(unsigned long *) AllocateMemory(1024*sizeof(unsigned long));
      if (page_table == (unsigned long *) NULL)
        WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (scene=0; scene < 1024; scene++)
        LSBFirstWriteLong(image,0x00000000L);
    }
  scene=0;
  do
  {
    if (page_table != (unsigned long *) NULL)
      page_table[scene]=TellBlob(image);
    /*
      Initialize PCX raster file header.
    */
    pcx_header.identifier=0x0a;
    pcx_header.version=5;
    pcx_header.encoding=1;
    pcx_header.bits_per_pixel=8;
    if (IsMonochromeImage(image))
      pcx_header.bits_per_pixel=1;
    pcx_header.left=0;
    pcx_header.top=0;
    pcx_header.right=image->columns-1;
    pcx_header.bottom=image->rows-1;
    pcx_header.horizontal_resolution=(short) image->columns;
    pcx_header.vertical_resolution=(short) image->rows;
    if (image->units == PixelsPerInchResolution)
      {
        pcx_header.horizontal_resolution=(short) image->x_resolution;
        pcx_header.vertical_resolution=(short) image->y_resolution;
      }
    if (image->units == PixelsPerCentimeterResolution)
      {
        pcx_header.horizontal_resolution=(short) (2.54*image->x_resolution);
        pcx_header.vertical_resolution=(short) (2.54*image->y_resolution);
      }
    pcx_header.reserved=0;
    pcx_header.planes=1;
    if (!IsPseudoClass(image))
      {
        pcx_header.planes=3;
        if (image->matte)
          pcx_header.planes++;
      }
    pcx_header.bytes_per_line=(image->columns*pcx_header.bits_per_pixel+7)/8;
    pcx_header.palette_info=1;
    pcx_header.colormap_signature=0x0c;
    /*
      Write PCX header.
    */
    (void) WriteByte(image,pcx_header.identifier);
    (void) WriteByte(image,pcx_header.version);
    (void) WriteByte(image,pcx_header.encoding);
    (void) WriteByte(image,pcx_header.bits_per_pixel);
    LSBFirstWriteShort(image,(unsigned int) pcx_header.left);
    LSBFirstWriteShort(image,(unsigned int) pcx_header.top);
    LSBFirstWriteShort(image,(unsigned int) pcx_header.right);
    LSBFirstWriteShort(image,(unsigned int) pcx_header.bottom);
    LSBFirstWriteShort(image,(unsigned int) pcx_header.horizontal_resolution);
    LSBFirstWriteShort(image,(unsigned int) pcx_header.vertical_resolution);
    /*
      Dump colormap to file.
    */
    pcx_colormap=(unsigned char *) AllocateMemory(3*256*sizeof(unsigned char));
    if (pcx_colormap == (unsigned char *) NULL)
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    for (i=0; i < (3*256); i++)
      pcx_colormap[i]=0;
    q=pcx_colormap;
    if (image->class == PseudoClass)
      for (i=0; i < (int) image->colors; i++)
      {
        *q++=DownScale(image->colormap[i].red);
        *q++=DownScale(image->colormap[i].green);
        *q++=DownScale(image->colormap[i].blue);
      }
    (void) WriteBlob(image,3*16,(char *) pcx_colormap);
    (void) WriteByte(image,pcx_header.reserved);
    (void) WriteByte(image,pcx_header.planes);
    LSBFirstWriteShort(image,(unsigned int) pcx_header.bytes_per_line);
    LSBFirstWriteShort(image,(unsigned int) pcx_header.palette_info);
    for (i=0; i < 58; i++)
      (void) WriteByte(image,'\0');
    packets=image->rows*pcx_header.bytes_per_line*pcx_header.planes;
    pcx_pixels=(unsigned char *) AllocateMemory(packets*sizeof(unsigned char));
    if (pcx_pixels == (unsigned char *) NULL)
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    q=pcx_pixels;
    if (image->class == DirectClass)
      {
        /*
          Convert DirectClass image to PCX raster pixels.
        */
        for (y=0; y < (int) image->rows; y++)
        {
          q=pcx_pixels+(y*pcx_header.bytes_per_line*pcx_header.planes);
          for (i=0; i < (int) pcx_header.planes; i++)
          {
            p=GetPixelCache(image,0,y,image->columns,1);
            if (p == (PixelPacket *) NULL)
              break;
            for (x=0; x < pcx_header.bytes_per_line; x++)
            {
              switch (i)
              {
                case 0:
                {
                  *q++=DownScale(p->red);
                  break;
                }
                case 1:
                {
                  *q++=DownScale(p->green);
                  break;
                }
                case 2:
                {
                  *q++=DownScale(p->blue);
                  break;
                }
                case 3:
                default:
                {
                  *q++=DownScale(p->opacity);
                  break;
                }
              }
              p++;
            }
          }
          if (QuantumTick(y,image->rows))
            ProgressMonitor(SaveImageText,y,image->rows);
        }
      }
    else
      if (pcx_header.bits_per_pixel > 1)
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          q=pcx_pixels+y*pcx_header.bytes_per_line;
          for (x=0; x < (int) image->columns; x++)
            *q++=image->indexes[x];
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
        }
      else
        {
          register unsigned char
            bit,
            byte,
            polarity;

          /*
            Convert PseudoClass image to a PCX monochrome image.
          */
          polarity=Intensity(image->colormap[0]) > (MaxRGB >> 1);
          if (image->colors == 2)
            polarity=Intensity(image->colormap[0]) <
              Intensity(image->colormap[1]);
          for (y=0; y < (int) image->rows; y++)
          {
            if (!GetPixelCache(image,0,y,image->columns,1))
              break;
            bit=0;
            byte=0;
            q=pcx_pixels+y*pcx_header.bytes_per_line;
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
        }
    /*
      Runlength-encoded PCX pixels.
    */
    for (y=0; y < (int) image->rows; y++)
    {
      q=pcx_pixels+(y*pcx_header.bytes_per_line*pcx_header.planes);
      for (i=0; i < (int) pcx_header.planes; i++)
      {
        previous=(*q++);
        count=1;
        for (x=0; x < (pcx_header.bytes_per_line-1); x++)
        {
          packet=(*q++);
          if ((packet == previous) && (count < 63))
            {
              count++;
              continue;
            }
          if ((count > 1) || ((previous & 0xc0) == 0xc0))
            {
              count|=0xc0;
              (void) WriteByte(image,count);
            }
          (void) WriteByte(image,previous);
          previous=packet;
          count=1;
        }
        if ((count > 1) || ((previous & 0xc0) == 0xc0))
          {
            count|=0xc0;
            (void) WriteByte(image,count);
          }
        (void) WriteByte(image,previous);
        if (QuantumTick(y,image->rows))
          ProgressMonitor(SaveImageText,y,image->rows);
      }
    }
    (void) WriteByte(image,pcx_header.colormap_signature);
    (void) WriteBlob(image,3*256,(char *) pcx_colormap);
    FreeMemory(pcx_pixels);
    FreeMemory(pcx_colormap);
    if (image->next == (Image *) NULL)
      break;
    image=GetNextImage(image);
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
    if (scene >= 1023)
      break;
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  if (page_table != (unsigned long *) NULL)
    {
      /*
        Write the DCX page table.
      */
      page_table[scene+1]=0;
      (void) SeekBlob(image,0L,SEEK_SET);
      LSBFirstWriteLong(image,0x3ADE68B1L);
      for (i=0; i <= (int) scene; i++)
        LSBFirstWriteLong(image,page_table[i]);
      FreeMemory(page_table);
    }
  CloseBlob(image);
  return(True);
}
