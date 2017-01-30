/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        ZZZZZ   OOO    OOO   M   M                           %
%                           ZZ  O   O  O   O  MM MM                           %
%                         ZZZ   O   O  O   O  M M M                           %
%                        ZZ     O   O  O   O  M   M                           %
%                        ZZZZZ   OOO    OOO   M   M                           %
%                                                                             %
%                                                                             %
%                      ImageMagick Image Zoom Methods                         %
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
  Typedef declarations.
*/
typedef struct _ContributionInfo
{
  int
    pixel;

  double
    weight;
} ContributionInfo;

typedef struct _FilterInfo
{
  double
    (*function)(double),
    support;
} FilterInfo;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   B e s s e l O r d e r O n e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method BesselOrderOne computes the Bessel function of x of the first kind
%  of order 0:
%
%    Reduce x to |x| since j1(x)= -j1(-x), and for x in (0,8]
%
%       j1(x) = x*j1(x);
%
%    For x in (8,inf)
%
%       j1(x) = sqrt(2/(pi*x))*(p1(x)*cos(x1)-q1(x)*sin(x1))
%
%    where x1 = x-3*pi/4. Compute sin(x1) and cos(x1) as follow:
%
%       cos(x1) =  cos(x)cos(3pi/4)+sin(x)sin(3pi/4)
%               =  1/sqrt(2) * (sin(x) - cos(x))
%       sin(x1) =  sin(x)cos(3pi/4)-cos(x)sin(3pi/4)
%               = -1/sqrt(2) * (sin(x) + cos(x))
%
%  The format of the BesselOrderOne method is:
%
%      double BesselOrderOne(double x)
%
%  A description of each parameter follows:
%
%    o value: Method BesselOrderOne returns the Bessel function of x of the
%      first kind of orders 1.
%
%    o x: double value.
%
%
*/

static double J1(double x)
{
  double
    p,
    q;

  register int
    i;

  static const double
    Pone[] =
    {
       0.581199354001606143928050809e+21,
      -0.6672106568924916298020941484e+20,
       0.2316433580634002297931815435e+19,
      -0.3588817569910106050743641413e+17,
       0.2908795263834775409737601689e+15,
      -0.1322983480332126453125473247e+13,
       0.3413234182301700539091292655e+10,
      -0.4695753530642995859767162166e+7,
       0.270112271089232341485679099e+4
    },
    Qone[] =
    {
      0.11623987080032122878585294e+22,
      0.1185770712190320999837113348e+20,
      0.6092061398917521746105196863e+17,
      0.2081661221307607351240184229e+15,
      0.5243710262167649715406728642e+12,
      0.1013863514358673989967045588e+10,
      0.1501793594998585505921097578e+7,
      0.1606931573481487801970916749e+4,
      0.1e+1
    };

  p=Pone[8];
  q=Qone[8];
  for (i=7; i >= 0; i--)
  {
    p=p*x*x+Pone[i];
    q=q*x*x+Qone[i];
  }
  return(p/q);
}

static double P1(double x)
{
  double
    p,
    q;

  register int
    i;

  static const double
    Pone[] =
    {
      0.352246649133679798341724373e+5,
      0.62758845247161281269005675e+5,
      0.313539631109159574238669888e+5,
      0.49854832060594338434500455e+4,
      0.2111529182853962382105718e+3,
      0.12571716929145341558495e+1
    },
    Qone[] =
    {
      0.352246649133679798068390431e+5,
      0.626943469593560511888833731e+5,
      0.312404063819041039923015703e+5,
      0.4930396490181088979386097e+4,
      0.2030775189134759322293574e+3,
      0.1e+1
    };

  p=Pone[5];
  q=Qone[5];
  for (i=4; i >= 0; i--)
  {
    p=p*(8.0/x)*(8.0/x)+Pone[i];
    q=q*(8.0/x)*(8.0/x)+Qone[i];
  }
  return(p/q);
}

static double Q1(double x)
{
  double
    p,
    q;

  register int
    i;

  static const double
    Pone[] =
    {
      0.3511751914303552822533318e+3,
      0.7210391804904475039280863e+3,
      0.4259873011654442389886993e+3,
      0.831898957673850827325226e+2,
      0.45681716295512267064405e+1,
      0.3532840052740123642735e-1
    },
    Qone[] =
    {
      0.74917374171809127714519505e+4,
      0.154141773392650970499848051e+5,
      0.91522317015169922705904727e+4,
      0.18111867005523513506724158e+4,
      0.1038187585462133728776636e+3,
      0.1e+1
    };

  p=Pone[5];
  q=Qone[5];
  for (i=4; i >= 0; i--)
  {
    p=p*(8.0/x)*(8.0/x)+Pone[i];
    q=q*(8.0/x)*(8.0/x)+Qone[i];
  }
  return(p/q);
}

