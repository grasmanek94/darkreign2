/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        JJJJJ  PPPP   EEEEE   GGGG                           %
%                          J    P   P  E      G                               %
%                          J    PPPP   EEE    G  GG                           %
%                        J J    P      E      G   G                           %
%                        JJJ    P      EEEEE   GGG                            %
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
% This software is based in part on the work of the Independent JPEG Group.
% See ftp://ftp.uu.net/graphics/jpeg/jpegsrc.v6b.tar.gz for copyright and
% licensing restrictions.  Blob support contributed by Glenn Randers-Pehrson.
%
%
*/

/*
  Include declarations.
*/
#include "magick.h"
#include "defines.h"
#if defined(HasJPEG)
#define JPEG_INTERNAL_OPTIONS
#include <setjmp.h>
#include "jpeglib.h"
#include "jerror.h"

/*
  Define declarations.
*/
#define ICC_MARKER  (JPEG_APP0+2)
#define IPTC_MARKER  (JPEG_APP0+13)
#define MaxBufferExtent  8192

typedef struct _DestinationManager
{
  struct jpeg_destination_mgr
    manager;

  Image
    *image;

  JOCTET
    *buffer;
} DestinationManager;

typedef struct _SourceManager
{
  struct jpeg_source_mgr
    manager;

  Image
    *image;

  JOCTET
    *buffer;

  boolean
    start_of_blob;
} SourceManager;

static Image
  *image;

static jmp_buf
  error_recovery;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s J P E G                                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsJPEG returns True if the image format type, identified by the
