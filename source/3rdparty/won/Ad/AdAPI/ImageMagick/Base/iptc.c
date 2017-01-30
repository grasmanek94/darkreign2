/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        IIIII  PPPP   TTTTT   CCCC                           %
%                          I    P   P    T    C                               %
%                          I    PPPP     T    C                               %
%                          I    P        T    C                               %
%                        IIIII  P        T     CCCC                           %
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
%   I s I P T C                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsIPTC returns True if the image format type, identified by the
%  magick string, is IPTC.
%
%  The format of the ReadIPTCImage method is:
%
%      unsigned int IsIPTC(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsIPTC returns True if the image format type is IPTC.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsIPTC(const unsigned char *magick,
  const unsigned int length)
{
  if (length < 2)
    return(False);
  if (strncmp((char *) magick,"\034\002",2) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d I P T C I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadIPTCImage reads an image file in the IPTC format and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.  This method differs from the other decoder
%  methods in that only the iptc profile information is useful in the
%  returned image.
%
%  The format of the ReadIPTCImage method is:
%
%      Image *ReadIPTCImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadIPTCImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadIPTCImage(const ImageInfo *image_info)
{
  Image
    *image;

  int
    c;

  register unsigned char
    *q;

  unsigned char
    *data;

  unsigned int
    tag_length,
    length,
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
    Read IPTC image.
  */
  length=MaxTextExtent;
  tag_length=12;
  data=(unsigned char *)
    AllocateMemory((length+2)*sizeof(unsigned char));
  if (data == (unsigned char *) NULL)
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  (void) memcpy((char *) data,"8BIM\04\04\0\0\0\0\0\0",tag_length);
  q=data;
  q+=tag_length;
  while (1)
  {
    c=ReadByte(image);
    if (c == EOF)
      break;
    if ((q-data+1) >= (int) length)
      {
        image->iptc_profile.length=q-data;
        length<<=1;
        data=(unsigned char *) ReallocateMemory((char *)
          data,(length+2)*sizeof(unsigned char));
        if (data == (unsigned char *) NULL)
          break;
        q=data+image->iptc_profile.length;
      }
    *q++=(unsigned char) c;
  }
  image->iptc_profile.length=0;
  if (data != (unsigned char *) NULL)
    {
      image->iptc_profile.length=q-data;
      length=image->iptc_profile.length-tag_length;
      data[10]=length >> 8;
      data[11]=length & 0xff;
      image->iptc_profile.info=data;
    }
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e I P T C I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteIPTCImage writes an image in the IPTC format.
%
%  Contributed by Bill Radcliffe.
%
%  The format of the WriteIPTCImage method is:
%
%      unsigned int WriteIPTCImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteIPTCImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/

static long GetIPTCStream(unsigned char **info,long length)
{
  register unsigned char
    *p;

  unsigned char
    c;

  unsigned int
    info_length,
    marker,
    tag_length;

  /*
    Find the beginning of the IPTC info.
  */
  p=(*info);
  tag_length=0;
  info_length=0;
  marker=False;
  while (length > 0)
  {
    c=(*p++);
    length--;
    if (length <= 0)
      break; 
    if (c == 0x1c)
      {
        p--;
        *info=p; /* let the caller know were it is */
        break;
      }
  }
  /*
    Determine the length of the IPTC info.
  */
  while (length > 0)
  {
    c=(*p++);
    length--;
    if (length <= 0)
      break; 
    if (c == 0x1c)
      marker=True;
    else
      if (marker)
        break; 
      else
        continue;
    info_length++;
    /*
      Found the 0x1c tag; skip the dataset and record number tags.
    */
    p++; /* dataset */
    length--;
    if (length <= 0)
      break; 
    info_length++;
    p++; /* record number */
    length--;
    if (length <= 0)
      break; 
    info_length++;
    /*
      Then we decode the length of the block that follows - long or short fmt.
    */
    c=(*p++);
    length--;
    if (length <= 0)
      break; 
    info_length++;
    if (c & (unsigned char) 0x80)
      {
        unsigned char
          buffer[4];

        int
          i;

        for (i=0; i < 4; i++)
        {
          buffer[i]=(*p++);
          length--;
          if (length <= 0)
            break; 
          info_length++;
        }
        tag_length=(((long) buffer[0]) << 24) | (((long) buffer[1]) << 16) |
          (((long) buffer[2]) << 8) | (((long) buffer[3]));
      }
    else
      {
        tag_length=((long) c) << 8;
        c=(*p++);
        length--;
        if (length <= 0)
          break; 
        info_length++;
        tag_length|=(long) c;
      }
    p+=tag_length;
    length-=tag_length;
    info_length+=tag_length;  
  }
  return(info_length);
}

Export unsigned int WriteIPTCImage(const ImageInfo *image_info,Image *image)
{
  unsigned int
    status;

  unsigned char
    *info;

  long
    length;

  if (image->iptc_profile.length == 0)
    WriterExit(FileOpenWarning,"No IPTC profile available",image);
  info=image->iptc_profile.info;
  length=image->iptc_profile.length;
  length=GetIPTCStream(&info,length);
  if (length <= 0)
    WriterExit(FileOpenWarning,"No IPTC info was found",image);
  /*
    Open image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  (void) WriteBlob(image,(int) length,info);
  CloseBlob(image);
  return(True);
}
