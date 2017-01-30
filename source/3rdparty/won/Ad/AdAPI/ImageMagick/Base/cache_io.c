/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                      CCCC   AAA    CCCC  H   H  EEEEE                       %
%                     C      A   A  C      H   H  E                           %
%                     C      AAAAA  C      HHHHH  EEE                         %
%                     C      A   A  C      H   H  E                           %
%                      CCCC  A   A   CCCC  H   H  EEEEE                       %
%                                                                             %
%                            IIIII     /  OOO                                 %
%                              I      /  O   O                                %
%                              I     /   O   O                                %
%                              I    /    O   O                                %
%                            IIIII /      OOO                                 %
%                                                                             %
%                                                                             %
%                    ImageMagick Pixel Cache I/O Methods                      %
%                                                                             %
%                                                                             %
%                              Software Design                                %
%                             William Radcliffe                               %
%                                John Cristy                                  %
%                               November 1999                                 %
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
typedef struct _CacheInfo
{
  ClassType
#if defined(__cplusplus) || defined(c_plusplus)
    c_class;
#else
    class;
#endif

  CacheType
    type;

  off_t
    number_pixels;

  unsigned int
    columns,
    rows;

  PixelPacket
    *pixels;

  IndexPacket
    *indexes;

  char
    filename[MaxTextExtent];

  int
    file;

  off_t
    length;

  void
    *stash;
} CacheInfo;

/*
  Global declarations.
*/
static off_t
  cache_threshold = PixelCacheThreshold;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   A l l o c a t e C a c h e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method AllocateCache initializes the pixel cache.  This includes defining
