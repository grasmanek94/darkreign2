/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                             GGGG  IIIII  FFFFF                              %
%                            G        I    F                                  %
%                            G  GG    I    FFF                                %
%                            G   G    I    F                                  %
%                             GGG   IIIII  F                                  %
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

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DecodeImage uncompresses an image via GIF-coding.
%
%  The format of the DecodeImage method is:
%
%      unsigned int DecodeImage(Image *image,const int opacity)
%
%  A description of each parameter follows:
%
%    o status:  Method DecodeImage returns True if all the pixels are
%      uncompressed without error, otherwise False.
%
%    o image: The address of a structure of type Image.
%
%    o opacity:  The colormap index associated with the transparent
%      color.
%
%
*/
static unsigned int DecodeImage(Image *image,const int opacity)
{
#define MaxStackSize  4096
#define NullCode  (-1)

  IndexPacket
    index;

  int
    available,
    bits,
    code,
    clear,
    code_mask,
    code_size,
    count,
    end_of_information,
    in_code,
    old_code,
    y;

  register int
    i,
    x;

  register PixelPacket
    *q;

  register unsigned char
    *c;

  register unsigned int
    datum;

  short
    *prefix;

  unsigned char
    data_size,
    first,
    *packet,
    *pixel_stack,
    *suffix,
    *top_stack;

  /*
    Allocate decoder tables.
  */
  assert(image != (Image *) NULL);
  packet=(unsigned char *) AllocateMemory(256*sizeof(unsigned char));
  prefix=(short *) AllocateMemory(MaxStackSize*sizeof(short));
  suffix=(unsigned char *) AllocateMemory(MaxStackSize*sizeof(unsigned char));
  pixel_stack=(unsigned char *)
    AllocateMemory((MaxStackSize+1)*sizeof(unsigned char));
  if ((packet == (unsigned char *) NULL) ||
      (prefix == (short *) NULL) ||
      (suffix == (unsigned char *) NULL) ||
      (pixel_stack == (unsigned char *) NULL))
    {
      MagickWarning(ResourceLimitWarning,"Memory allocation failed",
        image->filename);
      return(False);
    }
  /*
    Initialize GIF data stream decoder.
  */
  data_size=ReadByte(image);
  clear=1 << data_size;
  end_of_information=clear+1;
  available=clear+2;
  old_code=NullCode;
  code_size=data_size+1;
  code_mask=(1 << code_size)-1;
  for (code=0; code < clear; code++)
  {
    prefix[code]=0;
    suffix[code]=(unsigned char) code;
  }
  /*
    Decode GIF pixel stream.
  */
  datum=0;
  bits=0;
  c=0;
  count=0;
  first=0;
  top_stack=pixel_stack;
  for (y=0; y < (int) image->rows; y++)
  {
    q=SetPixelCache(image,0,y,image->columns,1);
    if (q == (PixelPacket *) NULL)
      break;
    for (x=0; x < (int) image->columns; )
    {
      if (top_stack == pixel_stack)
        {
          if (bits < code_size)
            {
              /*
                Load bytes until there is enough bits for a code.
              */
              if (count == 0)
                {
                  /*
                    Read a new data block.
                  */
                  count=ReadBlobBlock(image,(char *) packet);
                  if (count <= 0)
                    break;
                  c=packet;
                }
              datum+=(*c) << bits;
              bits+=8;
              c++;
              count--;
              continue;
            }
          /*
            Get the next code.
          */
          code=datum & code_mask;
          datum>>=code_size;
          bits-=code_size;
          /*
            Interpret the code
          */
          if ((code > available) || (code == end_of_information))
            break;
          if (code == clear)
            {
              /*
                Reset decoder.
              */
              code_size=data_size+1;
              code_mask=(1 << code_size)-1;
              available=clear+2;
              old_code=NullCode;
              continue;
            }
          if (old_code == NullCode)
            {
              *top_stack++=suffix[code];
              old_code=code;
              first=(unsigned char) code;
              continue;
            }
          in_code=code;
          if (code >= available)
            {
              *top_stack++=first;
              code=old_code;
            }
          while (code >= clear)
          {
            *top_stack++=suffix[code];
            code=prefix[code];
          }
          first=suffix[code];
          /*
            Add a new string to the string table,
          */
          if (available >= MaxStackSize)
            break;
          *top_stack++=first;
          prefix[available]=old_code;
          suffix[available]=first;
          available++;
          if (((available & code_mask) == 0) && (available < MaxStackSize))
            {
              code_size++;
              code_mask+=available;
            }
          old_code=in_code;
        }
      /*
        Pop a pixel off the pixel stack.
      */
      top_stack--;
      index=(*top_stack);
      image->indexes[x++]=index;
      *q=image->colormap[index];
      q->opacity=index == opacity ? Transparent : Opaque;
      q++;
    }
    if (!SyncPixelCache(image))
      break;
    if (x < image->columns)
      break;
    if (image->previous == (Image *) NULL)
      if (QuantumTick(y,image->rows))
        ProgressMonitor(LoadImageText,y,image->rows);
  }
  FreeMemory(pixel_stack);
  FreeMemory(suffix);
  FreeMemory(prefix);
  FreeMemory(packet);
  image->compression=LZWCompression;
  image->matte=opacity >= 0;
  if (y < image->rows)
    {
      MagickWarning(CorruptImageWarning,"Corrupt GIF image",image->filename);
      return(False);
    }
  if (image->interlace != NoInterlace)
    {
      Image
        *interlace_image;

      int
        pass,
        y;

      register PixelPacket
        *p,
        *q;

      static const int
        interlace_rate[4] = { 8, 8, 4, 2 },
        interlace_start[4] = { 0, 4, 2, 1 };

      /*
        Interlace image.
      */
      interlace_image=CloneImage(image,image->columns,image->rows,True);
      if (interlace_image == (Image *) NULL)
        return(False);
      i=0;
      for (pass=0; pass < 4; pass++)
      {
        y=interlace_start[pass];
        while (y < (int) image->rows)
        {
          p=GetPixelCache(interlace_image,0,i,interlace_image->columns,1);
          q=SetPixelCache(image,0,y,image->columns,1);
          if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
            break;
          (void) memcpy(image->indexes,interlace_image->indexes,
            image->columns*sizeof(IndexPacket));
          (void) memcpy(q,p,image->columns*sizeof(PixelPacket));
          if (!SyncPixelCache(image))
            break;
          i++;
          y+=interlace_rate[pass];
        }
      }
      DestroyImage(interlace_image);
    }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   E n c o d e I m a g e                                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method EncodeImage compresses an image via GIF-coding.
%
%  The format of the EncodeImage method is:
%
%      unsigned int EncodeImage(const ImageInfo *image_info,Image *image,
%        const unsigned int data_size)
%
%  A description of each parameter follows:
%
%    o status:  Method EncodeImage returns True if all the pixels are
%      compressed without error, otherwise False.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image: The address of a structure of type Image.
%
%    o data_size:  The number of bits in the compressed packet.
%
%
*/
static unsigned int EncodeImage(const ImageInfo *image_info,Image *image,
  const unsigned int data_size)
{
#define MaxCode(number_bits)  ((1 << (number_bits))-1)
#define MaxHashTable  5003
#define MaxGIFBits  12
#if defined(HasLZW)
#define MaxGIFTable  (1 << MaxGIFBits)
#else
#define MaxGIFTable  max_code
#endif
#define GIFOutputCode(code) \
{ \
  /*  \
    Emit a code. \
  */ \
  if (bits > 0) \
    datum|=((long) code << bits); \
  else \
    datum=(long) code; \
  bits+=number_bits; \
  while (bits >= 8) \
  { \
    /*  \
      Add a character to current packet. \
    */ \
    packet[byte_count++]=(unsigned char) (datum & 0xff); \
    if (byte_count >= 254) \
      { \
        (void) WriteByte(image,byte_count); \
        (void) WriteBlob(image,byte_count,(char *) packet); \
        byte_count=0; \
      } \
    datum>>=8; \
    bits-=8; \
  } \
  if (free_code > max_code)  \
    { \
      number_bits++; \
      if (number_bits == MaxGIFBits) \
        max_code=MaxGIFTable; \
      else \
        max_code=MaxCode(number_bits); \
    } \
}

  int
    bits,
    byte_count,
    displacement,
    k,
    next_pixel,
    number_bits,
    y;

  long
    datum;

  register int
    i,
    x;

  register PixelPacket
    *p;

  short
    clear_code,
    end_of_information_code,
    free_code,
    *hash_code,
    *hash_prefix,
    index,
    max_code,
    waiting_code;

  unsigned char
    *packet,
    *hash_suffix;

  /*
    Allocate encoder tables.
  */
  assert(image != (Image *) NULL);
  packet=(unsigned char *) AllocateMemory(256*sizeof(unsigned char));
  hash_code=(short *) AllocateMemory(MaxHashTable*sizeof(short));
  hash_prefix=(short *) AllocateMemory(MaxHashTable*sizeof(short));
  hash_suffix=(unsigned char *)
    AllocateMemory(MaxHashTable*sizeof(unsigned char));
  if ((packet == (unsigned char *) NULL) || (hash_code == (short *) NULL) ||
      (hash_prefix == (short *) NULL) ||
      (hash_suffix == (unsigned char *) NULL))
    return(False);
  /*
    Initialize GIF encoder.
  */
  number_bits=data_size;
  max_code=MaxCode(number_bits);
  clear_code=((short) 1 << (data_size-1));
  end_of_information_code=clear_code+1;
  free_code=clear_code+2;
  byte_count=0;
  datum=0;
  bits=0;
  for (i=0; i < MaxHashTable; i++)
    hash_code[i]=0;
  GIFOutputCode(clear_code);
  /*
    Encode pixels.
  */
  waiting_code=0;
  for (y=0; y < (int) image->rows; y++)
  {
    p=GetPixelCache(image,0,y,image->columns,1);
    if (p == (PixelPacket *) NULL)
      break;
    if (y == 0)
      waiting_code=(*image->indexes);
    for (x=(y == 0) ? 1 : 0; x < (int) image->columns; x++)
    {
      /*
        Probe hash table.
      */
      index=image->indexes[x] & 0xff;
      p++;
      k=(int) ((int) index << (MaxGIFBits-8))+waiting_code;
      if (k >= MaxHashTable)
        k-=MaxHashTable;
#if defined(HasLZW)
      if ((image_info->compression != NoCompression) && (hash_code[k] > 0))
        {
          if ((hash_prefix[k] == waiting_code) && (hash_suffix[k] == index))
            {
              waiting_code=hash_code[k];
              continue;
            }
          if (k == 0)
            displacement=1;
          else
            displacement=MaxHashTable-k;
          next_pixel=False;
          for ( ; ; )
          {
            k-=displacement;
            if (k < 0)
              k+=MaxHashTable;
            if (hash_code[k] == 0)
              break;
            if ((hash_prefix[k] == waiting_code) && (hash_suffix[k] == index))
              {
                waiting_code=hash_code[k];
                next_pixel=True;
                break;
              }
          }
          if (next_pixel == True)
            continue;
        }
#endif
      GIFOutputCode(waiting_code);
      if (free_code < MaxGIFTable)
        {
          hash_code[k]=free_code++;
          hash_prefix[k]=waiting_code;
          hash_suffix[k]=(unsigned char) index;
        }
      else
        {
          /*
            Fill the hash table with empty entries.
          */
          for (k=0; k < MaxHashTable; k++)
            hash_code[k]=0;
          /*
            Reset compressor and issue a clear code.
          */
          free_code=clear_code+2;
          GIFOutputCode(clear_code);
          number_bits=data_size;
          max_code=MaxCode(number_bits);
        }
      waiting_code=index;
    }
    if (image->previous == (Image *) NULL)
      if (QuantumTick(y,image->rows))
        ProgressMonitor(SaveImageText,y,image->rows);
  }
  /*
    Flush out the buffered code.
  */
  GIFOutputCode(waiting_code);
  GIFOutputCode(end_of_information_code);
  if (bits > 0)
    {
      /*
        Add a character to current packet.
      */
      packet[byte_count++]=(unsigned char) (datum & 0xff);
      if (byte_count >= 254)
        {
          (void) WriteByte(image,byte_count);
          (void) WriteBlob(image,byte_count,(char *) packet);
          byte_count=0;
        }
    }
  /*
    Flush accumulated data.
  */
  if (byte_count > 0)
    {
      (void) WriteByte(image,byte_count);
      (void) WriteBlob(image,byte_count,(char *) packet);
    }
  /*
    Free encoder memory.
  */
  FreeMemory(hash_suffix);
  FreeMemory(hash_prefix);
  FreeMemory(hash_code);
  FreeMemory(packet);
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s G I F                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method IsGIF returns True if the image format type, identified by the
%  magick string, is GIF.
%
%  The format of the ReadGIFImage method is:
%
%      unsigned int IsGIF(const unsigned char *magick,
%        const unsigned int length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsGIF returns True if the image format type is GIF.
%
%    o magick: This string is generally the first few bytes of an image file
%      or blob.
%
%    o length: Specifies the length of the magick string.
%
%
*/
Export unsigned int IsGIF(const unsigned char *magick,const unsigned int length)
{
  if (length < 4)
    return(False);
  if (strncmp((char *) magick,"GIF8",4) == 0)
    return(True);
  return(False);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d G I F I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadGIFImage reads a Compuserve Graphics image file and returns it.
%  It allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadGIFImage method is:
%
%      Image *ReadGIFImage(const ImageInfo *image_info)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadGIFImage returns a pointer to the image after
%      reading.  A null image is returned if there is a memory shortage or
%      an error occurs.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%
*/
Export Image *ReadGIFImage(const ImageInfo *image_info)
{
#define BitSet(byte,bit)  (((byte) & (bit)) == (bit))
#define LSBFirstOrder(x,y)  (((y) << 8) | (x))

  Image
    *image;

  int
    opacity,
    status;

  RectangleInfo
    page_info;

  register int
    i;

  register unsigned char
    *p;

  unsigned char
    background,
    c,
    flag,
    *global_colormap,
    header[MaxTextExtent],
    magick[12];

  unsigned int
    delay,
    dispose,
    global_colors,
    image_count,
    iterations;

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
    Determine if this is a GIF file.
  */
  status=ReadBlob(image,6,(char *) magick);
  if ((status == False) || ((strncmp((char *) magick,"GIF87",5) != 0) &&
      (strncmp((char *) magick,"GIF89",5) != 0)))
    ReaderExit(CorruptImageWarning,"Not a GIF image file",image);
  global_colors=0;
  global_colormap=(unsigned char *) NULL;
  page_info.width=LSBFirstReadShort(image);
  page_info.height=LSBFirstReadShort(image);
  flag=ReadByte(image);
  background=ReadByte(image);
  c=ReadByte(image);  /* reserved */
  if (BitSet(flag,0x80))
    {
      /*
        opacity global colormap.
      */
      global_colors=1 << ((flag & 0x07)+1);
      global_colormap=(unsigned char *)
        AllocateMemory(3*global_colors*sizeof(unsigned char));
      if (global_colormap == (unsigned char *) NULL)
        ReaderExit(ResourceLimitWarning,"Unable to read image colormap file",
          image);
      (void) ReadBlob(image,3*global_colors,(char *) global_colormap);
    }
  delay=0;
  dispose=0;
  iterations=1;
  opacity=(-1);
  image_count=0;
  for ( ; ; )
  {
    status=ReadBlob(image,1,(char *) &c);
    if (status == False)
      break;
    if (c == ';')
      break;  /* terminator */
    if (c == '!')
      {
        /*
          GIF Extension block.
        */
        status=ReadBlob(image,1,(char *) &c);
        if (status == False)
          ReaderExit(CorruptImageWarning,"Unable to read extension block",
            image);
        switch (c)
        {
          case 0xf9:
          {
            /*
              Read Graphics Control extension.
            */
            while (ReadBlobBlock(image,(char *) header) > 0);
            dispose=header[0] >> 2;
            delay=(header[2] << 8) | header[1];
            if ((header[0] & 0x01) == 1)
              opacity=header[3];
            break;
          }
          case 0xfe:
          {
            int
              length;

            /*
              Read Comment extension.
            */
            for ( ; ; )
            {
              length=ReadBlobBlock(image,(char *) header);
              if (length <= 0)
                break;
              if (image->comments != (char *) NULL)
                {
                  image->comments=(char *) ReallocateMemory((char *)
                    image->comments,(Extent(image->comments)+length+1)*
                    sizeof(char));
                }
              else
                {
                  image->comments=(char *)
                    AllocateMemory((length+1)*sizeof(char));
                  if (image->comments != (char *) NULL)
                    *image->comments='\0';
                }
              if (image->comments == (char *) NULL)
                ReaderExit(ResourceLimitWarning,"Memory allocation failed",
                  image);
              header[length]='\0';
              (void) strcat(image->comments,(char *) header);
            }
            break;
          }
          case 0xff:
          {
            /*
              Read Netscape Loop extension.
            */
            int
              found_netscape_loop=False;

            if (ReadBlobBlock(image,(char *) header) > 0)
              found_netscape_loop=!strncmp((char *) header,"NETSCAPE2.0",11);
            while (ReadBlobBlock(image,(char *) header) > 0)
            if (found_netscape_loop)
              {
                /*
                  Look for terminator.
                */
                iterations=(header[2] << 8) | header[1];
              }
            break;
          }
          default:
          {
            while (ReadBlobBlock(image,(char *) header) > 0);
            break;
          }
        }
      }
    if (c != ',')
      continue;
    if (image_count != 0)
      {
        /*
          Allocate next image structure.
        */
        AllocateNextImage(image_info,image);
        if (image->next == (Image *) NULL)
          {
            DestroyImages(image);
            return((Image *) NULL);
          }
        image=image->next;
        ProgressMonitor(LoadImagesText,TellBlob(image),image->filesize);
      }
    image_count++;
    /*
      Read image attributes.
    */
    image->class=PseudoClass;
    page_info.x=LSBFirstReadShort(image);
    page_info.y=LSBFirstReadShort(image);
    image->columns=LSBFirstReadShort(image);
    image->rows=LSBFirstReadShort(image);
    image->depth=8;
    flag=ReadByte(image);
    image->interlace=BitSet(flag,0x40) ? PlaneInterlace : NoInterlace;
    image->colors=!BitSet(flag,0x80) ? global_colors : 1 << ((flag & 0x07)+1);
    image->page_info.width=page_info.width;
    image->page_info.height=page_info.height;
    image->page_info.y=page_info.y;
    image->page_info.x=page_info.x;
    if (image_info->delay == (char *) NULL)
      image->delay=delay;
    if (image_info->dispose == (char *) NULL)
      image->dispose=dispose;
    if (image_info->iterations == (char *) NULL)
      image->iterations=iterations;
    delay=0;
    dispose=0;
    iterations=1;
    if (image_info->ping)
      {
        if (opacity >= 0)
          image->matte=True;
        CloseBlob(image);
        return(image);
      }
    if ((image->columns == 0) || (image->rows == 0))
      ReaderExit(CorruptImageWarning,"image size is 0",image);
    /*
      Inititialize colormap.
    */
    if (opacity >= (int) image->colors)
      image->colormap=(PixelPacket *)
        AllocateMemory((opacity+1)*sizeof(PixelPacket));
    else
      image->colormap=(PixelPacket *)
        AllocateMemory(image->colors*sizeof(PixelPacket));
    if (image->colormap == (PixelPacket *) NULL)
      ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
    if (!BitSet(flag,0x80))
      {
        /*
          Use global colormap.
        */
        p=global_colormap;
        for (i=0; i < (int) image->colors; i++)
        {
          image->colormap[i].red=UpScale(*p++);
          image->colormap[i].green=UpScale(*p++);
          image->colormap[i].blue=UpScale(*p++);
        }
        image->background_color=
          image->colormap[Min(background,image->colors-1)];
      }
    else
      {
        unsigned char
          *colormap;

        /*
          Read local colormap.
        */
        colormap=(unsigned char *)
          AllocateMemory(3*image->colors*sizeof(unsigned char));
        if (colormap == (unsigned char *) NULL)
          ReaderExit(ResourceLimitWarning,"Memory allocation failed",image);
        (void) ReadBlob(image,3*image->colors,(char *) colormap);
        p=colormap;
        for (i=0; i < (int) image->colors; i++)
        {
          image->colormap[i].red=UpScale(*p++);
          image->colormap[i].green=UpScale(*p++);
          image->colormap[i].blue=UpScale(*p++);
        }
        FreeMemory(colormap);
      }
    if (opacity >= (int) image->colors)
      {
        for (i=image->colors; i < (opacity+1); i++)
        {
          image->colormap[i].red=0;
          image->colormap[i].green=0;
          image->colormap[i].blue=0;
        }
        image->colors=opacity+1;
      }
    /*
      Decode image.
    */
    status=DecodeImage(image,opacity);
    if (status == False)
      {
        MagickWarning(CorruptImageWarning,"Corrupt GIF image",image->filename);
        break;
      }
    if (image_info->subrange != 0)
      if (image->scene >= (image_info->subimage+image_info->subrange-1))
        break;
  }
  if (global_colormap != (unsigned char *) NULL)
    FreeMemory(global_colormap);
  while (image->previous != (Image *) NULL)
    image=image->previous;
  CloseBlob(image);
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e G I F I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteGIFImage writes an image to a file in the Compuserve Graphics
%  image format.
%
%  The format of the WriteGIFImage method is:
%
%      unsigned int WriteGIFImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Method WriteGIFImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to an ImageInfo structure.
%
%    o image:  A pointer to a Image structure.
%
%
*/
Export unsigned int WriteGIFImage(const ImageInfo *image_info,Image *image)
{
  Image
    *next_image;

  int
    j,
    y;

  QuantizeInfo
    quantize_info;

  RectangleInfo
    page_info;

  register int
    i,
    x;

  register PixelPacket
    *p;

  register unsigned char
    *q;

  unsigned char
    bits_per_pixel,
    c,
    *colormap,
    *global_colormap;

  unsigned int
    interlace,
    opacity,
    scene,
    status;

  /*
    Open output image file.
  */
  status=OpenBlob(image_info,image,WriteBinaryType);
  if (status == False)
    WriterExit(FileOpenWarning,"Unable to open file",image);
  /*
    Determine image bounding box.
  */
  page_info.width=image->columns;
  page_info.height=image->rows;
  page_info.x=0;
  page_info.y=0;
  for (next_image=image; next_image != (Image *) NULL; )
  {
    page_info.x=next_image->page_info.x;
    page_info.y=next_image->page_info.y;
    if ((next_image->columns+page_info.x) > page_info.width)
      page_info.width=next_image->columns+page_info.x;
    if ((next_image->rows+page_info.y) > page_info.height)
      page_info.height=next_image->rows+page_info.y;
    next_image=next_image->next;
  }
  /*
    Allocate colormap.
  */
  global_colormap=(unsigned char *) AllocateMemory(768*sizeof(unsigned char));
  colormap=(unsigned char *) AllocateMemory(768*sizeof(unsigned char));
  if ((global_colormap == (unsigned char *) NULL) ||
      (colormap == (unsigned char *) NULL))
    WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
  for (i=0; i < 768; i++)
    colormap[i]=0;
  /*
    Write GIF header.
  */
  if ((image->comments == (char *) NULL) && !image_info->adjoin &&
      !image->matte)
    (void) WriteBlob(image,6,"GIF87a");
  else
    if (Latin1Compare(image_info->magick,"GIF87") == 0)
      (void) WriteBlob(image,6,"GIF87a");
    else
      (void) WriteBlob(image,6,"GIF89a");
  if ((image->page_info.width != 0) && (image->page_info.height != 0))
    page_info=image->page_info;
  LSBFirstWriteShort(image,page_info.width);
  LSBFirstWriteShort(image,page_info.height);
  page_info.x=0;
  page_info.y=0;
  /*
    Write images to file.
  */
  interlace=image_info->interlace;
  if (image_info->adjoin && (image->next != (Image *) NULL))
    interlace=NoInterlace;
  opacity=0;
  scene=0;
  do
  {
    TransformRGBImage(image,RGBColorspace);
    if (!IsPseudoClass(image))
      {
        /*
          GIF requires that the image is colormapped.
        */
        GetQuantizeInfo(&quantize_info);
        quantize_info.dither=image_info->dither;
        quantize_info.number_colors=image->matte ? 255 : 256;
        (void) QuantizeImage(&quantize_info,image);
        if (image->matte)
          {
            /*
              Set transparent pixel.
            */
            opacity=image->colors++;
            image->colormap=(PixelPacket *) ReallocateMemory(image->colormap,
              image->colors*sizeof(PixelPacket));
            if (image->colormap == (PixelPacket *) NULL)
              {
                FreeMemory(global_colormap);
                FreeMemory(colormap);
                WriterExit(ResourceLimitWarning,"Memory allocation failed",
                  image);
              }
            image->colormap[opacity]=image->background_color;
            for (y=0; y < (int) image->rows; y++)
            {
              p=GetPixelCache(image,0,y,image->columns,1);
              if (p == (PixelPacket *) NULL)
                break;
              for (x=0; x < (int) image->columns; x++)
              {
                if (p->opacity == Transparent)
                  image->indexes[x]=opacity;
                p++;
              }
              if (!SyncPixelCache(image))
                break;
            }
          }
      }
    else
      if (image->matte)
        {
          /*
            Identify transparent pixel index.
          */
          for (y=0; y < (int) image->rows; y++)
          {
            p=GetPixelCache(image,0,y,image->columns,1);
            if (p == (PixelPacket *) NULL)
              break;
            for (x=0; x < (int) image->columns; x++)
            {
              if (p->opacity == Transparent)
                {
                  opacity=image->indexes[x];
                  break;
                }
              p++;
            }
            if (x < (int) image->columns)
              break;
          }
        }
    for (bits_per_pixel=1; bits_per_pixel < 8; bits_per_pixel++)
      if ((1 << bits_per_pixel) >= (int) image->colors)
        break;
    q=colormap;
    for (i=0; i < (int) image->colors; i++)
    {
      *q++=DownScale(image->colormap[i].red);
      *q++=DownScale(image->colormap[i].green);
      *q++=DownScale(image->colormap[i].blue);
    }
    for ( ; i < (int) (1 << bits_per_pixel); i++)
    {
      *q++=(Quantum) 0x0;
      *q++=(Quantum) 0x0;
      *q++=(Quantum) 0x0;
    }
    if ((image->previous == (Image *) NULL) || !image_info->adjoin)
      {
        /*
          Write global colormap.
        */
        c=0x80;
        c|=(8-1) << 4;  /* color resolution */
        c|=(bits_per_pixel-1);   /* size of global colormap */
        (void) WriteByte(image,(char) c);
        for (j=0; j < (int) (image->colors-1); j++)
          if (ColorMatch(image->background_color,image->colormap[j],0))
            break;
        (void) WriteByte(image,j);  /* background color */
        (void) WriteByte(image,0x0);  /* reserved */
        (void) WriteBlob(image,3*(1 << bits_per_pixel),(char *) colormap);
        for (j=0; j < 768; j++)
          global_colormap[j]=colormap[j];
      }
    if (Latin1Compare(image_info->magick,"GIF87") != 0)
      {
        /*
          Write Graphics Control extension.
        */
        (void) WriteByte(image,0x21);
        (void) WriteByte(image,0xf9);
        (void) WriteByte(image,0x04);
        c=image->dispose << 2;
        if (image->matte)
          c|=0x01;
        (void) WriteByte(image,c);
        LSBFirstWriteShort(image,image->delay);
        (void) WriteByte(image,(char) opacity);
        (void) WriteByte(image,0x00);
        if (image->comments != (char *) NULL)
          {
            register char
              *p;

            register unsigned int
              count;

            /*
              Write Comment extension.
            */
            (void) WriteByte(image,0x21);
            (void) WriteByte(image,0xfe);
            p=image->comments;
            while (Extent(p) > 0)
            {
              count=Min(Extent(p),255);
              (void) WriteByte(image,count);
              for (i=0; i < (int) count; i++)
                (void) WriteByte(image,*p++);
            }
            (void) WriteByte(image,0x0);
          }
        if ((image->previous == (Image *) NULL) &&
            (image->next != (Image *) NULL) && (image->iterations != 1))
          {
            /*
              Write Netscape Loop extension.
            */
            (void) WriteByte(image,0x21);
            (void) WriteByte(image,0xff);
            (void) WriteByte(image,0x0b);
            (void) WriteBlob(image,11,"NETSCAPE2.0");
            (void) WriteByte(image,0x03);
            (void) WriteByte(image,0x01);
            LSBFirstWriteShort(image,image->iterations);
            (void) WriteByte(image,0x00);
          }
      }
    (void) WriteByte(image,',');  /* image separator */
    /*
      Write the image header.
    */
    if ((image->page_info.width != 0) && (image->page_info.height != 0))
      {
        page_info=image->page_info;
        if ((image->previous == (Image *) NULL) &&
            (image->next == (Image *) NULL))
          {
            page_info.x=0;
            page_info.y=0;
          }
      }
    LSBFirstWriteShort(image,page_info.x);
    LSBFirstWriteShort(image,page_info.y);
    LSBFirstWriteShort(image,image->columns);
    LSBFirstWriteShort(image,image->rows);
    c=0x00;
    if (interlace != NoInterlace)
      c|=0x40;  /* pixel data is interlaced */
    for (j=0; j < (int) (3*image->colors); j++)
      if (colormap[j] != global_colormap[j])
        break;
    if (j == (int) (3*image->colors))
      (void) WriteByte(image,(char) c);
    else
      {
        c|=0x80;
        c|=(bits_per_pixel-1);   /* size of local colormap */
        (void) WriteByte(image,(char) c);
        (void) WriteBlob(image,3*(1 << bits_per_pixel),(char *) colormap);
      }
    /*
      Write the image data.
    */
    c=Max(bits_per_pixel,2);
    (void) WriteByte(image,(char) c);
    if (interlace == NoInterlace)
      status=EncodeImage(image_info,image,Max(bits_per_pixel,2)+1);
    else
      {
        Image
          *interlace_image;

        int
          pass;

        register PixelPacket
          *q;

        static const int
          interlace_rate[4] = { 8, 8, 4, 2 },
          interlace_start[4] = { 0, 4, 2, 1 };

        /*
          Interlace image.
        */
        interlace_image=CloneImage(image,image->columns,image->rows,True);
        if (interlace_image == (Image *) NULL)
          {
            FreeMemory(global_colormap);
            FreeMemory(colormap);
            WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
          }
        i=0;
        for (pass=0; pass < 4; pass++)
        {
          y=interlace_start[pass];
          while (y < (int) image->rows)
          {
            p=GetPixelCache(image,0,y,image->columns,1);
            q=SetPixelCache(interlace_image,0,i,interlace_image->columns,1);
            if ((p == (PixelPacket *) NULL) || (q == (PixelPacket *) NULL))
              break;
            (void) memcpy(interlace_image->indexes,image->indexes,
              image->columns*sizeof(IndexPacket));
            (void) memcpy(q,p,image->columns*sizeof(PixelPacket));
            if (!SyncPixelCache(interlace_image))
              break;
            i++;
            y+=interlace_rate[pass];
          }
        }
        interlace_image->file=image->file;
        status=EncodeImage(image_info,interlace_image,Max(bits_per_pixel,2)+1);
        interlace_image->file=(FILE *) NULL;
        DestroyImage(interlace_image);
      }
    if (status == False)
      {
        FreeMemory(global_colormap);
        FreeMemory(colormap);
        WriterExit(ResourceLimitWarning,"Memory allocation failed",image);
      }
    (void) WriteByte(image,0x0);
    if (image->next == (Image *) NULL)
      break;
    image=GetNextImage(image);
    ProgressMonitor(SaveImagesText,scene++,GetNumberScenes(image));
  } while (image_info->adjoin);
  (void) WriteByte(image,';'); /* terminator */
  FreeMemory(global_colormap);
  FreeMemory(colormap);
  if (image_info->adjoin)
    while (image->previous != (Image *) NULL)
      image=image->previous;
  CloseBlob(image);
  return(True);
}
