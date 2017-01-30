///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Random Number Generation
//
// 9-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Random
//
namespace Random
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  //  Definitions
  //

  // must be equal to 1 + 4k where k is cardinal
  static const U32 MULT = 9579501;

  // must be relatively prime to modulus
  static const U32 INC = 37;

  // modulus
  static const F32 MODULUS = 4294967296.0;
                             
  // inverse of the modulus
  static const F32 MODULUS_INV = 1 / MODULUS;


  //
  // Random Number Streams
  //
  Generator sync;
  Generator nonSync;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Generator
  //

  //
  // Generator::Generator
  //
  // Constructor
  //
  Generator::Generator(U32 seed) :
    seed(seed)
  {
  }


  //
  // Generator::SetSeed
  //
  // sets the current random seed value
  //
  void Generator::SetSeed(U32 newSeed)
  {
    seed = newSeed;
  }


  //
  // Generator::Raw
  //
  // returns a random 32 bit number
  //
  U32 Generator::Raw()
  {
    // get the next random number
    seed = MULT * seed + INC;
    seed = _rotr(seed, 16);
    
    // return raw value
    return (seed);
  }


  //
  // Generator::Integer
  //
  // returns a random number from 0 to mod - 1
  // if mod is zero the return value will be zero
  //
  U32 Generator::Integer(U32 mod)
  {
    // get the next random number
    seed = MULT * seed + INC;
    seed = _rotr(seed, 16);

    // return clipped value
    return ((mod > 0) ? (seed % mod) : 0);
  }


  //
  // Generator::Float
  //
  // returns a random number from 0 to 1
  //
  F32 Generator::Float()
  {
    // get the next random number
    seed = MULT * seed + INC;
    seed = _rotr(seed, 16);

    return (((F32) seed) * MODULUS_INV);
  }

}
