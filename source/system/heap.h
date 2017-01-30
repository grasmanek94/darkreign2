///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// heap.h
//
// 13-FEB-2000
//


#ifndef __HEAP_H
#define __HEAP_H


///////////////////////////////////////////////////////////////////////////////
// 
// Includes
//

///////////////////////////////////////////////////////////////////////////////
//
// Template Heap
//
class Heap
{
private:

  struct Record
  {
    U32 size;
  };

protected:

  char * top, * current;
  U32 size, used;

public:

  //
  // Heap
  //
  // Constructor
  //
  Heap()
  {
    ClearData();
  }
  Heap( U32 _size)
  {
    ClearData();
    Alloc( _size);
  }  

  //
  // Heap
  //
  // Destructor
  //
  ~Heap()
  {
    Release();
  }

  void ClearData();
  void Alloc( U32 _size);
  void Release();

  void * Request( U32 _size);

  struct Vector;
  struct VertexTL;
  struct VertexL;
  struct Vertex;

  Vector *    ReqVector(   U32 _count);
  U16 *       ReqIndex(    U32 _count);
  VertexTL *  ReqVertexTL( U32 _count);
  VertexL  *  ReqVertexL(  U32 _count);
  Vertex   *  ReqVertex(   U32 _count);

  void Restore();

  
};


#endif 