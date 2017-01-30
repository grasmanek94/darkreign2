/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            TTTTT   GGGG   AAA                               %
%                              T    G      A   A                              %
%                              T    G  GG  AAAAA                              %
%                              T    G   G  A   A                              %
%                              T     GGG   A   A                              %
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
%   R e a d T G A I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadTGAImage reads a Truevision TGA image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  The format of the ReadTGAImage method is:
%
%      Image *ReadTGAImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadTGAImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadTGAImage(const ImageInfo *image_info)
{
#define TGAColormap 1
#define TGARGB 2
#define TGAMonochrome 3
#define TGARLEColormap  9
#define TGARLERGB  10
#define TGARLEMonochrome  11

  typedef struct _TGAHeader
  {
    unsigned char
      id_length,
      colormap_type,
      image_type;

    unsigned short
      colormap_index,
      colormap_length;

    unsigned char
      colormap_size;

    unsigned short
      x_origin,
      y_origin,
      width,
      height;

    unsigned char
      bits_per_pixel,
      attributes;
  } TGAHeader;

  Image
    *image;

  IndexPacket
    index;

  int
    y;

  PixelPacket
    pixel;

  register int
    i,
    x;

  register PixelPacket
    *q;

  TGAHeader
    tga_header;

  unsigned char
    j,
    k,
    runlength;

  unsigned int
    base,
    flag,
    offset,
    real,
    skip,
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
    Read TGA header information.
  */
  status=ReadBlob(image,1,(char *) &tga_header.id_length);
  tga_header.colormap_type=ReadByte(image);
  tga_header.image_type=ReadByte(image);
  do
  {
    if ((status == False) || (tga_header.image_type == 0) ||
        (tga_header.image_type > 11))
      ReaderExit(CorruptImageWarning,"Not a TGA image file",image);
    tga_header.colormap_index=LSBFirstReadShort(image);
    tga_header.colormap_length=LSBFirstReadShort(image);
    tga_header.colormap_size=ReadByte(image);
    tga_header.x_origin=LSBFirstReadShort(image);
    tga_header.y_origin=LSBFirstReadShort(image);
    tga_header.width=LSBFirstReadShort(image);
    tga_header.height=LSBFirstReadShort(image);
    tga_header.bits_per_pixel=ReadByte(image);
    tga_header.attributes=ReadByte(image);
    /*
      Initialize image structure.
    */
    image->matte=tga_header.bits_per_pixel == 32;
    image->columns=tga_header.width;
    image->rows=tga_header.height;
    image->depth=tga_header.bits_per_pixel <= 8 ? 8 : QuantumDepth;
    if (tga_header.colormap_type != 0)
      {
        if ((tga_header.image_type == TGARLEColormap) ||
            (tga_header.image_type == TGARLERGB))
          image->class=PseudoClass;
        image->colors=tga_header.colormap_length;
      }
    if (image_info->ping)
      {
        CloseBlob(image);
        return(image);
      }
    if (tga_header.id_length != 0)
      {
        /*
          TGA image comment.
        */
        image->comments=(char *)
          AllocateMemory((tga_header.id_length+1)*sizeof(char));
        if (image->comments == (char *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        (void) ReadBlob(image,tga_header.id_length,image->comments);
        image->comments[tga_header.id_length]='\0';
      }
    GetPixelPacket(&pixel);
    if (tga_header.colormap_type != 0)
      {
        /*
          Read TGA raster colormap.
        */
        image->colormap=(PixelPacket *)
          AllocateMemory(image->colors*sizeof(PixelPacket));
        if (image->colormap == (PixelPacket *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        for (i=0; i < (int) image->colors; i++)
        {
          switch (tga_header.colormap_size)
          {
            case 8:
            default:
            {
              /*
                Gray scale.
              */
              pixel.red=UpScale(ReadByte(image));
              pixel.green=pixel.red;
              pixel.blue=pixel.red;
              break;
            }
            case 15:
            case 16:
            {
              /*
                5 bits each of red green and blue.
              */
              j=ReadByte(image);
              k=ReadByte(image);
              pixel.red=(Quantum) ((MaxRGB*((int) (k & 0x7c) >> 2))/31);
              pixel.green=(Quantum)
                ((MaxRGB*(((int) (k & 0x03) << 3)+((int) (j & 0xe0) >> 5)))/31);
              pixel.blue=(Quantum) ((MaxRGB*((int) (j & 0x1f)))/31);
              break;
            }
            case 24:
            case 32:
            {
              /*
                8 bits each of blue, green and red.
              */
              pixel.blue=UpScale(ReadByte(image));
              pixel.green=UpScale(ReadByte(image));
              pixel.red=UpScale(ReadByte(image));
              break;
            }
          }
          image->colormap[i]=pixel;
        }
      }
    /*
      Convert TGA pixels to pixel packets.
    */
    base=0;
    flag=0;
    skip=False;
    real=0;
    index=0;
    runlength=0;
    offset=0;
    for (y=0; y < (int) image->rows; y++)
    {
      real=offset;
      if (((unsigned char) (tga_header.attributes & 0x20) >> 5) == 0)
        real=image->rows-real-1;
      q=SetPixelCache(image,0,real,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      for (x=0; x < (int) image->columns; x++)
      {
        if ((tga_header.image_type == TGARLEColormap) ||
            (tga_header.image_type == TGARLERGB) ||
            (tga_header.image_type == TGARLEMonochrome))
          {
            if (runlength != 0)
              {
                runlength--;
                skip=flag != 0;
              }
            else
              {
                status=ReadBlob(image,1,(char *) &runlength);
                if (status == False)
                  ReaderExit(CorruptImageWarning,"Unable to read image data",
                    image);
                flag=runlength & 0x80;
                if (flag != 0)
                  runlength-=128;
                skip=False;
              }
          }
        if (!skip)
          switch (tga_header.bits_per_pixel)
          {
            case 8:
            default:
            {
              /*
                Gray scale.
              */
              index=ReadByte(image);
              if (tga_header.colormap_type != 0)
                pixel=image->colormap[index];
              else
                {
                  pixel.red=(Quantum) UpScale(index);
                  pixel.green=(Quantum) UpScale(index);
                  pixel.blue=(Quantum) UpScale(index);
                }
              break;
            }
            case 15:
            case 16:
            {
              /*
                5 bits each of red green and blue.
              */
              j=ReadByte(image);
              k=ReadByte(image);
              pixel.red=(Quantum) ((MaxRGB*((int) (k & 0x7c) >> 2))/31);
              pixel.green=(Quantum)
                ((MaxRGB*(((int) (k & 0x03) << 3)+((int) (j & 0xe0) >> 5)))/31);
              pixel.blue=(Quantum) ((MaxRGB*((int) (j & 0x1f)))/31);
              index=((unsigned short) k << 8)+j;
              break;
            }
            case 24:
            case 32:
            {
              /*
                8 bits each of blue green and red.
              */
              pixel.blue=UpScale(ReadByte(image));
              pixel.green=UpScale(ReadByte(image));
              pixel.red=UpScale(ReadByte(image));
              if (tga_header.bits_per_pixel == 32)
                pixel.opacity=UpScale(ReadByte(image));
              break;
            }
          }
        if (status == False)
          ReaderExit(CorruptImageWarning,"Unable to read image data",image);
        if (image->class == PseudoClass)
          image->indexes[x]=index;
        *q++=pixel;
      }
      if (((unsigned char) (tga_header.attributes & 0xc0) >> 6) == 4)
        offset+=4;
      else
        if (((unsigned char) (tga_header.attributes & 0xc0) >> 6) == 2)
          offset+=2;
        else
          offset++;
      if (offset >= image->rows)
        {
          base++;
          offset=base;
        }
      if (!SyncPixelCache(image))
        break;
      if (EOFBlob(image))
        break;
      if (image->previous == (Image *) NULL)
        if (QuantumTick(y,image->rows))
          ProgressMonitor(LoadImageText,y,image->rows);
    }
    (void) IsGrayImage(image);
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    status=ReadBlob(image,1,(char *) &tga_header.id_length);
    tga_header.colormap_type=ReadByte(image);
    tga_header.image_type=ReadByte(image);
    status&=((tga_header.image_type != 0) && (tga_header.image_type <= 11));
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e T G A I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteTGAImage writes a image in the Truevision Targa rasterfile
%  format.
%
%  The format of the WriteTGAImage method is:
%
%      unsigned int WriteTGAImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteTGAImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteTGAImage(const ImageInfo *image_info,Image *image)
{
#define TargaColormap 1
#define TargaRGB 2
#define TargaMonochrome 3
#define TargaRLEColormap  9
#define TargaRLERGB  10
#define TargaRLEMonochrome  11

  typedef struct _TargaHeader
  {
    unsigned char
      id_length,
      colormap_type,
      image_type;

    unsigned short
      colormap_index,
      colormap_length;

    unsigned char
      colormap_size;

    unsigned short
      x_origin,
      y_origin,
      width,
      height;

    unsigned char
      bits_per_pixel,
      attributes;
  } TargaHeader;

  int
    count,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p;

  register unsigned char
    *q;

  TargaHeader
    targa_header;

  unsigned char
    *targa_pixels;

  unsigned int
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
      Initialize TGA raster file header.
    */
    TransformRGBImage(image,RGBColorspace);
    targa_header.id_length=0;
    if (image->comments != (char *) NULL)
      targa_header.id_length=Min(Extent(image->comments),255);
    targa_header.colormap_type=0;
    targa_header.colormap_index=0;
    targa_header.colormap_length=0;
    targa_header.colormap_size=0;
    targa_header.x_origin=0;
    targa_header.y_origin=0;
    targa_header.width=image->columns;
    targa_header.height=image->rows;
    targa_header.bits_per_pixel=8;
    targa_header.attributes=0;
    if (!IsPseudoClass(image))
      {
        /*
          Full color TGA raster.
        */
        targa_header.image_type=TargaRGB;
        targa_header.bits_per_pixel=image->matte ? 32 : 24;
      }
    else
      {
        /*
          Colormapped TGA raster.
        */
        targa_header.image_type=TargaColormap;
        targa_header.colormap_type=1;
        targa_header.colormap_index=0;
        targa_header.colormap_length=image->colors;
        targa_header.colormap_size=24;
      }
    /*
      Write TGA header.
    */
    (void) WriteByte(image,(char) targa_header.id_length);
    (void) WriteByte(image,(char) targa_header.colormap_type);
    (void) WriteByte(image,(char) targa_header.image_type);
    LSBFirstWriteShort(image,targa_header.colormap_index);
    LSBFirstWriteShort(image,targa_header.colormap_length);
    (void) WriteByte(image,(char) targa_header.colormap_size);
    LSBFirstWriteShort(image,targa_header.x_origin);
    LSBFirstWriteShort(image,targa_header.y_origin);
    LSBFirstWriteShort(image,targa_header.width);
    LSBFirstWriteShort(image,targa_header.height);
    (void) WriteByte(image,(char) targa_header.bits_per_pixel);
    (void) WriteByte(image,(char) targa_header.attributes);
    if (targa_header.id_length != 0)
      (void) WriteBlob(image,targa_header.id_length,(char *) image->comments);
    if (IsPseudoClass(image))
      {
        unsigned char
          *targa_colormap;

        /*
          Dump colormap to file (blue, green, red byte order).
        */
        targa_colormap=(unsigned char *) AllocateMemory(3*
          targa_header.colormap_length*sizeof(unsigned char));
        if (targa_colormap == (unsigned char *) NULL)
          WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
        q=targa_colormap;
        for (i=0; i < (int) image->colors; i++)
        {
          *q++=DownScale(image->colormap[i].blue);
          *q++=DownScale(image->colormap[i].green);
          *q++=DownScale(image->colormap[i].red);
        }
        (void) WriteBlob(image,(int) 3*targa_header.colormap_length,
          (char *) targa_colormap);
        FreeMemory(targa_colormap);
      }
    /*
      Convert MIFF to TGA raster pixels.
    */
    count=(unsigned int) (targa_header.bits_per_pixel*targa_header.width) >> 3;
    targa_pixels=(unsigned char *) AllocateMemory(count*sizeof(unsigned char));
    if (targa_pixels == (unsigned char *) NULL)
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    for (y=(int) (image->rows-1); y >= 0; y--)
    {
      p=GetPixelCache(image,0,y,image->columns,1);
      if (p == (PixelPacket *) NULL)
        break;
      q=targa_pixels;
      for (x=0; x < (int) image->columns; x++)
      {
        if (image->class == PseudoClass)
          *q++=image->indexes[x];
        else
          {
            *q++=DownScale(p->blue);
            *q++=DownScale(p->green);
            *q++=DownScale(p->red);
            if (image->matte || (image->colorspace == CMYKColorspace))
              *q++=p->opacity;
          }
        p++;
      }
      status=WriteBlob(image,q-targa_pixels,(char *) targa_pixels);
      if (image->previous == (Image *) NULL)
        if (QuantumTick(y,image->rows))
          ProgressMonitor(SaveImageText,y,image->rows);
    }
    FreeMemory(targa_pixels);
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
