/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   SSSSS  DDDD                               %
%                            P   P  SS     D   D                              %
%                            PPPP    SSS   D   D                              %
%                            P         SS  D   D                              %
%                            P      SSSSS  DDDD                               %
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
%  Method DecodeImage uncompresses an image via Macintosh encoding specific to
%  the Adobe Photoshop image format.
%
%  The format of the DecodeImage method is:
%
%      unsigned int DecodeImage(Image *image,const int channel)
%
%  A description of each parameter follows:
%
%    o status: Method DecodeImage return True if the image is
%      decoded.  False is returned if there is an error occurs.
%
%    o image,image: The address of a structure of type Image.
%
%    o channel:  Specifies which channel: red, green, blue, or index to
%      decode the pixel values into.
%
%
*/
static unsigned int DecodeImage(Image *image,const int channel)
{
  int
    count,
    pixel;

  long
    length;

  register int
    i,
    x;

  register PixelPacket
    *q;

  x=0;
  length=image->columns*image->rows;
  while (length > 0)
  {
    count=ReadByte(image);
    if (count >= 128)
      count-=256;
    if (count < 0)
      {
        if (count == -128)
          continue;
        pixel=ReadByte(image);
        for (count=(-count+1); count > 0; count--)
        {
          q=SetPixelCache(image,x % image->columns,x/image->columns,1,1);
          if (q == (PixelPacket *) NULL)
            break;
          switch (channel)
          {
            case 0:
            {
              q->red=pixel;
              if (image->class == PseudoClass)
                {
                  *image->indexes=pixel;
                  *q=image->colormap[pixel];
                }
              break;
            }
            case 1:
            {
              q->green=pixel;
              break;
            }
            case 2:
            {
              q->blue=pixel;
              break;
            }
            case 3:
            default:
            {
              q->opacity=pixel;
              break;
            }
          }
          if (!SyncPixelCache(image))
            break;
          x++;
          length--;
        }
        continue;
      }
    count++;
    for (i=count; i > 0; i--)
    {
      pixel=ReadByte(image);
      q=SetPixelCache(image,x % image->columns,x/image->columns,1,1);
      if (q == (PixelPacket *) NULL)
        break;
      switch (channel)
      {
        case 0:
        {
          q->red=pixel;
          if (image->class == PseudoClass)
            {
              *image->indexes=pixel;
              *q=image->colormap[pixel];
            }
          break;
        }
        case 1:
        {
          q->green=pixel;
          break;
        }
        case 2:
        {
          q->blue=pixel;
          break;
        }
        case 3:
        default:
        {
          q->opacity=pixel;
          break;
        }
      }
      if (!SyncPixelCache(image))
        break;
      x++;
      length--;
    }
  }
  /*
    Guarentee the correct number of pixel packets.
  */
  if (length > 0)
    {
      MagickWarning(CorruptImageWarning,"insufficient image data in file",
        image->filename);
      return(False);
    }
  else
    if (length < 0)
      {
        MagickWarning(CorruptImageWarning,"too much image data in file",
          image->filename);
        return(False);
      }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P S D                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsPSD returns True if the image format type, identified by the
%  magick string, is PSD.
%
%  The format of the ReadPSDImage method is:
%
%      unsigned int IsPSD(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsPSD returns True if the image format type is PSD.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsPSD(const unsigned char *magick,const unsigned int length)
{
  if (length < 4)
    return(False);
  if (strncmp((char *) magick,"8BPS",4) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P S D I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadPSDImage reads an Adobe Photoshop image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadPSDImage method is:
%
%      image=ReadPSDImage(image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPSDImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadPSDImage(const ImageInfo *image_info)
{
#define BitmapMode  0
#define GrayscaleMode  1
#define IndexedMode  2
#define CMYKMode  4

  typedef struct _ChannelInfo
  {
    short int
      type;

    unsigned long
      size;
  } ChannelInfo;

  typedef struct _LayerInfo
  {
    unsigned int
      width,
      height;

    int
      x,
      y;

    unsigned short
      channels;

    ChannelInfo
      channel_info[24];

    char
      blendkey[4];

    unsigned char
      opacity,
      clipping,
      flags;

    Image
      *image;
  } LayerInfo;

  typedef struct _PSDHeader
  {
    char
      signature[4];

    unsigned short
      channels,
      version;

    unsigned char
      reserved[6];

    unsigned int
      rows,
      columns;

    unsigned short
      depth,
      mode;
  } PSDHeader;

  char
    type[4];

  Image
    *image;

  int
    j,
    y;

  LayerInfo
    *layer_info;

  long
    length,
    size;

  PSDHeader
    psd_header;

  register int
    i,
    x;

  register PixelPacket
    *q;

  short int
    number_layers;

  unsigned char
    *scanline;

  unsigned int
    packet_size,
    status;

  unsigned short
    compression;

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
    Read image header.
  */
  status=ReadBlob(image,4,(char *) psd_header.signature);
  psd_header.version=MSBFirstReadShort(image);
  if ((status == False) || (strncmp(psd_header.signature,"8BPS",4) != 0) ||
      (psd_header.version != 1))
    ReaderExit(CorruptImageWarning,"Not a PSD image file",image);
  (void) ReadBlob(image,6,(char *) psd_header.reserved);
  psd_header.channels=MSBFirstReadShort(image);
  psd_header.rows=MSBFirstReadLong(image);
  psd_header.columns=MSBFirstReadLong(image);
  psd_header.depth=MSBFirstReadShort(image);
  psd_header.mode=MSBFirstReadShort(image);
  /*
    Initialize image.
  */
  if (psd_header.mode == CMYKMode)
    image->colorspace=CMYKColorspace;
  else
    image->matte=psd_header.channels >= 4;
  image->columns=psd_header.columns;
  image->rows=psd_header.rows;
  image->depth=psd_header.depth <= 8 ? 8 : QuantumDepth;
  length=MSBFirstReadLong(image);
  if ((psd_header.mode == BitmapMode) ||
      (psd_header.mode == GrayscaleMode) ||
      (psd_header.mode == IndexedMode) || (length > 0))
    {
      /*
        Create colormap.
      */
      image->class=PseudoClass;
      image->colors=256;
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
      if (length > 0)
        {
          /*
            Read PSD raster colormap.
          */
          for (i=0; i < (int) image->colors; i++)
            image->colormap[i].red=UpScale(ReadByte(image));
          for (i=0; i < (int) image->colors; i++)
            image->colormap[i].green=UpScale(ReadByte(image));
          for (i=0; i < (int) image->colors; i++)
            image->colormap[i].blue=UpScale(ReadByte(image));
        }
    }
  length=MSBFirstReadLong(image);
  if (length > 0)
    {
      unsigned char
        *data;

      data=(unsigned char *)
        AllocateMemory(length*sizeof(unsigned char));
      if (data == (unsigned char *) NULL)
        ReaderExit(ResourceLimitWarning,
          "8BIM resource memory allocation failed",image);
      status=ReadBlob(image,length,(char *) data);
      if ((status == False) || (strncmp((char *) data,"8BIM",4) != 0))
        ReaderExit(CorruptImageWarning,"Not a PSD image file",image);
      image->iptc_profile.info=data;
      image->iptc_profile.length=length;
    }
  if (image_info->ping)
    {
      CloseBlob(image);
      return(image);
    }
  layer_info=(LayerInfo *) NULL;
  number_layers=0;
  length=MSBFirstReadLong(image);
  if (length > 0)
    {
      /*
        Read layer and mask block.
      */
      size=MSBFirstReadLong(image);
      number_layers=MSBFirstReadShort(image);
      number_layers=AbsoluteValue(number_layers);
      layer_info=(LayerInfo *) AllocateMemory(number_layers*sizeof(LayerInfo));
      if (layer_info == (LayerInfo *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (i=0; i < number_layers; i++)
      {
        layer_info[i].y=MSBFirstReadLong(image);
        layer_info[i].x=MSBFirstReadLong(image);
        layer_info[i].height=MSBFirstReadLong(image)-layer_info[i].y;
        layer_info[i].width=MSBFirstReadLong(image)-layer_info[i].x;
        layer_info[i].channels=MSBFirstReadShort(image);
        if (layer_info[i].channels > 24)
          ReaderExit(CorruptImageWarning,"Not a PSD image file",image);
        for (j=0; j < (int) layer_info[i].channels; j++)
        {
          layer_info[i].channel_info[j].type=MSBFirstReadShort(image);
          layer_info[i].channel_info[j].size=MSBFirstReadLong(image);
        }
        status=ReadBlob(image,4,(char *) type);
        if ((status == False) || (strncmp(type,"8BIM",4) != 0))
          ReaderExit(CorruptImageWarning,"Not a PSD image file",image);
        (void) ReadBlob(image,4,(char *) layer_info[i].blendkey);
        layer_info[i].opacity=ReadByte(image);
        layer_info[i].clipping=ReadByte(image);
        layer_info[i].flags=ReadByte(image);
        (void) ReadByte(image);  /* filler */
        size=MSBFirstReadLong(image);
        for (j=0; j < size; j++)
          (void) ReadByte(image);
        /*
          Allocate layered image.
        */
        layer_info[i].image=
          CloneImage(image,layer_info[i].width,layer_info[i].height,True);
        if (layer_info[i].image == (Image *) NULL)
          {
            for (j=0; j < i; j++)
              DestroyImage(layer_info[j].image);
            ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
          }
        if (psd_header.mode == CMYKMode)
          layer_info[i].image->colorspace=CMYKColorspace;
        else
          layer_info[i].image->matte=layer_info[i].channels >= 4;
      }
      /*
        Read pixel data for each layer.
      */
      for (i=0; i < number_layers; i++)
      {
        layer_info[i].image->file=image->file;
        layer_info[i].image->blob_info=image->blob_info;
        for (j=0; j < (int) layer_info[i].channels; j++)
        {
          compression=MSBFirstReadShort(layer_info[i].image);
          if (compression != 0)
            {
              for (y=0; y < (int) layer_info[i].image->rows; y++)
                (void) MSBFirstReadShort(layer_info[i].image);
              (void) DecodeImage(layer_info[i].image,
                layer_info[i].channel_info[j].type);
            }
          else
            {
              /*
                Read uncompressed pixel data as separate planes.
              */
              packet_size=1;
              if (layer_info[i].image->class == PseudoClass)
                {
                  if (layer_info[i].image->colors > 256)
                    packet_size++;
                }
              else
                if (layer_info[i].image->depth > 8)
                  packet_size++;
              scanline=(unsigned char *) AllocateMemory((packet_size*
                layer_info[i].image->columns+1)*sizeof(unsigned char));
              if (scanline == (unsigned char *) NULL)
                ReaderExit(ResourceLimitWarning,"Memory allocation failed",
                  image);
              for (y=0; y < (int) layer_info[i].image->rows; y++)
              {
                q=SetPixelCache(layer_info[i].image,0,y,
                  layer_info[i].image->columns,1);
                if (q == (PixelPacket *) NULL)
                  break;
                (void) ReadBlob(layer_info[i].image,packet_size*
                  layer_info[i].image->columns,(char *) scanline);
                switch (layer_info[i].channel_info[j].type)
                {
                  case 0:
                  {
                    if (layer_info[i].image->class == PseudoClass)
                      (void) ReadPixelCache(layer_info[i].image,IndexQuantum,
                        scanline);
                    else
                      (void) ReadPixelCache(layer_info[i].image,RedQuantum,
                        scanline);
                    break;
                  }
                  case 1:
                  {
                    (void) ReadPixelCache(layer_info[i].image,GreenQuantum,
                      scanline);
                    break;
                  }
                  case 2:
                  {
                    (void) ReadPixelCache(layer_info[i].image,BlueQuantum,
                      scanline);
                    break;
                  }
                  case 3:
                  default:
                  {
                    (void) ReadPixelCache(layer_info[i].image,OpacityQuantum,
                      scanline);
                    break;
                  }
                }
                if (!SyncPixelCache(layer_info[i].image))
                  break;
              }
              FreeMemory(scanline);
            }
        }
        image->file=layer_info[i].image->file;
        image->blob_info=layer_info[i].image->blob_info;
        if (layer_info[i].image->colorspace == CMYKColorspace)
          {
            /*
              Correct CMYK levels.
            */
            for (y=0; y < (int) layer_info[i].image->rows; y++)
            {
              q=SetPixelCache(layer_info[i].image,0,y,
                layer_info[i].image->columns,1);
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) layer_info[i].image->columns; x++)
              {
                q->red=MaxRGB-q->red;
                q->green=MaxRGB-q->green;
                q->blue=MaxRGB-q->blue;
                q->opacity=MaxRGB-q->opacity;
                q++;
              }
              if (!SyncPixelCache(layer_info[i].image))
                break;
            }
          }
        else
          if (layer_info[i].opacity != Opaque)
            {
              /*
                Correct for opacity level.
              */
              for (y=0; y < (int) layer_info[i].image->rows; y++)
              {
                q=GetPixelCache(layer_info[i].image,0,y,
                  layer_info[i].image->columns,1);
                if (q == (PixelPacket *) NULL)
                  break;
                for (x=0; x < (int) layer_info[i].image->columns; x++)
                {
                  q->opacity=(int) (q->opacity*layer_info[i].opacity)/Opaque;
                  q++;
                }
                if (!SyncPixelCache(layer_info[i].image))
                  break;
              }
            }
        layer_info[i].image->file=(FILE *) NULL;
      }
      for (i=0; i < 4; i++)
        (void) ReadByte(image);
    }
  /*
    Convert pixels to Runlength encoded.
  */
  compression=MSBFirstReadShort(image);
  SetImage(image);
  if (compression != 0)
    {
      /*
        Read Packbit encoded pixel data as separate planes.
      */
      for (i=0; i < (int) (image->rows*psd_header.channels); i++)
        (void) MSBFirstReadShort(image);
      for (i=0; i < (int) psd_header.channels; i++)
        (void) DecodeImage(image,i);
    }
  else
    {
      int
        channel_map[24];

      /*
        Read uncompressed pixel data as separate planes.
      */
      packet_size=1;
      if (image->class == PseudoClass)
        {
          if (image->colors > 256)
            packet_size++;
        }
      else
        if (image->depth > 8)
          packet_size++;
      for (i=0; i < psd_header.channels; i++)
        channel_map[i]=!image->matte ? i : i-1;
      scanline=(unsigned char *)
        AllocateMemory(packet_size*image->columns*sizeof(unsigned char));
      if (scanline == (unsigned char *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (i=0; i < (int) psd_header.channels; i++)
      {
        for (y=0; y < (int) image->rows; y++)
        {
          q=GetPixelCache(image,0,y,image->columns,1);
          status=ReadBlob(image,packet_size*image->columns,(char *) scanline);
          if ((status == False) || (q == (PixelPacket *) NULL))
            break;
          switch (channel_map[i])
          {
            case 0:
            {
              if (image->class == PseudoClass)
                (void) ReadPixelCache(image,IndexQuantum,scanline);
              else
                (void) ReadPixelCache(image,RedQuantum,scanline);
              break;
            }
            case 1:
            {
              (void) ReadPixelCache(image,GreenQuantum,scanline);
              break;
            }
            case 2:
            {
              (void) ReadPixelCache(image,BlueQuantum,scanline);
              break;
            }
            case 3:
            default:
            {
              (void) ReadPixelCache(image,OpacityQuantum,scanline);
              break;
            }
          }
          if (!SyncPixelCache(image))
            break;
        }
      }
      if (image->matte && (number_layers != 0))
        MatteImage(image,Transparent);
      FreeMemory(scanline);
    }
  if (image->colorspace == CMYKColorspace)
    {
      /*
        Correct CMYK levels.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          q->red=MaxRGB-q->red;
          q->green=MaxRGB-q->green;
          q->blue=MaxRGB-q->blue;
          q->opacity=MaxRGB-q->opacity;
          q++;
        }
        if (!SyncPixelCache(image))
          break;
      }
    }
  for (i=0; i < number_layers; i++)
  {
    /*
      Composite layer onto image.
    */
    if ((layer_info[i].width != 0) && (layer_info[i].height != 0))
      CompositeImage(image,OverCompositeOp,layer_info[i].image,layer_info[i].x,
        layer_info[i].y);
    DestroyImage(layer_info[i].image);
  }
  image->matte=False;
  if (image->colorspace != CMYKColorspace)
    image->matte=psd_header.channels >= 4;
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S D I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WritePSDImage writes an image in the Adobe Photoshop encoded image
%  format.
%
%  The format of the WritePSDImage method is:
%
%      unsigned int WritePSDImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WritePSDImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WritePSDImage(const ImageInfo *image_info,Image *image)
{
  int
    y;

  register int
    i;

  unsigned char
    *pixels;

  unsigned int
    packet_size,
    status;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  packet_size=image->depth > 8 ? 6 : 3;
  if (image->matte)
    packet_size+=image->depth > 8 ? 2 : 1;
  pixels=(unsigned char *)
    AllocateMemory(packet_size*image->columns*sizeof(PixelPacket));
  if (pixels == (unsigned char *) NULL)
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  (void) WriteBlob(image,4,"8BPS");
  MSBFirstWriteShort(image,1);  /* version */
  (void) WriteBlob(image,6,"      ");  /* reserved */
  if (image->class == PseudoClass)
    MSBFirstWriteShort(image,1);
  else
    MSBFirstWriteShort(image,image->matte ? 4 : 3);
  MSBFirstWriteLong(image,image->rows);
  MSBFirstWriteLong(image,image->columns);
  MSBFirstWriteShort(image,image->class == PseudoClass ? 8 : image->depth);
  if (((image_info->colorspace != UndefinedColorspace) ||
       (image->colorspace != CMYKColorspace)) &&
       (image_info->colorspace != CMYKColorspace))
    {
      TransformRGBImage(image,RGBColorspace);
      MSBFirstWriteShort(image,image->class == PseudoClass ? 2 : 3);
    }
  else
    {
      if (image->colorspace != CMYKColorspace)
        RGBTransformImage(image,CMYKColorspace);
      MSBFirstWriteShort(image,4);
    }
  if ((image->class == DirectClass) || (image->colors > 256))
    MSBFirstWriteLong(image,0);
  else
    {
      /*
        Write PSD raster colormap.
      */
      MSBFirstWriteLong(image,768);
      for (i=0; i < (int) image->colors; i++)
        (void) WriteByte(image,DownScale(image->colormap[i].red));
      for ( ; i < 256; i++)
        (void) WriteByte(image,0);
      for (i=0; i < (int) image->colors; i++)
        (void) WriteByte(image,DownScale(image->colormap[i].green));
      for ( ; i < 256; i++)
        (void) WriteByte(image,0);
      for (i=0; i < (int) image->colors; i++)
        (void) WriteByte(image,DownScale(image->colormap[i].blue));
      for ( ; i < 256; i++)
        (void) WriteByte(image,0);
    }
  MSBFirstWriteLong(image,0);  /* image resource block */
  MSBFirstWriteLong(image,0);  /* layer and mask block */
  MSBFirstWriteShort(image,0);  /* compression */
  /*
    Write uncompressed pixel data as separate planes.
  */
  if (image->class == PseudoClass)
    for (y=0; y < (int) image->rows; y++)
    {
      if (!GetPixelCache(image,0,y,image->columns,1))
        break;
      (void) WritePixelCache(image,IndexQuantum,pixels);
      (void) WriteBlob(image,image->columns,pixels);
    }
  else
    {
      packet_size=image->depth > 8 ? 2 : 1;
      if (image->matte)
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) WritePixelCache(image,OpacityQuantum,pixels);
          (void) WriteBlob(image,packet_size*image->columns,pixels);
        }
      for (y=0; y < (int) image->rows; y++)
      {
        if (!GetPixelCache(image,0,y,image->columns,1))
          break;
        if (image->colorspace == CMYKColorspace)
          (void) WritePixelCache(image,CyanQuantum,pixels);
        else
          (void) WritePixelCache(image,RedQuantum,pixels);
        (void) WriteBlob(image,packet_size*image->columns,pixels);
      }
      for (y=0; y < (int) image->rows; y++)
      {
        if (!GetPixelCache(image,0,y,image->columns,1))
          break;
        if (image->colorspace == CMYKColorspace)
          (void) WritePixelCache(image,YellowQuantum,pixels);
        else
          (void) WritePixelCache(image,GreenQuantum,pixels);
        (void) WriteBlob(image,packet_size*image->columns,pixels);
      }
      for (y=0; y < (int) image->rows; y++)
      {
        if (!GetPixelCache(image,0,y,image->columns,1))
          break;
        if (image->colorspace == CMYKColorspace)
          (void) WritePixelCache(image,MagentaQuantum,pixels);
        else
          (void) WritePixelCache(image,BlueQuantum,pixels);
        (void) WriteBlob(image,packet_size*image->columns,pixels);
      }
      if (image->colorspace == CMYKColorspace)
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) WritePixelCache(image,BlackQuantum,pixels);
          (void) WriteBlob(image,packet_size*image->columns,pixels);
        }
    }
  FreeMemory(pixels);
  CloseBlob(image);
  return(True);
}
