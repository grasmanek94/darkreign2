/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                        DDDD   RRRR    AAA   W   W                           %
%                        D   D  R   R  A   A  W   W                           %
%                        D   D  RRRR   AAAAA  W   W                           %
%                        D   D  R R    A   A  W W W                           %
%                        DDDD   R  R   A   A   W W                            %
%                                                                             %
%                                                                             %
%                     ImageMagick Image Drawing Methods                       %
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
  Define declarations.
*/
#define MatteMatch(color,target,delta) \
  (ColorMatch(color,target,delta) && ((color).opacity == (target).opacity))
#define MaxStacksize  (1 << 15)
#define Push(up,left,right,delta) \
  if ((s < (segment_stack+MaxStacksize)) && (((up)+(delta)) >= 0) && \
      (((up)+(delta)) < (int) image->rows)) \
    { \
      s->y1=(up); \
      s->x1=(left); \
      s->x2=(right); \
      s->y2=(delta); \
      s++; \
    }

/*
  Typedef declaractions.
*/
typedef struct _PrimitiveInfo
{
  PrimitiveType
    primitive;

  unsigned int
    coordinates;

  double
    x,
    y;

  PaintMethod
    method;

  char
    *text;
} PrimitiveInfo;

/*
  Forward declarations
*/
static unsigned short
  InsidePrimitive(PrimitiveInfo *,const AnnotateInfo *,const PointInfo *,
    Image *);

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C o l o r F l o o d f i l l I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ColorFloodfillImage floodfills the designated area with a color.
%  The floodfill algorithm is strongly based on a similar algorithm in
%  "Graphics Gems" by Paul Heckbert.
%
%  The format of the ColorFloodfillImage method is:
%
%      void ColorFloodfillImage(Image *image,const PixelPacket *target,
%        Image *tile,const int x_offset,const int y_offset,
%        const PaintMethod method)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o target: A PixelPacket structure.  This is the RGB value of the target
%      color.
%
%    o tile: An image representing the image to tile onto the floodplane.
%
%    o x,y: Unsigned integers representing the current location of the pen.
%
%    o method: drawing method of type PrimitiveType: floodfill or fill to
%      border.
%
%
*/
Export void ColorFloodfillImage(Image *image,const PixelPacket *target,
  Image *tile,const int x_offset,const int y_offset,const PaintMethod method)
{
  PixelPacket
    color;

  int
    offset,
    skip,
    start,
    x1,
    x2,
    y;

  register int
    i,
    x;

  register PixelPacket
    *p,
    *q;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  assert(tile != (Image *) NULL);
  if ((x_offset < 0) || (x_offset >= (int) image->columns))
    return;
  if ((y_offset < 0) || (y_offset >= (int) image->rows))
    return;
  /*
    Set floodfill color.
  */
  (void) QueryColorDatabase("black",&color);
  p=GetPixelCache(tile,0,0,1,1);
  if (p != (PixelPacket *) NULL)
    color=(*p);
  if (ColorMatch(color,*target,image->fuzz))
    return;
  segment_stack=(SegmentInfo *)
    AllocateMemory(MaxStacksize*sizeof(SegmentInfo));
  if (segment_stack == (SegmentInfo *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to floodfill image",
        "Memory allocation failed");
      return;
    }
  /*
    Track "hot" pixels with the image index packets.
  */
  image->class=PseudoClass;
  for (y=0; y < (int) image->rows; y++)
  {
    if (!GetPixelCache(image,0,y,image->columns,1))
      break;
    for (x=0; x < (int) image->columns; x++)
      image->indexes[x]=False;
    if (!SyncPixelCache(image))
      break;
  }
  /*
    Push initial segment on stack.
  */
  x=x_offset;
  y=y_offset;
  start=0;
  s=segment_stack;
  Push(y,x,x,1);
  Push(y+1,x,x,-1);
  while (s > segment_stack)
  {
    /*
      Pop segment off stack.
    */
    s--;
    x1=(int) s->x1;
    x2=(int) s->x2;
    offset=(int) s->y2;
    y=(int) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    q=GetPixelCache(image,0,y,x1+1,1);
    if (q == (PixelPacket *) NULL)
      break;
    q+=x1;
    for (x=x1; x >= 0 ; x--)
    {
      if (method == FloodfillMethod)
        {
          if (!ColorMatch(*q,*target,image->fuzz))
            break;
        }
      else
        if (ColorMatch(*q,*target,image->fuzz) ||
            ColorMatch(*q,color,image->fuzz))
          break;
      image->indexes[x]=True;
      *q=color;
      q--;
    }
    if (!SyncPixelCache(image))
      break;
    skip=x >= x1;
    if (!skip)
      {
        start=x+1;
        if (start < x1)
          Push(y,start,x1-1,-offset);
        x=x1+1;
      }
    do
    {
      if (!skip)
        {
          if (x < image->columns)
            {
              q=GetPixelCache(image,x,y,image->columns-x,1);
              if (q == (PixelPacket *) NULL)
                break;
              for (i=0; x < (int) image->columns; x++)
              {
                if (method == FloodfillMethod)
                  {
                    if (!ColorMatch(*q,*target,image->fuzz))
                      break;
                  }
                else
                  if (ColorMatch(*q,*target,image->fuzz) ||
                      ColorMatch(*q,color,image->fuzz))
                    break;
                image->indexes[i++]=True;
                *q=color;
                q++;
              }
              if (!SyncPixelCache(image))
                break;
            }
          Push(y,start,x-1,offset);
          if (x > (x2+1))
            Push(y,x2+1,x-1,-offset);
        }
      skip=False;
      x++;
      if (x <= x2)
        {
          q=GetPixelCache(image,x,y,x2-x+1,1);
          if (q == (PixelPacket *) NULL)
            break;
          for ( ; x <= x2; x++)
          {
            if (method == FloodfillMethod)
              {
                if (ColorMatch(*q,*target,image->fuzz))
                  break;
              }
            else
              if (!ColorMatch(*q,*target,image->fuzz) &&
                  !ColorMatch(*q,color,image->fuzz))
                break;
            q++;
          }
        }
      start=x;
    } while (x <= x2);
  }
  /*
    Tile image onto floodplane.
  */
  for (y=0; y < (int) image->rows; y++)
  {
    q=GetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      if (image->indexes[x])
        {
          p=GetPixelCache(tile,x % tile->columns,y % tile->rows,1,1);
          if (p == (PixelPacket *) NULL)
            break;
          if (!tile->matte)
            *q=(*p);
          else
            {
              q->red=((unsigned long) (p->red*p->opacity+
                q->red*(Opaque-p->opacity))/Opaque);
              q->green=((unsigned long) (p->green*p->opacity+
                q->green*(Opaque-p->opacity))/Opaque);
              q->blue=((unsigned long) (p->blue*p->opacity+
                q->blue*(Opaque-p->opacity))/Opaque);
              q->opacity=((unsigned long) (p->opacity*
                p->opacity+q->opacity*(Opaque-p->opacity))/Opaque);
            }
        }
      q++;
    }
    if (!SyncPixelCache(image))
      break;
  }
  image->class=DirectClass;
  FreeMemory(segment_stack);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D r a w I m a g e                                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DrawImage draws a primitive (line, rectangle, ellipse) on the
