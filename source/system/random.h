///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Random Number Generation
//
// 9-APR-1998
//


#ifndef __RANDOM_H
#define __RANDOM_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Random
//
namespace Random
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Generator - Provides random numbers via the Linear Congruential Method
  //
  class Generator
  {
  private:

    // Current seed value 
    U32 seed;

  public:

    // Constructor
    Generator(U32 seed = 0);

    // sets the random number seed
    void SetSeed(U32 newSeed);

    // returns a random 32 bit number
    U32 Raw();

    // returns a random number from 0 to mod - 1
    // if mod is zero the return value will be zero
    U32 Integer(U32 mod);

    // returns a random number from 0 to 1
    F32 Float();

  };

  // Random Number Streams
  extern Generator sync;
  extern Generator nonSync;

}


#endif
