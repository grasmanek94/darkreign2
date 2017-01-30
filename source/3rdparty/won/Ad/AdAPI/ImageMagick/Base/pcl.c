/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP    CCCC  L                                  %
%                            P   P  C      L                                  %
%                            PPPP   C      L                                  %
%                            P      C      L                                  %
%                            P       CCCC  LLLLL                              %
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
%   I s P C L                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsPCL returns True if the image format type, identified by the
%  magick string, is PCL.
%
%  The format of the ReadPCLImage method is:
%
%      unsigned int IsPCL(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsPCL returns True if the image format type is PCL.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsPCL(const unsigned char *magick,const unsigned int length)
{
  if (length < 4)
    return(False);
  if (strncmp((char *) magick,"\033E\033&",4) == 0)
    return(False);
  if (strncmp((char *) magick,"\033E\033",3) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P C L I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadPCLImage reads a Page Control Language image file and returns
%  it.  It allocates the memory necessary for the new Image structure and
%  returns a pointer to the new image.
%
%  The format of the ReadPCLImage method is:
%
%      Image *ReadPCLImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPCLImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadPCLImage(const ImageInfo *image_info)
{
  MagickWarning(MissingDelegateWarning,"Cannot read PCL images",
    image_info->filename);
  return((Image *) NULL);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P C L I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             % %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WritePCLImage writes an image in the Page Control Language encoded
%  image format.
%
%  The format of the WritePCLImage method is:
%
%      unsigned int WritePCLImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WritePCLImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
%
*/
Export unsigned int WritePCLImage(const ImageInfo *image_info,Image *image)
{
  char
    buffer[MaxTextExtent],
    geometry[MaxTextExtent];

  int
    sans_offset,
    x,
    y;

  register PixelPacket
    *p;

  RectangleInfo
    media_info;

  unsigned int
    density,
    height,
    page_size,
    status,
    text_size,
    width;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  TransformRGBImage(image,RGBColorspace);
  /*
    Initialize the printer.
  */
  (void) strcpy(buffer,"\033E");  /* portrait orientation */
  (void) WriteBlob(image,strlen(buffer),buffer);
  (void) strcpy(buffer,"\033&l0O");  /* portrait orientation */
  (void) WriteBlob(image,strlen(buffer),buffer);
  (void) strcpy(buffer,"\033&l0E");  /* top margin 0 */
  (void) WriteBlob(image,strlen(buffer),buffer);
  /*
    Center image on PCL page.
  */
  text_size=0;
  if (image->label != (char *) NULL)
    text_size=MultilineCensus(image->label)*image_info->pointsize+12;
  width=image->columns;
  height=image->rows;
  x=0;
  y=text_size;
  FormatString(geometry,"%ux%u",image->columns,image->rows);
  if (image_info->page != (char *) NULL)
    (void) strcpy(geometry,image_info->page);
  else
    if ((image->page_info.width != 0) && (image->page_info.height != 0))
      (void) FormatString(geometry,"%ux%u%+d%+d",image->page_info.width,
        image->page_info.height,image->page_info.x,image->page_info.y);
    else
      if (Latin1Compare(image_info->magick,"PCL") == 0)
        (void) strcpy(geometry,PSPageGeometry);
  (void) ParseImageGeometry(geometry,&x,&y,&width,&height);
  (void) GetGeometry(geometry,&media_info.x,&media_info.y,
    &media_info.width,&media_info.height);
  page_size=2;
  if ((media_info.width == 540) && (media_info.height == 720))
    page_size=1;  /* executive */
  if ((media_info.width == 612) && (media_info.height == 792))
    page_size=2;  /* letter */
  if ((media_info.width == 612) && (media_info.height == 1008))
    page_size=3;  /* legal */
  if ((media_info.width == 1224) && (media_info.height == 792))
    page_size=6;  /* ledger */
  if ((media_info.width == 595) && (media_info.height == 842))
    page_size=26;  /* A4 */
  if ((media_info.width == 842) && (media_info.height == 1191))
    page_size=27;  /* A3 */
  if ((media_info.width == 729) && (media_info.height == 1032))
    page_size=45;  /* B5 */
  if ((media_info.width == 516) && (media_info.height == 729))
    page_size=46;  /* B4 */
  (void) sprintf(buffer,"\033&l%uA",page_size);  /* papersize */
  (void) WriteBlob(image,strlen(buffer),buffer);
  density=72;
  if (image_info->density != (char *) NULL)
    (void) ParseGeometry(image_info->density,&sans_offset,&sans_offset,
      &density,&density);
  else
    (void) ParseGeometry("75x75",&sans_offset,&sans_offset,
      &density,&density);
  (void) sprintf(buffer,"\033*p%dx%dY",x,y);
  (void) WriteBlob(image,strlen(buffer),buffer);
  if (image->label != (char *) NULL)
    {
      /*
        Print label.
      */
      (void) strcpy(buffer,"\033&k2G");
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) sprintf(buffer,"\033(s1p%uv5t3b",(unsigned int)
        image_info->pointsize);
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) sprintf(buffer,"\n%.1024s\n",image->label);
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) strcpy(buffer,"\033(s0B");
      (void) WriteBlob(image,strlen(buffer),buffer);
    }
  (void) sprintf(buffer,"\033*t%uR",density);  /* graphic resolution */
  (void) WriteBlob(image,strlen(buffer),buffer);
  width=(density*width)/75;
  height=(density*height)/75;
  if (!IsGrayImage(image))
    {
      /*
        Write PCL color image.
      */
      (void) sprintf(buffer,"\033*r%us%uT",image->columns,image->rows);
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) sprintf(buffer,"\033*t%uh%uV",width,height);
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) strcpy(buffer,"\033*v6W");
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) WriteByte(image,'\000');  /* color model */
      (void) WriteByte(image,'\003');  /* direct pixel encoding */
      (void) WriteByte(image,'\000');  /* bits per index */
      (void) WriteByte(image,'\010');  /* bits red*/
      (void) WriteByte(image,'\010');  /* bits green*/
      (void) WriteByte(image,'\010');  /* bits blue */
      (void) strcpy(buffer,"\033*r2A");  /* start graphics */
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) strcpy(buffer,"\033*b0M");  /* no compression */
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) sprintf(buffer,"\033*b%uW",3*image->columns);
      (void) WriteBlob(image,strlen(buffer),buffer);
      for (y=0; y < (int) image->rows; y++)
      {
        p=GetPixelCache(image,0,y,image->columns,1);
        if (p == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          (void) sprintf(buffer,"%c%c%c",(int) DownScale(p->red),
            (int) DownScale(p->green),(int) DownScale(p->blue));
          (void) WriteBlob(image,strlen(buffer),buffer);
          p++;
        }
        (void) sprintf(buffer,"\033*b%uW",3*image->columns);
        (void) WriteBlob(image,strlen(buffer),buffer);
        if (QuantumTick(y,image->rows))
          ProgressMonitor(SaveImageText,y,image->rows);
      }
      (void) strcpy(buffer,"\033*rC");  /* end graphics */
      (void) WriteBlob(image,strlen(buffer),buffer);
    }
  else
    {
      Image
        *monochrome_image;

      register unsigned char
        bit,
        byte,
        polarity;

      /*
        Write PCL monochrome image.
      */
      monochrome_image=image;
      if ((width != image->columns) || (height != image->rows))
        {
          /*
            Scale image.
          */
          image->orphan=True;
          monochrome_image=ZoomImage(image,width,height);
          if (monochrome_image == (Image *) NULL)
            WriterExit(ResourceLimitWarning,"Unable to scale image",image);
        }
      if (!IsMonochromeImage(monochrome_image))
        {
          QuantizeInfo
            quantize_info;

          GetQuantizeInfo(&quantize_info);
          quantize_info.number_colors=2;
          quantize_info.dither=image_info->dither;
          quantize_info.colorspace=GRAYColorspace;
          (void) QuantizeImage(&quantize_info,monochrome_image);
        }
      polarity=Intensity(image->colormap[0]) > (MaxRGB >> 1);
      if (monochrome_image->colors == 2)
        polarity=Intensity(monochrome_image->colormap[0]) >
          Intensity(monochrome_image->colormap[1]);
      (void) sprintf(buffer,"\033*r%us%uT",monochrome_image->columns,
        monochrome_image->rows);
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) strcpy(buffer,"\033*r1A");  /* start graphics */
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) strcpy(buffer,"\033*b0M");  /* no compression */
      (void) WriteBlob(image,strlen(buffer),buffer);
      (void) sprintf(buffer,"\033*b%uW",(image->columns+7)/8);
      (void) WriteBlob(image,strlen(buffer),buffer);
      for (y=0; y < (int) image->rows; y++)
      {
        if (!GetPixelCache(image,0,y,image->columns,1))
          break;
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
              (void) WriteByte(image,byte);
              bit=0;
              byte=0;
            }
        }
        if (bit != 0)
          (void) WriteByte(image,byte << (8-bit));
        if (y < (int) monochrome_image->rows)
          {
            (void) sprintf(buffer,"\033*b%uW",
              (monochrome_image->columns+7)/8);
            (void) WriteBlob(image,strlen(buffer),buffer);
          }
        if (QuantumTick(y,monochrome_image->rows))
          ProgressMonitor(SaveImageText,y,monochrome_image->rows);
      }
      (void) strcpy(buffer,"\033*rB");  /* end graphics */
      (void) WriteBlob(image,strlen(buffer),buffer);
      if (image != monochrome_image)
        DestroyImage(monochrome_image);
    }
  (void) strcpy(buffer,"\033&l0H");
  (void) WriteBlob(image,strlen(buffer),buffer);
  (void) strcpy(buffer,"\033E");  /* portrait orientation */
  (void) WriteBlob(image,strlen(buffer),buffer);
  CloseBlob(image);
  return(True);
}
