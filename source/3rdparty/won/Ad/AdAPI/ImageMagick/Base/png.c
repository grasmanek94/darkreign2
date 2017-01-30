/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            PPPP   N   N   GGGG                              %
%                            P   P  NN  N  G                                  %
%                            PPPP   N N N  G  GG                              %
%                            P      N  NN  G   G                              %
%                            P      N   N   GGG                               %
%                                                                             %
%                                                                             %
%                    Read/Write ImageMagick Image Format.                     %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                               November 1997                                 %
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
#if defined(HasPNG)
#include "png.h"
#include "zlib.h"
#include "mng.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o m p r e s s C o l o r m a p T r a n s F i r s t                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method CompressColormapTransFirst compresses an image colormap removing
%  any duplicate and unused color entries and putting the transparent colors
%  first.
%
%  The format of the CompressColormapTransFirst method is:
%
%      void CompressColormapTransFirst(Image *image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%
*/
Export void CompressColormapTransFirst(Image *image)
{
  int
    number_colors,
    y;

  PixelPacket
    *colormap;

  register IndexPacket
    index;

  register int
    i,
    j,
    x;

  unsigned char
    have_transparency,
    *marker;

  unsigned short
    *link,
    *opacity;

  /*
    Determine if colormap can be compressed.
  */
  assert(image != (Image *) NULL);
  if (image->class != PseudoClass)
    return;
  marker=(unsigned char *) AllocateMemory(image->colors*sizeof(unsigned char));
  if (marker == (unsigned char *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to compress colormap",
        "Memory allocation failed");
      return;
    }
  opacity=(unsigned short *) AllocateMemory(image->colors*sizeof(unsigned
      short));
  if (opacity == (unsigned short *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to compress colormap",
        "Memory allocation failed");
      FreeMemory(marker);
      return;
    }
  /*
    Mark colors that are present.
  */
  number_colors=image->colors;
  for (i=0; i < number_colors; i++)
    {
      marker[i]=False;
      opacity[i]=Opaque;
    }
  for (y=0; y < (int) image->rows; y++)
  {
    register PixelPacket
      *p;
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
      {
         marker[image->indexes[x]]=True;
         opacity[image->indexes[x]]=p->opacity;
         p++;
      }
  }
  /*
    Unmark duplicates.
  */
  for (i=0; i < number_colors-1; i++)
     if (marker[i])
       {
        for (j=i+1; j<number_colors; j++)
           if (marker[j] && (opacity[i]==opacity[j]) &&
               (ColorMatch(image->colormap[i],image->colormap[j],0)))
             {
               marker[j]=False;
             }
       }
  /*
    Count colors that still remain.
   */
  image->colors=0;
  have_transparency=False;
  for (i=0; i < number_colors; i++)
    if (marker[i])
      {
        image->colors++;
        if (opacity[i] != Opaque)
           have_transparency=True;
      }
  if ((!have_transparency || opacity[0] == Transparent) &&
     ((int) image->colors == number_colors))
    {
      /* no duplicate or unused entries, and transparency-swap not needed */
      FreeMemory(marker);
      FreeMemory(opacity);
      return;
    }
  /*
    Compress colormap.
  */
  colormap=(PixelPacket *) AllocateMemory(image->colors*sizeof(PixelPacket));
  if (colormap == (PixelPacket *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to compress colormap",
        "Memory allocation failed");
      FreeMemory(marker);
      FreeMemory(opacity);
      image->colors=number_colors;
      return;
    }
  /*
    Eliminate unused colormap entries.
  */
  link=(unsigned short *) AllocateMemory(number_colors*sizeof(unsigned
      short));
  if (link == (unsigned short *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to compress colormap",
        "Memory allocation failed");
      FreeMemory(marker);
      FreeMemory(opacity);
      FreeMemory(colormap);
      image->colors=number_colors;
      return;
    }
  index=0;
  for (i=0; i < number_colors; i++)
    {
      if (marker[i])
        {
          link[i]=index;
          index++;
        }
    }

  for (i=0; i < number_colors; i++)
  {
     if (marker[i])
       {
         for (j=i+1; j<image->colors; j++)
         {
           if ((opacity[i]==opacity[j]) &&
               (ColorMatch(image->colormap[i],image->colormap[j],0)))
             {
               link[j]=link[i];
             }
         }
       }
  }

  index=0;
  for (i=0; i < number_colors; i++)
    {
      if (marker[i])
        {
          colormap[index]=image->colormap[i];
          opacity[index]=opacity[i];
          index++;
        }
    }
   
  FreeMemory(marker);
  if (have_transparency && opacity[0] != Transparent)
    {
      /* Move the first transparent color to palette entry 0 */
      for (i=1; i < image->colors; i++)
      {
        if (opacity[i] == Transparent)
          {
            PixelPacket
               temp_colormap;

            temp_colormap=colormap[0];
            colormap[0]=colormap[i];
            colormap[i]=temp_colormap;

            for (j=0; j < number_colors; j++)
            {
              if (link[j] == 0)
                 link[j]=i;
              else if (link[j] == i)
                 link[j]=0;
            }
            break;
          }
      }
   }
  FreeMemory(opacity);
  /*
    Remap pixels.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    if (!GetPixelCache(image,0,y,image->columns,1))
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      index=image->indexes[x];
      assert (index < number_colors);
      assert (link[index] < image->colors);
      image->indexes[x]=link[index];
    }
    if (!SyncPixelCache(image))
      break;
  }
  FreeMemory(link);
  FreeMemory(image->colormap);
  image->colormap=colormap;
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M N G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsMNG returns True if the image format type, identified by the
%  magick string, is MNG.
%
%  The format of the ReadMNGImage method is:
%
%      unsigned int IsMNG(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsMNG returns True if the image format type is MNG.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsMNG(const unsigned char *magick,const unsigned int length)
{
  if (length < 8)
    return(False);
  if (strncmp((char *) magick,"\212MNG\r\n\032\n",8) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s P N G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsPNG returns True if the image format type, identified by the
%  magick string, is PNG.
%
%  The format of the IsPNG method is:
%
%      unsigned int IsPNG(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsPNG returns True if the image format type is PNG.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsPNG(const unsigned char *magick,const unsigned int length)
{
  if (length < 8)
    return(False);
  if (strncmp((char *) magick,"\211PNG\r\n\032\n",8) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%   S e t T r a n s p a r e n t I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SetTransparentImage initializes the reference image to the background
%  color and the matte channel to transparent.
%
%  The format of the SetTransparentImage method is:
%
%      void SetTransparentImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export void SetTransparentImage(Image *image)
{
  int
    y;

  PixelPacket
    background_color;

  register int
    x;

  register PixelPacket
    *q;

  assert(image != (Image *) NULL);
  background_color=image->background_color;
  background_color.opacity=Transparent;
  for (y=0; y < (int) image->rows; y++)
  {
    q=SetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      if (image->class == PseudoClass)
        image->indexes[x]=0;
      *q++=background_color;
    }
    if (!SyncPixelCache(image))
      break;
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d P N G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadPNGImage reads a Portable Network Graphics (PNG) or
%  Multiple-image Network Graphics (MNG) image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image or set of images.
%
%  MNG support written by Glenn Randers-Pehrson, randeg@alum.rpi.edu.
%
%  The format of the ReadPNGImage method is:
%
%      Image *ReadPNGImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadPNGImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%  To do, more or less in chronological order (as of version 5.1.1,
%   January 25, 2000 -- glennrp -- see also "To do" under WritePNGImage):
%
%    Restore features that were lost in the 4.2.9-to-5.1.0 update:
%
%       o preserve transparency when reading gray-alpha PNGs
%       o variable interframe durations (restart_animation_here feature
%         was broken)
%       o coalescing frames that have zero interframe duration
%
%    (At this point, PNG decoding is supposed to be in full MNG-LC compliance)
%
%    Preserve all unknown and not-yet-handled known chunks found in input
%    PNG file and copy them into output PNG files according to the PNG
%    copying rules.
%
%    (At this point, PNG encoding should be in full MNG compliance)
%
%    Provide options for choice of background to use when the MNG BACK
%    chunk is not present or is not mandatory (i.e., leave transparent,
%    user specified, MNG BACK, PNG bKGD)
%
%    Implement LOOP/ENDL [done, but could do discretionary loops more
%    efficiently by linking in the duplicate frames.].
%
%    Decode JNG datastreams.
%
%    Decode and act on the MHDR simplicity profile (offer option to reject
%    files or attempt to process them anyway when the profile isn't LC or VLC).
%
%    Upgrade to full MNG without Delta-PNG.
%
%        o  BACK [done a while ago except for background image ID]
%        o  MOVE [done 15 May 1999]
%        o  CLIP [done 15 May 1999]
%        o  DISC [done 19 May 1999]
%        o  SAVE [partially done 19 May 1999 (marks objects frozen)]
%        o  SEEK [partially done 19 May 1999 (discard function only)]
%        o  SHOW
%        o  PAST
%        o  BASI
%        o  MNG-level tEXt/zTXt
%        o  pHYg
%        o  pHYs
%        o  sBIT
%        o  bKGD
%        o  iCCP (wait for libpng implementation).
%        o  iTXt (wait for libpng implementation).
%
%    Use the scene signature to discover when an identical scene is
%    being reused, and just point to the original image->pixels instead
%    of storing another set of pixels.  This is not specific to MNG
%    but could be applied generally.
%
%    Upgrade to full MNG with Delta-PNG.
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(PNG_NO_STDIO)
/*
  This is the function that does the actual reading of data.  It is
  the same as the one supplied in libpng, except that it receives the
  datastream from the ReadBlob() function instead of standard input.
*/
static void png_get_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  Image
    *image;

  image=(Image *) png_get_io_ptr(png_ptr);
  if (length)
    {
      png_size_t
        check;

      check=(png_size_t) ReadBlob(image,(size_t) length,(char *) data);
      if (check != length)
        png_error(png_ptr,"Read Error");
    }
}

#ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
/* We use mng_get_data() instead of png_get_data() if we have a libpng
 * older than libpng-1.0.3a, which was the first to allow the empty
 * PLTE, or a newer libpng in which PNG_READ_EMPTY_PLTE_SUPPORTED was
 * ifdef'ed out.  Earlier versions would crash if the bKGD chunk was
 * encountered after an empty PLTE, so we have to look ahead for bKGD
 * chunks and remove them from the datastream that is passed to libpng,
 * and store their contents for later use.
 */
static void mng_get_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  MngInfo
    *mng_info;

  Image
    *image;

  png_size_t
    check;

  register int
    i;

  i=0;
  mng_info=(MngInfo *) png_get_io_ptr(png_ptr);
  image=(Image *) mng_info->image;
  while (mng_info->bytes_in_read_buffer && length)
  {
    data[i]=mng_info->read_buffer[i];
    mng_info->bytes_in_read_buffer--;
    length--;
    i++;
  }
  if (length)
    {
      check=(png_size_t) ReadBlob(image,(size_t) length,(char *) data);
      if (check != length)
        png_error(png_ptr,"Read Error");
      if (length == 4)
        {
          if ((data[0] == 0) && (data[1] == 0) && (data[2] == 0) &&
              (data[3] == 0))
            {
              check=(png_size_t) ReadBlob(image,(size_t) length,
                (char *) mng_info->read_buffer);
              mng_info->read_buffer[4]=0;
              mng_info->bytes_in_read_buffer=4;
              if (!png_memcmp(mng_info->read_buffer,mng_PLTE,4))
                mng_info->found_empty_plte=True;
              if (!png_memcmp(mng_info->read_buffer,mng_IEND, 4))
                {
                  mng_info->found_empty_plte=False;
                  mng_info->have_saved_bkgd_index=False;
                }
            }
          if ((data[0] == 0) && (data[1] == 0) && (data[2] == 0) &&
              (data[3] == 1))
            {
              check=(png_size_t) ReadBlob(image,(size_t) length,
                (char *) mng_info->read_buffer);
              mng_info->read_buffer[4]=0;
              mng_info->bytes_in_read_buffer=4;
              if (!png_memcmp(mng_info->read_buffer,mng_bKGD,4))
                if (mng_info->found_empty_plte)
                  {
                    /*
                      Skip the bKGD data byte and CRC.
                    */
                    check=(png_size_t)
                      ReadBlob(image,5,(char *) mng_info->read_buffer);
                    check=(png_size_t) ReadBlob(image,(size_t) length,
                      (char *) mng_info->read_buffer);
                    mng_info->saved_bkgd_index=mng_info->read_buffer[0];
                    mng_info->have_saved_bkgd_index=True;
                    mng_info->bytes_in_read_buffer=0;
                  }
            }
        }
    }
}
#endif

static void png_put_data(png_structp png_ptr,png_bytep data,png_size_t length)
{
  Image
    *image;

  image=(Image *) png_get_io_ptr(png_ptr);
  if (length)
    {
      png_size_t
        check;

      check=(png_size_t) WriteBlob(image,(unsigned long) length,(char *) data);
      if (check != length)
        png_error(png_ptr,"Write Error");
    }
}

static void png_flush_data(png_structp png_ptr)
{
  Image
    *image;

  image=(Image *) png_get_io_ptr(png_ptr);
  SyncBlob(image);
}
#endif

static int PalettesAreEqual(const ImageInfo *image_info,Image *a,Image *b)
{
  int
    i;

  if ((a == (Image *) NULL) || (b == (Image *) NULL))
    return((int) False);
  if (!IsPseudoClass(a) || !IsPseudoClass(b))
    return((int) False);
  if (a->colors != b->colors)
    return((int) False);
  for (i=0; i < (int) a->colors; i++)
  {
    if ((a->colormap[i].red != b->colormap[i].red) ||
        (a->colormap[i].green != b->colormap[i].green) ||
        (a->colormap[i].blue != b->colormap[i].blue))
      return((int) False);
  }
  return((int) True);
}

static void MngInfoDiscardObject(MngInfo *mng_info,int i)
{
  if (i && (i < MNG_MAX_OBJECTS) && (mng_info != (MngInfo *) NULL) &&
      mng_info->exists[i] && !mng_info->frozen[i])
    {
#ifdef MNG_OBJECT_BUFFERS
      if (mng_info->ob[i] != (MngInfoBuffer *) NULL)
        {
          if (mng_info->ob[i]->reference_count > 0)
            mng_info->ob[i]->reference_count--;
          if (mng_info->ob[i]->reference_count == 0)
            {
              if (mng_info->ob[i]->image != (Image *) NULL)
                DestroyImage(mng_info->ob[i]->image);
              FreeMemory(mng_info->ob[i]);
            }
        }
      mng_info->ob[i]=(MngInfoBuffer *) NULL;
#endif
      mng_info->exists[i]=False;
      mng_info->visible[i]=True;
      mng_info->viewable[i]=False;
      mng_info->frozen[i]=False;
      mng_info->x_off[i]=0;
      mng_info->y_off[i]=0;
      mng_info->object_clip[i].left=0;
      mng_info->object_clip[i].right=PNG_MAX_UINT;
      mng_info->object_clip[i].top=0;
      mng_info->object_clip[i].bottom=PNG_MAX_UINT;
    }
}

