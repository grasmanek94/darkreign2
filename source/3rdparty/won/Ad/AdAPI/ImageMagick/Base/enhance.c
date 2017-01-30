/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%              EEEEE  N   N  H   H   AAA   N   N   CCCC  EEEEE                %
%              E      NN  N  H   H  A   A  NN  N  C      E                    %
%              EEE    N N N  HHHHH  AAAAA  N N N  C      EEE                  %
%              E      N  NN  H   H  A   A  N  NN  C      E                    %
%              EEEEE  N   N  H   H  A   A  N   N   CCCC  EEEEE                %
%                                                                             %
%                                                                             %
%                    ImageMagick Image Enhancement Methods                    %
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
%     C o n t r a s t I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ContrastImage enhances the intensity differences between the
%  lighter and darker elements of the image.
%
%  The format of the ContrastImage method is:
%
%      void ContrastImage(Image *image,const unsigned int sharpen)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o sharpen: If True, the intensity is increased otherwise it is
%      decreased.
%
%
*/
Export void ContrastImage(Image *image,const unsigned int sharpen)
{
#define DullContrastImageText  "  Dulling image contrast...  "
#define SharpenContrastImageText  "  Sharpening image contrast...  "

  int
    sign,
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  assert(image != (Image *) NULL);
  sign=sharpen ? 1 : -1;
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      /*
        Contrast enhance DirectClass image.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          Contrast(sign,&q->red,&q->green,&q->blue);
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          {
            if (sharpen)
              ProgressMonitor(SharpenContrastImageText,y,image->rows);
            else
              ProgressMonitor(DullContrastImageText,y,image->rows);
          }
      }
      break;
    }
    case PseudoClass:
    {
      Quantum
        blue,
        green,
        red;

      /*
        Contrast enhance PseudoClass image.
      */
      for (i=0; i < (int) image->colors; i++)
      {
        red=image->colormap[i].red;
        green=image->colormap[i].green;
        blue=image->colormap[i].blue;
        Contrast(sign,&red,&green,&blue);
        image->colormap[i].red=red;
        image->colormap[i].green=green;
        image->colormap[i].blue=blue;
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
%     E q u a l i z e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method EqualizeImage performs histogram equalization on the reference
%  image.
%
%  The format of the EqualizeImage method is:
%
%      void EqualizeImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export void EqualizeImage(Image *image)
{
#define EqualizeImageText  "  Equalizing image...  "

  int
    j,
    y;

  Quantum
    *equalize_map;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q;

  unsigned int
    high,
    *histogram,
    low,
    *map;

  /*
    Allocate and initialize histogram arrays.
  */
  assert(image != (Image *) NULL);
  histogram=(unsigned int *) AllocateMemory((MaxRGB+1)*sizeof(unsigned int));
  map=(unsigned int *) AllocateMemory((MaxRGB+1)*sizeof(unsigned int));
  equalize_map=(Quantum *) AllocateMemory((MaxRGB+1)*sizeof(Quantum));
  if ((histogram == (unsigned int *) NULL) || (map == (unsigned int *) NULL) ||
      (equalize_map == (Quantum *) NULL))
    {
      MagickWarning(ResourceLimitWarning,"Unable to equalize image",
        "Memory allocation failed");
      return;
    }
  /*
    Form histogram.
  */
  for (i=0; i <= MaxRGB; i++)
    histogram[i]=0;
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      histogram[Intensity(*p)]++;
      p++;
    }
  }
  /*
    Integrate the histogram to get the equalization map.
  */
  j=0;
  for (i=0; i <= MaxRGB; i++)
  {
    j+=histogram[i];
    map[i]=j;
  }
  FreeMemory(histogram);
  if (map[MaxRGB] == 0)
    {
      FreeMemory(equalize_map);
      FreeMemory(map);
      return;
    }
  /*
    Equalize.
  */
  low=map[0];
  high=map[MaxRGB];
  for (i=0; i <= MaxRGB; i++)
    equalize_map[i]=(Quantum)
      ((((double) (map[i]-low))*MaxRGB)/Max(high-low,1));
  FreeMemory(map);
  /*
    Stretch the histogram.
  */
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      /*
        Equalize DirectClass packets.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          q->red=equalize_map[q->red];
          q->green=equalize_map[q->green];
          q->blue=equalize_map[q->blue];
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(EqualizeImageText,y,image->rows);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Equalize PseudoClass packets.
      */
      for (i=0; i < (int) image->colors; i++)
      {
        image->colormap[i].red=equalize_map[image->colormap[i].red];
        image->colormap[i].green=equalize_map[image->colormap[i].green];
        image->colormap[i].blue=equalize_map[image->colormap[i].blue];
      }
      SyncImage(image);
      break;
    }
  }
  FreeMemory(equalize_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     G a m m a I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GammaImage converts the reference image to gamma corrected colors.
%
%  The format of the GammaImage method is:
%
%      void GammaImage(Image *image,const char *gamma)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o gamma: A character string indicating the level of gamma correction.
%
%
*/
Export void GammaImage(Image *image,const char *gamma)
{
#define GammaImageText  "  Gamma correcting the image...  "

  double
    blue_gamma,
    green_gamma,
    opacity_gamma,
    red_gamma;

  int
    count,
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  PixelPacket
    *gamma_map;

  assert(image != (Image *) NULL);
  if (gamma == (char *) NULL)
    return;
  red_gamma=1.0;
  green_gamma=1.0;
  blue_gamma=1.0;
  opacity_gamma=1.0;
  count=sscanf(gamma,"%lf,%lf,%lf,%lf",&red_gamma,&green_gamma,&blue_gamma,
    &opacity_gamma);
  count=sscanf(gamma,"%lf/%lf/%lf/%lf",&red_gamma,&green_gamma,&blue_gamma,
    &opacity_gamma);
  if (count == 1)
    {
      if (red_gamma == 1.0)
        return;
      green_gamma=red_gamma;
      blue_gamma=red_gamma;
    }
  /*
    Allocate and initialize gamma maps.
  */
  gamma_map=(PixelPacket *) AllocateMemory((MaxRGB+1)*sizeof(PixelPacket));
  if (gamma_map == (PixelPacket *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to gamma correct image",
        "Memory allocation failed");
      return;
    }
  for (i=0; i <= MaxRGB; i++)
  {
    gamma_map[i].red=0;
    gamma_map[i].green=0;
    gamma_map[i].blue=0;
    gamma_map[i].opacity=0;
  }
  /*
    Initialize gamma table.
  */
  for (i=0; i <= MaxRGB; i++)
  {
    if (red_gamma != 0.0)
      gamma_map[i].red=(Quantum)
        ((pow((double) i/MaxRGB,1.0/red_gamma)*MaxRGB)+0.5);
    if (green_gamma != 0.0)
      gamma_map[i].green=(Quantum)
        ((pow((double) i/MaxRGB,1.0/green_gamma)*MaxRGB)+0.5);
    if (blue_gamma != 0.0)
      gamma_map[i].blue=(Quantum)
        ((pow((double) i/MaxRGB,1.0/blue_gamma)*MaxRGB)+0.5);
    if (opacity_gamma != 0.0)
      gamma_map[i].opacity=(Quantum)
        ((pow((double) i/MaxRGB,1.0/opacity_gamma)*MaxRGB)+0.5);
  }
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      /*
        Gamma-correct DirectClass image.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          q->red=gamma_map[q->red].red;
          q->green=gamma_map[q->green].green;
          q->blue=gamma_map[q->blue].blue;
          q->opacity=gamma_map[q->opacity].opacity;
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(GammaImageText,y,image->rows);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Gamma-correct PseudoClass image.
      */
      for (i=0; i < (int) image->colors; i++)
      {
        image->colormap[i].red=gamma_map[image->colormap[i].red].red;
        image->colormap[i].green=gamma_map[image->colormap[i].green].green;
        image->colormap[i].blue=gamma_map[image->colormap[i].blue].blue;
      }
      SyncImage(image);
      break;
    }
  }
  if (image->gamma != 0.0)
    image->gamma*=(red_gamma+green_gamma+blue_gamma)/3.0;
  FreeMemory(gamma_map);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     M o d u l a t e I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ModulateImage modulates the hue, saturation, and brightness of an
