///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Line Of Sight System
//
// 24-NOV-1998
//


#ifndef __SIGHT_BYTEMAP_H
#define __SIGHT_BYTEMAP_H


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sight - The line of sight system
//
namespace Sight
{

  // Max viewing radius
  const U32 MAXR = 20;

  // Dimensions of the sight map
  const U32 MAPSIDE = (2 * MAXR - 1);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ByteMap - Manages 8 see maps
  //
  class ByteMap
  {
  protected:

    // FIXME: NLIST
    ByteMap *prev;
    ByteMap *next;
    U32  id;
    U8   bits;
    U8   bytes[MAPSIDE][MAPSIDE];

  protected:

    // List of all Maps
    static ByteMap *list;

    // Number of maps
    static U32 count;

    // Next available ID
    static U32 nextId;

  protected:

    // Move map to front of list
    void MoveToFront();

  public:

    // Default constructor
    ByteMap();

    // Destructor
    ~ByteMap();

    // Return pointer to bytes
    U8 *GetBytes()
    {
      return (bytes[0]);
    }

    // Get first free bit in ByteMap list
    static void Get(ByteMap * &byteMap, U8 &mask);

    // Free a bit in a bytemap
    static void Unget(ByteMap * &byteMap, U8 mask);

    // Return number of maps allocated
    static U32 GetCount()
    {
      return (count);
    }

    // Return next available Id
    static U32 GetNextId()
    {
      return (nextId);
    }

  };

}

#endif