%  the cache dimensions, allocating space for the image pixels and optionally
%  the colormap indexes, and memory mapping the cache if it is disk based.
%
%  The format of the AllocateCache method is:
%
%      unsigned int AllocateCache(Cache cache,const ClassType class_type,
%        const unsigned int columns,const unsigned int rows)
%
%  A description of each parameter follows:
%
%    o status: Method AllocateCache returns True if the pixel cache is
%      initialized successfully otherwise False.
%
%    o cache: Specifies a pointer to a Cache structure.
%
%    o class_type: DirectClass or PseudoClass.
%
%    o columns: This unsigned integer defines the number of columns in the
%      pixel cache.
%
%    o rows: This unsigned integer defines the number of rows in the pixel
%      cache.
%
%
*/
Export unsigned int AllocateCache(Cache cache,const ClassType class_type,
  const unsigned int columns,const unsigned int rows)
{
  CacheInfo
    *cache_info;

  char
    null = 0;

  off_t
    length;

  void
    *allocation;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  if (class_type == cache_info->class)
    return(True);
  length=cache_info->number_pixels*sizeof(PixelPacket);
  if (cache_info->class == PseudoClass)
    length+=cache_info->number_pixels*sizeof(IndexPacket);
  if (cache_info->class != UndefinedClass)
    {
      /*
        Free memory-based cache resources.
      */
      if (cache_info->type == MemoryCache)
        (void) GetCacheMemory(length);
      if (cache_info->type == MemoryMappedCache)
        (void) UnmapBlob(cache_info->pixels,length);
    }
  cache_info->rows=rows;
  cache_info->columns=columns;
  cache_info->number_pixels=columns*rows;
  length=cache_info->number_pixels*sizeof(PixelPacket);
  if (class_type == PseudoClass)
    length+=cache_info->number_pixels*sizeof(IndexPacket);
  if ((cache_info->type == MemoryCache) ||
      ((cache_info->type == UndefinedCache) && (length <= GetCacheMemory(0))))
    {
      if (cache_info->class == UndefinedClass)
        allocation=AllocateMemory(length);
      else
        {
          allocation=ReallocateMemory(cache_info->pixels,length);
          if (allocation == (void *) NULL)
            return(False);
        }
      if (allocation != (void *) NULL)
        {
          /*
            Create in-memory pixel cache.
          */
          (void) GetCacheMemory(-length);
          cache_info->class=class_type;
          cache_info->type=MemoryCache;
          cache_info->pixels=(PixelPacket *) allocation;
          if (cache_info->class == PseudoClass)
            cache_info->indexes=(IndexPacket *)
              (cache_info->pixels+cache_info->number_pixels);
          return(True);
        }
    }
  /*
    Create pixel cache on disk.
  */
  if (cache_info->class == UndefinedClass)
    TemporaryFilename(cache_info->filename);
  if (cache_info->file == -1)
    {
      cache_info->file=
        open(cache_info->filename,O_RDWR | O_CREAT | O_BINARY,0777);
      if (cache_info->file == -1)
        return(False);
    }
  if (lseek(cache_info->file,length,SEEK_SET) == -1)
    return(False);
  if (write(cache_info->file,&null,sizeof(null)) == -1)
    return(False);
#if !defined(vms) && !defined(macintosh) && !defined(WIN32)
  (void) ftruncate(cache_info->file,length);
#endif
  cache_info->class=class_type;
  if (cache_info->type != DiskCache)
    {
      size_t
        offset;

      cache_info->type=DiskCache;
      allocation=MapBlob(cache_info->file,IOMode,&offset);
      if (allocation != (void *) NULL)
        {
          /*
            Create memory-mapped pixel cache.
          */
          cache_info->type=MemoryMappedCache;
          cache_info->pixels=(PixelPacket *) allocation;
          if (cache_info->class == PseudoClass)
            cache_info->indexes=(IndexPacket *)
              (cache_info->pixels+cache_info->number_pixels);
        }
    }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C l o s e C a c h e                                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method CloseCache closes the file handle associated with a disk pixel cache.
%
%  The format of the CloseCache method is:
%
%      void CloseCache(Cache cache)
%
%  A description of each parameter follows:
%
%    o cache: Specifies a pointer to a Cache structure.
%
%
*/
Export void CloseCache(Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  if (cache_info->file != -1)
    (void) close(cache_info->file);
  cache_info->file=(-1);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   D e s t r o y C a c h e I n f o                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method DestroyCacheInfo deallocates memory associated with the pixel cache.
%
%  The format of the DestroyCacheInfo method is:
%
%      void DestroyCacheInfo(Cache cache)
%
%  A description of each parameter follows:
%
%    o cache: Specifies a pointer to a Cache structure.
%
%
*/
Export void DestroyCacheInfo(Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  if (cache_info->stash != (void *) NULL)
    FreeMemory(cache_info->stash);
  switch (cache_info->type)
  {
    case MemoryCache:
    {
      FreeMemory(cache_info->pixels);
      if (cache_info->class == PseudoClass)
        (void) GetCacheMemory(cache_info->number_pixels*sizeof(IndexPacket));
      (void) GetCacheMemory(cache_info->number_pixels*sizeof(PixelPacket));
      break;
    }
    case MemoryMappedCache:
    {
      size_t
        length;

      /*
        Unmap memory-mapped pixels and indexes.
      */
      length=cache_info->number_pixels*sizeof(PixelPacket);
      if (cache_info->class == PseudoClass)
        length+=cache_info->number_pixels*sizeof(IndexPacket);
      (void) UnmapBlob(cache_info->pixels,length);
    }
    case DiskCache:
    {
      CloseCache(cache);
      (void) remove(cache_info->filename);
      break;
    }
    default:
      break;
  }
  FreeMemory(cache_info);
  cache=(void *) NULL;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e C l a s s T y p e                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetCacheClassType returns the class type of the pixel cache.
%
%  The format of the GetCacheClassType method is:
%
%      ClassType GetCacheClassType(Cache cache)
%
%  A description of each parameter follows:
%
%    o type: Method GetCacheClassType returns DirectClass or PseudoClass.
%
%    o cache: Specifies a pointer to a Cache structure.
%
%
*/
Export ClassType GetCacheClassType(Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  return(cache_info->class);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e I n d e x e s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetCacheIndexes returns the address of the cache colormap index
%  buffer.
%
%  The format of the GetCacheIndexes method is:
%
%      void *GetCacheIndexes(Cache cache,const unsigned int x,
%        const unsigned int y)
%
%  A description of each parameter follows:
%
%    o cache: Specifies a pointer to a Cache structure.
%
%    o x,y: This unsigned integer defines the offset into the pixel buffer.
%
%
*/
Export IndexPacket *GetCacheIndexes(Cache cache,const unsigned int x,
  const unsigned int y)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  if ((cache_info->class != PseudoClass) || (cache_info->type == DiskCache))
    return((IndexPacket *) NULL);
  return(cache_info->indexes+(y*cache_info->columns+x));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e I n f o                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetCacheInfo initializes the Cache structure.
%
%  The format of the GetCacheInfo method is:
%
%      void GetCacheInfo(Cache *cache)
%
%  A description of each parameter follows:
%
%    o cache: Specifies a pointer to a Cache structure.
%
%
*/
Export void GetCacheInfo(Cache *cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache *) NULL);
  cache_info=(CacheInfo *) AllocateMemory(sizeof(CacheInfo));
  cache_info->class=UndefinedClass;
  cache_info->type=UndefinedCache;
  cache_info->number_pixels=0;
  cache_info->rows=0;
  cache_info->columns=0;
  cache_info->pixels=(PixelPacket *) NULL;
  cache_info->indexes=(IndexPacket *) NULL;
  *cache_info->filename='\0';
  cache_info->file=(-1);
  cache_info->length=0;
  cache_info->stash=(void *) NULL;
  *cache=cache_info;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e M e m o r y                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetCacheMemory adjusts the amount of free cache memory and then
%  returns the resulting value.
%
%  The format of the GetCacheMemory method is:
%
%      off_t GetCacheMemory(const off_t memory)
%
%  A description of each parameter follows:
%
%    o memory: Specifies the adjustment to the cache memory.  Use 0 to
%      return the current free memory in the cache.
%
%
*/
Export off_t GetCacheMemory(const off_t memory)
{
  static off_t
    free_memory = PixelCacheThreshold*1024*1024;

#if defined(HasPTHREADS)
  {
    static pthread_mutex_t
      memory_mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&memory_mutex);
    free_memory+=memory;
    pthread_mutex_unlock(&memory_mutex);
  }
#else
  free_memory+=memory;
#endif
  return(free_memory);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e P i x e l s                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetCachePixels returns the address of the cache pixel buffer.
%
%  The format of the GetCachePixels method is:
%
%      void *GetCachePixels(Cache cache,const unsigned int x,
%        const unsigned int y)
%
%  A description of each parameter follows:
%
%    o cache: Specifies a pointer to a Cache structure.
%
%    o x,y: This unsigned integer defines the offset into the pixel buffer.
%
%
*/
Export PixelPacket *GetCachePixels(Cache cache,const unsigned int x,
  const unsigned int y)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  if (cache_info->type == DiskCache)
    return((PixelPacket *) NULL);
  return(cache_info->pixels+(y*cache_info->columns+x));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e S t a s h                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetCacheStash allocates memory for the cache pixel buffer.
%
%  The format of the GetCacheStash method is:
%
%      void GetCacheStash(Cache *cache,unsigned int number_pixels)
%
%  A description of each parameter follows:
%
%    o cache: Specifies a pointer to a Cache structure.
%
%    o number_pixels: This unsigned integer defines how many pixels are
%      needed in the buffer.
%
%
*/
Export void *GetCacheStash(Cache cache,unsigned int number_pixels)
{
  CacheInfo
    *cache_info;

  off_t
    length;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  length=number_pixels*sizeof(PixelPacket);
  if (cache_info->class == PseudoClass)
    length+=number_pixels*sizeof(IndexPacket);
  if (cache_info->stash == (PixelPacket *) NULL)
    cache_info->stash=AllocateMemory(length);
  else
    if (cache_info->length < length)
      cache_info->stash=ReallocateMemory(cache_info->stash,length);
  cache_info->length=length;
  return(cache_info->stash);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e T h e s h o l d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetCacheThreshold gets the amount of free memory allocated for the
%  pixel cache.  Once this threshold is exceeded, all subsequent pixels cache
%  operations are to/from disk.
%
%  The format of the GetCacheThreshold method is:
%
%      off_t GetCacheThreshold()
%
%  A description of each parameter follows:
%
%    o threshold: The number of megabytes of memory available to the pixel
%      cache.
%
%
*/
Export off_t GetCacheThreshold()
{
  return(cache_threshold);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   G e t C a c h e T y p e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method GetCacheType returns the class type of the pixel cache.
%
%  The format of the GetCacheType method is:
%
%      Type GetCacheType(Cache cache)
%
%  A description of each parameter follows:
%
%    o type: Method GetCacheType returns Direct or Pseudo.
%
%    o cache: Specifies a pointer to a Cache structure.
%
%
*/
Export CacheType GetCacheType(Cache cache)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  return(cache_info->type);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C a c h e I n d e x e s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadCacheIndexes reads colormap indexes from the specified region
%  of the pixel cache.
%
%  The format of the ReadCacheIndexes method is:
%
%      unsigned int ReadCacheIndexes(Cache cache,
%        const RectangleInfo *region_info,IndexPacket *indexes)
%
%  A description of each parameter follows:
%
%    o status:  Method ReadCacheIndexes returns True if the colormap indexes
%      are successfully read from the pixel cache, otherwise False.
%
%    o cache_info: Specifies a pointer to a CacheInfo structure.
%
%    o region_info:  The address of a RectangleInfo structure that defines
%      the cache region to read.
%
%    o indexes: The colormap indexes are copied from this IndexPacket address
%      to the pixel cache.
%
%
*/
Export unsigned int ReadCacheIndexes(Cache cache,
  const RectangleInfo *region_info,IndexPacket *indexes)
{
  CacheInfo
    *cache_info;

  off_t
    count,
    offset;

  register int
    y;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  if (cache_info->class != PseudoClass)
    return(False);
  offset=region_info->y*cache_info->columns+region_info->x;
  if (cache_info->type != DiskCache)
    {
      /*
        Read pixels from memory.
      */
      if (indexes == (cache_info->indexes+offset))
        return(True);
      for (y=0; y < (int) region_info->height; y++)
      {
        (void) memcpy(indexes,cache_info->indexes+offset,
          region_info->width*sizeof(IndexPacket));
        indexes+=region_info->width;
        offset+=cache_info->columns;
      }
      return(True);
    }
  /*
    Read pixels from disk.
  */
  if (cache_info->file == -1)
    {
      cache_info->file=open(cache_info->filename,O_RDWR | O_BINARY,0777);
      if (cache_info->file == -1)
        return(False);
    }
  for (y=0; y < (int) region_info->height; y++)
  {
    count=lseek(cache_info->file,cache_info->number_pixels*sizeof(PixelPacket)+
      offset*sizeof(IndexPacket),SEEK_SET);
    if (count == -1)
      return(False);
    count=read(cache_info->file,(char *) indexes,region_info->width*
      sizeof(IndexPacket));
    if (count != (region_info->width*sizeof(IndexPacket)))
      return(False);
    indexes+=region_info->width;
    offset+=cache_info->columns;
  }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C a c h e P i x e l s                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadCachePixels reads pixels from the specified region of the pixel
%  cache.
%
%  The format of the ReadCachePixels method is:
%
%      unsigned int ReadCachePixels(Cache cache,
%        const RectangleInfo *region_info,IndexPacket *indexes)
%
%  A description of each parameter follows:
%
%    o status:  Method ReadCachePixels returns True if the pixels are
%      successfully read from the pixel cache, otherwise False.
%
%    o cache_info: Specifies a pointer to a CacheInfo structure.
%
%    o region_info:  The address of a RectangleInfo structure that defines
%      the cache region to read.
%
%    o pixels: The pixels are copied from this PixelPacket address to the
%      pixel cache.
%
%
*/
Export unsigned int ReadCachePixels(Cache cache,
  const RectangleInfo *region_info,PixelPacket *pixels)
{
  CacheInfo
    *cache_info;

  off_t
    count,
    offset;

  register int
    y;

  assert(cache != (Cache *) NULL);
  cache_info=(CacheInfo *) cache;
  offset=region_info->y*cache_info->columns+region_info->x;
  if (cache_info->type != DiskCache)
    {
      /*
        Read pixels from memory.
      */
      if (pixels == (cache_info->pixels+offset))
        return(True);
      for (y=0; y < (int) region_info->height; y++)
      {
        (void) memcpy(pixels,cache_info->pixels+offset,
          region_info->width*sizeof(PixelPacket));
        pixels+=region_info->width;
        offset+=cache_info->columns;
      }
      return(True);
    }
  /*
    Read pixels from disk.
  */
  if (cache_info->file == -1)
    {
      cache_info->file=open(cache_info->filename,O_RDWR | O_BINARY,0777);
      if (cache_info->file == -1)
        return(False);
    }
  for (y=0; y < (int) region_info->height; y++)
  {
    count=lseek(cache_info->file,offset*sizeof(PixelPacket),SEEK_SET);
    if (count == -1)
      return(False);
    count=read(cache_info->file,(char *) pixels,region_info->width*
      sizeof(PixelPacket));
    if (count != (region_info->width*sizeof(PixelPacket)))
      return(False);
    pixels+=region_info->width;
    offset+=cache_info->columns;
  }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C a c h e T h e s h o l d                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SetCacheThreshold sets the amount of free memory allocated for the
%  pixel cache.  Once this threshold is exceeded, all subsequent pixels cache
%  operations are to/from disk.
%
%  The format of the SetCacheThreshold method is:
%
%      void SetCacheThreshold(const off_t threshold)
%
%  A description of each parameter follows:
%
%    o threshold: The number of megabytes of memory available to the pixel
%      cache.
%
%
*/
Export void SetCacheThreshold(const off_t threshold)
{
  off_t
    offset;

  offset=1024*1024*(cache_threshold-threshold);
  (void) GetCacheMemory(-offset);
  cache_threshold=threshold;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S e t C a c h e T y p e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method SetCacheType sets the cache type:  MemoryCache or DiskCache.
%
%  The format of the SetCacheType method is:
%
%      void SetCacheType(Cache cache,const CacheType type)
%
%  A description of each parameter follows:
%
%    o cache: Specifies a pointer to a Cache structure.
%
%    o type: The pixel cache type MemoryCache or DiskCache.
%
%
*/
Export void SetCacheType(Cache cache,const CacheType type)
{
  CacheInfo
    *cache_info;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  if (cache_info->type == UndefinedCache)
    cache_info->type=type;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C a c h e I n d e x e s                                         %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteCachePixels writes the colormap indexes to the specified region
%  of the pixel cache.
%
%  The format of the WriteCachePixels method is:
%
%      unsigned int WriteCachePixels(Cache cache,
%        const RectangleInfo *region_info,const IndexPacket *indexes)
%
%  A description of each parameter follows:
%
%    o status:  Method WriteCachePixels returns True if the colormap indexes
%      are successfully written to the pixel cache, otherwise False.
%
%    o cache_info: Specifies a pointer to a CacheInfo structure.
%
%    o region_info:  The address of a RectangleInfo structure that defines
%      the cache region to write.
%
%    o indexes: The colormap indexes are copied from the pixel cache to this
%      IndexPacket address.
%
%
*/
Export unsigned int WriteCacheIndexes(Cache cache,
  const RectangleInfo *region_info,const IndexPacket *indexes)
{
  CacheInfo
    *cache_info;

  off_t
    count,
    offset;

  register int
    y;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  if (cache_info->class != PseudoClass)
    return(False);
  offset=region_info->y*cache_info->columns+region_info->x;
  if (cache_info->type != DiskCache)
    {
      /*
        Write indexes to memory.
      */
      if (indexes == (cache_info->indexes+offset))
        return(True);
      for (y=0; y < (int) region_info->height; y++)
      {
        (void) memcpy(cache_info->indexes+offset,indexes,
          region_info->width*sizeof(IndexPacket));
        indexes+=region_info->width;
        offset+=cache_info->columns;
      }
      return(True);
    }
  /*
    Write indexes to disk.
  */
  if (cache_info->file == -1)
    {
      cache_info->file=open(cache_info->filename,O_RDWR | O_BINARY,0777);
      if (cache_info->file == -1)
        return(False);
    }
  for (y=0; y < (int) region_info->height; y++)
  {
    count=lseek(cache_info->file,cache_info->number_pixels*sizeof(PixelPacket)+
      offset*sizeof(IndexPacket),SEEK_SET);
    if (count == -1)
      return(False);
    count=write(cache_info->file,(char *) indexes,region_info->width*
      sizeof(IndexPacket));
    if (count != (region_info->width*sizeof(IndexPacket)))
      return(False);
    indexes+=region_info->width;
    offset+=cache_info->columns;
  }
  return(True);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e C a c h e P i x e l s                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method WriteCachePixels writes image pixels to the specified region of the
%  pixel cache.
%
%  The format of the WriteCachePixels method is:
%
%      unsigned int WriteCachePixels(Cache cache,
%        const RectangleInfo *region_info,const PixelPacket *pixels)
%
%  A description of each parameter follows:
%
%    o status:  Method WriteCachePixels returns True if the pixels are
%      successfully written to the cache, otherwise False.
%
%    o cache_info: Specifies a pointer to a CacheInfo structure.
%
%    o region_info:  The address of a RectangleInfo structure that defines
%      the cache region to write.
%
%    o pixels: The pixels are copied from the pixel cache to this PixelPacket
%      address.
%
%
*/
Export unsigned int WriteCachePixels(Cache cache,
  const RectangleInfo *region_info,const PixelPacket *pixels)
{
  CacheInfo
    *cache_info;

  off_t
    count,
    offset;

  register int
    y;

  assert(cache != (Cache) NULL);
  cache_info=(CacheInfo *) cache;
  offset=region_info->y*cache_info->columns+region_info->x;
  if (cache_info->type != DiskCache)
    {
      /*
        Write pixels to memory.
      */
      if (pixels == (cache_info->pixels+offset))
        return(True);
      for (y=0; y < (int) region_info->height; y++)
      {
        (void) memcpy(cache_info->pixels+offset,pixels,
          region_info->width*sizeof(PixelPacket));
        pixels+=region_info->width;
        offset+=cache_info->columns;
      }
      return(True);
    }
  /*
    Write pixels to disk.
  */
  if (cache_info->file == -1)
    {
      cache_info->file=open(cache_info->filename,O_RDWR | O_BINARY,0777);
      if (cache_info->file == -1)
        return(False);
    }
  for (y=0; y < (int) region_info->height; y++)
  {
    count=lseek(cache_info->file,offset*sizeof(PixelPacket),SEEK_SET);
    if (count == -1)
      return(False);
    count=write(cache_info->file,(char *) pixels,region_info->width*
      sizeof(PixelPacket));
    if (count != (region_info->width*sizeof(PixelPacket)))
      return(False);
    pixels+=region_info->width;
    offset+=cache_info->columns;
  }
  return(True);
}