static void MngInfoFreeStruct(MngInfo *mng_info,int *have_mng_structure)
{
  if (*have_mng_structure && (mng_info != (MngInfo *) NULL))
    {
      register int
        i;

      for (i=1; i < MNG_MAX_OBJECTS; i++)
        MngInfoDiscardObject(mng_info,i);
      if (mng_info->global_plte != (png_colorp) NULL)
        FreeMemory(mng_info->global_plte);
      if (mng_info->global_sbit != (png_color_8p) NULL)
        FreeMemory(mng_info->global_sbit);
      FreeMemory(mng_info);
      *have_mng_structure=False;
    }
}

static MngBox mng_minimum_box(MngBox box1,MngBox box2)
{
  MngBox
    box;

  box=box1;
  if (box.left < box2.left)
    box.left=box2.left;
  if (box.top < box2.top)
    box.top=box2.top;
  if (box.right > box2.right)
    box.right=box2.right;
  if (box.bottom > box2.bottom)
    box.bottom=box2.bottom;
  return box;
}

static MngBox mng_read_box(MngBox previous_box,char delta_type,unsigned char *p)
{
   MngBox
      box;

  /*
    Read clipping boundaries from DEFI, CLIP, FRAM, or PAST chunk.
  */
  box.left=(long) ((p[0]  << 24) | (p[1]  << 16) | (p[2]  << 8) | p[3]);
  box.right=(long) ((p[4]  << 24) | (p[5]  << 16) | (p[6]  << 8) | p[7]);
  box.top=(long) ((p[8]  << 24) | (p[9]  << 16) | (p[10] << 8) | p[11]);
  box.bottom=(long) ((p[12] << 24) | (p[13] << 16) | (p[14] << 8) | p[15]);
  if (delta_type != 0)
    {
      box.left+=previous_box.left;
      box.right+=previous_box.right;
      box.top+=previous_box.top;
      box.bottom+=previous_box.bottom;
    }
  return(box);
}

static MngPair mng_read_pair(MngPair previous_pair,char delta_type,
  unsigned char *p)
{
  MngPair
    pair;

  /*
    Read two longs from MHDR, CLON, MOVE or PAST chunk
  */
  pair.a=(long) ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
  pair.b=(long) ((p[4] << 24) | (p[5] << 16) | (p[6] << 8) | p[7]);
  if (delta_type != 0)
    {
      pair.a+=previous_pair.a;
      pair.b+=previous_pair.b;
    }
  return(pair);
}

static long mng_get_long(unsigned char *p)
{
  return((long) ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]));
}

static void MNGCoalesce(Image *image)
{
/* I have been unable to get this working after version 4.2.9 */
#if 0
  long
    delay;

  register Image
    *p;

  if (image->previous == (Image *) NULL)
    return;
  p=image->previous;
  assert(p->next != (Image *) NULL);
  if (p->delay != 0)
    return;
  delay=(long) image->delay;
  CoalesceImages(p);
  p->file=(FILE *) NULL;
  p->blob_info.mapped=False;
  p->orphan=False;
  DestroyImage(p);
  image->delay=delay;
#endif
}

static void PNGErrorHandler(png_struct *ping,png_const_charp message)
{
  MagickWarning(DelegateWarning,message,(char *) NULL);
  longjmp(ping->jmpbuf,1);
}

static void ReadTextChunk(png_textp text,unsigned int i,char **value)
{
  unsigned int
    length;

  length=text[i].text_length;
  if (*value != (char *) NULL)
    *value=(char *) ReallocateMemory((char *) *value,strlen(*value)+length+1);
  else
    {
      *value=(char *) AllocateMemory(length+1);
      if (*value != (char *) NULL)
        **value='\0';
    }
  if (*value == (char *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"a. Memory allocation failed",
        (char *) NULL);
      return;
    }
  (void) strncat(*value,text[i].text,length);
  (*value)[length]='\0';
}

static void PNGWarningHandler(png_struct *ping,png_const_charp message)
{
  MagickWarning(DelegateWarning,message,(char *) NULL);
  if (ping == (png_struct *) NULL)
     return;
}

#ifdef PNG_USER_MEM_SUPPORTED
extern PNG_EXPORT(png_voidp,png_IM_malloc)
  PNGARG((png_structp png_ptr,png_uint_32 size));
extern PNG_EXPORT(png_free_ptr,png_IM_free)
  PNGARG((png_structp png_ptr,png_voidp ptr));
png_voidp png_IM_malloc(png_structp png_ptr,png_uint_32 size)
{
  return((png_voidp) AllocateMemory((size_t) size));
}

/*
  Free a pointer.  It is removed from the list at the same time.
*/
static png_free_ptr png_IM_free(png_structp png_ptr,png_voidp ptr)
{
  FreeMemory(ptr);
  return((png_free_ptr) NULL);
}
#endif

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