%  image.
%
%  The format of the ModulateImage method is:
%
%      void ModulateImage(Image *image,const char *modulate)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o modulate: A character string indicating the percent change in brightness,
%      saturation, and hue in floating point notation separated by commas
%      (e.g. 10.1,0.0,3.1).
%
%
*/
Export void ModulateImage(Image *image,const char *modulate)
{
#define ModulateImageText  "  Modulating image...  "

  double
    percent_brightness,
    percent_hue,
    percent_saturation;

  int
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  /*
    Initialize gamma table.
  */
  assert(image != (Image *) NULL);
  if (modulate == (char *) NULL)
    return;
  percent_hue=0.0;
  percent_brightness=0.0;
  percent_saturation=0.0;
  (void) sscanf(modulate,"%lf,%lf,%lf",&percent_brightness,&percent_saturation,
    &percent_hue);
  (void) sscanf(modulate,"%lf/%lf/%lf",&percent_brightness,&percent_saturation,
    &percent_hue);
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      /*
        Modulate the color for a DirectClass image.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          Modulate(percent_hue,percent_saturation,percent_brightness,
            &q->red,&q->green,&q->blue);
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(ModulateImageText,y,image->rows);
      }
      break;
    }
    case PseudoClass:
    {
      Quantum
        blue,
        green,
        red;

      /*
        Modulate the color for a PseudoClass image.
      */
      for (i=0; i < (int) image->colors; i++)
      {
        red=image->colormap[i].red;
        green=image->colormap[i].green;
        blue=image->colormap[i].blue;
        Modulate(percent_hue,percent_saturation,percent_brightness,
          &red,&green,&blue);
        image->colormap[i].red=red;
        image->colormap[i].green=green;
        image->colormap[i].blue=blue;
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
%     N e g a t e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method NegateImage negates the colors in the reference image.  The
%  Grayscale option means that only grayscale values within the image are
%  negated.
%
%  The format of the NegateImage method is:
%
%      void NegateImage(Image *image,const unsigned int grayscale)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export void NegateImage(Image *image,const unsigned int grayscale)
{
#define NegateImageText  "  Negating the image colors...  "

  int
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  assert(image != (Image *) NULL);
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      /*
        Negate DirectClass packets.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          if (grayscale)
            if ((q->red != q->green) || (q->green != q->blue))
              {
                q++;
                continue;
              }
          q->red=(~q->red);
          q->green=(~q->green);
          q->blue=(~q->blue);
          q->opacity=(~q->opacity);
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(NegateImageText,y,image->rows);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Negate PseudoClass packets.
      */
      for (i=0; i < (int) image->colors; i++)
      {
        if (grayscale)
          if ((image->colormap[i].red != image->colormap[i].green) ||
              (image->colormap[i].green != image->colormap[i].blue))
            continue;
        image->colormap[i].red=(Quantum) (~image->colormap[i].red);
        image->colormap[i].green=(Quantum) (~image->colormap[i].green);
        image->colormap[i].blue=(Quantum) (~image->colormap[i].blue);
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
%     N o r m a l i z e I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method NormalizeImage normalizes the pixel values to span the full
%  range of color values.  This is a contrast enhancement technique.
%
%  The format of the NormalizeImage method is:
%
%      void NormalizeImage(Image *image)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%
*/
Export void NormalizeImage(Image *image)
{
#define NormalizeImageText  "  Normalizing image...  "

  int
    *histogram,
    threshold_intensity,
    y;

  Quantum
    gray_value,
    *normalize_map;

  register int
    i,
    intensity,
    x;

  register PixelPacket
    *p,
    *q;

  unsigned int
    high,
    low;

  /*
    Allocate histogram and normalize map.
  */
  assert(image != (Image *) NULL);
  histogram=(int *) AllocateMemory((MaxRGB+1)*sizeof(int));
  normalize_map=(Quantum *) AllocateMemory((MaxRGB+1)*sizeof(Quantum));
  if ((histogram == (int *) NULL) || (normalize_map == (Quantum *) NULL))
    {
      MagickWarning(ResourceLimitWarning,"Unable to normalize image",
        "Memory allocation failed");
      return;
    }
  /*
    Form histogram.
  */
  for (i=0; i <= MaxRGB; i++)
    histogram[i]=0;
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      gray_value=Intensity(*p);
      histogram[gray_value]++;
      p++;
    }
  }
  /*
    Find the histogram boundaries by locating the 1 percent levels.
  */
  threshold_intensity=(image->columns*image->rows)/100;
  intensity=0;
  for (low=0; low < MaxRGB; low++)
  {
    intensity+=histogram[low];
    if (intensity > threshold_intensity)
      break;
  }
  intensity=0;
  for (high=MaxRGB; high != 0; high--)
  {
    intensity+=histogram[high];
    if (intensity > threshold_intensity)
      break;
  }
  if (low == high)
    {
      /*
        Unreasonable contrast;  use zero threshold to determine boundaries.
      */
      threshold_intensity=0;
      intensity=0;
      for (low=0; low < MaxRGB; low++)
      {
        intensity+=histogram[low];
        if (intensity > threshold_intensity)
          break;
      }
      intensity=0;
      for (high=MaxRGB; high != 0; high--)
      {
        intensity+=histogram[high];
        if (intensity > threshold_intensity)
          break;
      }
      if (low == high)
        return;  /* zero span bound */
    }
  /*
    Stretch the histogram to create the normalized image mapping.
  */
  for (i=0; i <= MaxRGB; i++)
    if (i < (int) low)
      normalize_map[i]=0;
    else
      if (i > (int) high)
        normalize_map[i]=MaxRGB;
      else
        normalize_map[i]=(MaxRGB-1)*(i-low)/(high-low);
  /*
    Normalize the image.
  */
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      /*
        Normalize DirectClass image.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          q->red=normalize_map[q->red];
          q->green=normalize_map[q->green];
          q->blue=normalize_map[q->blue];
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(NormalizeImageText,y,image->rows);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Normalize PseudoClass image.
      */
      for (i=0; i < (int) image->colors; i++)
      {
        image->colormap[i].red=normalize_map[image->colormap[i].red];
        image->colormap[i].green=normalize_map[image->colormap[i].green];
        image->colormap[i].blue=normalize_map[image->colormap[i].blue];
      }
      SyncImage(image);
      break;
    }
  }
  FreeMemory(normalize_map);
  FreeMemory(histogram);
}
