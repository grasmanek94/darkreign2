/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            EEEEE  PPPP   TTTTT                              %
%                            E      P   P    T                                %
%                            EEE    PPPP     T                                %
%                            E      P        T                                %
%                            EEEEE  P        T                                %
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
%   I s E P T                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsEPT returns True if the image format type, identified by the
%  magick string, is EPT.
%
%  The format of the ReadEPTImage method is:
%
%      unsigned int IsEPT(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsEPT returns True if the image format type is EPT.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsEPT(const unsigned char *magick,const unsigned int length)
{
  if (length < 4)
    return(False);
  if (strncmp((char *) magick,"\305\320\323\306",4) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e E P T I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteEPTImage writes an image in the Adobe Encapsulated Postscript
%  format with a TIFF preview.
%
%  The format of the WriteEPTImage method is:
%
%      unsigned int WriteEPTImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteEPTImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteEPTImage(const ImageInfo *image_info,Image *image)
{
  char
    filename[MaxTextExtent];

  FILE
    *ps_file,
    *tiff_file;

  int
    c;

  unsigned int
    status;

  unsigned long
    eps_length,
    tiff_length;

  ps_file=(FILE *) NULL;
  if (Latin1Compare(image_info->magick,"PS") == 0)
    ps_file=fopen(image->magick_filename,ReadBinaryType);
  if (ps_file != (FILE *) NULL)
    {
      struct stat
        attributes;

      /*
        Read existing Encapsulated Postscript.
      */
      eps_length=
        fstat(fileno(ps_file),&attributes) < 0 ? 0 : attributes.st_size;
    }
  else
    {
      /*
        Write image as Encapsulated Postscript to a temporary file.
      */
      (void) strcpy(filename,image->filename);
      TemporaryFilename(image->filename);
      status=WritePSImage(image_info,image);
      if (status == False)
        return(status);
      status=OpenBlob(image_info,image,ReadBinaryType);
      if (status == False)
        WriterExit(FileOpenWarning,"Unable to open file",image);
      (void) remove(image->filename);
      eps_length=image->filesize;
      ps_file=image->file;
      image->file=(FILE *) NULL;
    }
  /*
    Write image as TIFF to a temporary file.
  */
  TemporaryFilename(image->filename);
  status=WriteTIFFImage(image_info,image);
  if (status == False)
    return(status);
  status=OpenBlob(image_info,image,ReadBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  (void) remove(image->filename);
  tiff_length=image->filesize;
  tiff_file=image->file;
  image->file=(FILE *) NULL;
  /*
    Write EPT image.
  */
  (void) strcpy(image->filename,filename);
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  LSBFirstWriteLong(image,0xc6d3d0c5ul);
  LSBFirstWriteLong(image,30);
  LSBFirstWriteLong(image,eps_length);
  LSBFirstWriteLong(image,0);
  LSBFirstWriteLong(image,0);
  LSBFirstWriteLong(image,eps_length+30);
  LSBFirstWriteLong(image,tiff_length);
  LSBFirstWriteShort(image,0xffff);
  for (c=fgetc(ps_file); c != EOF; c=fgetc(ps_file))
    (void) WriteByte(image,(char) c);
  for (c=fgetc(tiff_file); c != EOF; c=fgetc(tiff_file))
    (void) WriteByte(image,(char) c);
  (void) fclose(tiff_file);
  CloseBlob(image);
  return(True);
}