Export Image *ReadPNGImage(const ImageInfo *image_info)
{
  char
    page_geometry[MaxTextExtent];

  PixelPacket
    mng_background_color,
    mng_global_bkgd,
    transparent_color;

  Image
    *image;

  int
    have_global_bkgd,
    have_global_chrm,
    have_global_gama,
    have_global_phys,
    have_global_sbit,
    have_global_srgb,
    global_plte_length,
    global_trns_length,
    first_mng_object,
    image_found,
    have_mng_structure,
    object_id,
#if (QuantumDepth == 8)
    reduction_warning,
#endif
    term_chunk_found,
    skip_to_iend,
    y;

  long
    image_count = 0;

  MngInfo
    *mng_info;

  MngBox
    clip,
    default_fb,
    fb,
    frame,
    image_box,
    previous_fb;

  register int
    i,
    x;

  register unsigned char
    *p;

  register PixelPacket
    *q;

  png_info
    *end_info,
    *ping_info;

  png_struct
    *ping;

  short
    loop_level,
    loops_active,
    skipping_loop;

  unsigned char
    *png_pixels,
    **scanlines;

  unsigned int
    framing_mode = 1,
    mandatory_back = 0,
    mng_background_object = 0,
#ifdef MNG_LEVEL
    mng_level,
#endif
    mng_type = 0,   /* 0: PNG; 1: MNG; 2: MNG-LC; 3: MNG-VLC */
    simplicity = 0,
    status;

  unsigned long
    default_frame_delay,
    default_frame_timeout,
    final_delay,
    frame_delay,
    frame_timeout,
    image_height,
    image_width,
    length,
    mng_height = 0,
    mng_width = 0,
    subframe_height,
    subframe_width,
    ticks_per_second;

#ifdef MNG_LEVEL
  mng_level=MNG_LEVEL;
#endif
#if (QuantumDepth == 8)
  reduction_warning=False;
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
  first_mng_object=0;
  skipping_loop=(-1);
  have_mng_structure=False;
  if (Latin1Compare(image_info->magick,"MNG") == 0)
    {
      char
        magic_number[MaxTextExtent];

      /*
        Verify MNG signature.
      */
      (void) ReadBlob(image,8,magic_number);
      if (strncmp(magic_number,"\212MNG\r\n\032\n",8) != 0)
        ReaderExit(CorruptImageWarning,"Not a MNG image file",image);
      first_mng_object=1;
      /*
        Allocate a MngInfo structure.
      */
      mng_info=(MngInfo *) AllocateMemory(sizeof(MngInfo));
      if (mng_info == (MngInfo *) NULL)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      have_mng_structure=True;
      mng_info->image=image;
      mng_info->global_plte=(png_colorp) NULL;
      mng_info->global_sbit=(png_color_8p) NULL;
      mng_info->basi_warning=False;
      mng_info->clon_warning=False;
      mng_info->dhdr_warning=False;
      mng_info->jhdr_warning=False;
      mng_info->past_warning=False;
      mng_info->phyg_warning=False;
      mng_info->phys_warning=False;
      mng_info->sbit_warning=False;
      mng_info->show_warning=False;
      for (i=0; i < MNG_MAX_OBJECTS; i++)
      {
        mng_info->exists[i]=False;
        mng_info->visible[i]=True;
        mng_info->viewable[i]=False;
        mng_info->frozen[i]=False;
        mng_info->x_off[i]=0;
        mng_info->y_off[i]=0;
        mng_info->object_clip[i].left=0;
        mng_info->object_clip[i].right=PNG_MAX_UINT;
        mng_info->object_clip[i].top=0;
        mng_info->object_clip[i].bottom=PNG_MAX_UINT;
#ifdef MNG_OBJECT_BUFFERS
        mng_info->ob[i]=(MngInfoBuffer *) NULL;
#endif
      }
      mng_info->exists[0]=True;
      loop_level=0;
      loops_active=0;
      for (i=0; i < 256; i++)
      {
        mng_info->loop_active[i]=0;
        mng_info->loop_count[i]=0;
        mng_info->loop_iteration[i]=0;
        mng_info->loop_jump[i]=0;
      }
    }
  mng_type=0;
  default_frame_delay=0;
  default_frame_timeout=0;
  frame_delay=0;
  final_delay=100;
  ticks_per_second=100;
  object_id=0;
  skip_to_iend=False;
  term_chunk_found=False;
  image_found=0;
  framing_mode=1;
  mandatory_back=0;
  have_global_chrm=False;
  have_global_bkgd=False;
  have_global_gama=False;
  have_global_phys=False;
  have_global_sbit=False;
  have_global_srgb=False;
  global_plte_length=0;
  global_trns_length=0;
  mng_background_color=image->background_color;
  do
  {
    if (Latin1Compare(image_info->magick,"MNG") == 0)
      {
        char
          type[MaxTextExtent];

        unsigned char
          *chunk;

        /*
          Read a new chunk.
        */
        type[0]='\0';
        strcat(type,"errr");
        length=MSBFirstReadLong(image);
        status=ReadBlob(image,4,type);
        if (length > PNG_MAX_UINT)
          status=False;
        if (status == False)
          {
            MagickWarning(CorruptImageWarning,"Corrupt MNG image",
              image->filename);
            break;
          }
        if (!png_memcmp(type,mng_JHDR,4))
          {
            skip_to_iend=True;
            if (!mng_info->jhdr_warning)
              MagickWarning(DelegateWarning,"JNG is not implemented yet",
                image->filename);
            mng_info->jhdr_warning++;
          }
        if (!png_memcmp(type,mng_DHDR,4))
          {
            skip_to_iend=True;
            if (!mng_info->dhdr_warning)
            MagickWarning(DelegateWarning,"Delta-PNG is not implemented yet",
              image->filename);
            mng_info->dhdr_warning++;
          }
        if (length)
          {
            chunk=(unsigned char *)
              AllocateMemory(length*sizeof(unsigned char));
            if (chunk == (unsigned char *) NULL)
              ReaderExit(ResourceLimitWarning,
               "Unable to allocate memory for chunk data",image);
            for (i=0; i < (int) length; i++)
              chunk[i]=ReadByte(image);
            p=chunk;
          }
        (void) MSBFirstReadLong(image);  /* read crc word */
        if (!png_memcmp(type,mng_MEND,4))
          break;
        if (skip_to_iend)
          {
            if (!png_memcmp(type,mng_IEND,4))
              skip_to_iend=False;
            if (length)
              FreeMemory(chunk);
            continue;
          }

        if (!png_memcmp(type,mng_MHDR,4))
          {
            MngPair
               pair;

            pair.a=0;
            pair.b=0;
            pair=mng_read_pair(pair,0,p);
            mng_width=(unsigned long) pair.a;
            mng_height=(unsigned long) pair.b;
            p+=8;
            ticks_per_second=(unsigned long) mng_get_long(p);
            if (ticks_per_second == 0)
              default_frame_delay=0;
            else
              default_frame_delay=100/ticks_per_second;
            frame_delay=default_frame_delay;
            simplicity=0;
            if (length > 16)
              {
                p+=16;
                simplicity=(unsigned long) mng_get_long(p);
              }
            mng_type=1;    /* Full MNG */
            if ((simplicity != 0) && ((simplicity | 11) == 11))
              mng_type=2; /* LC */
            if ((simplicity != 0) && ((simplicity | 9) == 9))
              mng_type=3; /* VLC */
            if (image->pixels != (PixelPacket *) NULL)
              {
                /*
                  Allocate next image structure.
                */
                AllocateNextImage(image_info,image);
                if (image->next == (Image *) NULL)
                  {
                    DestroyImages(image);
                    MngInfoFreeStruct(mng_info,&have_mng_structure);
                    return((Image *) NULL);
                  }
                image->next->blob_info=image->blob_info;
                image=image->next;
                mng_info->image=image;
              }

            if (mng_width > 65535 || mng_height > 65535)
              MagickWarning(DelegateWarning,"Image dimensions are too large.",
                (char *) NULL);
            FormatString(page_geometry,"%lux%lu%+0+0",mng_width,mng_height);
            frame.left=0;
            frame.right=(long) mng_width;
            frame.top=0;
            frame.bottom=(long) mng_height;
            clip=default_fb=previous_fb=frame;
            for (i=0; i < MNG_MAX_OBJECTS; i++)
              mng_info->object_clip[i]=frame;
            FreeMemory(chunk);
            continue;
          }

        if (!png_memcmp(type,mng_TERM,4))
          {
            int
              repeat=0;

            png_uint_32
              iterations;

            if (length > 0)
              repeat=p[0];
            if (repeat == 3)
              {
                final_delay=(int) mng_get_long(&p[2]);
                iterations=(int) mng_get_long(&p[6]);
                if (iterations == PNG_MAX_UINT)
                  iterations=0;
                if (image_info->iterations == (char *) NULL)
                  image->iterations=iterations;
                term_chunk_found=True;
              }
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_DEFI,4))
          {
            if (mng_type == 3)
              MagickWarning(DelegateWarning,
                "DEFI chunk found in MNG-VLC datastream",(char *) NULL);
            object_id = (p[0]<<8) | p[1];
            if (mng_type == 2 && object_id != 0)
              MagickWarning(DelegateWarning,
               "Nonzero object_id in MNG-LC datastream",(char *) NULL);
            if (object_id > MNG_MAX_OBJECTS)
              {
                /*
                  Instead of issuing a warning we should allocate a larger
                  MngInfo structure and continue.
                */
                MagickWarning(DelegateWarning,"object_id too large",
                  (char *) NULL);
                object_id=MNG_MAX_OBJECTS;
              }
            if (mng_info->exists[object_id])
              if (mng_info->frozen[object_id])
                {
                  MagickWarning(DelegateWarning,
                   "DEFI cannot redefine a frozen MNG object",(char *) NULL);
                  FreeMemory(chunk);
                  continue;
                }
            mng_info->exists[object_id]=True;
            if (length > 2)
              mng_info->visible[object_id]=!p[2];
            /*
              Extract object offset info.
            */
            if (length>11)
              {
                MngPair
                  pair;

                pair.a=0;
                pair.b=0;
                pair=mng_read_pair(pair,0,&p[4]);
                mng_info->x_off[object_id]=pair.a;
                mng_info->y_off[object_id]=pair.b;
              }
            /*
              Extract object clipping info.
            */
            if (length>27)
              mng_info->object_clip[object_id]=mng_read_box(frame,0,&p[12]);
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_bKGD,4))
          {
            have_global_bkgd=False;
            if (length > 5)
              {
                mng_global_bkgd.red=
                  (unsigned short) XDownScale((p[0]<<8) | p[1]);
                mng_global_bkgd.green =
                  (unsigned short) XDownScale((p[2]<<8) | p[3]);
                mng_global_bkgd.blue  =
                  (unsigned short) XDownScale((p[4]<<8) | p[5]);
                have_global_bkgd=True;
              }
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_BACK,4))
          {
            if (length>6)
              mandatory_back=p[6];
            else
              mandatory_back=0;
            if (mandatory_back && length>5)
              {
                mng_background_color.red=
                  (unsigned short) XDownScale((p[0]<<8) | p[1]);
                mng_background_color.green=
                  (unsigned short) XDownScale((p[2]<<8) | p[3]);
                mng_background_color.blue=
                  (unsigned short) XDownScale((p[4]<<8) | p[5]);
              }
            if (length > 8)
              mng_background_object=(p[7] << 8) | p[8];
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_PLTE,4))
          {
            register int
             i;

            /*
              Read global PLTE.
            */
            if (length && (length < 769))
              {
                if (mng_info->global_plte == (png_colorp) NULL)
                  mng_info->global_plte=
                   (png_colorp) AllocateMemory(256*sizeof(png_color));
                for (i=0; i< (int) (length/3); i++)
                {
                  mng_info->global_plte[i].red=p[3*i];
                  mng_info->global_plte[i].green=p[3*i+1];
                  mng_info->global_plte[i].blue=p[3*i+2];
                }
                global_plte_length=(int) length/3;
              }
#ifdef MNG_LOOSE
            for ( ; i < 256; i++)
            {
              mng_info->global_plte[i].red=i;
              mng_info->global_plte[i].green=i;
              mng_info->global_plte[i].blue=i;
            }
            if (length)
              global_plte_length=256;
#endif
            else
              global_plte_length=0;
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_tRNS,4))
          {
            register int
              i;

            /* read global tRNS */

            if (length < 257)
              for (i=0; i<(int) length; i++)
                mng_info->global_trns[i]=p[i];

#ifdef MNG_LOOSE
            for ( ; i<256; i++)
              mng_info->global_trns[i]=255;
#endif
            global_trns_length=(int) length;
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_gAMA,4))
          {
            if (length == 4)
              {
                int igamma=(int) mng_get_long(p);
                mng_info->global_gamma=((float) igamma)*0.00001;
                have_global_gama=True;
              }
            else
              have_global_gama=False;
            FreeMemory(chunk);
            continue;
          }

        if (!png_memcmp(type,mng_cHRM,4))
          {
            /*
              Read global cHRM
            */
            if (length == 32)
              {
                mng_info->global_chrm.white_point.x=0.00001*mng_get_long(p);
                mng_info->global_chrm.white_point.y=0.00001*mng_get_long(&p[4]);
                mng_info->global_chrm.red_primary.x=0.00001*mng_get_long(&p[8]);
                mng_info->global_chrm.red_primary.y=0.00001*mng_get_long(&p[12]);
                mng_info->global_chrm.green_primary.x=0.00001*mng_get_long(&p[16]);
                mng_info->global_chrm.green_primary.y=0.00001*mng_get_long(&p[20]);
                mng_info->global_chrm.blue_primary.x=0.00001*mng_get_long(&p[24]);
                mng_info->global_chrm.blue_primary.y=0.00001*mng_get_long(&p[28]);
                have_global_chrm=True;
              }
            else
              have_global_chrm=False;
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_sRGB,4))
          {
            /*
              Read global sRGB.
            */
            if (length)
              {
                mng_info->global_srgb_intent=(RenderingIntent) p[0];
                have_global_srgb=True;
              }
            else
              have_global_srgb=False;
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_FRAM,4))
          {
            if (mng_type == 3)
              MagickWarning(DelegateWarning,
                 "FRAM chunk found in MNG-VLC datastream",(char *) NULL);
            if ((framing_mode == 2) || (framing_mode == 4))
              image->delay=(unsigned int) frame_delay;
            frame_delay=default_frame_delay;
            frame_timeout=default_frame_timeout;
            fb=default_fb;
            if (length > 0)
              framing_mode=p[0];
            if (length > 6)
              {
                /*
                  Note the delay and frame clipping boundaries.
                */
                p++; /* framing mode */
                while (*p && ((p-chunk) < (int) length))
                  p++;  /* frame name */
                p++;  /* frame name terminator */
                if ((p-chunk) < ((int) length-4))
                  {
                    int
                      change_delay,
                      change_timeout,
                      change_clipping;

                    change_delay=(*p++);
                    change_timeout=(*p++);
                    change_clipping=(*p++);
                    p++; /* change_sync */
                    if (change_delay)
                      {
                        frame_delay=(unsigned int)
                          (100*(mng_get_long(p))/ticks_per_second);
                        if (change_delay == 2)
                          default_frame_delay=frame_delay;
                        p+=4;
                      }
                    if (change_timeout)
                      {
                        frame_timeout=(unsigned int)
                          (100*(mng_get_long(p))/ticks_per_second);
                        if (change_delay == 2)
                          default_frame_timeout=frame_timeout;
                        p+=4;
                      }
                    if (change_clipping)
                      {
                        fb=mng_read_box(previous_fb,p[0],&p[1]);
                        p+=17;
                        previous_fb=fb;
                      }
                    if (change_clipping == 2)
                      default_fb=fb;
                  }
              }
            clip=fb;
            clip=mng_minimum_box(fb,frame);
            subframe_width=(unsigned int) (clip.right-clip.left);
            subframe_height=(unsigned int) (clip.bottom-clip.top);
            /*
              Insert a background layer behind the frame if framing_mode is 4.
            */
            if (image_info->insert_backdrops && (framing_mode == 4) &&
                (subframe_width > 0) && (subframe_height > 0))
              {
                /*
                  Allocate next image structure.
                */
                AllocateNextImage(image_info,image);
                if (image->next == (Image *) NULL)
                  {
                    DestroyImages(image);
                    MngInfoFreeStruct(mng_info,&have_mng_structure);
                    return((Image *) NULL);
                  }
                image->next->blob_info=image->blob_info;
                image=image->next;
                mng_info->image=image;
                if (term_chunk_found)
                  {
                    image->restart_animation_here=True;
                    term_chunk_found=False;
                  }
                image->columns=subframe_width;
                image->rows=subframe_height;
                image->page_info.width=subframe_width;
                image->page_info.height=subframe_height;
                image->page_info.x=clip.left;
                image->page_info.y=clip.top;
                image->background_color=mng_background_color;
                image->matte=False;
                image->delay=0;
                if (image_info->coalesce_frames)
                  MNGCoalesce(image);
              }
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_CLIP,4))
          {
            unsigned int
              first_object,
              last_object;

            /*
              Read CLIP.
            */
            first_object=(p[0]<<8) | p[1];
            last_object=(p[2]<<8) | p[3];
            for (i=(int) first_object; i<=(int) last_object; i++)
            {
              if (mng_info->exists[i] && !mng_info->frozen[i])
                {
                  MngBox
                    box;

                  box=mng_info->object_clip[i];
                  mng_info->object_clip[i]=mng_read_box(box,p[4],&p[5]);
                }
            }
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_SAVE,4))
          {
            register int
              i;

            for (i=1; i<MNG_MAX_OBJECTS; i++)
              if (mng_info->exists[i])
                {
                 mng_info->frozen[i]=True;
#ifdef MNG_OBJECT_BUFFERS
                 if (mng_info->ob[i] != (MngInfoBuffer *) NULL)
                    mng_info->ob[i]->frozen=True;
#endif
                }
            if (length > 0)
              FreeMemory(chunk);
            continue;
          }

        if (!png_memcmp(type,mng_DISC,4) || !png_memcmp(type,mng_SEEK,4))
          {
            register int
              i;

            /*
              Read DISC or SEEK.
            */
            if ((length == 0) || !png_memcmp(type,mng_SEEK,4))
              {
                for (i=1; i < MNG_MAX_OBJECTS; i++)
                  MngInfoDiscardObject(mng_info,i);
              }
            else
              {
                register int
                  j;

                for (j=0; j < (int) length; j+=2)
                {
                  i=p[j]<<8 | p[j+1];
                  MngInfoDiscardObject(mng_info,i);
                }
              }
            if (length > 0)
              FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_MOVE,4))
          {
            long
              first_object,
              last_object;

            /*
              read MOVE
            */
            first_object=(p[0]<<8) | p[1];
            last_object=(p[2]<<8) | p[3];
            for (i=first_object; i <= last_object; i++)
            {
              if (mng_info->exists[i] && !mng_info->frozen[i])
                {
                  MngPair
                    new_pair,
                    old_pair;

                  old_pair.a=mng_info->x_off[i];
                  old_pair.b=mng_info->y_off[i];
                  new_pair=mng_read_pair(old_pair,p[4],&p[5]);
                  mng_info->x_off[i]=new_pair.a;
                  mng_info->y_off[i]=new_pair.b;
                }
            }
            FreeMemory(chunk);
            continue;
          }

        if (!png_memcmp(type,mng_LOOP,4))
          {
            long loop_iters=1;
            loop_level=chunk[0];
            loops_active++;
            mng_info->loop_active[loop_level]=1;  /* mark loop active */
            /*
              Record starting point.
            */
            loop_iters = mng_get_long(&chunk[1]);
            if (loop_iters == 0)
              skipping_loop=loop_level;
            else
              {
                mng_info->loop_jump[loop_level]=TellBlob(image);
                mng_info->loop_count[loop_level]=loop_iters;
              }
            mng_info->loop_iteration[loop_level]=0;
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_ENDL,4))
          {
            loop_level=chunk[0];
            if (skipping_loop > 0)
              {
                if (skipping_loop == loop_level)
                  {
                    /*
                      Found end of zero-iteration loop.
                    */
                    skipping_loop=(-1);
                    loops_active--;
                    mng_info->loop_active[loop_level]=0;
                  }
              }
            else
              {
                if (mng_info->loop_active[loop_level] == 1)
                  {
                    mng_info->loop_count[loop_level]--;
                    mng_info->loop_iteration[loop_level]++;
                    if (mng_info->loop_count[loop_level] != 0)
                      (void) SeekBlob(image,mng_info->loop_jump[loop_level],
                        SEEK_SET);
                    else
                      {
                        int
                          last_level;

                        /*
                          Finished loop.
                        */
                        loops_active--;
                        mng_info->loop_active[loop_level]=0;
                        last_level=(-1);
                        for (i=0; i < loop_level; i++)
                          if (mng_info->loop_active[i] == 1)
                            last_level=i;
                        loop_level=last_level;
                      }
                  }
              }
            FreeMemory(chunk);
            continue;
          }
        if (!png_memcmp(type,mng_CLON,4))
          {
            if (!mng_info->clon_warning)
              MagickWarning(DelegateWarning,"CLON is not implemented yet",
                image->filename);
            mng_info->clon_warning++;
          }
        if (!png_memcmp(type,mng_PAST,4))
          {
            if (!mng_info->past_warning)
              MagickWarning(DelegateWarning,"PAST is not implemented yet",
                image->filename);
            mng_info->past_warning++;
          }
        if (!png_memcmp(type,mng_SHOW,4))
          {
            if (!mng_info->show_warning)
              MagickWarning(DelegateWarning,"SHOW is not implemented yet",
                image->filename);
            mng_info->show_warning++;
          }
        if (!png_memcmp(type,mng_sBIT,4))
          {
            if (length < 4)
              have_global_sbit=False;
            else
              {
                if (mng_info->global_sbit == (png_color_8p) NULL)
                  mng_info->global_sbit=
                    (png_color_8p) AllocateMemory(sizeof(png_color_8));
                mng_info->global_sbit->gray=p[0];
                mng_info->global_sbit->red=p[0];
                mng_info->global_sbit->green=p[1];
                mng_info->global_sbit->blue=p[2];
                mng_info->global_sbit->alpha=p[3];
                have_global_sbit=True;
             }
          }
        if (!png_memcmp(type,mng_pHYs,4))
          {
            if (length > 8)
              {
                mng_info->global_x_pixels_per_unit=mng_get_long(p);
                mng_info->global_y_pixels_per_unit=mng_get_long(&p[4]);
                mng_info->global_phys_unit_type=p[8];
                have_global_phys=True;
              }
            else
              have_global_phys=False;
          }
        if (!png_memcmp(type,mng_pHYg,4))
          {
            if (!mng_info->phyg_warning)
              MagickWarning(DelegateWarning,"pHYg is not implemented.",
                image->filename);
            mng_info->phyg_warning++;
          }
        if (!png_memcmp(type,mng_BASI,4))
          {
#ifdef MNG_BASI_SUPPORTED
            MngInfoPair
               pair;
#endif
            skip_to_iend=True;
            if (!mng_info->basi_warning)
              MagickWarning(DelegateWarning,"BASI is not implemented yet",
                image->filename);
            mng_info->basi_warning++;
#ifdef MNG_BASI_SUPPORTED
            pair.a=0;
            pair.b=0;
            pair=mng_read_pair(pair,0,p);
            basi_width=(unsigned long) pair.a;
            basi_height=(unsigned long) pair.b;
            basi_color_type=p[8];
            basi_compression_method=p[9];
            basi_filter_type=p[10];
            basi_interlace_method=p[11];
            if (length > 11)
              basi_red=(p[12] << 8) & p[13];
            else
              basi_red=0;
            if (length > 13)
              basi_green=(p[14] << 8) & p[15];
            else
              basi_green=0;
            if (length > 15)
              basi_blue=(p[16] << 8) & p[17];
            else
              basi_blue=0;
            if (length > 17)
              basi_alpha=(p[18] << 8) & p[19];
            else
              {
                if (basi_sample_depth == 16)
                  basi_alpha=65535;
                else
                  basi_alpha=255;
              }
            if (length > 19)
              basi_viewable=p[20];
            else
              basi_viewable=0;
#endif
            FreeMemory(chunk);
            continue;
          }
        if (png_memcmp(type,mng_IHDR,4))
          {
            if (length > 0)
              FreeMemory(chunk);
            continue;
          }
        mng_info->exists[object_id]=True;
        mng_info->viewable[object_id]=True;
        if (!mng_info->visible[object_id])
          {
            if (!image_info->decode_all_MNG_objects)
              {
                skip_to_iend=True;
                FreeMemory(chunk);
                continue;
              }
          }
        /*
          Insert a background layer behind the upcoming image if
          framing_mode is 3 and transparency might be present.
        */
        image_width=(unsigned int) mng_get_long(p);
        image_height=(unsigned int) mng_get_long(&p[4]);
        FreeMemory(chunk);
        if (image_info->insert_backdrops && (framing_mode == 3) &&
            ((first_mng_object == 0) || ((clip.left == 0) && (clip.top == 0) &&
              (image_width == mng_width) && (image_height == mng_height))))
          {
            if (simplicity == 0 || (simplicity & 0x08) == 0x08)
              {
                unsigned int
                  delay;

                /*
                  Allocate next image structure.
                */
                AllocateNextImage(image_info,image);
                if (image->next == (Image *) NULL)
                  {
                    DestroyImages(image);
                    MngInfoFreeStruct(mng_info,&have_mng_structure);
                    return((Image *) NULL);
                  }
                image->next->blob_info=image->blob_info;
                image=image->next;
                mng_info->image=image;
                if (term_chunk_found)
                  {
                    image->restart_animation_here=True;
                    term_chunk_found=False;
                  }
                delay=image->delay;
                image->delay=0;
                image->columns=subframe_width;
                image->rows=subframe_height;
                image->page_info.width=subframe_width;
                image->page_info.height=subframe_height;
                image->page_info.x=clip.left;
                image->page_info.y=clip.top;
                image->background_color=mng_background_color;
                image->matte=False;
                SetImage(image);
                image->delay=delay;
              }
          }
        /*
          Insert a background layer behind the entire animation if it
          is not full screen or transparency might be present.
        */
        if (mng_type && image_info->insert_backdrops && first_mng_object)
          {
            if ((simplicity == 0) || ((simplicity & 0x08) == 0x08) ||
                ((clip.left != 0) || (clip.top != 0) ||
                (image_width != mng_width) || (image_height != mng_height)))
              {
                unsigned int
                  delay;

                /*
                  Make a background rectangle.
                */
                delay=image->delay;
                image->delay=0;
                image->columns=mng_width;
                image->rows=mng_height;
                image->page_info.width=mng_width;
                image->page_info.height=mng_height;
                image->page_info.x=0;
                image->page_info.y=0;
                image->background_color=mng_background_color;
                image->matte=False;
                image->delay=delay;
                SetImage(image);
              }
          }
        first_mng_object=0;
        /*
          Read the PNG image.
        */
        if (image->pixels != (PixelPacket *) NULL)
          {
            /*
              Allocate next image structure.
            */
            AllocateNextImage(image_info,image);
            if (image->next == (Image *) NULL)
              {
                DestroyImages(image);
                MngInfoFreeStruct(mng_info,&have_mng_structure);
                return((Image *) NULL);
              }
            image->next->blob_info=image->blob_info;
            image=image->next;
            mng_info->image=image;
            ProgressMonitor(LoadImagesText,TellBlob(image),image->filesize);
          }
        if (term_chunk_found)
          {
            image->restart_animation_here=True;
            term_chunk_found=False;
          }
        if (framing_mode == 1 || framing_mode == 3)
          {
            image->delay=(unsigned int) frame_delay;
            frame_delay = default_frame_delay;
          }
        else
          image->delay=0;
        image->page_info.width=mng_width;
        image->page_info.height=mng_height;
        image->page_info.x=mng_info->x_off[object_id];
        image->page_info.y=mng_info->y_off[object_id];
        /*
          Seek back to the beginning of the IHDR chunk's length field.
        */
        (void) SeekBlob(image,-((int) length+12),SEEK_CUR);
      }

    /*
      Allocate the PNG structures
    */
