///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Heap.cpp
//
// 13-FEB-2000
//


///////////////////////////////////////////////////////////////////////////////
// 
// Includes
//
#include "heap.h"
//----------------------------------------------------------------------------

//
// Heap
//
// Constructor
//
void Heap::ClearData() 
{
  top = current = NULL;
  size = used = 0;
}
//----------------------------------------------------------------------------

void Heap::Alloc( U32 _size)
{
  if (!top || _size != size)
  {
    if (top)
    {
      delete [] top;
    }
    size = _size;
    top = new char[size];
  }
  used = 0;
  current = top;
}
//----------------------------------------------------------------------------

void Heap::Release()
{
  if (top)
  {
    delete [] top;
  }
  ClearData();
}
//----------------------------------------------------------------------------

void * Heap::Request( U32 _size)
{
  ASSERT( current && current <= top + size);

  _size += sizeof(Record);

  if (used + _size > size)
  {
    LOG_DIAG(("Heap::Request: out of space"));
    return NULL;
  }
  void * ret = (void *) current;

  current += _size;
  ((Record *)(current - sizeof(Record)))->size  =_size;
  used = _size;

  return ret;
}
//----------------------------------------------------------------------------

void Heap::Restore()
{
  ASSERT( current && current <= top + size);

  U32 s = ((Record *)(current - sizeof(Record)))->size;
  current -= s;
  used -= s;
}
//----------------------------------------------------------------------------