%  magick string, is JPEG.
%
%  The format of the ReadJPEGImage method is:
%
%      unsigned int IsJPEG(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsJPEG returns True if the image format type is JPEG.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsJPEG(const unsigned char *magick,
  const unsigned int length)
{
  if (length < 3)
    return(False);
  if (strncmp((char *) magick,"\377\330\377",3) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d J P E G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadJPEGImage reads a JPEG image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadJPEGImage method is:
%
%      Image *ReadJPEGImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadJPEGImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o filename:  Specifies the name of the jpeg image to read.
%
%
*/

static void EmitMessage(j_common_ptr jpeg_info,int level)
{
  char
    message[JMSG_LENGTH_MAX];

  (jpeg_info->err->format_message)(jpeg_info,message);
  if (level < 0)
    {
      if ((jpeg_info->err->num_warnings == 0) ||
          (jpeg_info->err->trace_level >= 3))
        MagickWarning(DelegateWarning,(char *) message,image->filename);
      jpeg_info->err->num_warnings++;
    }
  else
    if (jpeg_info->err->trace_level >= level)
      MagickWarning(DelegateWarning,(char *) message,image->filename);
}

static boolean FillInputBuffer(j_decompress_ptr cinfo)
{
  SourceManager
    *source;

  source=(SourceManager *) cinfo->src;
  source->manager.bytes_in_buffer=
    ReadBlob(source->image,MaxBufferExtent,(char *) source->buffer);
  if (source->manager.bytes_in_buffer == 0)
    {
      if (source->start_of_blob)
        ERREXIT(cinfo,JERR_INPUT_EMPTY);
      WARNMS(cinfo,JWRN_JPEG_EOF);
      source->buffer[0]=(JOCTET) 0xff;
      source->buffer[1]=(JOCTET) JPEG_EOI;
      source->manager.bytes_in_buffer=2;
    }
  source->manager.next_input_byte=source->buffer;
  source->start_of_blob=FALSE;
  return(TRUE);
}

static unsigned int GetCharacter(j_decompress_ptr jpeg_info)
{
  if (jpeg_info->src->bytes_in_buffer == 0)
    (*jpeg_info->src->fill_input_buffer)(jpeg_info);
  jpeg_info->src->bytes_in_buffer--;
  return(GETJOCTET(*jpeg_info->src->next_input_byte++));
}

static void InitializeSource(j_decompress_ptr cinfo)
{
  SourceManager
    *source;

  source=(SourceManager *) cinfo->src;
  source->start_of_blob=TRUE;
}

static void JPEGErrorHandler(j_common_ptr jpeg_info)
{
  EmitMessage(jpeg_info,0);
  longjmp(error_recovery,1);
}

static boolean ReadColorProfile(j_decompress_ptr jpeg_info)
{
  char
    magick[12];

  long int
    length;

  register unsigned char
    *p;

  register int
    i;

  /*
    Determine length of color profile.
  */
  length=GetCharacter(jpeg_info) << 8;
  length+=GetCharacter(jpeg_info);
  length-=2;
  for (i=0; i < 12; i++)
    magick[i]=GetCharacter(jpeg_info);
  if (Latin1Compare(magick,"ICC_PROFILE") != 0)
    {
      /*
        Not a ICC profile, return.
      */
      for (i=0; i < length-12; i++)
        (void) GetCharacter(jpeg_info);
      return(True);
    }
  (void) GetCharacter(jpeg_info);  /* id */
  (void) GetCharacter(jpeg_info);  /* markers */
  length-=14;
  if (image->color_profile.length == 0)
    image->color_profile.info=(unsigned char *)
      AllocateMemory((unsigned int) length*sizeof(unsigned char));
  else
    image->color_profile.info=(unsigned char *) ReallocateMemory((char *)
      image->color_profile.info,(image->color_profile.length+length)*
      sizeof(unsigned char));
  if (image->color_profile.info == (unsigned char *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Memory allocation failed",
        (char *) NULL);
      return(False);
    }
  /*
    Read color profile.
  */
  p=image->color_profile.info+image->color_profile.length;
  image->color_profile.length+=length;
  while (--length >= 0)
    *p++=GetCharacter(jpeg_info);
  return(True);
}

static boolean ReadComment(j_decompress_ptr jpeg_info)
{
  long int
    length;

  register char
    *p;

  /*
    Determine length of comment.
  */
  length=GetCharacter(jpeg_info) << 8;
  length+=GetCharacter(jpeg_info);
  length-=2;
  if (image->comments != (char *) NULL)
    image->comments=(char *) ReallocateMemory((char *) image->comments,
      (unsigned int) (Extent(image->comments)+length+1)*sizeof(char));
  else
    {
      image->comments=(char *)
        AllocateMemory((unsigned int) (length+1)*sizeof(char));
      if (image->comments != (char *) NULL)
        *image->comments='\0';
    }
  if (image->comments == (char *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Memory allocation failed",
        (char *) NULL);
      return(False);
    }
  /*
    Read comment.
  */
  p=image->comments+Extent(image->comments);
  while (--length >= 0)
    *p++=GetCharacter(jpeg_info);
  *p='\0';
  return(True);
}

static boolean ReadNewsProfile(j_decompress_ptr jpeg_info)
{
  long int
    taglen,
    length;

  register unsigned char
    *p;

  register int
    i;

#ifdef GET_ONLY_IPTC_DATA
  unsigned char
    tag[2];
#else
  char
    magick[11];
#endif

  /*
    Determine length of binary data stored here.
  */
  length=GetCharacter(jpeg_info) << 8;
  length+=GetCharacter(jpeg_info);
  length-=2;
#ifdef GET_ONLY_IPTC_DATA
  /*
    The following tries to find the beginning of
    the IPTC portion of the binary data
   */
  for (*tag='\0'; length > 0; )
  {
    *tag=GetCharacter(jpeg_info);
    *(tag+1)=GetCharacter(jpeg_info);
    length-=2;
    if ((*tag == 0x1c) && (*(tag+1) == 0x02))
      break;
  }
  taglen=2;
#else
  /*
    The following validates that this was written
    as a Photoshop resource format slug
   */
  for (i=0; i < 10; i++)
    magick[i]=GetCharacter(jpeg_info);
  magick[10]='\0';
  length-=10;
  if (Latin1Compare(magick,"Photoshop ") != 0)
    {
      /*
        Not a ICC profile, return.
      */
      for (i=0; i < length; i++)
        (void) GetCharacter(jpeg_info);
      return(True);
    }
  /*
    Remove the version number.
  */
  for (i=0; i < 4; i++)
    (void) GetCharacter(jpeg_info);
  length-=4;
  taglen=0;
#endif
  if (length <= 0)
    return(True);
  if (image->iptc_profile.length != 0)
    image->iptc_profile.info=(unsigned char *) ReallocateMemory((char *)
      image->iptc_profile.info,
        (unsigned int) (length+taglen)*sizeof(unsigned char));
  else
    {
      image->iptc_profile.info=(unsigned char *)
        AllocateMemory((unsigned int) (length+taglen)*sizeof(unsigned char));
      if (image->iptc_profile.info != (unsigned char *) NULL)
        image->iptc_profile.length=0;
    }
  if (image->iptc_profile.info == (unsigned char *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Memory allocation failed",
        (char *) NULL);
      return(False);
    }
  /*
    Read the payload of this binary data, decoding
    is left as a excercise for the application.
   */
  p=image->iptc_profile.info;
  image->iptc_profile.length=length+taglen;
#ifdef GET_ONLY_IPTC_DATA
  *p++=0x1c;
  *p++=0x02;
#endif
  while (--length >= 0)
    *p++=GetCharacter(jpeg_info);
  return(True);
}

static void SkipInputData(j_decompress_ptr cinfo,long number_bytes)
{
  SourceManager
    *source;

  if (number_bytes <= 0)
    return;
  source=(SourceManager *) cinfo->src;
  while (number_bytes > (long) source->manager.bytes_in_buffer)
  {
    number_bytes-=(long) source->manager.bytes_in_buffer;
    (void) FillInputBuffer(cinfo);
  }
  source->manager.next_input_byte+=(size_t) number_bytes;
  source->manager.bytes_in_buffer-=(size_t) number_bytes;
}

static void TerminateSource(j_decompress_ptr cinfo)
{
}

static void JPEGSourceManager(j_decompress_ptr cinfo,Image *image)
{
  SourceManager
    *source;

  if (cinfo->src == (JOCTET) NULL)
    {
      cinfo->src=(struct jpeg_source_mgr *) (*cinfo->mem->alloc_small)
        ((j_common_ptr) cinfo,JPOOL_PERMANENT,sizeof(SourceManager));
      source=(SourceManager *) cinfo->src;
      source->buffer=(JOCTET *) (*cinfo->mem->alloc_small)
        ((j_common_ptr) cinfo,JPOOL_PERMANENT,MaxBufferExtent*sizeof(JOCTET));
    }
  source=(SourceManager *) cinfo->src;
  source->manager.init_source=InitializeSource;
  source->manager.fill_input_buffer=FillInputBuffer;
  source->manager.skip_input_data=SkipInputData;
  source->manager.resync_to_restart=jpeg_resync_to_restart;
  source->manager.term_source=TerminateSource;
  source->manager.bytes_in_buffer=0;
  source->manager.next_input_byte=NULL;
  source->image=image;
}

Export Image *ReadJPEGImage(const ImageInfo *image_info)
{
  IndexPacket
    index;

  int
    x,
    y;

  JSAMPLE
    *jpeg_pixels;

  JSAMPROW
    scanline[1];

  register int
    i;

  struct jpeg_decompress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

  register JSAMPLE
    *p;

  register PixelPacket
    *q;

  unsigned int
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
    Initialize image structure.
  */
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=EmitMessage;
  jpeg_info.err->error_exit=JPEGErrorHandler;
  jpeg_pixels=(JSAMPLE *) NULL;
  if (setjmp(error_recovery))
    {
      /*
        JPEG image is corrupt.
      */
      if (jpeg_pixels != (JSAMPLE *) NULL)
        FreeMemory(jpeg_pixels);
      jpeg_destroy_decompress(&jpeg_info);
      DestroyImage(image);
      return((Image *) NULL);
    }
  jpeg_create_decompress(&jpeg_info);
  JPEGSourceManager(&jpeg_info,image);
  jpeg_set_marker_processor(&jpeg_info,JPEG_COM,ReadComment);
  jpeg_set_marker_processor(&jpeg_info,ICC_MARKER,ReadColorProfile);
  jpeg_set_marker_processor(&jpeg_info,IPTC_MARKER,ReadNewsProfile);
  (void) jpeg_read_header(&jpeg_info,True);
  if (jpeg_info.out_color_space == JCS_CMYK)
    image->colorspace=CMYKColorspace;
  if (jpeg_info.saw_JFIF_marker)
    {
      if ((jpeg_info.density_unit != 0) || (jpeg_info.X_density != 1.0) ||
          (jpeg_info.Y_density != 1.0))
        {
          /*
            Set image resolution.
          */
          image->x_resolution=jpeg_info.X_density;
          image->y_resolution=jpeg_info.Y_density;
          if (jpeg_info.density_unit == 1)
            image->units=PixelsPerInchResolution;
          if (jpeg_info.density_unit == 2)
            image->units=PixelsPerCentimeterResolution;
        }
    }
  if ((image->columns*image->rows) != 0)
    {
      double
        scale_factor;

      /*
        Let the JPEG library subsample for us.
      */
      jpeg_calc_output_dimensions(&jpeg_info);
      image->magick_columns=jpeg_info.output_width;
      image->magick_rows=jpeg_info.output_height;
      scale_factor=(double) jpeg_info.output_width/image->columns;
      if (scale_factor > ((double) jpeg_info.output_height/image->rows))
        scale_factor=(double) jpeg_info.output_height/image->rows;
      jpeg_info.scale_denom=scale_factor;
      jpeg_calc_output_dimensions(&jpeg_info);
    }
  if (image_info->subrange != 0)
    {
      jpeg_info.scale_denom=image_info->subrange;
      jpeg_calc_output_dimensions(&jpeg_info);
    }
#if (JPEG_LIB_VERSION >= 61) && !defined(D_LOSSLESS_SUPPORTED)
  image->interlace=jpeg_info.progressive_mode ? PlaneInterlace : NoInterlace;
#endif
  jpeg_start_decompress(&jpeg_info);
  image->compression=JPEGCompression;
  image->columns=jpeg_info.output_width;
  image->rows=jpeg_info.output_height;
  image->depth=jpeg_info.data_precision <= 8 ? 8 : QuantumDepth;
  if (image_info->ping)
    {
      jpeg_destroy_decompress(&jpeg_info);
      CloseBlob(image);
      return(image);
    }
  if (jpeg_info.out_color_space == JCS_GRAYSCALE)
    {
      /*
        Initialize grayscale colormap.
      */
      image->class=PseudoClass;
      image->colors=1 << jpeg_info.data_precision;
      image->colormap=(PixelPacket *)
        AllocateMemory(image->colors*sizeof(PixelPacket));
      if (image->colormap == (PixelPacket *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (i=0; i < (int) image->colors; i++)
      {
        image->colormap[i].red=
          ((unsigned long) (MaxRGB*i)/(image->colors-1));
        image->colormap[i].green=
          ((unsigned long) (MaxRGB*i)/(image->colors-1));
        image->colormap[i].blue=
          ((unsigned long) (MaxRGB*i)/(image->colors-1));
      }
    }
  jpeg_pixels=(JSAMPLE *)
    AllocateMemory(jpeg_info.output_components*image->columns*sizeof(JSAMPLE));
  if (jpeg_pixels == (JSAMPLE *) NULL)
    ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
  /*
    Convert JPEG pixels to pixel packets.
  */
  scanline[0]=(JSAMPROW) jpeg_pixels;
  for (y=0; y < (int) image->rows; y++)
  {
    p=jpeg_pixels;
    q=SetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    (void) jpeg_read_scanlines(&jpeg_info,scanline,1);
    for (x=0; x < (int) image->columns; x++)
    {
      if (jpeg_info.data_precision > QuantumDepth)
        {
          if (jpeg_info.out_color_space == JCS_GRAYSCALE)
            {
              index=GETJSAMPLE(*p++) >> 4;
              image->indexes[x]=index;
              *q=image->colormap[index];
            }
          else
            {
              q->red=(Quantum) (GETJSAMPLE(*p++) >> 4);
              q->green=(Quantum) (GETJSAMPLE(*p++) >> 4);
              q->blue=(Quantum) (GETJSAMPLE(*p++) >> 4);
              if (image->colorspace == CMYKColorspace)
                q->opacity=(Quantum) (GETJSAMPLE(*p++) >> 4);
            }
        }
      else
        if (jpeg_info.out_color_space == JCS_GRAYSCALE)
          {
            index=GETJSAMPLE(*p++);
            image->indexes[x]=index;
            *q=image->colormap[index];
          }
        else
          {
            q->red=(Quantum) UpScale(GETJSAMPLE(*p++));
            q->green=(Quantum) UpScale(GETJSAMPLE(*p++));
            q->blue=(Quantum) UpScale(GETJSAMPLE(*p++));
            if (image->colorspace == CMYKColorspace)
              q->opacity=(Quantum) UpScale(GETJSAMPLE(*p++));
          }
      q++;
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(LoadImageText,y,image->rows);
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
  /*
    Free jpeg resources.
  */
  (void) jpeg_finish_decompress(&jpeg_info);
  jpeg_destroy_decompress(&jpeg_info);
  FreeMemory(jpeg_pixels);
  CloseBlob(image);
  return(image);
}
#else
Export Image *ReadJPEGImage(const ImageInfo *image_info)
{
  MagickWarning(MissingDelegateWarning,"JPEG library is not available",
    image_info->filename);
  return((Image *) NULL);
}
#endif

#if defined(HasJPEG)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  W r i t e J P E G I m a g e                                                %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteJPEGImage writes a JPEG image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the WriteJPEGImage method is:
%
%      unsigned int WriteJPEGImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows:
%
%    o status:  Method WriteJPEGImage return True if the image is written.
%      False is returned is there is of a memory shortage or if the image
%      file cannot be opened for writing.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o jpeg_image:  A pointer to a Image structure.
%
%
*/

static boolean EmptyOutputBuffer(j_compress_ptr cinfo)
{
  DestinationManager
    *destination;

  destination=(DestinationManager *) cinfo->dest;
  destination->manager.free_in_buffer=WriteBlob(destination->image,
    MaxBufferExtent,(char *) destination->buffer);
  if (destination->manager.free_in_buffer != MaxBufferExtent)
    ERREXIT(cinfo,JERR_FILE_WRITE);
  destination->manager.next_output_byte=destination->buffer;
  return(TRUE);
}

static void InitializeDestination(j_compress_ptr cinfo)
{
  DestinationManager
    *destination;

  destination=(DestinationManager *) cinfo->dest;
  destination->buffer=(JOCTET *) (*cinfo->mem->alloc_small)
    ((j_common_ptr) cinfo,JPOOL_IMAGE,MaxBufferExtent*sizeof(JOCTET));
  destination->manager.next_output_byte=destination->buffer;
  destination->manager.free_in_buffer=MaxBufferExtent;
}

static void JPEGWarningHandler(j_common_ptr jpeg_info,int level)
{
  char
    message[JMSG_LENGTH_MAX];

  (jpeg_info->err->format_message)(jpeg_info,message);
  if (level < 0)
    {
      if ((jpeg_info->err->num_warnings == 0) ||
          (jpeg_info->err->trace_level >= 3))
        MagickWarning(DelegateWarning,(char *) message,(char *) NULL);
      jpeg_info->err->num_warnings++;
    }
  else
    if (jpeg_info->err->trace_level >= level)
      MagickWarning(DelegateWarning,(char *) message,(char *) NULL);
}

static void TerminateDestination(j_compress_ptr cinfo)
{
  DestinationManager
    *destination;

  destination=(DestinationManager *) cinfo->dest;
  if ((MaxBufferExtent-destination->manager.free_in_buffer) > 0)
    {
      unsigned long
        number_bytes;

      number_bytes=WriteBlob(destination->image,MaxBufferExtent-
        destination->manager.free_in_buffer,(char *) destination->buffer);
      if (number_bytes != (MaxBufferExtent-destination->manager.free_in_buffer))
        ERREXIT(cinfo,JERR_FILE_WRITE);
    }
  if (SyncBlob(destination->image))
    ERREXIT(cinfo,JERR_FILE_WRITE);
}

static void WriteColorProfile(j_compress_ptr jpeg_info,Image *image)
{
  register int
    i,
    j;

  unsigned char
    *profile;

  unsigned int
    length;

  /*
    Save color profile as a APP marker.
  */
  for (i=0; i < (int) image->color_profile.length; i+=65519)
  {
    length=Min(image->color_profile.length-i,65519);
    profile=(unsigned char *) AllocateMemory((length+14)*sizeof(unsigned char));
    if (profile == (unsigned char *) NULL)
      break;
    (void) strcpy((char *) profile,"ICC_PROFILE");
    profile[12]=(i/65519)+1;
    profile[13]=(image->color_profile.length/65519)+1;
    for (j=0; j < (int) length; j++)
      profile[j+14]=image->color_profile.info[i+j];
    jpeg_write_marker(jpeg_info,ICC_MARKER,profile,(unsigned int) length+14);
    FreeMemory(profile);
  }
}

static void WriteNewsProfile(j_compress_ptr jpeg_info,Image *image)
{
  int
    roundup;

  register int
    i;

  unsigned char
    *profile;

  unsigned int
    taglen,
    length;

  /*
    Save binary Photoshop resource data using an APP marker.
  */
#ifdef GET_ONLY_IPTC_DATA
  taglen=26;
#else
  taglen=14;
#endif
  for (i=0; i < (int) image->iptc_profile.length; i+=65500)
  {
    length=Min(image->iptc_profile.length-i,65500);
    roundup=(length & 0x01); /* round up for Photoshop */
    profile=(unsigned char *)
      AllocateMemory((length+roundup+taglen)*sizeof(unsigned char));
    if (profile == (unsigned char *) NULL)
      break;
#ifdef GET_ONLY_IPTC_DATA
    (void) memcpy((char *) profile,"Photoshop 3.0 8BIM\04\04\0\0\0\0",24);
    profile[13]=0x00;
    profile[24]=length >> 8;
    profile[25]=length & 0xff;
#else
    (void) memcpy((char *) profile,"Photoshop 3.0 ",14);
    profile[13]=0x00;
#endif
    (void) memcpy((char *) &(profile[taglen]),&(image->iptc_profile.info[i]),
      length);
    if (roundup)
      profile[length+taglen]=0;
    jpeg_write_marker(jpeg_info,IPTC_MARKER,profile,(unsigned int)
      length+roundup+taglen);
    FreeMemory(profile);
  }
}

static void JPEGDestinationManager(j_compress_ptr cinfo,Image * image)
{
  DestinationManager
    *destination;

  if (cinfo->dest == (JOCTET) NULL)
    cinfo->dest=(struct jpeg_destination_mgr *) (*cinfo->mem->alloc_small)
      ((j_common_ptr) cinfo,JPOOL_PERMANENT,sizeof(DestinationManager));
  destination=(DestinationManager *) cinfo->dest;
  destination->manager.init_destination=InitializeDestination;
  destination->manager.empty_output_buffer=EmptyOutputBuffer;
  destination->manager.term_destination=TerminateDestination;
  destination->image=image;
}

Export unsigned int WriteJPEGImage(const ImageInfo *image_info,Image *image)
{
  int
    flags,
    sans_offset,
    y;

  JSAMPLE
    *jpeg_pixels;

  JSAMPROW
    scanline[1];

  register int
    i,
    x;

  register JSAMPLE
    *q;

  register PixelPacket
    *p;

  struct jpeg_compress_struct
    jpeg_info;

  struct jpeg_error_mgr
    jpeg_error;

  unsigned int
    status,
    x_resolution,
    y_resolution;

  /*
    Open image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  /*
    Initialize JPEG parameters.
  */
  jpeg_info.err=jpeg_std_error(&jpeg_error);
  jpeg_info.err->emit_message=JPEGWarningHandler;
  jpeg_create_compress(&jpeg_info);
  JPEGDestinationManager(&jpeg_info,image);
  jpeg_info.image_width=image->columns;
  jpeg_info.image_height=image->rows;
  jpeg_info.input_components=3;
  jpeg_info.data_precision=image->depth <= 8 ? 8 : 12;
  jpeg_info.in_color_space=JCS_RGB;
  if (Latin1Compare(image_info->magick,"JPEG24") != 0)
    if (IsGrayImage(image))
      {
        jpeg_info.input_components=1;
        jpeg_info.in_color_space=JCS_GRAYSCALE;
      }
  if (((image_info->colorspace != UndefinedColorspace) ||
       (image->colorspace != CMYKColorspace)) &&
       (image_info->colorspace != CMYKColorspace))
    TransformRGBImage(image,RGBColorspace);
  else
    {
      jpeg_info.input_components=4;
      jpeg_info.in_color_space=JCS_CMYK;
      if (image->colorspace != CMYKColorspace)
        RGBTransformImage(image,CMYKColorspace);
    }
  jpeg_set_defaults(&jpeg_info);
  flags=NoValue;
  x_resolution=72;
  y_resolution=72;
  if (image_info->density != (char *) NULL)
    flags=ParseGeometry(image_info->density,&sans_offset,&sans_offset,
      &x_resolution,&y_resolution);
  if (flags & WidthValue)
    image->x_resolution=x_resolution;
  if (flags & HeightValue)
    image->y_resolution=y_resolution;
  jpeg_info.density_unit=1;  /* default to DPI */
  if ((image->x_resolution != 0) && (image->y_resolution != 0))
    {
      /*
        Set image resolution.
      */
      jpeg_info.X_density=(short) image->x_resolution;
      jpeg_info.Y_density=(short) image->y_resolution;
      if (image->units == PixelsPerInchResolution)
        jpeg_info.density_unit=1;
      if (image->units == PixelsPerCentimeterResolution)
        jpeg_info.density_unit=2;
    }
  /*
     Allow user to ask for no color subsampling
  */
  if (image->compression == NoCompression)
    for (i=0; i < MAX_COMPONENTS; i++)
    {
      jpeg_info.comp_info[i].h_samp_factor=1;
      jpeg_info.comp_info[i].v_samp_factor=1;
    }
  jpeg_set_quality(&jpeg_info,image_info->quality,True);
  jpeg_info.optimize_coding=True;
#if (JPEG_LIB_VERSION >= 61) && !defined(C_LOSSLESS_SUPPORTED)
  if (image_info->interlace != NoInterlace)
    jpeg_simple_progression(&jpeg_info);
#endif
  jpeg_start_compress(&jpeg_info,True);
  if (image->comments != (char *) NULL)
    for (i=0; i < Extent(image->comments); i+=65533)
      jpeg_write_marker(&jpeg_info,JPEG_COM,(unsigned char *) image->comments+i,
        (unsigned int) Min(Extent(image->comments+i),65533));
  if (image->color_profile.length > 0)
    WriteColorProfile(&jpeg_info,image);
  if (image->iptc_profile.length > 0)
    WriteNewsProfile(&jpeg_info,image);
  /*
    Convert MIFF to JPEG raster pixels.
  */
  jpeg_pixels=(JSAMPLE *)
    AllocateMemory(jpeg_info.input_components*image->columns*sizeof(JSAMPLE));
  if (jpeg_pixels == (JSAMPLE *) NULL)
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  scanline[0]=(JSAMPROW) jpeg_pixels;
  if (jpeg_info.data_precision > 8)
    {
      if (jpeg_info.in_color_space == JCS_GRAYSCALE)
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          for (x=0; x < (int) image->columns; x++)
          {
            *q++=(JSAMPLE) (Intensity(*p) >> 4);
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          if (QuantumTick(y,image->rows))
            ProgressMonitor(SaveImageText,y,image->rows);
        }
      else
        if (jpeg_info.in_color_space == JCS_RGB)
          for (y=0; y < (int) image->rows; y++)
          {
            p=GetPixelCache(image,0,y,image->columns,1);
            if (p == (PixelPacket *) NULL)
              break;
            q=jpeg_pixels;
            for (x=0; x < (int) image->columns; x++)
            {
              *q++=(JSAMPLE) (p->red >> 4);
              *q++=(JSAMPLE) (p->green >> 4);
              *q++=(JSAMPLE) (p->blue >> 4);
              p++;
            }
            (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
          }
        else
          for (y=0; y < (int) image->rows; y++)
          {
            p=GetPixelCache(image,0,y,image->columns,1);
            if (p == (PixelPacket *) NULL)
              break;
            q=jpeg_pixels;
            for (x=0; x < (int) image->columns; x++)
            {
              /*
                Convert DirectClass packets to contiguous CMYK scanlines.
              */
              *q++=(JSAMPLE) (MaxRGB-(p->red >> 4));
              *q++=(JSAMPLE) (MaxRGB-(p->green >> 4));
              *q++=(JSAMPLE) (MaxRGB-(p->blue >> 4));
              *q++=(JSAMPLE) (MaxRGB-(p->opacity >> 4));
              p++;
            }
            (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
          }
    }
  else
    if (jpeg_info.in_color_space == JCS_GRAYSCALE)
      for (y=0; y < (int) image->rows; y++)
      {
        p=GetPixelCache(image,0,y,image->columns,1);
        if (p == (PixelPacket *) NULL)
          break;
        q=jpeg_pixels;
        for (x=0; x < (int) image->columns; x++)
        {
          *q++=(JSAMPLE) DownScale(Intensity(*p));
          p++;
        }
        (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
        if (QuantumTick(y,image->rows))
          ProgressMonitor(SaveImageText,y,image->rows);
      }
    else
      if (jpeg_info.in_color_space == JCS_RGB)
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          for (x=0; x < (int) image->columns; x++)
          {
            *q++=(JSAMPLE) DownScale(p->red);
            *q++=(JSAMPLE) DownScale(p->green);
            *q++=(JSAMPLE) DownScale(p->blue);
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          if (QuantumTick(y,image->rows))
            ProgressMonitor(SaveImageText,y,image->rows);
        }
      else
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          q=jpeg_pixels;
          for (x=0; x < (int) image->columns; x++)
          {
            /*
              Convert DirectClass packets to contiguous CMYK scanlines.
            */
            *q++=(JSAMPLE) DownScale(MaxRGB-p->red);
            *q++=(JSAMPLE) DownScale(MaxRGB-p->green);
            *q++=(JSAMPLE) DownScale(MaxRGB-p->blue);
            *q++=(JSAMPLE) DownScale(MaxRGB-p->opacity);
            p++;
          }
          (void) jpeg_write_scanlines(&jpeg_info,scanline,1);
          if (QuantumTick(y,image->rows))
            ProgressMonitor(SaveImageText,y,image->rows);
        }
  jpeg_finish_compress(&jpeg_info);
  /*
    Free memory.
  */
  jpeg_destroy_compress(&jpeg_info);
  FreeMemory(jpeg_pixels);
  CloseBlob(image);
  return(True);
}
#else
Export unsigned int WriteJPEGImage(const ImageInfo *image_info,Image *image)
{
  MagickWarning(MissingDelegateWarning,"JPEG library is not available",
    image->filename);
  return(False);
}
#endif

