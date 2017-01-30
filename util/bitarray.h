///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Bit array
//
// 16-MAR-1999
//

#ifndef __BITARRAY_H
#define __BITARRAY_H


///////////////////////////////////////////////////////////////////////////////
//
// Class BitArray1d - 1 dimensional bit array
//
class BitArray1d
{
protected:

  // Data
  U8 *data;

  // Size of array in bytes
  U32 blocks;

public:

  //
  // Reset all values in array
  //
  void Reset(U8 value = 0)
  {
    memset(data, value, blocks);
  }

public:

  //
  // Constructor
  //
  BitArray1d(U32 size)
  {
    // Calculate array size
    blocks = ((size + 7) & (~7)) >> 3;

    // Allocate new arrray
    data = new U8[blocks];

    // Reset array
    Reset(0);
  }


  //
  // Destructor
  //
  ~BitArray1d()
  {
    delete[] data;
    data = NULL;
  }


  //
  // Sets the value in the array
  //
  void Set1(U32 offset)
  {
    ASSERT((offset >> 3) < blocks)
    data[offset >> 3] |= U8(1 << (offset & 7));
  }


  //
  // Clears the value in the array
  //
  void Clear1(U32 offset)
  {
    ASSERT((offset >> 3) < blocks)
    data[offset >> 3] &= ~U8(1 << (offset & 7));
  }


  //
  // Retrieves a value in the array
  //
  Bool Get1(U32 offset)
  {
    ASSERT((offset >> 3) < blocks)
    return (data[offset >> 3] & (1 << (offset & 7)));
  }


  //
  // Retrieves a group of 8 values
  //
  U8 &GetBlock(U32 offset)
  {
    ASSERT(offset < blocks)
    return (data[offset]);
  }


  //
  // Returns the pitch of the array
  //
  U32 Blocks()
  {
    return (blocks);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class BitArray2d - 2 dimensional bit array
//
class BitArray2d : public BitArray1d
{
protected:

  // Pitch of each row
  U32 pitch;

public:

  //
  // Constructor
  //
  BitArray2d(U32 x, U32 y) 
  : pitch(((x + 7) & (~7)) >> 3),
    BitArray1d(((x + 7) & (~7)) * y)
  {
  }


  //
  // Sets the value in the array
  //
  void Set2(U32 x, U32 y)
  {
    BitArray1d::Set1((pitch * y * 8) + x);
  }


  //
  // Clears the value in the array
  //
  void Clear2(U32 x, U32 y)
  {
    BitArray1d::Clear1((pitch * y * 8) + x);
  }


  //
  // Retrieves a value in the array
  //
  Bool Get2(U32 x, U32 y)
  {
    return (BitArray1d::Get1((pitch * y * 8) + x));
  }


  //
  // Returns the pitch of the array
  //
  U32 Pitch()
  {
    return (pitch);
  }
};

#endif
