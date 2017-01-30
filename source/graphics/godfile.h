///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Godfile
//
// 21-MAY-1999
//


#ifndef __GODFILE_H
#define __GODFILE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "blockfile.h"
#include "mesh.h"


///////////////////////////////////////////////////////////////////////////////
//
// class GodFile
//
class GodFile
{
public:

  // Data pointer for reading
  U8 *memPtr, *initPtr;
  U32 size, version;

  // Block file for writing
  BlockFile *bFile;

public:

  // Constructor for reading
  GodFile(U8 *ptr, U32 size, U32 ver) : memPtr(ptr), initPtr(ptr), size(size), bFile(NULL), version(ver) {}

  // Constructor for writing
  GodFile(BlockFile *bFile) : bFile(bFile), memPtr(NULL) {}

  // Look ahead at data
  U32 Peek()
  {
    ASSERT(memPtr)
    return (*(U32 *)memPtr);
  }

  // Advance the pointer
  void Skip(S32 offset)
  {
    ASSERT(memPtr)
    memPtr += offset;
  }

  // Load a U32
  U32 LoadU32()
  {
    ASSERT(memPtr)
    U32 data = *(U32 *)memPtr;
    memPtr += 4;
    return (data);
  }

  // Read a chunk of data
  void LoadData(void *data, U32 size)
  {
    ASSERT(memPtr)
    ASSERT(U32(memPtr - initPtr) + size <= this->size)
    memcpy(data, memPtr, size);
    memPtr += size;
  }

  // Save a chunk of data
  void SaveData(const void *data, U32 size)
  {
    ASSERT(bFile)
    bFile->WriteToBlock(data, size);
  }

  // Read a string
  void LoadStr(char *str, U32 maxLen)
  {
    U16 len;

    LoadData(&len, sizeof(U16));
    len = Min<U16>(U16(maxLen), len);
    LoadData(str, len);
    str[maxLen] = 0;
  }

  // Save a string
  void SaveStr(const char *str)
  {
    U16 len = U16(Utils::Strlen(str) + 1);
    SaveData(&len, sizeof(U16));
    SaveData(str, len);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Namespace God - God reading utility templates
//
namespace God
{

  // Template to read data
  template <class T> void Load(GodFile &god, T &data)
  {
    ASSERT(god.memPtr)
    ASSERT(U32(god.memPtr - god.initPtr) + sizeof(T) <= god.size)
    memcpy(&data, god.memPtr, sizeof(T));
    god.memPtr += sizeof(T);
  }

  // Template to save data
  template <class T> void Save(GodFile &god, const T &data)
  {
    ASSERT(god.bFile)
    god.bFile->WriteToBlock(&data, sizeof T);
  }

  // Array
  template <class T> void LoadArray(GodFile &god, Array<T> &array, U32 max = U32_MAX)
  {
    U32 count = god.LoadU32();
    if (count > max)
    {
      ERR_FATAL(("GodFile::LoadArray: overflow %d ; max %d", count, max));
    }

    if (count)
    {
      array.Alloc(count);

      for (U32 i = 0; i < count; i++)
      {
        Load(god, array[i]);
      }
    }
  }

  template <class T> void SaveArray(GodFile &god, const Array<T> &array)
  {
    Save(god, U32(array.count));
    for (U32 i = 0; i < array.count; i++)
    {
      Save(god, array[i]);
    }
  }

  // Aligned arrays
  template <class T> void LoadArray4(GodFile &god, Array<T, 4> &array, U32 max = U32_MAX)
  {
    U32 count = god.LoadU32();
    if (count > max)
    {
      ERR_FATAL(("GodFile::LoadArray: overflow %d ; max %d", count, max));
    }

    if (count)
    {
      array.Alloc(count);

      for (U32 i = 0; i < count; i++)
      {
        Load(god, array[i]);
      }
    }
  }

  template <class T> void SaveArray4(GodFile &god, const Array<T, 4> &array)
  {
    Save(god, U32(array.count));
    for (U32 i = 0; i < array.count; i++)
    {
      Save(god, array[i]);
    }
  }

  // Sphere specialisation
  template<> void Load<Sphere>(GodFile &god, Sphere &sphere);
  template<> void Save<Sphere>(GodFile &god, const Sphere &sphere);

  // Bounds specialisation
  template<> void Load<Bounds>(GodFile &god, Bounds &bounds);
  template<> void Save<Bounds>(GodFile &god, const Bounds &bounds);

  // Animkey specialisation
  template<> void Load<AnimKey>(GodFile &god, AnimKey &key);
  template<> void Save<AnimKey>(GodFile &god, const AnimKey &key);

  // FaceObj specialisation
  template<> void Load<FaceObj>(GodFile &god, FaceObj &face);
  template<> void Save<FaceObj>(GodFile &god, const FaceObj &face);

  // VertGroup specialisation
  template<> void Load<VertGroup>(GodFile &god, VertGroup &vertGroup);
  template<> void Save<VertGroup>(GodFile &god, const VertGroup &vertGroup);

  // VertIndex specialisation
  template<> void Load<VertIndex>(GodFile &god, VertIndex &vertIndex);
  template<> void Save<VertIndex>(GodFile &god, const VertIndex &vertIndex);

  // BucketDesc specialisation
  template<> void Load<BucketDesc>(GodFile &god, BucketDesc &bucky);
  template<> void Save<BucketDesc>(GodFile &god, const BucketDesc &bucky);
  template<> void Load<BucketLock>(GodFile &god, BucketLock &bucky);
  template<> void Save<BucketLock>(GodFile &god, const BucketLock &bucky);
}

#endif

