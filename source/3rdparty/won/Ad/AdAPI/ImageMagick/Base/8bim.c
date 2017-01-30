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
%   I s 8 B I M                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method Is8BIM returns True if the image format type, identified by the
%  magick string, is 8BIM.
%
%  The format of the Read8BIMImage method is:
%
%      unsigned int Is8BIM(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method Is8BIM returns True if the image format type is 8BIM.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int Is8BIM(const unsigned char *magick,
  const unsigned int length)
{
  if (length < 4)
    return(False);
  if (strncmp((char *) (magick),"8BIM",4) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d 8 B I M I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method Read8BIMImage reads an binary file in the 8BIM format and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.  This method differs from the other decoder
%  methods in that only the Photoshop resource (8BIM) information is useful in
%  the returned image.
%
%  The format of the Read8BIMImage method is:
%
%      Image *Read8BIMImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method Read8BIMImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *Read8BIMImage(const ImageInfo *image_info)
{
  Image
    *image;

  int
    c;

  register unsigned char
    *q;

  unsigned int
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
    Read 8BIM image.
  */
  length=MaxTextExtent;
  image->iptc_profile.info=(unsigned char *)
    AllocateMemory((length+2)*sizeof(unsigned char));
  for (q=image->iptc_profile.info; ; q++)
  {
    c=ReadByte(image);
    if (c == EOF)
      break;
    if ((q-image->iptc_profile.info+1) >= (int) length)
      {
        image->iptc_profile.length=q-image->iptc_profile.info;
        length<<=1;
        image->iptc_profile.info=(unsigned char *) ReallocateMemory((char *)
          image->iptc_profile.info,(length+2)*sizeof(unsigned char));
        if (image->iptc_profile.info == (unsigned char *) NULL)
          break;
        q=image->iptc_profile.info+image->iptc_profile.length;
      }
    *q=(unsigned char) c;
  }
  image->iptc_profile.length=0;
  if (image->iptc_profile.info != (unsigned char *) NULL)
    image->iptc_profile.length=q-image->iptc_profile.info;
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e 8 B I M I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method Write8BIMImage writes an image in the 8BIM format.
%
%  The format of the Write8BIMImage method is:
%
%      unsigned int Write8BIMImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method Write8BIMImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int Write8BIMImage(const ImageInfo *image_info,Image *image)
{
  unsigned int
    status;

  if (image->iptc_profile.length == 0)
    WriterExit(FileOpenWarning,"No 8BIM data is available",image);
  /*
    Open image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  (void) WriteBlob(image,(int) image->iptc_profile.length,
    (char *) image->iptc_profile.info);
  CloseBlob(image);
  return(True);
}
