///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// array.h    simple array template
//
// 9-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "debug_memory.h"


#ifndef __ARRAY_H
#define __ARRAY_H
//----------------------------------------------------------------------------

template <class DATA, U32 ALIGNMENT = 0> class Array
{
public:
  U32                     count, size;
  DATA                    * data;

  void ClearData()
  {
    data = NULL;
    count = size = 0;
  }
  Array()
  {
    ClearData();
  }
  Array( U32 c)
  {
    ClearData();
    Alloc( c);
  }

  ~Array()
  {
    Release();
  }
  void Release()
  {
    if (data)
    {
      Debug::Memory::Aligning::AligningFree( data);
      data = NULL;
    }
    count = size = 0;
  }
  DATA * Alloc( U32 c)
  {
    if (data)
    {
      if (count == c)
      {
        return data;
      }
      Release();
    }
    count = c;
    size = count * sizeof( DATA);
    return data = (DATA *) Debug::Memory::Aligning::AligningAlloc( size, ALIGNMENT);
  }
  DATA * Setup( const Array<DATA> &array)
  {
    if (!Alloc( array.count))
    {
      return NULL;
    }
    Copy( array);

    return data;
  }
  void Copy( const Array<DATA> & array)
  {
    memcpy( data, array.data, array.count * sizeof( DATA));
  }

  DATA * Setup( U32 _count, const DATA * array)
  {
    if (!Alloc( _count))
    {
      return NULL;
    }
    Copy( array);

    return data;
  }
  void Copy( const DATA * array)
  {
    memcpy( data, array, count * sizeof( DATA));
  }
  void Swap( Array<DATA> & array)
  {
    U32 i = count;
    count = array.count;
    array.count = i;
    i = size;
    size = array.size;
    array.size = i;
    DATA * d = data;
    data = array.data;
    array.data = d;
  }

  DATA & operator[]( U32 index)
  {
    return data[ index];
  }
  DATA & operator[]( U32 index) const
  {
    return data[ index];
  }

};
//----------------------------------------------------------------------------

// pads array data to always contain multiples of 4 entries
// count is as passed in
//
template <class DATA, U32 ALIGNMENT = 0> class Array4 : public Array<DATA, ALIGNMENT>
{
public:
  Array4()
  {
    ClearData();
  }
  Array4( U32 c)
  {
    ClearData();
    Alloc( c);
  }

  DATA * Alloc( U32 c)
  {
    if (data)
    {
      if (count == c)
      {
        return data;
      }
      Release();
    }
    count = c;
    U32 mod = c % 4;
    if (mod)
    {
      c += 4 - mod;
    }
    size = c * sizeof( DATA);
    return data = (DATA *) Debug::Memory::Aligning::AligningAlloc( size, ALIGNMENT);
  }
  DATA * Setup( const Array<DATA> & array)
  {
    if (!Alloc( array.count))
    {
      return NULL;
    }
    Copy( array);

    return data;
  }
  DATA * Setup( U32 _count, const DATA * array)
  {
    if (!Alloc( _count))
    {
      return NULL;
    }
    Copy( array);

    return data;
  }
  void Copy( const DATA * array)
  {
    memcpy( data, array, count * sizeof( DATA));
  }

  void Copy( const Array4<DATA,ALIGNMENT> & array)
  {
    memcpy( data, array.data, array.count * sizeof( DATA));
  }
  DATA * Setup( const Array4<DATA,ALIGNMENT> & array)
  {
    if (!Alloc( array.count))
    {
      return NULL;
    }
    Copy( array);

    return data;
  }

  void Swap( Array4<DATA,ALIGNMENT> & array)
  {
    U32 i = count;
    count = array.count;
    array.count = i;
    i = size;
    size = array.size;
    array.size = i;
    DATA * d = data;
    data = array.data;
    array.data = d;
  }
};
//----------------------------------------------------------------------------

#endif		// __ARRAY_H
