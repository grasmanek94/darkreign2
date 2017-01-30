/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        M   M  IIIII  FFFFF  FFFFF                           %
%                        MM MM    I    F      F                               %
%                        M M M    I    FFF    FFF                             %
%                        M   M    I    F      F                               %
%                        M   M  IIIII  F      F                               %
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
#if defined(HasBZLIB)
#include "bzlib.h"
#endif
#if defined(HasZLIB)
#include "zlib.h"
#endif

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M I F F                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsMIFF returns True if the image format type, identified by the
%  magick string, is MIFF.
%
%  The format of the ReadMIFFImage method is:
%
%      unsigned int IsMIFF(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsMIFF returns True if the image format type is MIFF.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsMIFF(const unsigned char *magick,
  const unsigned int length)
{
  if (length < 14)
    return(False);
  if (strncmp((char *) magick,"id=ImageMagick",14) == 0)
    return(True);
  if (strncmp((char *) magick,"Id=ImageMagick",14) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M I F F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadMIFFImage reads a MIFF image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadMIFFImage method is:
%
%      Image *ReadMIFFImage(const ImageInfo *image_info)
%
%  Decompression code contributed by Kyle Shorter.
%
%  A description of each parameter follows:
%
%    o image: Method ReadMIFFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadMIFFImage(const ImageInfo *image_info)
{
#if defined(HasBZLIB)
  bz_stream
    bzip_info;
#endif

  char
    id[MaxTextExtent],
    keyword[MaxTextExtent],
    values[MaxTextExtent];

  Image
    *image;

  IndexPacket
    index;

  int
    c,
    length,
    y;

  PixelPacket
    pixel;

  register int
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *p;

  unsigned char
    *compressed_pixels,
    *pixels;

  unsigned int
    packet_size,
    status;

#if defined(HasZLIB)
  z_stream
    zip_info;
#endif

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
    Decode image header;  header terminates one character beyond a ':'.
  */
  c=ReadByte(image);
  if (c == EOF)
    {
      DestroyImage(image);
      return((Image *) NULL);
    }
  *id='\0';
  do
  {
    /*
      Decode image header;  header terminates one character beyond a ':'.
    */
    image->depth=8;
    image->compression=NoCompression;
    while (isgraph(c) && (c != ':'))
    {
      register char
        *p;

      if (c == '{')
        {
          /*
            Read comment-- any text between { }.
          */
          if (image->comments != (char *) NULL)
            {
              length=Extent(image->comments);
              p=image->comments+length;
            }
          else
            {
              length=MaxTextExtent;
              image->comments=(char *) AllocateMemory(length*sizeof(char));
              p=image->comments;
            }
          for ( ; image->comments != (char *) NULL; p++)
          {
            c=ReadByte(image);
            if ((c == EOF) || (c == '}'))
              break;
            if ((p-image->comments+1) >= (int) length)
              {
                *p='\0';
                length<<=1;
                image->comments=(char *) ReallocateMemory((char *)
                  image->comments,length*sizeof(char));
                if (image->comments == (char *) NULL)
                  break;
                p=image->comments+Extent(image->comments);
              }
            *p=(unsigned char) c;
          }
          if (image->comments == (char *) NULL)
            ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
          *p='\0';
          c=ReadByte(image);
        }
      else
        if (isalnum(c))
          {
            /*
              Determine a keyword and its value.
            */
            p=keyword;
            do
            {
              if ((p-keyword) < (MaxTextExtent-1))
                *p++=(char) c;
              c=ReadByte(image);
            } while (isalnum(c) || (c == '-'));
            *p='\0';
            while (isspace(c) || (c == '='))
              c=ReadByte(image);
            p=values;
            if (c != '"')
              while (!isspace(c) && (c != EOF))
              {
                if ((p-values) < (MaxTextExtent-1))
                  *p++=(char) c;
                c=ReadByte(image);
              }
            else
              {
                c=ReadByte(image);
                while ((c != '"') && (c != EOF))
                {
                  if ((p-values) < (MaxTextExtent-1))
                    *p++=(char) c;
                  c=ReadByte(image);
                }
              }
            *p='\0';
            /*
              Assign a value to the specified keyword.
            */
            if (Latin1Compare(keyword,"background-color") == 0)
              (void) QueryColorDatabase(values,&image->background_color);
            if (Latin1Compare(keyword,"blue-primary") == 0)
              (void) sscanf(values,"%lf,%lf",
                &image->chromaticity.blue_primary.x,
                &image->chromaticity.blue_primary.y);
            if (Latin1Compare(keyword,"border-color") == 0)
              (void) QueryColorDatabase(values,&image->border_color);
            if (Latin1Compare(keyword,"class") == 0)
              {
                if (Latin1Compare(values,"PseudoClass") == 0)
                  image->class=PseudoClass;
                else
                  if (Latin1Compare(values,"DirectClass") == 0)
                    image->class=DirectClass;
                  else
                    image->class=UndefinedClass;
              }
            if (Latin1Compare(keyword,"colors") == 0)
              image->colors=(unsigned int) atoi(values);
            if (Latin1Compare(keyword,"color-profile") == 0)
              image->color_profile.length=(unsigned int) atoi(values);
            if (Latin1Compare(keyword,"colorspace") == 0)
              {
                if (Latin1Compare(values,"CMYK") == 0)
                  image->colorspace=CMYKColorspace;
                else
                  if (Latin1Compare(values,"RGB") == 0)
                    image->colorspace=RGBColorspace;
              }
            if (Latin1Compare(keyword,"compression") == 0)
              {
                if (Latin1Compare(values,"Zip") == 0)
                  image->compression=ZipCompression;
                else
                  if (Latin1Compare(values,"BZip") == 0)
                    image->compression=BZipCompression;
                  else
                    if (Latin1Compare(values,"RunlengthEncoded") == 0)
                      image->compression=RunlengthEncodedCompression;
                    else
                      image->compression=UndefinedCompression;
              }
            if (Latin1Compare(keyword,"columns") == 0)
              image->columns=(unsigned int) atoi(values);
            if (Latin1Compare(keyword,"delay") == 0)
              {
                if (image_info->delay == (char *) NULL)
                  image->delay=atoi(values);
              }
            if (Latin1Compare(keyword,"depth") == 0)
              image->depth=atoi(values) <= 8 ? 8 : 16;
            if (Latin1Compare(keyword,"dispose") == 0)
              {
                if (image_info->dispose == (char *) NULL)
                  image->dispose=atoi(values);
              }
            if (Latin1Compare(keyword,"gamma") == 0)
              image->gamma=atof(values);
            if (Latin1Compare(keyword,"green-primary") == 0)
              (void) sscanf(values,"%lf,%lf",
                &image->chromaticity.green_primary.x,
                &image->chromaticity.green_primary.y);
            if (Latin1Compare(keyword,"id") == 0)
              (void) strcpy(id,values);
            if (Latin1Compare(keyword,"iterations") == 0)
              {
                if (image_info->iterations == (char *) NULL)
                  image->iterations=atoi(values);
              }
            if (Latin1Compare(keyword,"label") == 0)
              (void) CloneString(&image->label,values);
            if (Latin1Compare(keyword,"matte") == 0)
              image->matte=(Latin1Compare(values,"True") == 0) ||
                (Latin1Compare(values,"true") == 0);
            if (Latin1Compare(keyword,"matte-color") == 0)
              (void) QueryColorDatabase(values,&image->matte_color);
            if (Latin1Compare(keyword,"montage") == 0)
              (void) CloneString(&image->montage,values);
            if (Latin1Compare(keyword,"page") == 0)
              ParseImageGeometry(PostscriptGeometry(values),
                &image->page_info.x,&image->page_info.y,
                &image->page_info.width,&image->page_info.height);
            if (Latin1Compare(keyword,"red-primary") == 0)
              (void) sscanf(values,"%lf,%lf",&image->chromaticity.red_primary.x,
                &image->chromaticity.red_primary.y);
            if (Latin1Compare(keyword,"rendering-intent") == 0)
              {
                if (Latin1Compare(values,"saturation") == 0)
                  image->rendering_intent=SaturationIntent;
                else
                  if (Latin1Compare(values,"perceptual") == 0)
                    image->rendering_intent=PerceptualIntent;
                  else
                    if (Latin1Compare(values,"absolute") == 0)
                      image->rendering_intent=AbsoluteIntent;
                    else
                      if (Latin1Compare(values,"relative") == 0)
                        image->rendering_intent=RelativeIntent;
                      else
                        image->rendering_intent=UndefinedIntent;
              }
            if (Latin1Compare(keyword,"resolution") == 0)
              (void) sscanf(values,"%lfx%lf",&image->x_resolution,
                &image->y_resolution);
            if (Latin1Compare(keyword,"rows") == 0)
              image->rows=(unsigned int) atoi(values);
            if (Latin1Compare(keyword,"scene") == 0)
              image->scene=(unsigned int) atoi(values);
            if (Latin1Compare(keyword,"signature") == 0)
              (void) CloneString(&image->signature,values);
            if (Latin1Compare(keyword,"units") == 0)
              {
                if (Latin1Compare(values,"undefined") == 0)
                  image->units=UndefinedResolution;
                else
                  if (Latin1Compare(values,"pixels-per-inch") == 0)
                    image->units=PixelsPerInchResolution;
                  else
                    if (Latin1Compare(values,"pixels-per-centimeter") == 0)
                      image->units=PixelsPerCentimeterResolution;
              }
            if (Latin1Compare(keyword,"white-point") == 0)
              (void) sscanf(values,"%lf,%lf",&image->chromaticity.white_point.x,
                &image->chromaticity.white_point.y);
          }
        else
          c=ReadByte(image);
      while (isspace(c))
        c=ReadByte(image);
    }
    (void) ReadByte(image);
    /*
      Verify that required image information is defined.
    */
    if ((strcmp(id,"ImageMagick") != 0) || (image->class == UndefinedClass) ||
        (image->compression == UndefinedCompression) || (image->columns == 0) ||
        (image->rows == 0))
      ReaderExit(CorruptImageWarning,"Incorrect image header in file",image);
    if (image_info->ping)
      {
        CloseBlob(image);
        return(image);
      }
    if (image->montage != (char *) NULL)
      {
        register char
          *p;

        /*
          Image directory.
        */
        image->directory=(char *) AllocateMemory(MaxTextExtent*sizeof(char));
        if (image->directory == (char *) NULL)
          ReaderExit(CorruptImageWarning,"Unable to read image data",image);
        p=image->directory;
        do
        {
          *p='\0';
          if (((Extent(image->directory)+1) % MaxTextExtent) == 0)
            {
              /*
                Allocate more memory for the image directory.
              */
              image->directory=(char *) ReallocateMemory((char *)
                image->directory,(Extent(image->directory)+MaxTextExtent+1)*
                sizeof(char));
              if (image->directory == (char *) NULL)
                ReaderExit(CorruptImageWarning,"Unable to read image data",
                  image);
              p=image->directory+Extent(image->directory);
            }
          c=ReadByte(image);
          *p++=(unsigned char) c;
        } while (c != '\0');
      }
    if (image->color_profile.length > 0)
      {
        /*
          Color profile.
        */
        image->color_profile.info=(unsigned char *)
          AllocateMemory(image->color_profile.length*sizeof(unsigned char));
        if (image->color_profile.info == (unsigned char *) NULL)
          ReaderExit(CorruptImageWarning,"Unable to read color profile",image);
        (void) ReadBlob(image,image->color_profile.length,
          image->color_profile.info);
      }
    if (image->class == PseudoClass)
      {
        /*
          Create image colormap.
        */
        image->colormap=(PixelPacket *)
          AllocateMemory(Max(image->colors,256)*sizeof(PixelPacket));
        if (image->colormap == (PixelPacket *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        if (image->colors == 0)
          for (i=0; i < 256; i++)
          {
            image->colormap[i].red=UpScale(i);
            image->colormap[i].green=UpScale(i);
            image->colormap[i].blue=UpScale(i);
            image->colors++;
          }
        else
          {
            unsigned char
              *colormap;

            unsigned int
              packet_size;

            /*
              Read image colormap from file.
            */
            packet_size=image->colors > 256 ? 6 : 3;
            colormap=(unsigned char *)
              AllocateMemory(packet_size*image->colors*sizeof(unsigned char));
            if (colormap == (unsigned char *) NULL)
              ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
            (void) ReadBlob(image,packet_size*image->colors,colormap);
            p=colormap;
            if (image->colors <= 256)
              for (i=0; i < (int) image->colors; i++)
              {
                image->colormap[i].red=UpScale(*p++);
                image->colormap[i].green=UpScale(*p++);
                image->colormap[i].blue=UpScale(*p++);
              }
            else
              for (i=0; i < (int) image->colors; i++)
              {
                image->colormap[i].red=(*p++ << 8);
                image->colormap[i].red|=(*p++);
                image->colormap[i].green=(*p++ << 8);
                image->colormap[i].green|=(*p++);
                image->colormap[i].blue=(*p++ << 8);
                image->colormap[i].blue|=(*p++);
              }
            FreeMemory(colormap);
          }
      }
    /*
      Allocate image pixels.
    */
    if (image->class == DirectClass)
      packet_size=image->depth > 8 ? 6 : 3;
    else
      packet_size=image->colors > 256 ? 2 : 1;
    if (image->matte || (image->colorspace == CMYKColorspace))
      packet_size+=image->depth > 8 ? 2 : 1;
    if (image->compression == RunlengthEncodedCompression)
      packet_size+=image->depth > 8 ? 2 : 1;
    pixels=(unsigned char *)
      AllocateMemory(packet_size*image->columns*sizeof(unsigned char));
    compressed_pixels=(unsigned char *) AllocateMemory((unsigned int)
      (1.01*packet_size*image->columns+600)*sizeof(unsigned char));
    if ((pixels == (unsigned char *) NULL) ||
        (compressed_pixels == (unsigned char *) NULL))
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    /*
      Read image pixels.
    */
    index=0;
    length=0;
    for (y=0; y < (int) image->rows; y++)
    {
      q=SetPixelCache(image,0,y,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
#if defined(HasZLIB)
        if (image->compression == ZipCompression)
          {
            if (y == 0)
              {
                zip_info.zalloc=NULL;
                zip_info.zfree=NULL;
                zip_info.opaque=NULL;
                (void) inflateInit(&zip_info);
                zip_info.avail_in=0;
              }
            zip_info.next_out=pixels;
            zip_info.avail_out=packet_size*image->columns;
            do
            {
              if (zip_info.avail_in == 0)
                {
                  zip_info.next_in=compressed_pixels;
                  length=1.01*packet_size*image->columns+12;
                  zip_info.avail_in=ReadBlob(image,length,zip_info.next_in);
                }
              if (inflate(&zip_info,Z_NO_FLUSH) == Z_STREAM_END)
                break;
            } while (zip_info.avail_out > 0);
            if (y == (int) (image->rows-1))
              {
                (void) SeekBlob(image,-((off_t) zip_info.avail_in),SEEK_CUR);
                status=!inflateEnd(&zip_info);
              }
          }
        else
#endif
#if defined(HasBZLIB)
        if (image->compression == BZipCompression)
          {
            if (y == 0)
              {
                bzip_info.bzalloc=NULL;
                bzip_info.bzfree=NULL;
                bzip_info.opaque=NULL;
                (void) bzDecompressInit(&bzip_info,image_info->verbose,False);
                bzip_info.avail_in=0;
              }
            bzip_info.next_out=(char *) pixels;
            bzip_info.avail_out=packet_size*image->columns;
            do
            {
              if (bzip_info.avail_in == 0)
                {
                  bzip_info.next_in=(char *) compressed_pixels;
                  length=1.01*packet_size*image->columns+12;
                  bzip_info.avail_in=ReadBlob(image,length,bzip_info.next_in);
                }
              if (bzDecompress(&bzip_info) == BZ_STREAM_END)
                break;
            } while (bzip_info.avail_out > 0);
            if (y == (int) (image->rows-1))
              {
                (void) SeekBlob(image,-((off_t) bzip_info.avail_in),SEEK_CUR);
                status=!bzDecompressEnd(&bzip_info);
              }
          }
        else
#endif
          if (image->compression != RunlengthEncodedCompression)
            (void) ReadBlob(image,packet_size*image->columns,pixels);
      if (image->compression != RunlengthEncodedCompression)
        {
          if (image->class == PseudoClass)
            {
              if (!image->matte)
                (void) ReadPixelCache(image,IndexQuantum,pixels);
              else
                (void) ReadPixelCache(image,IndexOpacityQuantum,pixels);
            }
          else
            if (image->colorspace == CMYKColorspace)
              (void) ReadPixelCache(image,CMYKQuantum,pixels);
            else
              if (!image->matte)
                (void) ReadPixelCache(image,RGBQuantum,pixels);
              else
                (void) ReadPixelCache(image,RGBAQuantum,pixels);
        }
      else
        {
          if (y == 0)
            GetPixelPacket(&pixel);
          p=pixels;
          for (x=0; x < (int) image->columns; x++)
          {
            if (length == 0)
              {
                if (image->class != DirectClass)
                  {
                    index=ReadByte(image);
                    if (image->colors > 256)
                      index=(index << 8)+ReadByte(image);
                    if (index >= image->colors)
                      ReaderExit(CorruptImageWarning,"invalid colormap index",
                        image);
                    pixel=image->colormap[index];
                  }
                else
                  {
                    if (image->depth <= 8)
                      {
                        pixel.red=UpScale(ReadByte(image));
                        pixel.green=UpScale(ReadByte(image));
                        pixel.blue=UpScale(ReadByte(image));
                        if (image->matte ||
                            (image->colorspace == CMYKColorspace))
                          pixel.opacity=UpScale(ReadByte(image));
                      }
                    else
                      {
                        pixel.red=MSBFirstReadShort(image) >>
                          (image->depth-QuantumDepth);
                        pixel.green=MSBFirstReadShort(image) >>
                          (image->depth-QuantumDepth);
                        pixel.blue=MSBFirstReadShort(image) >>
                          (image->depth-QuantumDepth);
                        if (image->matte ||
                            (image->colorspace == CMYKColorspace))
                          pixel.opacity=MSBFirstReadShort(image) >>
                            (image->depth-QuantumDepth);
                      }
                  }
                length=ReadByte(image)+1;
              }
            length--;
            if (image->class == PseudoClass)
              image->indexes[x]=index;
            *q++=pixel;
          }
        }
      if (!SyncPixelCache(image))
        break;
    }
    FreeMemory(pixels);
    FreeMemory(compressed_pixels);
    if (status == False)
      {
        DestroyImages(image);
        return((Image *) NULL);
      }
    /*
      Proceed to next image.
    */
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
    do
    {
      c=ReadByte(image);
    } while (!isgraph(c) && (c != EOF));
    if (c != EOF)
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
  } while (c != EOF);
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
%   W r i t e M I F F I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteMIFFImage writes an image to a file.
%
%  The format of the WriteMIFFImage method is:
%
%      unsigned int WriteMIFFImage(const ImageInfo *image_info,Image *image)
%
%  Compression code contributed by Kyle Shorter.
%
%  A description of each parameter follows:
%
%    o status: Method WriteMIFFImage return True if the image is written.
%      False is returned if there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: A pointer to a Image structure.
%
%
*/
Export unsigned int WriteMIFFImage(const ImageInfo *image_info,Image *image)
{
#if defined(HasBZLIB)
  bz_stream
    bzip_info;
#endif

  char
    buffer[MaxTextExtent],
    color[MaxTextExtent];

  CompressionType
    compression;

  IndexPacket
    index;

  int
    length,
    y;

  PixelPacket
    pixel;

  register PixelPacket
    *p;

  register int
    i,
    x;

  register unsigned char
    *q;

  unsigned char
    *compressed_pixels,
    *pixels;

  unsigned int
    packet_size,
    scene,
    status,
    value;

#if defined(HasZLIB)
  z_stream
    zip_info;
#endif

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  (void) IsPseudoClass(image);
  (void) strcpy((char *) image_info->magick,"MIFF");
  compression=image->compression;
  if (image_info->compression != UndefinedCompression)
    compression=image_info->compression;
  scene=0;
  do
  {
    /*
      Allocate image pixels.
    */
    if (image->class == DirectClass)
      packet_size=image->depth > 8 ? 6 : 3;
    else
      packet_size=image->colors > 256 ? 2 : 1;
    if (image->matte || (image->colorspace == CMYKColorspace))
      packet_size+=image->depth > 8 ? 2 : 1;
    if (compression == RunlengthEncodedCompression)
      packet_size+=image->depth > 8 ? 2 : 1;
    pixels=(unsigned char *)
      AllocateMemory(packet_size*image->columns*sizeof(unsigned char));
    compressed_pixels=(unsigned char *) AllocateMemory((unsigned int)
      (1.01*packet_size*image->columns+600)*sizeof(unsigned char));
    if ((pixels == (unsigned char *) NULL) ||
        (compressed_pixels == (unsigned char *) NULL))
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    /*
      Write MIFF header.
    */
    if (((image_info->colorspace != UndefinedColorspace) ||
         (image->colorspace != CMYKColorspace)) &&
         (image_info->colorspace != CMYKColorspace))
      TransformRGBImage(image,RGBColorspace);
    else
      if (image->colorspace != CMYKColorspace)
        RGBTransformImage(image,CMYKColorspace);
    (void) strcpy(buffer,"id=ImageMagick\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    if (image->class == PseudoClass)
      (void) sprintf(buffer,"class=PseudoClass  colors=%u  matte=%s\n",
        image->colors,image->matte ? "True" : "False");
    else
      if (image->colorspace != CMYKColorspace)
        (void) sprintf(buffer,"class=DirectClass  matte=%s\n",
          image->matte ? "True" : "False");
      else
        (void) strcpy(buffer,"class=DirectClass  colorspace=CMYK\n");
    (void) WriteBlob(image,strlen(buffer),buffer);
    *buffer='\0';
    if (compression == RunlengthEncodedCompression)
      (void) sprintf(buffer,"compression=RunlengthEncoded\n");
    else
      if (compression == BZipCompression)
        (void) sprintf(buffer,"compression=BZip\n");
      else
        if (compression == ZipCompression)
          (void) sprintf(buffer,"compression=Zip\n");
    if (*buffer != '\0')
      (void) WriteBlob(image,strlen(buffer),buffer);
    (void) sprintf(buffer,"columns=%u  rows=%u  depth=%u\n",image->columns,
      image->rows,image->depth);
    (void) WriteBlob(image,strlen(buffer),buffer);
    if ((image->x_resolution != 0) && (image->y_resolution != 0))
      {
        char
          units[MaxTextExtent];

        /*
          Set image resolution.
        */
        (void) strcpy(units,"undefined");
        if (image->units == PixelsPerInchResolution)
          (void) strcpy(units,"pixels-per-inch");
        if (image->units == PixelsPerCentimeterResolution)
          (void) strcpy(units,"pixels-per-centimeter");
        (void) sprintf(buffer,"resolution=%gx%g  units=%.1024s\n",
          image->x_resolution,image->y_resolution,units);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    SignatureImage(image);
    if (image->signature != (char *) NULL)
      {
        (void) sprintf(buffer,"signature=%.1024s\n",image->signature);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    if ((image->page_info.width != 0) && (image->page_info.height != 0))
      {
        (void) sprintf(buffer,"page=%ux%u%+d%+d\n",image->page_info.width,
          image->page_info.height,image->page_info.x,image->page_info.y);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    (void) QueryColorName(&image->background_color,color);
    (void) sprintf(buffer,"background-color=%.1024s  ",color);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) QueryColorName(&image->border_color,color);
    (void) sprintf(buffer,"border-color=%.1024s  ",color);
    (void) WriteBlob(image,strlen(buffer),buffer);
    (void) QueryColorName(&image->matte_color,color);
    (void) sprintf(buffer,"matte-color=%.1024s\n",color);
    (void) WriteBlob(image,strlen(buffer),buffer);
    if ((image->next != (Image *) NULL) || (image->previous != (Image *) NULL))
      {
        (void) sprintf(buffer,"scene=%u  iterations=%u  delay=%u  dispose=%u\n",
          image->scene,image->iterations,image->delay,image->dispose);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    else
      {
        if (image->scene != 0)
          {
            (void) sprintf(buffer,"scene=%u\n",image->scene);
            (void) WriteBlob(image,strlen(buffer),buffer);
          }
        if (image->iterations != 1)
          {
            (void) sprintf(buffer,"iterations=%u\n",image->iterations);
            (void) WriteBlob(image,strlen(buffer),buffer);
          }
        if (image->delay != 0)
          {
            (void) sprintf(buffer,"delay=%u\n",image->delay);
            (void) WriteBlob(image,strlen(buffer),buffer);
          }
        if (image->dispose != 0)
          {
            (void) sprintf(buffer,"dispose=%u\n",image->dispose);
            (void) WriteBlob(image,strlen(buffer),buffer);
          }
      }
    if (image->rendering_intent != UndefinedIntent)
      {
        if (image->rendering_intent == SaturationIntent)
          (void) strcpy(buffer,"rendering-intent=saturation\n");
        else
          if (image->rendering_intent == PerceptualIntent)
            (void) strcpy(buffer,"rendering-intent=perceptual\n");
          else
            if (image->rendering_intent == AbsoluteIntent)
              (void) strcpy(buffer,"rendering-intent=absolute\n");
            else
              (void) strcpy(buffer,"rendering-intent=relative\n");
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    if (image->gamma != 0.0)
      {
        (void) sprintf(buffer,"gamma=%g\n",image->gamma);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    if (image->chromaticity.white_point.x != 0.0)
      {
        /*
          Note chomaticity points.
        */
        (void) sprintf(buffer,
          "red-primary=%g,%g  green-primary=%g,%g  blue-primary=%g,%g\n",
          image->chromaticity.red_primary.x,image->chromaticity.red_primary.y,
          image->chromaticity.green_primary.x,
          image->chromaticity.green_primary.y,
          image->chromaticity.blue_primary.x,
          image->chromaticity.blue_primary.y);
        (void) WriteBlob(image,strlen(buffer),buffer);
        (void) sprintf(buffer,"white-point=%g,%g\n",
          image->chromaticity.white_point.x,image->chromaticity.white_point.y);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    if (image->color_profile.length > 0)
      {
        (void) sprintf(buffer,"color-profile=%u\n",image->color_profile.length);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    if (image->montage != (char *) NULL)
      {
        (void) sprintf(buffer,"montage=%.1024s\n",image->montage);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    if (image->label != (char *) NULL)
      {
        (void) sprintf(buffer,"label=\"%.1024s\"\n",image->label);
        (void) WriteBlob(image,strlen(buffer),buffer);
      }
    if (image->comments != (char *) NULL)
      {
        (void) WriteByte(image,'{');
        (void) WriteByte(image,'\n');
        (void) WriteBlob(image,strlen(image->comments),image->comments);
        (void) WriteByte(image,'}');
        (void) WriteByte(image,'\n');
      }
    (void) strcpy(buffer,"\f\n:\032");
    (void) WriteBlob(image,strlen(buffer),buffer);
    if (image->montage != (char *) NULL)
      {
        /*
          Write montage tile directory.
        */
        if (image->directory != (char *) NULL)
          (void) WriteBlob(image,strlen(image->directory),image->directory);
        (void) WriteByte(image,'\0');
      }
    if (image->color_profile.length > 0)
      (void) WriteBlob(image,(int) image->color_profile.length,
        (char *) image->color_profile.info);
    if (image->class == PseudoClass)
      {
        register unsigned char
          *q;

        unsigned char
          *colormap;

        unsigned int
          packet_size;

        /*
          Allocate colormap.
        */
        packet_size=image->colors > 256 ? 6 : 3;
        colormap=(unsigned char *)
          AllocateMemory(packet_size*image->colors*sizeof(unsigned char));
        if (colormap == (unsigned char *) NULL)
          WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
        /*
          Write colormap to file.
        */
        q=colormap;
        if (image->colors <= 256)
          for (i=0; i < (int) image->colors; i++)
          {
            *q++=image->colormap[i].red;
            *q++=image->colormap[i].green;
            *q++=image->colormap[i].blue;
          }
        else
          for (i=0; i < (int) image->colors; i++)
          {
            *q++=image->colormap[i].red >> 8;
            *q++=image->colormap[i].red & 0xff;
            *q++=image->colormap[i].green >> 8;
            *q++=image->colormap[i].green  & 0xff;
            *q++=image->colormap[i].blue >> 8;
            *q++=image->colormap[i].blue  & 0xff;
          }
        (void) WriteBlob(image,packet_size*image->colors,colormap);
        FreeMemory(colormap);
      }
    /*
      Write image pixels to file.
    */
    status=True;
    for (y=0; y < (int) image->rows; y++)
    {
      p=GetPixelCache(image,0,y,image->columns,1);
      if (p == (PixelPacket *) NULL)
        break;
      q=pixels;
      if (compression != RunlengthEncodedCompression)
        {
          if (image->class == PseudoClass)
            {
              if (!image->matte)
                (void) WritePixelCache(image,IndexQuantum,pixels);
              else
                (void) WritePixelCache(image,IndexOpacityQuantum,pixels);
            }
          else
            if (image->colorspace == CMYKColorspace)
              (void) WritePixelCache(image,CMYKQuantum,pixels);
            else
              if (!image->matte)
                (void) WritePixelCache(image,RGBQuantum,pixels);
              else
                (void) WritePixelCache(image,RGBAQuantum,pixels);
        }
      else
        {
          pixel=(*p);
          index=0;
          if (image->class == PseudoClass)
            index=(*image->indexes);
          length=0;
          for (x=0; x < (int) image->columns; x++)
          {
            if ((p->red == pixel.red) && (p->green == pixel.green) && 
                (p->blue == pixel.blue) && (p->opacity == pixel.opacity) &&
                (length < 255) && (x < (image->columns-1)))
              length++;
            else
              {
                if (image->class != DirectClass)
                  {
                    if (image->colors > 256)
                      *q++=index >> 8;
                    *q++=index;
                  }
                else
                  {
                    if (image->depth <= 8)
                      {
                        *q++=DownScale(pixel.red);
                        *q++=DownScale(pixel.green);
                        *q++=DownScale(pixel.blue);
                        if (image->matte ||
                            (image->colorspace == CMYKColorspace))
                          *q++=DownScale(pixel.opacity);
                      }
                    else
                      {
                        value=pixel.red;
                        if ((QuantumDepth-image->depth) > 0)
                          value*=257;
                        *q++=value >> 8;
                        *q++=value & 0xff;
                        value=pixel.green;
                        if ((QuantumDepth-image->depth) > 0)
                          value*=257;
                        *q++=value >> 8;
                        *q++=value & 0xff;
                        value=pixel.blue;
                        if ((QuantumDepth-image->depth) > 0)
                          value*=257;
                        *q++=value >> 8;
                        *q++=value & 0xff;
                        if (image->matte ||
                            (image->colorspace == CMYKColorspace))
                          {
                            value=pixel.opacity;
                            if ((QuantumDepth-image->depth) > 0)
                              value*=257;
                            *q++=value >> 8;
                            *q++=value & 0xff;
                          }
                      }
                  }
                *q++=(unsigned char) length;
                length=0;
              }
            if (image->class == PseudoClass)
              index=image->indexes[x];
            pixel=(*p);
            p++;
          }
        }
#if defined(HasZLIB)
        if (compression == ZipCompression)
          {
            if (y == 0)
              {
                zip_info.zalloc=NULL;
                zip_info.zfree=NULL;
                zip_info.opaque=NULL;
                (void) deflateInit(&zip_info,Min(image_info->quality/10,9));
              }
            zip_info.next_in=pixels;
            zip_info.avail_in=packet_size*image->columns;
            do
            {
              zip_info.next_out=compressed_pixels;
              zip_info.avail_out=1.01*packet_size*image->columns+12;
              status=!deflate(&zip_info,Z_NO_FLUSH);
              length=zip_info.next_out-compressed_pixels;
              if (zip_info.next_out != compressed_pixels)
                (void) WriteBlob(image,length,compressed_pixels);
            } while (zip_info.avail_in > 0);
            if (y == (int) (image->rows-1))
              {
                for ( ; ; )
                {
                  zip_info.next_out=compressed_pixels;
                  zip_info.avail_out=1.01+packet_size*image->columns+12;
                  status=!deflate(&zip_info,Z_FINISH);
                  if (zip_info.next_out == compressed_pixels)
                    break;
                  length=zip_info.next_out-compressed_pixels;
                  (void) WriteBlob(image,length,compressed_pixels);
                }
                status=!deflateEnd(&zip_info);
              }
          }
        else
#endif
#if defined(HasBZLIB)
        if (compression == BZipCompression)
          {
            if (y == 0)
              {
                bzip_info.bzalloc=NULL;
                bzip_info.bzfree=NULL;
                bzip_info.opaque=NULL;
                (void) bzCompressInit(&bzip_info,Min(image_info->quality/10,9),
                  image_info->verbose,0);
              }
            bzip_info.next_in=(char *) pixels;
            bzip_info.avail_in=packet_size*image->columns;
            do
            {
              bzip_info.next_out=(char *) compressed_pixels;
              bzip_info.avail_out=packet_size*image->columns+600;
              (void) bzCompress(&bzip_info,BZ_RUN);
              length=bzip_info.next_out-(char *) compressed_pixels;
              if (bzip_info.next_out != (char *) compressed_pixels)
                (void) WriteBlob(image,length,compressed_pixels);
            } while (bzip_info.avail_in > 0);
            if (y == (int) (image->rows-1))
              {
                for ( ; ; )
                {
                  bzip_info.next_out=(char *) compressed_pixels;
                  bzip_info.avail_out=packet_size*image->columns+600;
                  (void) bzCompress(&bzip_info,BZ_FINISH);
                  if (bzip_info.next_out == (char *) compressed_pixels)
                    break;
                  length=bzip_info.next_out-(char *) compressed_pixels;
                  (void) WriteBlob(image,length,compressed_pixels);
                }
                status=!bzCompressEnd(&bzip_info);
              }
          }
        else
#endif
          if (compression == RunlengthEncodedCompression)
            status=WriteBlob(image,q-pixels,pixels);
          else
            status=WriteBlob(image,packet_size*image->columns,pixels);
      if (image->previous == (Image *) NULL)
        if (QuantumTick(y,image->rows))
          ProgressMonitor(SaveImageText,y,image->rows);
    }
    FreeMemory(pixels);
    FreeMemory(compressed_pixels);
    if (image->next == (Image *) NULL)
      break;
    image=GetNextImage(image);
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  CloseBlob(image);
  return(status);
}
