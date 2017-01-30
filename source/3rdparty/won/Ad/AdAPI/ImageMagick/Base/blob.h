/*
  Image Compression/Decompression Methods.
*/
#ifndef _BLOBS_H
#define _BLOBS_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if !defined(BlobQuantum)
#define BlobQuantum  65535
#endif

/*
  Blob methods.
*/
extern Export char
  *GetStringBlob(Image *,char *);

extern Export Image
  *BlobToImage(const ImageInfo *,const char *,const size_t);

extern Export int
  EOFBlob(const Image *),
  ReadByte(Image *),
  SyncBlob(const Image *);

extern Export off_t
  SeekBlob(Image *,const off_t,const int),
  SizeBlob(Image *image),
  TellBlob(Image *image);

extern Export size_t
  LSBFirstWriteLong(Image *,const unsigned long),
  LSBFirstWriteShort(Image *,const unsigned int),
  MSBFirstWriteLong(Image *,const unsigned long),
  MSBFirstWriteShort(Image *,const unsigned int),
  ReadBlob(Image *,const size_t,void *),
  ReadBlobBlock(Image *,char *),
  WriteBlob(Image *,const size_t,const void *),
  WriteByte(Image *,const int value);

extern Export unsigned int
  OpenBlob(const ImageInfo *,Image *,const char *),
  UnmapBlob(void *,const size_t);

extern Export unsigned short
  LSBFirstReadShort(Image *),
  MSBFirstReadShort(Image *);

extern Export unsigned long
  LSBFirstReadLong(Image *),
  MSBFirstReadLong(Image *);

extern Export void
  CloseBlob(Image *),
  DestroyBlobInfo(BlobInfo *),
  *ImageToBlob(const ImageInfo *,Image *,size_t *),
  *MapBlob(int,const MapMode,size_t *),
  MSBFirstOrderLong(char *,const size_t),
  MSBFirstOrderShort(char *,const size_t),
  GetBlobInfo(BlobInfo *),
  SetBlobQuantum(BlobInfo *,const size_t);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
