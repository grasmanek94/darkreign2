///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Line Of Sight System
//
// 24-NOV-1998
//


#include "sight_bytemap.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Sight - The line of sight system
//
namespace Sight
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ByteMap - Manages 8 see maps
  //

  //
  // Static data
  //

  // List of all Maps
  ByteMap *ByteMap::list = NULL;

  // Number of maps
  U32 ByteMap::count = 0;

  // Next available ID
  U32 ByteMap::nextId = 0;


  //
  // Default constructor
  //
  ByteMap::ByteMap() : prev(NULL), next(NULL)
  {
    // Add to start of list
    if (list)
    {
      list->prev = this;
    }

    next = list;
    prev = NULL;
    bits = 0;
    id   = nextId++;
    list = this;

    Utils::Memset(bytes, 0, sizeof(bytes));

    ++count;
  }


  //
  // Destructor
  //
  ByteMap::~ByteMap()
  {
    // Unlink previous
    if (prev)
    {
      prev->next = next;
    }
    else
    {
      list = next;
    }

    // Unlink next
    if (next)
    {
      next->prev = prev;
    }

    // Decrement count
    --count;
  }


  //
  // Move to front of list
  //
  void ByteMap::MoveToFront()
  {
    // Ignore If already at front of list
    if (!prev)
    {
      return;
    }

    // Unlink from list
    prev->next = next;

    if (next)
    {
      next->prev = prev;
    }

    // Add to front
    next = list;
    prev = NULL;
    list->prev = this;
    list = this;
  }


  //
  // Get first free bit in ByteMap list
  //
  void ByteMap::Get(ByteMap * &byteMap, U8 &mask)
  {
    ASSERT(!byteMap);

    U8 bit, bits;

    // make sure first bytemap has a free entry, allocate if necessary
    if (!list || list->bits == 0xff)
    {
      new ByteMap;
    }

    // find first vacant bit
    bits = list->bits;

    for (bit = 1; bit <= 0x80; bit <<= 1)
    {
      if ((bits & bit) == 0)
      {
        byteMap = list;
        mask    = bit;

        // clear all bits at position bit in bytemask
        for (U32 i = 0; i < MAPSIDE; i++)
        {
          for (U32 j = 0; j < MAPSIDE; j++)
          {
            byteMap->bytes[i][j] &= ~bit;
          }
        }

        // set the bit in the bytemaplist to flag use
        byteMap->bits |= bit;

        return;
      }
    }

    ASSERT(byteMap);
  }


  //
  // Free a bit in a bytemap
  //
  void ByteMap::Unget(ByteMap * &byteMap, U8 mask)
  {
    ASSERT(byteMap);
    ASSERT(byteMap->bits & mask);

    // if no bits left free bytemap, else move it to start
    if ((byteMap->bits &= (~mask)) == 0)
    {
      delete byteMap;
    }
    else
    {
      byteMap->MoveToFront();
    }

    byteMap = NULL;
  }

}
