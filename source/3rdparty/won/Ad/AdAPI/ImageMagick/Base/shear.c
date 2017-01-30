/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      SSSSS  H   H  EEEEE   AAA    RRRR                      %
%                      SS     H   H  E      A   A   R   R                     %
%                       SSS   HHHHH  EEE    AAAAA   RRRR                      %
%                         SS  H   H  E      A   A   R R                       %
%                      SSSSS  H   H  EEEEE  A   A   R  R                      %
%                                                                             %
%                                                                             %
%            Methods to Shear or Rotate an Image by an Arbitrary Angle        %
%                                                                             %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                  July 1992                                  %
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
%  Method RotateImage, XShearImage, and YShearImage is based on the paper
%  "A Fast Algorithm for General Raster Rotatation" by Alan W. Paeth,
%  Graphics Interface '86 (Vancouver).  RotateImage is adapted from a similar
%  method based on the Paeth paper written by Michael Halle of the Spatial
%  Imaging Group, MIT Media Lab.
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
+   I n t e g r a l R o t a t e I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IntegralRotateImage rotates the image an integral of 90 degrees.
%  It allocates the memory necessary for the new Image structure and returns
%  a pointer to the rotated image.
%
%  The format of the IntegralRotateImage method is:
%
%      Image *IntegralRotateImage(Image *image,unsigned int rotations)
%
%  A description of each parameter follows.
%
%    o rotate_image: Method IntegralRotateImage returns a pointer to the
%      rotated image.  A null image is returned if there is a a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o rotations: Specifies the number of 90 degree rotations.
%
%
*/
static Image *IntegralRotateImage(Image *image,unsigned int rotations)
{
#define RotateImageText  "  Rotating image...  "

  Image
    *rotate_image;

  int
    y;

  RectangleInfo
    page_info;

  register int
    x;

  register PixelPacket
    *p,
    *q;

  /*
    Initialize rotated image attributes.
  */
  assert(image != (Image *) NULL);
  page_info=image->page_info;
  rotations%=4;
  if ((rotations == 1) || (rotations == 3))
    rotate_image=CloneImage(image,image->rows,image->columns,False);
  else
    rotate_image=CloneImage(image,image->columns,image->rows,False);
  if (rotate_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to rotate image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Integral rotate the image.
  */
  switch (rotations)
  {
    case 0:
    {
      /*
        Rotate 0 degrees.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        p=GetPixelCache(image,0,y,image->columns,1);
        q=SetPixelCache(rotate_image,0,y,rotate_image->columns,1);
        if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          break;
        if (image->class == PseudoClass)
          (void) memcpy(rotate_image->indexes,image->indexes,
            image->columns*sizeof(IndexPacket));
        (void) memcpy(q,p,image->columns*sizeof(PixelPacket));
        if (!SyncPixelCache(rotate_image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(RotateImageText,y,image->rows);
      }
      break;
    }
    case 1:
    {
      /*
        Rotate 90 degrees.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        p=GetPixelCache(image,0,y,image->columns,1);
        q=SetPixelCache(rotate_image,image->rows-y-1,0,1,rotate_image->rows);
        if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          break;
        if (image->class == PseudoClass)
          (void) memcpy(rotate_image->indexes,image->indexes,
            image->columns*sizeof(IndexPacket));
        (void) memcpy(q,p,image->columns*sizeof(PixelPacket));
        if (!SyncPixelCache(rotate_image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(RotateImageText,y,image->rows);
      }
      Swap(page_info.width,page_info.height);
      Swap(page_info.x,page_info.y);
      page_info.x=page_info.width-rotate_image->columns-page_info.x;
      break;
    }
    case 2:
    {
      /*
        Rotate 180 degrees.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        p=GetPixelCache(image,0,y,image->columns,1);
        q=SetPixelCache(rotate_image,0,image->rows-y-1,image->columns,1);
        if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          break;
        q+=image->columns;
        for (x=0; x < (int) image->columns; x++)
          *--q=(*p++);
        if (image->class == PseudoClass)
          for (x=0; x < (int) image->columns; x++)
            rotate_image->indexes[image->columns-x-1]=image->indexes[x];
        if (!SyncPixelCache(rotate_image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(RotateImageText,y,image->rows);
      }
      page_info.x=page_info.width-rotate_image->columns-page_info.x;
      page_info.y=page_info.height-rotate_image->rows-page_info.y;
      break;
    }
    case 3:
    {
      /*
        Rotate 270 degrees.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        p=GetPixelCache(image,0,y,image->columns,1);
        q=SetPixelCache(rotate_image,y,0,1,rotate_image->rows);
        if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          break;
        q+=image->columns;
        for (x=0; x < (int) image->columns; x++)
          *--q=(*p++);
        if (image->class == PseudoClass)
          for (x=0; x < (int) image->columns; x++)
            rotate_image->indexes[image->columns-x-1]=image->indexes[x];
        if (!SyncPixelCache(rotate_image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(RotateImageText,y,image->rows);
      }
      Swap(page_info.width,page_info.height);
      Swap(page_info.x,page_info.y);
      page_info.y=page_info.height-rotate_image->rows-page_info.y;
      break;
    }
  }
  rotate_image->page_info=page_info;
  return(rotate_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   X S h e a r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure XShearImage shears the image in the X direction with a shear angle
%  of 'degrees'.  Positive angles shear counter-clockwise (right-hand rule),
%  and negative angles shear clockwise.  Angles are measured relative to a
%  vertical Y-axis.  X shears will widen an image creating 'empty' triangles
%  on the left and right sides of the source image.
%
%  The format of the XShearImage method is:
%
%      void XShearImage(Image *image,const double degrees,
%        const unsigned int width,const unsigned int height,const int x_offset,
%        int y_offset,register Quantum *range_limit)
%
%  A description of each parameter follows.
%
%    o image: The address of a structure of type Image.
%
%    o degrees: A double representing the shearing angle along the X axis.
%
%    o width, height, x_offset, y_offset: Defines a region of the image
%      to shear.
%
*/
static void XShearImage(Image *image,const double degrees,
  const unsigned int width,const unsigned int height,const int x_offset,
  int y_offset,register Quantum *range_limit)
{
#define XShearImageText  "  X Shear image...  "

  double
    displacement;

  enum {LEFT,RIGHT}
    direction;

  int
    step,
    y;

  long
    opacity;

  register PixelPacket
    *p,
    *q;

  register int
    i;

  PixelPacket
    pixel;

  assert(image != (Image *) NULL);
  y_offset--;
  for (y=0; y < (int) height; y++)
  {
    y_offset++;
    displacement=degrees*(y-height/2.0);
    if (displacement == 0.0)
      continue;
    if (displacement > 0.0)
      direction=RIGHT;
    else
      {
        displacement*=(-1.0);
        direction=LEFT;
      }
    step=(int) floor(displacement);
    opacity=MaxRGB*(displacement-step);
    if (opacity == 0)
      {
        /*
          No fractional displacement-- just copy.
        */
        switch (direction)
        {
          case LEFT:
          {
            /*
              Transfer pixels left-to-right.
            */
            if (step > x_offset)
              break;
            p=GetPixelCache(image,0,y_offset,image->columns,1);
            if (p == (PixelPacket *) NULL)
              break;
            p+=x_offset;
            q=p-step;
            (void) memcpy(q,p,width*sizeof(PixelPacket));
            /*
              Set old row to border color.
            */
            q+=width;
            for (i=0; i < (int) step; i++)
              *q++=image->border_color;
            break;
          }
          case RIGHT:
          {
            /*
              Transfer pixels right-to-left.
            */
            p=GetPixelCache(image,0,y_offset,image->columns,1);
            if (p == (PixelPacket *) NULL)
              break;
            p+=x_offset+width;
            q=p+step;
            for (i=0; i < (int) width; i++)
              *--q=(*--p);
            /*
              Set old row to border color.
            */
            for (i=0; i < (int) step; i++)
              *--q=image->border_color;
            break;
          }
        }
        if (!SyncPixelCache(image))
          break;
        continue;
      }
    /*
      Fractional displacement.
    */
    step++;
    pixel=image->border_color;
    switch (direction)
    {
      case LEFT:
      {
        /*
          Transfer pixels left-to-right.
        */
        if (step > x_offset)
          break;
        p=GetPixelCache(image,0,y_offset,image->columns,1);
        if (p == (PixelPacket *) NULL)
          break;
        p+=x_offset;
        q=p-step;
        for (i=0; i < (int) width; i++)
        {
          if ((x_offset+i) < step)
            {
              p++;
              q++;
              continue;
            }
          q->red=range_limit[(unsigned long) (pixel.red*(MaxRGB-
             opacity)+p->red*opacity)/MaxRGB];
          q->green=range_limit[(unsigned long) (pixel.green*(MaxRGB-
             opacity)+p->green*opacity)/MaxRGB];
          q->blue=range_limit[(unsigned long) (pixel.blue*(MaxRGB-
             opacity)+p->blue*opacity)/MaxRGB];
          q->opacity=range_limit[(unsigned long) (pixel.opacity*(MaxRGB-
            opacity)+p->opacity*opacity)/MaxRGB];
          pixel=(*p);
          p++;
          q++;
        }
        /*
          Set old row to border color.
        */
        q->red=range_limit[(unsigned long) (pixel.red*(MaxRGB-
          opacity)+image->border_color.red*opacity)/MaxRGB];
        q->green=range_limit[(unsigned long) (pixel.green*(MaxRGB-
          opacity)+image->border_color.green*opacity)/MaxRGB];
        q->blue=range_limit[(unsigned long) (pixel.blue*(MaxRGB-
          opacity)+image->border_color.blue*opacity)/MaxRGB];
        q->opacity=range_limit[(unsigned long) (pixel.opacity*(MaxRGB-
          opacity)+image->border_color.opacity*opacity)/MaxRGB];
        for (i=0; i < (step-1); i++)
          *++q=image->border_color;
        break;
      }
      case RIGHT:
      {
        /*
          Transfer pixels right-to-left.
        */
        p=GetPixelCache(image,0,y_offset,image->columns,1);
        if (p == (PixelPacket *) NULL)
          break;
        p+=x_offset+width;
        q=p+step;
        for (i=0; i < (int) width; i++)
        {
          p--;
          q--;
          if ((x_offset+width+step-i) >= image->columns)
            continue;
          q->red=range_limit[(unsigned long) (pixel.red*(MaxRGB-
            opacity)+p->red*opacity)/MaxRGB];
          q->green=range_limit[(unsigned long) (pixel.green*(MaxRGB-
            opacity)+p->green*opacity)/MaxRGB];
          q->blue=range_limit[(unsigned long) (pixel.blue*(MaxRGB-
            opacity)+p->blue*opacity)/MaxRGB];
          q->opacity=range_limit[(unsigned long) (pixel.opacity*(MaxRGB-
            opacity)+p->opacity*opacity)/MaxRGB];
          pixel=(*p);
        }
        /*
          Set old row to border color.
        */
        q--;
        q->red=range_limit[(unsigned long) (pixel.red*(MaxRGB-
          opacity)+image->border_color.red*opacity)/MaxRGB];
        q->green=range_limit[(unsigned long) (pixel.green*(MaxRGB-
          opacity)+image->border_color.green*opacity)/MaxRGB];
        q->blue=range_limit[(unsigned long) (pixel.blue*(MaxRGB-
          opacity)+image->border_color.blue*opacity)/MaxRGB];
        q->opacity=range_limit[(unsigned long) (pixel.opacity*(MaxRGB-
          opacity)+image->border_color.opacity*opacity)/MaxRGB];
        for (i=0; i < (step-1); i++)
          *--q=image->border_color;
        break;
      }
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(y,height))
      ProgressMonitor(XShearImageText,y,height);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   Y S h e a r I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Procedure YShearImage shears the image in the Y direction with a shear
%  angle of 'degrees'.  Positive angles shear counter-clockwise (right-hand
%  rule), and negative angles shear clockwise.  Angles are measured relative
%  to a horizontal X-axis.  Y shears will increase the height of an image
%  creating 'empty' triangles on the top and bottom of the source image.
%
%  The format of the YShearImage method is:
%
%      void YShearImage(Image *image,const double degrees,
%        const unsigned int width,const unsigned int height,int x_offset,
%        const int y_offset,register Quantum *range_limit)
%
%  A description of each parameter follows.
%
%    o image: The address of a structure of type Image.
%
%    o degrees: A double representing the shearing angle along the Y axis.
%
%    o width, height, x_offset, y_offset: Defines a region of the image
%      to shear.
%
%
*/
static void YShearImage(Image *image,const double degrees,
  const unsigned int width,const unsigned int height,int x_offset,
  const int y_offset,register Quantum *range_limit)
{
#define YShearImageText  "  Y Shear image...  "

  double
    displacement;

  enum {UP,DOWN}
    direction;

  int
    step,
    y;

  long
    opacity;

  register PixelPacket
    *p,
    *q;

  register int
    i;

  PixelPacket
    pixel;

  assert(image != (Image *) NULL);
  x_offset--;
  for (y=0; y < (int) width; y++)
  {
    x_offset++;
    displacement=degrees*(y-width/2.0);
    if (displacement == 0.0)
      continue;
    if (displacement > 0.0)
      direction=DOWN;
    else
      {
        displacement*=(-1.0);
        direction=UP;
      }
    step=(int) floor(displacement);
    opacity=MaxRGB*(displacement-step);
    if (opacity == 0)
      {
        /*
          No fractional displacement-- just copy the pixels.
        */
        switch (direction)
        {
          case UP:
          {
            /*
              Transfer pixels top-to-bottom.
            */
            if (step > y_offset)
              break;
            p=GetPixelCache(image,x_offset,0,1,image->rows);
            if (p == (PixelPacket *) NULL)
              break;
            p+=y_offset;
            q=p-step;
            (void) memcpy(q,p,height*sizeof(PixelPacket));
            /*
              Set old column to border color.
            */
            q+=height;
            for (i=0; i < (int) step; i++)
              *q++=image->border_color;
            break;
          }
          case DOWN:
          {
            /*
              Transfer pixels bottom-to-top.
            */
            p=GetPixelCache(image,x_offset,0,1,image->rows);
            if (p == (PixelPacket *) NULL)
              break;
            p+=y_offset+height;
            q=p+step;
            for (i=0; i < (int) height; i++)
              *--q=(*--p);
            /*
              Set old column to border color.
            */
            for (i=0; i < (int) step; i++)
              *--q=image->border_color;
            break;
          }
        }
        if (!SyncPixelCache(image))
          break;
        continue;
      }
    /*
      Fractional displacment.
    */
    step++;
    pixel=image->border_color;
    switch (direction)
    {
      case UP:
      {
        /*
          Transfer pixels top-to-bottom.
        */
        if (step > y_offset)
          break;
        p=GetPixelCache(image,x_offset,0,1,image->rows);
        if (p == (PixelPacket *) NULL)
          break;
        p+=y_offset;
        q=p-step;
        for (i=0; i < (int) height; i++)
        {
          q->red=range_limit[(unsigned long) (pixel.red*(MaxRGB-
            opacity)+p->red*opacity)/MaxRGB];
          q->green=range_limit[(unsigned long) (pixel.green*(MaxRGB-
            opacity)+p->green*opacity)/MaxRGB];
          q->blue=range_limit[(unsigned long) (pixel.blue*(MaxRGB-
            opacity)+p->blue*opacity)/MaxRGB];
          q->opacity=range_limit[(unsigned long) (pixel.opacity*(MaxRGB-
            opacity)+p->opacity*opacity)/MaxRGB];
          pixel=(*p);
          p++;
          q++;
        }
        /*
          Set old column to border color.
        */
        q->red=range_limit[(unsigned long) (pixel.red*(MaxRGB-
          opacity)+image->border_color.red*opacity)/MaxRGB];
        q->green=range_limit[(unsigned long) (pixel.green*(MaxRGB-
          opacity)+image->border_color.green*opacity)/MaxRGB];
        q->blue=range_limit[(unsigned long) (pixel.blue*(MaxRGB-
          opacity)+image->border_color.blue*opacity)/MaxRGB];
        q->opacity=range_limit[(unsigned long) (pixel.opacity*(MaxRGB-
          opacity)+image->border_color.opacity*opacity)/MaxRGB];
        for (i=0; i < (step-1); i++)
          *++q=image->border_color;
        break;
      }
      case DOWN:
      {
        /*
          Transfer pixels bottom-to-top.
        */
        p=GetPixelCache(image,x_offset,0,1,image->rows);
        if (p == (PixelPacket *) NULL)
          break;
        p+=y_offset+height;
        q=p+step;
        for (i=0; i < (int) height; i++)
        {
          p--;
          q--;
          if ((y_offset+height+step-i) >= image->rows)
            continue;
          q->red=range_limit[(unsigned long) (pixel.red*(MaxRGB-
            opacity)+p->red*opacity)/MaxRGB];
          q->green=range_limit[(unsigned long) (pixel.green*(MaxRGB-
            opacity)+p->green*opacity)/MaxRGB];
          q->blue=range_limit[(unsigned long) (pixel.blue*(MaxRGB-
            opacity)+p->blue*opacity)/MaxRGB];
          q->opacity=range_limit[(unsigned long) (pixel.opacity*(MaxRGB-
            opacity)+p->opacity*opacity)/MaxRGB];
          pixel=(*p);
        }
        /*
          Set old column to border color.
        */
        q--;
        q->red=range_limit[(unsigned long) (pixel.red*(MaxRGB-
          opacity)+image->border_color.red*opacity)/MaxRGB];
        q->green=range_limit[(unsigned long) (pixel.green*(MaxRGB-
          opacity)+image->border_color.green*opacity)/MaxRGB];
        q->blue=range_limit[(unsigned long) (pixel.blue*(MaxRGB-
          opacity)+image->border_color.blue*opacity)/MaxRGB];
        q->opacity=range_limit[(unsigned long) (pixel.opacity*(MaxRGB-
          opacity)+image->border_color.opacity*opacity)/MaxRGB];
        for (i=0; i < (step-1); i++)
          *--q=image->border_color;
        break;
      }
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(y,width))
      ProgressMonitor(YShearImageText,y,width);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R o t a t e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RotateImage creates a new image that is a rotated copy of an
%  existing one.  Positive angles rotate counter-clockwise (right-hand rule),
%  while negative angles rotate clockwise.  Rotated images are usually larger
%  than the originals and have 'empty' triangular corners.  X axis.  Empty
%  triangles left over from shearing the image are filled with the color
%  defined by the pixel at location (0,0).  RotateImage allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  Method RotateImage is based on the paper "A Fast Algorithm for General
%  Raster Rotatation" by Alan W. Paeth.  RotateImage is adapted from a similar
%  method based on the Paeth paper written by Michael Halle of the Spatial
%  Imaging Group, MIT Media Lab.
%
%  The format of the RotateImage method is:
%
%      Image *RotateImage(Image *image,const double degrees)
%
%  A description of each parameter follows.
%
%    o status: Method RotateImage returns a pointer to the image after
%      rotating.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o degrees: Specifies the number of degrees to rotate the image.
%
%
*/
Export Image *RotateImage(Image *image,const double degrees)
{
  double
    angle;

  Image
    *integral_image,
    *rotate_image;

  int
    x_offset,
    y_offset;

  PointInfo
    shear;

  Quantum
    *range_limit,
    *range_table;

  RectangleInfo
    border_info;

  register int
    i;

  unsigned int
    height,
    rotations,
    width,
    y_width;

  /*
    Adjust rotation angle.
  */
  assert(image != (Image *) NULL);
  angle=degrees;
  while (angle < -45.0)
    angle+=360.0;
  for (rotations=0; angle > 45.0; rotations++)
    angle-=90.0;
  rotations%=4;
  /*
    Calculate shear equations.
  */
  integral_image=IntegralRotateImage(image,rotations);
  if (integral_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to rotate image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  shear.x=(-tan(DegreesToRadians(angle)/2.0));
  shear.y=sin(DegreesToRadians(angle));
  if ((shear.x == 0.0) || (shear.y == 0.0))
    return(integral_image);
  /*
    Initialize range table.
  */
  range_table=(Quantum *) AllocateMemory(3*(MaxRGB+1)*sizeof(Quantum));
  if (range_table == (Quantum *) NULL)
    {
      DestroyImage(integral_image);
      MagickWarning(ResourceLimitWarning,"Unable to rotate image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  for (i=0; i <= MaxRGB; i++)
  {
    range_table[i]=0;
    range_table[i+(MaxRGB+1)]=(Quantum) i;
    range_table[i+(MaxRGB+1)*2]=MaxRGB;
  }
  range_limit=range_table+(MaxRGB+1);
  /*
    Compute image size.
  */
  width=image->columns;
  height=image->rows;
  if ((rotations == 1) || (rotations == 3))
    {
      width=image->rows;
      height=image->columns;
    }
  y_width=width+ceil(height*fabs(shear.x));
  x_offset=width+2.0*ceil(height*fabs(shear.y))-width;
  y_offset=height+ceil(y_width*fabs(shear.y))-height;
  /*
    Surround image with a border.
  */
  if (!integral_image->matte)
    MatteImage(integral_image,Opaque);
  integral_image->border_color.opacity=Transparent;
  border_info.width=x_offset;
  border_info.height=y_offset;
  rotate_image=BorderImage(integral_image,&border_info);
  DestroyImage(integral_image);
  if (rotate_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to rotate image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  rotate_image->class=DirectClass;
  /*
    Rotate the image.
  */
  XShearImage(rotate_image,shear.x,width,height,x_offset,
    (rotate_image->rows-height+1)/2,range_limit);
  YShearImage(rotate_image,shear.y,y_width,height,
    (rotate_image->columns-y_width+1)/2,y_offset,range_limit);
  XShearImage(rotate_image,shear.x,y_width,rotate_image->rows,
    (rotate_image->columns-y_width+1)/2,0,range_limit);
  TransformImage(&rotate_image,"0x0",(char *) NULL);
  GetPageInfo(&rotate_image->page_info);
  FreeMemory(range_table);
  return(rotate_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S h e a r I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ShearImage creates a new image that is a shear_image copy of an
%  existing one.  Shearing slides one edge of an image along the X or Y
%  axis, creating a parallelogram.  An X direction shear slides an edge
%  along the X axis, while a Y direction shear slides an edge along the Y
%  axis.  The amount of the shear is controlled by a shear angle.  For X
%  direction shears, x_shear is measured relative to the Y axis, and
%  similarly, for Y direction shears y_shear is measured relative to the
%  X axis.  Empty triangles left over from shearing the image are filled
%  with the color defined by the pixel at location (0,0).  ShearImage
%  allocates the memory necessary for the new Image structure and returns
%  a pointer to the new image.
%
%  Method ShearImage is based on the paper "A Fast Algorithm for General
%  Raster Rotatation" by Alan W. Paeth.
%
%  The format of the ShearImage method is:
%
%      Image *ShearImage(Image *image,const double x_shear,const double y_shear)
%
%  A description of each parameter follows.
%
%    o status: Method ShearImage returns a pointer to the image after
%      rotating.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o x_shear, y_shear: Specifies the number of degrees to shear the image.
%
%
*/
Export Image *ShearImage(Image *image,const double x_shear,const double y_shear)
{
  Image
    *integral_image,
    *shear_image;

  int
    x_offset,
    y_offset;

  PointInfo
    shear;

  Quantum
    *range_limit,
    *range_table;

  RectangleInfo
    border_info;

  register int
    i;

  unsigned int
    y_width;

  assert(image != (Image *) NULL);
  if ((x_shear == 180.0) || (y_shear == 180.0))
    {
      MagickWarning(OptionWarning,"Unable to shear image",
        "angle is discontinuous");
      return((Image *) NULL);
    }
  /*
    Initialize shear angle.
  */
  integral_image=IntegralRotateImage(image,0);
  if (integral_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to shear image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  shear.x=(-tan(DegreesToRadians(x_shear)/2.0));
  shear.y=sin(DegreesToRadians(y_shear));
  if ((shear.x == 0.0) || (shear.y == 0.0))
    return(integral_image);
  /*
    Initialize range table.
  */
  range_table=(Quantum *) AllocateMemory(3*(MaxRGB+1)*sizeof(Quantum));
  if (range_table == (Quantum *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to shear image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  for (i=0; i <= MaxRGB; i++)
  {
    range_table[i]=0;
    range_table[i+(MaxRGB+1)]=(Quantum) i;
    range_table[i+(MaxRGB+1)*2]=MaxRGB;
  }
  range_limit=range_table+(MaxRGB+1);
  /*
    Compute image size.
  */
  y_width=image->columns+ceil(image->rows*fabs(shear.x));
  x_offset=image->columns+ceil(2*image->rows*fabs(shear.y))-image->columns;
  y_offset=image->rows+ceil(y_width*fabs(shear.y))-image->rows;
  /*
    Surround image with border.
  */
  if (!integral_image->matte)
    MatteImage(integral_image,Opaque);
  integral_image->border_color.opacity=Transparent;
  border_info.width=x_offset;
  border_info.height=y_offset;
  shear_image=BorderImage(integral_image,&border_info);
  if (shear_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to shear image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  DestroyImage(integral_image);
  shear_image->class=DirectClass;
  /*
    Shear the image.
  */
  XShearImage(shear_image,shear.x,image->columns,image->rows,x_offset,
    (shear_image->rows-image->rows+1)/2,range_limit);
  YShearImage(shear_image,shear.y,y_width,image->rows,
    (shear_image->columns-y_width+1)/2,y_offset,range_limit);
  TransformImage(&shear_image,"0x0",(char *) NULL);
  GetPageInfo(&shear_image->page_info);
  FreeMemory(range_table);
  return(shear_image);
}