static double BesselOrderOne(double x)
{
  double
    p,
    q;

  if (x == 0.0)
    return(0.0);
  p=x;
  if (x < 0.0)
    x=(-x);
  if (x < 8.0)
    return(p*J1(x));
  q=sqrt(2.0/(M_PI*x))*(P1(x)*(1.0/sqrt(2.0)*(sin(x)-cos(x)))-8.0/x*Q1(x)*
    (-1.0/sqrt(2.0)*(sin(x)+cos(x))));
  if (p < 0.0)
    q=(-q);
  return(q);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a g n i f y I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method MagnifyImage creates a new image that is a integral size greater
%  than an existing one.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  MagnifyImage scans the reference image to create a magnify image by
%  bilinear interpolation.  The magnify image columns and rows become:
%
%    number_columns << 1
%    number_rows << 1
%
%  The format of the MagnifyImage method is:
%
%      magnify_image=MagnifyImage(image)
%
%  A description of each parameter follows:
%
%    o magnify_image: Method MagnifyImage returns a pointer to the image
%      after magnification.  A null image is returned if there is a memory
%      shortage.
%
%    o image: The address of a structure of type Image.
%
%
*/
Export Image *MagnifyImage(Image *image)
{
#define MagnifyImageText  "  Magnifying the image...  "

  Image
    *magnify_image;

  int
    rows,
    y;

  PixelPacket
    *scanline;

  register int
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  /*
    Initialize magnify image attributes.
  */
  assert(image != (Image *) NULL);
  magnify_image=CloneImage(image,image->columns << 1,image->rows << 1,False);
  if (magnify_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to magnify image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  magnify_image->class=DirectClass;
  /*
    Allocate image buffer and scanline buffer for 4 rows of the image.
  */
  scanline=(PixelPacket *)
    AllocateMemory(magnify_image->columns*sizeof(PixelPacket));
  if (scanline == (PixelPacket *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to magnify image",
        "Memory allocation failed");
      DestroyImage(magnify_image);
      return((Image *) NULL);
    }
  /*
    Initialize zoom image pixels.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    q=SetPixelCache(magnify_image,0,y,magnify_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    (void) memcpy(q,p,image->columns*sizeof(PixelPacket));
    if (!SyncPixelCache(magnify_image))
      break;
  }
  /*
    Magnify each row.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(magnify_image,0,image->rows-1-y,magnify_image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    (void) memcpy(scanline,p,magnify_image->columns*sizeof(PixelPacket));
    q=GetPixelCache(magnify_image,0,(image->rows-1-y) << 1,
      magnify_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    p=scanline+image->columns-1;
    q+=((image->columns-1) << 1);
    *q=(*p);
    *(q+1)=(*(p));
    for (x=1; x < (int) image->columns; x++)
    {
      p--;
      q-=2;
      *q=(*p);
      (q+1)->red=(((int) p->red)+((int) (p+1)->red)+1) >> 1;
      (q+1)->green=(((int) p->green)+((int) (p+1)->green)+1) >> 1;
      (q+1)->blue=(((int) p->blue)+((int) (p+1)->blue)+1) >> 1;
      (q+1)->opacity=(((int) p->opacity)+((int) (p+1)->opacity)+1) >> 1;
    }
    if (!SyncPixelCache(magnify_image))
      break;
  }
  for (y=0; y < (int) image->rows; y++)
  {
    rows=Min(image->rows-y,3);
    p=GetPixelCache(magnify_image,0,y << 1,magnify_image->columns,rows);
    if (p == (PixelPacket *) NULL)
      break;
    q=p;
    if (rows > 1)
      q=p+magnify_image->columns;
    s=p;
    if (rows > 2)
      s=q+magnify_image->columns;
    for (x=0; x < (int) (image->columns-1); x++)
    {
      q->red=(((int) p->red)+((int) s->red)+1) >> 1;
      q->green=(((int) p->green)+((int) s->green)+1) >> 1;
      q->blue=(((int) p->blue)+((int) s->blue)+1) >> 1;
      q->opacity=(((int) p->opacity)+((int) s->opacity)+1) >> 1;
      (q+1)->red=(((int) p->red)+((int) (p+2)->red)+((int) s->red)+
        ((int) (s+2)->red)+2) >> 2;
      (q+1)->green=(((int) p->green)+((int) (p+2)->green)+((int) s->green)+
        ((int) (s+2)->green)+2) >> 2;
      (q+1)->blue=(((int) p->blue)+((int) (p+2)->blue)+((int) s->blue)+
        ((int) (s+2)->blue)+2) >> 2;
      (q+1)->opacity=(((int) p->opacity)+((int) (p+2)->opacity)+
        ((int) s->opacity)+((int) (s+2)->opacity)+2) >> 2;
      q+=2;
      p+=2;
      s+=2;
    }
    q->red=(((int) p->red)+((int) s->red)+1) >> 1;
    q->green=(((int) p->green)+((int) s->green)+1) >> 1;
    q->blue=(((int) p->blue)+((int) s->blue)+1) >> 1;
    q->opacity=(((int) p->opacity)+((int) s->opacity)+1) >> 1;
    p++;
    q++;
    s++;
    q->red=(((int) p->red)+((int) s->red)+1) >> 1;
    q->green=(((int) p->green)+((int) s->green)+1) >> 1;
    q->blue=(((int) p->blue)+((int) s->blue)+1) >> 1;
    q->opacity=(((int) p->opacity)+((int) s->opacity)+1) >> 1;
    if (!SyncPixelCache(magnify_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(MagnifyImageText,y,image->rows);
  }
  p=GetPixelCache(magnify_image,0,2*image->rows-2,magnify_image->columns,1);
  if (p != (PixelPacket *) NULL)
    (void) memcpy(scanline,p,magnify_image->columns*sizeof(PixelPacket));
  q=GetPixelCache(magnify_image,0,2*image->rows-1,magnify_image->columns,1);
  if (q != (PixelPacket *) NULL)
    (void) memcpy(q,scanline,magnify_image->columns*sizeof(PixelPacket));
  (void) SyncPixelCache(magnify_image);
  FreeMemory(scanline);
  return(magnify_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M i n i f y I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method MinifyImage creates a new image that is a integral size less than
%  an existing one.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.
%
%  MinifyImage scans the reference image to create a minified image by
%  computing the weighted average of a 4x4 cell centered at each reference
%  pixel.  The target pixel requires two columns and two rows of the reference
%  pixels.  Therefore the minified image columns and rows become:
%
%    number_columns/2
%    number_rows/2
%
%  Weights assume that the importance of neighboring pixels is negately
%  proportional to the square of their distance from the target pixel.
%
%  The scan only processes pixels that have a full set of neighbors.  Pixels
%  in the top, bottom, left, and right pairs of rows and columns are omitted
%  from the scan.
%
%  The format of the MinifyImage method is:
%
%      Image *MinifyImage(Image *image)
%
%  A description of each parameter follows:
%
%    o minify_image: Method MinifyImage returns a pointer to the image
%      after reducing.  A null image is returned if there is a memory
%      shortage or if the image size is less than IconSize*2.
%
%    o image: The address of a structure of type Image.
%
%
*/
Export Image *MinifyImage(Image *image)
{
#define Minify(weight) \
  total_red+=(weight)*(s->red); \
  total_green+=(weight)*(s->green); \
  total_blue+=(weight)*(s->blue); \
  total_opacity+=(weight)*(s->opacity); \
  s++;
#define MinifyImageText  "  Minifying image...  "

  Image
    *minify_image;

  int
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q,
    *s;

  unsigned long
    total_blue,
    total_green,
    total_opacity,
    total_red;

  assert(image != (Image *) NULL);
  if ((image->columns < 4) || (image->rows < 4))
    return((Image *) NULL);
  /*
    Initialize minified image.
  */
  minify_image=CloneImage(image,image->columns >> 1,image->rows >> 1,False);
  if (minify_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to minify image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  minify_image->class=DirectClass;
  /*
    Reduce each row.
  */
  for (y=0; y < (int) minify_image->rows; y++)
  {
    p=GetPixelCache(image,0,Min(y << 1,image->rows-4),image->columns,4);
    q=SetPixelCache(minify_image,0,y,minify_image->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (int) (minify_image->columns-1); x++)
    {
      /*
        Compute weighted average of target pixel color components.
      */
      total_red=0;
      total_green=0;
      total_blue=0;
      total_opacity=0;
      s=p;
      Minify(3); Minify(7);  Minify(7);  Minify(3);
      s=p+image->columns;
      Minify(7); Minify(15); Minify(15); Minify(7);
      s=p+2*image->columns;
      Minify(7); Minify(15); Minify(15); Minify(7);
      s=p+3*image->columns;
      Minify(3); Minify(7);  Minify(7);  Minify(3);
      q->red=((total_red+63) >> 7);
      q->green=((total_green+63) >> 7);
      q->blue=((total_blue+63) >> 7);
      q->opacity=((total_opacity+63) >> 7);
      p+=2;
      q++;
    }
    p++;
    *q++=(*p);
    if (!SyncPixelCache(minify_image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(MinifyImageText,y,image->rows-1);
  }
  return(minify_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S a m p l e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SampleImage creates a new image that is a scaled size of an
%  existing one using pixel sampling.  It allocates the memory necessary
%  for the new Image structure and returns a pointer to the new image.
%
%  The format of the SampleImage method is:
%
%      Image *SampleImage(Image *image,const unsigned int columns,
%        const unsigned int rows)
%
%  A description of each parameter follows:
%
%    o sample_image: Method SampleImage returns a pointer to the image after
%      scaling.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o columns: An integer that specifies the number of columns in the sampled
%      image.
%
%    o rows: An integer that specifies the number of rows in the sampled
%      image.
%
%
*/
Export Image *SampleImage(Image *image,const unsigned int columns,
  const unsigned int rows)
{
#define SampleImageText  "  Sampling image...  "

  double
    *x_offset,
    *y_offset;

  Image
    *sample_image;

  IndexPacket
    *indexes;

  int
    j,
    k,
    y;

  register int
    x;

  register PixelPacket
    *p,
    *q;

  PixelPacket
    *pixels;

  assert(image != (Image *) NULL);
  if ((columns == 0) || (rows == 0))
    {
      MagickWarning(OptionWarning,"Unable to resize image",
        "image dimensions are zero");
      return((Image *) NULL);
    }
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,columns,rows,False));
  /*
    Initialize sampled image attributes.
  */
  sample_image=CloneImage(image,columns,rows,False);
  if (sample_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to sample image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  /*
    Allocate scan line buffer and column offset buffers.
  */
  pixels=(PixelPacket *)
    AllocateMemory(image->columns*sizeof(PixelPacket));
  indexes=(IndexPacket *)
    AllocateMemory(image->columns*sizeof(IndexPacket));
  x_offset=(double *) AllocateMemory(sample_image->columns*sizeof(double));
  y_offset=(double *) AllocateMemory(sample_image->rows*sizeof(double));
  if ((pixels == (PixelPacket *) NULL) || (indexes == (IndexPacket *) NULL) ||
      (x_offset == (double *) NULL) || (y_offset == (double *) NULL))
    {
      MagickWarning(ResourceLimitWarning,"Unable to sample image",
        "Memory allocation failed");
      DestroyImage(sample_image);
      return((Image *) NULL);
    }
  /*
    Initialize pixel offsets.
  */
  for (x=0; x < (int) sample_image->columns; x++)
    x_offset[x]=x*image->columns/(double) sample_image->columns;
  for (y=0; y < (int) sample_image->rows; y++)
    y_offset[y]=y*image->rows/(double) sample_image->rows;
  /*
    Sample each row.
  */
  j=(-1);
  for (y=0; y < (int) sample_image->rows; y++)
  {
    q=SetPixelCache(sample_image,0,y,sample_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    if (j != y_offset[y])
      {
        /*
          Read a scan line.
        */
        j=y_offset[y];
        p=GetPixelCache(image,0,j,image->columns,1);
        if (p == (PixelPacket *) NULL)
          break;
        if (image->class == PseudoClass)
          (void) memcpy(indexes,image->indexes,
            image->columns*sizeof(IndexPacket));
        (void) memcpy(pixels,p,image->columns*sizeof(PixelPacket));
      }
    /*
      Sample each column.
    */
    for (x=0; x < (int) sample_image->columns; x++)
    {
      k=x_offset[x];
      if (sample_image->class == PseudoClass)
        sample_image->indexes[x]=indexes[k];
      *q++=pixels[k];
    }
    if (!SyncPixelCache(sample_image))
      break;
    if (QuantumTick(y,sample_image->rows))
      ProgressMonitor(SampleImageText,y,sample_image->rows);
  }
  FreeMemory(y_offset);
  FreeMemory(x_offset);
  FreeMemory(indexes);
  FreeMemory(pixels);
  return(sample_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S c a l e I m a g e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ScaleImage creates a new image that is a scaled size of an
%  existing one.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.  To scale a scanline
%  from x pixels to y pixels, each new pixel represents x/y old pixels.  To
%  read x/y pixels, read (x/y rounded up) pixels but only count the required
%  fraction of the last old pixel read in your new pixel.  The remainder
%  of the old pixel will be counted in the next new pixel.
%
%  The scaling algorithm was suggested by rjohnson@shell.com and is adapted
%  from pnmscale(1) of PBMPLUS by Jef Poskanzer.
%
%  The format of the ScaleImage method is:
%
%      Image *ScaleImage(Image *image,const unsigned int columns,
%        const unsigned int rows)
%
%  A description of each parameter follows:
%
%    o scale_image: Method ScaleImage returns a pointer to the image after
%      scaling.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o columns: An integer that specifies the number of columns in the scaled
%      image.
%
%    o rows: An integer that specifies the number of rows in the scaled
%      image.
%
%
*/
Export Image *ScaleImage(Image *image,const unsigned int columns,
  const unsigned int rows)
{
#define ScaleImageText  "  Scaling image...  "

  typedef struct ScaledPacket
  {
    double
      red,
      green,
      blue,
      opacity;
  } ScaledPacket;

  double
    blue,
    green,
    opacity,
    red,
    x_scale,
    x_span,
    y_scale,
    y_span;

  Image
    *scale_image;

  int
    next_column,
    next_row,
    number_rows,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q;

  register ScaledPacket
    *s,
    *t;

  ScaledPacket
    *scale_scanline,
    *scanline,
    *x_vector,
    *y_vector;

  assert(image != (Image *) NULL);
  if ((columns == 0) || (rows == 0))
    return((Image *) NULL);
  /*
    Initialize scaled image attributes.
  */
  scale_image=CloneImage(image,columns,rows,False);
  if (scale_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to scale image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  scale_image->class=DirectClass;
  /*
    Allocate memory.
  */
  x_vector=(ScaledPacket *) AllocateMemory(image->columns*sizeof(ScaledPacket));
  scanline=x_vector;
  if (image->rows != scale_image->rows)
    scanline=(ScaledPacket *)
      AllocateMemory(image->columns*sizeof(ScaledPacket));
  scale_scanline=(ScaledPacket *)
    AllocateMemory(scale_image->columns*sizeof(ScaledPacket));
  y_vector=(ScaledPacket *) AllocateMemory(image->columns*sizeof(ScaledPacket));
  if ((scanline == (ScaledPacket *) NULL) ||
      (scale_scanline == (ScaledPacket *) NULL) ||
      (x_vector == (ScaledPacket *) NULL) ||
      (y_vector == (ScaledPacket *) NULL))
    {
      MagickWarning(ResourceLimitWarning,"Unable to scale image",
        "Memory allocation failed");
      DestroyImage(scale_image);
      return((Image *) NULL);
    }
  /*
    Scale image.
  */
  number_rows=0;
  next_row=True;
  y_span=1.0;
  y_scale=(double) scale_image->rows/image->rows;
  for (x=0; x < (int) image->columns; x++)
  {
    y_vector[x].red=0;
    y_vector[x].green=0;
    y_vector[x].blue=0;
    y_vector[x].opacity=0;
  }
  i=0;
  for (y=0; y < scale_image->rows; y++)
  {
    q=SetPixelCache(scale_image,0,y,scale_image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    if (scale_image->rows == image->rows)
      {
        /*
          Read a new scanline.
        */
        p=GetPixelCache(image,0,i++,image->columns,1);
        if (p == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          x_vector[x].red=p->red;
          x_vector[x].green=p->green;
          x_vector[x].blue=p->blue;
          x_vector[x].opacity=p->opacity;
          p++;
        }
      }
    else
      {
        /*
          Scale Y direction.
        */
        while (y_scale < y_span)
        {
          if (next_row && (number_rows < (int) image->rows))
            {
              /*
                Read a new scanline.
              */
              p=GetPixelCache(image,0,i++,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                x_vector[x].red=p->red;
                x_vector[x].green=p->green;
                x_vector[x].blue=p->blue;
                x_vector[x].opacity=p->opacity;
                p++;
              }
              number_rows++;
            }
          for (x=0; x < (int) image->columns; x++)
          {
            y_vector[x].red+=y_scale*x_vector[x].red;
            y_vector[x].green+=y_scale*x_vector[x].green;
            y_vector[x].blue+=y_scale*x_vector[x].blue;
            y_vector[x].opacity+=y_scale*x_vector[x].opacity;
          }
          y_span-=y_scale;
          y_scale=(double) scale_image->rows/image->rows;
          next_row=True;
        }
        if (next_row && (number_rows < (int) image->rows))
          {
            /*
              Read a new scanline.
            */
            p=GetPixelCache(image,0,i++,image->columns,1);
            if (p == (PixelPacket *) NULL)
              break;
            for (x=0; x < (int) image->columns; x++)
            {
              x_vector[x].red=p->red;
              x_vector[x].green=p->green;
              x_vector[x].blue=p->blue;
              x_vector[x].opacity=p->opacity;
              p++;
            }
            number_rows++;
            next_row=False;
          }
        s=scanline;
        for (x=0; x < (int) image->columns; x++)
        {
          red=y_vector[x].red+y_span*x_vector[x].red;
          green=y_vector[x].green+y_span*x_vector[x].green;
          blue=y_vector[x].blue+y_span*x_vector[x].blue;
          opacity=y_vector[x].opacity+y_span*x_vector[x].opacity;
          s->red=red > MaxRGB ? MaxRGB : red;
          s->green=green > MaxRGB ? MaxRGB : green;
          s->blue=blue > MaxRGB ? MaxRGB : blue;
          s->opacity=opacity > Opaque ? Opaque : opacity;
          s++;
          y_vector[x].red=0;
          y_vector[x].green=0;
          y_vector[x].blue=0;
          y_vector[x].opacity=0;
        }
        y_scale-=y_span;
        if (y_scale <= 0)
          {
            y_scale=(double) scale_image->rows/image->rows;
            next_row=True;
          }
        y_span=1.0;
      }
    if (scale_image->columns == image->columns)
      {
        /*
          Transfer scanline to scaled image.
        */
        s=scanline;
        for (x=0; x < (int) scale_image->columns; x++)
        {
          q->red=s->red;
          q->green=s->green;
          q->blue=s->blue;
          q->opacity=s->opacity;
          q++;
          s++;
        }
      }
    else
      {
        /*
          Scale X direction.
        */
        red=0;
        green=0;
        blue=0;
        opacity=0;
        next_column=False;
        x_span=1.0;
        s=scanline;
        t=scale_scanline;
        for (x=0; x < (int) image->columns; x++)
        {
          x_scale=(double) scale_image->columns/image->columns;
          while (x_scale >= x_span)
          {
            if (next_column)
              {
                red=0;
                green=0;
                blue=0;
                opacity=0;
                t++;
              }
            red+=x_span*s->red;
            green+=x_span*s->green;
            blue+=x_span*s->blue;
            opacity+=x_span*s->opacity;
            t->red=red > MaxRGB ? MaxRGB : red;
            t->green=green > MaxRGB ? MaxRGB : green;
            t->blue=blue > MaxRGB ? MaxRGB : blue;
            t->opacity=opacity > Opaque ? Opaque : opacity;
            x_scale-=x_span;
            x_span=1.0;
            next_column=True;
          }
        if (x_scale > 0)
          {
            if (next_column)
              {
                red=0;
                green=0;
                blue=0;
                opacity=0;
                next_column=False;
                t++;
              }
            red+=x_scale*s->red;
            green+=x_scale*s->green;
            blue+=x_scale*s->blue;
            opacity+=x_scale*s->opacity;
            x_span-=x_scale;
          }
        s++;
      }
      if (x_span > 0)
        {
          s--;
          red+=x_span*s->red;
          green+=x_span*s->green;
          blue+=x_span*s->blue;
          opacity+=x_span*s->opacity;
        }
      if (!next_column && ((t-scale_scanline) < scale_image->columns))
        {
          t->red=red > MaxRGB ? MaxRGB : red;
          t->green=green > MaxRGB ? MaxRGB : green;
          t->blue=blue > MaxRGB ? MaxRGB : blue;
          t->opacity=opacity > Opaque ? Opaque : opacity;
        }
      /*
        Transfer scanline to scaled image.
      */
      t=scale_scanline;
      for (x=0; x < (int) scale_image->columns; x++)
      {
        q->red=t->red;
        q->green=t->green;
        q->blue=t->blue;
        q->opacity=t->opacity;
        q++;
        t++;
      }
    }
    if (!SyncPixelCache(scale_image))
      break;
    if (QuantumTick(y,scale_image->rows))
      ProgressMonitor(ScaleImageText,y,scale_image->rows);
  }
  /*
    Free allocated memory.
  */
  FreeMemory(y_vector);
  FreeMemory(scale_scanline);
  if (scale_image->rows != image->rows)
    FreeMemory(scanline);
  FreeMemory(x_vector);
  return(scale_image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   Z o o m I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ZoomImage creates a new image that is a scaled size of an
%  existing one.  It allocates the memory necessary for the new Image
%  structure and returns a pointer to the new image.  The Point filter gives
%  fast pixel replication, Triangle is equivalent to bi-linear interpolation,
%  and Mitchel giver slower, very high-quality results.  See Graphic Gems III
%  for details on this algorithm.
%
%  The filter member of the Image structure specifies which image filter to
%  use. Blur specifies the blur factor where > 1 is blurry, < 1 is sharp.
%
%  The format of the ZoomImage method is:
%
%      Image *ZoomImage(Image *image,const unsigned int columns,
%        const unsigned int rows)
%
%  A description of each parameter follows:
%
%    o zoom_image: Method ZoomImage returns a pointer to the image after
%      scaling.  A null image is returned if there is a memory shortage.
%
%    o image: The address of a structure of type Image.
%
%    o columns: An integer that specifies the number of columns in the zoom
%      image.
%
%    o rows: An integer that specifies the number of rows in the scaled
%      image.
%
%
*/

#define ZoomImageText  "  Zooming image...  "

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

static double Box(double x)
{
  if ((x >= -0.5) && (x < 0.5))
    return(1.0);
  return(0.0);
}

static double Bessel(double x)
{
  if (x == 0.0)
    return(M_PI/4.0);
  return(BesselOrderOne(M_PI*x)/(2.0*x));
}

static double Blackman(double x)
{
  return(0.42+0.50*cos(M_PI*x)+0.08*cos(2.0*M_PI*x));
}

static double Catrom(double x)
{
  if (x < 0)
    x=(-x);
  if (x < 1.0)
    return(0.5*(2.0+x*x*(-5.0+x*3.0)));
  if (x < 2.0)
    return(0.5*(4.0+x*(-8.0+x*(5.0-x))));
  return(0.0);
}

static double Cubic(double x)
{
  if (x < 0)
    x=(-x);
  if (x < 1.0)
    return((0.5*x*x*x)-x*x+(2.0/3.0));
  if (x < 2.0)
    {
      x=2.0-x;
      return((1.0/6.0)*x*x*x);
    }
  return(0.0);
}

static double Gaussian(double x)
{
  return(exp(-2.0*x*x)*sqrt(2.0/M_PI));
}

static double Hanning(double x)
{
  return(0.5+0.5*cos(M_PI*x));
}

static double Hamming(double x)
{
  return(0.54+0.46*cos(M_PI*x));
}

static double Hermite(double x)
{
  if (x < 0)
    x=(-x);
  if (x < 1.0)
    return((2.0*x-3.0)*x*x+1.0);
  return(0.0);
}

static double Sinc(double x)
{
  x*=M_PI;
  if (x != 0.0)
    return(sin(x)/x);
  return(1.0);
}

static double Lanczos(double x)
{
  if (x < 0)
    x=(-x);
  if (x < 3.0)
   return(Sinc(x)*Sinc(x/3.0));
  return(0.0);
}

static double Mitchell(double x)
{
  double
    b,
    c;

  b=1.0/3.0;
  c=1.0/3.0;
  if (x < 0)
    x=(-x);
  if (x < 1.0)
    {
      x=((12.0-9.0*b-6.0*c)*(x*x*x))+((-18.0+12.0*b+6.0*c)*x*x)+(6.0-2.0*b);
      return(x/6.0);
    }
 if (x < 2.0)
   {
     x=((-1.0*b-6.0*c)*(x*x*x))+((6.0*b+30.0*c)*x*x)+((-12.0*b-48.0*c)*x)+
       (8.0*b+24.0*c);
     return(x/6.0);
   }
  return(0.0);
}

static double Quadratic(double x)
{
  if (x < 0)
    x=(-x);
  if (x < 0.5)
    return(0.75-x*x);
  if (x < 1.5)
    {
      x-=1.5;
      return(0.5*x*x);
    }
  return(0.0);
}

static double Triangle(double x)
{
  if (x < 0.0)
    x=(-x);
  if (x < 1.0)
    return(1.0-x);
  return(0.0);
}

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

static unsigned int HorizontalFilter(Image *source,Image *destination,
  double x_factor,const FilterInfo *filter_info,
  ContributionInfo *contribution_info,const Quantum *range_limit,
  const unsigned int span,unsigned int *quantum)
{
  double
    blue_weight,
    center,
    density,
    green_weight,
    opacity_weight,
    red_weight,
    scale_factor,
    support;

  int
    end,
    j,
    n,
    start,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q;

  /*
    Apply filter to zoom horizontally from source to destination.
  */
  scale_factor=source->blur*Max(1.0/x_factor,1.0);
  support=Max(scale_factor*filter_info->support,0.5);
  destination->class=source->class;
  if (support > 0.5)
    destination->class=DirectClass;
  else
    {
      /*
        Reduce to point sampling.
      */
      support=0.5;
      scale_factor=1.0;
    }
  support+=1.0e-7;
  for (x=0; x < (int) destination->columns; x++)
  {
    density=0.0;
    n=0;
    center=(double) x/x_factor;
    start=(int) (center-support+0.5);
    end=(int) (center+support+0.5);
    for (i=Max(start,0); i < Min(end,(int) source->columns); i++)
    {
      contribution_info[n].pixel=i;
      contribution_info[n].weight=
        filter_info->function(((double) i-center+0.5)/scale_factor);
      contribution_info[n].weight/=scale_factor;
      density+=contribution_info[n].weight;
      n++;
    }
    if ((density != 0.0) && (density != 1.0))
      for (i=0; i < n; i++)
        contribution_info[i].weight/=density;  /* normalize */
    p=GetPixelCache(source,contribution_info[0].pixel,0,
      contribution_info[n-1].pixel-contribution_info[0].pixel+1,source->rows);
    q=SetPixelCache(destination,x,0,1,destination->rows);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (y=0; y < (int) destination->rows; y++)
    {
      j=0;
      blue_weight=0.0;
      green_weight=0.0;
      red_weight=0.0;
      opacity_weight=0.0;
      for (i=0; i < n; i++)
      {
        j=y*(contribution_info[n-1].pixel-contribution_info[0].pixel+1)+
          (contribution_info[i].pixel-contribution_info[0].pixel);
        red_weight+=contribution_info[i].weight*(p+j)->red;
        green_weight+=contribution_info[i].weight*(p+j)->green;
        blue_weight+=contribution_info[i].weight*(p+j)->blue;
        opacity_weight+=contribution_info[i].weight*(p+j)->opacity;
      }
      q->red=range_limit[(int) (red_weight+0.5)];
      q->green=range_limit[(int) (green_weight+0.5)];
      q->blue=range_limit[(int) (blue_weight+0.5)];
      q->opacity=range_limit[(int) (opacity_weight+0.5)];
      if (destination->class == PseudoClass)
        destination->indexes[y]=source->indexes[j];
      q++;
    }
    if (!SyncPixelCache(destination))
      break;
    if (QuantumTick(*quantum,span))
      ProgressMonitor(ZoomImageText,*quantum,span);
    (*quantum)++;
  }
  return(x == (int) destination->columns);
}

static unsigned int VerticalFilter(Image *source,Image *destination,
  double y_factor,const FilterInfo *filter_info,
  ContributionInfo *contribution_info,const Quantum *range_limit,
  const unsigned int span,unsigned int *quantum)
{
  double
    blue_weight,
    center,
    density,
    green_weight,
    opacity_weight,
    red_weight,
    scale_factor,
    support;

  int
    end,
    j,
    n,
    start,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q;

  /*
    Apply filter to zoom vertically from source to destination.
  */
  scale_factor=source->blur*Max(1.0/y_factor,1.0);
  support=Max(scale_factor*filter_info->support,0.5);
  destination->class=source->class;
  if (support > 0.5)
    destination->class=DirectClass;
  else
    {
      /*
        Reduce to point sampling.
      */
      support=0.5;
      scale_factor=1.0;
    }
  support+=1.0e-7;
  for (y=0; y < (int) destination->rows; y++)
  {
    density=0.0;
    n=0;
    center=(double) y/y_factor;
    start=(int) (center-support+0.5);
    end=(int) (center+support+0.5);
    for (i=Max(start,0); i < Min(end,(int) source->rows); i++)
    {
      contribution_info[n].pixel=i;
      contribution_info[n].weight=
        filter_info->function(((double) i-center+0.5)/scale_factor);
      contribution_info[n].weight/=scale_factor;
      density+=contribution_info[n].weight;
      n++;
    }
    if ((density != 0.0) && (density != 1.0))
      for (i=0; i < n; i++)
        contribution_info[i].weight/=density;  /* normalize */
    p=GetPixelCache(source,0,contribution_info[0].pixel,source->columns,
      contribution_info[n-1].pixel-contribution_info[0].pixel+1);
    q=SetPixelCache(destination,0,y,destination->columns,1);
    if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
      break;
    for (x=0; x < (int) destination->columns; x++)
    {
      j=0;
      blue_weight=0.0;
      green_weight=0.0;
      red_weight=0.0;
      opacity_weight=0.0;
      for (i=0; i < n; i++)
      {
        j=(contribution_info[i].pixel-contribution_info[0].pixel)*
          source->columns+x;
        red_weight+=contribution_info[i].weight*(p+j)->red;
        green_weight+=contribution_info[i].weight*(p+j)->green;
        blue_weight+=contribution_info[i].weight*(p+j)->blue;
        opacity_weight+=contribution_info[i].weight*(p+j)->opacity;
      }
      q->red=range_limit[(int) (red_weight+0.5)];
      q->green=range_limit[(int) (green_weight+0.5)];
      q->blue=range_limit[(int) (blue_weight+0.5)];
      q->opacity=range_limit[(int) (opacity_weight+0.5)];
      if (destination->class == PseudoClass)
        destination->indexes[x]=source->indexes[j];
      q++;
    }
    if (!SyncPixelCache(destination))
      break;
    if (QuantumTick(*quantum,span))
      ProgressMonitor(ZoomImageText,*quantum,span);
    (*quantum)++;
  }
  return(y == (int) destination->rows);
}

Export Image *ZoomImage(Image *image,const unsigned int columns,
  const unsigned int rows)
{
  ContributionInfo
    *contribution_info;

  double
    support,
    x_factor,
    y_factor;

  Image
    *source_image,
    *zoom_image;

  Quantum
    *range_table;

  register int
     i;

  register Quantum
    *range_limit;

  static const FilterInfo
    filters[SincFilter+1] =
    {
      { Box, 0.0 },
      { Box, 0.0 },
      { Box, 0.5 },
      { Triangle, 1.0 },
      { Hermite, 1.0 },
      { Hanning, 1.0 },
      { Hamming, 1.0 },
      { Blackman, 1.0 },
      { Gaussian, 1.25 },
      { Quadratic, 1.5 },
      { Cubic, 2.0 },
      { Catrom, 2.0 },
      { Mitchell, 2.0 },
      { Lanczos, 3.0 },
      { Bessel, 3.2383 },
      { Sinc, 4.0 }
    };

  unsigned int
    quantum,
    span,
    status;

  assert(image != (Image *) NULL);
  assert((image->filter >= 0) && (image->filter <= SincFilter));
  if ((columns == 0) || (rows == 0))
    {
      MagickWarning(OptionWarning,"Unable to zoom image",
        "image dimensions are zero");
      return((Image *) NULL);
    }
  if ((columns == image->columns) && (rows == image->rows))
    return(CloneImage(image,columns,rows,False));
  /*
    Initialize zoom image attributes.
  */
  zoom_image=CloneImage(image,columns,rows,False);
  if (zoom_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to zoom image",
        "Memory allocation failed");
      return((Image *) NULL);
    }
  if (zoom_image->rows >= image->rows)
    source_image=CloneImage(image,zoom_image->columns,image->rows,True);
  else
    source_image=CloneImage(image,image->columns,zoom_image->rows,True);
  if (source_image == (Image *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to zoom image",
        "Memory allocation failed");
      DestroyImage(zoom_image);
      return((Image *) NULL);
    }
  /*
    Allocate the range table.
  */
  range_table=(Quantum *) AllocateMemory(3*(MaxRGB+1)*sizeof(Quantum));
  if (range_table == (Quantum *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to zoom image",
        "Memory allocation failed");
      DestroyImage(source_image);
      DestroyImage(zoom_image);
      return((Image *) NULL);
    }
  /*
    Pre-compute conversion tables.
  */
  for (i=0; i <= MaxRGB; i++)
  {
    range_table[i]=0;
    range_table[i+(MaxRGB+1)]=i;
    range_table[i+(MaxRGB+1)*2]=MaxRGB;
  }
  range_limit=range_table+(MaxRGB+1);
  /*
    Allocate filter info list.
  */
  x_factor=(double) zoom_image->columns/(double) image->columns;
  y_factor=(double) zoom_image->rows/(double) image->rows;
  support=Max(filters[image->filter].support/x_factor,
    filters[image->filter].support/y_factor);
  if (support < filters[image->filter].support)
    support=filters[image->filter].support;
  contribution_info=(ContributionInfo *)
    AllocateMemory((int) (support*2+3)*sizeof(ContributionInfo));
  if (contribution_info == (ContributionInfo *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to zoom image",
        "Memory allocation failed");
      FreeMemory(range_table);
      DestroyImage(source_image);
      DestroyImage(zoom_image);
      return((Image *) NULL);
    }
  /*
    Zoom image.
  */
  quantum=0;
  if (zoom_image->rows >= image->rows)
    {
      span=source_image->columns+zoom_image->rows;
      status=HorizontalFilter(image,source_image,x_factor,
        &filters[image->filter],contribution_info,range_limit,span,&quantum);
      status|=VerticalFilter(source_image,zoom_image,y_factor,
        &filters[image->filter],contribution_info,range_limit,span,&quantum);
    }
  else
    {
      span=zoom_image->columns+source_image->columns;
      status=VerticalFilter(image,source_image,y_factor,&filters[image->filter],
        contribution_info,range_limit,span,&quantum);
      status|=HorizontalFilter(source_image,zoom_image,x_factor,
        &filters[image->filter],contribution_info,range_limit,span,&quantum);
    }
  if (status == False)
    MagickWarning(CacheWarning,"Unable to zoom image",(char *) NULL);
  /*
    Free allocated memory.
  */
  FreeMemory(contribution_info);
  FreeMemory(range_table);
  DestroyImage(source_image);
  return(zoom_image);
}