%  image.
%
%  The format of the DrawImage method is:
%
%      void DrawImage(Image *image,const AnnotateInfo *annotate_info)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o annotate_info: The address of a DrawInfo structure.
%
%
*/
Export void DrawImage(Image *image,const AnnotateInfo *annotate_info)
{
#define DrawImageText  "  Drawing on image...  "

  AnnotateInfo
    *local_info;

  char
    keyword[MaxTextExtent],
    *primitive;

  double
    mid;

  int
    j,
    n,
    y;

  PointInfo
    point,
    target;

  PrimitiveInfo
    *primitive_info;

  PrimitiveType
    primitive_type;

  SegmentInfo
    bounds;

  register char
    *p;

  register int
    i,
    x;

  register PixelPacket
    *q;

  unsigned int
    indirection,
    length,
    number_coordinates;

  unsigned short
    opacity;

  /*
    Ensure the annotation info is valid.
  */
  assert(image != (Image *) NULL);
  assert(annotate_info != (AnnotateInfo *) NULL);
  assert(annotate_info->primitive != (char *) NULL);
  assert(annotate_info->tile != (Image *) NULL);
  if (*annotate_info->primitive == '\0')
    return;
  local_info=CloneAnnotateInfo(annotate_info->image_info,annotate_info);
  primitive=local_info->primitive;
  indirection=(*primitive == '@');
  if (indirection)
    {
      FILE
        *file;

      int
        c;

      register char
        *q;

      /*
        Read text from a file.
      */
      file=(FILE *) fopen(primitive+1,"r");
      if (file == (FILE *) NULL)
        {
          MagickWarning(FileOpenWarning,"Unable to read primitive file",
            primitive+1);
          DestroyAnnotateInfo(local_info);
          return;
        }
      length=MaxTextExtent;
      primitive=(char *) AllocateMemory(length);
      q=primitive;
      while (primitive != (char *) NULL)
      {
        c=fgetc(file);
        if (c == '#')
          {
            /*
              Eat comments.
            */
            for (c=fgetc(file); c != EOF; c=fgetc(file))
              if ((c == '\r') || (c == '\n'))
                break;
            continue;
          }
        if (c == EOF)
          break;
        if ((q-primitive+1) >= (int) length)
          {
            *q='\0';
            length<<=1;
            primitive=(char *) ReallocateMemory(primitive,length);
            if (primitive == (char *) NULL)
              break;
            q=primitive+Extent(primitive);
          }
        *q++=(unsigned char) c;
      }
      (void) fclose(file);
      if (primitive == (char *) NULL)
        {
          MagickWarning(ResourceLimitWarning,"Unable to draw image",
            "Memory allocation failed");
          DestroyAnnotateInfo(local_info);
          return;
        }
      *q='\0';
    }
  /*
    Allocate primitive info memory.
  */
  number_coordinates=2048;
  primitive_info=(PrimitiveInfo *)
    AllocateMemory(number_coordinates*sizeof(PrimitiveInfo));
  local_info->geometry=(char *) AllocateMemory(MaxTextExtent*sizeof(char));
  local_info->text=(char *) AllocateMemory(Extent(primitive)*sizeof(char));
  if ((primitive_info == (PrimitiveInfo *) NULL) ||
      (local_info->geometry == (char *) NULL) ||
      (local_info->text == (char *) NULL))
    {
      MagickWarning(ResourceLimitWarning,"Unable to draw image",
        "Memory allocation failed");
      if (indirection)
        FreeMemory(primitive);
      DestroyAnnotateInfo(local_info);
      return;
    }
  /*
    Parse the primitive attributes.
  */
  primitive_type=UndefinedPrimitive;
  p=primitive;
  bounds.x1=image->columns-1;
  bounds.y1=image->rows-1;
  bounds.x2=0;
  bounds.y2=0;
  for (i=0; *p != '\0'; )
  {
    /*
      Define primitive.
    */
    while (isspace((int) (*p)))
      p++;
    for (x=0; isalpha((int) (*p)); x++)
      keyword[x]=(*p++);
    keyword[x]='\0';
    if (*keyword == '\0')
      break;
    primitive_type=UndefinedPrimitive;
    if (Latin1Compare("Point",keyword) == 0)
      primitive_type=PointPrimitive;
    if (Latin1Compare("Line",keyword) == 0)
      primitive_type=LinePrimitive;
    if (Latin1Compare("Rectangle",keyword) == 0)
      primitive_type=RectanglePrimitive;
    if (Latin1Compare("FillRectangle",keyword) == 0)
      primitive_type=FillRectanglePrimitive;
    if (Latin1Compare("Circle",keyword) == 0)
      primitive_type=CirclePrimitive;
    if (Latin1Compare("FillCircle",keyword) == 0)
      primitive_type=FillCirclePrimitive;
    if (Latin1Compare("Ellipse",keyword) == 0)
      primitive_type=EllipsePrimitive;
    if (Latin1Compare("FillEllipse",keyword) == 0)
      primitive_type=FillEllipsePrimitive;
    if (Latin1Compare("Polygon",keyword) == 0)
      primitive_type=PolygonPrimitive;
    if (Latin1Compare("FillPolygon",keyword) == 0)
      primitive_type=FillPolygonPrimitive;
    if (Latin1Compare("Color",keyword) == 0)
      primitive_type=ColorPrimitive;
    if (Latin1Compare("Matte",keyword) == 0)
      primitive_type=MattePrimitive;
    if (Latin1Compare("Text",keyword) == 0)
      primitive_type=TextPrimitive;
    if (Latin1Compare("Image",keyword) == 0)
      primitive_type=ImagePrimitive;
    if (primitive_type == UndefinedPrimitive)
      break;
    j=i;
    for (x=0; *p != '\0'; x++)
    {
      /*
        Define points.
      */
      while (isspace((int) (*p)))
        p++;
      if (!IsGeometry(p))
        break;
      point.x=0;
      point.y=0;
      n=0;
      (void) sscanf(p,"%lf%lf%n",&point.x,&point.y,&n);
      if (n == 0)
        (void) sscanf(p,"%lf,%lf%n",&point.x,&point.y,&n);
      if (n == 0)
        (void) sscanf(p,"%lf, %lf%n",&point.x,&point.y,&n);
      if (n == 0)
        (void) sscanf(p,"%lf %lf%n",&point.x,&point.y,&n);
      if (n == 0)
        {
          MagickWarning(OptionWarning,
            "Non-conforming drawing primitive definition",p);
          break;
        }
      if (point.x < bounds.x1)
        bounds.x1=point.x;
      if (point.y < bounds.y1)
        bounds.y1=point.y;
      if (point.x > bounds.x2)
        bounds.x2=point.x;
      if (point.y > bounds.y2)
        bounds.y2=point.y;
      primitive_info[i].primitive=primitive_type;
      primitive_info[i].coordinates=0;
      primitive_info[i].x=point.x;
      primitive_info[i].y=point.y;
      primitive_info[i].method=FloodfillMethod;
      p+=n;
      while (isspace((int) (*p)) || (*p == ','))
        p++;
      i++;
      if (i < (int) (number_coordinates-360-1))
        continue;
      number_coordinates<<=1;
      primitive_info=(PrimitiveInfo *) ReallocateMemory(primitive_info,
        number_coordinates*sizeof(PrimitiveInfo));
      if (primitive_info != (PrimitiveInfo *) NULL)
        continue;
      MagickWarning(ResourceLimitWarning,"Unable to draw image",
        "Memory allocation failed");
      if (indirection)
        FreeMemory(primitive);
      DestroyAnnotateInfo(local_info);
      return;
    }
    primitive_info[j].coordinates=x;
    primitive_info[j].method=FloodfillMethod;
    primitive_info[j].text=(char *) NULL;
    switch (primitive_type)
    {
      case PointPrimitive:
      default:
      {
        if (primitive_info[j].coordinates != 1)
          primitive_type=UndefinedPrimitive;
        break;
      }
      case LinePrimitive:
      case RectanglePrimitive:
      case FillRectanglePrimitive:
      {
        if (primitive_info[j].coordinates != 2)
          primitive_type=UndefinedPrimitive;
        break;
      }
      case CirclePrimitive:
      case FillCirclePrimitive:
      {
        double
          radius;

        if (primitive_info[j].coordinates != 2)
          {
            primitive_type=UndefinedPrimitive;
            break;
          }
        /*
          Determine circle bounding box.
        */
        x=(int) (primitive_info[j+1].x-primitive_info[j].x);
        y=(int) (primitive_info[j+1].y-primitive_info[j].y);
        radius=
          sqrt((double) (x*x+y*y))+local_info->image_info->linewidth/2.0+0.5;
        point.x=Max(primitive_info[j].x-radius,0);
        point.y=Max(primitive_info[j].y-radius,0);
        if (point.x < bounds.x1)
          bounds.x1=point.x;
        if (point.y < bounds.y1)
          bounds.y1=point.y;
        point.x=Min(primitive_info[j].x+radius,image->columns-1);
        point.y=Min(primitive_info[j].y+radius,image->rows-1);
        if (point.x > bounds.x2)
          bounds.x2=point.x;
        if (point.y > bounds.y2)
          bounds.y2=point.y;
        break;
      }
      case EllipsePrimitive:
      case FillEllipsePrimitive:
      {
        double
          n;

        PointInfo
          degrees,
          end,
          start;

        if (primitive_info[j].coordinates != 3)
          {
            primitive_type=UndefinedPrimitive;
            break;
          }
        /*
          Arc's are just short segmented polygons.
        */
        primitive_info[j].primitive=PolygonPrimitive;
        if (primitive_type == FillEllipsePrimitive)
          primitive_info[j].primitive=FillPolygonPrimitive;
        start.x=primitive_info[j].x;
        start.y=primitive_info[j].y;
        end.x=primitive_info[j+1].x/2;
        end.y=primitive_info[j+1].y/2;
        degrees.x=primitive_info[j+2].x;
        degrees.y=primitive_info[j+2].y;
        while (degrees.y < degrees.x)
          degrees.y+=360;
        i=j;
        if ((primitive_type == FillEllipsePrimitive) &&
            (fmod(degrees.y-degrees.x,360.0) != 0.0))
          {
            if (start.x < bounds.x1)
              bounds.x1=start.x;
            if (start.y < bounds.y1)
              bounds.y1=start.y;
            if (end.x > bounds.x2)
              bounds.x2=end.x;
            if (end.y > bounds.y2)
              bounds.y2=end.y;
            primitive_info[j].coordinates++;
            i++;
          }
        for (n=(degrees.x+1.0); n <= degrees.y; n+=1.0)
        {
          point.x=cos(DegreesToRadians(fmod(n,360.0)))*end.x+start.x;
          point.y=sin(DegreesToRadians(fmod(n,360.0)))*end.y+start.y;
          if (point.x < bounds.x1)
            bounds.x1=point.x;
          if (point.y < bounds.y1)
            bounds.y1=point.y;
          if (point.x > bounds.x2)
            bounds.x2=point.x;
          if (point.y > bounds.y2)
            bounds.y2=point.y;
          primitive_info[i].coordinates=0;
          primitive_info[i].x=point.x;
          primitive_info[i].y=point.y;
          primitive_info[j].coordinates++;
          i++;
        }
        break;
      }
      case PolygonPrimitive:
      case FillPolygonPrimitive:
      {
        if (primitive_info[j].coordinates < 3)
          primitive_type=UndefinedPrimitive;
        break;
      }
      case ColorPrimitive:
      case MattePrimitive:
      {
        if (primitive_info[j].coordinates != 1)
          {
            primitive_type=UndefinedPrimitive;
            break;
          }
        /*
          Define method.
        */
        while (isspace((int) (*p)))
          p++;
        for (x=0; isalpha((int) (*p)); x++)
          keyword[x]=(*p++);
        keyword[x]='\0';
        if (*keyword == '\0')
          break;
        if (Latin1Compare("point",keyword) == 0)
          primitive_info[j].method=PointMethod;
        else
          if (Latin1Compare("replace",keyword) == 0)
            primitive_info[j].method=ReplaceMethod;
          else
            if (Latin1Compare("floodfill",keyword) == 0)
              primitive_info[j].method=FloodfillMethod;
            else
              if (Latin1Compare("filltoborder",keyword) == 0)
                primitive_info[j].method=FillToBorderMethod;
              else
                if (Latin1Compare("reset",keyword) == 0)
                  primitive_info[j].method=ResetMethod;
                else
                  primitive_type=UndefinedPrimitive;
        while (isspace((int) (*p)))
          p++;
        break;
      }
      case TextPrimitive:
      {
        if (primitive_info[j].coordinates != 1)
          {
            primitive_type=UndefinedPrimitive;
            break;
          }
        if (*p != '\0')
          {
            primitive_info[j].text=p;
            if (*p == '"')
              {
                for (p++; *p != '\0'; p++)
                  if ((*p == '"') && (*(p-1) != '\\'))
                    break;
              }
            else
              if (*p == '\'')
                {
                  for (p++; *p != '\0'; p++)
                    if ((*p == '\'') && (*(p-1) != '\\'))
                      break;
                }
              else
                for (p++;  *p != '\0'; p++)
                  if (isspace((int) *p) && (*(p-1) != '\\'))
                    break;
            if (*p != '\0')
              p++;
          }
        break;
      }
      case ImagePrimitive:
      {
        Image
          *composite_image;

        ImageInfo
          *composite_info;

        if (primitive_info[j].coordinates != 1)
          {
            primitive_type=UndefinedPrimitive;
            break;
          }
        if (*p != '\0')
          {
            primitive_info[j].text=p;
            if (*p == '"')
              {
                for (p++; *p != '\0'; p++)
                  if ((*p == '"') && (*(p-1) != '\\'))
                    break;
              }
            else
              if (*p == '\'')
                {
                  for (p++; *p != '\0'; p++)
                    if ((*p == '\'') && (*(p-1) != '\\'))
                      break;
                }
              else
                for (p++;  *p != '\0'; p++)
                  if (isspace((int) *p) && (*(p-1) != '\\'))
                    break;
            if (*p != '\0')
              p++;
          }
        composite_info=CloneImageInfo((ImageInfo *) NULL);
        (void) strcpy(composite_info->filename,primitive_info[j].text);
        composite_image=ReadImage(composite_info);
        if (composite_image == (Image *) NULL)
          break;
        CompositeImage(image,ReplaceCompositeOp,composite_image,
          (int) point.x,(int) point.y);
        DestroyImage(composite_image);
        DestroyImageInfo(composite_info);
        break;
      }
    }
    if (primitive_type == UndefinedPrimitive)
      break;
  }
  primitive_info[i].primitive=UndefinedPrimitive;
  if (primitive_type == UndefinedPrimitive)
    {
      MagickWarning(OptionWarning,"Non-conforming drawing primitive definition",
        keyword);
      FreeMemory(primitive_info);
      if (indirection)
        FreeMemory(primitive);
      DestroyAnnotateInfo(local_info);
      return;
    }
  for (i=0; primitive_info[i].primitive != UndefinedPrimitive; i++)
    if ((primitive_info[i].method == ReplaceMethod) ||
        (primitive_info[i].method == ResetMethod))
      {
        /*
          Replace and reset methods affect the entire image.
        */
        bounds.x1=0;
        bounds.y1=0;
        bounds.x2=image->columns-1;
        bounds.y2=image->rows-1;
      }
  /*
    Account for linewidth.
  */
  mid=local_info->image_info->linewidth/2.0;
  if ((bounds.x1 != bounds.x2) || (bounds.y1 != bounds.y2))
    {
      bounds.x1=Max(bounds.x1-mid,0);
      bounds.y1=Max(bounds.y1-mid,0);
      bounds.x2=Min(bounds.x2+ceil(mid),image->columns-1);
      bounds.y2=Min(bounds.y2+ceil(mid),image->rows-1);
    }
  /*
    Draw the primitive on the image.
  */
  image->class=DirectClass;
  for (y=(int) bounds.y1; y <= (int) bounds.y2; y++)
  {
    target.y=y;
    q=GetPixelCache(image,(int) bounds.x1,y,(int) (bounds.x2-bounds.x1+1.0),1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=(int) bounds.x1; x <= (int) bounds.x2; x++)
    {
      target.x=x;
      opacity=InsidePrimitive(primitive_info,local_info,&target,image);
      if (!local_info->image_info->antialias)
        if (opacity != Transparent)
          opacity=Opaque;
      if (opacity != Transparent)
        {
          register PixelPacket
            *p;

          p=GetPixelCache(local_info->tile,x % local_info->tile->columns,
            y % local_info->tile->rows,1,1);
          if (p == (PixelPacket *) NULL)
            break;
          if (!local_info->tile->matte)
            {
              q->red=((unsigned long)
                (p->red*opacity+q->red*(Opaque-opacity))/Opaque);
              q->green=((unsigned long)
                (p->green*opacity+q->green*(Opaque-opacity))/Opaque);
              q->blue=((unsigned long)
                (p->blue*opacity+q->blue*(Opaque-opacity))/Opaque);
            }
          else
            {
              q->red=((unsigned long)
                (p->red*p->opacity+q->red*(Opaque-p->opacity))/Opaque);
              q->green=((unsigned long)
                (p->green*p->opacity+q->green*(Opaque-p->opacity))/Opaque);
              q->blue=((unsigned long)
                (p->blue*p->opacity+q->blue*(Opaque-p->opacity))/Opaque);
              q->opacity=((unsigned long)
                (p->opacity*p->opacity+q->opacity*(Opaque-p->opacity))/Opaque);
            }
        }
      q++;
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(DrawImageText,y,image->rows);
  }
  for (i=0; primitive_info[i].primitive != UndefinedPrimitive; i++)
    if ((primitive_info[i].primitive == MattePrimitive) &&
	(primitive_info[i].method == ResetMethod))
      image->matte=False;
  /*
    Free resources.
  */
  FreeMemory(primitive_info);
  if (indirection)
    FreeMemory(primitive);
  DestroyAnnotateInfo(local_info);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I n s i d e P r i m i t i v e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method InsidePrimitive returns the opacity of the pen at the (x,y) position
%  of the image.  The opacity is Opaque if the (x,y) position is within the
%  primitive as defined in primitive_info.  A value less than fully opaque
%  and greater than fully transparent is returned for a primitive edge pixel
%  to allow for anti-aliasing.  Otherwise fully transparent is returned.
%
%  Rick Mabry provided the algorithms for anti-aliased primitives.
%
%  The format of the InsidePrimitive method is:
%
%      unsigned short InsidePrimitive(PrimitiveInfo *primitive_info,
%        const AnnotateInfo *annotate_info,const PointInfo *pixel,Image *image)
%
%  A description of each parameter follows:
%
%    o opacity:  Method InsidePrimitive returns a pen opacity associated with
%      the (x,y) position of the image.
%
%    o primitive_info: Specifies a pointer to a PrimitiveInfo structure.
%
%    o annotate_info: Specifies a pointer to a AnnotateInfo structure.
%
%    o target: PointInfo representing the (x,y) location in the image.
%
%    o image: The address of a structure of type Image.
%
%
*/

static double DistanceToLine(const PointInfo *pixel,const SegmentInfo *line)
{
  register double
    alpha,
    beta,
    dot_product,
    gamma,
    v;

  alpha=pixel->x-line->x1;
  beta=pixel->y-line->y1;
  dot_product=alpha*(line->x2-line->x1)+beta*(line->y2-line->y1);
  if (dot_product <= 0)
    return(alpha*alpha+beta*beta);
  v=(line->x2-line->x1)*(line->x2-line->x1)+
    (line->y2-line->y1)*(line->y2-line->y1);
  gamma=dot_product*dot_product/v;
  if (gamma <= v)
    return(alpha*alpha+beta*beta-gamma);
  alpha=pixel->x-line->x2;
  beta=pixel->y-line->y2;
  return(alpha*alpha+beta*beta);
}

static unsigned short PixelOnLine(const PointInfo *pixel,
  const SegmentInfo *line,const double mid,const unsigned short opacity)
{
  register double
    alpha,
    distance;

  if ((mid == 0) || (opacity == Opaque))
    return(opacity);
  if ((line->x1 == line->x2) && (line->y1 == line->y2))
    return((pixel->x == line->x1) && (pixel->y == line->y1) ? Opaque : opacity);
  distance=DistanceToLine(pixel,line);
  alpha=mid-0.5;
  if (distance <= (alpha*alpha))
    return(Opaque);
  alpha=mid+0.5;
  if (distance <= (alpha*alpha))
    {
      alpha=sqrt(distance)-mid-0.5;
      return((unsigned short) Max((int) opacity,Opaque*alpha*alpha));
    }
  return(opacity);
}

static unsigned short InsidePrimitive(PrimitiveInfo *primitive_info,
  const AnnotateInfo *annotate_info,const PointInfo *pixel,Image *image)
{
  PixelPacket
    border_color;

  double
    alpha,
    beta,
    distance,
    mid,
    radius;

  register PrimitiveInfo
    *p,
    *q;

  register unsigned short
    opacity;

  PixelPacket
    target;

  SegmentInfo
    line;

  assert(primitive_info != (PrimitiveInfo *) NULL);
  assert(annotate_info != (AnnotateInfo *) NULL);
  assert(image != (Image *) NULL);
  opacity=Transparent;
  mid=annotate_info->image_info->linewidth/2.0;
  p=primitive_info;
  while (p->primitive != UndefinedPrimitive)
  {
    q=p+p->coordinates-1;
    switch (p->primitive)
    {
      case PointPrimitive:
      default:
      {
        if ((pixel->x == (int) (p->x+0.5)) && (pixel->y == (int) (p->y+0.5)))
          opacity=Opaque;
        break;
      }
      case LinePrimitive:
      {
        line.x1=p->x;
        line.y1=p->y;
        line.x2=q->x;
        line.y2=q->y;
        opacity=PixelOnLine(pixel,&line,mid,opacity);
        break;
      }
      case RectanglePrimitive:
      {
        if (((pixel->x >= (int) (Min(p->x-mid,q->x+mid)+0.5)) &&
             (pixel->x < (int) (Max(p->x-mid,q->x+mid)+0.5)) &&
             (pixel->y >= (int) (Min(p->y-mid,q->y+mid)+0.5)) &&
             (pixel->y < (int) (Max(p->y-mid,q->y+mid)+0.5))) &&
           !((pixel->x >= (int) (Min(p->x+mid,q->x-mid)+0.5)) &&
             (pixel->x < (int) (Max(p->x+mid,q->x-mid)+0.5)) &&
             (pixel->y >= (int) (Min(p->y+mid,q->y-mid)+0.5)) &&
             (pixel->y < (int) (Max(p->y+mid,q->y-mid)+0.5))))
          opacity=Opaque;
        break;
      }
      case FillRectanglePrimitive:
      {
        if ((pixel->x >= (int) (Min(p->x,q->x)+0.5)) &&
            (pixel->x <= (int) (Max(p->x,q->x)+0.5)) &&
            (pixel->y >= (int) (Min(p->y,q->y)+0.5)) &&
            (pixel->y <= (int) (Max(p->y,q->y)+0.5)))
          opacity=Opaque;
        break;
      }
      case CirclePrimitive:
      {
        alpha=p->x-pixel->x;
        beta=p->y-pixel->y;
        distance=sqrt(alpha*alpha+beta*beta);
        alpha=p->x-q->x;
        beta=p->y-q->y;
        radius=sqrt(alpha*alpha+beta*beta);
        beta=fabs(distance-radius);
        if (beta < (mid+0.5))
          {
            if (beta <= (mid-0.5))
              opacity=Opaque;
            else
              {
                alpha=mid-beta+0.5;
                opacity=(Quantum) Max((int) opacity,Opaque*alpha*alpha);
              }
          }
        break;
      }
      case FillCirclePrimitive:
      {
        alpha=p->x-pixel->x;
        beta=p->y-pixel->y;
        distance=sqrt(alpha*alpha+beta*beta);
        alpha=p->x-q->x;
        beta=p->y-q->y;
        radius=sqrt(alpha*alpha+beta*beta);
        if (distance <= (radius-1.0))
          opacity=Opaque;
        else
          if (distance < (radius+1.0))
            {
              alpha=(radius-distance+1.0)/2.0;
              opacity=(Quantum) Max((int) opacity,Opaque*alpha*alpha);
            }
        break;
      }
      case PolygonPrimitive:
      {
        unsigned short
          poly_opacity;

        poly_opacity=Transparent;
        for ( ; (p < q) && (poly_opacity != Opaque); p++)
        {
          line.x1=p->x;
          line.y1=p->y;
          line.x2=(p+1)->x;
          line.y2=(p+1)->y;
          poly_opacity=PixelOnLine(pixel,&line,mid,(unsigned short)
            Max(opacity,poly_opacity));
        }
        opacity=Max(opacity,poly_opacity);
        break;
      }
      case FillPolygonPrimitive:
      {
        double
          distance,
          minimum_distance;

        int
          crossing,
          crossings;

        PrimitiveInfo
          *r;

        unsigned short
          poly_opacity;

        r=p;
        crossings=0;
        if ((pixel->y < q->y) != (pixel->y < p->y))
          {
            crossing=pixel->x < q->x;
            if (crossing != (pixel->x < p->x))
              crossings+=pixel->x <
                (q->x-(q->y-pixel->y)*(p->x-q->x)/(p->y-q->y));
            else
              if (crossing)
                crossings++;
          }
        for (p++; p <= q; p++)
        {
          if (pixel->y < (p-1)->y)
            {
              while ((p <= q) && (pixel->y < p->y))
                p++;
              if (p > q)
                break;
              crossing=pixel->x < (p-1)->x;
              if (crossing != (pixel->x < p->x))
                crossings+=pixel->x < ((p-1)->x-((p-1)->y-pixel->y)*
                  (p->x-(p-1)->x)/(p->y-(p-1)->y));
              else
                if (crossing)
                  crossings++;
              continue;
            }
          while ((p <= q) && (pixel->y >= p->y))
            p++;
          if (p > q)
            break;
          crossing=pixel->x < (p-1)->x;
          if (crossing != (pixel->x < p->x))
            crossings+=pixel->x < ((p-1)->x-((p-1)->y-pixel->y)*
              (p->x-(p-1)->x)/(p->y-(p-1)->y));
          else
            if (crossing)
              crossings++;
        }
        /*
          Now find distance to polygon.
        */
        p=r;
        line.x1=p->x;
        line.y1=p->y;
        line.x2=q->x;
        line.y2=q->y;
        minimum_distance=DistanceToLine(pixel,&line);
        for ( ; p < q; p++)
        {
          line.x1=p->x;
          line.y1=p->y;
          line.x2=(p+1)->x;
          line.y2=(p+1)->y;
          distance=DistanceToLine(pixel,&line);
          if (distance < minimum_distance)
            minimum_distance=distance;
        }
        minimum_distance=sqrt(minimum_distance);
        if (crossings & 0x01)
          {
            poly_opacity=Opaque;
            if (minimum_distance < 0.5)
              {
                alpha=0.5+minimum_distance;
                poly_opacity=(Quantum) (Opaque*alpha*alpha);
              }
            opacity=Max(opacity,poly_opacity);
            break;
          }
        poly_opacity=Transparent;
        if (minimum_distance < 0.5)
          {
            alpha=0.5-minimum_distance;
            poly_opacity=(Quantum) (Opaque*alpha*alpha);
          }
        opacity=Max(opacity,poly_opacity);
        break;
      }
      case ColorPrimitive:
      {
        register PixelPacket
          *q;

        switch (p->method)
        {
          case PointMethod:
          default:
          {
            if ((pixel->x != (int) (p->x+0.5)) &&
                (pixel->y != (int) (p->y+0.5)))
              break;
            opacity=Opaque;
            break;
          }
          case ReplaceMethod:
          {
            PixelPacket
              color;

            static PixelPacket
              target;

            if ((pixel->x == 0) && (pixel->y == 0))
              {
                q=GetPixelCache(image,(int) p->x,(int) p->y,1,1);
                if (q != (PixelPacket *) NULL)
                  target=(*q);
              }
            q=GetPixelCache(image,(int) pixel->x,(int) pixel->y,1,1);
            if (q != (PixelPacket *) NULL)
              color=(*q);
            if (ColorMatch(color,target,(int) image->fuzz))
              opacity=Opaque;
            break;
          }
          case FloodfillMethod:
          case FillToBorderMethod:
          {
            if ((pixel->x != (int) (p->x+0.5)) &&
                (pixel->y != (int) (p->y+0.5)))
              break;
            q=GetPixelCache(image,(int) pixel->x,(int) pixel->y,1,1);
            if (q != (PixelPacket *) NULL)
              target=(*q);
            if (p->method == FillToBorderMethod)
              {
                border_color=annotate_info->image_info->border_color;
                target.red=border_color.red;
                target.green=border_color.green;
                target.blue=border_color.blue;
              }
            ColorFloodfillImage(image,&target,annotate_info->tile,
              (int) pixel->x,(int) pixel->y,p->method);
            break;
          }
          case ResetMethod:
          {
            opacity=Opaque;
            break;
          }
        }
        break;
      }
      case MattePrimitive:
      {
        register PixelPacket
          *q;

        if (!image->matte)
          MatteImage(image,Opaque);
        switch (p->method)
        {
          case PointMethod:
          default:
          {
            if ((pixel->x != (int) (p->x+0.5)) &&
                (pixel->y != (int) (p->y+0.5)))
              break;
            q=GetPixelCache(image,(int) pixel->x,(int) pixel->y,1,1);
            if (q != (PixelPacket *) NULL)
              {
                q->opacity=Transparent;
                (void) SyncPixelCache(image);
              }
            break;
          }
          case ReplaceMethod:
          {
            PixelPacket
              color;

            static PixelPacket
              target;

            if ((pixel->x == 0) && (pixel->y == 0))
              {
                q=GetPixelCache(image,(int) p->x,(int) p->y,1,1);
                if (q != (PixelPacket *) NULL)
                  target=(*q);
              }
            q=GetPixelCache(image,(int) pixel->x,(int) pixel->y,1,1);
            if (q != (PixelPacket *) NULL)
              color=(*q);
            if (ColorMatch(color,target,image->fuzz))
              {
                q->opacity=Transparent;
                (void) SyncPixelCache(image);
              }
            break;
          }
          case FloodfillMethod:
          case FillToBorderMethod:
          {
            if ((pixel->x != (int) (p->x+0.5)) &&
                (pixel->y != (int) (p->y+0.5)))
              break;
            q=GetPixelCache(image,(int) pixel->x,(int) pixel->y,1,1);
            if (q != (PixelPacket *) NULL)
              target=(*q);
            if (p->method == FillToBorderMethod)
              {
                border_color=annotate_info->image_info->border_color;
                target.red=border_color.red;
                target.green=border_color.green;
                target.blue=border_color.blue;
              }
            MatteFloodfillImage(image,&target,Transparent,(int) pixel->x,
              (int) pixel->y,p->method);
            break;
          }
          case ResetMethod:
          {
            q=GetPixelCache(image,(int) pixel->x,(int) pixel->y,1,1);
            if (q != (PixelPacket *) NULL)
              {
                q->opacity=Opaque;
                (void) SyncPixelCache(image);
              }
            break;
          }
        }
        break;
      }
      case TextPrimitive:
      {
        register char
          *r;

        if ((pixel->x != (int) (p->x+0.5)) && (pixel->y != (int) (p->y+0.5)))
          break;
        if (p->text == (char *) NULL)
          break;
        r=p->text;
        if (*r == '"')
          {
            p->text++;
            for (r++; *r != '\0'; r++)
              if ((*r == '"') && (*(r-1) != '\\'))
                break;
          }
        else
          if (*r == '\'')
            {
              p->text++;
              for (r++; *r != '\0'; r++)
                if ((*r == '\'') && (*(r-1) != '\\'))
                  break;
            }
          else
            for (r++;  *r != '\0'; r++)
              if (isspace((int) *r) && (*(r-1) != '\\'))
                break;
        (void) strncpy(annotate_info->text,p->text,r-p->text);
        annotate_info->text[r-p->text]='\0';
        FormatString(annotate_info->geometry,"%+d%+d",(int) p->x,(int) p->y);
        AnnotateImage(image,annotate_info);
        break;
      }
      case ImagePrimitive:
        break;
    }
    if (opacity == Opaque)
      return(opacity);
    while (p <= q)
      p++;
  }
  return(opacity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   M a t t e F l o o d f i l l I m a g e                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method MatteFloodfillImage floodfills the designated area with a matte
%  value.  The floodfill algorithm is strongly based on a similar algorithm in
%  "Graphics Gems" by Paul Heckbert.
%
%  The format of the MatteFloodfillImage method is:
%
%      void MatteFloodfillImage(Image *image,const PixelPacket *target,
%        const unsigned int matte,const int x_offset,const int y_offset,
%        const PaintMethod method)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image.
%
%    o target: A PixelPacket structure.  This is the RGB value of the target
%      color.
%
%    o matte: A integer value representing the amount of transparency.
%
%    o x,y: Unsigned integers representing the current location of the pen.
%
%    o method: drawing method of type PrimitiveType: floodfill or fill to
%      border.
%
%
*/
Export void MatteFloodfillImage(Image *image,const PixelPacket *target,
  const unsigned int matte,const int x_offset,const int y_offset,
  const PaintMethod method)
{
  int
    offset,
    skip,
    start,
    x1,
    x2,
    y;

  register int
    x;

  register PixelPacket
    *q;

  register SegmentInfo
    *s;

  SegmentInfo
    *segment_stack;

  /*
    Check boundary conditions.
  */
  assert(image != (Image *) NULL);
  if ((x_offset < 0) || (x_offset >= (int) image->columns))
    return;
  if ((y_offset < 0) || (y_offset >= (int) image->rows))
    return;
  if (target->opacity == (unsigned short) matte)
    return;
  q=GetPixelCache(image,x_offset,y_offset,1,1);
  if (q == (PixelPacket *) NULL)
    return;
  if (q->opacity == matte)
    return;
  /*
    Allocate segment stack.
  */
  segment_stack=(SegmentInfo *)
    AllocateMemory(MaxStacksize*sizeof(SegmentInfo));
  if (segment_stack == (SegmentInfo *) NULL)
    {
      MagickWarning(ResourceLimitWarning,"Unable to recolor image",
        "Memory allocation failed");
      return;
    }
  /*
    Push initial segment on stack.
  */
  image->class=DirectClass;
  if (!image->matte)
    MatteImage(image,Opaque);
  x=x_offset;
  y=y_offset;
  start=0;
  s=segment_stack;
  Push(y,x,x,1);
  Push(y+1,x,x,-1);
  while (s > segment_stack)
  {
    /*
      Pop segment off stack.
    */
    s--;
    x1=(int) s->x1;
    x2=(int) s->x2;
    offset=(int) s->y2;
    y=(int) s->y1+offset;
    /*
      Recolor neighboring pixels.
    */
    q=GetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    q+=x1;
    for (x=x1; x >= 0 ; x--)
    {
      if (method == FloodfillMethod)
        {
          if (!MatteMatch(*q,*target,image->fuzz))
            break;
        }
      else
        if (MatteMatch(*q,*target,image->fuzz) ||
            (q->opacity == (unsigned short) matte))
          break;
      q->opacity=(Quantum) matte;
      q--;
    }
    if (!SyncPixelCache(image))
      break;
    skip=x >= x1;
    if (!skip)
      {
        start=x+1;
        if (start < x1)
          Push(y,start,x1-1,-offset);
        x=x1+1;
      }
    do
    {
      if (!skip)
        {
          q=GetPixelCache(image,0,y,image->columns,1);
          if (q == (PixelPacket *) NULL)
            break;
          q+=x;
          for ( ; x < (int) image->columns; x++)
          {
            if (method == FloodfillMethod)
              {
                if (!MatteMatch(*q,*target,image->fuzz))
                  break;
              }
            else
              if (MatteMatch(*q,*target,image->fuzz) ||
                  (q->opacity == (unsigned short) matte))
                break;
            q->opacity=(Quantum) matte;
            q++;
          }
          if (!SyncPixelCache(image))
            break;
          Push(y,start,x-1,offset);
          if (x > (x2+1))
            Push(y,x2+1,x-1,-offset);
        }
      skip=False;
      q=GetPixelCache(image,0,y,image->columns,1);
      if (q == (PixelPacket *) NULL)
        break;
      q+=x;
      for (x++; x <= x2 ; x++)
      {
        q++;
        if (method == FloodfillMethod)
          {
            if (MatteMatch(*q,*target,image->fuzz))
              break;
          }
        else
          if (!MatteMatch(*q,*target,image->fuzz) &&
              (q->opacity != (unsigned short) matte))
            break;
      }
      start=x;
    } while (x <= x2);
  }
  FreeMemory(segment_stack);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%     O p a q u e I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method OpaqueImage changes the color of an opaque pixel to the pen color.
%
%  The format of the OpaqueImage method is:
%
%      void OpaqueImage(Image *image,const char *opaque_color,
%        const char *pen_color)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o opaque_color,
%      pen_color: A character string that contain an X11 color string.
%
%
*/
Export void OpaqueImage(Image *image,const char *opaque_color,
  const char *pen_color)
{
#define OpaqueImageText  "  Setting opaque color in the image...  "

  PixelPacket
    target,
    target_color;

  int
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  unsigned int
    status;

  /*
    Determine RGB values of the opaque color.
  */
  assert(image != (Image *) NULL);
  status=QueryColorDatabase(opaque_color,&target);
  if (status == False)
    return;
  status=QueryColorDatabase(pen_color,&target_color);
  if (status == False)
    return;
  /*
    Make image color opaque.
  */
  switch (image->class)
  {
    case DirectClass:
    default:
    {
      /*
        Make DirectClass image opaque.
      */
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          if (ColorMatch(*q,target,image->fuzz))
            *q=target_color;
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if (QuantumTick(y,image->rows))
          ProgressMonitor(OpaqueImageText,y,image->rows);
      }
      break;
    }
    case PseudoClass:
    {
      /*
        Make PseudoClass image opaque.
      */
      for (i=0; i < (int) image->colors; i++)
      {
        if (ColorMatch(image->colormap[i],target,image->fuzz))
          image->colormap[i]=target_color;
        if (QuantumTick(i,image->colors))
          ProgressMonitor(OpaqueImageText,i,image->colors);
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
%     T r a n s p a r e n t I m a g e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method TransparentImage creates a matte image associated with the
%  image.  All pixel locations are initially set to opaque.  Any pixel
%  that matches the specified color are set to transparent.
%
%  The format of the TransparentImage method is:
%
%      void TransparentImage(Image *image,const char *color)
%
%  A description of each parameter follows:
%
%    o image: The address of a structure of type Image;  returned from
%      ReadImage.
%
%    o color: A character string that contain an X11 color string.
%
%
*/
Export void TransparentImage(Image *image,const char *color)
{
#define TransparentImageText  "  Setting transparent color in the image...  "

  PixelPacket
    target;

  int
    y;

  register int
    x;

  register PixelPacket
    *q;

  unsigned int
    status;

  /*
    Determine RGB values of the transparent color.
  */
  assert(image != (Image *) NULL);
  status=QueryColorDatabase(color,&target);
  if (status == False)
    return;
  /*
    Make image color transparent.
  */
  if (!image->matte)
    MatteImage(image,Opaque);
  for (y=0; y < (int) image->rows; y++)
  {
    q=GetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      if (ColorMatch(*q,target,image->fuzz))
        q->opacity=Transparent;
      q++;
    }
    if (!SyncPixelCache(image))
      break;
    if (QuantumTick(y,image->rows))
      ProgressMonitor(TransparentImageText,y,image->rows);
  }
}
