/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     L       AAA   BBBB   EEEEE  L                           %
%                     L      A   A  B   B  E      L                           %
%                     L      AAAAA  BBBB   EEE    L                           %
%                     L      A   A  B   B  E      L                           %
%                     LLLLL  A   A  BBBB   EEEEE  LLLLL                       %
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
#if defined(HasX11)
#include "xwindows.h"
#endif
#if defined(HasTTF)
#if defined(HAVE_FREETYPE_FREETYPE_H)
#include "freetype/freetype.h"
#else
#include "freetype.h"
#endif
#endif

/*
  Font declaration.
*/
Export const char
  *DefaultXFont = "-adobe-helvetica-medium-r-*-*-14-*-*-*-*-*-iso8859-*";

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d L A B E L I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadLABELImage reads a LABEL image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadLABELImage method is:
%
%      Image *ReadLABELImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadLABELImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      if the image cannot be read.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/

static char *EscapeParenthesis(const char *text)
{
  int
    escapes;

  register char
    *p;

  register int
    i;

  static char
    buffer[MaxTextExtent];

  escapes=0;
  p=buffer;
  for (i=0; i < Min((int) strlen(text),(MaxTextExtent-escapes-1)); i++)
  {
    if ((text[i] == '(') || (text[i] == ')'))
      {
        *p++='\\';
        escapes++;
      }
    *p++=text[i];
  }
  *p='\0';
  return(buffer);
}

#if defined(HasTTF)
static void GetFontInfo(TT_Face face,TT_Face_Properties *face_properties,
  Image *image)
{
  char
    *name;

  static const unsigned short
    ids[] = { 4, 5, 0, 7 };

  register char
    *p;

  register int
    i,
    j;

  unsigned short
    encoding,
    id,
    language,
    length,
    platform;

  /*
    Note font info as image comment.
  */
  if (face_properties->num_Names == 0)
    return;
  image->label=(char *)
    AllocateMemory((face_properties->num_Names*MaxTextExtent)*sizeof(char));
  if (image->label == (char *) NULL)
    return;
  *image->label='\0';
  for (i=0; i < (int) (sizeof(ids)/sizeof(unsigned short)); i++)
  {
    TT_Get_Name_ID(face,ids[i],&platform,&encoding,&language,&id);
    if (((platform != 0) || (language != 0)) &&
        ((platform != 3) || (encoding > 1) || ((language & 0x3FF) != 0x009)))
      continue;
    TT_Get_Name_String(face,ids[i],&name,&length);
    p=image->label+strlen(image->label);
    for (j=1; j < (int) Min(length,MaxTextExtent); j+=2)
      *p++=name[j];
    *p='\0';
    break;
  }
  image->label=(char *) ReallocateMemory((char *)
    image->label,(strlen(image->label)+1)*sizeof(char));
}

static void RenderGlyph(TT_Raster_Map *canvas,TT_Raster_Map *character,
  TT_Glyph glyph,int x_offset,int y_offset,TT_Glyph_Metrics *glyph_metrics)
{
  int
    y;

  register int
    i,
    x;

  register unsigned char
    *p,
    *q;

  SegmentInfo
    bounds;

  /*
    Render Glyph.
  */
  q=(unsigned char *) character->bitmap;
  for (i=0; i < character->size; i++)
    *q++=0;
  TT_Get_Glyph_Pixmap(glyph,character,-(glyph_metrics->bbox.xMin & -64),
    -(glyph_metrics->bbox.yMin & -64));
  /*
    Composite character on canvas.
  */
  x_offset+=(glyph_metrics->bbox.xMin & -64)/64;
  y_offset=(-(glyph_metrics->bbox.yMin & -64)/64)-y_offset;
  bounds.x1=x_offset < 0 ? -x_offset : 0;
  bounds.y1=y_offset < 0 ? -y_offset : 0;
  bounds.x2=canvas->cols-x_offset;
  if (bounds.x2 > character->cols)
    bounds.x2=character->cols;
  bounds.y2=canvas->rows-y_offset;
  if (bounds.y2 > character->rows)
    bounds.y2=character->rows;
  if (bounds.x1 >= bounds.x2)
    return;
  for (y=(int) bounds.y1; y < (int) bounds.y2; y++)
  {
    p=((unsigned char *) character->bitmap)+y*character->cols+(int) bounds.x1;
    q=((unsigned char *) canvas->bitmap)+(y+y_offset)*canvas->cols+
      (int) bounds.x1+x_offset;
    for (x=(int) bounds.x1; x < bounds.x2; x++)
      *q++|=(*p++);
  }
}
#endif