#ifdef PNG_USER_MEM_SUPPORTED
   ping=png_create_read_struct_2(PNG_LIBPNG_VER_STRING,(void *) NULL,
     PNGErrorHandler,PNGWarningHandler,(void *) NULL,
     (png_malloc_ptr) png_IM_malloc,(png_free_ptr) png_IM_free);
#else
    ping=png_create_read_struct(PNG_LIBPNG_VER_STRING,(void *) NULL,
      PNGErrorHandler,PNGWarningHandler);
#endif
    if (ping == (png_struct *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    ping_info=png_create_info_struct(ping);
    if (ping_info == (png_info *) NULL)
      {
        png_destroy_read_struct(&ping,(png_info **) NULL,(png_info **) NULL);
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      }
    end_info=png_create_info_struct(ping);
    if (end_info == (png_info *) NULL)
      {
        png_destroy_read_struct(&ping,&ping_info,(png_info **) NULL);
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      }
    png_pixels=(unsigned char *) NULL;
    scanlines=(unsigned char **) NULL;
    if (setjmp(ping->jmpbuf))
      {
        /*
          PNG image is corrupt.
        */
        png_destroy_read_struct(&ping,&ping_info,&end_info);
        if (scanlines != (unsigned char **) NULL)
          FreeMemory(scanlines);
        if (png_pixels != (unsigned char *) NULL)
          FreeMemory(png_pixels);
        CloseBlob(image);
        if ((image->columns == 0) || (image->rows == 0))
          {
            DestroyImage(image);
            MngInfoFreeStruct(mng_info,&have_mng_structure);
            return((Image *) NULL);
          }
        MngInfoFreeStruct(mng_info,&have_mng_structure);
        return(image);
      }
    /*
      Prepare PNG for reading.
    */
    image_found++;
    if (Latin1Compare(image_info->magick,"MNG") == 0)
      {
        png_set_sig_bytes(ping,8);
#ifdef PNG_READ_EMPTY_PLTE_SUPPORTED
        png_permit_empty_plte(ping,True);
        png_set_read_fn(ping,image,png_get_data);
#else
        mng_info->image=image;
        mng_info->bytes_in_read_buffer=0;
        mng_info->found_empty_plte=False;
        mng_info->have_saved_bkgd_index=False;
        png_set_read_fn(ping,mng_info,mng_get_data);
#endif
      }
    else
      png_set_read_fn(ping,image,png_get_data);
    png_read_info(ping,ping_info);
    image->depth=ping_info->bit_depth;
    if (ping_info->bit_depth < 8)
      {
        if ((ping_info->color_type == PNG_COLOR_TYPE_PALETTE))
          {
            png_set_packing(ping);
            image->depth=8;
          }
      }
#if defined(PNG_READ_sRGB_SUPPORTED)
    {
      int
        intent;

      if (have_global_srgb)
        image->rendering_intent=
          (RenderingIntent) (mng_info->global_srgb_intent+1);
      if (png_get_sRGB(ping, ping_info, &intent))
        image->rendering_intent=(RenderingIntent) (intent+1);
    }
#endif
    {
       double
          file_gamma;

       if (have_global_gama)
         image->gamma=mng_info->global_gamma;
       if (png_get_gAMA(ping, ping_info, &file_gamma))
         image->gamma=(float) file_gamma;
    }
    if (have_global_chrm)
        image->chromaticity=mng_info->global_chrm;
    if (ping_info->valid & PNG_INFO_cHRM)
      {
        png_get_cHRM(ping, ping_info,
          &image->chromaticity.white_point.x,
          &image->chromaticity.white_point.y,
          &image->chromaticity.red_primary.x,
          &image->chromaticity.red_primary.y,
          &image->chromaticity.green_primary.x,
          &image->chromaticity.green_primary.y,
          &image->chromaticity.blue_primary.x,
          &image->chromaticity.blue_primary.y);
      }
    if (image->rendering_intent)
      {
        image->gamma=.45455;
        image->chromaticity.white_point.x  =0.3127;
        image->chromaticity.white_point.y  =0.3290;
        image->chromaticity.red_primary.x  =0.6400;
        image->chromaticity.red_primary.y  =0.3300;
        image->chromaticity.green_primary.x=0.3000;
        image->chromaticity.green_primary.y=0.6000;
        image->chromaticity.blue_primary.x =0.1500;
        image->chromaticity.blue_primary.y =0.0600;
      }
    if (have_global_gama || image->rendering_intent)
      ping_info->valid |= PNG_INFO_gAMA;
    if (have_global_chrm || image->rendering_intent)
      ping_info->valid |= PNG_INFO_cHRM;
    if (ping_info->valid & PNG_INFO_pHYs)
      {
        /*
          Set image resolution.
        */
        png_uint_32
           res_x,
           res_y;

        int
           unit_type;

        png_get_pHYs(ping, ping_info, &res_x, &res_y, &unit_type);
        image->x_resolution=(float) res_x;
        image->y_resolution=(float) res_y;
        if (unit_type == PNG_RESOLUTION_METER)
          {
            image->units=PixelsPerCentimeterResolution;
            image->x_resolution=(float) res_x/100.0;
            image->y_resolution=(float) res_y/100.0;
          }
      }
    else
      {
        if (have_global_phys)
          {
            image->x_resolution=(float) mng_info->global_x_pixels_per_unit;
            image->y_resolution=(float) mng_info->global_y_pixels_per_unit;
            if (mng_info->global_phys_unit_type == PNG_RESOLUTION_METER)
              {
                image->units=PixelsPerCentimeterResolution;
                image->x_resolution=(float) mng_info->global_x_pixels_per_unit
                   /100.0;
                image->y_resolution=(float) mng_info->global_y_pixels_per_unit
                   /100.0;
              }
            ping_info->valid|=PNG_INFO_pHYs;
          }
      }
    if (ping_info->valid & PNG_INFO_PLTE)
      {
        int
          num_palette;

        png_colorp
          palette;

        png_get_PLTE(ping,ping_info,&palette,&num_palette);
        if (num_palette == 0 && ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
          {
            if (global_plte_length)
              {
                png_set_PLTE(ping,ping_info,mng_info->global_plte,
                  global_plte_length);
                if (!(ping_info->valid & PNG_INFO_tRNS))
                  if (global_trns_length)
                    {
                      if (global_trns_length > global_plte_length)
                        MagickWarning(DelegateWarning,
                          "global tRNS has more entries than global PLTE",
                          image_info->filename);
                      png_set_tRNS(ping,ping_info,mng_info->global_trns,
                        global_trns_length,NULL);
                    }
#if defined(PNG_READ_bKGD_SUPPORTED)
                if (
#ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
                     mng_info->have_saved_bkgd_index ||
#endif
			 ping_info->valid & PNG_INFO_bKGD)
		      {
			png_color_16
			   background;

    #ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
			if (mng_info->have_saved_bkgd_index)
			  background.index=mng_info->saved_bkgd_index;
			else
    #endif
			  background.index=ping_info->background.index;
			background.red=
			  (png_uint_16) mng_info->global_plte[background.index].red;
			background.green= (png_uint_16)
			  mng_info->global_plte[background.index].green;
			background.blue=(png_uint_16)
			  mng_info->global_plte[background.index].blue;
			png_set_bKGD(ping,ping_info,&background);
		      }
    #endif
		  }
		else
		  MagickWarning(DelegateWarning,
		    "No global PLTE in file",image_info->filename);
	      }
	  }

#if defined(PNG_READ_bKGD_SUPPORTED)
    if (have_global_bkgd && !(ping_info->valid & PNG_INFO_bKGD))
      {
#if (QuantumDepth == 16)
        image->background_color=mng_global_bkgd;
#else
        image->background_color.red=XDownScale(mng_global_bkgd.red);
        image->background_color.green=XDownScale(mng_global_bkgd.green);
        image->background_color.blue=XDownScale(mng_global_bkgd.blue);
#endif
      }
    if (ping_info->valid & PNG_INFO_bKGD)
      {
        /*
          Set image background color.
        */
        if (ping_info->bit_depth <= QuantumDepth)
          {
            image->background_color.red=ping_info->background.red;
            image->background_color.green=ping_info->background.green;
            image->background_color.blue=ping_info->background.blue;
          }
        else
          {
            image->background_color.red=XDownScale(ping_info->background.red);
            image->background_color.green=
              XDownScale(ping_info->background.green);
            image->background_color.blue=XDownScale(ping_info->background.blue);
          }
      }
#endif
    if (ping_info->valid & PNG_INFO_tRNS)
      {
        /*
          Image has a transparent background.
        */
        transparent_color.red=ping_info->trans_values.red;
        transparent_color.green=ping_info->trans_values.green;
        transparent_color.blue=ping_info->trans_values.blue;
        transparent_color.opacity=ping_info->trans_values.gray;
        if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
          {
            transparent_color.red=transparent_color.opacity;
            transparent_color.green=transparent_color.opacity;
            transparent_color.blue=transparent_color.opacity;
          }
      }
#if defined(PNG_READ_sBIT_SUPPORTED)
    if (have_global_sbit)
      if ((!ping_info->valid & PNG_INFO_sBIT))
        png_set_sBIT(ping,ping_info,mng_info->global_sbit);
#endif
    png_read_update_info(ping,ping_info);
    /*
      Initialize image structure.
    */
    image_box.left=0;
    image_box.right=(long) ping_info->width;
    image_box.top=0;
    image_box.bottom=(long) ping_info->height;
    if (mng_type == 0)
      {
        mng_width=ping_info->width;
        mng_height=ping_info->height;
        frame=image_box;
        clip=image_box;
      }
    image->compression=ZipCompression;
    image->columns=(unsigned int) ping_info->width;
    image->rows=(unsigned int) ping_info->height;
    if ((ping_info->color_type == PNG_COLOR_TYPE_PALETTE) ||
        (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
        (ping_info->color_type == PNG_COLOR_TYPE_GRAY))
      {
        image->class=PseudoClass;
        image->colors=1 << ping_info->bit_depth;
        if (ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
          {
            png_colorp
               palette;
            int
               num_palette;
            png_get_PLTE(ping, ping_info, &palette, &num_palette);
            image->colors=num_palette;
          }
      }
    if (image_info->ping)
      {
        png_destroy_read_struct(&ping,&ping_info,&end_info);
        CloseBlob(image);
        MngInfoFreeStruct(mng_info,&have_mng_structure);
        return(image);
      }
    png_pixels=(unsigned char *)
      AllocateMemory(ping_info->rowbytes*image->rows*sizeof(Quantum));
    scanlines=(unsigned char **)
      AllocateMemory(image->rows*sizeof(unsigned char *));
    if ((png_pixels == (unsigned char *) NULL) ||
        (scanlines == (unsigned char **) NULL))
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    if (image->class == PseudoClass)
      {
        /*
          Initialize image colormap.
        */
        image->colormap=(PixelPacket *)
          AllocateMemory(image->colors*sizeof(PixelPacket));
        if (image->colormap == (PixelPacket *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        if (ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
          {
            png_colorp
               palette;

            int
               num_palette;

            png_get_PLTE(ping, ping_info, &palette, &num_palette);
            for (i=0; i < (int) image->colors; i++)
            {
              image->colormap[i].red= (unsigned short) UpScale(palette[i].red);
              image->colormap[i].green=
                (unsigned short) UpScale(palette[i].green);
              image->colormap[i].blue= (unsigned short) UpScale(palette[i].blue);
            }
          }
        else
          for (i=0; i < (int) image->colors; i++)
          {
            image->colormap[i].red=(MaxRGB*i)/Max(image->colors-1,1);
            image->colormap[i].green=(MaxRGB*i)/Max(image->colors-1,1);
            image->colormap[i].blue=(MaxRGB*i)/Max(image->colors-1,1);
          }
      }
    /*
      Read image scanlines.
    */
    for (i=0; i < (int) image->rows; i++)
      scanlines[i]=png_pixels+(i*ping_info->rowbytes);
    png_read_image(ping,scanlines);
    png_read_end(ping,ping_info);
    /*
      Convert PNG pixels to pixel packets.
    */
    if (image->class == DirectClass)
      {
        /*
          Convert image to DirectClass pixel packets.
        */
        if ((ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA) ||
            (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA) ||
            (ping_info->valid & PNG_INFO_tRNS))
          image->matte=True;
        for (y=0; y < (int) image->rows; y++)
        {
          if (!SetPixelCache(image,0,y,image->columns,1))
            break;
          if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
            (void) ReadPixelCache(image,GrayQuantum,scanlines[y]);
          else
            if (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
              (void) ReadPixelCache(image,GrayOpacityQuantum,scanlines[y]);
            else
              if (ping_info->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
                (void) ReadPixelCache(image,RGBAQuantum,scanlines[y]);
              else
                if (ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
                  (void) ReadPixelCache(image,IndexQuantum,scanlines[y]);
                else
                  (void) ReadPixelCache(image,RGBQuantum,scanlines[y]);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,y,image->rows);
        }
      }
    else
      {
        Quantum
          *quantum_scanline;

        register Quantum
          *r;

        /*
          Convert image to PseudoClass pixel packets.
        */
        quantum_scanline=(Quantum *)
          AllocateMemory((ping_info->color_type == 4 ? 2 : 1) * image->columns
             *sizeof(Quantum));
        if (quantum_scanline == (Quantum *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);

        for (y=0; y < (int) image->rows; y++)
        {
          q=SetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          p=scanlines[y];
          r=quantum_scanline;
          switch (ping_info->bit_depth)
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
                }
              break;
            }
            case 2:
            {
              for (x=0; x < ((int) image->columns-3); x+=4)
              {
                *r++=(*p >> 6) & 0x03;
                *r++=(*p >> 4) & 0x03;
                *r++=(*p >> 2) & 0x03;
                *r++=(*p++) & 0x03;
              }
              if ((image->columns % 4) != 0)
                {
                  for (i=3; i >= (int) (4-(image->columns % 4)); i--)
                    *r++=(*p >> (i*2)) & 0x03;
                }
              break;
            }
            case 4:
            {
              for (x=0; x < ((int) image->columns-1); x+=2)
              {
                *r++=(*p >> 4) & 0x0f;
                *r++=(*p++) & 0x0f;
              }
              if ((image->columns % 2) != 0)
                *r++=(*p++ >> 4) & 0x0f;
              break;
            }
            case 8:
            {
#if (QuantumDepth == 108)
              memcpy(r,p, (int) image->columns * sizeof(Quantum));
#else
              for (x=0; x < (int) image->columns; x++)
              {
                *r++=(*p++);
                if (ping_info->color_type == 4)
                  {
                    q->opacity=(*p++);
                    q++;
                  }
              }
#endif
              break;
            }
            case 16:
            {
#if (QuantumDepth == 116 && WORDS_BIGENDIAN)
              memcpy(r,p, (ping_info->color_type == 4 ? 2 : 1) *
                   image->columns * sizeof(Quantum));
#else
              for (x=0; x < (int) image->columns; x++)
              {
#if (QuantumDepth == 16)
                *r=((*p++) << 8);
                *r++|=(*p++);
                if (ping_info->color_type == 4)
                  {
                    q->opacity=((*p++) << 8);
                    q->opacity|=(*p++);
                    q++;
                  }
#else
                image->indexes[x]=(unsigned short) (((*p) << 8) | (*(p+1)));
                *r++=(*p++);
                p++;
                if (ping_info->color_type == 4)
                  {
                    *r++|=(*p++);
                    p++;
                  }
#endif
              }
#endif
              break;
            }

            default:
              break;
          }
          /*
            Transfer image scanline.
          */
          r=quantum_scanline;
#if (QuantumDepth == 8)
          if (ping_info->bit_depth < 16)
#endif
            for (x=0; x < (int) image->columns; x++)
                image->indexes[x]=(*r++);
          if (!SyncPixelCache(image))
            break;
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(LoadImageText,y,image->rows);
        }
        FreeMemory(quantum_scanline);
      }
        if (image->class == PseudoClass)
          SyncImage(image);
        if (ping_info->valid & PNG_INFO_tRNS)
          {
            ClassType
              class;
            /*
              Image has a transparent background.
            */
            image->matte=True;
            class=image->class;
            for (y=0; y < (int) image->rows; y++)
            {
              image->class=class;
              q=GetPixelCache(image,0,y,image->columns,1);
              image->class=DirectClass;
              if (q == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                unsigned short
                  index;

                q->opacity=Opaque;
                if (class == PseudoClass)
                  {
                    index=image->indexes[x];
                    if (ping_info->color_type != PNG_COLOR_TYPE_PALETTE)
                      {
                        q->red=image->colormap[index].red;
                        q->green=image->colormap[index].green;
                        q->blue=image->colormap[index].blue;
                      }
                    if (ping_info->color_type == PNG_COLOR_TYPE_PALETTE)
                      {
                        if (index < ping_info->num_trans)
                          q->opacity=(Quantum) UpScale(ping_info->trans[index]);
                      }
                  if (ping_info->color_type != PNG_COLOR_TYPE_PALETTE)
                    {
                      if (q->red == transparent_color.opacity)
                        q->opacity=Transparent;
                    }
                  }
                else
                  if (q->red == transparent_color.red &&
                      q->green == transparent_color.green &&
                      q->blue == transparent_color.blue)
                     q->opacity=Transparent;
                q++;
              }
              if (!SyncPixelCache(image))
                break;
            }
          }
#if (QuantumDepth == 8)
    if (image->depth > 8)
      image->depth = 8;
#endif
    {
    png_textp
       text;

    int
       num_text;

    if (png_get_text(ping, ping_info, &text, &num_text) > 0)
      for (i=0; i < num_text; i++)
        {
          if (Latin1Compare(text[i].key,"Comment") == 0)
            ReadTextChunk(text,i,&image->comments);
          if (Latin1Compare(text[i].key,"Delay") == 0)
            if (image_info->delay == (char *) NULL)
              {
                char
                  *delay;

                delay=(char *) NULL;
                ReadTextChunk(text,i,&delay);
                image->delay=atoi(delay);
                FreeMemory(delay);
              }
          if (Latin1Compare(text[i].key,"Description") == 0)
            ReadTextChunk(text,i,&image->comments);
          if (Latin1Compare(text[i].key,"Directory") == 0)
            ReadTextChunk(text,i,&image->directory);
          if (Latin1Compare(text[i].key,"Label") == 0)
            ReadTextChunk(text,i,&image->label);
          if (Latin1Compare(text[i].key,"Montage") == 0)
            ReadTextChunk(text,i,&image->montage);
          if (Latin1Compare(text[i].key,"Scene") == 0)
            {
              char
                *scene;

              scene=(char *) NULL;
              ReadTextChunk(text,i,&scene);
              image->scene=atoi(scene);
              FreeMemory(scene);
            }
          if (Latin1Compare(text[i].key,"Signature") == 0)
            ReadTextChunk(text,i,&image->signature);
          if (Latin1Compare(text[i].key,"Title") == 0)
            ReadTextChunk(text,i,&image->label);
        }
    }
#ifdef MNG_OBJECT_BUFFERS
    /*
      Store the object if necessary.
    */
    if (object_id && !mng_info->frozen[object_id])
      {
        if (mng_info->ob[object_id] == (MngInfoBuffer *) NULL)
          {
            /*
              create a new object buffer.
            */
            mng_info->ob[object_id]=(MngInfoBuffer *)
              AllocateMemory(sizeof(MngInfoBuffer));
            if (mng_info->ob[object_id] != (MngInfoBuffer *) NULL)
              {
                mng_info->ob[object_id]->image=(Image *) NULL;
                mng_info->ob[object_id]->reference_count=1;
              }
          }
        if ((mng_info->ob[object_id] == (MngInfoBuffer *) NULL) ||
            mng_info->ob[object_id]->frozen)
          {
            if (mng_info->ob[object_id] == (MngInfoBuffer *) NULL)
              MagickWarning(ResourceLimitWarning,
                "Memory allocation of MNG object buffer failed",
                image->filename);
            if (mng_info->ob[object_id]->frozen)
              MagickWarning(ResourceLimitWarning,
                "Cannot overwrite frozen MNG object buffer",image->filename);
          }
        else
          {
            png_uint_32
              width,
              height;

            int
              bit_depth,
              color_type,
              interlace_method,
              compression_method,
              filter_method;

            if (mng_info->ob[object_id]->image != (Image *) NULL)
              DestroyImage(mng_info->ob[object_id]->image);
            mng_info->ob[object_id]->image=
              CloneImage(image,image->columns,image->rows,True);
            if (mng_info->ob[object_id]->image != (Image *) NULL)
              mng_info->ob[object_id]->image->file=(FILE *) NULL;
            else
              MagickWarning(ResourceLimitWarning,
                "Cloning image for object buffer failed",image->filename);
            png_get_IHDR(ping,ping_info,&width,&height,&bit_depth,&color_type,
              &interlace_method,&compression_method,&filter_method);
            mng_info->ob[object_id]->width=width;
            mng_info->ob[object_id]->height=height;
            mng_info->ob[object_id]->color_type=color_type;
            mng_info->ob[object_id]->sample_depth=bit_depth;
            mng_info->ob[object_id]->interlace_method=interlace_method;
            mng_info->ob[object_id]->compression_method=compression_method;
            mng_info->ob[object_id]->filter_method=filter_method;
            if (ping_info->valid & PNG_INFO_PLTE)
              {
                int
                  num_palette;

                png_colorp
                  plte;

                /*
                  Copy the PLTE to the object buffer.
                */
                png_get_PLTE(ping,ping_info,&plte,&num_palette);
                mng_info->ob[object_id]->plte_length = num_palette;
                for (i=0; i<num_palette; i++)
                {
                  mng_info->ob[object_id]->plte[i]=plte[i];
                }
              }
            else
                mng_info->ob[object_id]->plte_length=0;
          }
      }
#endif
    /*
      Free memory.
    */
    png_destroy_read_struct(&ping,&ping_info,&end_info);
    FreeMemory(png_pixels);
    FreeMemory(scanlines);
    if (mng_type)
      {
        MngBox
          crop_box;

        /*
          Crop_box is with respect to the upper left corner of the MNG.
        */
        crop_box.left=image_box.left+mng_info->x_off[object_id];
        crop_box.right=image_box.right+mng_info->x_off[object_id];
        crop_box.top=image_box.top+mng_info->y_off[object_id];
        crop_box.bottom=image_box.bottom+mng_info->y_off[object_id];
        crop_box=mng_minimum_box(crop_box,clip);
        crop_box=mng_minimum_box(crop_box,frame);
        crop_box=mng_minimum_box(crop_box,mng_info->object_clip[object_id]);
        if ((crop_box.left != (image_box.left+mng_info->x_off[object_id])) ||
            (crop_box.right != (image_box.right+mng_info->x_off[object_id])) ||
            (crop_box.top != (image_box.top+mng_info->y_off[object_id])) ||
            (crop_box.bottom != (image_box.bottom+mng_info->y_off[object_id])))
          {
            if ((crop_box.left < crop_box.right) &&
                (crop_box.top < crop_box.bottom))
              {
                Image
                  *p;

                RectangleInfo
                  crop_info;

                /*
                  Crop_info is with respect to the upper left corner of
                  the image.
                */
                crop_info.x=crop_box.left-mng_info->x_off[object_id];
                crop_info.y=crop_box.top-mng_info->y_off[object_id];
                crop_info.width=(unsigned int)
                  (crop_box.right-crop_box.left);
                crop_info.height=(unsigned int)
                  (crop_box.bottom-crop_box.top);
                image->page_info.width=image->columns;
                image->page_info.height=image->rows;
                image->page_info.x=0;
                image->page_info.y=0;
                image->orphan=True;
                p=CropImage(image,&crop_info);
                if (p != (Image *) NULL)
                  {
                    p->orphan=True;
                    image->columns=p->columns;
                    image->rows=p->rows;
                    p->orphan=True;
                    p->file=(FILE *) NULL;
                    p->blob_info.mapped=False;
                    DestroyImage(p);
                    image->page_info.width=image->columns;
                    image->page_info.height=image->rows;
                    image->page_info.x=crop_box.left;
                    image->page_info.y=crop_box.top;
                  }
              }
            else
              {
                /*
                  No pixels in crop area.  The MNG spec still requires
                  a layer, though, so make a single transparent pixel in
                  the top left corner.
                */
                image->columns=(unsigned int) 1;
                image->rows=(unsigned int) 1;
                image->matte=True;
                image->colors=2;
                SetTransparentImage(image);
                image->page_info.width=1;
                image->page_info.height=1;
                image->page_info.x=0;
                image->page_info.y=0;
              }
          }
        if (image_info->coalesce_frames)
          {
            image->background_color = mng_background_color;
            MNGCoalesce(image);
          }
#ifndef PNG_READ_EMPTY_PLTE_SUPPORTED
        image=mng_info->image;
#endif
      }
  } while (Latin1Compare(image_info->magick,"MNG") == 0);
  if (image_info->insert_backdrops && !image_found && (mng_width > 0) &&
      (mng_height > 0))
    {
      /*
        Insert a background layer if nothing else was found.
      */
      if (image->pixels != (PixelPacket *) NULL)
        {
          /*
            Allocate next image structure.
          */
          AllocateNextImage(image_info,image);
          if (image->next == (Image *) NULL)
            {
              DestroyImages(image);
              MngInfoFreeStruct(mng_info,&have_mng_structure);
              return((Image *) NULL);
            }
          image=image->next;
          image->next->blob_info=image->blob_info;
        }
      image->columns=mng_width;
      image->rows=mng_height;
      image->page_info.width=mng_width;
      image->page_info.height=mng_height;
      image->page_info.x=0;
      image->page_info.y=0;
      image->background_color=mng_background_color;
      image->matte=False;
      SetImage(image);
      image_found++;
    }
  if (ticks_per_second)
     image->delay=(unsigned int) (100*final_delay/ticks_per_second);
  else
     image->delay=(unsigned int) final_delay;
  while (image->previous != (Image *) NULL)
  {
    image_count++;
    if (image_count>10*image_found)
      {
        MagickWarning(DelegateWarning,
          "Linked list is corrupted, beginning of list not found",
          image_info->filename);
        return((Image *) NULL);
      }
    image=image->previous;
    if (image->next == (Image *) NULL)
      MagickWarning(DelegateWarning,
       "Linked list is corrupted; next_image is NULL",image_info->filename);
  }
  if (ticks_per_second && image_found > 1 && image->next == (Image *) NULL)
    MagickWarning(DelegateWarning,
     "image->next for first image is NULL but shouldn't be.",
     image_info->filename);
  if (!image_found)
    {
      MagickWarning(DelegateWarning,
        "No visible images in file",image_info->filename);
      if (image != (Image *) NULL)
        DestroyImages(image);
      MngInfoFreeStruct(mng_info,&have_mng_structure);
      return((Image *) NULL);
    }
  CloseBlob(image);
  MngInfoFreeStruct(mng_info,&have_mng_structure);
  have_mng_structure=False;
  return(image);
}
#else
Export Image *ReadPNGImage(const ImageInfo *image_info)
{
  MagickWarning(MissingDelegateWarning,"PNG library is not available",
    image_info->filename);
  return((Image *) NULL);
}
#endif

#if defined(HasPNG)
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e P N G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WritePNGImage writes an image in the Portable Network Graphics
%  encoded image format.
%
%  MNG support written by Glenn Randers-Pehrson, randeg@alum.rpi.edu
%
%  The format of the WritePNGImage method is:
%
%      unsigned int WritePNGImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WritePNGImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to an Image structure.
%
%
%  To do (as of version 4.2.9, August 29, 1999 -- glennrp -- see also
%    "To do" under ReadPNGImage):
%
%    Preserve all unknown and not-yet-handled known chunks found in input
%    PNG file and copy them  into output PNG files according to the PNG
%    copying rules.
%
%    Write the iCCP chunk at MNG level when (image->color_profile.length > 0)
%    Write the iCCP chunk at PNG level if appropriate, when libpng has
%    implemented iCCP.
%
%    Improve selection of color type (use indexed-colour or indexed-colour
%    with tRNS when 256 or fewer unique RGBA values are present).
%
%    Figure out what to do with "dispose=<restore-to-previous>" (dispose==3)
%    This will be complicated if we limit ourselves to generating MNG-LC
%    files.  For now we ignore disposal method 3 and simply overlay the next
%    image on it.
%
%    Check for identical PLTE's or PLTE/tRNS combinations and use a
%    global MNG PLTE or PLTE/tRNS combination when appropriate.
%    [mostly done 15 June 1999 but still need to take care of tRNS]
%
%    Put the transparent color first in the PLTE of indexed-color PNGs.
%
%    Check for identical sRGB and replace with a global sRGB (and remove
%    gAMA/cHRM if sRGB is found; check for identical gAMA/cHRM and
%    replace with global gAMA/cHRM (or with sRGB if appropriate; replace
%    local gAMA/cHRM with local sRGB if appropriate).
%
%    Check for identical sBIT chunks and write global ones.
%
%    Provide option to skip writing the signature tEXt chunks.
%
%    Use signatures to detect identical objects and reuse the first
%    instance of such objects instead of writing duplicate objects.
%
%    Use a smaller-than-32k value of compression window size when
%    appropriate.
%
%    Encode JNG datastreams.
%
%    Provide an option to force LC files (to ensure exact framing rate)
%    instead of VLC.
%
%    Provide an option to force VLC files instead of LC, even when offsets
%    are present.  This will involve expanding the embedded images with a
%    transparent region at the top and/or left.
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static void PNGLong(png_bytep p,png_uint_32 value)
{
  *p++=(png_byte) ((value >> 24) & 0xff);
  *p++=(png_byte) ((value >> 16) & 0xff);
  *p++=(png_byte) ((value >> 8) & 0xff);
  *p++=(png_byte) (value & 0xff);
}

static void PNGShort(png_bytep p,png_uint_16 value)
{
  *p++=(png_byte) ((value >> 8) & 0xff);
  *p++=(png_byte) (value & 0xff);
}

static void PNGType(png_bytep p,png_bytep type)
{
  (void) memcpy(p,type,4*sizeof(png_byte));
}

static void WriteTextChunk(const Image *image, const ImageInfo *image_info,
  png_struct *ping, png_info *ping_info,char *keyword,char *value)
{
#if (PNG_LIBPNG_VER > 10005)
  png_textp
    text;

  text=(png_textp) png_malloc(ping,(png_uint_32) sizeof(png_text));
  text[0].key=keyword;
  text[0].text=value;
  text[0].text_length=Extent(value);
  text[0].compression=
    image_info->compression == NoCompression ||
    (image_info->compression == UndefinedCompression &&
    text[0].text_length < 128) ? -1 : 0;
  png_set_text(ping,ping_info,text,1);
  png_free(ping,text);
#else
  /* Work directly with ping_info struct; png_set_text before libpng version
   * 1.0.5a is leaky */
  register int
    i;

  if (ping_info->num_text == 0)
    {
      ping_info->text=(png_text *) AllocateMemory(256*sizeof(png_text));
      if (ping_info->text == (png_text *) NULL)
        MagickWarning(ResourceLimitWarning,"Memory allocation failed",
          image->filename);
    }
  i=ping_info->num_text++;
  if (i > 255)
    MagickWarning(ResourceLimitWarning,
      "Cannot write more than 256 PNG text chunks",image->filename);
  ping_info->text[i].key=keyword;
  ping_info->text[i].text=value;
  ping_info->text[i].text_length=Extent(value);
  ping_info->text[i].compression=
    image_info->compression == NoCompression ||
    (image_info->compression == UndefinedCompression &&
    ping_info->text[i].text_length < 128) ? -1 : 0;
#endif
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

Export unsigned int WritePNGImage(const ImageInfo *image_info,Image *image)
{
  int
    all_images_are_gray,
    equal_backgrounds,
    equal_chrms,
    equal_gammas,
    equal_palettes,
    equal_physs,
    equal_srgbs,
    framing_mode,
    have_write_global_chrm,
    have_write_global_gama,
    have_write_global_plte,
    have_write_global_srgb,
    image_count,
    need_defi,
    need_fram,
    need_iterations,
    need_local_plte,
    need_matte,
    old_framing_mode,
    rowbytes,
    save_image_depth,
    use_global_plte,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p;

  RectangleInfo
    page_info;

  png_info
    *ping_info;

  png_struct
    *ping;

  unsigned char
    chunk[800],
    *png_pixels,
    **scanlines;

  unsigned int
    delay,
    final_delay,
    initial_delay,
    matte,
#ifdef MNG_LEVEL
    mng_level,
#endif
    scene,
    status,
    ticks_per_second;

  /*
    Open image file.
  */
#ifdef MNG_LEVEL
  mng_level=MNG_LEVEL;
#endif
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  use_global_plte=False;
  page_info.width=0;
  page_info.height=0;
  page_info.x=0;
  page_info.y=0;
  have_write_global_plte=False;
  need_local_plte=True;
  have_write_global_srgb=False;
  have_write_global_gama= False;
  have_write_global_chrm=False;
  need_defi=False;
  need_fram=False;
  need_matte=False;
  framing_mode=1;
  old_framing_mode=1;
  if (image_info->adjoin)
    {
      Image
        *next_image;

      unsigned int
        need_geom;

      unsigned short
        red,
        green,
        blue;
      /*
        Determine image bounding box.
      */
      need_geom=True;
      if (image_info->page != (char *) NULL)
        {
          (void) ParseGeometry(image_info->page,&page_info.x,&page_info.y,
            &page_info.width,&page_info.height);
          need_geom=False;
        }
      /*
        Check all the scenes.
      */
      initial_delay=image->delay;
      need_iterations=False;
      need_local_plte=False;
      equal_backgrounds=True;
      all_images_are_gray=True;
      equal_chrms=image->chromaticity.white_point.x != 0.0;
      equal_gammas=True;
      equal_srgbs=True;
      equal_palettes=False;
      equal_physs=True;
      image_count=0;
      for (next_image=image; next_image != (Image *) NULL; )
      {
        page_info=next_image->page_info;
        if (need_geom)
          {
            if ((next_image->columns+page_info.x) > page_info.width)
              page_info.width=next_image->columns+page_info.x;
            if ((next_image->rows+page_info.y) > page_info.height)
              page_info.height=next_image->rows+page_info.y;
          }
        if (page_info.x || page_info.y)
          need_defi=True;
        if (next_image->matte)
          need_matte=True;
        if (next_image->dispose >= 2)
          if (next_image->matte || page_info.x || page_info.y ||
              ((next_image->columns < page_info.width) &&
               (next_image->rows < page_info.height)))
            need_fram=True;
        if (next_image->iterations)
          need_iterations=True;
        final_delay=next_image->delay;
        if (final_delay != initial_delay)
          need_fram=1;
#ifdef PNG_WRITE_EMPTY_PLTE_SUPPORTED
        /*
          check for global palette possibility.
        */
        if (!IsGrayImage(image))
          all_images_are_gray=False;
        equal_palettes=PalettesAreEqual(image_info,image,next_image);
        if (!use_global_plte)
          use_global_plte=equal_palettes;
        if (!need_local_plte)
          need_local_plte=!equal_palettes;
#else
        need_local_plte=True;
#endif
        if (next_image->next != (Image *) NULL)
          {
            if (next_image->background_color.red !=
                next_image->next->background_color.red ||
                next_image->background_color.green !=
                next_image->next->background_color.green ||
                next_image->background_color.blue !=
                next_image->next->background_color.blue)
              equal_backgrounds=False;
            if (next_image->gamma != next_image->next->gamma)
              equal_gammas=False;
            if (next_image->rendering_intent != 
                next_image->next->rendering_intent)
              equal_srgbs=False;
            if ((next_image->units != next_image->next->units) ||
                (next_image->x_resolution != next_image->next->x_resolution) ||
                (next_image->y_resolution != next_image->next->y_resolution))
              equal_physs=False;
            if (equal_chrms)
              {
                if (next_image->chromaticity.red_primary.x !=
                    next_image->next->chromaticity.red_primary.x ||
                    next_image->chromaticity.red_primary.y !=
                    next_image->next->chromaticity.red_primary.y ||
                    next_image->chromaticity.green_primary.y !=
                    next_image->next->chromaticity.green_primary.x ||
                    next_image->chromaticity.blue_primary.x !=
                    next_image->next->chromaticity.green_primary.y ||
                    next_image->chromaticity.blue_primary.x !=
                    next_image->next->chromaticity.blue_primary.x ||
                    next_image->chromaticity.blue_primary.y !=
                    next_image->next->chromaticity.blue_primary.y ||
                    next_image->chromaticity.white_point.x !=
                    next_image->next->chromaticity.white_point.x ||
                    next_image->chromaticity.white_point.y !=
                    next_image->next->chromaticity.white_point.y)
                  equal_chrms=False;
              }
          }
        image_count++;
        next_image=next_image->next;
      }
      if (image_count < 2)
        {
          equal_backgrounds=False;
          equal_chrms=False;
          equal_gammas=False;
          equal_srgbs=False;
          equal_physs=False;
          use_global_plte=False;
          need_local_plte=True;
          need_iterations=False;
        }
     if (!need_fram)
       {
         /*
           Only certain framing rates 100/n are exactly representable without
           the FRAM chunk but we'll allow some slop in VLC files
         */
         if (final_delay == 0)
           {
             if (need_iterations)
               {
                 /*
                   It's probably a GIF with loop; don't run it *too* fast.
                 */
                 final_delay=10;
                 MagickWarning(DelegateWarning,
                   "input has zero delay between all frames; assuming 10 cs",
                   image->filename);
               }
             else
               ticks_per_second=0;
           }
         if (final_delay > 0)
           ticks_per_second=100/final_delay;
         if (final_delay > 50)
           ticks_per_second=2;
         if (final_delay > 75)
           ticks_per_second=1;
         if (final_delay > 125)
           need_fram=True;
         if (need_defi && final_delay > 2 && (final_delay != 4) &&
             (final_delay != 5) && (final_delay != 10) && (final_delay != 20) &&
             (final_delay != 25) && (final_delay != 50) && (final_delay != 100))
           need_fram=True;  /* make it exact; we cannot have VLC anyway */
       }
     if (need_fram)
        ticks_per_second = 100;
     /*
        If pseudocolor, we should also check to see if all the
        palettes are identical and write a global PLTE if they are.
        ../glennrp Feb 99.
     */
     /*
        Write the MNG version 0.96 signature and MHDR chunk.
     */
     (void) WriteBlob(image,8,"\212MNG\r\n\032\n");
     MSBFirstWriteLong(image,28L);  /* chunk data length = 28 */
     PNGType(chunk,mng_MHDR);
     PNGLong(chunk+4,page_info.width);
     PNGLong(chunk+8,page_info.height);
     PNGLong(chunk+12,ticks_per_second);
     PNGLong(chunk+16,0L);  /* layer count = unknown */
     PNGLong(chunk+20,0L);  /* frame count = unknown */
     PNGLong(chunk+24,0L);  /* play time = unknown   */
     if (need_matte)
       {
         if (need_defi || need_fram || use_global_plte)
           PNGLong(chunk+28,11L);    /* simplicity =  LC */
         else
           PNGLong(chunk+28,9L);    /* simplicity = VLC */
       }
     else
       {
         if (need_defi || need_fram || use_global_plte)
           PNGLong(chunk+28,3L);    /* simplicity =  LC, no transparency */
         else
           PNGLong(chunk+28,1L);    /* simplicity = VLC, no transparency */
       }
     (void) WriteBlob(image,32,(char *) chunk);
     MSBFirstWriteLong(image,crc32(0,chunk,32));
     if ((image->previous == (Image *) NULL) &&
         (image->next != (Image *) NULL) && (image->iterations != 1))
       {
         /*
           Write MNG TERM chunk
         */
         MSBFirstWriteLong(image,10L);  /* data length = 10 */
         PNGType(chunk,mng_TERM);
         chunk[4]=3;  /* repeat animation */
         chunk[5]=0;  /* show last frame when done */
         PNGLong(chunk+6,(png_uint_32) (ticks_per_second*final_delay/100));
         if (image->iterations == 0)
           PNGLong(chunk+10,PNG_MAX_UINT);
         else
           PNGLong(chunk+10,(png_uint_32) image->iterations);
         (void) WriteBlob(image,14,(char *) chunk);
         MSBFirstWriteLong(image,crc32(0,chunk,14));
       }
     /*
       To do: check for cHRM+gAMA == sRGB, and write sRGB instead.
     */
     if ((image_info->colorspace==sRGBColorspace || image->rendering_intent) &&
          equal_srgbs)
       {
         /*
           Write MNG sRGB chunk
         */
         MSBFirstWriteLong(image,1L);
         PNGType(chunk,mng_sRGB);
         chunk[4]=image->rendering_intent+1;
         (void) WriteBlob(image,5,(char *) chunk);
         MSBFirstWriteLong(image,crc32(0,chunk,5));
         have_write_global_srgb=True;
       }
     else
       {
         if (image->gamma && equal_gammas)
           {
             /*
                Write MNG gAMA chunk
             */
             MSBFirstWriteLong(image,4L);
             PNGType(chunk,mng_gAMA);
             PNGLong(chunk+4,(unsigned long) (100000*image->gamma+0.5));
             (void) WriteBlob(image,8,(char *) chunk);
             MSBFirstWriteLong(image,crc32(0,chunk,8));
             have_write_global_gama=True;
           }
         if (equal_chrms)
           {
             /*
                Write MNG cHRM chunk
             */
             MSBFirstWriteLong(image,32L);
             PNGType(chunk,mng_cHRM);
             PNGLong(chunk+4,(unsigned long)
               (100000*image->chromaticity.white_point.x+0.5));
             PNGLong(chunk+8,(unsigned long)
               (100000*image->chromaticity.white_point.y+0.5));
             PNGLong(chunk+12,(unsigned long)
               (100000*image->chromaticity.red_primary.x+0.5));
             PNGLong(chunk+16,(unsigned long)
               (100000*image->chromaticity.red_primary.y+0.5));
             PNGLong(chunk+20,(unsigned long)
               (100000*image->chromaticity.green_primary.x+0.5));
             PNGLong(chunk+24,(unsigned long)
               (100000*image->chromaticity.green_primary.y+0.5));
             PNGLong(chunk+28,(unsigned long)
               (100000*image->chromaticity.blue_primary.x+0.5));
             PNGLong(chunk+32,(unsigned long)
               (100000*image->chromaticity.blue_primary.y+0.5));
             (void) WriteBlob(image,36,(char *) chunk);
             MSBFirstWriteLong(image,crc32(0,chunk,36));
             have_write_global_chrm=True;
           }
       }
     if (image->x_resolution && image->y_resolution && equal_physs)
       {
         /*
            Write MNG pHYs chunk
         */
         MSBFirstWriteLong(image,9L);
         PNGType(chunk,mng_pHYs);
         if (image->units == PixelsPerInchResolution)
           {
             PNGLong(chunk+4,(unsigned long)
               (image->x_resolution*100.0/2.54+0.5));
             PNGLong(chunk+8,(unsigned long)
               (image->y_resolution*100.0/2.54+0.5));
             chunk[12]=1;
           }
         else
           {
             if (image->units==PixelsPerCentimeterResolution)
               {
                 PNGLong(chunk+4,(unsigned long)
                   (image->x_resolution*100.0+0.5));
                 PNGLong(chunk+8,(unsigned long)
                   (image->y_resolution*100.0+0.5));
                 chunk[12]=1;
               }
             else
               {
                 PNGLong(chunk+4,(unsigned long) (image->x_resolution+0.5));
                 PNGLong(chunk+8,(unsigned long) (image->y_resolution+0.5));
                 chunk[12]=0;
               }
           }
         (void) WriteBlob(image,13,(char *) chunk);
         MSBFirstWriteLong(image,crc32(0,chunk,13));
       }
     /*
       Write MNG BACK chunk and global bKGD chunk
     */
     MSBFirstWriteLong(image,6L);
     PNGType(chunk,mng_BACK);
     red=(unsigned short) UpScale(image->background_color.red);
     green=(unsigned short) UpScale(image->background_color.green);
     blue=(unsigned short) UpScale(image->background_color.blue);
     PNGShort(chunk+4,red);
     PNGShort(chunk+6,green);
     PNGShort(chunk+8,blue);
     (void) WriteBlob(image,10,(char *) chunk);
     MSBFirstWriteLong(image,crc32(0,chunk,10));
     if (equal_backgrounds)
       {
         MSBFirstWriteLong(image,6L);
         PNGType(chunk,mng_bKGD);
         (void) WriteBlob(image,10,(char *) chunk);
         MSBFirstWriteLong(image,crc32(0,chunk,10));
       }
     if (!need_local_plte && IsPseudoClass(image) && !all_images_are_gray)
       {
         long
           data_length;

         /*
           Write MNG PLTE chunk
         */
         data_length=3*image->colors;
         MSBFirstWriteLong(image,data_length);
         PNGType(chunk,mng_PLTE);
         for (i=0; i < (int) image->colors; i++)
         {
           chunk[4+i*3]=
             (unsigned char) DownScale(image->colormap[i].red)&0xff;
           chunk[5+i*3]=
             (unsigned char) DownScale(image->colormap[i].green)&0xff;
           chunk[6+i*3]=
             (unsigned char) DownScale(image->colormap[i].blue)&0xff;
         }
         (void) WriteBlob(image,data_length+4,(char *) chunk);
         MSBFirstWriteLong(image,crc32(0,chunk,data_length+4));
         have_write_global_plte=True;
       }
    }
  scene=0;
  delay=0;
  equal_palettes=False;
  do
  {
    png_colorp
       palette;
    /*
      If we aren't using a global palette for the entire MNG, check to
      see if we can use one for two or more consecutive images.
    */
    if (need_local_plte && use_global_plte && !all_images_are_gray)
      {
        if (IsPseudoClass(image))
          {
            /*
              When equal_palettes is true, this image has the same palette
              as the previous PseudoClass image
            */
            have_write_global_plte=equal_palettes;
            equal_palettes=PalettesAreEqual(image_info,image,image->next);
            if (equal_palettes && !have_write_global_plte)
              {
                /*
                  Write MNG PLTE chunk
                */
                long
                  data_length;

                data_length=3*image->colors;
                MSBFirstWriteLong(image,data_length);
                PNGType(chunk,mng_PLTE);
                for (i=0; i < (int) image->colors; i++)
                {
                  chunk[4+i*3]=
                    (unsigned char) DownScale(image->colormap[i].red) & 0xff;
                  chunk[5+i*3]=
                    (unsigned char) DownScale(image->colormap[i].green) & 0xff;
                  chunk[6+i*3]=
                    (unsigned char) DownScale(image->colormap[i].blue) & 0xff;
                }
                (void) WriteBlob(image,data_length+4,(char *) chunk);
                MSBFirstWriteLong(image,crc32(0,chunk,data_length+4));
                have_write_global_plte=True;
              }
          }
        else
          have_write_global_plte=False;
      }
    if (need_defi)
      {
        int
          previous_x,
          previous_y;

        if (scene)
          {
            previous_x=page_info.x;
            previous_y=page_info.y;
          }
        else
          {
            previous_x=0;
            previous_y=0;
          }
        page_info=image->page_info;
        if ((page_info.x !=  previous_x) || (page_info.y != previous_y))
          {
             MSBFirstWriteLong(image,12L);  /* data length = 12 */
             PNGType(chunk,mng_DEFI);
             chunk[4]=0; /* object 0 MSB */
             chunk[5]=0; /* object 0 LSB */
             chunk[6]=0; /* visible  */
             chunk[7]=0; /* abstract */
             PNGLong(chunk+8,page_info.x);
             PNGLong(chunk+12,page_info.y);
             (void) WriteBlob(image,16,(char *) chunk);
             MSBFirstWriteLong(image,crc32(0,chunk,16));
          }
      }
    /*
      Allocate the PNG structures
    */
    TransformRGBImage(image,RGBColorspace);
#ifdef PNG_USER_MEM_SUPPORTED
    ping = png_create_write_struct_2(PNG_LIBPNG_VER_STRING,(void *) NULL,
      PNGErrorHandler,PNGWarningHandler,(void *) NULL,
      (png_malloc_ptr) png_IM_malloc,(png_free_ptr) png_IM_free);
#else
    ping=png_create_write_struct(PNG_LIBPNG_VER_STRING,(void *) NULL,
      PNGErrorHandler,PNGWarningHandler);
#endif
    if (ping == (png_struct *) NULL)
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    ping_info=png_create_info_struct(ping);
    if (ping_info == (png_info *) NULL)
      {
        png_destroy_write_struct(&ping,(png_info **) NULL);
        WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
      }
    png_set_write_fn(ping,image,png_put_data,png_flush_data);
    png_pixels=(unsigned char *) NULL;
    scanlines=(unsigned char **) NULL;
    if (setjmp(ping->jmpbuf))
      {
        /*
          PNG write failed.
        */
        png_destroy_write_struct(&ping,&ping_info);
        if (scanlines != (unsigned char **) NULL)
          FreeMemory(scanlines);
        if (png_pixels != (unsigned char *) NULL)
          FreeMemory(png_pixels);
        CloseBlob(image);
        return(False);
      }
    /*
      Prepare PNG for writing.
    */
#ifdef PNG_WRITE_EMPTY_PLTE_SUPPORTED
    png_permit_empty_plte(ping,True);
#endif
    ping_info->width=image->columns;
    ping_info->height=image->rows;
    save_image_depth=image->depth;
    ping_info->bit_depth=save_image_depth;
#if (QuantumDepth == 16)
    if (ping_info->bit_depth == 16)
      {
        int
          ok_to_reduce;

        /*
          Determine if bit depth can be reduced from 16 to 8.
        */
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            ok_to_reduce=((((p->red >> 8) & 0xff) == (p->red & 0xff)) &&
              (((p->green >> 8) & 0xff) == (p->green & 0xff)) &&
              (((p->blue >> 8) & 0xff) == (p->blue & 0xff)) &&
              (((!image->matte ||
              ((p->opacity >> 8) & 0xff) == (p->opacity & 0xff)))));
            if (!ok_to_reduce)
              break;
            p++;
          }
          if (x < (int) image->columns)
            break;
        }
        if (ok_to_reduce)
          {
            ping_info->bit_depth=8;
            image->depth=8;
          }
      }
#endif
    if ((image->x_resolution != 0) && (image->y_resolution != 0) &&
        (!image_info->adjoin || !equal_physs))
      {
        png_uint_32
          res_x,
          res_y;

        int
          unit_type;

        if (image->units == PixelsPerInchResolution)
          {
            unit_type=PNG_RESOLUTION_METER;
            res_x=(png_uint_32) (100.0*image->x_resolution/2.54);
            res_y=(png_uint_32) (100.0*image->y_resolution/2.54);
          }
        else if (image->units == PixelsPerCentimeterResolution)
          {
            unit_type=PNG_RESOLUTION_METER;
            res_x=(png_uint_32) (100.0*image->x_resolution);
            res_y=(png_uint_32)
              (100.0*image->y_resolution);
          }
        else
          {
            unit_type=PNG_RESOLUTION_UNKNOWN;
            res_x=(png_uint_32) image->x_resolution;
            res_y=(png_uint_32) image->y_resolution;
          }
         png_set_pHYs(ping, ping_info, res_x, res_y, unit_type);
      }
    if (image->matte && (!image_info->adjoin || !equal_backgrounds))
      {
        png_color_16
           background;

        background.red=
          (unsigned short) DownScale(image->background_color.red);
        background.green=
          (unsigned short) DownScale(image->background_color.green);
        background.blue=
          (unsigned short) DownScale(image->background_color.blue);
        background.gray=
          (unsigned short) DownScale(Intensity(image->background_color));
        background.index=background.gray;

        png_set_bKGD(ping, ping_info, &background);
      }
    /*
      Select the color type.
    */
    if (IsMonochromeImage(image))
      {
        if (image->matte)
          ping_info->bit_depth=8;
        else
          ping_info->bit_depth=1;
      }
    ping_info->color_type=PNG_COLOR_TYPE_RGB;
    matte=image->matte;
    if (matte)
      {
        ping_info->color_type=PNG_COLOR_TYPE_GRAY_ALPHA;
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            if (!IsGray(*p))
              {
                ping_info->color_type=PNG_COLOR_TYPE_RGB_ALPHA;
                break;
              }
            p++;
          }
        }
        /*
          Determine if there is any transparent color.
        */
        for (y=0; y < (int) image->rows; y++)
        {
          p=GetPixelCache(image,0,y,image->columns,1);
          if (p == (PixelPacket *) NULL)
            break;
          for (x=0; x < (int) image->columns; x++)
          {
            if (p->opacity != Opaque)
              break;
            p++;
          }
          if (x < (int) image->columns)
            break;
        }
        if ((y == (int) image->rows) && (x == (int) image->columns))
          { 
            /*
              No transparent pixels are present.  Change 4 or 6 to 0 or 2,
              and do not set the PNG_INFO_tRNS flag in ping_info->valid.
            */
            image->matte=False;
            ping_info->color_type&=0x03;
          }
        else
          {
            ping_info->valid|=PNG_INFO_tRNS;
            ping_info->trans_values.red=p->red;
            ping_info->trans_values.green=p->green;
            ping_info->trans_values.blue=p->blue;
            ping_info->trans_values.gray=(unsigned short) Intensity(*p);
            ping_info->trans_values.index=(unsigned short)
              DownScale(p->opacity);
          }
        if (ping_info->valid & PNG_INFO_tRNS)
          {
            /*
              Determine if there is one and only one transparent color
              and if so if it is fully transparent.
            */
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              x=0;
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                if (p->opacity != Opaque)
                  {
                    if (!ColorMatch(ping_info->trans_values,*p,0))
                       break;  /* Can't use RGB + tRNS for multiple transparent
                                  colors.  */
                    if (p->opacity)
                       break;  /* Can't use RGB + tRNS for semitransparency. */
                  }
                 else
                  {
                    if (ColorMatch(ping_info->trans_values,*p,0))
                        break; /* Can't use RGB + tRNS when another pixel
                                  having the same RGB samples is transparent. */
                  }
              p++;
              }
              if (x < (int) image->columns)
                 break;
            }
            if (x < (int) image->columns)
                ping_info->valid&=(~PNG_INFO_tRNS);
          }
        if (ping_info->valid & PNG_INFO_tRNS)
          {
            ping_info->color_type &= 0x03;  /* changes 4 or 6 to 0 or 2 */
            if (save_image_depth == 16 && image->depth == 8)
              {
                ping_info->trans_values.red&=0xff;
                ping_info->trans_values.green&=0xff;
                ping_info->trans_values.blue&=0xff;
                ping_info->trans_values.gray&=0xff;
              }
          }
      }
    matte=image->matte;
    if (ping_info->valid & PNG_INFO_tRNS)
      image->matte=False;
    if (IsGrayImage(image) && (!image->matte || image->depth >= 8))
      {
        if (image->matte)
          {
            ping_info->color_type=PNG_COLOR_TYPE_GRAY_ALPHA;
          }
        else
          {
            ping_info->color_type=PNG_COLOR_TYPE_GRAY;
            if (save_image_depth == 16 && image->depth == 8)
              ping_info->trans_values.gray*=0x0101;
          }
      }
    else
      if (IsPseudoClass(image))
      {
        if (image->depth <= 8)
          {
            int
               num_palette;
            if (matte)
               ping_info->valid|=PNG_INFO_tRNS;
            /*
              Set image palette.
            */
            ping_info->color_type=PNG_COLOR_TYPE_PALETTE;
            ping_info->valid|=PNG_INFO_PLTE;
            if (have_write_global_plte && !matte)
              png_set_PLTE(ping, ping_info, NULL, 0);
            else
              {
#if (PNG_LIBPNG_VER > 10005)
                CompressColormapTransFirst(image);
#endif
                num_palette=image->colors;
                palette=(png_color *)
                  AllocateMemory(image->colors*sizeof(png_color));
                if (palette == (png_color *) NULL)
                  WriterExit(ResourceLimitWarning,"Memory allocation failed",
                    image);
                for (i=0; i < (int) image->colors; i++)
                {
                  palette[i].red=
                    (unsigned short) DownScale(image->colormap[i].red);
                  palette[i].green=
                    (unsigned short) DownScale(image->colormap[i].green);
                  palette[i].blue=
                    (unsigned short) DownScale(image->colormap[i].blue);
                }
                png_set_PLTE(ping, ping_info, palette, num_palette);
#if (PNG_LIBPNG_VER >= 10100)
                FreeMemory(palette);
#endif
              }
            ping_info->bit_depth=1;
            while ((1 << ping_info->bit_depth) < (int) image->colors)
              ping_info->bit_depth<<=1;
            /*
              Identify which colormap entry is transparent.
            */
            ping_info->trans=(unsigned char *)
              AllocateMemory(image->colors*sizeof(unsigned char));
            if (ping_info->trans == (unsigned char *) NULL)
              WriterExit(ResourceLimitWarning,"Memory allocation failed",
                image);
            for (i=0; i < (int) image->colors; i++)
               ping_info->trans[i]=(png_byte) DownScale(Opaque);
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                if (p->opacity != Opaque)
                  {
                   unsigned short
                      index;
                   index=image->indexes[x];
                   assert(index < image->colors);
                   ping_info->trans[index]=(png_byte) DownScale(p->opacity);
                  }
                p++;
              }
            }
            ping_info->num_trans=0;
            for (i=0; i < (int) image->colors; i++)
              if (ping_info->trans[i] != (png_byte) DownScale(Opaque))
                ping_info->num_trans=i+1;
            if (ping_info->num_trans == 0)
                ping_info->valid&=(~PNG_INFO_tRNS);
            if (!(ping_info->valid & PNG_INFO_tRNS))
              ping_info->num_trans=0;
            /*
              Identify which colormap entry is the background color.
            */
            for (i=0; i < (int) (image->colors-1); i++)
              if (ColorMatch(ping_info->background,image->colormap[i],0))
                break;
            ping_info->background.index=(unsigned short) i;
          }
      }
    else
      {
        if (image->depth < 8)
            image->depth = 8;
        if (save_image_depth == 16 && image->depth == 8)
          {
            ping_info->trans_values.red*=0x0101;
            ping_info->trans_values.green*=0x0101;
            ping_info->trans_values.blue*=0x0101;
            ping_info->trans_values.gray*=0x0101;
          }
      }
    /* image->matte=matte; */
#if defined(PNG_WRITE_sRGB_SUPPORTED)
    if (!have_write_global_srgb &&
        ((image->rendering_intent != UndefinedIntent) ||
        image_info->colorspace == sRGBColorspace))
      {
        /*
          Note image rendering intent.
        */
        png_set_sRGB(ping, ping_info, (int) image->rendering_intent+1);
        png_set_gAMA(ping, ping_info, 0.45455);
      }
    if (!image_info->adjoin || (!ping_info->valid&PNG_INFO_sRGB))
#endif
      {
        if (!have_write_global_gama && image->gamma != 0.0)
          {
            /*
              Note image gamma.
              To do: check for cHRM+gAMA == sRGB, and write sRGB instead. 
            */
            png_set_gAMA(ping, ping_info, image->gamma);
          }
        if (!have_write_global_chrm && image->chromaticity.white_point.x != 0.0)
          {
            /*
              Note image chromaticity.
              To do: check for cHRM+gAMA == sRGB, and write sRGB instead. 
            */
         png_set_cHRM(ping, ping_info,
            image->chromaticity.white_point.x,
            image->chromaticity.white_point.y,
            image->chromaticity.red_primary.x,
            image->chromaticity.red_primary.y,
            image->chromaticity.green_primary.x,
            image->chromaticity.green_primary.y,
            image->chromaticity.blue_primary.x,
            image->chromaticity.blue_primary.y);
         }
      }
    ping_info->interlace_type=image_info->interlace != NoInterlace;
    /*
      Initialize compression level and filtering.
    */
    png_set_compression_level(ping,Min(image_info->quality/10,9));
    if ((image_info->quality % 10) > 5)
      png_set_filter(ping,PNG_FILTER_TYPE_BASE,PNG_ALL_FILTERS);
    else
      if ((image_info->quality % 10) != 5)
        png_set_filter(ping,PNG_FILTER_TYPE_BASE,image_info->quality % 10);
      else
        if ((ping_info->color_type == PNG_COLOR_TYPE_GRAY) ||
            (ping_info->color_type == PNG_COLOR_TYPE_PALETTE) ||
            (image_info->quality < 50))
          png_set_filter(ping,PNG_FILTER_TYPE_BASE,PNG_NO_FILTERS);
        else
          png_set_filter(ping,PNG_FILTER_TYPE_BASE,PNG_ALL_FILTERS);
    if (need_fram && image_info->adjoin && ((image->delay != delay) ||
        (framing_mode != old_framing_mode)))
      {
        if (image->delay == delay)
          {
            /*
              Write a MNG FRAM chunk with the new framing mode.
            */
            MSBFirstWriteLong(image,1L);  /* data length = 1 */
            PNGType(chunk,mng_FRAM);
            chunk[4]=framing_mode;
            (void) WriteBlob(image,5,(char *) chunk);
            MSBFirstWriteLong(image,crc32(0,chunk,5));
          }
        else
          {
            /*
              Write a MNG FRAM chunk with the delay.
            */
            MSBFirstWriteLong(image,10L);  /* data length = 10 */
            PNGType(chunk,mng_FRAM);
            chunk[4]=framing_mode;
            chunk[5]=0;  /* frame name separator (no name) */
            chunk[6]=2;  /* flag for changing default delay */
            chunk[7]=0;  /* flag for changing frame timeout */
            chunk[8]=0;  /* flag for changing frame clipping */
            chunk[9]=0;  /* flag for changing frame sync_id */
            PNGLong(chunk+10,(png_uint_32)
              ((ticks_per_second*image->delay)/100));
            (void) WriteBlob(image,14,(char *) chunk);
            MSBFirstWriteLong(image,crc32(0,chunk,14));
            delay=image->delay;
          }
        old_framing_mode=framing_mode;
      }

    if (image_info->adjoin)
      png_set_sig_bytes(ping,8);
    png_write_info(ping,ping_info);
    png_set_packing(ping);
    /*
      Allocate memory.
    */
    rowbytes=image->columns;
    if (image->depth == 8)
      {
        if (IsGrayImage(image))
          rowbytes*=(image->matte ? 2 : 1);
        else
          if (!IsPseudoClass(image))
            rowbytes*=(image->matte ? 4 : 3);
      }
    else if (image->depth == 16)
      {
        if (IsGrayImage(image))
          rowbytes*=(image->matte ? 4 : 2);
        else
          rowbytes*=(image->matte ? 8 : 6);
      }
    png_pixels=(unsigned char *)
      AllocateMemory(rowbytes*image->rows*sizeof(Quantum));
    scanlines=(unsigned char **)
      AllocateMemory(image->rows*sizeof(unsigned char *));
    if ((png_pixels == (unsigned char *) NULL) ||
        (scanlines == (unsigned char **) NULL))
      WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
    /*
      Initialize image scanlines.
    */
    for (i=0; i < (int) image->rows; i++)
      scanlines[i]=png_pixels+(rowbytes*i);
    if (!image->matte && IsMonochromeImage(image))
      {
        /*
          Convert PseudoClass image to a PNG monochrome image.
        */
        for (y=0; y < (int) image->rows; y++)
        {
          if (!GetPixelCache(image,0,y,image->columns,1))
            break;
          (void) WritePixelCache(image,GrayQuantum,scanlines[y]);
          if (image->previous == (Image *) NULL)
            if (QuantumTick(y,image->rows))
              ProgressMonitor(SaveImageText,y,image->rows);
        }
      }
    else
      {
      if ((!image->matte || ping_info->bit_depth >= 8) && IsGrayImage(image))
        {
          for (y=0; y < (int) image->rows; y++)
          {
            if (!GetPixelCache(image,0,y,image->columns,1))
              break;
            if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
              (void) WritePixelCache(image,GrayQuantum,scanlines[y]);
            else
              (void) WritePixelCache(image,GrayOpacityQuantum,scanlines[y]);
            if (image->previous == (Image *) NULL)
              if (QuantumTick(y,image->rows))
                ProgressMonitor(SaveImageText,y,image->rows);
          }
        }
      else
        {
          if (image->depth > 8 || !IsPseudoClass(image))
            for (y=0; y < (int) image->rows; y++)
            {
              if (!GetPixelCache(image,0,y,image->columns,1))
                break;
              if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
                (void) WritePixelCache(image,GrayQuantum,scanlines[y]);
              else
                if (ping_info->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
                  (void) WritePixelCache(image,GrayOpacityQuantum,scanlines[y]);
                else
                  if (!image->matte)
                    (void) WritePixelCache(image,RGBQuantum,scanlines[y]);
                  else
                    (void) WritePixelCache(image,RGBAQuantum,scanlines[y]);
              if (image->previous == (Image *) NULL)
                if (QuantumTick(y,image->rows))
                  ProgressMonitor(SaveImageText,y,image->rows);
            }
        else
          for (y=0; y < (int) image->rows; y++)
          {
            if (!GetPixelCache(image,0,y,image->columns,1))
              break;
            if (ping_info->color_type == PNG_COLOR_TYPE_GRAY)
              (void) WritePixelCache(image,GrayQuantum,scanlines[y]);
            else
              (void) WritePixelCache(image,IndexQuantum,scanlines[y]);
            if (image->previous == (Image *) NULL)
              if (QuantumTick(y,image->rows))
                ProgressMonitor(SaveImageText,y,image->rows);
          }
       }
    }
    png_write_image(ping,scanlines);
    /*
      Generate text chunks.
    */
    {
#if (PNG_LIBPNG_VER <= 10005)
    ping_info->num_text=0;
#endif
    /*
      Write a Software tEXt chunk only in the first PNG datastream.
    */
    if (image->scene == 0)
      WriteTextChunk(image,image_info,ping,ping_info,"Software",MagickVersion);
    if (!image_info->adjoin)
      {
        SignatureImage(image);
        if (image->signature != (char *) NULL)
          WriteTextChunk(image,image_info,ping,ping_info,"Signature",
            image->signature);
        if (image->delay != 0)
          {
            char
              delay[MaxTextExtent];

            FormatString(delay,"%u",image->delay);
            WriteTextChunk(image,image_info,ping,ping_info,"Delay",delay);
          }
        if (image->scene != 0)
          {
            char
              scene[MaxTextExtent];

            FormatString(scene,"%u",image->scene);
            WriteTextChunk(image,image_info,ping,ping_info,"Scene",scene);
          }
      }
    if (image->label != (char *) NULL)
      WriteTextChunk(image,image_info,ping,ping_info,"Title",image->label);
    if (image->montage != (char *) NULL)
      WriteTextChunk(image,image_info,ping,ping_info,"Montage",image->montage);
    if (image->directory != (char *) NULL)
      WriteTextChunk(image,image_info,ping,ping_info,"Directory",image->directory);
    if (image->comments != (char *) NULL)
      WriteTextChunk(image,image_info,ping,ping_info,"Comment",image->comments);
    }
    png_write_end(ping,ping_info);
    if (need_fram && image->dispose == 2)
      {
        if (page_info.x || page_info.y ||
            (ping_info->width != page_info.width) ||
            (ping_info->height != page_info.height))
          {
            /*
              Write FRAM 4 with clipping boundaries followed by FRAM 1.
            */
            MSBFirstWriteLong(image,27L);  /* data length = 27 */
            PNGType(chunk,mng_FRAM);
            chunk[4]=4;
            chunk[5]=0;  /* frame name separator (no name) */
            chunk[6]=1;  /* flag for changing delay, for next frame only */
            chunk[7]=0;  /* flag for changing frame timeout */
            chunk[8]=1;  /* flag for changing frame clipping for next frame */
            chunk[9]=0;  /* flag for changing frame sync_id */
            PNGLong(chunk+10,(png_uint_32) (0L)); /* temporary 0 delay */
            chunk[14]=0; /* clipping boundaries delta type */
            PNGLong(chunk+15,(png_uint_32) (page_info.x)); /* left cb */
            PNGLong(chunk+19,(png_uint_32) (page_info.x + ping_info->width));
            PNGLong(chunk+23,(png_uint_32) (page_info.y)); /* top cb */
            PNGLong(chunk+27,(png_uint_32) (page_info.y + ping_info->height));
            (void) WriteBlob(image,31,(char *) chunk);
            MSBFirstWriteLong(image,crc32(0,chunk,31));
            old_framing_mode=4;
            framing_mode=1;
          }
        else
          framing_mode=3;
      }
    if (need_fram && (image->dispose == 3))
       MagickWarning(DelegateWarning,
         "Cannot convert GIF with disposal method 3 to MNG-LC",(char *) NULL);
    image->depth=save_image_depth;
    /*
      Free PNG resources.
    */
#if (PNG_LIBPNG_VER < 10100)
    /* Prior to libpng version 1.1.0, the palette had to be free'ed manually. */
    if (ping_info->valid & PNG_INFO_PLTE)
      {
        FreeMemory(ping_info->palette);
        ping_info->valid &= (~PNG_INFO_PLTE);
      }
#endif
    png_destroy_write_struct(&ping,&ping_info);
    FreeMemory(scanlines);
    FreeMemory(png_pixels);
    if (image->next == (Image *) NULL)
      break;
    image=GetNextImage(image);
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
  } while (image_info->adjoin);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  if (image_info->adjoin)
    {
      /*
        Write the MEND chunk.
      */
      MSBFirstWriteLong(image,0x00000000L);
      PNGType(chunk,mng_MEND);
      (void) WriteBlob(image,4,(char *) chunk);
      MSBFirstWriteLong(image,crc32(0,chunk,4));
    }
  /*
    Free memory.
  */
  CloseBlob(image);
  return(True);
}
#else
Export unsigned int WritePNGImage(const ImageInfo *image_info,Image *image)
{
  MagickWarning(MissingDelegateWarning,"PNG library is not available",
    image->filename);
  return(False);
}
#endif

