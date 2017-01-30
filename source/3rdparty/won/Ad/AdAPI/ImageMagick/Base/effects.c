/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                EEEEE  FFFFF  FFFFF  EEEEE  CCCC  TTTTT  SSSSS               %
%                E      F      F      E     C        T    SS                  %
%                EEE    FFF    FFF    EEE   C        T     SSS                %
%                E      F      F      E     C        T       SS               %
%                EEEEE  F      F      EEEEE  CCCC    T    SSSSS               %
%                                                                             %
%                                                                             %
%                      ImageMagick Image Effects Methods                      %
%                                                                             %
%                                                                             %
%                               Software Design                               %
%                                 John Cristy                                 %
%                                 October 1996                                %
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
%     A d d N o i s e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method AddNoiseImage creates a new image that is a copy of an existing
%  one with noise added.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  The format of the AddNoiseImage method is:
%
%      Image *AddNoiseImage(Image *image,const NoiseType noise_type)
%
%  A description of each parameter follows:
%
%    o noisy_image: Method AddNoiseImage returns a pointer to the image after
%      the noise is minified.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o noise_type:  The type of noise: Gaussian, multiplicative Gaussian,
%      impulse, laplacian, or Poisson.
%
%
*/
Export Image *AddNoiseImage(Image *image,const NoiseType noise_type)
{
#define AddNoiseImageText  "  Adding noise to the image...  "

  Image
    *noisy_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q;

  /*
    Initialize noisy image attributes.
  */
  assert(image != (Image *) NULL);
  noisy_image=CloneImage(image,image->columns,image->rows,False);
  if (noisy_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to reduce noise",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  noisy_image->class=DirectClass;
  /*
    Add noise in each row.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    q=SetPixelCache(noisy_image,0,y,noisy_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      q->red=GenerateNoise(p->red,noise_type);
      q->green=GenerateNoise(p->green,noise_type);
      q->blue=GenerateNoise(p->blue,noise_type);
      q++;
    }
    if (!SyncPixelCache(noisy_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(AddNoiseImageText,y,image->rows);
  }
  return(noisy_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     B l u r I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method BlurImage creates a new image that is a copy of an existing
%  one with the pixels blurred.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  BlurImage convolves the pixel neighborhood with this blurring mask:
%
%     1  2  1
%     2  W  2
%     1  2  1
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the BlurImage method is:
%
%      Image *BlurImage(Image *image,const double factor)
%
%  A description of each parameter follows:
%
%    o blur_image: Method BlurImage returns a pointer to the image
%      after it is blurred.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o factor:  An double value reflecting the percent weight to give to the
%      center pixel of the neighborhood.
%
%
*/
Export Image *BlurImage(Image *image,const double factor)
{
#define Blur(weight) \
  total_red+=(weight)*s->red; \
  total_green+=(weight)*s->green; \
  total_blue+=(weight)*s->blue; \
  total_opacity+=(weight)*s->opacity; \
  s++;
#define BlurImageText  "  Bluring image...  "

  double
    quantum,
    total_blue,
    total_green,
    total_opacity,
    total_red,
    weight;

  Image
    *blur_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  assert(image != (Image *) NULL);
  if ((image->columns < 3) || (image->rows < 3))
    return((Image *) NULL);
  /*
    Initialize blured image attributes.
  */
  blur_image=CloneImage(image,image->columns,image->rows,False);
  if (blur_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to blur image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  blur_image->class=DirectClass;
  /*
    Blur image.
  */
  weight=((100.0-factor)/2)+1;
  quantum=Max(weight+12.0,1.0);
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,Min(Max(y-1,0),image->rows-3),image->columns,3);
    q=SetPixelCache(blur_image,0,y,blur_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    /*
      Blur this row of pixels.
    */
    *q++=(*(p+image->columns));
    for (x=1; x < (int) (image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0.0;
      total_green=0.0;
      total_blue=0.0;
      total_opacity=0.0;
      s=p;
      Blur(1); Blur(2); Blur(1);
      s=p+image->columns;
      Blur(2); Blur(weight); Blur(2);
      s=p+2*image->columns;
      Blur(1); Blur(2); Blur(1);
      q->red=(total_red+(quantum/2))/quantum;
      q->green=(total_green+(quantum/2))/quantum;
      q->blue=(total_blue+(quantum/2))/quantum;
      q->opacity=(total_opacity+(quantum/2))/quantum;
      p++;
      q++;
    }
    p++;
    *q++=(*p);
    if (!SyncPixelCache(blur_image))
      break;
    if (QuantumTick(y,image->rows-1))
      ProgressMonitor(BlurImageText,y,image->rows-1);
  }
  return(blur_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     C o l o r i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ColorizeImage creates a new image that is a copy of an existing
%  one with the image pixels colorized.  The colorization is controlled
%  with the pen color and the opacity levels.
%
%  The format of the ColorizeImage method is:
%
%      Image *ColorizeImage(Image *image,const char *opacity,
%        const char *pen_color)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o opacity:  A character string indicating the level of opacity as a
%      percentage (0-100).
%
%      pen_color: A color string.
%
%
*/
Export Image *ColorizeImage(Image *image,const char *opacity,
  const char *pen_color)
{
#define ColorizeImageText  "  Colorizing the image...  "

  Image
    *colorize_image;

  int
    y;

  long
    blue_opacity,
    count,
    green_opacity,
    red_opacity;

  PixelPacket
    target;

  register int
    x;

  register PixelPacket
    *p,
    *q;

  /*
    Allocate colorized image.
  */
  assert(image != (Image *) NULL);
  colorize_image=CloneImage(image,image->columns,image->rows,False);
  if (colorize_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to colorized image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  colorize_image->class=DirectClass;
  /*
    Determine RGB values of the pen color.
  */
  (void) QueryColorDatabase(pen_color,&target);
  red_opacity=100;
  green_opacity=100;
  blue_opacity=100;
  count=sscanf(opacity,"%ld/%ld/%ld",&red_opacity,&green_opacity,&blue_opacity);
  if (count == 1)
    {
      if (red_opacity == 0)
        return(colorize_image);
      green_opacity=red_opacity;
      blue_opacity=red_opacity;
    }
  /*
    Colorize DirectClass image.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    q=SetPixelCache(colorize_image,0,y,colorize_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      q->red=(Quantum) ((unsigned long)
        (p->red*(100-red_opacity)+target.red*red_opacity)/100);
      q->green=(Quantum) ((unsigned long)
        (p->green*(100-green_opacity)+target.green*green_opacity)/100);
      q->blue=(Quantum) ((unsigned long)
        (p->blue*(100-blue_opacity)+target.blue*blue_opacity)/100);
      p++;
      q++;
    }
    if (!SyncPixelCache(colorize_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(ColorizeImageText,y,image->rows);
  }
  return(colorize_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     D e s p e c k l e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DespeckleImage creates a new image that is a copy of an existing
%  one with the speckle noise minified.  It uses the eight hull algorithm
%  described in Applied Optics, Vol. 24, No. 10, 15 May 1985, "Geometric filter
%  for Speckle Reduction", by Thomas R Crimmins.  Each pixel in the image is
%  replaced by one of its eight of its surrounding pixels using a polarity and
%  negative hull function.  DespeckleImage allocates the memory necessary for
%  the new Image structure and returns a pointer to the new image.
%
%  The format of the DespeckleImage method is:
%
%      Image *DespeckleImage(Image *image)
%
%  A description of each parameter follows:
%
%    o despeckle_image: Method DespeckleImage returns a pointer to the image
%      after it is despeckled.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export Image *DespeckleImage(Image *image)
{
#define DespeckleImageText  "  Despeckling image...  "

  Image
    *despeckle_image;

  int
    y;

  Quantum
    *blue_channel,
    *buffer,
    *green_channel,
    *matte_channel,
    *red_channel;

  register int
    i,
    j,
    x;

  register PixelPacket
    *p,
    *q;

  static const int
    X[4]= {0, 1, 1,-1},
    Y[4]= {1, 0, 1, 1};

  unsigned int
    packets;

  /*
    Allocate despeckled image.
  */
  assert(image != (Image *) NULL);
  despeckle_image=CloneImage(image,image->columns,image->rows,False);
  if (despeckle_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to despeckle image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  despeckle_image->class=DirectClass;
  /*
    Allocate image buffers.
  */
  packets=(image->columns+2)*(image->rows+2);
  red_channel=(Quantum *) AllocateMemory(packets*sizeof(Quantum));
  green_channel=(Quantum *) AllocateMemory(packets*sizeof(Quantum));
  blue_channel=(Quantum *) AllocateMemory(packets*sizeof(Quantum));
  matte_channel=(Quantum *) AllocateMemory(packets*sizeof(Quantum));
  buffer=(Quantum *) AllocateMemory(packets*sizeof(Quantum));
  if ((red_channel == (Quantum *) NULL) ||
      (green_channel == (Quantum *) NULL) ||
      (blue_channel == (Quantum *) NULL) ||
      (matte_channel == (Quantum *) NULL) ||
      (buffer == (Quantum *) NULL))
    {
      MagickWarning(ResourceLimitWarning,"Unable to despeckle image",
        "Memory allocation failed");
      DestroyImage(despeckle_image);
      return((Image *) NULL);
    }
  /*
    Zero image buffers.
  */
  for (i=0; i < (int) packets; i++)
  {
    red_channel[i]=0;
    green_channel[i]=0;
    blue_channel[i]=0;
    matte_channel[i]=0;
    buffer[i]=0;
  }
  /*
    Copy image pixels to color component buffers
  */
  j=image->columns+2;
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    j++;
    for (x=0; x < (int) image->columns; x++)
    {
      red_channel[j]=p->red;
      green_channel[j]=p->green;
      blue_channel[j]=p->blue;
      matte_channel[j]=p->opacity;
      p++;
      j++;
    }
    j++;
  }
  /*
    Reduce speckle in red channel.
  */
  for (i=0; i < 4; i++)
  {
    ProgressMonitor(DespeckleImageText,i,12);
    Hull(X[i],Y[i],1,image->columns,image->rows,red_channel,buffer);
    Hull(-X[i],-Y[i],1,image->columns,image->rows,red_channel,buffer);
    Hull(-X[i],-Y[i],-1,image->columns,image->rows,red_channel,buffer);
    Hull(X[i],Y[i],-1,image->columns,image->rows,red_channel,buffer);
  }
  /*
    Reduce speckle in green channel.
  */
  for (i=0; i < (int) packets; i++)
    buffer[i]=0;
  for (i=0; i < 4; i++)
  {
    ProgressMonitor(DespeckleImageText,i+4,12);
    Hull(X[i],Y[i],1,image->columns,image->rows,green_channel,buffer);
    Hull(-X[i],-Y[i],1,image->columns,image->rows,green_channel,buffer);
    Hull(-X[i],-Y[i],-1,image->columns,image->rows,green_channel,buffer);
    Hull(X[i],Y[i],-1,image->columns,image->rows,green_channel,buffer);
  }
  /*
    Reduce speckle in blue channel.
  */
  for (i=0; i < (int) packets; i++)
    buffer[i]=0;
  for (i=0; i < 4; i++)
  {
    ProgressMonitor(DespeckleImageText,i+8,12);
    Hull(X[i],Y[i],1,image->columns,image->rows,blue_channel,buffer);
    Hull(-X[i],-Y[i],1,image->columns,image->rows,blue_channel,buffer);
    Hull(-X[i],-Y[i],-1,image->columns,image->rows,blue_channel,buffer);
    Hull(X[i],Y[i],-1,image->columns,image->rows,blue_channel,buffer);
  }
  /*
    Copy color component buffers to despeckled image.
  */
  j=image->columns+2;
  for (y=0; y < (int) image->rows; y++)
  {
    q=SetPixelCache(despeckle_image,0,y,despeckle_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    j++;
    for (x=0; x < (int) image->columns; x++)
    {
      q->red=red_channel[j];
      q->green=green_channel[j];
      q->blue=blue_channel[j];
      q->opacity=matte_channel[j];
      q++;
      j++;
    }
    if (!SyncPixelCache(despeckle_image))
      break;
    j++;
  }
  /*
    Free memory.
  */
  FreeMemory(buffer);
  FreeMemory(matte_channel);
  FreeMemory(blue_channel);
  FreeMemory(green_channel);
  FreeMemory(red_channel);
  return(despeckle_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     E d g e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method EdgeImage creates a new image that is a copy of an existing
%  one with the edges highlighted.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  EdgeImage convolves the pixel neighborhood with this edge detection mask:
%
%    -1 -1 -1
%    -1  W -1
%    -1 -1 -1
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the EdgeImage method is:
%
%      Image *EdgeImage(Image *image,const double factor)
%
%  A description of each parameter follows:
%
%    o edge_image: Method EdgeImage returns a pointer to the image
%      after it is edge.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o factor:  An double value reflecting the percent weight to give to the
%      center pixel of the neighborhood.
%
%
*/
Export Image *EdgeImage(Image *image,const double factor)
{
#define Edge(weight) \
  total_red+=(weight)*s->red; \
  total_green+=(weight)*s->green; \
  total_blue+=(weight)*s->blue; \
  total_opacity+=(weight)*s->opacity; \
  s++;
#define EdgeImageText  "  Detecting image edges...  "

  double
    total_blue,
    total_green,
    total_opacity,
    total_red,
    weight;

  Image
    *edge_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  assert(image != (Image *) NULL);
  if ((image->columns < 3) || (image->rows < 3))
    return((Image *) NULL);
  /*
    Initialize edge image attributes.
  */
  edge_image=CloneImage(image,image->columns,image->rows,False);
  if (edge_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to detect edges",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  edge_image->class=DirectClass;
  /*
    Edge detect image.
  */
  weight=factor/8.0;
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,Min(Max(y-1,0),image->rows-3),image->columns,3);
    q=SetPixelCache(edge_image,0,y,edge_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    /*
      Edge detect this row of pixels.
    */
    *q++=(*(p+image->columns));
    for (x=1; x < (int) (image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0.0;
      total_green=0.0;
      total_blue=0.0;
      total_opacity=0.0;
      s=p;
      Edge(-weight/8); Edge(-weight/8) Edge(-weight/8);
      s=p+image->columns;
      Edge(-weight/8); Edge(weight); Edge(-weight/8);
      s=p+2*image->columns;
      Edge(-weight/8); Edge(-weight/8); Edge(-weight/8);
      q->red=(total_red < 0) ? 0 : (total_red > MaxRGB) ? MaxRGB : total_red;
      q->green=
        (total_green < 0) ? 0 : (total_green > MaxRGB) ? MaxRGB : total_green;
      q->blue=
        (total_blue < 0) ? 0 : (total_blue > MaxRGB) ? MaxRGB : total_blue;
      q->opacity=((total_opacity < Transparent) ? Transparent :
        (total_opacity > Opaque) ? Opaque : total_opacity);
      p++;
      q++;
    }
    p++;
    *q++=(*p);
    if (!SyncPixelCache(edge_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(EdgeImageText,y,image->rows-1);
  }
  /*
    Normalize image.
  */
  NormalizeImage(edge_image);
  return(edge_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     E m b o s s I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method EmbossImage creates a new image that is a copy of an existing
%  one with the edge highlighted.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  EmbossImage convolves the pixel neighborhood with this edge detection mask:
%
%    -1 -2  0
%    -2  0  2
%     0  2  1
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the EmbossImage method is:
%
%      Image *EmbossImage(Image *image)
%
%  A description of each parameter follows:
%
%    o emboss_image: Method EmbossImage returns a pointer to the image
%      after it is embossed.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export Image *EmbossImage(Image *image)
{
#define EmbossImageText  "  Embossing image...  "
#define Emboss(weight) \
  total_red+=(weight)*s->red; \
  total_green+=(weight)*s->green; \
  total_blue+=(weight)*s->blue; \
  s++;

  double
    total_blue,
    total_green,
    total_red;

  Image
    *emboss_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  assert(image != (Image *) NULL);
  if ((image->columns < 3) || (image->rows < 3))
    return((Image *) NULL);
  /*
    Initialize embossed image attributes.
  */
  emboss_image=CloneImage(image,image->columns,image->rows,False);
  if (emboss_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to enhance image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  emboss_image->class=DirectClass;
  /*
    Emboss image.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,Min(Max(y-1,0),image->rows-3),image->columns,3);
    q=SetPixelCache(emboss_image,0,y,emboss_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    /*
      Emboss this row of pixels.
    */
    *q++=(*(p+image->columns));
    for (x=1; x < (int) (image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0.0;
      total_green=0.0;
      total_blue=0.0;
      s=p;
      Emboss(-1); Emboss(-2); Emboss( 0);
      s=p+image->columns;
      Emboss(-2); Emboss( 0); Emboss( 2);
      s=p+2*image->columns;
      Emboss( 0); Emboss( 2); Emboss( 1);
      total_red+=(MaxRGB+1)/2;
      q->red=(total_red < 0) ? 0 : (total_red > MaxRGB) ? MaxRGB : total_red;
      total_green+=(MaxRGB+1)/2;
      q->green=
        (total_green < 0) ? 0 : (total_green > MaxRGB) ? MaxRGB : total_green;
      total_blue+=(MaxRGB+1)/2;
      q->blue=
        (total_blue < 0) ? 0 : (total_blue > MaxRGB) ? MaxRGB : total_blue;
      q->opacity=(p+image->columns)->opacity;
      p++;
      q++;
    }
    p++;
    *q++=(*p);
    if (!SyncPixelCache(emboss_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(EmbossImageText,y,image->rows-1);
  }
  /*
    Convert image to grayscale and normalize.
  */
  emboss_image->class=DirectClass;
  (void) IsGrayImage(emboss_image);
  NormalizeImage(emboss_image);
  return(emboss_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     E n h a n c e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method EnhanceImage creates a new image that is a copy of an existing
%  one with the noise minified.  It allocates the memory necessary for the new
%  Image structure and returns a pointer to the new image.
%
%  EnhanceImage does a weighted average of pixels in a 5x5 cell around each
%  target pixel.  Only pixels in the 5x5 cell that are within a RGB distance
%  threshold of the target pixel are averaged.
%
%  Weights assume that the importance of neighboring pixels is negately
%  proportional to the square of their distance from the target pixel.
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the EnhanceImage method is:
%
%      Image *EnhanceImage(Image *image)
%
%  A description of each parameter follows:
%
%    o enhance_image: Method EnhanceImage returns a pointer to the image
%      after it is enhanced.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export Image *EnhanceImage(Image *image)
{
#define Enhance(weight) \
  mean=(int) (s->red+red)/2; \
  distance=s->red-(int) red; \
  distance_squared= \
    (2.0*(MaxRGB+1)+mean)*squares[distance]/(double) (MaxRGB+1); \
  distance=s->green-(int) green; \
  distance_squared+=4.0*squares[distance]; \
  distance=s->blue-(int) blue; \
  distance_squared+= \
    (3.0*(MaxRGB+1)-1.0-mean)*squares[distance]/(double) (MaxRGB+1); \
  if (distance_squared < Threshold) \
    { \
      total_red+=(weight)*s->red; \
      total_green+=(weight)*s->green; \
      total_blue+=(weight)*s->blue; \
      total_weight+=(weight); \
    } \
  s++;
#define EnhanceImageText  "  Enhancing image...  "
#define Threshold  2500

  double
    distance_squared,
    mean,
    total_blue,
    total_green,
    total_red,
    total_weight;

  Image
    *enhance_image;

  int
    y;

  long
    distance;

  Quantum
    blue,
    green,
    red;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  register unsigned int
    *squares;

  assert(image != (Image *) NULL);
  if ((image->columns < 5) || (image->rows < 5))
    return((Image *) NULL);
  /*
    Initialize enhanced image attributes.
  */
  enhance_image=CloneImage(image,image->columns,image->rows,False);
  if (enhance_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to enhance image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  enhance_image->class=DirectClass;
  /*
    Allocate the squares buffer.
  */
  squares=(unsigned int *)
    AllocateMemory((MaxRGB+MaxRGB+1)*sizeof(unsigned int));
  if (squares == (unsigned int *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to enhance image",
        "Memory allocation failed");
      DestroyImage(enhance_image);
      return((Image *) NULL);
    }
  squares+=MaxRGB;
  for (i=(-MaxRGB); i <= MaxRGB; i++)
    squares[i]=i*i;
  /*
    Enhance image.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    /*
      Read another scan line.
    */
    p=GetPixelCache(image,0,Min(Max(y-2,0),image->rows-5),image->columns,5);
    q=SetPixelCache(enhance_image,0,y,enhance_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    /*
      Transfer first 2 pixels of the scanline.
    */
    *q++=(*(p+2*image->columns));
    *q++=(*(p+2*image->columns+1));
    for (x=2; x < (int) (image->columns-2); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0.0;
      total_green=0.0;
      total_blue=0.0;
      total_weight=0.0;
      s=p+2*image->columns+2;
      red=s->red;
      green=s->green;
      blue=s->blue;
      s=p;
      Enhance(5);  Enhance(8);  Enhance(10); Enhance(8);  Enhance(5);
      s=p+image->columns;
      Enhance(8);  Enhance(20); Enhance(40); Enhance(20); Enhance(8);
      s=p+2*image->columns;
      Enhance(10); Enhance(40); Enhance(80); Enhance(40); Enhance(10);
      s=p+3*image->columns;
      Enhance(8);  Enhance(20); Enhance(40); Enhance(20); Enhance(8);
      s=p+4*image->columns;
      Enhance(5);  Enhance(8);  Enhance(10); Enhance(8);  Enhance(5);
      q->red=(total_red+(total_weight/2)-1)/total_weight;
      q->green=(total_green+(total_weight/2)-1)/total_weight;
      q->blue=(total_blue+(total_weight/2)-1)/total_weight;
      q->opacity=(p+2*image->columns)->opacity;
      q++;
    }
    p++;
    *q++=(*p);
    p++;
    *q++=(*p);
    if (!SyncPixelCache(enhance_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(EnhanceImageText,y,image->rows-2);
  }
  /*
    Free memory resources.
  */
  squares-=MaxRGB;
  FreeMemory(squares);
  return(enhance_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     I m p l o d e I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ImplodeImage creates a new image that is a copy of an existing
%  one with the image pixels "implode" by the specified percentage.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ImplodeImage method is:
%
%      Image *ImplodeImage(Image *image,const double factor)
%
%  A description of each parameter follows:
%
%    o implode_image: Method ImplodeImage returns a pointer to the image
%      after it is implode.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o factor:  A double value that defines the extent of the implosion.
%
%
*/
Export Image *ImplodeImage(Image *image,const double factor)
{
#define ImplodeImageText  "  Imploding image...  "

  double
    amount,
    distance,
    radius,
    x_center,
    x_distance,
    x_scale,
    y_center,
    y_distance,
    y_scale;

  Image
    *implode_image;

  int
    y;

  register PixelPacket
    *p,
    *q;

  register unsigned int
    x;

  /*
    Initialize implode image attributes.
  */
  assert(image != (Image *) NULL);
  if (!image->matte)
    MatteImage(image,Opaque);
  implode_image=CloneImage(image,image->columns,image->rows,False);
  if (implode_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to implode image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Compute scaling factor.
  */
  x_scale=1.0;
  y_scale=1.0;
  x_center=(double) 0.5*image->columns;
  y_center=(double) 0.5*image->rows;
  radius=x_center;
  if (image->columns > image->rows)
    y_scale=(double) image->columns/image->rows;
  else
    if (image->columns < image->rows)
      {
        x_scale=(double) image->rows/image->columns;
        radius=y_center;
      }
  amount=factor/10.0;
  if (amount >= 0)
    amount/=10.0;
  /*
    Implode each row.
  */
  for (y=0; y < image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    q=SetPixelCache(implode_image,0,y,implode_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    y_distance=y_scale*(y-y_center);
    for (x=0; x < image->columns; x++)
    {
      /*
        Determine if the pixel is within an ellipse.
      */
      *q=(*p);
      x_distance=x_scale*(x-x_center);
      distance=x_distance*x_distance+y_distance*y_distance;
      if (distance < (radius*radius))
        {
          double
            factor;

          /*
            Implode the pixel.
          */
          factor=1.0;
          if (distance > 0.0)
            factor=
              pow(sin(0.5000000000000001*M_PI*sqrt(distance)/radius),-amount);
          *q=InterpolateColor(image,factor*x_distance/x_scale+x_center,
            factor*y_distance/y_scale+y_center);
        }
      p++;
      q++;
    }
    if (!SyncPixelCache(implode_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(ImplodeImageText,y,image->rows);
  }
  return(implode_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     M e d i a n F i l t e r I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method MedianFilterImage creates a new image that is a copy of an existing
%  one with each pixel component replaced with the median color in a
%  circular neighborhood.
%
%  The format of the MedianFilterImage method is:
%
%      Image *MedianFilterImage(Image *image,const unsigned int radius)
%
%  A description of each parameter follows:
%
%    o median_image: Method MedianFilterImage returns a pointer to the image
%      after it is `filtered'.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o radius: An unsigned int that is the radius of the circular
%      neighborhood.
%
%
*/

static int MedianCompare(const void *x,const void *y)
{
  PixelPacket
    *color_1,
    *color_2;

  color_1=(PixelPacket *) x;
  color_2=(PixelPacket *) y;
  return((int) Intensity(*color_1)-(int) Intensity(*color_2));
}

Export Image *MedianFilterImage(Image *image,const unsigned int radius)
{
#define MedianFilterImageText  "  Filtering image with neighborhood ranking...  "

  Image
    *median_image;

  int
    y;

  PixelPacket
    *neighbors;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q,
    *s,
    *t;

  unsigned int
    length;

  assert(image != (Image *) NULL);
  if ((image->columns < (2*radius+1)) || (image->rows < (2*radius+1)))
    {
      MagickWarning(ResourceLimitWarning,"Unable to median filter",
        "image smaller than radius");
      return((Image *) NULL);
    }
  /*
    Initialize median image attributes.
  */
  median_image=CloneImage(image,image->columns,image->rows,False);
  if (median_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to reduce noise",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  median_image->class=DirectClass;
  /*
    Allocate neighbors and scanline.
  */
  length=M_PI*(radius+1)*(radius+1)*sizeof(PixelPacket);
  neighbors=(PixelPacket *) AllocateMemory(length*sizeof(PixelPacket));
  if (neighbors == (PixelPacket *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to reduce noise",
        "Memory allocation failed");
      DestroyImage(median_image);
      return((Image *) NULL);
    }
  /*
    Paint each row of the image.
  */
  for (y=radius; y < (int) (image->rows-radius); y++)
  {
    p=GetPixelCache(image,0,y-radius,image->columns,2*radius+1);
    q=GetPixelCache(median_image,0,y,median_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    p+=radius*image->columns+radius;
    q+=radius;
    for (x=radius; x < (int) (image->columns-radius); x++)
    {
      /*
        Determine most frequent color.
      */
      t=neighbors;
      for (i=0; i < (int) radius; i++)
      {
        s=p-(radius-i)*image->columns-i-1;
        (void) memcpy(t,s,(2*i+1)*sizeof(PixelPacket));
        t+=2*i+1;
        s=p+(radius-i)*image->columns-i-1;
        (void) memcpy(t,s,(2*i+1)*sizeof(PixelPacket));
        t+=2*i+1;
      }
      s=p-radius;
      (void) memcpy(t,s,(radius+radius+1)*sizeof(PixelPacket));
      t+=radius+radius+1;
      qsort((void *) neighbors,t-neighbors,sizeof(PixelPacket),
        (int (*)(const void *, const void *)) MedianCompare);
      t-=(t-neighbors)/2;
      *q=(*t);
      p++;
      q++;
    }
    if (!SyncPixelCache(median_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(MedianFilterImageText,y,image->rows);
  }
  FreeMemory(neighbors);
  return(median_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     M o r p h I m a g e s                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method MorphImages morphs a set of images.  Both the image pixels and size
%  are linearly interpolated to give the appearance of a meta-morphosis from
%  one image to the next.
%
%  The format of the MorphImage method is:
%
%      Image *MorphImages(Image *images,const unsigned int number_frames)
%
%  A description of each parameter follows:
%
%    o morph_image: Method MorphImages returns an image sequence that
%      has linearly interpolated pixels and size between two input images.
%
%    o images: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o number_frames:  This unsigned integer reflects the number of in-between
%      images to generate.  The more in-between frames, the smoother
%      the morph.
%
%
*/
Export Image *MorphImages(Image *images,const unsigned int number_frames)
{
#define MorphImageText  "  Morphing image sequence...  "

  double
    alpha,
    beta;

  Image
    *image,
    *morph_image,
    *morph_images;

  int
    y;

  MonitorHandler
    handler;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q;

  unsigned int
    scene;

  assert(images != (Image *) NULL);
  if (images->next == (Image *) NULL)
    {
      MagickWarning(OptionWarning,"Unable to morph image",
        "image sequence required");
      return((Image *) NULL);
    }
  /*
    Clone first frame in sequence.
  */
  morph_images=CloneImage(images,images->columns,images->rows,True);
  if (morph_images == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to morph image sequence",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Morph image.
  */
  scene=0;
  for (image=images; image->next != (Image *) NULL; image=image->next)
  {
    handler=SetMonitorHandler((MonitorHandler) NULL);
    for (i=0; i < (int) number_frames; i++)
    {
      beta=(double) (i+1.0)/(number_frames+1.0);
      alpha=1.0-beta;
      image->orphan=True;
      morph_images->next=ZoomImage(image,
        (unsigned int) (alpha*image->columns+beta*image->next->columns+0.5),
        (unsigned int) (alpha*image->rows+beta*image->next->rows+0.5));
      if (morph_images->next == (Image *) NULL)
        {
          MagickWarning(ResourceLimitWarning,"Unable to morph image sequence",
            "Memory allocation failed");
          break;
        }
      morph_images->next->previous=morph_images;
      morph_images=morph_images->next;
      image->next->orphan=True;
      morph_image=
        ZoomImage(image->next,morph_images->columns,morph_images->rows);
      if (morph_image == (Image *) NULL)
        {
          MagickWarning(ResourceLimitWarning,"Unable to morph image sequence",
            "Memory allocation failed");
          break;
        }
      morph_images->class=DirectClass;
      for (y=0; y < (int) morph_images->rows; y++)
      {
        p=GetPixelCache(morph_image,0,y,morph_image->columns,1);
        q=GetPixelCache(morph_images,0,y,morph_images->columns,1);
        if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
          break;
        for (x=0; x < (int) morph_images->columns; x++)
        {
          q->red=(alpha*q->red+beta*p->red+0.5);
          q->green=(alpha*q->green+beta*p->green+0.5);
          q->blue=(alpha*q->blue+beta*p->blue+0.5);
          q->opacity=(alpha*q->opacity+beta*p->opacity+0.5);
          p++;
          q++;
        }
        if (!SyncPixelCache(morph_images))
          break;
      }
      DestroyImage(morph_image);
    }
    /*
      Clone last frame in sequence.
    */
    morph_images->next=
      CloneImage(image->next,image->next->columns,image->next->rows,True);
    if (morph_images->next == (Image *) NULL)
      {
        MagickWarning(ResourceLimitWarning,"Unable to morph image sequence",
          "Memory allocation failed");
        break;
      }
    morph_images->next->previous=morph_images;
    morph_images=morph_images->next;
    (void) SetMonitorHandler(handler);
    ProgressMonitor(MorphImageText,scene,GetNumberScenes(images));
    scene++;
  }
  while (morph_images->previous != (Image *) NULL)
    morph_images=morph_images->previous;
  if (image->next != (Image *) NULL)
    {
      DestroyImages(morph_images);
      return((Image *) NULL);
    }
  return(morph_images);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     O i l P a i n t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method OilPaintImage creates a new image that is a copy of an existing
%  one with each pixel component replaced with the color of greatest frequency
%  in a circular neighborhood.
%
%  The format of the OilPaintImage method is:
%
%      Image *OilPaintImage(Image *image,const unsigned int radius)
%
%  A description of each parameter follows:
%
%    o paint_image: Method OilPaintImage returns a pointer to the image
%      after it is `painted'.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o radius: An unsigned int that is the radius of the circular
%      neighborhood.
%
%
*/
Export Image *OilPaintImage(Image *image,const unsigned int radius)
{
#define OilPaintImageText  "  Oil painting image...  "

  Image
    *paint_image;

  int
    count,
    j,
    k,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  unsigned int
    *histogram;

  assert(image != (Image *) NULL);
  if ((image->columns < (2*radius+1)) || (image->rows < (2*radius+1)))
    {
      MagickWarning(ResourceLimitWarning,"Unable to oil paint",
        "image smaller than radius");
      return((Image *) NULL);
    }
  /*
    Initialize painted image attributes.
  */
  paint_image=CloneImage(image,image->columns,image->rows,False);
  if (paint_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to oil paint",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  paint_image->class=DirectClass;
  /*
    Allocate histogram and scanline.
  */
  histogram=(unsigned int *) AllocateMemory((MaxRGB+1)*sizeof(unsigned int));
  if (histogram == (unsigned int *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to oil paint",
        "Memory allocation failed");
      DestroyImage(paint_image);
      return((Image *) NULL);
    }
  /*
    Paint each row of the image.
  */
  k=0;
  for (y=radius; y < (int) (image->rows-radius-1); y++)
  {
    p=GetPixelCache(image,0,y-radius,image->columns,2*radius+1);
    q=SetPixelCache(paint_image,0,y,paint_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    p+=radius*image->columns+radius;
    q+=radius;
    for (x=radius; x < (int) (image->columns-radius); x++)
    {
      /*
        Determine most frequent color.
      */
      count=0;
      for (i=0; i < (int) (MaxRGB+1); i++)
        histogram[i]=0;
      for (i=0; i < (int) radius; i++)
      {
        s=p-(radius-i)*image->columns-i-1;
        for (j=0; j < (2*i+1); j++)
        {
          k=Intensity(*s);
          histogram[k]++;
          if ((int) histogram[k] > count)
            {
              *q=(*s);
              count=histogram[k];
            }
          s++;
        }
        s=p+(radius-i)*image->columns-i-1;
        for (j=0; j < (2*i+1); j++)
        {
          k=Intensity(*s);
          histogram[k]++;
          if ((int) histogram[k] > count)
            {
              *q=(*s);
              count=histogram[k];
            }
          s++;
        }
      }
      s=p-radius;
      for (j=0; j < (int) (radius+radius+1); j++)
      {
        k=Intensity(*s);
        histogram[k]++;
        if ((int) histogram[k] > count)
          {
            *q=(*s);
            count=histogram[k];
          }
        s++;
      }
      p++;
      q++;
    }
    if (!SyncPixelCache(paint_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(OilPaintImageText,y,image->rows);
  }
  FreeMemory(histogram);
  return(paint_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     P l a s m a I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method PlasmaImage initializes an image with plasma fractal values.  The
%  image must be initialized with a base color and the random number generator
%  seeded before this method is called.
%
%  The format of the PlasmaImage method is:
%
%      unsigned int PlasmaImage(Image *image,const SegmentInfo *segment,
%        int attenuate,int depth)
%
%  A description of each parameter follows:
%
%    o status: Method PlasmaImage returns True when the fractal process
%      is complete.  Otherwise False is returned.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o segment:  specifies a structure of type SegmentInfo that defines
%      the boundaries of the area where the plasma fractals are applied.
%
%    o attenuate:  specifies the plasma attenuation factor.
%
%    o depth: this integer values define the plasma recursion depth.
%
%
*/

static Quantum PlasmaPixel(const double pixel,const double noise)
{
  double
    value;

  value=pixel+(noise/2.0)-((int) noise ? (rand() % (int) noise) : 0.0);
  if (value < 0.0)
    return(0);
  if (value > MaxRGB)
    return(MaxRGB);
  return((Quantum) (value+0.5));
}

Export unsigned int PlasmaImage(Image *image,const SegmentInfo *segment,
  int attenuate,int depth)
{
  double
    plasma,
    x_mid,
    y_mid;

  PixelPacket
    pixel_1,
    pixel_2;

  register PixelPacket
    *q;

  assert(image != (Image *) NULL);
  if (depth != 0)
    {
      SegmentInfo
        local_info;

      /*
        Divide the area into quadrants and recurse.
      */
      depth--;
      attenuate++;
      x_mid=(segment->x1+segment->x2)/2;
      y_mid=(segment->y1+segment->y2)/2;
      local_info=(*segment);
      local_info.x2=x_mid;
      local_info.y2=y_mid;
      (void) PlasmaImage(image,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.y1=y_mid;
      local_info.x2=x_mid;
      (void) PlasmaImage(image,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.x1=x_mid;
      local_info.y2=y_mid;
      (void) PlasmaImage(image,&local_info,attenuate,depth);
      local_info=(*segment);
      local_info.x1=x_mid;
      local_info.y1=y_mid;
      return(PlasmaImage(image,&local_info,attenuate,depth));
    }
  x_mid=(segment->x1+segment->x2)/2;
  y_mid=(segment->y1+segment->y2)/2;
  if ((segment->x1 == x_mid) && (segment->x2 == x_mid) &&
      (segment->y1 == y_mid) && (segment->y2 == y_mid))
    return(False);
  /*
    Average pixels and apply plasma.
  */
  plasma=(MaxRGB+1)/(2.0*(double) attenuate);
  if ((segment->x1 != x_mid) || (segment->x2 != x_mid))
    {
      /*
        Left pixel.
      */
      q=GetPixelCache(image,(int) segment->x1,(int) segment->y1,1,1);
      if (q != (PixelPacket *) NULL)
        pixel_1=(*q);
      q=GetPixelCache(image,(int) segment->x1,(int) segment->y2,1,1);
      if (q != (PixelPacket *) NULL)
        pixel_2=(*q);
      q=SetPixelCache(image,(int) segment->x1,(int) y_mid,1,1);
      if (q != (PixelPacket *) NULL)
        {
          q->red=PlasmaPixel((int) (pixel_1.red+pixel_2.red)/2,plasma);
          q->green=PlasmaPixel((int) (pixel_1.green+pixel_2.green)/2,plasma);
          q->blue=PlasmaPixel((int) (pixel_1.blue+pixel_2.blue)/2,plasma);
          (void) SyncPixelCache(image);
        }
      if (segment->x1 != segment->x2)
        {
          /*
            Right pixel.
          */
          q=GetPixelCache(image,(int) segment->x2,(int) segment->y1,1,1);
          if (q != (PixelPacket *) NULL)
            pixel_1=(*q);
          q=GetPixelCache(image,(int) segment->x2,(int) segment->y2,1,1);
          if (q != (PixelPacket *) NULL)
            pixel_2=(*q);
          q=SetPixelCache(image,(int) segment->x2,(int) y_mid,1,1);
          if (q != (PixelPacket *) NULL)
            {
              q->red=PlasmaPixel((int) (pixel_1.red+pixel_2.red)/2,plasma);
              q->green=
                PlasmaPixel((int) (pixel_1.green+pixel_2.green)/2,plasma);
              q->blue=PlasmaPixel((int) (pixel_1.blue+pixel_2.blue)/2,plasma);
              (void) SyncPixelCache(image);
            }
        }
    }
  if ((segment->y1 != y_mid) || (segment->y2 != y_mid))
    {
      if ((segment->x1 != x_mid) || (segment->y2 != y_mid))
        {
          /*
            Bottom pixel.
          */
          q=GetPixelCache(image,(int) segment->x1,(int) segment->y2,1,1);
          if (q != (PixelPacket *) NULL)
            pixel_1=(*q);
          q=GetPixelCache(image,(int) segment->x2,(int) segment->y2,1,1);
          if (q != (PixelPacket *) NULL)
            pixel_2=(*q);
          q=SetPixelCache(image,(int) x_mid,(int) segment->y2,1,1);
          if (q != (PixelPacket *) NULL)
            {
              q->red=PlasmaPixel((int) (pixel_1.red+pixel_2.red)/2,plasma);
              q->green=
                PlasmaPixel((int) (pixel_1.green+pixel_2.green)/2,plasma);
              q->blue=PlasmaPixel((int) (pixel_1.blue+pixel_2.blue)/2,plasma);
              (void) SyncPixelCache(image);
            }
        }
      if (segment->y1 != segment->y2)
        {
          /*
            Top pixel.
          */
          q=GetPixelCache(image,(int) segment->x1,(int) segment->y1,1,1);
          if (q != (PixelPacket *) NULL)
            pixel_1=(*q);
          q=GetPixelCache(image,(int) segment->x2,(int) segment->y1,1,1);
          if (q != (PixelPacket *) NULL)
            pixel_2=(*q);
          q=SetPixelCache(image,(int) x_mid,(int) segment->y1,1,1);
          if (q != (PixelPacket *) NULL)
            {
              q->red=PlasmaPixel((int) (pixel_1.red+pixel_2.red)/2,plasma);
              q->green=
                PlasmaPixel((int) (pixel_1.green+pixel_2.green)/2,plasma);
              q->blue=PlasmaPixel((int) (pixel_1.blue+pixel_2.blue)/2,plasma);
              (void) SyncPixelCache(image);
            }
        }
    }
  if ((segment->x1 != segment->x2) ||
      (segment->y1 != segment->y2))
    {
      /*
        Middle pixel.
      */
      q=GetPixelCache(image,(int) segment->x1,(int) segment->y1,1,1);
      if (q != (PixelPacket *) NULL)
        pixel_1=(*q);
      q=GetPixelCache(image,(int) segment->x2,(int) segment->y2,1,1);
      if (q != (PixelPacket *) NULL)
        pixel_2=(*q);
      q=SetPixelCache(image,(int) x_mid,(int) y_mid,1,1);
      if (q != (PixelPacket *) NULL)
        {
          q->red=PlasmaPixel((int) (pixel_1.red+pixel_2.red)/2,plasma);
          q->green=PlasmaPixel((int) (pixel_1.green+pixel_2.green)/2,plasma);
          q->blue=PlasmaPixel((int) (pixel_1.blue+pixel_2.blue)/2,plasma);
        }
      (void) SyncPixelCache(image);
    }
  if (((segment->x2-segment->x1) < 3.0) && ((segment->y2-segment->y1) < 3.0))
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     R e d u c e N o i s e I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReduceNoiseImage creates a new image that is a copy of an existing
%  one with the noise minified with a noise peak elimination filter.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The principal function of noise peak elimination filter is to smooth the
%  objects within an image without losing edge information and without
%  creating undesired structures.  The central idea of the algorithm is to
%  replace a pixel with its next neighbor in value within a 3 x 3 window,
%  if this pixel has been found to be noise.  A pixel is defined as noise
%  if and only if this pixel is a maximum or minimum within the 3 x 3
%  window.
%
%  The format of the ReduceNoiseImage method is:
%
%      Image *ReduceNoiseImage(Image *image)
%
%  A description of each parameter follows:
%
%    o noisy_image: Method ReduceNoiseImage returns a pointer to the image
%      after the noise is minified.  A null image is returned if there is a
%      memory shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/

static int ReduceNoiseCompare(const void *x,const void *y)
{
  PixelPacket
    *color_1,
    *color_2;

  color_1=(PixelPacket *) x;
  color_2=(PixelPacket *) y;
  return((int) Intensity(*color_1)-(int) Intensity(*color_2));
}

Export Image *ReduceNoiseImage(Image *image)
{
#define ReduceNoiseImageText  "  Reducing the image noise...  "

  Image
    *noisy_image;

  int
    y;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  PixelPacket
    pixel,
    window[9];

  assert(image != (Image *) NULL);
  if ((image->columns < 3) || (image->rows < 3))
    return((Image *) NULL);
  /*
    Initialize noisy image attributes.
  */
  noisy_image=CloneImage(image,image->columns,image->rows,False);
  if (noisy_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to reduce noise",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  noisy_image->class=DirectClass;
  /*
    Reduce noise in image.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    /*
      Read another scan line.
    */
    p=GetPixelCache(image,0,Min(Max(y-1,0),image->rows-3),image->columns,3);
    q=SetPixelCache(noisy_image,0,y,noisy_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    /*
      Reduce noise in this row.
    */
    *q++=(*(p+image->columns));
    for (x=1; x < (int) (image->columns-1); x++)
    {
      /*
        Sort window pixels by increasing intensity.
      */
      s=p;
      window[0]=(*s++);
      window[1]=(*s++);
      window[2]=(*s++);
      s=p+image->columns;
      window[3]=(*s++);
      window[4]=(*s++);
      window[5]=(*s++);
      s=p+2*image->columns;
      window[6]=(*s++);
      window[7]=(*s++);
      window[8]=(*s++);
      pixel=window[4];
      qsort((void *) window,9,sizeof(PixelPacket),
        (int (*)(const void *, const void *)) ReduceNoiseCompare);
      if (Intensity(pixel) == Intensity(window[0]))
        {
          /*
            Pixel is minimum noise; replace with next neighbor in value.
          */
          for (i=1; i < 8; i++)
            if (Intensity(window[i]) != Intensity(window[0]))
              break;
          pixel=window[i];
        }
      else
        if (Intensity(pixel) == Intensity(window[8]))
          {
            /*
              Pixel is maximum noise; replace with next neighbor in value.
            */
            for (i=7; i > 0; i--)
              if (Intensity(window[i]) != Intensity(window[8]))
                break;
            pixel=window[i];
          }
      p++;
      q++;
    }
    p++;
    *q++=(*p);
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(ReduceNoiseImageText,y,image->rows-1);
  }
  return(noisy_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S h a d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ShadeImage creates a new image that is a copy of an existing
%  one with the image pixels shaded using a distance light source.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ShadeImage method is:
%
%      Image *ShadeImage(Image *image,const unsigned int color_shading,
%        double azimuth,double elevation)
%
%  A description of each parameter follows:
%
%    o shade_image: Method ShadeImage returns a pointer to the image
%      after it is shaded.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o color_shading: A value other than zero shades the red, green, and blue
%      components of the image.
%
%    o azimuth, elevation:  A double value that indicates the light source
%      direction.
%
%
*/
Export Image *ShadeImage(Image *image,const unsigned int color_shading,
  double azimuth,double elevation)
{
#define ShadeImageText  "  Shading image...  "

  double
    distance,
    normal_distance,
    shade;

  Image
    *shade_image;

  int
    y;

  PointInfo
    light,
    normal;

  register int
    x;

  register PixelPacket
    *p,
    *s0,
    *s1,
    *s2,
    *q;

  /*
    Initialize shaded image attributes.
  */
  assert(image != (Image *) NULL);
  shade_image=CloneImage(image,image->columns,image->rows,False);
  if (shade_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to shade image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  shade_image->class=DirectClass;
  /*
    Compute the light vector.
  */
  azimuth=DegreesToRadians(azimuth);
  elevation=DegreesToRadians(elevation);
  light.x=MaxRGB*cos(azimuth)*cos(elevation);
  light.y=MaxRGB*sin(azimuth)*cos(elevation);
  light.z=MaxRGB*sin(elevation);
  normal.z=2*MaxRGB;  /* constant Z of surface normal */
  /*
    Shade image.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,Min(Max(y-1,0),image->rows-3),image->columns,3);
    q=SetPixelCache(shade_image,0,y,shade_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    /*
      Shade this row of pixels.
    */
    *q++=(*(p+image->columns));
    p++;
    s0=p;
    s1=p+image->columns;
    s2=p+2*image->columns;
    for (x=1; x < (int) (image->columns-1); x++)
    {
      /*
        Determine the surface normal and compute shading.
      */
      normal.x=Intensity(*(s0-1))+Intensity(*(s1-1))+Intensity(*(s2-1))-
        (double) Intensity(*(s0+1))-(double) Intensity(*(s1+1))-
        (double) Intensity(*(s2+1));
      normal.y=Intensity(*(s2-1))+Intensity(*s2)+Intensity(*(s2+1))-
        (double) Intensity(*(s0-1))-(double) Intensity(*s0)-
        (double) Intensity(*(s0+1));
      if ((normal.x == 0) && (normal.y == 0))
        shade=light.z;
      else
        {
          shade=0.0;
          distance=normal.x*light.x+normal.y*light.y+normal.z*light.z;
          if (distance > 0.0)
            {
              normal_distance=
                normal.x*normal.x+normal.y*normal.y+normal.z*normal.z;
              if (AbsoluteValue(normal_distance) > 0.0000001)
                shade=distance/sqrt(normal_distance);
            }
        }
      if (!color_shading)
        {
          q->red=shade;
          q->green=shade;
          q->blue=shade;
        }
      else
        {
          q->red=(shade*s1->red)/(MaxRGB+1);
          q->green=(shade*s1->green)/(MaxRGB+1);
          q->blue=(shade*s1->blue)/(MaxRGB+1);
        }
      q->opacity=s1->opacity;
      s0++;
      s1++;
      s2++;
      q++;
    }
    *q++=(*s1);
    if (!SyncPixelCache(shade_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(ShadeImageText,y,image->rows);
  }
  return(shade_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S h a r p e n I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SharpenImage creates a new image that is a copy of an existing
%  one with the pixels sharpened.  It allocates the memory necessary for the
%  new Image structure and returns a pointer to the new image.
%
%  SharpenImage convolves the pixel neighborhood with this sharpening mask:
%
%    -1 -2 -1
%    -2  W -2
%    -1 -2 -1
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the SharpenImage method is:
%
%      Image *SharpenImage(Image *image,const double factor)
%
%  A description of each parameter follows:
%
%    o sharpen_image: Method SharpenImage returns a pointer to the image
%      after it is sharpened.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o factor:  An double value reflecting the percent weight to give to the
%      center pixel of the neighborhood.
%
%
*/
Export Image *SharpenImage(Image *image,const double factor)
{
#define Sharpen(weight) \
  total_red+=(weight)*s->red; \
  total_green+=(weight)*s->green; \
  total_blue+=(weight)*s->blue; \
  total_opacity+=(weight)*s->opacity; \
  s++;
#define SharpenImageText  "  Sharpening image...  "

  double
    quantum,
    total_blue,
    total_green,
    total_opacity,
    total_red,
    weight;

  Image
    *sharpen_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  assert(image != (Image *) NULL);
  if ((image->columns < 3) || (image->rows < 3))
    return((Image *) NULL);
  /*
    Initialize sharpened image attributes.
  */
  sharpen_image=CloneImage(image,image->columns,image->rows,False);
  if (sharpen_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to sharpen image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  sharpen_image->class=DirectClass;
  /*
    Sharpen image.
  */
  weight=((100.0-factor)/2.0+13.0);
  quantum=Max(weight-12.0,1.0);
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,Min(Max(y-1,0),image->rows-3),image->columns,3);
    q=SetPixelCache(sharpen_image,0,y,sharpen_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    /*
      Sharpen this row of pixels.
    */
    *q++=(*(p+image->columns));
    for (x=1; x < (int) (image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0.0;
      total_green=0.0;
      total_blue=0.0;
      total_opacity=0.0;
      s=p;
      Sharpen(-1); Sharpen(-2); Sharpen(-1);
      s=p+image->columns;
      Sharpen(-2); Sharpen(weight); Sharpen(-2);
      s=p+2*image->columns;
      Sharpen(-1); Sharpen(-2); Sharpen(-1);
      if (total_red < 0)
        q->red=0;
      else
        if (total_red > (int) (MaxRGB*quantum))
          q->red=MaxRGB;
        else
          q->red=(Quantum) ((total_red+(quantum/2.0))/quantum);
      if (total_green < 0)
        q->green=0;
      else
        if (total_green > (int) (MaxRGB*quantum))
          q->green=MaxRGB;
        else
          q->green=(Quantum) ((total_green+(quantum/2.0))/quantum);
      if (total_blue < 0)
        q->blue=0;
      else
        if (total_blue > (int) (MaxRGB*quantum))
          q->blue=MaxRGB;
        else
          q->blue=(Quantum) ((total_blue+(quantum/2.0))/quantum);
      if (total_opacity < 0)
        q->opacity=0;
      else
        if (total_opacity > (int) (MaxRGB*quantum))
          q->opacity=MaxRGB;
        else
          q->opacity=(Quantum) ((total_opacity+(quantum/2.0))/quantum);
      p++;
      q++;
    }
    p++;
    *q++=(*p);
    if (!SyncPixelCache(sharpen_image))
      break;
    if (QuantumTick(y,image->rows-1))
      ProgressMonitor(SharpenImageText,y,image->rows-1);
  }
  return(sharpen_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S o l a r i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SolarizeImage produces a 'solarization' effect seen when exposing
%  a photographic film to light during the development process.
%
%  The format of the SolarizeImage method is:
%
%      void SolarizeImage(Image *image,const double factor)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o factor:  An double value that defines the extent of the solarization.
%
%
*/
Export void SolarizeImage(Image *image,const double factor)
{
#define SolarizeImageText  "  Solarizing the image colors...  "

  int
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  unsigned int
    threshold;

  assert(image != (Image *) NULL);
  threshold=(unsigned int) (factor*(MaxRGB+1)/100.0);
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      /*
        Solarize DirectClass packets.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          q->red=q->red > threshold ? MaxRGB-q->red : q->red;
          q->green=q->green > threshold ? MaxRGB-q->green : q->green;
          q->blue=q->blue > threshold ? MaxRGB-q->blue : q->blue;
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(SolarizeImageText,y,image->rows);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Solarize PseudoClass packets.
      */
      for (i=0; i < (int) image->colors; i++)
      {
        image->colormap[i].red=image->colormap[i].red > threshold ?
          MaxRGB-image->colormap[i].red : image->colormap[i].red;
        image->colormap[i].green=image->colormap[i].green > threshold ?
          MaxRGB-image->colormap[i].green : image->colormap[i].green;
        image->colormap[i].blue=image->colormap[i].blue > threshold ?
          MaxRGB-image->colormap[i].blue : image->colormap[i].blue;
      }
      SyncImage(image);
      break;
    }
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S p r e a d I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SpreadImage creates a new image that is a copy of an existing
%  one with the image pixels randomly displaced.  It allocates the memory
%  necessary for the new Image structure and returns a pointer to the new
%  image.
%
%  The format of the SpreadImage method is:
%
%      Image *SpreadImage(Image *image,const unsigned int amount)
%
%  A description of each parameter follows:
%
%    o spread_image: Method SpreadImage returns a pointer to the image
%      after it is spread.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o amount:  An unsigned value constraining the "vicinity" for choosing
%      a random pixel to swap.
%
%
*/
Export Image *SpreadImage(Image *image,const unsigned int amount)
{
#define SpreadImageText  "  Spreading image...  "

  Image
    *spread_image;

  int
    quantum,
    y;

  long
    x_distance,
    y_distance;

  register int
    x;

  register PixelPacket
    *p,
    *q;

  assert(image != (Image *) NULL);
  if ((image->columns < 3) || (image->rows < 3))
    return((Image *) NULL);
  /*
    Initialize spread image attributes.
  */
  spread_image=CloneImage(image,image->columns,image->rows,False);
  if (spread_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to enhance image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  spread_image->class=DirectClass;
  /*
    Convolve each row.
  */
  quantum=(amount+1) >> 1;
  for (y=0; y < image->rows; y++)
  {
    q=SetPixelCache(spread_image,0,y,spread_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < image->columns; x++)
    {
      x_distance=(rand() & (amount+1))-quantum;
      y_distance=(rand() & (amount+1))-quantum;
      p=GetPixelCache(image,Min(x+x_distance,image->columns-1),
        Min(y+y_distance,image->rows-1),1,1);
      if (p == (PixelPacket *) NULL)
        break;
      *q++=(*p);
    }
    if (!SyncPixelCache(spread_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(SpreadImageText,y,image->rows);
  }
  return(spread_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t e g a n o I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SteganoImage hides a digital watermark within the image.
%
%  The format of the SteganoImage method is:
%
%      Image *SteganoImage(Image *image,Image *watermark)
%
%  A description of each parameter follows:
%
%    o stegano_image: Method SteganoImage returns a pointer to the
%      steganographic image with the watermark hidden.  A null image is
%      returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o watermark: The address of a structure of type Image.
%
%
*/
Export Image *SteganoImage(Image *image,Image *watermark)
{
#define EmbedBit(byte) \
{ \
  q=GetPixelCache(watermark,j % watermark->columns,j/watermark->columns,1,1); \
  if (q == (PixelPacket *) NULL) \
    break;  \
  (byte)&=(~0x01); \
  (byte)|=(Intensity(*q) >> shift) & 0x01; \
  j++; \
  if (j == (watermark->columns*watermark->rows)) \
    { \
      j=0; \
      shift--; \
      if (shift < 0) \
        break; \
    } \
}
#define SteganoImageText  "  Hiding image...  "

  Image
    *stegano_image;

  int
    j,
    shift,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q;

  /*
    Initialize steganographic image attributes.
  */
  assert(image != (Image *) NULL);
  assert(watermark != (Image *) NULL);
  stegano_image=CloneImage(image,image->columns,image->rows,False);
  if (stegano_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,
        "Unable to create steganographic image","Memory allocation failed");
      return((Image *) NULL);
    }
  if (stegano_image->class == PseudoClass)
    {
      if (stegano_image->colors > ((MaxRGB+1) >> 1))
        stegano_image->class=DirectClass;
      else
        {
          /*
            Shift colormap to make room for information hiding.
          */
          stegano_image->colors<<=1;
          stegano_image->colormap=(PixelPacket *) ReallocateMemory((char *)
            stegano_image->colormap,stegano_image->colors*sizeof(PixelPacket));
          if (stegano_image->colormap == (PixelPacket *) NULL)
            {
              MagickWarning(ResourceLimitWarning,
                "Unable to create steganographic image",
                "Memory allocation failed");
              DestroyImage(stegano_image);
              return((Image *) NULL);
            }
          for (i=stegano_image->colors-1; i >= 0; i--)
            stegano_image->colormap[i]=stegano_image->colormap[i >> 1];
          for (y=0; y < (int) stegano_image->rows; y++)
          {
            if (!GetPixelCache(stegano_image,0,y,stegano_image->columns,1))
              break;
            for (x=0; x < (int) stegano_image->columns; x++)
              stegano_image->indexes[x]<<=1;
            if (!SyncPixelCache(stegano_image))
              break;
          }
        }
    }
  /*
    Hide watermark in low-order bits of image.
  */
  i=image->offset;
  j=0;
  shift=QuantumDepth-1;
  for (y=0; y < (int) image->rows; y++)
  {
    for (x=0; x < (int) image->columns; x++)
    {
      if (i == (stegano_image->columns*stegano_image->rows))
        i=0;
      p=GetPixelCache(stegano_image,i % stegano_image->columns,
        i/stegano_image->columns,1,1);
      if (p == (PixelPacket *) NULL)
        break;
      if (stegano_image->class == PseudoClass)
        EmbedBit(*stegano_image->indexes)
      else
        {
          EmbedBit(p->red);
          EmbedBit(p->green);
          EmbedBit(p->blue);
        }
      if (!SyncPixelCache(stegano_image))
        break;
      i++;
    }
    if (shift < 0)
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(SteganoImageText,y,image->rows);
  }
  if (stegano_image->class == PseudoClass)
    SyncImage(stegano_image);
  return(stegano_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t e r e o I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method StereoImage combines two images and produces a single image that
%  is the composite of a left and right image of a stereo pair.  The left
%  image is converted to gray scale and written to the red channel of the
%  stereo image.  The right image is converted to gray scale and written to the
%  blue channel of the stereo image.  View the composite image with red-blue
%  glasses to create a stereo effect.
%
%  The format of the StereoImage method is:
%
%      Image *StereoImage(Image *left_image,Image *right_image)
%
%  A description of each parameter follows:
%
%    o stereo_image: Method StereoImage returns a pointer to the stereo
%      image.  A null image is returned if there is a memory shortage.
%
%    o left_image: The address of a structure of type Image.
%
%    o right_image: The address of a structure of type Image.
%
%
*/
Export Image *StereoImage(Image *left_image,Image *right_image)
{
#define StereoImageText  "  Stereo image...  "

  Image
    *stereo_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q,
    *r;

  assert(left_image != (Image *) NULL);
  assert(right_image != (Image *) NULL);
  if ((left_image->columns != right_image->columns) ||
      (left_image->rows != right_image->rows))
    {
      MagickWarning(ResourceLimitWarning,"Unable to create stereo image",
        "left and right image sizes differ");
      return((Image *) NULL);
    }
  /*
    Initialize stereo image attributes.
  */
  stereo_image=
    CloneImage(left_image,left_image->columns,left_image->rows,False);
  if (stereo_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to create stereo image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  stereo_image->class=DirectClass;
  /*
    Copy left image to red channel and right image to blue channel.
  */
  for (y=0; y < (int) stereo_image->rows; y++)
  {
    p=GetPixelCache(left_image,0,y,left_image->columns,1);
    q=GetPixelCache(right_image,0,y,right_image->columns,1);
    r=SetPixelCache(stereo_image,0,y,stereo_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL) ||
        (r == (PixelPacket *) NULL))
      break;
    for (x=0; x < (int) stereo_image->columns; x++)
    {
      r->red=Intensity(*p);
      r->green=0;
      r->blue=Intensity(*q);
      r->opacity=0;
      p++;
      q++;
      r++;
    }
    if (!SyncPixelCache(stereo_image))
      break;
    if (QuantumTick(y,stereo_image->rows))
      ProgressMonitor(StereoImageText,y,stereo_image->rows);
  }
  return(stereo_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     S w i r l I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SwirlImage creates a new image that is a copy of an existing
%  one with the image pixels "swirl" at a specified angle.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to the
%  new image.
%
%  The format of the SwirlImage method is:
%
%      Image *SwirlImage(Image *image,double degrees)
%
%  A description of each parameter follows:
%
%    o swirl_image: Method SwirlImage returns a pointer to the image
%      after it is swirl.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o degrees:  An double value that defines the tightness of the swirling.
%
%
*/
Export Image *SwirlImage(Image *image,double degrees)
{
#define SwirlImageText  "  Swirling image...  "

  double
    cosine,
    distance,
    factor,
    radius,
    sine,
    x_center,
    x_distance,
    x_scale,
    y_center,
    y_distance,
    y_scale;

  int
    y;

  Image
    *swirl_image;

  register int
    x;

  register PixelPacket
    *p,
    *q;

  /*
    Initialize swirl image attributes.
  */
  assert(image != (Image *) NULL);
  if (!image->matte)
    MatteImage(image,Opaque);
  swirl_image=CloneImage(image,image->columns,image->rows,False);
  if (swirl_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to swirl image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Compute scaling factor.
  */
  x_center=image->columns/2.0;
  y_center=image->rows/2.0;
  radius=Max(x_center,y_center);
  x_scale=1.0;
  y_scale=1.0;
  if (image->columns > image->rows)
    y_scale=(double) image->columns/image->rows;
  else
    if (image->columns < image->rows)
      x_scale=(double) image->rows/image->columns;
  degrees=DegreesToRadians(degrees);
  /*
    Swirl each row.
  */
  for (y=0; y < image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    q=SetPixelCache(swirl_image,0,y,swirl_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    y_distance=y_scale*(y-y_center);
    for (x=0; x < image->columns; x++)
    {
      /*
        Determine if the pixel is within an ellipse.
      */
      *q=(*p);
      x_distance=x_scale*(x-x_center);
      distance=x_distance*x_distance+y_distance*y_distance;
      if (distance < (radius*radius))
        {
          /*
            Swirl the pixel.
          */
          factor=1.0-sqrt(distance)/radius;
          sine=sin(degrees*factor*factor);
          cosine=cos(degrees*factor*factor);
          *q=InterpolateColor(image,
            (cosine*x_distance-sine*y_distance)/x_scale+x_center,
            (sine*x_distance+cosine*y_distance)/y_scale+y_center);
        }
      p++;
      q++;
    }
    if (!SyncPixelCache(swirl_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(SwirlImageText,y,image->rows);
  }
  return(swirl_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     T h r e s h o l d I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ThresholdImage thresholds the reference image.
%
%  The format of the ThresholdImage method is:
%
%      void ThresholdImage(Image *image,const double threshold)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o threshold: A double indicating the threshold value.
%
%
*/
Export void ThresholdImage(Image *image,const double threshold)
{
#define ThresholdImageText  "  Threshold the image...  "

  IndexPacket
    index;

  int
    y;

  PixelPacket
    *colormap;

  register int
    x;

  register PixelPacket
    *q;

  /*
    Threshold image.
  */
  assert(image != (Image *) NULL);
  colormap=(PixelPacket *) AllocateMemory(2*sizeof(PixelPacket));
  if (colormap == (PixelPacket *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to allocate image",
        "Memory allocation failed");
      return;
    }
  if (image->colormap != (PixelPacket *) NULL)
    FreeMemory(image->colormap);
  image->class=PseudoClass;
  image->colors=2;
  image->colormap=colormap;
  image->colormap[0].red=0;
  image->colormap[0].green=0;
  image->colormap[0].blue=0;
  image->colormap[1].red=MaxRGB;
  image->colormap[1].green=MaxRGB;
  image->colormap[1].blue=MaxRGB;
  for (y=0; y < (int) image->rows; y++)
  {
    q=GetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      index=Intensity(*q) < threshold ? 0 : 1;
      image->indexes[x]=index;
      *q++=image->colormap[index];
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(ThresholdImageText,y,image->rows);
  }
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     W a v e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WaveImage creates a new image that is a copy of an existing
%  one with the image pixels altered along a sine wave.  It allocates the
%  memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the WaveImage method is:
%
%      Image *WaveImage(Image *image,const double amplitude,
%        const double wavelength)
%
%  A description of each parameter follows:
%
%    o shade_image: Method WaveImage returns a pointer to the image
%      after it is shaded.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o amplitude, frequency:  A double value that indicates the amplitude
%      and wavelength of the sine wave.
%
%
*/
Export Image *WaveImage(Image *image,const double amplitude,
  const double wavelength)
{
#define WaveImageText  "  Waving image...  "

  double
    *sine_map;

  Image
    *wave_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *q;

  /*
    Initialize waved image attributes.
  */
  assert(image != (Image *) NULL);
  if (!image->matte)
    MatteImage(image,Opaque);
  wave_image=CloneImage(image,image->columns,image->rows+
    (int) (2*AbsoluteValue(amplitude)),False);
  if (wave_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to wave image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Allocate sine map.
  */
  sine_map=(double *) AllocateMemory(wave_image->columns*sizeof(double));
  if (sine_map == (double *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to wave image",
        "Memory allocation failed");
      DestroyImage(wave_image);
      return((Image *) NULL);
    }
  for (x=0; x < (int) wave_image->columns; x++)
    sine_map[x]=AbsoluteValue(amplitude)+amplitude*sin((2*M_PI*x)/wavelength);
  /*
    Wave image.
  */
  for (y=0; y < (int) wave_image->rows; y++)
  {
    q=SetPixelCache(wave_image,0,y,wave_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) wave_image->columns; x++)
    {
      *q=InterpolateColor(image,x,(int) (y-sine_map[x]));
      q++;
    }
    if (!SyncPixelCache(wave_image))
      break;
    if (QuantumTick(y,wave_image->rows))
      ProgressMonitor(WaveImageText,y,wave_image->rows);
  }
  FreeMemory(sine_map);
  return(wave_image);
}