Export Image *ReadLABELImage(const ImageInfo *image_info)
{
#define MaxGlyphs  65535

  char
    filename[MaxTextExtent],
    geometry[MaxTextExtent],
    text[MaxTextExtent],
    page[MaxTextExtent];

  PixelPacket
    pen_color;

  FILE
    *file;

  Image
    *image;

  ImageInfo
    *local_info;

  int
    y;

  RectangleInfo
    crop_info;

  register int
    x;

  register PixelPacket
    *p,
    *q;

  PixelPacket
    corner;

  /*
    Allocate image structure.
  */
  image=AllocateImage(image_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  /*
    Create image label.
  */
  local_info=CloneImageInfo(image_info);
  if (local_info->font == (char *) NULL)
    (void) CloneString(&local_info->font,DefaultXFont);
  (void) strcpy(text,local_info->filename);
  (void) QueryColorDatabase("black",&pen_color);
  if (local_info->pen != (char *) NULL)
    (void) QueryColorDatabase(local_info->pen,&pen_color);
  if (*local_info->font == '@')
    {
#if defined(HasTTF)
      char
        *path,
        *path_end;

      int
        character_map,
        length,
        number_glyphs;

      register int
        i;

      register unsigned char
        *p;

      TT_CharMap
        char_map;

      TT_Engine
        engine;

      TT_Error
        error;

      TT_Face
        face;

      TT_Face_Properties
        face_properties;

      TT_Glyph
        *glyphs;

      TT_Glyph_Metrics
        glyph_metrics;

      TT_Instance
        instance;

      TT_Instance_Metrics
        instance_metrics;

      TT_Raster_Map
        canvas,
        character;

      TT_UShort
        code;

      unsigned short
        encoding,
        platform,
        *unicode;

      /*
        Initialize font engine.
      */
      error=TT_Init_FreeType(&engine);
      if (error)
        ReaderExit(DelegateWarning,"Cannot initialize TTF engine",image);
      /*
        Search for Truetype font filename.
      */
      error=True;
      path=getenv("TT_FONT_PATH");
      if (path != (char *) NULL)
        {
          /*
            Environment variable TT_FONT_PATH.
          */
          for ( ; ; )
          {
            path_end=strchr(path,DirectoryListSeparator);
            if (path_end == (char *) NULL)
              (void) strcpy(filename,path);
            else
              {
                i=(int) (path_end-path);
                (void) strncpy(filename,path,i);
                filename[i]='\0';
              }
            i=strlen(filename);
            if ((i > 0) && (!IsBasenameSeparator(filename[i-1])))
              (void) strcat(filename,DirectorySeparator);
            (void) strcat(filename,local_info->font+1);
            error=TT_Open_Face(engine,filename,&face);
            if (!error || (path_end == (char *) NULL) || (*path_end == '\0'))
              break;
            path=path_end+1;
          }
       }
#if defined(TT_FONT_PATH)
      if (error)
        {
          /*
            Configured Truetype font path.
          */
          path=TT_FONT_PATH;
          for ( ; ; )
          {
            path_end=strchr(path,DirectoryListSeparator);
            if (path_end == (char *) NULL)
              (void) strcpy(filename,path);
            else
              {
                i=(int)(path_end-path);
                (void) strncpy(filename,path,i);
                filename[i]='\0';
              }
            i=strlen(filename);
            if ((i > 0) && (!IsBasenameSeparator(filename[i-1])))
              (void) strcat(filename,DirectorySeparator);
            (void) strcat(filename,local_info->font+1);
            error=TT_Open_Face(engine,filename,&face);
            if (!error || (path_end == (char *) NULL) || (*path_end == '\0'))
              break;
            path=path_end+1;
          }
        }
#endif
      if (error)
        error=TT_Open_Face(engine,local_info->font+1,&face);
      if (error)
        {
          /*
            Use default font.
          */
          MagickWarning(DelegateWarning,"Unable to open TTF font",
            local_info->font+1);
          DestroyImage(image);
          (void) CloneString(&local_info->font,DefaultXFont);
          image=ReadLABELImage(local_info);
          DestroyImageInfo(local_info);
          return(image);
        }
      TT_Get_Face_Properties(face,&face_properties);
      if (strcmp(text,Alphabet) == 0)
        GetFontInfo(face,&face_properties,image);
      error=TT_New_Instance(face,&instance);
      if ((image->x_resolution == 0.0) || (image->y_resolution == 0.0))
        {
          image->x_resolution=96.0;
          image->y_resolution=96.0;
        }
      error|=TT_Set_Instance_Resolutions(instance,(unsigned short)
        image->x_resolution,(unsigned short) image->y_resolution);
      error|=
        TT_Set_Instance_CharSize(instance,(int) (64.0*local_info->pointsize));
      if (error)
        ReaderExit(DelegateWarning,"Cannot initialize TTF instance",image);
      for (code=0; (int) code < (int) face_properties.num_CharMaps; code++)
      {
        TT_Get_CharMap_ID(face,code,&platform,&encoding);
        if (((platform == 3) && (encoding == 1)) ||
            ((platform == 0) && (encoding == 0)))
          {
            TT_Get_CharMap(face,code,&char_map);
            break;
          }
      }
      number_glyphs=0;
      character_map=True;
      if (code == face_properties.num_CharMaps)
        {
          TT_Get_Face_Properties(face,&face_properties);
          number_glyphs=face_properties.num_Glyphs;
          character_map=False;
        }
      glyphs=(TT_Glyph *) AllocateMemory(MaxGlyphs*sizeof(TT_Glyph));
      if (glyphs == (TT_Glyph *) NULL)
        ReaderExit(DelegateWarning,"Memory allocation failed",image);
      for (i=0; i < MaxGlyphs; i++)
        glyphs[i].z=(TT_Glyph *) NULL;
      unicode=ConvertTextToUnicode(text,&length);
      if (unicode == (unsigned short *) NULL)
        ReaderExit(DelegateWarning,"Memory allocation failed",image);
      for (i=0; i < length; i++)
      {
        if (glyphs[unicode[i]].z != (TT_Glyph *) NULL)
          continue;
        if (character_map)
          code=TT_Char_Index(char_map,unicode[i]);
        else
          {
            code=((int) unicode[i]-' '+1) < 0 ? 0 : ((int) unicode[i]-' '+1);
            if ((int) code >= number_glyphs)
              code=0;
          }
        error=TT_New_Glyph(face,&glyphs[unicode[i]]);
        error|=TT_Load_Glyph(instance,glyphs[unicode[i]],code,
          TTLOAD_SCALE_GLYPH | TTLOAD_HINT_GLYPH);
        if (error)
          ReaderExit(DelegateWarning,"Cannot initialize TTF glyph",image);
      }
      TT_Get_Face_Properties(face,&face_properties);
      TT_Get_Instance_Metrics(instance,&instance_metrics);
      canvas.width=4;
      for (i=0; i < length; i++)
      {
        if (glyphs[unicode[i]].z == (TT_Glyph *) NULL)
          continue;
        TT_Get_Glyph_Metrics(glyphs[unicode[i]],&glyph_metrics);
        if (i == (length-1))
          {
            canvas.width+=
              (glyph_metrics.bbox.xMin+glyph_metrics.bbox.xMax) >> 6;
            continue;
          }
        canvas.width+=(glyph_metrics.advance >> 6)+1;
      }
      canvas.width=(canvas.width+3) & -4;
      canvas.rows=instance_metrics.y_ppem*(face_properties.horizontal->Ascender-
        face_properties.horizontal->Descender)/
        face_properties.header->Units_Per_EM;
      canvas.flow=TT_Flow_Down;
      canvas.cols=canvas.width;
      canvas.size=canvas.rows*canvas.width;
      canvas.bitmap=(void *) AllocateMemory(canvas.size);
      if (!canvas.bitmap)
        ReaderExit(DelegateWarning,"Memory allocation failed",image);
      p=(unsigned char *) canvas.bitmap;
      for (i=0; i < canvas.size; i++)
        *p++=0;
      character.rows=canvas.rows;
      character.width=(instance_metrics.x_ppem+32+3) & -4;
      character.flow=TT_Flow_Down;
      character.cols=character.width;
      character.size=character.rows*character.width;
      character.bitmap=(void *) AllocateMemory(character.size);
      if (!character.bitmap)
        ReaderExit(DelegateWarning,"Memory allocation failed",image);
      x=0;
      y=(-instance_metrics.y_ppem*face_properties.horizontal->Descender)/
        face_properties.header->Units_Per_EM+1;
      for (i=0; i < length; i++)
      {
        if (glyphs[unicode[i]].z == (TT_Glyph *) NULL)
          continue;
        TT_Get_Glyph_Metrics(glyphs[unicode[i]],&glyph_metrics);
        RenderGlyph(&canvas,&character,glyphs[unicode[i]],x,y,&glyph_metrics);
        x+=glyph_metrics.advance/64;
      }
      /*
        Render label with a TrueType font.
      */
      image->matte=True;
      image->columns=canvas.width;
      image->rows=canvas.rows;
      crop_info.width=0;
      crop_info.height=image->rows;
      p=(unsigned char *) canvas.bitmap;
      for (y=0; y < (int) image->rows; y++)
      {
        q=SetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          q->red=pen_color.red;
          q->green=pen_color.green;
          q->blue=pen_color.blue;
          if (local_info->antialias)
            q->opacity=(int) (Opaque*Min(*p,4))/4;
          else
            q->opacity=(*p) > 1 ? Opaque : Transparent;
          if (q->opacity != Transparent)
            {
              if (x > (int) crop_info.width)
                crop_info.width=x;
            }
          else
            {
              q->red=(~q->red);
              q->green=(~q->green);
              q->blue=(~q->blue);
            }
          p++;
          q++;
        }
        if (!SyncPixelCache(image))
          break;
        if ((image->columns % 2) != 0)
          p++;
      }
      (void) sprintf(geometry,"%ux%u+0+0",crop_info.width+1,crop_info.height);
      TransformImage(&image,geometry,(char *) NULL);
      /*
        Free TrueType resources.
      */
      FreeMemory(canvas.bitmap);
      FreeMemory(character.bitmap);
      for (i=0; i < MaxGlyphs; i++)
        TT_Done_Glyph(glyphs[i]);
      FreeMemory(glyphs);
      FreeMemory(unicode);
      TT_Done_Instance(instance);
      TT_Close_Face(face);
      TT_Done_FreeType(engine);
      DestroyImageInfo(local_info);
      return(image);
#else
      MagickWarning(MissingDelegateWarning,"FreeType library is not available",
        (char *) NULL);
#endif
    }
  if (*local_info->font == '-')
    {
#if defined(HasX11)
      int
        status;

      static Display
        *display = (Display *) NULL;

      static ImageInfo
        cache_info;

      static XAnnotateInfo
        annotate_info;

      static XFontStruct
        *font_info;

      static XPixelInfo
        pixel;

      static XResourceInfo
        resource_info;

      static XrmDatabase
        resource_database;

      static XStandardColormap
        *map_info;

      static XVisualInfo
        *visual_info;

      /*
        Allocate image structure.
      */
      if (display == (Display *) NULL)
        {
          /*
            Open X server connection.
          */
          display=XOpenDisplay(local_info->server_name);
          if (display != (Display *) NULL)
            {
              char
                *client_name;

              /*
                Get user defaults from X resource database.
              */
              XSetErrorHandler(XError);
              client_name=SetClientName((char *) NULL);
              resource_database=XGetResourceDatabase(display,client_name);
              XGetResourceInfo(resource_database,client_name,&resource_info);
              resource_info.close_server=False;
              resource_info.colormap=PrivateColormap;
              resource_info.font=AllocateString(local_info->font);
              resource_info.background_color=AllocateString("black");
              resource_info.foreground_color=AllocateString("white");
              map_info=XAllocStandardColormap();
              if (map_info == (XStandardColormap *) NULL)
                ReaderExit(ResourceLimitWarning,"Memory allocation failed",
                  image);
              /*
                Initialize visual info.
              */
              visual_info=XBestVisualInfo(display,map_info,&resource_info);
              if (visual_info == (XVisualInfo *) NULL)
                ReaderExit(XServerWarning,"Unable to get visual",image);
              map_info->colormap=(Colormap) NULL;
              pixel.pixels=(unsigned long *) NULL;
              pixel.gamma_map=(XColor *) NULL;
              /*
                Initialize Standard Colormap info.
              */
              XGetMapInfo(visual_info,XDefaultColormap(display,
                visual_info->screen),map_info);
              XGetPixelPacket(display,visual_info,map_info,&resource_info,
                (Image *) NULL,&pixel);
              pixel.annotate_context=
                XDefaultGC(display,visual_info->screen);
              /*
                Initialize font info.
              */
              font_info=XBestFont(display,&resource_info,False);
              if (font_info == (XFontStruct *) NULL)
                ReaderExit(XServerWarning,"Unable to load font",image);
              if ((map_info == (XStandardColormap *) NULL) ||
                  (visual_info == (XVisualInfo *) NULL) ||
                  (font_info == (XFontStruct *) NULL))
                {
                  XFreeResources(display,visual_info,map_info,&pixel,
                    font_info,&resource_info,(XWindowInfo *) NULL);
                  display=(Display *) NULL;
                }
              cache_info=(*local_info);
            }
        }
      if (display == (Display *) NULL)
        {
          /*
            Use default font.
          */
          MagickWarning(XServerWarning,"Unable to open X server",
            local_info->server_name);
          DestroyImage(image);
          (void) CloneString(&local_info->font,"Helvetica");
          image=ReadLABELImage(local_info);
          DestroyImageInfo(local_info);
          return(image);
        }
      /*
        Initialize annotate info.
      */
      XGetAnnotateInfo(&annotate_info);
      annotate_info.stencil=OpaqueStencil;
      if (cache_info.font != local_info->font)
        {
          /*
            Font name has changed.
          */
          XFreeFont(display,font_info);
          (void) CloneString(&resource_info.font,local_info->font);
          font_info=XBestFont(display,&resource_info,False);
          if (font_info == (XFontStruct *) NULL)
            ReaderExit(ResourceLimitWarning,"Unable to load font",image);
        }
      annotate_info.font_info=font_info;
      annotate_info.text=text;
      annotate_info.width=XTextWidth(font_info,text,Extent(text));
      annotate_info.height=font_info->ascent+font_info->descent;
      (void) sprintf(annotate_info.geometry,"%ux%u+0+0",annotate_info.width,
        annotate_info.height);
      cache_info=(*local_info);
      /*
        Render label with a X11 server font.
      */
      image->matte=True;
      image->columns=annotate_info.width;
      image->rows=annotate_info.height;
      image->background_color=image->border_color;
      status=XAnnotateImage(display,&pixel,&annotate_info,image);
      if (status == 0)
        ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
      for (y=0; y < (int) image->rows; y++)
      {
        q=GetPixelCache(image,0,y,image->columns,1);
        if (q == (PixelPacket *) NULL)
          break;
        for (x=0; x < (int) image->columns; x++)
        {
          q->opacity=Intensity(*q);
          q->red=pen_color.red;
          q->green=pen_color.green;
          q->blue=pen_color.blue;
          if (q->opacity == Transparent)
            {
              q->red=(~q->red);
              q->green=(~q->green);
              q->blue=(~q->blue);
            }
          q++;
        }
        if (!SyncPixelCache(image))
          break;
      }
      DestroyImageInfo(local_info);
      return(image);
#else
      MagickWarning(MissingDelegateWarning,"X11 library is not available",
        (char *) NULL);
#endif
    }
  /*
    Render label with a Postscript font.
  */
  local_info->density=(char *) NULL;
  (void) sprintf(page,"%ux%u+0+0!",
    (unsigned int) ceil(local_info->pointsize*Extent(text)),
    (unsigned int) ceil(2*local_info->pointsize));
  (void) CloneString(&local_info->page,page);
  TemporaryFilename(filename);
  file=fopen(filename,WriteBinaryType);
  if (file == (FILE *) NULL)
    ReaderExit(FileOpenWarning,"Unable to open file",image);
  (void) fprintf(file,"%%!PS-Adobe-3.0\n");
  (void) fprintf(file,"/ReencodeFont\n");
  (void) fprintf(file,"{\n");
  (void) fprintf(file,"  findfont dup length\n");
  (void) fprintf(file,
    "  dict begin { 1 index /FID ne {def} {pop pop} ifelse } forall\n");
  (void) fprintf(file,
    "  /Encoding ISOLatin1Encoding def currentdict end definefont pop\n");
  (void) fprintf(file,"} bind def\n");
  (void) fprintf(file,
    "/%.1024s-ISO dup /%.1024s ReencodeFont findfont %f scalefont setfont\n",
    local_info->font,local_info->font,local_info->pointsize);
  (void) fprintf(file,"0.0 0.0 0.0 setrgbcolor\n");
  (void) fprintf(file,"0 0 %u %u rectfill\n",
    (unsigned int) ceil(local_info->pointsize*Extent(text)),
    (unsigned int) ceil(2*local_info->pointsize));
  (void) fprintf(file,"1.0 1.0 1.0 setrgbcolor\n");
  (void) fprintf(file,"0 %f moveto (%.1024s) show\n",local_info->pointsize,
    EscapeParenthesis(text));
  (void) fprintf(file,"showpage\n");
  (void) fclose(file);
  (void) strcpy(local_info->filename,filename);
  DestroyImage(image);
  image=ReadPSImage(local_info);
  (void) remove(filename);
  /*
    Set bounding box to the image dimensions.
  */
  crop_info.width=0;
  crop_info.height=ceil(local_info->pointsize);
  crop_info.x=0;
  crop_info.y=local_info->pointsize/4;
  DestroyImageInfo(local_info);
  if (image == (Image *) NULL)
    return(image);
  corner.red=0;
  corner.green=0;
  corner.blue=0;
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      if (!ColorMatch(*p,corner,0))
        if (x > (int) crop_info.width)
          crop_info.width=x;
      p++;
    }
  }
  (void) sprintf(geometry,"%ux%u%+d%+d",crop_info.width+1,crop_info.height,
    crop_info.x,crop_info.y);
  TransformImage(&image,geometry,(char *) NULL);
  image->matte=True;
  for (y=0; y < (int) image->rows; y++)
  {
    q=GetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; x++)
    {
      q->opacity=Intensity(*q);
      q->red=pen_color.red;
      q->green=pen_color.green;
      q->blue=pen_color.blue;
      if (q->opacity == Transparent)
        {
          q->red=(~q->red);
          q->green=(~q->green);
          q->blue=(~q->blue);
        }
      q++;
    }
    if (!SyncPixelCache(image))
      break;
  }
  return(image);
}
