/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%          DDDD   EEEEE   CCCC   OOO   RRRR    AAA   TTTTT  EEEEE             %
%          D   D  E      C      O   O  R   R  A   A    T    E                 %
%          D   D  EEE    C      O   O  RRRR   AAAAA    T    EEE               %
%          D   D  E      C      O   O  R R    A   A    T    E                 %
%          DDDD   EEEEE   CCCC   OOO   R  R   A   A    T    EEEEE             %
%                                                                             %
%                                                                             %
%                   ImageMagick Image Decoration Methods                      %
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
%   B o r d e r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method BorderImage takes an image and puts a border around it of a
%  particular color.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the BorderImage method is:
%
%      Image *BorderImage(const Image *image,const RectangleInfo *border_info)
%
%  A description of each parameter follows:
%
%    o bordered_image: Method BorderImage returns a pointer to the bordered
%      image.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o border_info: Specifies a pointer to a structure of type Rectangle which
%      defines the border region.
%
*/
Export Image *BorderImage(Image *image,const RectangleInfo *border_info)
{
  PixelPacket
    matte_color;

  Image
    *bordered_image;

  FrameInfo
    frame_info;

  assert(image != (Image *) NULL);
  assert(border_info != (RectangleInfo *) NULL);
  frame_info.width=image->columns+(border_info->width << 1);
  frame_info.height=image->rows+(border_info->height << 1);
  frame_info.x=border_info->width;
  frame_info.y=border_info->height;
  frame_info.inner_bevel=0;
  frame_info.outer_bevel=0;
  matte_color=image->matte_color;
  image->matte_color=image->border_color;
  bordered_image=FrameImage(image,&frame_info);
  bordered_image->matte_color=matte_color;
  image->matte_color=matte_color;
  return(bordered_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   F r a m e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method FrameImage takes an image and puts a frame around it of a
%  particular color.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the FrameImage method is:
%
%      Image *FrameImage(Image *image,const FrameInfo *frame_info)
%
%  A description of each parameter follows:
%
%    o frame_image: Method FrameImage returns a pointer to the framed
%      image.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o frame_info: Specifies a pointer to a FrameInfo structure which
%      defines the framed region.
%
%
*/
Export Image *FrameImage(Image *image,const FrameInfo *frame_info)
{
#define FrameImageText  "  Adding frame to image...  "

  Image
    *frame_image;

  int
    height,
    width,
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q;

  PixelPacket
    accentuate,
    highlight,
    matte,
    shadow,
    trough;

  unsigned int
    bevel_width;

  /*
    Check frame geometry.
  */
  assert(image != (Image *) NULL);
  assert(frame_info != (FrameInfo *) NULL);
  if ((frame_info->outer_bevel < 0) || (frame_info->inner_bevel < 0))
    {
      MagickWarning(OptionWarning,"Unable to frame image",
        "bevel width is negative");
      return((Image *) NULL);
    }
  bevel_width=frame_info->outer_bevel+frame_info->inner_bevel;
  width=(int) frame_info->width-frame_info->x-bevel_width;
  height=(int) frame_info->height-frame_info->y-bevel_width;
  if ((width < (int) image->columns) || (height < (int) image->rows))
    {
      MagickWarning(OptionWarning,"Unable to frame image",
        "frame is less than image size");
      return((Image *) NULL);
    }
  /*
    Initialize framed image attributes.
  */
  frame_image=CloneImage(image,frame_info->width,frame_info->height,False);
  if (frame_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to frame image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  frame_image->class=DirectClass;
  /*
    Initialize 3D effects color.
  */
  matte=image->matte_color;
  accentuate.red=((unsigned long)
    (matte.red*AccentuateModulate+(MaxRGB-AccentuateModulate)*MaxRGB)/MaxRGB);
  accentuate.green=((unsigned long)
    (matte.green*AccentuateModulate+(MaxRGB-AccentuateModulate)*MaxRGB)/MaxRGB);
  accentuate.blue=((unsigned long)
    (matte.blue*AccentuateModulate+(MaxRGB-AccentuateModulate)*MaxRGB)/MaxRGB);
  accentuate.opacity=((unsigned long) (matte.opacity*
    AccentuateModulate+(MaxRGB-AccentuateModulate)*MaxRGB)/MaxRGB);
  highlight.red=((unsigned long)
    (matte.red*HighlightModulate+(MaxRGB-HighlightModulate)*MaxRGB)/MaxRGB);
  highlight.green=((unsigned long)
    (matte.green*HighlightModulate+(MaxRGB-HighlightModulate)*MaxRGB)/MaxRGB);
  highlight.blue=((unsigned long)
    (matte.blue*HighlightModulate+(MaxRGB-HighlightModulate)*MaxRGB)/MaxRGB);
  highlight.opacity=((unsigned long)
    (matte.opacity*HighlightModulate+(MaxRGB-HighlightModulate)*MaxRGB)/MaxRGB);
  shadow.red=((unsigned long) (matte.red*ShadowModulate)/MaxRGB);
  shadow.green=((unsigned long) (matte.green*ShadowModulate)/MaxRGB);
  shadow.blue=((unsigned long) (matte.blue*ShadowModulate)/MaxRGB);
  shadow.opacity=(Quantum)
    ((unsigned long) (matte.opacity*ShadowModulate)/MaxRGB);
  trough.red=((unsigned long) (matte.red*TroughModulate)/MaxRGB);
  trough.green=((unsigned long) (matte.green*TroughModulate)/MaxRGB);
  trough.blue=((unsigned long) (matte.blue*TroughModulate)/MaxRGB);
  trough.opacity=(Quantum)
    ((unsigned long) (matte.opacity*TroughModulate)/MaxRGB);
  /*
    Draw top of ornamental border.
  */
  height=frame_info->outer_bevel+(frame_info->y-bevel_width)+
    frame_info->inner_bevel;
  q=SetPixelCache(frame_image,0,0,frame_image->columns,Max(height,1));
  if (q == (PixelPacket *) NULL)
    {
      DestroyImage(frame_image);
      return((Image *) NULL);
    }
  for (y=0; y < frame_info->outer_bevel; y++)
  {
    for (x=0; x < (int) (frame_image->columns-y); x++)
      if (x < y)
        *q++=highlight;
      else
        *q++=accentuate;
    for ( ; x < (int) frame_image->columns; x++)
      *q++=shadow;
  }
  for (y=0; y < (int) (frame_info->y-bevel_width); y++)
  {
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (int) (frame_image->columns-2*frame_info->outer_bevel); x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
  }
  for (y=0; y < frame_info->inner_bevel; y++)
  {
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (int) (frame_info->x-bevel_width); x++)
      *q++=matte;
    for (x=0; x < (int) (image->columns+(frame_info->inner_bevel << 1)-y); x++)
      if (x < y)
        *q++=shadow;
      else
        *q++=trough;
    for ( ; x < (int) (image->columns+(frame_info->inner_bevel << 1)); x++)
      *q++=highlight;
    width=frame_info->width-frame_info->x-image->columns-bevel_width;
    for (x=0; x < width; x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
  }
  if (!SyncPixelCache(frame_image))
    {
      DestroyImage(frame_image);
      return((Image *) NULL);
    }
  /*
    Draw sides of ornamental border.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    /*
      Initialize scanline with border color.
    */
    p=GetPixelCache(image,0,y,image->columns,1);
    q=SetPixelCache(frame_image,0,frame_info->y+y,frame_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (int) (frame_info->x-bevel_width); x++)
      *q++=matte;
    for (x=0; x < frame_info->inner_bevel; x++)
      *q++=shadow;
    /*
      Transfer scanline.
    */
    (void) memcpy(q,p,image->columns*sizeof(PixelPacket));
    q+=image->columns;
    for (x=0; x < frame_info->inner_bevel; x++)
      *q++=highlight;
    width=frame_info->width-frame_info->x-image->columns-bevel_width;
    for (x=0; x < width; x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
    if (!SyncPixelCache(frame_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(FrameImageText,y,image->rows);
  }
  /*
    Draw bottom of ornamental border.
  */
  height=frame_info->inner_bevel+frame_info->height-frame_info->y-image->rows-
    bevel_width+frame_info->outer_bevel;
  q=SetPixelCache(frame_image,0,frame_image->rows-Max(height,1),
    frame_image->columns,Max(height,1));
  if (q == (PixelPacket *) NULL)
    {
      DestroyImage(frame_image);
      return((Image *) NULL);
    }
  for (y=frame_info->inner_bevel-1; y >= 0; y--)
  {
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (int) (frame_info->x-bevel_width); x++)
      *q++=matte;
    for (x=0; x < y; x++)
      *q++=shadow;
    for ( ; x < (int) (image->columns+(frame_info->inner_bevel << 1)); x++)
      if (x >= (int) (image->columns+(frame_info->inner_bevel << 1)-y))
        *q++=highlight;
      else
        *q++=accentuate;
    width=frame_info->width-frame_info->x-image->columns-bevel_width;
    for (x=0; x < (int) width; x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
  }
  height=frame_info->height-frame_info->y-image->rows-bevel_width;
  for (y=0; y < height; y++)
  {
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=highlight;
    for (x=0; x < (int) (frame_image->columns-2*frame_info->outer_bevel); x++)
      *q++=matte;
    for (x=0; x < frame_info->outer_bevel; x++)
      *q++=shadow;
  }
  for (y=frame_info->outer_bevel-1; y >= 0; y--)
  {
    for (x=0; x < y; x++)
      *q++=highlight;
    for ( ; x < (int) frame_image->columns; x++)
      if (x >= (int) (frame_image->columns-y))
        *q++=shadow;
      else
        *q++=trough;
  }
  if (!SyncPixelCache(frame_image))
    {
      DestroyImage(frame_image);
      return((Image *) NULL);
    }
  return(frame_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R a i s e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RaiseImage lightens and darkens the edges of an image to give a
%  3-D raised or lower effect.
%
%  The format of the RaiseImage method is:
%
%      void RaiseImage(Image *image,const RectangleInfo *raise_info,
%        const int raised)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o raise_info: Specifies a pointer to a XRectangle which defines the
%      raised region.
%
%    o raised: A value other than zero causes the image to have a 3-D raised
%      effect, otherwise it has a lowered effect.
%
%
*/
Export void RaiseImage(Image *image,const RectangleInfo *raise_info,
  const int raised)
{
#define AccentuateFactor  UpScale(135)
#define HighlightFactor  UpScale(190)
#define ShadowFactor  UpScale(190)
#define RaiseImageText  "  Raising image...  "
#define TroughFactor  UpScale(135)

  int
    y;

  Quantum
    foreground,
    background;

  register int
    i,
    x;

  register PixelPacket
    *q;

  unsigned int
    height;

  assert(image != (Image *) NULL);
  assert(raise_info != (RectangleInfo *) NULL);
  if ((image->columns <= (raise_info->width << 1)) ||
      (image->rows <= (raise_info->height << 1)))
    {
      MagickWarning(OptionWarning,"Unable to raise image",
        "image size must exceed bevel width");
      return;
    }
  foreground=MaxRGB;
  background=0;
  if (!raised)
    {
      foreground=0;
      background=MaxRGB;
    }
  i=0;
  image->class=DirectClass;
  for (y=0; y < (int) raise_info->height; y++)
  {
    q=GetPixelCache(image,0,i++,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < y; x++)
    {
      q->red=((unsigned long)
        (q->red*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q->green=((unsigned long)
        (q->green*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q->blue=((unsigned long)
        (q->blue*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q++;
    }
    for (x=0; x < (int) (image->columns-(y << 1)); x++)
    {
      q->red=((unsigned long)
        (q->red*AccentuateFactor+foreground*(MaxRGB-AccentuateFactor))/MaxRGB);
      q->green=((unsigned long) (q->green*
        AccentuateFactor+foreground*(MaxRGB-AccentuateFactor))/MaxRGB);
      q->blue=((unsigned long)
        (q->blue*AccentuateFactor+foreground*(MaxRGB-AccentuateFactor))/MaxRGB);
      q++;
    }
    for (x=0; x < y; x++)
    {
      q->red=((unsigned long)
        (q->red*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q->green=((unsigned long)
        (q->green*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q->blue=((unsigned long)
        (q->blue*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q++;
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(i,image->rows))
      ProgressMonitor(RaiseImageText,i,image->rows);
  }
  height=image->rows-(raise_info->height << 1);
  for (y=0; y < (int) height; y++)
  {
    q=GetPixelCache(image,0,i++,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) raise_info->width; x++)
    {
      q->red=((unsigned long)
	(q->red*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q->green=((unsigned long)
	(q->green*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q->blue=((unsigned long)
	(q->blue*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q++;
    }
    for (x=0; x < (int) (image->columns-(raise_info->width << 1)); x++)
      q++;
    for (x=0; x < (int) raise_info->width; x++)
    {
      q->red=((unsigned long)
        (q->red*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q->green=((unsigned long)
        (q->green*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q->blue=((unsigned long)
        (q->blue*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q++;
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(i,image->rows))
      ProgressMonitor(RaiseImageText,i,image->rows);
  }
  for (y=0; y < (int) raise_info->height; y++)
  {
    q=GetPixelCache(image,0,i++,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) (raise_info->width-y); x++)
    {
      q->red=((unsigned long)
	(q->red*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q->green=((unsigned long)
	(q->green*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q->blue=((unsigned long)
	(q->blue*HighlightFactor+foreground*(MaxRGB-HighlightFactor))/MaxRGB);
      q++;
    }
    for (x=0; x < (int) (image->columns-((raise_info->width-y) << 1)); x++)
    {
      q->red=((unsigned long)
        (q->red*TroughFactor+background*(MaxRGB-TroughFactor))/MaxRGB);
      q->green=((unsigned long)
        (q->green*TroughFactor+background*(MaxRGB-TroughFactor))/MaxRGB);
      q->blue=((unsigned long)
        (q->blue*TroughFactor+background*(MaxRGB-TroughFactor))/MaxRGB);
      q++;
    }
    for (x=0; x < (int) (raise_info->width-y); x++)
    {
      q->red=((unsigned long)
        (q->red*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q->green=((unsigned long)
        (q->green*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q->blue=((unsigned long)
        (q->blue*ShadowFactor+background*(MaxRGB-ShadowFactor))/MaxRGB);
      q++;
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(i,image->rows))
      ProgressMonitor(RaiseImageText,i,image->rows);
  }
  return;
}
