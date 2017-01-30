/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   SSSSS  33333                              %
%                            P   P  SS        33                              %
%                            PPPP    SSS    333                               %
%                            P         SS     33                              %
%                            P      SSSSS  33333                              %
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
#if defined(HasTIFF)
#define CCITTParam  "-1"
#else
#define CCITTParam  "0"
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P S 3 I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WritePS3Image translates an image to encapsulated Postscript
%  Level III for printing.  If the supplied geometry is null, the image is
%  centered on the Postscript page.  Otherwise, the image is positioned as
%  specified by the geometry.
%
%  The format of the WritePS3Image method is:
%
%      unsigned int WritePS3Image(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o status: Method WritePS3Image return True if the image is printed.
%      False is returned if the image file cannot be opened for printing.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export unsigned int WritePS3Image(const ImageInfo *image_info,Image *image)
{
#define CFormat  "/%s filter "

  char
    buffer[MaxTextExtent],
    date[MaxTextExtent],
    density[MaxTextExtent],
    geometry[MaxTextExtent];

  CompressionType
    compression;

  double
    dx_resolution,
    dy_resolution,
    x_resolution,
    x_scale,
    y_resolution,
    y_scale;

  int
    count,
    status,
    x,
    y;

  register PixelPacket
    *p;

  SegmentInfo
    bounding_box;

  time_t
    timer;

  unsigned char
    *pixels;

  unsigned int
    height,
    page,
    scene,
    text_size,
    width;

  unsigned long
    number_packets;

  if (!image->matte)
    WritePS2Image(image_info,image);
  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  compression=image->compression;
  if (image_info->compression != UndefinedCompression)
    compression=image_info->compression;
  page=1;
  scene=0;
  do
  {
    /*
      Scale image to size of Postscript page.
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
        if (Latin1Compare(image_info->magick,"PDF") == 0)
          (void) strcpy(geometry,PSPageGeometry);
    (void) ParseImageGeometry(geometry,&x,&y,&width,&height);
    /*
      Scale relative to dots-per-inch.
    */
    dx_resolution=72.0;
    dy_resolution=72.0;
    x_resolution=72.0;
    (void) strcpy(density,PSDensityGeometry);
    count=sscanf(density,"%lfx%lf",&x_resolution,&y_resolution);
    if (count != 2)
      y_resolution=x_resolution;
    if (image_info->density != (char *) NULL)
      {
        count=sscanf(image_info->density,"%lfx%lf",&x_resolution,&y_resolution);
        if (count != 2)
          y_resolution=x_resolution;
      }
    x_scale=(width*dx_resolution)/x_resolution;
    width=(unsigned int) (x_scale+0.5);
    y_scale=(height*dy_resolution)/y_resolution;
    height=(unsigned int) (y_scale+0.5);
    if (page == 1)
      {
        /*
          Output Postscript header.
        */
        (void) strcpy(buffer,"%!PS-Adobe-3.0 Resource-ProcSet\n");
        (void) WriteBlob(image,strlen(buffer),buffer);
        (void) strcpy(buffer,"%%Creator: (ImageMagick)\n");
        (void) WriteBlob(image,strlen(buffer),buffer);
        (void) sprintf(buffer,"%%%%Title: (%.1024s)\n",image->filename);
        (void) WriteBlob(image,strlen(buffer),buffer);
        timer=time((time_t *) NULL);
        (void) localtime(&timer);
        (void) strcpy(date,ctime(&timer));
        date[Extent(date)-1]='\0';
        (void) sprintf(buffer,"%%%%CreationDate: (%.1024s)\n",date);
        (void) WriteBlob(image,strlen(buffer),buffer);
        bounding_box.x1=x;
        bounding_box.y1=y;
        bounding_box.x2=x+width-1;
        bounding_box.y2=y+(height+text_size)-1;
        if (image_info->adjoin && (image->next != (Image *) NULL))
          (void) strcpy(buffer,"%%BoundingBox: (atend)\n");
        else
          (void) sprintf(buffer,"%%%%BoundingBox: %g %g %g %g\n",
            bounding_box.x1,bounding_box.y1,bounding_box.x2,bounding_box.y2);
        (void) WriteBlob(image,strlen(buffer),buffer);
        if (image->label != (char *) NULL)
          {
            (void) strcpy(buffer,
              "%%%%DocumentNeededResources: font Helvetica\n");
            (void) WriteBlob(image,strlen(buffer),buffer);
          }
        (void) strcpy(buffer,"%%LanguageLevel: 2\n");
        (void) WriteBlob(image,strlen(buffer),buffer);
        (void) strcpy(buffer,"%%Orientation: Portrait\n");
        (void) WriteBlob(image,strlen(buffer),buffer);
        (void) strcpy(buffer,"%%PageOrder: Ascend\n");
        (void) WriteBlob(image,strlen(buffer),buffer);
        (void) sprintf(buffer,"%%%%Pages: %u\n",GetNumberScenes(image));
        (void) WriteBlob(image,strlen(buffer),buffer);
        (void) strcpy(buffer,"%%EndComments\n");
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    (void) sprintf(buffer,"%%%%Page:  1 %u\n",page++);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"%%%%PageBoundingBox: %d %d %d %d\n",x,y,
      x+(int) width-1,y+(int) (height+text_size)-1);
    (void) WriteBlob(image,strlen(buffer),buffer);
    if (x < bounding_box.x1)
      bounding_box.x1=x;
    if (y < bounding_box.y1)
      bounding_box.y1=y;
    if ((x+(int) width-1) > bounding_box.x2)
      bounding_box.x2=x+width-1;
    if ((y+(int) (height+text_size)-1) > bounding_box.y2)
      bounding_box.y2=y+(height+text_size)-1;
    if (image->label != (char *) NULL)
      {
        (void) strcpy(buffer,"%%PageResources: font Helvetica\n");
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    /*
      Output image data.
    */
    (void) strcpy(buffer,"currentfile /ASCII85Decode filter ");
    (void) WriteBlob(image,strlen(buffer),buffer);
    if (compression != NoCompression)
      {
        switch (compression)
        {
          case JPEGCompression: sprintf(buffer,CFormat,"DCTDecode"); break;
          case LZWCompression: sprintf(buffer,CFormat,"LZWDecode"); break;
          case ZipCompression: sprintf(buffer,CFormat,"FlateDecode"); break;
          default: sprintf(buffer,CFormat,"RunLengthDecode"); break;
        }
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    (void) strcpy(buffer,"/ReusableStreamDecode filter\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    switch (compression)
    {
      case JPEGCompression:
      {
        char
          filename[MaxTextExtent];

        FILE
          *file;

        Image
          *jpeg_image;

        int
          c;

        /*
          Write image to temporary file in JPEG format.
        */
        TemporaryFilename(filename);
        jpeg_image=CloneImage(image,image->columns,image->rows,True);
        if (jpeg_image == (Image *) NULL)
          WriterExit(DelegateWarning,"Unable to clone image",image);
        (void) strcpy(jpeg_image->filename,filename);
        status=WriteJPEGImage(image_info,jpeg_image);
        DestroyImage(jpeg_image);
        if (status == False)
          WriterExit(DelegateWarning,"Unable to write image",image);
        file=fopen(filename,ReadBinaryType);
        if (file == (FILE *) NULL)
          WriterExit(FileOpenWarning,"Unable to open file",image);
        Ascii85Initialize();
        for (c=fgetc(file); c != EOF; c=fgetc(file))
          Ascii85Encode(image,c);
        Ascii85Flush(image);
        (void) fclose(file);
        (void) remove(filename);
        break;
      }
      case RunlengthEncodedCompression:
      default:
      {
        register unsigned char
          *q;

        /*
          Allocate pixel array.
        */
        number_packets=4*image->columns*image->rows;
        pixels=(unsigned char *)
          AllocateMemory(number_packets*sizeof(unsigned char));
        if (pixels == (unsigned char *) NULL)
          WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
        /*
          Dump Packbit encoded pixels.
        */
        q=pixels;
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            *q++=DownScale(p->opacity);
            *q++=DownScale(p->red);
            *q++=DownScale(p->green);
            *q++=DownScale(p->blue);
            p++;
          }
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
        }
        if (compression == ZipCompression)
          status=
            ZLIBEncodeImage(image,number_packets,image_info->quality,pixels);
        else
          if (compression == LZWCompression)
            status=LZWEncodeImage(image,number_packets,pixels);
          else
            status=PackbitsEncodeImage(image,number_packets,pixels);
        if (!status)
          {
            CloseBlob(image);
            return(False);
          }
        FreeMemory(pixels);
        break;
      }
      case NoCompression:
      {
        /*
          Dump uncompressed DirectColor packets.
        */
        Ascii85Initialize();
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            Ascii85Encode(image,DownScale(p->opacity));
            Ascii85Encode(image,DownScale(p->red));
            Ascii85Encode(image,DownScale(p->green));
            Ascii85Encode(image,DownScale(p->blue));
            p++;
          }
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
        }
        Ascii85Flush(image);
        break;
      }
    }
    (void) WriteByte(image,'\n');
    (void) WriteByte(image,'\n');
    (void) strcpy(buffer,"/datastream exch def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"/DeviceRGB setcolorspace\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"/ImageDataDictionary 8 dict def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"ImageDataDictionary begin\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /ImageType 1 def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"  /Width %u def\n",image->columns);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"  /Height %u def\n",image->rows);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /BitsPerComponent 8 def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /DataSource datastream def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /MultipleDataSources false def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"  /ImageMatrix [ %d 0 0 %d neg 0 %d ] def\n",
      image->columns,image->rows,image->rows);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /Decode [ 0 1 0 1 0 1 ] def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"end\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) WriteByte(image,'\n');
    (void) strcpy(buffer,"/ImageMaskDictionary 8 dict def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"ImageMaskDictionary begin\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /ImageType 1 def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"  /Width %u def\n",image->columns);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"  /Height %u def\n",image->rows);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /BitsPerComponent 8 def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /MultipleDataSources false def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"  /ImageMatrix [ %d 0 0 %d neg 0 %d ] def\n",
      image->columns,image->rows,image->rows);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /Decode [ 0 1 ] def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"end\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) WriteByte(image,'\n');
    (void) strcpy(buffer,"/MaskedImageDictionary 7 dict def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"MaskedImageDictionary begin\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /ImageType 3 def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /InterleaveType 1 def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /MaskDict ImageMaskDictionary def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"  /DataDict ImageDataDictionary def\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"end\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) WriteByte(image,'\n');
    (void) strcpy(buffer,"gsave\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"%d %d translate\n",x,y);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"%g %g scale\n",x_scale,y_scale);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"ImageMaskDictionary /Decode [ 1 0 ] put\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"MaskedImageDictionary image\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"grestore                    \n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) WriteByte(image,'\n');
    (void) strcpy(buffer,"showpage\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) strcpy(buffer,"%%EndData\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    if (image->next == (Image *) NULL)
      break;
    image=GetNextImage(image);
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  if (page > 1)
    {
      (void) sprintf(buffer,"%%%%BoundingBox: %g %g %g %g\n",
        bounding_box.x1,bounding_box.y1,bounding_box.x2,bounding_box.y2);
      (void) WriteBlob(image,strlen(buffer),buffer);
    }
  (void) strcpy(buffer,"%%EOF\n");
  (void) WriteBlob(image,strlen(buffer),buffer);
  CloseBlob(image);
  return(True);
}
